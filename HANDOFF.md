# Handoff

## Current task: rank-5 block preconditioning

Implemented on `codex/fix_rank5`, starting at merge commit
`75c25ed829c2c50aedb431c2d53274016030352a`. Initial working tree was clean;
there were no pre-existing user edits. This handoff accompanies the rank-5
implementation/test/documentation commit requested by the user. No push is
authorized. The user's subsequent sphere-input edits remain outside this commit.
The final executable `run_mlfma` is untracked and must be retained as an artifact.

`matrixBlock` now owns contiguous complex LU factors, row pivots, and RWG maps.
The builder validates self blocks, factors with partial pivoting, and is called
only for mode 1. `mvm_BDPmatrix` uses a per-call largest-block workspace and
supports in-place application. `clear_memory` releases the factors. The existing
left-preconditioning convention and physical operator are preserved. Singular
and nonfinite blocks stop setup explicitly; no regularization was introduced.

Production compilation passed with the root AGENTS commands. The user explicitly
approved the bounded rank-5 plan with “I approve”: archived/current block checks
normally and with ASan/UBSan, the existing 24-RWG Krylov reference suite, and
three interleaved before/after full-run repetitions per mode, one worker. All 18
new checks passed in each build (four cases require diagnostic exit 1). The
baseline reproduced the later-pivot failure, unused mode-0/2 allocations, and
missing cleanup. Both 10-case Krylov reference runs passed, with mode 1 physical
residual `9.547877159306173e-4` within `0.001`, unchanged iteration/product counts,
identical printed currents, and complex far-field relative L2 change at most
`5.21e-14`. Modes 0/2 results and statuses are byte-identical.

Three-trial median full-run seconds before/after: mode 0 `0.16/0.17`, mode 1
`0.20/0.19`, mode 2 `0.18/0.18`. Peak RSS KiB: `10660/10528`, `10552/10760`,
`10688/10748`. This subsecond fixture is too small to establish performance gains.
See `tests/README_BLOCKS.md` and the rank-5 entry in `CHANGELOG_DEV.md` for the
commands, geometry/settings, and numerical limits. Existing compiler warnings
remain. The initial plan excluded large sphere/SOM, whole-program leak, and
high-core benchmarks. The user subsequently requested the sphere23 comparison
recorded below. No approved rank-5 check remains pending.

Artifacts are retained at `/tmp/mlfma-rank5/implementation-xwtafvea`:

- `before/`: original source/tests/notes, executable/build log, baseline block and
  sanitizer probes, and full reference suite.
- `blocks/`, `blocks-sanitize/`: new checks and retained driver builds/logs.
- `reference/`, `reference-comparison.json`: full reference outputs, independent
  residual checks, and numerical comparisons.
- `benchmarks/`, `benchmark.py`, `benchmark-run.log`: all 18 full-run repetitions
  and `benchmarks/summary.json`.
- `build.log`, `run_mlfma`, `metadata.json`, `manifest.json`: final build,
  toolchain/approval details, and source/artifact hashes.

Commands: `python3 -B tests/check_blocks.py --output-dir <dir>` and the same with
`--sanitize`; archived probes add `--source-dir <before/src> --baseline`.
Reference command: `python3 -B tests/check_krylov_reference.py --solver <exe>
--operator-driver /tmp/mlfma-rank4/implementation-xjqpan3a/cfie/cfie_operators
--reference-input /tmp/mlfma-rank4/implementation-xjqpan3a/cfie/reference/input.pnu
--output-dir <dir>`. The archived independent driver does not apply this block
preconditioner. Build: archive the old executable, `rm run_mlfma`, then
`g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma`.

The user requested committing rank-5 on the current branch. Include the sources,
tests, and validation notes; exclude built executables, temporary artifacts,
and the user's sphere-input edits. No push is authorized. Partial pivoting supplies no
condition-number estimate; finite, nearly singular blocks remain sensitive to
roundoff. Application overflow uses the existing Krylov status checks.

## Rank-5 sphere23 performance follow-up

Latest repeat after the user's PW change: one illumination at theta 90 / phi 0
degrees, all other settings unchanged. One sequential before/after pair passed,
with all five outputs/statuses byte-identical, seven outer iterations/nine
products, physical residual `5.01456521557973e-4` within `0.001`. Full wall
seconds `10.40/10.08` (-3.08%); peak RSS KiB `296424/287928` (-8,496 KiB,
-8.30 MiB, -2.87%). Solution seconds `9.121022/8.921626`. One pair cannot
establish a reliable runtime gain. Artifacts retained separately at
`/tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK`, including metadata,
input/binary copies, runner/comparison scripts, logs/resources, and `REPORT.md`.
Commands were the same two timed executable runs and postprocessing as below;
the explicit user request authorized this repeat. No rebuild/source/input edits.
Current input still contains the user's one-illumination edit and must be
excluded from a rank-5 source commit.

Previous comparison at theta 0 / phi 0 degrees:

User explicitly requested performance checks for `src/test_23_sphere_0p01m.pnu`.
The file then had a pre-existing user edit selecting one illumination at
0/0 degrees instead of the tracked 181-angle sweep, plus an EOF newline. It was
copied unchanged to both runs, never edited by the agent, and must be excluded
from a rank-5 source commit.

One sequential before/after full run passed with the retained rank-4 baseline
and validated rank-5 executable, no rebuild. Geometry: 37,587 RWGs, 3 GHz,
mode 2, six MLFMA levels, digits 2, interpolation 2, approximation 0.3,
tolerance 0.001, outer/inner limits 100/10, eight workers, one illumination.
Full wall seconds: `10.31/9.93` (-3.69%); peak RSS KiB: `296376/287996`
(-8,380 KiB, -8.18 MiB, -2.83%). Solution-stage seconds: `9.048314/8.819485`.
Both converged in seven outer iterations/nine products, physical residual
`5.891066763345023e-4`. All five numerical output/status files are byte-identical.
The memory reduction is consistent with skipping unused mode-2 block storage.
One pair cannot isolate the runtime benefit from variation; large mode-1 LU
performance and the 181-angle sweep were not tested in this follow-up.

All inputs, binaries, resource files, logs, comparisons, hashes, and report are
retained at `/tmp/mlfma-rank5/sphere23-before-after-MgLKxUum`. Executed
`python3 -B <that-directory>/run_comparison.py` (two sequential
`/usr/bin/time -v -o resources.txt ./run_mlfma` runs, `OMP_DYNAMIC=FALSE`,
`OMP_THREAD_LIMIT` unset), then `python3 -B <that-directory>/compare_results.py`.
Source/executable hashes matched the original rank-5 validated manifest.
No additional repetitions or solver settings were introduced. These measurement
notes accompany the requested rank-5 commit; the check changed no solver sources.

## Previous rank-4 handoff (retained for context)

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
