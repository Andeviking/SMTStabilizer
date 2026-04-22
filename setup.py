import hashlib
import os
import string
import subprocess
import sys
import shutil
import time
import tempfile
import urllib.request
import contextlib
import zipfile
import tarfile
from pathlib import Path
from typing import Optional

# --------------- Colored printing utilities ----------------
_COLORS = {
    "reset": "\033[0m",
    "red": "\033[31m",
    "green": "\033[32m",
    "yellow": "\033[33m",
    "blue": "\033[34m",
    "magenta": "\033[35m",
    "cyan": "\033[36m",
    "bold": "\033[1m",
}


def color_text(text: str, color: str) -> str:
    return f"{_COLORS.get(color, '')}{text}{_COLORS['reset']}"


def info(msg: str):
    print(color_text(msg, "blue"))


def success(msg: str):
    print(color_text(msg, "green"))


def warn(msg: str):
    print(color_text(msg, "yellow"))


def error(msg: str):
    print(color_text(msg, "red"), file=sys.stderr)


# --------------- Progress printer ----------------
class ProgressPrinter:
    def __init__(self, prefix: str = "", color: str = "cyan"):
        self.prefix = prefix
        self.color = color
        self._start = time.time()
        self._last_len = 0
        self._spinner = "|/-\\"
        self._spin_i = 0

    def _write(self, text: str):
        sys.stdout.write("\r\033[2K" + text)
        sys.stdout.flush()
        self._last_len = len(text)

    def info(self, msg: str):
        self._write(color_text(f"{self.prefix} {msg}", self.color))

    def progress(self, done: int, total: Optional[int] = None):
        if total:
            pct = (done / total) * 100 if total else 0
            text = f"{self.prefix} {done}/{total} ({pct:5.1f}%)"
        else:
            spin = self._spinner[self._spin_i % len(self._spinner)]
            self._spin_i += 1
            text = f"{self.prefix} {spin} {done}"
        self._write(color_text(text, self.color))

    def finish(self, msg: Optional[str] = None, success_flag: bool = True):
        if msg is None:
            msg = "done" if success_flag else "failed"
        final = color_text(f"{self.prefix} {msg}", "green" if success_flag else "red")
        sys.stdout.write("\r\033[2K" + final + "\n")
        sys.stdout.flush()


# --------------- File download ----------------
def download_file(url: str, dest_dir: str, filename: Optional[str] = None,
                  retries: int = 3, timeout: int = 30, sha256: Optional[str] = None) -> Path:
    """
    Download a file to dest_dir and return the full path. Shows progress.

    Optional sha256:
      - If None: skip verification.
      - If a 64-char hex string: treated as the expected checksum.
      - If a local path: read the file and take the first token as checksum.
      - If an http(s) URL: download the file and take the first token as checksum.

    On checksum mismatch the file is removed and a retry is attempted (up to retries).
    """
    dest_dir = Path(dest_dir)
    dest_dir.mkdir(parents=True, exist_ok=True)
    if not filename:
        filename = Path(urllib.request.urlsplit(url).path).name or "downloaded.file"
    dest_path = dest_dir / filename

    # Resolve expected checksum if provided
    expected: Optional[str] = None
    if sha256:
        s = sha256.strip()
        try:
            if s.startswith(("http://", "https://")):
                info(f"Fetching checksum from URL: {s}")
                with contextlib.closing(urllib.request.urlopen(s, timeout=timeout)) as resp:
                    body = resp.read().decode(errors="ignore")
                    token = body.strip().split()[0] if body.strip() else ""
                    expected = token.strip()
            elif Path(s).exists():
                with open(s, "r", encoding="utf-8", errors="ignore") as fh:
                    token = fh.read().strip().split()[0] if fh.readable() else ""
                    expected = token.strip()
            else:
                expected = s
        except Exception as e:
            warn(f"Unable to read provided sha256 source ({sha256}): {e}")
            expected = None

        if expected:
            expected = expected.lower()
            if not (len(expected) == 64 and all(c in string.hexdigits for c in expected)):
                warn(f"Ignoring invalid sha256 value: {expected}")
                expected = None

    # If file already exists, optionally verify checksum
    if dest_path.exists():
        if expected:
            info(f"Verifying existing file checksum: {dest_path}")
            h = hashlib.sha256()
            with open(dest_path, "rb") as f:
                for chunk in iter(lambda: f.read(8192), b""):
                    h.update(chunk)
            if h.hexdigest() == expected:
                info(f"File exists and SHA256 matches, skipping download: {dest_path}")
                return dest_path
            else:
                warn(f"Existing file SHA256 mismatch, will re-download: {dest_path}")
                try:
                    dest_path.unlink()
                except Exception:
                    pass
        else:
            info(f"File already exists, skipping download: {dest_path}")
            return dest_path

    attempt = 0
    while attempt < retries:
        attempt += 1
        try:
            info(f"Starting download: {url} -> {dest_path} (attempt {attempt}/{retries})")
            with contextlib.closing(urllib.request.urlopen(url, timeout=timeout)) as resp:
                total = resp.getheader("Content-Length")
                total = int(total) if total and total.isdigit() else None
                printer = ProgressPrinter(prefix=f"[download]", color="blue")
                hasher = hashlib.sha256() if expected else None
                with open(dest_path, "wb") as f:
                    downloaded = 0
                    chunk_size = 8192
                    while True:
                        chunk = resp.read(chunk_size)
                        if not chunk:
                            break
                        f.write(chunk)
                        if hasher is not None:
                            hasher.update(chunk)
                        downloaded += len(chunk)
                        printer.progress(downloaded, total)
                printer.finish("Download complete", True)

                # verify checksum if requested
                if expected:
                    got = hasher.hexdigest()
                    if got != expected:
                        # remove bad file and raise to trigger retry logic
                        try:
                            dest_path.unlink()
                        except Exception:
                            pass
                        raise RuntimeError(f"SHA256 mismatch: expected {expected}, got {got}")
            return dest_path
        except Exception as e:
            warn(f"Download failed (attempt {attempt}/{retries}): {e}")
            if dest_path.exists():
                try:
                    dest_path.unlink()
                except Exception:
                    pass
            if attempt >= retries:
                error(f"Unable to download file: {url}")
                raise
            time.sleep(1 + attempt)
    raise RuntimeError("Should not reach here")


