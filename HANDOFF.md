# Handoff

## Current task: commit build and documentation work — default options retained

Branch `main`, starting HEAD `0362fc027e8a9c856302fa9896f2b8db14b05ac0`. This handoff accompanies the
user-requested commit of Makefile, README.md, .gitignore and development notes.
Default `make` retains `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`, with
`LTO=0`, empty `ARCH`/`TUNE`, and `PGO=off`; optional switches remain opt-in.
No solver source or numerical setting changed. No new build/test/solver run
needed in this commit-only follow-up; prior compilation and sphere23 comparison
results are recorded below. No push authorized or performed.

The pre-existing user edit in `src/test_23_sphere_0p01m.pnu` (181 -> 31 incident
phi samples) is excluded and remains uncommitted. Preserve all artifacts.
Commit-review records: `/tmp/mlfma-build-docs-commit.3v_uaw_b/`.
Sphere comparison report: `/tmp/mlfma-options-sphere23.Lg5Hef/README.md`.
Additional tests/repetitions still require approval; this commit request does
not authorize pushing. No requested implementation or validation remains pending.

## Previous task: sphere23 compiler-option comparison — complete

Branch main, HEAD 0362fc0. The user explicitly requested this case; one measured
run per default/LTO+tuning/LTO+native/PGO build plus one PGO training run completed
sequentially, all eight-thread, on the unchanged user input (31 illuminations,
preconditioner 2). No further performance runs or repetitions are authorized.
Pre-existing user change: src/test_23_sphere_0p01m.pnu PW phi count 181 -> 31;
preserve it separately from Makefile/docs edits. All 98 source/test/input hashes
match the task-start manifest. No solver or numerical parameter edited.

Confirmed full wall / peak RSS (includes setup, solves, output and cleanup):

| Build | Wall s | Change | Peak RSS KiB |
| --- | ---: | ---: | ---: |
| Default O3 | 221.34 | baseline | 287864 |
| LTO=2 TUNE=native | 222.72 | +0.62% | 287920 |
| LTO=2 ARCH=native | 201.79 | -8.83% | 287852 |
| PGO=use, default CPU flags | 238.18 | +7.61% | 287920 |

Separate PGO training: 1430.57 s / 288192 KiB, same full input. Atomic profile
updates are costly; this is excluded from optimized timings. One trial each,
no statistical estimate or independent PGO training/generalization evidence.
Combined LTO/native was fastest here; no isolated attribution to either option.

All five runs exit 0, all 31 illuminations converge below unchanged 0.001.
Per-angle statuses/iterations/products match (181 outer iterations, 243 products).
All output comparisons pass unchanged 1e-12 relative L2; maximum coefficient
relative difference 7.29e-14 (native), fields/triangle diagnostics byte-identical.
LTO+tuning and PGO outputs byte-identical to default. Maximum physical residual
about 9.93188384084108e-4. Broader suites NOT RUN, outside this case request.

Focused supporting fix: Makefile recursively tracks .gcda profiles, including
nested directories produced by absolute object paths; README documents this and
avoids a stale fixed sweep count. Initial runner stopped at flat profile discovery
after successful training; all 43 profiles intact. Corrected discovery and resumed
only missing PGO-use build/run; no completed case repeated. Both PGO builds pass
with 38 existing warnings, no missing/mismatched-profile diagnostics. All 43
profiles tracked and unchanged after optimized execution. Incremental build
preserves hashes/timestamps; profile-update dry-run selects all 43 objects plus
link. Agent files pass whitespace review; existing input trailing space retained.

Retain /tmp/mlfma-options-sphere23.Lg5Hef/: README.md (full report), summary.json,
metadata.json, executed-commands.json (exact commands/environment), final-checks.json,
analysis.log, runner/analyzer/report/check scripts, starting-files, five case
directories with inputs/executables/logs/status/results/resources/memory samples,
pgo-build and profiles (43 .gcda files), build logs and profile manifest.
Commands: python3 -B <artifact>/run_comparison.py, then --resume-pgo only after
harness correction; analyze_results.py; write_report.py; final_checks.py. Each
solver call: /usr/bin/time -v -o resources.txt ./run_mlfma in its case directory.
PGO builds use make -j2 PGO=generate/use with the same absolute pgo-build/TARGET/
PROFILE_DIR paths; exact arrays are in command records. No artifacts deleted.

