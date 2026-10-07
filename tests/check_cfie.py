#!/usr/bin/env python3
"""CFIE coefficient, surface topology, and numerical regression checks."""
import argparse
from array import array
import math
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sphere():
    points = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
    faces = [(0, 2, 4), (2, 1, 4), (1, 3, 4), (3, 0, 4),
             (2, 0, 5), (1, 2, 5), (3, 1, 5), (0, 3, 5)]
    for _ in range(2):
        edges, refined = {}, []
        def midpoint(a, b):
            key = tuple(sorted((a, b)))
            if key not in edges:
                v = tuple(points[a][j] + points[b][j] for j in range(3))
                length = math.sqrt(sum(x*x for x in v))
                edges[key] = len(points)
                points.append(tuple(x / length for x in v))
            return edges[key]
        for a, b, c in faces:
            ab, bc, ca = midpoint(a, b), midpoint(b, c), midpoint(c, a)
            refined += [(a, ab, ca), (ab, b, bc), (ca, bc, c), (ab, bc, ca)]
        faces = refined
    points = [tuple(0.3*x for x in p) for p in points]
    return points, [(int(sum(points[v][0] for v in f) < 0), *f) for f in faces]


def grid():
    n = 5
    points = [(2 + 0.1*x, 0.1*y, 0) for y in range(n) for x in range(n)]
    faces = []
    for y in range(n-1):
        for x in range(n-1):
            a = y*n+x
            faces += [(0, a, a+1, a+n+1), (0, a, a+n+1, a+n)]
    return points, faces


def fixture(points, faces, coefficients):
    lines = ["SimulationInputs", "model_name regression", "Formulations 0",
             "Frequency 1000000000 1000000000 1", "source_type 0",
             "source_PW 37 37 1 19 19 1 0", "Preconditioners 2", "num_mlfmalevels 1",
             "dir_ff 1", "num_digits 3", "val_approx 0.8", "num_interpol 2",
             "val_itermax_pc 15", "val_soltol 0.001", "dir_cfc 90 0", "val_itermax 192",
             "parallel_omp 1 1", "## End-Param", f"ModelParts {len(coefficients)}"]
    lines += [f"part{i} {c}" for i, c in enumerate(coefficients)]
    lines += [f"POINTS {len(points)}"] + [" ".join(map(str, p)) for p in points]
    lines += [f"CELLS {len(faces)}"] + [" ".join(map(str, f)) for f in faces]
    return "\n".join(lines) + "\n"


