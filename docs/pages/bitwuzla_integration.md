# Bitwuzla Integration

SMTStabilizer can optionally integrate with the Bitwuzla SMT solver library to
provide a tighter coupling with Bitwuzla-based workflows.

## Enabling via `setup.py`

On Linux/macOS you can ask the repository `setup.py` to download, build, and
stage Bitwuzla into the repository `submodules` layout by passing the
`--bitwuzla` flag:

```bash
python3 setup.py --bitwuzla
```

This will place headers under `submodules/include` and libraries under
`submodules/lib` (or `lib64` depending on the platform). The main CMake build
only enables Bitwuzla when these staged files exist; do not attempt to force
Bitwuzla enablement via a CMake flag. When Bitwuzla support is available, the
build defines the macro `SMTSTABILIZER_HAVE_BITWUZLA` — use it for
conditional compilation:

```cpp
#ifdef SMTSTABILIZER_HAVE_BITWUZLA
  // Call into Bitwuzla API
#else
  // Bitwuzla not available; handle gracefully or error as appropriate
#endif
```

## Detection rules

- Bitwuzla support is enabled only when the staged bundled layout is present.
- CMake checks for `submodules/include/bitwuzla/` plus the bundled static
  archives `submodules/lib/libbitwuzla.a`, `submodules/lib/libbitwuzlabv.a`,
  `submodules/lib/libbitwuzlals.a`, and `submodules/lib/libbitwuzlabb.a`.
- If all staged files are present, the build defines `SMTSTABILIZER_HAVE_BITWUZLA`.
- The project does not search the system for Bitwuzla or allow forcing
  enablement via a CMake flag; if the staged files are not present, Bitwuzla
  support remains disabled.