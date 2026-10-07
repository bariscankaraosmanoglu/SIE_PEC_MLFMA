# CFIE coefficients and open surfaces

Rank-2 fixes the far operators using the first material coefficient instead of
the testing RWG's coefficient. Both accurate and approximate receiving stages
now use `rwg_media[testfunction]`, matching near assembly and the RHS.

Triangles are connected through shared mesh edges. A component containing any
boundary edge uses EFIE (`alpha = 1`) for every RWG, including interior RWGs.
Closed components retain the average of their supporting triangles' material
coefficients. Components are classified before RWG construction; edges shared
by more than two triangles and disconnected triangle fans at a vertex are
rejected before the old connectivity builder can corrupt memory. Connectivity
uses node indices; this change does not weld coincident vertices or repair mesh
winding. The classifier sorts three edges per triangle, takes O(T log T) time
and O(T + N) temporary memory, and retains one byte per triangle.

`solver_globals.h` supplies compatible declarations alongside the definitions
in `main.h`. It replaces scalar declarations of `val_cfiealpha[50]`, the pointer
declaration of `source_PW[4]`, and cleanup's `int*` declarations of the `long int*`
near and aggregation indices. Unused pointer declarations of the model-name
arrays were removed. Full radiation-pattern caching is outside this change.

## Running checks

Build according to root `AGENTS.md`, then run from the repository root:

```sh
rm run_mlfma
g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma
python3 -B tests/check_cfie.py --full-solver ./run_mlfma
python3 -B tests/check_cfie.py --sanitize
python3 -B tests/check_thread_parallel.py --input src/test_29_sphere_0p03m.pnu
python3 -B tests/check_thread_parallel.py --sanitize --input src/test_29_sphere_0p03m.pnu
```

The CFIE runner generates its small meshes itself. The thread runner's sphere
is an existing local, untracked input; it can instead accept another input via
`--input`. Binaries, inputs, logs, vectors and fields are written under `/tmp`.
Specify `--output-dir` to retain a known directory. Both runners accept
`--skip-build` for repeated runs of an unchanged binary; omit it after editing
the code or changing sanitizer flags.

The CFIE checks cover:

- A closed sphere with uniform coefficients and with two different material
  coefficients, including RWGs crossing the material interface.
- Accurate and approximate far products, near products and RHS vectors compared
  with independent row combinations of uniform electric/magnetic endpoint
  responses, using relative L2 tolerance `1e-12`.
- Changing one testing coefficient while leaving material coefficients fixed.
- Open-grid EFIE invariance under changing the configured material coefficients.
- Propagation of EFIE to 16 interior RWGs whose supporting triangles have no
  boundary edges, and preservation of coefficients on a separate closed part.
- Empty, closed, open and mixed topology; triangle permutation; nonmanifold
  edges/vertices, invalid node indices, and repeated triangle nodes.
- Independently recomputed solution residuals and a full direct MoM reference
  for two separated small closed surfaces with nonzero far interactions.
- With `--full-solver`, the actual executable's uniform/open GMRES solves and
  its mixed reference GMRES/FGMRES solves. Saved coefficients are read back and
  checked independently against the operator and, on the reference, direct MoM.

Direct MoM uses the existing singular-integration kernel to assemble all pairs
in one artificial leaf, independently of MLFMA interpolation and translation.
Coarse-sphere and open-grid direct comparisons are diagnostic only: they expose
approximation discrepancies beyond this fix. The separated-surface reference
asserts both direct product error and direct residual against the unchanged
input tolerance `0.001`; it uses `num_digits = 6` and `num_interpol = 4`.
The coarse fixtures use `num_digits = 3` and `num_interpol = 2`.

Sanitizer checks use AddressSanitizer/UBSan. Existing solver leaks are excluded
in the operator driver, but isolated topology checks enable LeakSanitizer.
LeakSanitizer cannot run under ptrace supervision, so that check needed an
execution outside the workspace sandbox. It passed. This is not a claim that
the whole solver is leak-free.

## Baseline and validation results

