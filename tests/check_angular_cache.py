#!/usr/bin/env python3
"""Bounded angular/stencil/child-phase cache checks; retains all artifacts."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--skip-leaks", action="store_true", help="For ptrace environments; run LSan separately")
    args = parser.parse_args()
    output = (args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-angular-cache-"))).resolve()
    output.mkdir(parents=True, exist_ok=True)
    source = ROOT / "src"
    flags = ["-std=gnu++17", "-fopenmp", "-I" + str(source)]
    flags += (["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
              if args.sanitize else ["-O3"])
    command = ["g++", *flags, str(ROOT / "tests/angular_cache.cpp"),
               *[str(source / name) for name in ("mlfma_workspace.cpp", "sample_locations_weights.cpp",
                  "interpolationweights.cpp", "input_validation.cpp")], "-o", str(output / "angular_cache")]
    (output / "build-command.json").write_text(json.dumps(command, indent=2) + "\n")
    with (output / "build.log").open("w") as log:
        subprocess.run(command, stdout=log, stderr=log, check=True)
    env = dict(os.environ, OMP_DYNAMIC="FALSE", ASAN_OPTIONS="detect_leaks=" + str(int(not args.skip_leaks)),
               UBSAN_OPTIONS="halt_on_error=1")
    env.pop("OMP_THREAD_LIMIT", None)
    with (output / "run.log").open("w") as log:
        subprocess.run([str(output / "angular_cache")], cwd=output, env=env,
                       stdout=log, stderr=log, check=True)
    print((output / "run.log").read_text(), end="")
    print("Artifacts:", output)


if __name__ == "__main__":
    main()
