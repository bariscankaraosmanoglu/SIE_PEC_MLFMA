# Ownership and MLFMA workspace regression

Rank 6 repairs ownership and reuses MLFMA sampling/interpolation metadata and
worker scratch. It does not change Krylov storage or cache RWG radiation
patterns. Quadrature, truncation, precision, solver tolerances, and arithmetic
order are preserved.

`mlfma_workspace()` owns grids keyed by theta/phi counts and sampling convention,
and interpolation tables keyed by both grid shapes, interpolation order, and
the final theta sampling convention. The uniform 181 by 360 output grid has a
separate key from a Gauss grid of the same size. Tables are prepared before
entering parallel regions and shared read-only. Each actual OpenMP worker has
independent field and interpolation scratch. Pool growth uses a `single` region
and its implicit barrier; requested teams, serialized teams, dynamic adjustment,
and runtime thread limits do not determine array indexing.

Buffers retain their high-water capacities during one simulation, including
accurate/approximate operator alternation and accelerated field output. Output
formatting arrays have local vector owners and are released after writing.
`generate_octtree()` and `clear_memory()` clear retained workspace storage.
Global simulation-array cleanup nulls each released owner and can be called
twice or after partial initialization. Direct field output and topology cleanup
null their borrowed global pointers when releasing their own arrays. Source
counts use fixed static storage. This does not make global solver operations
reentrant: overlapping operator calls or independent simulations in concurrent
threads remain unsupported by the existing global aggregation state.

Legacy interpolation signatures remain available with local scratch ownership.
MLFMA paths use overloads that receive reusable scratch. Fine-to-coarse scratch
is explicitly reset as before; caller output accumulation is unchanged.

Run only with the test authorization required by root `AGENTS.md`. First produce
the existing small CFIE fixture, then run:

```sh
python3 -B tests/check_cfie.py --output-dir /tmp/chosen-cfie
python3 -B tests/check_workspace.py \
  --input /tmp/chosen-cfie/reference/input.pnu --output-dir /tmp/chosen-workspace
python3 -B tests/check_workspace.py --sanitize \
  --input /tmp/chosen-cfie/reference/input.pnu --output-dir /tmp/chosen-workspace-san
```

Choose fresh output directories; these scripts retain artifacts and do not
delete previous outputs. `--source-dir` accepts archived sources. `--baseline`
records old cleanup omissions and expects the old implementation's leak report
when sanitizers are enabled; it does not weaken numerical tolerances.

The reference geometry has two closed octahedra, 12 nodes, 16 triangles, and 24
RWGs at 1 GHz. Its existing settings are six digits, interpolation order four,
approximation 0.8, and tolerance 0.001; automatic tree depth is unchanged. The
driver checks:

- Partial initialization and repeated cleanup of newly covered owners.
- Alternating deterministic vectors in accurate and approximate far products.
- Warmup at requested teams 1, 2, 8, and 32, then 100 product pairs at eight
  workers. Relative L2 error must stay at or below the existing operator
  regression threshold of `1e-12`.
- Six accelerated far-field outputs, compared byte for byte for repeated inputs.
- Stable retained capacity/table counts after warmup, release at cleanup, and
  identical metadata after rebuilding.
- ASan/UBSan with leak checks disabled for the long repetition, followed by a
  shorter four-pair run with LeakSanitizer enabled.

LeakSanitizer cannot inspect threads under some ptrace-based sandboxes. Its
fatal ptrace diagnostic is an environment failure, not a passing leak check.
In that environment, use `--skip-leaks` for address/UB checks and separately
run the already built driver with the needed sandbox permission:

```sh
env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 OMP_DYNAMIC=FALSE \
  /tmp/chosen-workspace-san/workspace_operators repeat 2 4
```

Run that command in a retained output directory containing the same input.pnu.
It requests two workers for the repetition and warms the same teams up to 32.

`retained_bytes()` measures owned vector capacities and table payloads for
regression checks. It excludes some allocator/map overhead and is not RSS.
RSS may remain high after freeing arrays because allocators can retain pages.
Retaining scratch can increase memory relative to a leak-free implementation
that releases every buffer immediately. The expected benefit is bounded storage
and fewer allocations, not an unconditional reduction of peak RSS.

The existing thread, CFIE, and 24-RWG Krylov reference suites provide broader
numerical and runtime-team coverage. Small cases on eight logical CPUs cannot
establish throughput scaling beyond 16 physical cores.

## Recorded validation

Rank-6 artifacts are retained at `/tmp/mlfma-rank6-26OJl7kI`, with untouched
sources under `before/src`, exact executed commands in `COMMANDS.md`, and source,
test, and executable hashes in `validated-manifest.json`. Starting HEAD was
`9d2a62412f80663d049f5f0488471e5069efba73` on `codex/fix_rank6`.

The old partial cleanup left owners populated. Its short leak exercise reported
24,312,560 bytes leaked in 1,966 allocations, including both far operators,
accelerated output, and approximate translation setup. The updated partial and
repeated cleanup checks pass; the same leak exercise reports no leaks. Initial
LSan attempts failed under sandbox ptrace supervision; matching checks were
rerun outside it, retaining both failed attempts and successful reruns.