All Makefile/README/.gitignore/development-note work remains uncommitted/unpushed.
No requested validation remains pending. Extra cases/repetitions, independent
training or broader tests require approval; commits/pushes require requests.
Native executable portability is limited to compatible CPU features.

## Previous task: optional compiler optimizations — implemented, build verified

Branch `main`, HEAD `0362fc0`. User requested Makefile support for the suggested
compiler optimizations. Pre-existing uncommitted work from this session is the
new Makefile, rewritten README, .gitignore, and development notes below; preserved.
This follow-up changes only Makefile, README.md, and the two development notes.
All 98 source/test/input files remain unchanged. No commit or push authorized.

Added opt-in GCC switches `LTO=2`, `TUNE=native`, `ARCH=native`, `PGO=generate/use`,
and `PROFILE_DIR`; defaults remain `LTO=0`, no additional architecture/tuning,
and `PGO=off`. Flags apply at compilation and linking and are part of the build
configuration record. PGO generation uses atomic counters for OpenMP; profile
use rejects missing/mismatched data and depends on existing `.gcda` files.
Training is never automatic. `make help` and README document usage, separate
build directories, profile lifecycle, portability, and rounding risks.
No fast-math options added.

Compilation/linking PASS with two compile jobs and at most two LTO jobs:

- `make -j2`
- `make -j2 LTO=2 TUNE=native BUILD_DIR=build/lto TARGET=build/lto/run_mlfma`
- `make -j2 LTO=2 ARCH=native BUILD_DIR=build/native TARGET=build/native/run_mlfma`
- `make -j2 PGO=generate BUILD_DIR=build/pgo TARGET=build/pgo/run_mlfma`

GCC 11.4.0 / GNU Make 4.3; 38 existing warnings in every build, no additional
LTO diagnostics. Default executable SHA-256 is unchanged:
`7baace161bd48a263d32042668f5c97ea1633a962a2cf92bb4d1e9f53c27b354`.
Unchanged repetitions of all four commands preserve hashes/timestamps and
produce empty logs. Off/tuned/native switch changes rebuild a single main.o
in `build/options-switch/`. A separate PGO-use main.o compilation without
profiles fails as intended with `-Werror=missing-profile` (make exit 2).

Numerical tests, solver timings, PGO training, and a successful profile-use build:
NOT RUN. Training/validation/benchmark execution needs approval under AGENTS.md;
no runtime benefit or numerical equivalence is claimed for the optional builds.
Native CPU instructions/FMA may change rounding and reduce binary portability.

Retain `/tmp/mlfma-build-options.nROib0/`: starting docs/build-file and old
executables; source hashes; build logs and exact argument arrays; missing-profile
diagnostic; incremental/option-switch logs; review/build summary JSON files.
Root/default, `build/lto`, `build/native`, and `build/pgo` executables retained.
No cleanup performed. Earlier `/tmp/mlfma-build-docs.zl8kd9/` remains intact.
Next: review changes; optionally approve a bounded baseline/LTO/native numerical
comparison and a separately scoped representative PGO training plan.
All changes from both Makefile tasks remain uncommitted and unpushed.

## Previous task: Makefile and user README — implemented, build verified

Branch `main`, starting HEAD `0362fc0` (merged rank 7); working tree initially
clean. User requested a proper Makefile and README explaining scope, compilation,
and usage. Changes are limited to `Makefile` (new), `README.md`, `.gitignore`,
and development notes. No solver, test, or input changes; all 98 source/test/input
files match the recorded SHA-256 manifest. Changes remain uncommitted and
unpushed; no new commit, push, test, or simulation authorization was requested.

`make -j2` builds `run_mlfma`, preserving `-Wall -fexceptions -O3 -std=gnu++17
-fopenmp`. Objects/dependencies/configuration are in `build/`; header dependencies
and a content-compared configuration record support incremental rebuilds.
Compiler, flags, source-list, build-directory, and target overrides are supported.
`make help` documents targets; cleanup is explicit and was NOT RUN. Inputs,
results, and logs are not removed by the clean recipe. Root executable and
default build directory are ignored by Git.

