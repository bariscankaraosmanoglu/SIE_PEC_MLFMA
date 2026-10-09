# Pivoted block preconditioning

Mode 1 now stores each leaf self block as contiguous complex LU factors, using
partial row pivoting at every elimination column. Application solves
`L*U*x = P*b`; there is no transpose, conjugation, or symmetry assumption.
`mvm_BDPmatrix` preserves the existing left-preconditioned operator and RHS.
The original near matrix remains unchanged. Modes 0 and 2 do not construct or
retain these factors; mode 2 continues to use the approximate-operator inner
GMRES.

The builder checks self-neighbor presence, matrix offsets, RWG ranges, and
exactly-once RWG coverage. Missing, singular, or nonfinite blocks stop setup
with a diagnostic identifying the leaf box. There is no regularization or
fallback to another preconditioner. A zero pivot is rejected without an
absolute epsilon cutoff, so uniformly small valid blocks are accepted.
Partial pivoting does not provide a condition-number estimate; very poorly
conditioned finite blocks remain a numerical limitation.

Blocks own their factors, maps, and pivots through vectors. `clear_memory`
releases the block array. Application allocates a per-call workspace sized to
the largest block, rather than all RWGs; gather/solve/scatter supports `x == y`
because blocks cover disjoint RWGs. Factors are read-only during application.
Arithmetic overflow during application propagates to the existing Krylov
finite-value and termination checks.

## Commands

Execution requires approval under the root `AGENTS.md`. From the repository root:

```sh
python3 -B tests/check_blocks.py
python3 -B tests/check_blocks.py --sanitize
```

Both drivers are built and run by this script. All executables, commands, logs,
and resources are retained in the printed temporary directory; specify
`--output-dir /tmp/chosen-directory` to choose a location. To probe archived
pre-fix sources, add `--source-dir /path/to/before/src --baseline`. The baseline
option records failures without requiring the new behavior. Sanitizers check
addresses and undefined behavior; leak detection is disabled because unrelated
whole-solver leaks remain. No whole-program leak audit is claimed.

The factorization driver uses separately multiplied manufactured solutions,
including first and later pivots, general complex matrices, scaling by `1e-150`
and `1e150`, near singularity, singular/nonfinite rejection, safe empty state,
and reinitialization. Well-conditioned forward and backward relative errors
must be at most `1e-12`; the near-singular case checks backward error only.
The integration driver calls the production builder, application, and cleanup,
checking permuted RWG maps, in-place application, unchanged physical storage,
mode gating, and rejection of malformed blocks.

For the existing full-solver reference checks:

```sh
python3 -B tests/check_krylov_reference.py --solver /path/to/run_mlfma \
  --operator-driver /path/to/cfie_operators \
  --reference-input /path/to/reference/input.pnu --output-dir /tmp/reference-checks
```

The independent operator driver and input are produced by the existing CFIE
suite. A retained rank-4 driver/input was reused for rank-5; it computes the
physical residual independently of this preconditioner. See `README_KRYLOV.md`
for the existing suite, including termination checks.

## Rank-5 validation

User-approved artifacts are retained at
`/tmp/mlfma-rank5/implementation-xwtafvea`. Untouched sources, tests, executable,
build log, and baseline checks are under `before/`. Final checks are under
`blocks/`, `blocks-sanitize/`, and `reference/`. `reference-comparison.json`
records numerical comparisons; `benchmark.py`, `benchmarks/summary.json`, and
the per-trial directories retain the measurement commands and results.

The baseline produced nonfinite solutions for a nonsingular 3-by-3 matrix
requiring a second-column row swap. The LU solve reduced forward error to
`3.21e-16` with zero measured backward error. Baseline production integration
also confirmed unused factors in modes 0/2 and missing block cleanup. All 18
new checks passed normally and with ASan/UBSan, including four expected setup
rejections. The near-singular fixture has forward error about `1.07e-6` but zero
measured backward error; pivoting cannot remove sensitivity to roundoff.

