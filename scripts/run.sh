#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bin="$script_dir/../build/bin/SMTStabilizer"
solver="/home/zhangx/exectool/xbv_exp/solver/z3"
prefix="/pub/netdisk1/zhangx/benchmark-2025/non-incremental/"

usage() {
	echo "Usage: $(basename "$0") <file>" >&2
	exit 2
}

if [ "$#" -lt 1 ]; then
	usage
fi

input=$prefix"$1"
if [ ! -e "$input" ]; then
	echo "Input file not found: $input" >&2
	exit 2
fi


if [ ! -x "$bin" ]; then
	echo "SMTStabilizer binary not found or not executable: $bin" >&2
	exit 3
fi

if [ ! -x "$solver" ]; then
	echo "z3 solver not found or not executable: $solver" >&2
	exit 4
fi

smt_tmp="$(mktemp)"
out_tmp="$(mktemp)"
trap 'rm -f "$smt_tmp" "$out_tmp"' EXIT

# Generate SMT output
if ! "$bin" "$input" >"$smt_tmp" 2>&1; then
	echo "SMTStabilizer failed generating SMT output:" >&2
	cat "$smt_tmp" >&2
	exit 5
fi

# Try solver with -in first, then fallback to reading from stdin
"$solver" -in <"$smt_tmp" >"$out_tmp" 2>&1
rc=$?
if [ "$rc" -eq 0 ]; then
	cat "$out_tmp"
	exit 0
fi

"$solver" <"$smt_tmp" >"$out_tmp" 2>&1
rc=$?
cat "$out_tmp"
exit $rc