The normal and ASan/UBSan workspace, thread, and CFIE checks pass. Both old/new
ten-case Krylov reference suites pass, including modes 0/1/2, repeated
illuminations, iteration limits, and independent physical residual checks.
All 424 saved normal-build numerical outputs are byte-identical before/after.
Repeated products have zero relative difference, and output files repeat
identically. Workspace payload/capacity stays at 8,045,800 bytes after warmup
with 32 actual workers, seven grids, and five interpolation tables, and reaches
zero after cleanup. This count is specific to the reference case and does not
include allocator overhead. Input/block subset drivers compile and link with
the new workspace dependencies; those extra suites were not executed.

Three sequential interleaved trials used GCC 11.4.0 with `-O3 -std=gnu++17
-fopenmp`, no relaxed floating-point options, and an Intel i7-7700HQ (four
physical cores, eight logical CPUs). `OMP_DYNAMIC=FALSE`, no thread limit;
warmup requested 1/2/8/32 workers, followed by 100 accurate/approximate product
pairs at eight workers. The same 24-RWG input and accuracy settings were used
on both sides. Build/sanitizer checks had finished before timing started.

| Median over three trials | Before | After |
| --- | ---: | ---: |
| Total wall time, seconds | 1.20 | 1.13 |
| Repeated section, seconds | 0.888264 | 0.827768 |
| Peak RSS, KiB | 58,804 | 38,128 |

Total time includes setup, assembly, warmup, 100 product pairs, six accelerated
field outputs with file I/O, comparisons, and cleanup. The repeated section
includes 100 pairs and four field outputs with I/O/comparisons. These are
operator-driver timings, not complete Krylov solves. The measured memory
decrease is 20.19 MiB (35.2%); the small timing difference is insufficient to
predict performance on production cases. That bounded validation did not
measure the larger sphere/SOM or throughput on hardware with more than 16 cores.

### Larger sphere follow-up

A separately requested single full before/after pair on
`src/test_23_sphere_0p01m.pnu` used the user's current 31-illumination sweep,
3 GHz, 37,587 RWGs, mode 2, six levels, digits 2, approximation 0.3,
interpolation 2, inner/outer limits 10/100, tolerance 0.001, and eight workers.
Compiler/hardware unchanged; OMP_DYNAMIC=FALSE and no thread limit.

| Full-run measurement | Before | After |
| --- | ---: | ---: |
| Wall time, seconds | 238.24 | 234.44 |
| Solution-stage time, seconds | 237.126272 | 233.295821 |
| Peak RSS, KiB | 301,120 | 287,540 |

Both runs exit 0. All 31 illuminations converge, with unchanged 181 outer
iterations and 243 Krylov matrix products. Maximum physical residual is
`9.93188384084087e-4`, below 0.001. All five numerical result files, including
currents, direct monostatic fields and solver statuses, are byte-identical.

Observed wall time decreases 1.60%; peak RSS decreases 13.26 MiB (4.51%).
Sampled high-water memory grows during the baseline sweep and stays nearly
flat after rank-6, consistent with removing repeated allocation leaks. These
samples span differing solve phases and are not a leak-rate estimate. One
pair cannot establish a reliable speedup. Full time includes input/setup,
assembly, solves, direct field output, current diagnostics and cleanup.
This input uses dir_ff=0, so it does not exercise accelerated field output.
The original 181-angle sweep, SOM, and high-core throughput were not rerun.

Artifacts and exact commands are retained in
`/tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b`; `metadata.json` records
settings/input/binary hashes, and `summary.json` records comparisons and
measurements. The user's changed input remains untouched.

### Larger sphere with mode 1

The user separately requested repeating the 31-illumination sphere comparison
with Preconditioners 1; all other input settings, compilers and hardware were
unchanged. One sequential pair used verified archived/current executables.

| Full-run measurement | Before rank-6 | After rank-6 |
| --- | ---: | ---: |
| Wall time, seconds | 339.18 | 333.36 |
| Solution-stage time, seconds | 338.016582 | 332.210418 |
| Peak RSS, KiB | 235,016 | 226,780 |

Observed wall time decreases 1.72%; peak RSS decreases 8.04 MiB (3.50%). Both
build 4,312 pivoted LU blocks with 417,495 complex coefficients. All five
result files are byte-identical, with unchanged 648 iterations and 710 Krylov
matrix products. Numerical identity passes, but physical convergence fails:
all 31 illuminations are flagged physical_residual_not_converged, and both
processes exit 1. Physical residuals range from 1.36948e-3 to 1.72394e-3,
above the requested 0.001, although preconditioned GMRES declares convergence.

Mode 1's left-preconditioned residual criterion and the physical equation's
residual criterion differ. stage_solution flags that discrepancy after solving;
this behavior exists in both versions and was left unchanged in this test.
No tolerance, iteration bound, stopping criterion or implementation was altered.

Against the previous current mode-2 measurement, current mode 1 uses 21.13%
less peak RSS and takes 42.19% longer. Mode 2 converged physically for all
31 illuminations; mode 1 did so for none. This is not a comparison at equal
achieved accuracy. One pair does not establish a reliable rank-6 speedup.

Artifacts and exact commands are retained at
`/tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1`, including summary.json,
metadata.json, inputs, binaries, scripts, resource files, statuses, outputs,
run logs and approximate RSS samples. Both user input edits remain untouched.
