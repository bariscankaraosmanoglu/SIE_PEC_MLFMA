#!/usr/bin/env python3
"""Pivoted block-LU and mode gating regressions. Retain every artifact."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / "src")
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--baseline", action="store_true")
    parser.add_argument("--sanitize", action="store_true")
    args = parser.parse_args()
    source = args.source_dir.resolve()
    output = (args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-blocks-"))).resolve()
    output.mkdir(parents=True, exist_ok=True)
    flags = ["-std=gnu++17", "-fopenmp", "-I" + str(source)]
    flags += ["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"] if args.sanitize else ["-O3"]
    lu = [source / "matrixBlock.cpp"] if (source / "matrixBlock.cpp").exists() else []
    workspace = [source / name for name in ("mlfma_workspace.cpp", "sample_locations_weights.cpp",
                 "interpolationweights.cpp", "input_validation.cpp")] if (source / "mlfma_workspace.cpp").exists() else []
    builds = {
        "block_factorization": [ROOT / "tests/block_factorization.cpp", *lu],
        "block_preconditioner": [ROOT / "tests/block_preconditioner.cpp", source / "calculate_blockdiagonalpreconditioner.cpp",
                                 source / "mvm_BDPmatrix.cpp", source / "clear_memory.cpp", *lu, *workspace],
    }
    commands = {}
    for name, files in builds.items():
        command = ["g++", *flags, *map(str, files), "-o", str(output / name)]
        commands[name] = command
        with (output / (name + "-build.log")).open("w") as log:
            subprocess.run(command, stdout=log, stderr=log, check=True)
    (output / "build-commands.json").write_text(json.dumps(commands, indent=2) + "\n")
    env = dict(os.environ, OMP_DYNAMIC="FALSE", ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    results = {}
    cases = [("block_factorization", case, 0) for case in
             ("first-pivot", "later-pivot", "complex", "scaled-small", "scaled-large", "near-singular",
              "singular", "nonfinite", "empty", "repeated")]
    cases += [("block_preconditioner", case, 0) for case in ("mode0", "mode2", "well-conditioned", "later-pivot")]
    if not args.baseline:
        cases += [("block_preconditioner", case, 1) for case in
                  ("singular-block", "nonfinite-block", "missing-self", "duplicate-index")]
    for binary, case, expected in cases:
        directory = output / (binary + "-" + case)
        directory.mkdir(exist_ok=True)
        with (directory / "run.log").open("w") as log:
            result = subprocess.run(["/usr/bin/time", "-f", "elapsed=%e peak_rss_kib=%M", "-o", "resources.txt",
                                     str(output / binary), case], cwd=directory, env=env, stdout=log, stderr=log, timeout=10)
        text = (directory / "run.log").read_text()
        results[directory.name] = result.returncode
        if not args.baseline:
            assert result.returncode == expected, (directory.name, result.returncode, directory / "run.log")
            if expected:
                assert "Block preconditioner error" in text, (case, text)
            else:
                assert "CHECK" in text, (case, text)
        print(directory.name, result.returncode, flush=True)
    (output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    print("Artifacts:", output)


if __name__ == "__main__":
    main()