# --------------- Extract archive ----------------
def extract_archive(archive_path: str, dest_dir: str):
    """
    Supports zip, tar, tar.gz, tgz, tar.bz2. Shows progress.
    """
    archive_path = Path(archive_path)
    dest_dir = Path(dest_dir)
    dest_dir.mkdir(parents=True, exist_ok=True)
    info(f"Extracting: {archive_path} -> {dest_dir}")

    if zipfile.is_zipfile(archive_path):
        with zipfile.ZipFile(archive_path, "r") as z:
            members = z.namelist()
            printer = ProgressPrinter(prefix="[unzip]", color="magenta")
            for i, member in enumerate(members, 1):
                z.extract(member, path=dest_dir)
                printer.progress(i, len(members))
            printer.finish("Extraction complete", True)
        return

    # Try tar (manual safe extraction to avoid tarfile.extract deprecation)
    try:
        with tarfile.open(archive_path, "r:*") as t:
            members = t.getmembers()
            printer = ProgressPrinter(prefix="[untar]", color="magenta")
            base_resolved = dest_dir.resolve()
            for i, member in enumerate(members, 1):
                member_name = member.name
                target = dest_dir.joinpath(member_name)
                try:
                    target_resolved = target.resolve()
                except Exception:
                    warn(f"Skipping unreadable entry: {member_name}")
                    printer.progress(i, len(members))
                    continue
                if not str(target_resolved).startswith(str(base_resolved)):
                    warn(f"Skipping potentially unsafe path in archive: {member_name}")
                    printer.progress(i, len(members))
                    continue

                if member.isdir():
                    target.mkdir(parents=True, exist_ok=True)
                elif member.issym() or member.islnk():
                    warn(f"Skipping symlink/hardlink: {member_name}")
                else:
                    fobj = t.extractfile(member)
                    if fobj is None:
                        printer.progress(i, len(members))
                        continue
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with open(target, "wb") as out_f:
                        shutil.copyfileobj(fobj, out_f)
                    try:
                        os.chmod(target, member.mode)
                    except Exception:
                        pass
                    try:
                        if getattr(member, "mtime", None) is not None:
                            os.utime(target, (member.mtime, member.mtime))
                    except Exception:
                        pass

                printer.progress(i, len(members))
            printer.finish("Extraction complete", True)
        return
    except tarfile.ReadError:
        pass

    raise ValueError(f"Unsupported archive type: {archive_path}")