Builds completed: default release and documented debug command with two compile
jobs, GCC 11.4.0 / GNU Make 4.3. Default release reports 38 existing warnings.
An unchanged release build preserves every object/dependency/config/executable
timestamp and hash; the debug no-change build invokes no compiler. A harmless
CPPFLAGS marker on debug main.o was removed by the following debug build, which
correctly recompiled it. Header dependency dry-run for `mlfma_workspace.h`
selects its nine consumers and relinking, excluding unrelated objects.
No solver executable was run. Numerical regressions/simulations: NOT RUN;
fresh execution requires explicit approval under AGENTS.md. No numerical impact
is expected because sources and default numerical/compiler settings are unchanged;
compilation alone is not numerical validation.

README covers PEC/CFIE/EFIE and open components, supported input modes, embedded
mesh and units/indexing, OpenMP requests, preconditioners, far-field selectors,
output conventions, physical residuals/status, and the modules in the solution
path. Its run example copies the tracked smaller sphere into a unique /tmp
directory; it was reviewed against code but NOT executed in this task.

Retained artifacts: `/tmp/mlfma-build-docs.zl8kd9/` contains `build.log`, exact
commands in `build-command.json` / `build-check-commands.json`, incremental
snapshots, source manifest, header dry-run, debug logs, and `build-summary.json`.
Root `run_mlfma`, `build/`, and `build/debug/run_mlfma` are also retained.
Next step: review the five changed files; optionally authorize a bounded
numerical smoke run or request a commit. Earlier artifacts below remain intact.

## Previous task: rank-7 shared angular tables and child shifts — implemented and validated

Branch `codex/fix_rank7`, starting HEAD `96eaa30277ada80b39777026b1ef293fd6c35095`.
Initial working tree clean. This handoff accompanies the user-requested rank-7
commit, including code, tests, and small/large validation notes. No push is
authorized. No input files changed, no root executable created. Preserve
all retained artifacts at `/tmp/mlfma-rank7-u598ls45`.

Implemented: translation setup borrows existing workspace angular grids;
`ChildShiftTable` caches eight upward vectors per angular shape/sampling/parent
width/wavenumber. Accurate and approximate far operators and accelerated field
aggregation use shared read-only tables, prepared before OpenMP loops. Downward
shifts use conjugates. Octants derive from displacement signs, independent of
missing children. If center subtraction differs enough to change the phase
argument by more than 64 double epsilons, retain the original direct formula.
Cleanup/new tree resets the cache; retained-byte accounting covers new tables.
Interpolation cache construction/settings unchanged. No full radiation-pattern
cache or Krylov storage work.

Changed sources: `mlfma_workspace.h/.cpp`, `calculate_translationoperator.cpp`,
`mvm_Zfardotx.cpp`, `mvm_Zfardotx_in.cpp`, `calculate_farfield_mlfma.cpp`.
Tests: new `angular_cache.cpp` and `check_angular_cache.py`, extended
`workspace_operators.cpp`; docs `README_ANGULAR_CACHE.md` and `tests/README.md`.
Development notes accompany this scope in `CHANGELOG_DEV.md`.

User approved bounded rank-7 validation. Production baseline/current build passes
with `g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o <artifact>`;
38 existing warnings unchanged. Baseline/current CFIE and ten-case full reference
suites pass. The latter uses the untouched baseline independent verifier.
Normal/sanitizer thread suite passes 1/2/8/16/32 requested workers and disabled,
limited, dynamic teams. Normal/sanitizer workspace checks pass zero-error
repetitions, stable capacities, and cleanup/rebuild. Focused cache tests pass
both signs, all octants, sparse translated children, forced fallback, key
identity, stencil comparisons, shared reads, one-theta grids, invalid/overflow
bounds, and lifecycle. Initial LSan attempts hit ptrace; identical built focused
and short workspace checks passed outside ptrace with no leaks. Final focused
edge-case rerun passes normal and ASan/UBSan with leaks disabled. Failed logs
and successful reruns are retained separately.

Before/after comparisons: 296 files, 212 byte-identical, maximum relative L2
`6.471349606033712e-13`, below unchanged `1e-12`. Statuses, iterations, and product
counts match; physical residuals meet existing reference expectations/tolerance.
All 30 timing output comparisons pass (18 byte-identical; max `7.61e-15`).
Actual tree phase checks: 32 cache hits, zero fallbacks, four tables,
maximum phase difference `7.02e-16`. Capacity after 32-worker warmup:
8,045,800 -> 8,839,208 bytes; released to zero at cleanup.