def relative_error(left, right):
    a, b = array("d"), array("d")
    a.frombytes(left.read_bytes()); b.frombytes(right.read_bytes())
    assert len(a) == len(b) and all(math.isfinite(x) for x in (*a, *b))
    norm = sum(x*x for x in a)
    return math.sqrt(sum((x-y)**2 for x, y in zip(a, b)) / (norm or 1))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / "src")
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--sanitize", action="store_true")
    parser.add_argument("--baseline", action="store_true", help="Record known failures before the fix")
    parser.add_argument("--baseline-dir", type=Path)
    parser.add_argument("--skip-build", action="store_true", help="Reuse the binary in --output-dir")
    parser.add_argument("--full-solver", type=Path, help="Also run and verify this solver executable")
    args = parser.parse_args()
    output = (args.output_dir or Path(tempfile.mkdtemp(prefix="mlfma-cfie-"))).resolve()
    output.mkdir(parents=True, exist_ok=True)
    sources = args.source_dir.resolve()
    flags = ["-std=gnu++17", "-fopenmp", "-I" + str(sources)]
    flags += ["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"] if args.sanitize else ["-O3"]
    command = ["g++", *flags, str(ROOT / "tests/cfie_operators.cpp"),
               *map(str, sorted(p for p in sources.glob("*.cpp") if p.name != "main.cpp")),
               "-o", str(output / "cfie_operators")]
    if not args.skip_build:
        with (output / "build.log").open("w") as log:
            subprocess.run(command, stdout=log, stderr=log, check=True)
    env = dict(os.environ, OMP_DYNAMIC="FALSE", ASAN_OPTIONS="detect_leaks=0", UBSAN_OPTIONS="halt_on_error=1")
    env.pop("OMP_THREAD_LIMIT", None)
    if not args.baseline:
        topology_command = ["g++", *flags, str(ROOT / "tests/surface_components.cpp"),
                            str(sources / "surface_components.cpp"), "-o", str(output / "surface_components")]
        if not args.skip_build:
            subprocess.run(topology_command, check=True)
        subprocess.run([str(output / "surface_components")],
                       env=dict(env, ASAN_OPTIONS="detect_leaks=1"), check=True)
    def run(name, text, mode="operators", threads=1, expected_code=0):
        directory = output / name; directory.mkdir(exist_ok=True)
        (directory / "input.pnu").write_text(text)
        with (directory / "run.log").open("w") as log:
            result = subprocess.run(["/usr/bin/time", "-f", "elapsed=%e peak_rss_kib=%M", "-o", "resources.txt",
                                     str(output / "cfie_operators"), mode, str(threads)],
                                    cwd=directory, env=env, stdout=log, stderr=log)
        assert result.returncode == expected_code, (name, result.returncode, directory / "run.log")
        checks = [line for line in (directory / "run.log").read_text().splitlines() if line.startswith("CHECK")]
        print(name, *checks, (directory / "resources.txt").read_text().strip(), sep="\n", flush=True)
        return directory
    points, faces = sphere()
    for name, coeff, threads in [("uniform", [0.5, 0.5], 1), ("mixed", [0.25, 0.75], 1),
                                 ("mixed8", [0.25, 0.75], 8)]:
        directory = run(name, fixture(points, faces, coeff), threads=threads,
                        expected_code=int(args.baseline))
        if args.baseline_dir and name == "uniform":
            for old in (args.baseline_dir / name).glob("*.bin"):
                assert relative_error(old, directory / old.name) <= 1e-12, old.name
            assert (directory / "result_farefield_mlfma_0.txt").read_bytes() == (
                args.baseline_dir / name / "result_farefield_mlfma_0.txt").read_bytes()
        if name == "mixed8":
            for serial in (output / "mixed").glob("*.bin"):
                assert relative_error(serial, directory / serial.name) <= 1e-12, serial.name
    # Two separated small closed surfaces exercise nonzero far interactions
    # while keeping direct MoM quadrature differences below the input tolerance.
    vertices = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
    octahedron = [(0, 2, 4), (2, 1, 4), (1, 3, 4), (3, 0, 4),
                  (2, 0, 5), (1, 2, 5), (3, 1, 5), (0, 3, 5)]
    reference_points = [(center + 0.03*x, 0.03*y, 0.03*z)
                        for center in (-0.3, 0.3) for x, y, z in vertices]
    reference_faces = [(part, *(v + 6*part for v in face))
                       for part in (0, 1) for face in octahedron]
    text = fixture(reference_points, reference_faces, [0.25, 0.75]).replace(
        "num_digits 3", "num_digits 6").replace("num_interpol 2", "num_interpol 4")
    run("reference", text, mode="reference", expected_code=int(args.baseline))
    open_points, open_faces = grid()
    run("open-operators", fixture(open_points, open_faces, [0.25, 0.75]), mode="open",
        expected_code=int(args.baseline))
    if args.full_solver:
        solver = args.full_solver.resolve()
        for name, preconditioner in (("uniform", 0), ("reference", 0), ("reference", 2), ("open-operators", 0)):
            directory = output / f"solver-{name}-pc{preconditioner}"
            directory.mkdir(exist_ok=True)
            solver_input = (output / name / "input.pnu").read_text().replace(
                "Preconditioners 2", f"Preconditioners {preconditioner}").replace(
                "source_PW 37 37 1 19 19 1 0", "source_PW 37 37 1 19 19 1 1")
            (directory / "input.pnu").write_text(solver_input)
            with (directory / "run.log").open("w") as log:
                subprocess.run(["/usr/bin/time", "-f", "elapsed=%e peak_rss_kib=%M", "-o", "resources.txt", str(solver)],
                               cwd=directory, env=env, stdout=log, stderr=log, check=True)
            if args.baseline_dir and name == "uniform":
                baseline_files = list((args.baseline_dir / directory.name).glob("result_*.txt"))
                assert len(baseline_files) == 4, "Missing full-solver baseline files"
                for old in baseline_files:
                    assert old.read_bytes() == (directory / old.name).read_bytes(), old.name
            verification = output / (directory.name + "-check")
            verification.mkdir(exist_ok=True)
            (verification / "input.pnu").write_text((output / name / "input.pnu").read_text())
            with (verification / "run.log").open("w") as log:
                mode = "reference" if name == "reference" else "open" if name == "open-operators" else "operators"
                result = subprocess.run([str(output / "cfie_operators"), mode,
                                         "1", str(directory / "result_rwgcoefficient.txt")],
                                        cwd=verification, env=env, stdout=log, stderr=log)
            assert result.returncode == int(args.baseline), (directory.name, verification / "run.log")
            print(directory.name, (directory / "resources.txt").read_text().strip(), flush=True)
    for name, pts, tris, closed_count in [("open", open_points, open_faces, 0),
        ("components", points + open_points,
         faces + [(m, *(v + len(points) for v in f)) for m, *f in open_faces], len(faces))]:
        directory = run(name, fixture(pts, tris, [0.25, 0.75]), mode="topology")
        incorrect = interior = 0
        edge_counts = {}
        for _, a, b, c in tris:
            for edge in ((a, b), (b, c), (c, a)):
                key = tuple(sorted(edge)); edge_counts[key] = edge_counts.get(key, 0) + 1
        boundary = {i for i, (_, a, b, c) in enumerate(tris)
                    if any(edge_counts[tuple(sorted(e))] == 1 for e in ((a, b), (b, c), (c, a)))}
        for line in (directory / "coefficients.txt").read_text().splitlines():
            a, b, alpha = line.split(); a, b, alpha = int(a), int(b), float(alpha)
            expected = ([0.25, 0.75][tris[a][0]] + [0.25, 0.75][tris[b][0]]) / 2 if a < closed_count else 1
            incorrect += alpha != expected
            interior += a >= closed_count and a not in boundary and b not in boundary
        assert interior > 0
        assert incorrect > 0 if args.baseline else incorrect == 0, (name, incorrect)
        print(name, "incorrect_coefficients=", incorrect, "interior_open_rwgs=", interior, flush=True)
    if not args.baseline:
        invalid_points = [(0, 0, 0), (1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1)]
        directory = run("nonmanifold", fixture(invalid_points, [(0, 0, 1, 2), (0, 1, 0, 3), (0, 0, 1, 4)], [0.5]),
                        mode="topology", expected_code=1)
        assert "Nonmanifold edge" in (directory / "run.log").read_text()
        directory = run("pinched", fixture([(0, 0, 0), (1, 0, 0), (0, 1, 0), (-1, 0, 0), (0, -1, 0)],
                        [(0, 0, 1, 2), (0, 0, 3, 4)], [0.5]), mode="topology", expected_code=1)
        assert "Nonmanifold vertex" in (directory / "run.log").read_text()
    print("Artifacts:", output)


if __name__ == "__main__":
    main()