Unmodified sources, executable, outputs, and logs were saved under
`/tmp/mlfma-rank2/baseline` and `/tmp/mlfma-rank2/before` before production edits.
Baseline reproduction used:

```sh
python3 -B tests/check_cfie.py --source-dir /tmp/mlfma-rank2/baseline/src --baseline --output-dir /tmp/mlfma-rank2/before --full-solver /tmp/mlfma-rank2/baseline/run_mlfma
python3 -B tests/check_cfie.py --output-dir /tmp/mlfma-rank2/after --baseline-dir /tmp/mlfma-rank2/before --full-solver ./run_mlfma
python3 -B tests/check_cfie.py --sanitize --output-dir /tmp/mlfma-rank2/sanitized
```

| Check | Before | After |
| --- | --- | --- |
| Mixed accurate far row-blend error | 0.881814 | 1.59e-14 |
| Mixed approximate far row-blend error | 0.991145 | 1.55e-15 |
| Incorrect open interior coefficients | 16 | 0 |
| Separated-surface direct product error | 0.003344 | 0.000169 |
| Separated-surface GMRES direct residual | 0.002514 | 0.000258 |

Near and RHS row-blend errors remain below `4e-16`. Corrected mixed one- and
eight-worker results agree within `1e-12`. Uniform operator vectors, solutions,
and fields agree with baseline within `1e-12`; the full executable's four
uniform-case result files are byte-identical. The actual corrected FGMRES
reference has direct residual `0.000235`, below `0.001`. The open full solve's
recomputed MLFMA residual is below `0.001`.

Normal and sanitized thread suites passed all eight team configurations through
32 requested workers, including OpenMP off, a two-worker limit and dynamic
teams. Their vectors matched the saved rank-1 uniform reference exactly.
Artifacts are under `/tmp/mlfma-rank2/threads` and `thread-sanitized`.

Three sequential before/after runs were measured by
`python3 -B /tmp/mlfma-rank2/benchmark.py`, using
`/usr/bin/time -f '%e %M'`, `OMP_DYNAMIC=FALSE`, and no thread limit:

| Full solver case | Median seconds before / after | Median peak RSS KiB before / after |
| --- | --- | --- |
| Existing local uniform sphere, 8 workers | 1.55 / 1.51 | 33,352 / 33,352 |
| Mixed separated-surface FGMRES, 1 worker | 0.18 / 0.18 | 10,716 / 10,708 |

All four local uniform-sphere result files were byte-identical across these
runs. These timings do not establish a speedup; affected solves can have
different iteration counts because their operators change. Existing compiler
warnings remain; the new topology source introduces none.

The coarse uniform sphere differs from direct MoM by about 15% both before and
after. The corrected coarse open grid differs by about 2.75%. These diagnostics
limit any claim about overall solver accuracy; the CFIE row-weighting checks and
small direct reference pass, but this change does not resolve those broader
approximation discrepancies.

Existing source CRLF line endings were preserved. Check whitespace using:

```sh
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

## Changed files

- `src/mvm_Zfardotx.cpp`, `src/mvm_Zfardotx_in.cpp`: testing-row coefficients.
- `src/surface_components.cpp`, `src/surface_components.h`,
  `src/generate_edgeconnection.cpp`, `src/calculate_Znearmatrix.cpp`: topology
  validation, component classification, and open-surface EFIE enforcement.
- `src/solver_globals.h`, `src/main.h`, `src/calculate_Znearmatrix.h`,
  `src/calculate_rhsvector.cpp`, `src/calculate_farfield_mlfma.cpp`,
  `src/read_simulationinput.cpp`, `src/stage_solution.cpp`,
  `src/clear_memory.cpp`: compatible shared declarations and obsolete declarations
  removed. The far operators also consume this header.
- `tests/cfie_operators.cpp`, `tests/check_cfie.py`,
  `tests/surface_components.cpp`: reproducible coefficient/topology/solve checks.
- `tests/check_thread_parallel.py`: reuse unchanged test binaries for baseline
  reruns; `tests/README.md`, `tests/README_CFIE.md`: validation documentation.
