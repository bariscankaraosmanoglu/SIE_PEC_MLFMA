# Handoff

Rank-4 input and dimension validation is implemented on `codex/fix_rank4`,
starting from `42b3a59`. The working tree had no user edits when this task began.
The shared input format and mode identifiers are preserved. This PEC executable
explicitly rejects unsupported formulations, illuminations, and frequency sweeps.
No full radiation-pattern caching was introduced.

This handoff accompanies the rank-4 source, regression tests, and documentation
commit requested by the user on `codex/fix_rank4`. No push is authorized.
`run_mlfma` is an untracked final build artifact; retain it outside the source
commit. `CHANGELOG_DEV.md` records the logical work item;
`tests/README_INPUTS.md` explains checks, numerical scope, commands, and measurements.

## Completed checks

- Production build passed using the root `AGENTS.md` commands after archiving
  existing executables. Existing warnings remain.
- Final input/dimension regression suite: 99 cases passed, ordinary and
  ASan/UBSan/float-cast-overflow builds.
- Existing CFIE and thread suites passed ordinary and sanitizer checks. Thread
  configurations through 32 requested workers matched the archived serial
  operator baseline. Topology LeakSanitizer passed outside sandbox ptrace after
  its initial environment failure.
- Final full Krylov reference suite passed modes 0/1/2, multiple illuminations,
  iteration-limit reporting, and early invalid-inner-input rejection.
- Closed/open full solver outputs and safe zero-order approximation cases matched
  baseline byte for byte. All three local meshes had byte-identical parsed arrays.
- Before/after measurements: solver memory approximately unchanged on small
  references; strict SOM parsing increased from 0.11 s / 8,872 KiB to
  0.30 s / 20,448 KiB (three-trial medians). No speed improvement is claimed.
- User-requested full `test_23_sphere_0p01m.pnu` pair passed: 37,587 RWGs,
  3 GHz, original 181 illuminations, eight requested workers. Both exited 0;
  all five numerical result files and all solver statuses were byte-identical.
  Maximum physical residual `9.33977e-4` satisfies the unchanged `1e-3` tolerance;
  both reported 727 outer iterations and 1,089 Krylov matrix products. Full wall
  time before/after: 1,099.11 / 1,128.97 s; peak RSS: 352,796 / 353,564 KiB.
  The observed +2.72% runtime is almost entirely in the solution stage; one pair
  does not establish its cause. Peak RSS increased by only 0.75 MiB (+0.22%).

## Commands and artifacts

Build with `rm run_mlfma`, then
`g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma`.
Archive the executable first: the user requests that artifacts be kept.
Focused tests: `python3 -B tests/check_inputs.py`, plus `--sanitize`.
Existing suites: `tests/check_cfie.py`, `tests/check_thread_parallel.py`, and
`tests/check_krylov_reference.py`; see their READMEs for arguments.
Honor `AGENTS.md` test authorization requirements for any expanded plan.

All implementation artifacts are retained at
`/tmp/mlfma-rank4/implementation-xjqpan3a`:

- `before/`: untouched source/tests, original executable, baseline cases.
- `inputs-final-v2/`, `inputs-final-sanitize-v2/`: final 99-case results.
- `cfie/`, `cfie-sanitize-reviewed/`, `threads/`, `threads-sanitize/`: regressions.
- `reference-final-v2/`, `final/`, `zero-order-full/`, `final-comparisons.json`:
  final executable checks and numerical comparisons.
- `large-inputs/`, `benchmarks/`: parsed-array comparisons and measured costs.
- `final-v2-build.log`, `run_mlfma`: final build log/executable; earlier builds and
  failed/intermediate checks are retained separately.

The additional sphere23 pair is retained at
`/tmp/mlfma-rank4/sphere23-before-after-8mm2uv5c`, with the unchanged input and
executable copies, `run_comparison.py`, `compare_results.py`, `comparison.json`,
`metadata.json`, full per-stage logs/results/resources, and `REPORT.md`.
The source and executable matched the validated manifest before running; no
rebuild or parser optimization was introduced for this comparison.

## Next steps and limits

Rank-4 is recorded in the commit containing this handoff. No push is authorized.
No validation required for this focused change is pending. A full SOM solve,
orientation/self-intersection audit, whole-program leak audit, and throughput
measurement on hardware with more cores were not run and are outside this scope.
The temporary duplicate-triangle set adds parser memory and is freed before
assembly. Integer-capacity guards do not guarantee that a problem fits RAM.
Unrelated compiler warnings and other repositories' IBC/source modes remain.

If investigating the sphere23 runtime difference, note that the new stencil
checks in `interpolationweights` execute inside both MVM paths. Their contribution
is unprofiled; unchanged iteration counts do not isolate per-product overhead.
Obtain approval for a targeted profiling/repetition plan before further runs.