Three paired operator-driver trial medians: wall 1.18 -> 1.18 s; repeated section
0.874590106 -> 0.856026532 s; peak RSS 38,064 -> 38,880 KiB (+816 KiB).
24 RWGs, 1 GHz, automatic tree, digits 6, interpolation 4, approximation 0.8,
mode 2, tolerance 0.001, eight requested workers, warmup through 32, 100 product
pairs and six accelerated outputs/I/O. Setup/assembly/checks/cleanup included;
this is not full-solver timing. No material total-time gain established.
Host four physical/eight logical CPUs: oversubscribed correctness tests do not
establish >16-core throughput. The initial bounded plan excluded large sphere/SOM
and full solver performance repetitions. The user subsequently requested the
sphere23 pair recorded below; further repetitions/SOM still require approval.
Previous large mode-1 physical residual failure remains outside this optimization;
no tolerance/stopping fix.

Useful artifacts: `before/` untouched baseline; `run_mlfma` final executable;
`build.log`; `cfie/`, `reference/`, `threads/`, `threads-san/`, `workspace/`,
`workspace-san/`; `angular-final/`, `angular-san-final/` focused final checks;
`angular-san/run-outside-ptrace.log`, `workspace-san/leaks-outside-ptrace/` leak
reruns; `comparison.json`, `timing/`, `metadata.json`, `COMMANDS.md`, and
`validated-manifest.json`. Exact commands are in `COMMANDS.md` and test READMEs.
No bounded approval remains pending. The user requested committing rank 7 on
the current branch; this handoff accompanies that commit. Additional geometry/
performance runs require approval. Artifacts retained; no push authorized.

## Rank-7 sphere23 performance follow-up

User explicitly requested the current `src/test_23_sphere_0p01m.pnu`. One
sequential pre-rank-7/current pair completed using retained validated binaries;
all 102 original manifest files and binary hashes matched before running and
before notes changed. No source/test/input edits or rebuild for this follow-up.
Current input uses 181 illuminations and preconditioner 2 (not the historical
rank-6 31-angle/mode-1 edits). Geometry/settings: 37,587 RWGs, 3 GHz, six levels,
digits 2, interpolation 2, approximation 0.3, tolerance 0.001, limits 100/10,
theta 90, phi 0..360, polarization 0, dir_ff 0; eight observed/requested workers.

Both exit 0 and all 181 converge; per-angle statuses, iterations and products
match (727 outer iterations, 1,089 reported outer Krylov products). Max physical
residual approximately 9.33977074068e-4; max difference 6.72e-17. Triangle-current
and contribution files byte-identical; coefficient relative L2 3.12e-14 and
reconstructed complex field relative L2 3.42e-14, within unchanged 1e-12.
One weak field component phase differs by 1e-8 radians in its last printed digit.
Raw mixed-unit field-column norm 3.30e-12 exceeds 1e-12; explicitly recorded as
failed raw diagnostic separately from passing physical field comparison.
No tolerances or references changed.

Full wall seconds: 1071.43 -> 972.46 (-9.24%, 98.97 s saved).
Solution stage: 1070.264144 -> 971.282542 s (-9.25%).
Peak RSS: 287,656 -> 287,960 KiB (+304 KiB, +0.30 MiB, +0.106%).
New shift payload calculated from truncations: 262 KiB, consistent with RSS
increase; exact allocator overhead unprofiled. Memory HWM nearly flat after
first illumination. One pair is not a repeated statistical estimate or a
high-core scaling result. Full wall includes setup/solves/residuals/fields/
output/cleanup; no other agent-initiated builds/tests overlapped.

All artifacts retained at `/tmp/mlfma-rank7-sphere23-before-after-beiek8po`:
`README.md`, `metadata.json`, `summary.json`, `analysis.log`, `run_comparison.py`,
`analyze_results.py`, before/after input and executable copies, logs, resource
reports, statuses, memory samples, and final hashes. Executed runner then
analyzer; actual solver command `/usr/bin/time -v -o resources.txt ./run_mlfma`,
OMP_DYNAMIC=FALSE, OMP_NUM_THREADS=8, OMP_THREAD_LIMIT unset. Original input
SHA-256 `457158ee5e1378e2de6947d82d117023a686a3287c299c768249a7ec4c94235f`
unchanged. No remaining approval/check for this pair; further cases/repetitions
need approval. These results accompany the requested rank-7 commit; no push.

