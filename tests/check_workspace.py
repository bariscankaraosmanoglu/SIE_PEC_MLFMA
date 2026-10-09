#!/usr/bin/env python3
"""Bounded ownership and workspace checks on the existing 24-RWG CFIE fixture."""
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
    parser.add_argument("--input", type=Path, required=True, help="CFIE reference/input.pnu")
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--skip-leaks", action="store_true", help="Run leak check separately when ptrace prevents LSan")
    parser.add_argument("--baseline", action="store_true", help="Record known old cleanup/leak failures")
    args = parser.parse_args()
    output = (args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-workspace-"))).resolve()
    output.mkdir(parents=True, exist_ok=True)
    source = args.source_dir.resolve()
    flags = ["-std=gnu++17", "-fopenmp", "-I" + str(source)]
    flags += (["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
              if args.sanitize else ["-O3"])
    command = ["g++", *flags, str(ROOT / "tests/workspace_operators.cpp"),
               *map(str, sorted(p for p in source.glob("*.cpp") if p.name != "main.cpp")),
               "-o", str(output / "workspace_operators")]
    (output / "build-command.json").write_text(json.dumps(command, indent=2) + "\n")
    with (output / "build.log").open("w") as log:
        subprocess.run(command, stdout=log, stderr=log, check=True)
    env = dict(os.environ, OMP_DYNAMIC="FALSE", ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    env.pop("OMP_THREAD_LIMIT", None)
    for name, arguments in [("cleanup", ["cleanup"]), ("repeat", ["repeat", "8", "100"])]:
        d = output / name
        d.mkdir(exist_ok=True)
        (d / "input.pnu").write_text(args.input.read_text())
        with (d / "run.log").open("w") as log:
            subprocess.run(["/usr/bin/time", "-f", "elapsed=%e peak_rss_kib=%M", "-o", "resources.txt",
                            str(output / "workspace_operators"), *arguments], cwd=d, env=env,
                           stdout=log, stderr=log, check=True)
        print(name, "\n".join(s for s in (d / "run.log").read_text().splitlines() if s.startswith("CHECK")), flush=True)
    if args.sanitize and not args.skip_leaks:
        d = output / "leaks"
        d.mkdir(exist_ok=True)
        (d / "input.pnu").write_text(args.input.read_text())
        with (d / "run.log").open("w") as log:
            result = subprocess.run([str(output / "workspace_operators"), "repeat", "2", "4"], cwd=d,
                                    env=dict(env, ASAN_OPTIONS="detect_leaks=1"), stdout=log, stderr=log)
        if args.baseline:
            assert result.returncode != 0 and "LeakSanitizer: detected memory leaks" in (d / "run.log").read_text()
            print("baseline leaks confirmed", flush=True)
        else:
            assert result.returncode == 0, d / "run.log"
            print("leak check passed", flush=True)
    print("Artifacts:", output)


if __name__ == "__main__":
    main()
