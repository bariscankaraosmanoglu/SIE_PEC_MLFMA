#!/usr/bin/env python3
"""Build and run thread regressions; all binaries and outputs go outside the repo."""
import argparse
from array import array
import hashlib
import math
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def relative_error(reference, result):
    left, right = array("d"), array("d")
    left.frombytes(reference.read_bytes())
    right.frombytes(result.read_bytes())
    assert len(left) == len(right)
    assert all(math.isfinite(x) for x in left) and all(math.isfinite(x) for x in right)
    error = sum((x - y) ** 2 for x, y in zip(left, right))
    norm = sum(x * x for x in left)
    return math.sqrt(error / norm) if norm else math.sqrt(error)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=ROOT / "src/input.pnu", help="Existing local mesh input")
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--sanitize", action="store_true", help="AddressSanitizer and UBSan; existing leaks excluded")
    parser.add_argument("--baseline-dir", type=Path, help="Compare to saved one-thread operator outputs")
    args = parser.parse_args()
    output = args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-thread-"))
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    flags = ["-std=gnu++17", "-fopenmp", "-I" + str(ROOT / "src")]
    flags += (["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
              if args.sanitize else ["-O3"])
    env = os.environ.copy()
    env.update(OMP_DYNAMIC="FALSE", ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    env.pop("OMP_THREAD_LIMIT", None)
    builds = [
        ("partitions", [ROOT / "tests/thread_partitions.cpp", ROOT / "src/calculate_loadbalance.cpp"]),
        ("operators", [ROOT / "tests/thread_operators.cpp"] + sorted(
            p for p in (ROOT / "src").glob("*.cpp") if p.name != "main.cpp")),
    ]
    for name, sources in builds:
        command = ["g++", *flags, *map(str, sources), "-o", str(output / name)]
        with (output / (name + "-build.log")).open("w") as log:
            subprocess.run(command, stdout=log, stderr=log, check=True)
    subprocess.run([str(output / "partitions")], env=env, check=True)
    limited = dict(env, OMP_THREAD_LIMIT="2")
    subprocess.run([str(output / "partitions")], env=limited, check=True)
    for invalid in ("0", "-1"):
        result = subprocess.run([str(output / "partitions"), invalid], env=env, capture_output=True, text=True)
        assert result.returncode == 1 and "Invalid thread count" in result.stderr

    fixture = args.input.read_text().splitlines()
    # Exercise both accurate and approximate operators with valid interpolation sizes.
    fixture = ["Preconditioners 2" if s.startswith("Preconditioners ") else
               "val_approx 0.8" if s.startswith("val_approx ") else s for s in fixture]
    cases = [("serial", 1, 1, env), ("off8", 0, 8, env)]
    cases += [(f"threads{n}", 1, n, env) for n in (2, 8, 16, 32)]
    cases += [("limited32", 1, 32, limited), ("dynamic32", 1, 32, dict(env, OMP_DYNAMIC="TRUE"))]
    for name, parallel, threads, case_env in cases:
        directory = output / name
        directory.mkdir(exist_ok=True)
        (directory / "input.pnu").write_text("\n".join(fixture) + "\n")
        command = ["/usr/bin/time", "-f", "elapsed=%e peak_rss_kib=%M", "-o", "resources.txt",
                   str(output / "operators"), str(parallel), str(threads)]
        with (directory / "run.log").open("w") as log:
            subprocess.run(command, cwd=directory, env=case_env, stdout=log, stderr=log, check=True)
        reference = output / "serial"
        assert hashlib.sha256((reference / "near_matrix.bin").read_bytes()).digest() == hashlib.sha256(
            (directory / "near_matrix.bin").read_bytes()).digest(), name
        errors = {filename: relative_error(reference / filename, directory / filename)
                  for filename in ("near.bin", "far.bin", "approximate.bin")}
        assert max(errors.values()) <= 1e-12, (name, errors)
        assert (reference / "result_farefield_mlfma_0.txt").read_bytes() == (
            directory / "result_farefield_mlfma_0.txt").read_bytes(), name
        if args.baseline_dir:
            assert (args.baseline_dir / "near_matrix.bin").read_bytes() == (directory / "near_matrix.bin").read_bytes()
            for filename in errors:
                assert relative_error(args.baseline_dir / filename, directory / filename) <= 1e-12
            assert (args.baseline_dir / "result_farefield_mlfma_0.txt").read_bytes() == (
                directory / "result_farefield_mlfma_0.txt").read_bytes()
        checks = " | ".join(s for s in (directory / "run.log").read_text().splitlines() if s.startswith("CHECK"))
        print(name, checks, "max_relative_error=", max(errors.values()),
              (directory / "resources.txt").read_text().strip(), flush=True)
    print("All checks passed. Artifacts:", output)


if __name__ == "__main__":
    main()