## Previous rank-6 handoff: ownership and MLFMA workspace — implemented and validated

Branch `codex/fix_rank6`, starting clean at
`9d2a62412f80663d049f5f0488471e5069efba73`. Initial implementation started clean.
The user subsequently changed the sphere23 PW phi sample count from 181 to 31
and Preconditioners from 2 to 1. Preserve both input edits and exclude them from
any rank-6 commit unless requested.
This handoff accompanies the user-requested rank-6 implementation commit,
including its tests and validation notes. No push is authorized. The user input
edits and untracked executable are excluded from the commit.
Preserve the untracked root `run_mlfma` and every artifact.

Ownership and MLFMA workspace reuse are complete for this phase. Krylov storage
has not been changed and remains deferred. No full RWG radiation-pattern cache.

New `src/mlfma_workspace.h/.cpp` owns angular/trigonometric grids and
interpolation stencils keyed by exact shape/order/sampling convention. The
accurate and approximate far operators plus accelerated field output reuse
worker field/interpolation buffers. Metadata is prepared serially and shared
read-only; a barrier-protected pool uses actual teams, including disabled,
limited, dynamic, and 32-worker requests. Global solver state remains sequential
and nonreentrant. Workspace clears at tree creation and simulation cleanup.

`clear_memory` releases/nulls all covered owners, including previously missing
material, illumination, solution, and approximate-operator arrays. Local direct
field and topology deletions null their globals. Source counts borrow static
storage. Unused approximate translation temporaries were removed. Legacy
interpolation signatures remain available. No numerical method/settings changed.

New regression: `tests/check_workspace.py` / `tests/workspace_operators.cpp`.
Instructions/evidence: `tests/README_WORKSPACE.md`. Existing input/block subset
build scripts gained workspace link dependencies; compile/link checks passed.
Those additional suites were NOT executed.

User approved: “bounded rank-6 validation is approved”. Completed baseline and
updated small-fixture checks, including normal and ASan/UBSan ownership/thread/
CFIE tests, both ten-case 24-RWG Krylov reference suites (modes 0/1/2, repeated
illuminations, limits), repeated accurate/approximate products/fields, and three
sequential interleaved timing trials. All 424 saved normal numerical outputs are
byte-identical. Repetition difference is zero; retained workspace capacity stays
at 8,045,800 bytes after 32-worker warmup and becomes zero at cleanup. Partial
and repeated cleanup plus metadata rebuild checks pass.

Baseline short LSan run: 24,312,560 bytes leaked in 1,966 allocations; updated
matching run: exit 0, no reported leaks. Sandbox ptrace initially blocked LSan;
failed logs were retained and checks rerun with sandbox approval outside ptrace.
Normal and sanitizer thread tests cover serialized/runtime-limited/dynamic teams
and 1/2/8/16/32 requests; throughput beyond eight logical CPUs remains untested.

Production builds passed using root AGENTS flags; existing 38 compiler warnings
unchanged. Final root executable hashes identically to the validated production
build (later source changes were whitespace/declaration formatting only).
CRLF-aware diff check and Python syntax inspection passed. Existing C++ CRLF
preserved; new files LF. Read-only review confirmed no Krylov storage/source edits.

Three-trial median before/after on the 24-RWG fixture (1 GHz, unchanged settings):
wall 1.20/1.13 s; repeated section 0.888264/0.827768 s; peak RSS 58,804/38,128 KiB
(-20.19 MiB). Setup/assembly, team warmup through 32, 100 product pairs at eight
workers, six accelerated field outputs/I/O/comparisons, cleanup included. These
are operator-driver timings, not solver solve-time claims. All timing output
files also match. Host: GCC 11.4.0, i7-7700HQ, four physical/eight logical CPUs.

