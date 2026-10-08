#!/usr/bin/env python3
"""Krylov correctness/sanitizer checks. All binaries, logs, and timings are retained."""
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
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--leaks", action="store_true", help="Enable LSan; requires an environment without ptrace")
    parser.add_argument("--skip-build", action="store_true")
    parser.add_argument("--memory-baseline-source", type=Path, help="Also benchmark the saved legacy FGMRES")
    args = parser.parse_args()
    output = (args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-krylov-"))).resolve()
    output.mkdir(parents=True, exist_ok=True)
    sources = args.source_dir.resolve()
    flags = ["-std=gnu++17", "-Wall", "-Wextra", "-I" + str(sources)]
    flags += ["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"] if args.sanitize else ["-O3"]
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=" + str(int(args.leaks)), UBSAN_OPTIONS="halt_on_error=1")
    commands = []

    def build_and_run(name, filenames, source_dir=sources, build_flags=flags):
        binary = output / name
        command = ["g++", *build_flags, str(ROOT / "tests" / (name.split("-legacy")[0] + ".cpp")),
                   *[str(source_dir / file) for file in filenames], "-o", str(binary)]
        commands.append(command)
        if not args.skip_build:
            with (output / (name + "-build.log")).open("w") as log:
                subprocess.run(command, stdout=log, stderr=log, check=True)
        command = ["/usr/bin/time", "-f", "elapsed=%e peak_rss_kib=%M", "-o", str(output / (name + "-resources.txt")), str(binary)]
        commands.append(command)
        (output / "commands.json").write_text(json.dumps(commands, indent=2) + "\n")
        with (output / (name + ".log")).open("w") as log:
            result = subprocess.run(command, cwd=output, stdout=log, stderr=log, env=env, timeout=60)
        assert result.returncode == 0, (name, result.returncode, output / (name + ".log"))
        print(name, (output / (name + "-resources.txt")).read_text().strip(), flush=True)
        print((output / (name + ".log")).read_text().splitlines()[-1], flush=True)

    build_and_run("krylov_solvers", ["gmres.cpp", "fgmres.cpp", "fgmres_in.cpp", "krylov.cpp"])
    build_and_run("krylov_preconditioner", ["gmres.cpp", "fgmres.cpp", "pc_solution.cpp", "krylov.cpp"])
    build_and_run("krylov_stage", ["gmres.cpp", "fgmres.cpp", "stage_solution.cpp", "krylov.cpp"])
    if not args.sanitize:
        build_and_run("krylov_memory", ["fgmres.cpp", "krylov.cpp"])
        if args.memory_baseline_source:
            build_and_run("krylov_memory-legacy", ["fgmres.cpp"], args.memory_baseline_source.resolve(), ["-O3", "-std=gnu++17"])
    print("Artifacts:", output)


if __name__ == "__main__":
    main()