# --------------- Copy directory ----------------
def copy_folder(src: str, dst: str, patterns=None, overwrite: bool = False):
    """
    Copy all files in src that match the given patterns into dst, preserving their relative directory structure.
    - patterns: a single glob pattern string or a list of strings; defaults to "**/*" (matches all files).
    - overwrite: if True, existing target files with the same name will be overwritten; otherwise existing files are skipped.
    Behavior notes:
    - Does not remove existing directories or files under dst; only creates missing parent directories as needed.
    - Only copies matched regular files (skips directories, unreadable items, or entries that would traverse outside the source path).
    """
    src = Path(src)
    dst = Path(dst)
    if not src.exists():
        raise FileNotFoundError(f"Source directory does not exist: {src}")
    if not src.is_dir():
        raise NotADirectoryError(f"Source is not a directory: {src}")

    if patterns is None:
        patterns = ["**/*"]
    elif isinstance(patterns, str):
        patterns = [patterns]
    else:
        try:
            patterns = list(patterns)
        except Exception:
            patterns = ["**/*"]

    matched = []
    seen = set()
    for pat in patterns:
        for p in src.rglob(pat):
            try:
                if p.is_file():
                    rp = p.resolve()
                    if rp not in seen:
                        seen.add(rp)
                        matched.append(p)
            except Exception:
                continue

    total_files = len(matched)
    if total_files == 0:
        info(f"No files matched pattern(s) {patterns} under {src}")
        return

    printer = ProgressPrinter(prefix="[copy]", color="cyan")
    copied = 0

    for i, entry in enumerate(matched, start=1):
        try:
            rel = entry.relative_to(src)
        except Exception:
            warn(f"Skipping unreadable/unrelated entry: {entry}")
            printer.progress(i, total_files)
            continue

        target = dst.joinpath(rel)
        target.parent.mkdir(parents=True, exist_ok=True)

        if target.exists():
            if not overwrite:
                printer.progress(i, total_files)
                continue
            else:
                try:
                    if target.is_file():
                        target.unlink()
                except Exception:
                    pass

        try:
            shutil.copy2(entry, target)
        except Exception as e:
            warn(f"Failed to copy {entry} -> {target}: {e}")
            printer.progress(i, total_files)
            continue

        copied += 1
        printer.progress(i, total_files)

    printer.finish(f"Copied {copied}/{total_files} files", True)

cache_dir = ""
submodules_dir = ""
include_dir = ""
lib_dir = ""

# ...existing code...
def build_mpfr():
    """
    Download, extract, configure, build and install MPFR into the submodules prefix.
    Uses global paths: cache_dir, submodules_dir, include_dir, lib_dir.
    """
    global cache_dir, submodules_dir, include_dir, lib_dir

    url = "https://www.mpfr.org/mpfr-current/mpfr-4.2.2.tar.gz"
    archive = download_file(url, cache_dir, sha256="826cbb24610bd193f36fde172233fb8c009f3f5c2ad99f644d0dea2e16a20e42")
    extract_archive(archive, cache_dir)

    source_dir = Path(cache_dir) / "mpfr-4.2.2"
    if not source_dir.exists():
        raise FileNotFoundError(f"MPFR source not found after extraction: {source_dir}")

    prefix = source_dir / "mpfr-install"
    jobs = max(1, (os.cpu_count() or 1))

    info(f"Building MPFR from {source_dir} -> prefix={prefix} with {jobs} jobs")
    try:
        # Configure
        info("Running configure")
        env = os.environ.copy()
        opt_flags = "-O3 -march=native -mtune=native -flto -fno-strict-aliasing -fwrapv -pipe"
        env.update({
            "CFLAGS": opt_flags,
            "CXXFLAGS": opt_flags + " -std=gnu++20",
            "LDFLAGS": "-flto",
        })

        # Point MPFR configure to the staged GMP include/lib that this script maintains
        cfg_cmd = [
            "./configure",
            f"--prefix={prefix}",
            "--enable-static",
            "--disable-shared",
            "--with-pic",
            f"--with-gmp-include={str(include_dir)}",
            f"--with-gmp-lib={str(lib_dir)}",
        ]
        info(f"Configuring MPFR: {' '.join(cfg_cmd)}")
        subprocess.run(cfg_cmd, cwd=str(source_dir), check=True, env=env)

        # Build
        info(f"Running make -j{jobs}")
        subprocess.run(["make", f"-j{jobs}"], cwd=str(source_dir), check=True, env=env)

        # Install
        info("Running make install")
        subprocess.run(["make", "install"], cwd=str(source_dir), check=True, env=env)

    except subprocess.CalledProcessError as e:
        error(f"MPFR build step failed: {e}")
        raise

    inc_src = prefix / "include"
    lib_src = prefix / "lib"

    if inc_src.exists():
        info(f"Copying headers from {inc_src} -> {include_dir}")
        try:
            copy_folder(str(inc_src), str(include_dir), patterns="**/*.h*", overwrite=True)
        except Exception as e:
            warn(f"Failed to copy headers: {e}")

    if lib_src.exists():
        info(f"Copying libs from {lib_src} -> {lib_dir}")
        try:
            copy_folder(str(lib_src), str(lib_dir), patterns="**/*.a", overwrite=True)
        except Exception as e:
            warn(f"Failed to copy libraries: {e}")

    info("MPFR build and installation complete.")