Artifacts: `/tmp/mlfma-rank6-26OJl7kI` (all retained). Exact commands in
`COMMANDS.md`; baseline sources in `before/src`; results in `before/` and
`after/`; `comparison.json`, `timing/summary.json`, and
`timing/numerical-comparison.json` contain comparisons/measurements;
`validated-manifest.json` records source/test/executable hashes. Useful production
log: `after/build-ready.log`; leak logs: `before/workspace-san/leaks/` and
`after/workspace-san/leaks/`; current CFIE sanitizer rerun:
`after/cfie-san-unrestricted-run.log`.

All requested checks have been executed. The larger sphere pairs are recorded
below. Mode 1 fails the physical residual tolerance in both builds; this
existing stopping-criterion limitation remains unresolved. SOM, the original
181-angle sweep, high-core throughput, a whole-program leak audit, and Krylov
storage work remain outside this phase.
Expanded tests require separate AGENTS approval; commits/pushes need explicit
requests. Next: user review before changing Krylov storage.

## Rank-6 sphere23 performance follow-up — completed

The user explicitly requested performance checks for
`src/test_23_sphere_0p01m.pnu`. One sequential full archived/current pair used
identical copies of the current 31-illumination input, leaving the source input
and rank-6 implementation untouched. Both existing executables were verified
against their recorded sources/manifest; no rebuild needed.

At 3 GHz, 37,587 RWGs, mode 2, six levels, digits 2, approximation 0.3,
interpolation 2, tolerance 0.001 and eight workers: full wall seconds
`238.24/234.44` (-1.60%); solution seconds `237.126272/233.295821` (-1.62%);
peak RSS KiB `301120/287540` (-13.26 MiB, -4.51%). Both exit 0 with all
31 illuminations converged, 181 outer iterations, 243 Krylov matrix products,
and maximum physical residual `9.93188384084087e-4`. All five result files
are byte-identical, including currents, direct monostatic fields and status.
Sampled HWM grows from 288028 to 301120 KiB in the baseline and stays near
287468–287540 KiB after rank-6. This is consistent with leak removal; samples
across different solve phases do not establish a leak rate. One timing pair
cannot establish a reliable speedup. Original 181-angle sweep/SOM not rerun.

All artifacts retained at
`/tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b`, including input, executables,
metadata, starting user input diff, exact commands, run/analysis scripts, logs,
RSS samples, and `summary.json`. No source/test-code changes or new test
approvals pending. Documentation only updated for this follow-up. No staging,
commit or push performed; preserve the user's 31-angle input edit.

## Rank-6 sphere23 mode-1 follow-up — measured, physical tolerance unmet

User explicitly requested the repeat after changing Preconditioners from 2 to 1.
One sequential baseline/current pair used identical current input copies;
no source/test-code or input changes. Existing verified executables reused.

For 31 illuminations, 37,587 RWGs, 3 GHz, tolerance 0.001, six levels,
interpolation 2, digits 2 and eight workers: full wall seconds
`339.18/333.36` (-1.72%); solution seconds `338.016582/332.210418` (-1.72%);
peak RSS KiB `235016/226780` (-8.04 MiB, -3.50%). Both build 4,312 pivoted LU
blocks containing 417,495 complex coefficients. All five result files are
byte-identical; 648 iterations and 710 Krylov matrix products on both sides.

Both complete all 31 illuminations but exit 1: every status is
`physical_residual_not_converged`. Preconditioned GMRES converges, but the
physical relative residual ranges from `1.36948e-3` to `1.72394e-3`, above
0.001. This is identical before/after rank-6. Numerical identity passes;
physical convergence validation fails. stage_solution checks the physical
residual only after the preconditioned solve and flags it without continuing.
No stopping-criterion or tolerance change was made; fixing that is separate work.

Compared with the previously measured current mode-2 run, current mode 1
uses 21.13% less peak memory but takes 42.19% longer (333.36 vs 234.44 s).
Mode 2 meets the physical tolerance for all illuminations; mode 1 meets it for
none, so the mode comparison is not at equal achieved accuracy. One pair cannot
establish a reliable speedup from rank-6.

All artifacts, commands, scripts, binaries, inputs, starting diffs, logs,
measurements, RSS samples and summary.json retained at
`/tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1`. The requested pair is
complete; no additional test approvals pending. Physical convergence remains an
unresolved mode-1 limitation. No source/test-code edits or Git mutations; retain
both user input edits and all prior artifacts. Documentation updated only.

## Earlier rank-5 handoff (historical)


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