The unchanged 10-case Krylov reference suite passed before and after: all three
modes, one/two illuminations, iteration limits, limited inner iterations, and
invalid inner input. Geometry is two separated octahedra, 12 nodes, 16 triangles,
24 RWGs, at 1 GHz with CFIE coefficients 0.25/0.75. Settings are six digits,
interpolation order 4, approximation factor 0.8, tolerance `0.001`, outer limit
192, inner limit 15, and one worker, except the suite's explicit limit/input
cases. Mode 1's physical residual changed from `9.547877159307686e-4` to
`9.547877159306173e-4`; both meet the unchanged tolerance. Iterations and
products remained 18 and 20. Printed RWG coefficients and triangle currents
are identical; complex far-field relative L2 change is at most `5.21e-14`.
Modes 0/2 numerical result files and termination reports are byte-identical.

Three sequential interleaved before/after repetitions per mode measured the
full executable, including setup, solve, and field output, for one illumination.
Compiler: GCC 11.4.0 with `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`.
Hardware: Intel i7-7700HQ, four physical/eight logical cores; one worker with
`OMP_DYNAMIC=FALSE`. Wall time and peak RSS use `/usr/bin/time`.

| Mode | Median seconds before / after | Median peak RSS KiB before / after | Outer iterations before / after |
| --- | --- | --- | --- |
| 0 | 0.16 / 0.17 | 10,660 / 10,528 | 8 / 8 |
| 1 | 0.20 / 0.19 | 10,552 / 10,760 | 18 / 18 |
| 2 | 0.18 / 0.18 | 10,688 / 10,748 | 3 / 3 |

These subsecond measurements do not establish a runtime or peak-RSS improvement.
The small reference contains only eight self blocks with 88 complex coefficients
(1,408 bytes of coefficients), so skipping their storage is below RSS noise.
Expected benefits on larger cases are removal of unused setup/storage in modes
0/2, removal of inverse construction and its extra temporary matrix in mode 1,
and smaller application workspace. LU application and inverse multiplication
both have quadratic cost per block; no solve-speed improvement is guaranteed.
The initial plan excluded large sphere/SOM cases, a whole-program leak audit,
and high-core throughput measurements. The subsequent user-requested large
sphere comparison is recorded below.

## Sphere23 follow-up

The user requested a before/after performance check on the current
`src/test_23_sphere_0p01m.pnu`. That file already had a user edit selecting one
illumination at 0/0 degrees rather than the tracked 181-angle sweep. Both runs
used an unchanged copy of the current file: 37,587 RWGs, 3 GHz, mode 2, six
levels, digits 2, interpolation 2, approximation 0.3, tolerance 0.001, outer/inner
limits 100/10, eight workers. No source changes or rebuild were required.

| Check | Before | After |
| --- | --- | --- |
| Full wall seconds | 10.31 | 9.93 |
| Peak RSS KiB | 296,376 | 287,996 |
| Reported solution seconds | 9.048314 | 8.819485 |
| Outer iterations / products | 7 / 9 | 7 / 9 |
| Physical relative residual | 5.891066763345023e-4 | 5.891066763345023e-4 |

Both runs exited zero and all five result/status files were byte-identical.
Observed full runtime fell 3.69% and peak RSS fell 8.18 MiB (2.83%). Skipping
unused mode-2 block storage is consistent with the memory reduction. One
sequential pair cannot separate the runtime saving from measurement variation;
the solution stage also varied despite identical algorithm/results. This case
does not measure large-case mode-1 LU performance or the 181-angle sweep.

Artifacts are retained at `/tmp/mlfma-rank5/sphere23-before-after-MgLKxUum`,
including unchanged inputs, executable copies, `metadata.json`,
`run_comparison.py`, `compare_results.py`, `comparison.json`, `REPORT.md`, and
per-run logs/resources. The runner executes `/usr/bin/time -v -o resources.txt
./run_mlfma` sequentially with `OMP_DYNAMIC=FALSE` and `OMP_THREAD_LIMIT` unset.
The source and executable matched the validated rank-5 hashes before running.

After the user changed PW to one illumination at theta 90 / phi 0 degrees,
another explicitly requested sequential pair used the new input unchanged.
Full wall seconds before/after: `10.40/10.08` (-3.08%); peak RSS KiB:
`296424/287928` (-8,496 KiB = -8.30 MiB, -2.87%). Both converged in seven
outer iterations/nine products, physical residual `5.01456521557973e-4`, with
all five numerical output/status files byte-identical. Solution seconds were
`9.121022/8.921626`. This is a separate pair with a different incident direction,
not a statistical repetition of the preceding input. Artifacts are retained at
`/tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK`; commands/settings otherwise
match the preceding comparison. No solver/input edits or rebuild were needed.