# ...existing code...

def build_gmp():
    """
    Download, extract, configure, build and install GMP into the submodules prefix.
    Uses global paths: cache_dir, submodules_dir (both should be Path objects after __main__ runs).
    """
    global cache_dir, submodules_dir, include_dir, lib_dir

    url = "https://gmplib.org/download/gmp-6.3.0/gmp-6.3.0.tar.gz"
    # download archive into cache_dir
    archive = download_file(url, cache_dir, sha256="e56fd59d76810932a0555aa15a14b61c16bed66110d3c75cc2ac49ddaa9ab24c")
    # extract into cache_dir
    extract_archive(archive, cache_dir)

    source_dir = Path(cache_dir) / "gmp-6.3.0"
    if not source_dir.exists():
        raise FileNotFoundError(f"GMP source not found after extraction: {source_dir}")

    prefix = source_dir / "gmp-install"
    jobs = max(1, (os.cpu_count() or 1))

    info(f"Building GMP from {source_dir} -> prefix={prefix} with {jobs} jobs")
    try:
        # Configure
        info("Running configure")
        env = os.environ.copy()
        opt_flags = "-O3 -march=native -mtune=native -flto -fno-strict-aliasing -fwrapv -pipe"
        env.update({
            "CFLAGS": opt_flags,
            "CXXFLAGS": opt_flags + " -std=gnu++20",
            "LDFLAGS": "-flto",
        })
        cfg_cmd = [
            "./configure",
            f"--prefix={prefix}",
            "--enable-cxx",
            "--enable-static",
            "--disable-shared",
            "--with-pic",
        ]
        info(f"Configuring GMP: {' '.join(cfg_cmd)}")
        subprocess.run(cfg_cmd, cwd=str(source_dir), check=True, env=env)

        # Build
        info(f"Running make -j{jobs}")
        subprocess.run(["make", f"-j{jobs}"], cwd=str(source_dir), check=True, env=env)

        # Install
        info("Running make install")
        subprocess.run(["make", "install"], cwd=str(source_dir), check=True, env=env)

    except subprocess.CalledProcessError as e:
        error(f"Build step failed: {e}")
        raise

    inc_src = prefix / "include"
    lib_src = prefix / "lib"

    if inc_src.exists():
        info(f"Copying headers from {inc_src} -> {include_dir}")
        try:
            copy_folder(str(inc_src), str(include_dir), patterns="**/*.h", overwrite=True)
        except Exception as e:
            warn(f"Failed to copy headers: {e}")

    if lib_src.exists():
        info(f"Copying libs from {lib_src} -> {lib_dir}")
        try:
            copy_folder(str(lib_src), str(lib_dir), patterns="**/*.a", overwrite=True)
        except Exception as e:
            warn(f"Failed to copy libraries: {e}")
    
    info("GMP build and installation complete.")



if __name__ == "__main__":
    # Determine a robust absolute base directory (script location)
    base_dir = Path(__file__).parent.resolve()

    # Allow overrides via environment variables, otherwise use sane defaults under repo
    submodules_dir = Path(os.environ.get("SMT_SUBMODULES_DIR", str(base_dir / "submodules"))).expanduser().resolve(strict=False)
    cache_dir = Path(os.environ.get("SMT_CACHE_DIR", str(submodules_dir / "cache"))).expanduser().resolve(strict=False)
    include_dir = Path(os.environ.get("SMT_INCLUDE_DIR", str(submodules_dir / "include"))).expanduser().resolve(strict=False)
    lib_dir = Path(os.environ.get("SMT_LIB_DIR", str(submodules_dir / "lib"))).expanduser().resolve(strict=False)

    dirs_to_create = [submodules_dir, cache_dir, include_dir, lib_dir]

    info(f"Base directory: {base_dir}")
    for d in dirs_to_create:
        try:
            # create directory tree if missing, safe if already exists
            d.mkdir(parents=True, exist_ok=True)
            success(f"Ensured directory: {d}")
        except Exception as e:
            error(f"Failed to create directory {d}: {e}")
            sys.exit(1)

    # Now run build step (uses the absolute paths above if needed)
    try:
        build_gmp()
    except Exception as e:
        error(f"build_gmp failed: {e}")
        sys.exit(1)

    try:
        build_mpfr()
    except Exception as e:
        error(f"build_mpfr failed: {e}")
        sys.exit(1)