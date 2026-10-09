# Development change log

### 2026-10-08 20:14 UTC — Rank-4 input and dimension validation

- User request and authorized scope: implement rank-4, validate inputs and
  explicitly reject unsupported modes, preserving the shared input parameters
  and identifiers (`Formulations`, `source_type`). Retain all artifacts. The
  session authorizes compilation, baseline/reference runs, local tests, and
  before/after measurements; no rank-4 commit or push was requested.
- Branch and starting HEAD: `codex/fix_rank4`, `42b3a59` (merged rank-1 through
  rank-3 work). No pre-existing working-tree edits were present at task start.
- Files/functions changed: `src/read_simulationinput.cpp::read_simulationinput`;
  new `src/input_validation.{h,cpp}`; `generate_octtree`,
  `generate_Znearcomponent`, `generate_Zfarcomponents`,
  `generate_translationcomponent`, and `interpolationweights` in their matching
  source files; new `tests/check_inputs.py`, `tests/input_validation.cpp`, and
  `tests/input_dimensions.cpp`; the invalid-inner expectation in
  `tests/check_krylov_reference.py`; `tests/README.md` and new
  `tests/README_INPUTS.md`; this log and `HANDOFF.md`.
- Changes and rationale: replace unchecked input extraction with checked
  records/conversions and owned intermediate mesh storage; reject unsupported
  PEC/source/sweep selections; validate mesh indices/materials/finite geometry;
  check active interpolation dimensions and index arithmetic before allocations;
  reject unsafe direct-grid steps; bound the interpolation location scan.
  Preserve named shared metadata, inactive settings, safe zero-order approximate
  grids, frequency endpoints with count 1, and thread counts above 16.
- Expected numerical impact: accepted inputs retain the physical formulation,
  truncation formula, quadrature, coefficients, tolerances, and solver algorithm.
  Invalid inputs stop with status 1 and a field-specific error instead of
  silently selecting another problem or overflowing storage.
- Validation plan and approval: verify the original reader/dimension findings
  against archived sources first; run focused input and sanitizer probes;
  repeat existing CFIE/thread/reference checks and small before/after cases.
  These runs continue the session's approved implementation and validation
  scope. No additional external repository work or full SOM solve was performed.
- Commands actually executed (output paths below):
  - `rm run_mlfma` and
    `g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma`
    for baseline and implementation builds. Existing executables were archived
    before removal. The first baseline removal reported an absent executable.
  - `python3 -B tests/check_inputs.py` using archived/current `--source-dir`,
    `--baseline`, `--baseline-dir`, `--output-dir`, and `--sanitize` variants.
    Exact driver compilation arguments are retained in `build-command.json`.
    Final ordinary and sanitizer outputs: `inputs-final-v2`,
    `inputs-final-sanitize-v2`.
  - `python3 -B tests/check_cfie.py --output-dir <artifacts>/cfie`;
    sanitizer runs under `cfie-sanitize` and `cfie-sanitize-reviewed`, the latter
    reusing binaries with `--sanitize --skip-build` outside sandbox ptrace.
  - `python3 -B tests/check_thread_parallel.py` with
    `--input src/test_29_sphere_0p03m.pnu`, retained `--output-dir` values,
    `--sanitize`, and archived serial `--baseline-dir` comparisons.
  - `python3 -B tests/check_krylov_reference.py --solver ./run_mlfma
    --operator-driver <artifacts>/cfie/cfie_operators
    --reference-input <artifacts>/cfie/reference/input.pnu
    --output-dir <artifacts>/reference-final-v2` (also earlier build checks).
  - Archived/current executables in isolated working directories, measured with
    `/usr/bin/time`, for small closed/open references, unsupported modes,
    direct-grid step 7, zero-order approximation, and three interleaved small
    sphere/reference trials. Parse-only checks/dumps used the three local meshes,
    including SOM. `OMP_DYNAMIC=FALSE` was used for numerical comparisons.
- Build outcome: final build succeeded; existing compiler warnings remain.
  Initial development driver builds missing `<complex>` failed, then were
  corrected; their logs were retained.
- Test outcome: both final input suites passed all 99 cases (14 accepted,
  85 expected errors). Actual baseline sanitizer findings include name/material
  overflow, invalid interpolation access, near-product overflow, and translation
  sum overflow. Baseline full impedance/sweep selections exited successfully
  with unsupported behavior; direct-grid step 7 aborted with heap corruption.
  Final cases reject these before numerical results are written. CFIE and thread
  suites passed ordinary and sanitizer checks; initial topology LeakSanitizer
  execution failed under sandbox ptrace and passed on the unrestricted rerun.
  Full Krylov references passed with unchanged tolerances, including multiple
  illuminations and intended iteration-limit exits. Reference physical residuals
  for modes 0/1/2: `2.35e-4`, `9.55e-4`, `2.26e-4` versus tolerance `1e-3`.
  Closed/open and safe zero-order full results were byte-identical to baseline;
  all local parsed mesh/material arrays matched baseline byte for byte.
- Measurements and output locations: retained under
  `/tmp/mlfma-rank4/implementation-xjqpan3a`. Three-trial median seconds and peak
  RSS KiB before/after: 4,080-RWG sphere, eight workers, `1.70/1.62` and
  `33432/33484`; small mode-2 reference, one worker, `0.19/0.18` and
  `10728/10748`; SOM parsing/dump only, `0.11/0.30` and `8872/20448`.
  These timings do not demonstrate a speed improvement; one sphere trial
  overlapped small reference checks. Parser validation costs time and temporary
  duplicate-triangle storage. Complete settings/evidence are in
  `tests/README_INPUTS.md`, `benchmarks/results.json`, and retained per-run files.
- Remaining uncertainty or pending work: no full SOM solve or more-core
  throughput measurement; no general mesh-quality or whole-program leak audit.
  Other repositories' impedance/source implementations and unrelated inversion
  warnings were not changed. Index guards cannot guarantee physical RAM capacity.
- Commit/push status: rank-4 changes uncommitted and unpushed, ready for review.

### 2026-10-08 21:13 UTC — Sphere23 full before/after comparison

- User request and authorized scope: explicitly requested a before/after test
  using `test_23_sphere_0p01m.pnu`. Ran one sequential baseline/current full pair
  with the original input and all 181 illuminations; retain all artifacts.
- Branch and starting HEAD: `codex/fix_rank4`, `42b3a59`. Existing uncommitted
  rank-4 changes were preserved. No production source or numerical setting changed.
- Files and functions changed: documentation only (`CHANGELOG_DEV.md`,
  `HANDOFF.md`, `tests/README_INPUTS.md`); runner, comparison script, metadata,
  inputs, executable copies, logs, and results retained in the artifact directory.
- Changes and rationale: extend numerical and resource evidence from parsing-only
  and smaller references to a complete run on the specifically requested sphere.
- Expected numerical impact: none; compare the executable archived before rank-4
  against the validated current rank-4 executable, without implementing the
  subsequently suggested parser optimizations.
- Validation plan: `/usr/bin/time -v` for one full run per executable, original
  parameters, sequentially; compare every result file byte for byte and verify all
  illumination statuses, true physical residuals, iteration totals, and exit codes.
- Approval received and its scope: the user's explicit request for this named
  before/after test satisfies AGENTS.md section 5. No extra solver cases or
  repetitions were run.
- Commands actually executed: `python3 -B <artifacts>/run_comparison.py`, running
  `/usr/bin/time -v -o resources.txt ./run_mlfma` in each isolated stage directory
  with stdout/stderr in `run.log`; `python3 -B <artifacts>/compare_results.py`;
  read-only source/artifact inspection, executable/source hash verification,
  `lscpu`, `g++ --version`, and patch whitespace check.
- Build outcome: no rebuild needed. Reused the archived production builds with
  flags `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`; current source/binary hashes
  matched the validated rank-4 manifest before execution. Compiler/host metadata
  retained. No compiler flags or dependency changes.
- Test outcome: both executables exited 0; all 181 illuminations converged within
  unchanged tolerance `0.001`. Maximum physical residual was
  `0.0009339770740679938` in both. All five result text files were byte-identical
  (RWG coefficients, triangle currents, field contributions, backscatter, solver
  status). Both reported 727 total outer iterations (4–7 per illumination) and
  1,089 Krylov matrix products.
- Measurements and output locations: radius approximately 0.3 m, 12,531 nodes,
  25,058 triangles, 37,587 RWGs, 3 GHz; plane waves theta 90 degrees, phi 0–360
  degrees, 181 samples; preconditioner 2, explicit six levels, two digits,
  approximation 0.3, interpolation width 2, inner limit 10, outer limit 100,
  backscatter selector 0; eight requested workers, `OMP_DYNAMIC=FALSE`, no
  `OMP_THREAD_LIMIT` override, on the i7-7700HQ/eight-logical-CPU host.
  Full setup/solve/output before: 1,099.11 s, peak RSS 352,796 KiB; after:
  1,128.97 s, 353,564 KiB. Changes: +29.86 s (+2.72%), +768 KiB (+0.22%).
  Reported solution stage: 1,097.840165 / 1,127.691979 s; non-solution wall-time
  remainder: about 1.270 / 1.278 s. The observed timing increase is almost entirely
  in the solution stage, with unchanged reported numerical work.
  All evidence retained at `/tmp/mlfma-rank4/sphere23-before-after-8mm2uv5c`.
- Remaining uncertainty or pending work: one sequential pair cannot establish
  timing variability or the cause of a 2.72% difference. No targeted performance
  profile, additional repeated runs, parser optimization, or full SOM solve was
  performed. The small full-sphere RSS increase does not negate the earlier
  confirmed SOM parsing overhead; they measure different geometries and phases.
- Commit/push status: documentation and rank-4 changes remain uncommitted and
  unpushed. No staging, commit, or push was authorized or performed.

- Inspection note: `interpolationweights` checks also run inside accurate and
  approximate MVM paths. Their runtime contribution is an unprofiled hypothesis;
  unchanged convergence counts do not prove zero added per-product overhead.

### 2026-10-08 21:18 UTC — Commit validated rank-4 changes

- User request and authorized scope: explicitly requested committing the completed
  changes, including the large sphere comparison, on the current branch.
- Branch and starting HEAD: `codex/fix_rank4`, `42b3a59`.
- Files and functions changed: commit-status documentation in `HANDOFF.md` and
  this append-only entry; no source, algorithm, input, or tolerance changes.
- Changes and rationale: commit the reviewed rank-4 sources, regression tests,
  and development/validation notes together. Exclude `run_mlfma` and retained
  external artifacts from staging.
- Expected numerical impact: none beyond the previously validated rank-4 change.
- Validation plan: inspect working/staged diffs, check whitespace, verify selected
  source/test hashes against reviewed snapshots, and inspect commit hooks.
- Approval received and its scope: user's current commit request authorizes
  staging and committing; it does not authorize pushing.
- Commands actually executed: read-only Git status/diff/log/config inspection,
  source/test hash and retained-result checks; Git staging, staged-diff review,
  and commit are the operations recorded by the commit containing this entry.
- Build outcome: existing validated production builds reused; no rebuild needed.
- Test outcome: previously executed checks and full sphere23 comparisons retained
  unchanged. No tests rerun for these documentation-only commit preparations;
  no active commit hook was found.
- Measurements and output locations: rank-4 artifacts under
  `/tmp/mlfma-rank4/implementation-xjqpan3a`; full sphere comparison under
  `/tmp/mlfma-rank4/sphere23-before-after-8mm2uv5c`. All retained.
- Remaining uncertainty or pending work: numerical/performance limits are recorded
  above and in `HANDOFF.md`; no additional implementation or test work introduced.
- Commit/push status: this entry accompanies the requested rank-4 commit. No push
  was requested or performed. The built executable remains untracked.

### 2026-10-08 21:49 UTC — Rank-5 pivoted LU blocks, mode 1 only

- User request and authorized scope: implement rank-5, “Use pivoted factorization
  for block preconditioning; construct it only for mode 1.” Preserve artifacts,
  the numerical formulation, existing input parameters, and accuracy settings.
  Full radiation-pattern caching remains excluded.
- Branch and starting HEAD: `codex/fix_rank5`,
  `75c25ed829c2c50aedb431c2d53274016030352a`; initially clean working tree, no
  pre-existing user changes. Untouched sources/tests/executable archived first.
- Files and functions changed: `src/matrixBlock.h` and new `matrixBlock.cpp`
  (`initmatrixBlock`, `assignvalue`, `factorize`, `solve_inplace`);
  `calculate_blockdiagonalpreconditioner.cpp`, `mvm_BDPmatrix.cpp`, `main.cpp`,
  `main.h`, `clear_memory.cpp`. Added `tests/block_factorization.cpp`,
  `block_preconditioner.cpp`, `check_blocks.py`, `README_BLOCKS.md`; updated
  `tests/README.md`, `HANDOFF.md`, and this log. Existing source CRLF preserved.
- Changes and rationale: replace Gauss-Jordan explicit inversion with owned,
  contiguous complex LU storage and partial row pivoting at every column.
  Original inversion only preorders rows by column zero, then lacks later
  pivots. Validate self interactions, near-matrix bounds, leaf RWG ranges/maps,
  and full disjoint coverage; reject zero pivots/nonfinite factors explicitly.
  No absolute epsilon cutoff, regularization, transpose, or symmetry assumption.
  Skip construction entirely for modes 0/2. Per-call largest-block application
  workspace replaces the full-RWG temporary; factors remain read-only and
  in-place application remains supported. RAII and cleanup remove block-array
  leaks and bypass the old leaked inverse pointer table. Dead inverse helpers
  remain untouched. Arithmetic overflow in application propagates to existing
  Krylov finite-value/termination reporting.
- Expected numerical impact: preserve left-preconditioned `M^-1 A` and `M^-1 b`
  and original near coefficients. Mode 1 may change rounding/iterations, and
  formerly broken pivot cases now solve correctly. Modes 0/2 lose only unused
  setup/storage. Tolerances, quadrature, MLFMA settings, and precision unchanged.
- Validation plan: reproduce with archived sources, run focused block drivers
  before/after normally and ASan/UBSan, existing full 24-RWG reference suite,
  then three sequential interleaved full-run repetitions per mode. Retain all
  artifacts. No large sphere/SOM case, whole-program leak audit, or throughput
  measurement on more cores included.
- Approval received and its scope: user explicitly replied “I approve” to the
  bounded plan: synthetic block/pivot/scaling/rejection/mapping/aliasing/cleanup
  tests, normal and ASan/UBSan before/after; existing Krylov suite on the two
  octahedra, modes 0/1/2, one/two illuminations, one worker; three before/after
  timing repetitions per mode. Estimate was 5–10 minutes and under 250 MiB
  solver RSS, compiler/sanitizer potentially 1 GiB. No expanded cases approved.
- Commands actually executed: archived source/test/notes copies; root build
  `rm run_mlfma` (baseline reported executable absent), then
  `g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma`
  before and after. `python3 -B tests/check_blocks.py --source-dir
  /tmp/mlfma-rank5/implementation-xwtafvea/before/src --baseline --output-dir
  /tmp/mlfma-rank5/implementation-xwtafvea/before/blocks` and the same with
  `--sanitize` into `before/blocks-sanitize`; current checks with `--output-dir
  /tmp/mlfma-rank5/implementation-xwtafvea/blocks`, plus `--sanitize` into
  `blocks-sanitize`. `python3 -B tests/check_krylov_reference.py --solver <exe>
  --operator-driver /tmp/mlfma-rank4/implementation-xjqpan3a/cfie/cfie_operators
  --reference-input /tmp/mlfma-rank4/implementation-xjqpan3a/cfie/reference/input.pnu
  --output-dir <dir>` for archived/current executables into `before/reference`
  and `reference`. `python3 -B /tmp/mlfma-rank5/implementation-xwtafvea/benchmark.py`
  ran all 18 repetitions using `/usr/bin/time -f '%e %M' -o resources.txt <exe>`.
  `compare_reference.py` postprocessed coefficients/complex fields. Read-only
  Git/diff, source review, compiler/hardware inspection, CRLF/whitespace, and
  source/output hash checks; exact driver build commands retained in their
  `build-commands.json` files. No unapproved tests executed.
- Build outcome: untouched and final production builds succeeded with GCC 11.4.0,
  unchanged `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`. Existing warnings remain.
- Test outcome: baseline later-pivot manufactured solve and production mapped
  solve yielded nonfinite values. Baseline mode-0/2 factors were allocated and
  cleanup left factors non-null. New 18-case suite passed both builds, including
  four expected diagnostic exits. Later-pivot forward error `3.2062e-16`, zero
  measured backward error; synthetic mapped/in-place error `6.93e-17`. Scaled
  complex solves passed `1e-12`. Near-singular forward error about `1.07e-6`
  with zero measured backward error, reflecting intrinsic conditioning.
  Both existing 10-case full reference runs passed, including failure reporting
  and independent physical residuals. Mode 1 residual before/after
  `9.547877159307686e-4` / `9.547877159306173e-4` satisfies unchanged `1e-3`;
  18 iterations and 20 products unchanged. Printed coefficients/currents
  identical; complex far-field relative L2 differences at most `5.21e-14`.
  Modes 0/2 all numerical files/statuses byte-identical.
- Measurements and output locations: `/tmp/mlfma-rank5/implementation-xwtafvea`;
  `benchmarks/summary.json` retains all trials. Two octahedra (12 nodes, 16
  triangles, 24 RWGs), 1 GHz, CFIE 0.25/0.75, digits 6, interpolation 4,
  approximation 0.8, tolerance 0.001, outer/inner limits 192/15, one illumination,
  one worker, `OMP_DYNAMIC=FALSE`. Intel i7-7700HQ, four physical/eight logical
  cores. Full setup+solve+output median seconds before/after: mode 0 `0.16/0.17`,
  mode 1 `0.20/0.19`, mode 2 `0.18/0.18`. Median peak RSS KiB: `10660/10528`,
  `10552/10760`, `10688/10748`; outer iterations `8/8`, `18/18`, `3/3`.
  Eight self blocks contain only 88 complex coefficients (1,408 bytes), below
  peak-RSS variability. These measurements establish no performance gain.
- Remaining uncertainty or pending work: no approved validation pending. Large
  cases and high-core throughput are NOT RUN, outside scope and need separate
  approval. Expected larger-case benefits: avoid unused block setup/storage in
  modes 0/2, avoid extra inverse matrix in mode 1, smaller application workspace.
  Quadratic block-application cost remains; no solve-speed gain promised.
  Finite ill-conditioned blocks have no condition estimator. ASan/UBSan used
  leak detection disabled, so no whole-program leak-clean claim.
- Commit/push status: uncommitted and unpushed, ready for review; neither operation
  requested. Retain untracked `run_mlfma` outside a future source commit.

### 2026-10-08 22:00 UTC — Rank-5 sphere23 performance comparison

- User request and authorized scope: explicitly requested performance checks
  for `test_23_sphere_0p01m.pnu` to assess rank-5. One sequential before/after
  pair using the current input unchanged; preserve all artifacts.
- Branch and starting HEAD: `codex/fix_rank5`,
  `75c25ed829c2c50aedb431c2d53274016030352a`. Rank-5 implementation/tests/notes
  and untracked executable already present. Input has a pre-existing user edit
  selecting one 0/0-degree illumination rather than the tracked 181-angle sweep,
  plus an EOF newline; preserve and exclude this input from a rank-5 commit.
- Files and functions changed: only `CHANGELOG_DEV.md`, `HANDOFF.md`, and
  `tests/README_BLOCKS.md` development notes. No solver/source/input edits.
  New measurement scripts/report and all outputs reside in the unique artifact
  directory `/tmp/mlfma-rank5/sphere23-before-after-MgLKxUum`.
- Changes and rationale: measure the effect of skipping unused mode-2 blocks on
  a 37,587-RWG sphere with identical inputs and retained before/after executables.
  Verify source/executable hashes against the validated rank-5 snapshot first.
- Expected numerical impact: none in mode 2; block preconditioning is unused.
  No changes to geometry, tolerances, quadrature, MLFMA settings, illumination,
  compiler flags, or precision between runs.
- Validation plan: one sequential full before/after pair, `/usr/bin/time -v`,
  compare result/status hashes, convergence/iterations/physical residuals, and
  record full/solution wall time and peak RSS. Output only within artifact root.
- Approval received and its scope: user's explicit request to check this named
  input counts as approval under AGENTS section 5. Announced current settings,
  one pair, eight workers, estimated 30–90 seconds total and under 500 MiB RSS
  per run (uncertain single-illumination runtime). No expanded cases/repetitions.
- Commands actually executed: read-only Git/source/input/diff/manifest and prior
  measurement inspection; `mktemp -d /tmp/mlfma-rank5/sphere23-before-after-XXXXXXXX`;
  copy current input and retained executables, save metadata/hashes. Executed
  `python3 -B /tmp/mlfma-rank5/sphere23-before-after-MgLKxUum/run_comparison.py`
  with two sequential `/usr/bin/time -v -o resources.txt ./run_mlfma` runs,
  `OMP_DYNAMIC=FALSE`, `OMP_THREAD_LIMIT` unset. Executed
  `python3 -B /tmp/mlfma-rank5/sphere23-before-after-MgLKxUum/compare_results.py`.
  Final input-hash/diff and whitespace checks; no further solver runs.
- Build outcome: no rebuild; reused previously successful baseline/final builds
  with GCC 11.4.0 and `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`. Source and
  executable hashes match `/tmp/mlfma-rank5/implementation-xwtafvea/manifest.json`.
- Test outcome: both exited 0, seven outer iterations, nine matrix products,
  physical relative residual `5.891066763345023e-4` within unchanged `1e-3`.
  RWG coefficients, triangle currents, CF contributions, far field, and solver
  status are all byte-identical. Current input hash remains unchanged.
- Measurements and output locations: `/tmp/mlfma-rank5/sphere23-before-after-MgLKxUum`.
  One illumination, 12,531 points, 25,058 triangles, 37,587 RWGs; 3 GHz, CFIE
  0.5/0.5, mode 2, levels 6, digits 2, interpolation 2, approximation 0.3,
  tolerance 0.001, outer/inner limits 100/10, eight requested workers. Intel
  i7-7700HQ, four physical/eight logical CPUs. Full setup+solve+output wall
  before/after `10.31/9.93 s` (-3.69%); peak RSS `296376/287996 KiB`
  (-8380 KiB = -8.18 MiB, -2.83%). Reported solution `9.048314/8.819485 s`;
  near assembly `0.839400/0.820428 s`; translation `0.042810/0.047911 s`.
- Remaining uncertainty or pending work: the measured memory reduction is
  consistent with skipping unused mode-2 block storage. One pair cannot isolate
  runtime benefit from ordinary variation; the solution stage changed by
  -0.228829 s despite identical numerics. Large mode-1 LU and the 181-angle
  sweep were NOT RUN in this follow-up, outside scope. No approved check pending.
- Commit/push status: still uncommitted and unpushed. No staging/commit/push
  requested. Preserve the unrelated user input edit and all artifacts.
- Final review note: agent-change whitespace check passed with the unrelated
  sphere input excluded. Whole-tree check flags existing trailing whitespace
  on the user's changed source_PW line; preserved unchanged. Input/executable
  hashes and all 75,174 finite coefficient components were verified after runs.

### 2026-10-08 22:06 UTC — Repeat sphere23 comparison with updated PW

- User request and authorized scope: explicitly requested testing
  `test_23_sphere_0p01m.pnu` once more after changing PW parameters. One
  sequential before/after pair, updated input unchanged; preserve artifacts.
- Branch and starting HEAD: `codex/fix_rank5`,
  `75c25ed829c2c50aedb431c2d53274016030352a`. Existing rank-5 changes remain;
  current sphere input's one-illumination PW edit and EOF newline belong to the
  user and are excluded from source commit scope.
- Files and functions changed: only development notes in this log, `HANDOFF.md`,
  and `tests/README_BLOCKS.md`; new retained comparison scripts/report/results
  under `/tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK`. No solver edits.
- Changes and rationale: repeat mode-2 performance/numerical comparison for the
  changed incident direction. Verify the same validated executable/source hashes
  and copy current input unchanged into both working directories.
- Expected numerical impact: no rank-5 change to mode-2 numerical results. The
  user's changed illumination changes this pair's physical solution relative
  to the previous theta-0 case, so comparisons are within each pair.
- Validation plan: one sequential before/after full solver pair; record runtime,
  peak RSS, convergence/physical residual, and byte identity of output/status.
- Approval received and its scope: explicit named-input repeat request authorizes
  the pair under AGENTS section 5. Announced theta 90 / phi 0 degrees, one
  illumination, eight workers, same settings, estimate 20–40 seconds total and
  approximately 300 MiB peak RSS per run. No extra cases/repetitions included.
- Commands actually executed: Git status/input/diff/source and artifact review;
  `mktemp -d /tmp/mlfma-rank5/sphere23-pw90-before-after-XXXXXXXX`; copy binaries,
  input, prior runner/comparator, and record metadata/hashes. Executed
  `python3 -B /tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK/run_comparison.py`,
  running `/usr/bin/time -v -o resources.txt ./run_mlfma` sequentially before
  then after with `OMP_DYNAMIC=FALSE`, `OMP_THREAD_LIMIT` unset; executed
  `python3 -B /tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK/compare_results.py`.
  Final input/executable hash, coefficient finiteness, and agent diff checks.
- Build outcome: no rebuild; same retained, previously successful baseline and
  final GCC 11.4.0 builds, `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`.
  Sources/executables match the validated rank-5 manifest.
- Test outcome: both exited 0, seven outer iterations/nine matrix products,
  physical residual `5.01456521557973e-4` within unchanged `1e-3`. All five
  result/status files are byte-identical. User's input remains unchanged.
- Measurements and output locations: `/tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK`.
  37,587 RWGs, 12,531 points, 25,058 triangles; 3 GHz, CFIE 0.5/0.5, mode 2,
  levels 6, digits 2, interpolation 2, approximation 0.3, tolerance 0.001,
  outer/inner limits 100/10, eight workers, PW theta 90 / phi 0 degrees, one
  illumination. Intel i7-7700HQ (four physical/eight logical CPUs). Full wall
  before/after `10.40/10.08 s` (-3.08%); peak RSS `296424/287928 KiB`
  (-8496 KiB = -8.30 MiB, -2.87%); solution `9.121022/8.921626 s`;
  near assembly `0.842018/0.847983 s`; translation `0.044403/0.046270 s`.
- Remaining uncertainty or pending work: memory decrease is consistent with
  omitted unused mode-2 blocks. One pair cannot establish a reliable speedup;
  solution-stage variation remains unisolated. This different incident direction
  is not a statistical repeat of the earlier theta-0 pair. No approved checks
  pending; large mode-1 performance and the 181-angle sweep remain outside scope.
- Commit/push status: still uncommitted and unpushed. No staging/commit/push
  requested. Preserve the user input edit and both comparison directories.

### 2026-10-08 22:07 UTC — Commit validated rank-5 changes

- User request and authorized scope: explicitly requested committing completed
  rank-5 sources/tests/notes, including both sphere comparisons, on the current
  branch. Preserve the user's input edits and all artifacts. No push authorized.
- Branch and starting HEAD: `codex/fix_rank5`,
  `75c25ed829c2c50aedb431c2d53274016030352a`.
- Files and functions changed: commit-status notes in `HANDOFF.md` and this log;
  no numerical or test-code changes during commit preparation.
- Changes and rationale: commit pivoted mode-1-only block LU, ownership/cleanup,
  regression tests, and numerical/performance evidence as one logical change.
  Exclude `src/test_23_sphere_0p01m.pnu`, `run_mlfma`, and temporary artifacts.
- Expected numerical impact: none beyond the already validated rank-5 change.
- Validation plan: inspect working/staged changes, verify source/tests/executable
  against the validated manifest and retained results, check agent whitespace,
  inspect hooks, then stage only scoped files and commit.
- Approval received and its scope: user's current commit request authorizes
  staging/commit, not push. No active Git hook or custom hooks path was found.
- Commands actually executed: Git status/diff/log/branch/config and hook-file
  inspection; hash/retained-results verification (no solver execution), scoped
  whitespace checks. Git staging, staged-diff review, and commit are the
  operations recorded by the commit containing this entry.
- Build outcome: previously successful baseline/final production builds reused;
  unchanged source hashes match the validated executable. No rebuild needed.
- Test outcome: existing 18-case normal/ASan/UBSan suites, full reference suites,
  small benchmarks, and both large sphere pairs retained. No tests rerun for
  these documentation-only commit preparations.
- Measurements and output locations: `/tmp/mlfma-rank5/implementation-xwtafvea`,
  `/tmp/mlfma-rank5/sphere23-before-after-MgLKxUum`, and
  `/tmp/mlfma-rank5/sphere23-pw90-before-after-Q4CIFNTK`; all retained.
- Remaining uncertainty or pending work: large mode-1 LU performance, a
  whole-program leak audit, and high-core throughput remain outside scope.
  Preserve the user input's changed PW/newline, including existing whitespace.
- Commit/push status: this entry accompanies the requested rank-5 commit.
  No push requested/performed. Built executable remains untracked.

### 2026-10-09 12:25 UTC — Rank-6 ownership and persistent MLFMA workspace

- User request and authorized scope: implement rank-6 ownership repair and MLFMA
  workspace reuse before any Krylov storage work. Preserve artifacts. No full
  radiation-pattern cache, Krylov storage edits, commits, or pushes authorized.
- Branch and starting HEAD: `codex/fix_rank6`,
  `9d2a62412f80663d049f5f0488471e5069efba73`; initial working tree clean.
- Files and functions changed:
  - New `src/mlfma_workspace.h/.cpp`: shape/convention-keyed angular grids and
    interpolation tables, actual-team worker pool, reusable interpolation/field
    scratch, capacity accounting, and explicit release.
  - `mvm_Zfardotx.cpp`, `mvm_Zfardotx_in.cpp`,
    `calculate_farfield_mlfma.cpp`: use shared metadata and worker scratch;
    accelerated output staging has local vector owners.
  - `interpolation_c2f.cpp`, `interpolation_f2c.cpp`: caller-scratch overloads;
    existing signatures preserved through local-scratch wrappers.
  - `clear_memory.cpp`: release/null all simulation-owned arrays, including
    material/source/solution and approximate-operator owners, plus workspace.
  - `generate_edgeconnection.cpp`, `calculate_farfield.cpp`: null globals after
    local deletion. `main.h`: static source-count storage with borrowed pointer.
    `generate_octtree.cpp`: clear workspace for a new tree lifetime.
    `calculate_translationoperator.cpp`: remove unused leaked outer temporaries.
  - New `tests/workspace_operators.cpp`, `tests/check_workspace.py`, and
    `tests/README_WORKSPACE.md`; `tests/README.md` links new documentation.
    `tests/check_blocks.py` and `tests/check_inputs.py` include required new
    workspace dependencies in subset link commands. This log and `HANDOFF.md`.
- Changes and rationale: eliminate confirmed lost allocations and missing
  cleanup owners. Reuse angular/trigonometric grids, interpolation stencils, and
  per-worker scratch across levels/calls. Metadata is populated outside parallel
  regions; `single` plus its barrier grows the pool to the actual team size.
  Accurate/approximate shapes and final uniform output sampling have separate
  keys. Worker capacity persists within a simulation and is cleared at tree
  creation/cleanup. Fine-to-coarse reset and output accumulation are unchanged.
- Expected numerical impact: none; same arithmetic order, formulation,
  quadrature, truncation, precision, iteration settings, and file formats.
- Validation plan: archived baseline/current normal and sanitizer ownership/
  repetition checks; existing thread (requested teams 1/2/8/16/32, disabled,
  limited, dynamic), CFIE, and 24-RWG Krylov reference suites; short leak checks;
  up to 100 alternating product pairs and repeated fields; three sequential
  interleaved before/after timing trials. No large sphere/SOM runs.
- Approval received and its scope: user explicitly said “bounded rank-6
  validation is approved”; covers the stated small-fixture plan. Sandbox
  escalation separately allowed the same leak checks/CFIE sanitizer suite
  outside ptrace supervision after environment failures.
- Commands actually executed: exact commands in
  `/tmp/mlfma-rank6-26OJl7kI/COMMANDS.md`. Production flags:
  `g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma`
  (archived prior binaries before the AGENTS `rm run_mlfma` step). Baseline used
  archived `before/src/*.cpp`. Executed `check_cfie.py`,
  `check_thread_parallel.py`, `check_krylov_reference.py`, `check_workspace.py`
  with baseline/current paths and sanitizer variants as recorded. Short leak
  command: `env ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1
  OMP_DYNAMIC=FALSE <workspace_operators> repeat 2 4`, outside ptrace. Executed
  retained `compare-results.py` and `benchmark.py`, Git status/diff/HEAD/branch
  inspection, CRLF-aware diff whitespace checks, Python syntax inspection, and
  source/executable hash checks. Compiled/linked the input/block subset drivers
  only; did not execute those additional suites.
- Build outcome: baseline and final production builds passed. Final executable
  is byte-identical to the validated production build after only whitespace/
  declaration formatting. Both subset driver link checks passed. Existing 38
  warnings unchanged in category/count; no new workspace warnings. Existing
  edited C++ files preserve CRLF; new workspace/test files use LF.
- Test outcome: updated normal and ASan/UBSan ownership, thread, and CFIE checks
  passed. Baseline/current ten-case Krylov reference suites passed in modes
  0/1/2, repeated illuminations, and limit/error cases. All 424 saved normal
  numerical files are byte-identical, including solver status, currents,
  independent products, and fields. Repeated products have zero relative
  difference (threshold 1e-12), and field output repeats identically. Retained
  payload/capacity is stable at 8,045,800 bytes after warmup (32 actual workers,
  seven grids, five tables) and zero after cleanup; partial/repeated cleanup and
  metadata rebuild checks pass. Before: omitted cleanup owners remain populated;
  LeakSanitizer reports 24,312,560 lost bytes in 1,966 allocations. After: same
  short exercise exits 0 with no leak report. Initial baseline workspace and
  current CFIE leak attempts failed due to sandbox ptrace supervision; these
  failed logs remain and the outside-sandbox reruns resolved that limitation.
  Additional input/block suites and large/high-core cases: NOT RUN.
- Measurements and output locations: all artifacts retained at
  `/tmp/mlfma-rank6-26OJl7kI`, including untouched `before/src`, build/run logs,
  `comparison.json`, `timing/summary.json`, `timing/numerical-comparison.json`,
  and `validated-manifest.json`. Three timing trials per side on the unchanged
  24-RWG two-octahedron CFIE fixture at 1 GHz, six digits, interpolation four,
  approximation 0.8, tolerance 0.001, automatic depth. GCC 11.4.0, -O3 C++17
  OpenMP, Intel i7-7700HQ (four cores/eight logical CPUs), OMP_DYNAMIC=FALSE,
  no thread limit. Teams warmed at 1/2/8/32; 100 accurate/approximate pairs at
  eight workers. Median wall seconds before/after 1.20/1.13; repeated section
  0.888264/0.827768; peak RSS KiB 58,804/38,128 (-20.19 MiB, -35.2%). Total
  includes setup/assembly/warmup, pairs, six field outputs/I/O, comparisons, and
  cleanup; repeated section includes pairs and four outputs/I/O/comparisons.
  These are operator-driver measurements, not Krylov solve times. All 30 timing
  output comparisons are byte-identical. Builds/checks finished before timing.
- Remaining uncertainty or pending work: numerical checks cover bounded small
  cases, not production-size performance or throughput beyond 16 cores. Scratch
  retention is bounded per observed shape/team but can raise memory relative to
  a leak-free implementation that frees buffers immediately. Capacity accounting
  excludes allocator overhead and is not RSS. Solver global state still prevents
  overlapping operator/simulation calls. This is not a whole-program leak audit.
  No approved bounded checks pending. Krylov storage remains explicitly deferred;
  large sphere/SOM or expanded tests need separate approval under AGENTS.md.
- Commit/push status: uncommitted and unpushed; no staging/commit/push performed.
  Final `run_mlfma` remains untracked and retained. No artifacts deleted.

### 2026-10-09 12:46 UTC — Rank-6 sphere23 performance follow-up, 31 illuminations

- User request and authorized scope: explicitly requested checking performance
  for `src/test_23_sphere_0p01m.pnu` after rank-6. Run one sequential archived
  baseline/current full-solver pair with the user's current input. Preserve all
  artifacts and the input edit. No source/test-code changes, commits, or pushes.
- Branch and starting HEAD: `codex/fix_rank6`,
  `9d2a62412f80663d049f5f0488471e5069efba73`. Existing rank-6 uncommitted edits
  retained. New pre-existing user edit: source_PW phi sample count 181 -> 31;
  geometry and all other input settings unchanged. Input SHA256
  `ddadfd4a4c8f0f46b0356cae84519e5e197f4f5cb49a3a8f935352328c2aca11`.
- Files and functions changed: documentation only: this log, `HANDOFF.md`, and
  `tests/README_WORKSPACE.md`. Comparison/analysis scripts and retained inputs,
  executables, metadata, logs, measurements and outputs in the unique task
  directory `/tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b`.
- Changes and rationale: measure rank-6 on a larger mesh and multi-illumination
  solve; no additional implementation changes. Baseline archived sources match
  starting HEAD byte for byte; current sources/tests/executable match the
  validated rank-6 manifest. Exact identical input copies used on both sides.
- Expected numerical impact: none; preserve geometry, formulation, quadrature,
  precision, tree/truncation settings, tolerance, and iteration limits.
- Validation plan: full wall time, peak RSS, CPU/stage times, all 31 statuses,
  iterations/products, finite currents/fields/diagnostics, byte comparison of
  result files. Sample process RSS/HWM/thread count between completed
  illuminations for context. One pair; no additional cases or repetitions.
- Approval received and its scope: user's explicit named-input performance-test
  request authorizes this pair under AGENTS section 5. Announced current
  settings, commands, outputs, eight workers, and uncertain 4–6 min/run,
  300–500 MiB estimates before execution. No new permission request needed.
- Commands actually executed: Git status/branch/HEAD and input-diff inspection;
  source/input/binary hash checks; `g++ --version`, `lscpu`; then
  `python3 -B /tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b/run_comparison.py`.
  Each side executed `/usr/bin/time -f 'wall_seconds=%e\npeak_rss_kib=%M\nuser_seconds=%U\nsystem_seconds=%S'
  -o resources.txt <side>/run_mlfma` in its own directory with input.pnu,
  OMP_DYNAMIC=FALSE, OMP_NUM_THREADS=8, and OMP_THREAD_LIMIT unset. Executed
  `python3 -B /tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b/analyze_results.py`;
  final source/input/manifest and diff checks. Full commands and environment in
  the artifact README and side-specific command.json files.
- Build outcome: no rebuild needed. Preserved baseline and validated current
  production executables reused, both previously built with GCC 11.4.0 and
  `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp`; hashes recorded in metadata.
- Test outcome: both exit 0. All 31 illuminations converge. Five result files
  are byte-identical, including RWG coefficients, triangle currents,
  contribution diagnostics, direct monostatic fields, and solver status.
  All numerical values finite; relative L2 differences zero. Both report
  181 outer iterations and 243 Krylov matrix products; maximum physical
  residual `9.93188384084087e-4` satisfies unchanged tolerance `0.001`.
- Measurements and output locations: 12,531 nodes, 25,058 triangles, 37,587 RWGs,
  3 GHz, CFIE alpha 0.5 on both materials, mode 2, six levels, digits 2,
  approximation 0.3, interpolation 2, inner/outer limits 10/100, tolerance
  0.001, theta 90 degrees, phi 0..360 in 31 samples, polarization 0, dir_ff 0.
  Intel i7-7700HQ, four physical/eight logical CPUs; eight active OpenMP workers
  observed. Sequential runs, no concurrent agent build/solver workloads. Full
  wall seconds before/after `238.24/234.44` (-1.60%); solution-stage seconds
  `237.126272/233.295821` (-1.62%); peak RSS KiB `301120/287540`
  (-13,580 KiB = -13.26 MiB, -4.51%). Near assembly `0.828842/0.858361 s`;
  translation `0.043816/0.044502 s`. Full time includes setup/assembly, all
  solves, direct field output, final current diagnostics, and cleanup.
  Sampled HWM from first to 30th completed illumination: before
  `288028 -> 301120 KiB`; after `287468 -> 287540 KiB`. Samples are not a leak
  rate measurement; allocator/solve phases affect RSS. Authoritative peaks
  are from /usr/bin/time. All inputs, binaries, scripts, outputs and logs
  retained under `/tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b`;
  `summary.json` contains numerical/resource/stage comparisons and hashes.
- Remaining uncertainty or pending work: one pair does not establish a reliable
  1.60% runtime gain or isolate its cause. Lower/flatter measured memory is
  consistent with removing previously confirmed repeated allocation leaks.
  Current case uses direct far fields; accelerated-output performance, SOM,
  the original 181-illumination sweep, other modes and high-core throughput
  were not measured in this follow-up. No checks from this pair remain pending.
- Commit/push status: existing rank-6 changes and new documentation remain
  uncommitted/unpushed. User input edit preserved unchanged; all artifacts kept.

### 2026-10-09 13:05 UTC — Rank-6 sphere23 mode-1 performance and residual follow-up

- User request and authorized scope: explicitly requested repeating the sphere
  test after changing Preconditioners to 1. One sequential archived/current
  full-solver pair at the current 31-illumination settings, plus offline
  comparison with retained mode-2 measurements. Preserve all artifacts/input
  edits; no solver/test-code changes, commits or pushes.
- Branch and starting HEAD: `codex/fix_rank6`,
  `9d2a62412f80663d049f5f0488471e5069efba73`. Existing rank-6 edits retained.
  User input changes relative to HEAD: PW phi count 181 -> 31 and
  Preconditioners 2 -> 1; only the latter differs from the preceding test.
- Input SHA256: `e2dca430b45e5c76eebb5448ce427917c8c5e423628ac240f61def20c9c65e8e`.
- Files and functions changed: documentation only: this log, `HANDOFF.md`, and
  `tests/README_WORKSPACE.md`. Input, binaries, metadata, initial input diff,
  run/analysis scripts, logs, resources, samples and outputs retained in
  `/tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1`.
- Changes and rationale: extend rank-6 performance evidence to mode 1 and
  distinguish numerical agreement from satisfying the physical tolerance.
  Both executables exactly match those used in the preceding mode-2 test;
  current source/test/executable hashes still match the validated manifest.
- Expected numerical impact: none within this baseline/current comparison;
  all numerical settings, arithmetic, formulations and stopping criteria kept.
  Mode 1 uses the existing pivoted LU blocks and left-preconditioned GMRES;
  approximate mode-2 operator/inner solve is not used.
- Validation plan: full wall/CPU time, peak RSS, setup/solution stages,
  block sizes, statuses and physical residuals for all 31 illuminations,
  iterations/products, finite outputs and result-file byte comparisons.
  Record nonconvergence honestly; finish both sides even if solver returns 1.
  Compare retained mode-2 resources without rerunning that case.
- Approval received and its scope: user's explicit mode-1 repeat request
  authorizes this named-input pair under AGENTS section 5. Announced eight
  workers, unchanged settings, sequential runs and uncertain several-minute,
  250–600 MiB estimates before execution. No extra permission required.
- Commands actually executed: Git status/branch/HEAD/input diff and manifest/
  prior-binary hash inspection; `lscpu`, `g++ --version`; then
  `python3 -B /tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1/run_comparison.py`.
  Each side used `/usr/bin/time -f 'wall_seconds=%e\npeak_rss_kib=%M\nuser_seconds=%U\nsystem_seconds=%S'
  -o resources.txt <side>/run_mlfma` with identical input.pnu copies,
  OMP_DYNAMIC=FALSE, OMP_NUM_THREADS=8, and OMP_THREAD_LIMIT unset. Executed
  `python3 -B /tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1/analyze_results.py`.
  Final source/input hash and whitespace/diff checks. Full commands/environment
  in artifact README and side command.json files.
- Build outcome: existing verified baseline/current production executables
  reused; no rebuild or source change. GCC 11.4.0 with
  `-Wall -fexceptions -O3 -std=gnu++17 -fopenmp` on both sides.
- Test outcome: both simulations completed all 31 illuminations and returned
  exit 1. All statuses are `physical_residual_not_converged`; none meets the
  physical tolerance 0.001. GMRES reports convergence for the preconditioned
  equation, but physical residuals range from `1.369482532608186e-3` to
  `1.723941814623232e-3`. This existing behavior is identical before/after:
  all five result files (currents, fields, contribution diagnostics, solver
  status) are byte-identical; all values finite, relative L2 differences zero;
  both report 648 iterations and 710 Krylov matrix products. Both construct
  4,312 pivoted LU blocks with 417,495 complex coefficients. Numerical identity
  checks pass; physical convergence validation FAILS on both sides.
- Measurements and output locations: 12,531 nodes, 25,058 triangles, 37,587 RWGs,
  3 GHz, alpha 0.5 on both materials, mode 1, six levels, digits 2,
  interpolation 2, retained val_approx 0.3 / inner bound 10, outer bound 100,
  tolerance 0.001, polarization 0, theta 90, phi 0..360 in 31 samples, dir_ff 0.
  Intel i7-7700HQ (four physical/eight logical CPUs); eight active workers
  sampled. Sequential runs without concurrent agent compilation/solver loads.
  Full wall seconds before/after `339.18/333.36` (-1.72%); solution seconds
  `338.016582/332.210418` (-1.72%); peak RSS KiB `235016/226780`
  (-8,236 KiB = -8.04 MiB, -3.50%). Near assembly `0.863478/0.822401 s`;
  translation `0.042142/0.041153 s`. Total includes setup/assembly/block LU,
  all solves/direct fields, final current diagnostics and cleanup. Sampled HWM
  grows from 226548 to 235016 KiB in the baseline; updated stays 226780 KiB
  from first to 30th completed illumination. Samples are not a leak-rate test.
  Authoritative peaks are from /usr/bin/time. All artifacts retained in
  `/tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1`, including summary.json.
  Current mode 1 versus previously measured current mode 2: wall
  `333.36/234.44 s` (+42.19%), peak RSS `226780/287540 KiB` (-21.13%).
  Mode 2 converged for all 31 illuminations; mode 1 converged physically for
  none, so this cross-mode comparison is not at equal achieved accuracy.
  The reported mode-2 outer Krylov product count excludes its inner solves.
- Remaining uncertainty or pending work: one pair cannot establish a reliable
  speedup or isolate timing causes. Memory is lower/flatter, consistent with
  prior leak findings. Existing mode-1 criterion checks
  ||M^-1(b-Ax)||/||M^-1 b||, then stage_solution separately checks
  ||b-Ax||/||b|| and flags failure without continuing the solve. This explains
  the distinct residual statuses; inspect src/mvm.cpp, src/krylov.cpp and
  src/stage_solution.cpp. No stopping-criterion/tolerance fix was authorized
  or applied. All requested runs completed; physical convergence remains an
  unresolved mode-1 limitation. No extra cases/repetitions or large sanitizers.
- Commit/push status: uncommitted/unpushed; no staging/commit/push performed.
  Both user input edits preserved unchanged. All artifacts retained.

### 2026-10-09 13:11 UTC — Commit rank-6 ownership and workspace changes

- User request and authorized scope: commit rank-6 code and provide a short PR
  description. Include focused tests and validation notes; preserve the user's
  sphere input edits and every artifact. No push or remote PR creation requested.
- Branch and starting HEAD: `codex/fix_rank6`,
  `9d2a62412f80663d049f5f0488471e5069efba73`.
- Files and functions changed: commit-status wording in HANDOFF.md and this log;
  no implementation/test-code changes during commit preparation. The commit
  includes the previously reviewed rank-6 source/tests/documentation and both
  sphere performance follow-ups. Exclude src/test_23_sphere_0p01m.pnu and
  the untracked run_mlfma executable.
- Changes and rationale: record ownership repair, immutable sampling/stencil
  reuse, actual-team scratch ownership and bounded lifetime in one reviewable
  commit. Keep Krylov storage and stopping criteria unchanged.
- Expected numerical impact: none beyond the previously validated change.
- Validation plan: inspect status/diffs, source/test/executable hashes against
  retained validation manifest, hooks, staged file scope and staged patch;
  then commit the reviewed files on the current branch.
- Approval received and its scope: user's current request explicitly authorizes
  staging and committing. No push authorized. No custom hooks path or active
  Git hooks were found; only sample hooks are present.
- Commands actually executed: Git status/branch/HEAD/config/hooks/diff review,
  source/test/executable manifest comparison and retained-results inspection;
  CRLF-aware whitespace checks. Scoped Git staging, staged-diff inspection and
  commit are the operations accompanying this entry.
- Build outcome: existing successful production build reused; source/test/binary
  hashes match the validated manifest. No rebuild needed.
- Test outcome: no tests rerun for commit preparation. Existing normal and
  sanitizer regression results retained. Numerical outputs match before/after;
  mode-2 larger sphere converges, mode-1 larger sphere identically fails physical
  tolerance on both sides, as documented. No new numerical validation claimed.
- Measurements and output locations: /tmp/mlfma-rank6-26OJl7kI,
  /tmp/mlfma-rank6-sphere23-before-after-9Rs5LV7b, and
  /tmp/mlfma-rank6-sphere23-pc1-before-after-z6ZFb6C1; all retained.
- Remaining uncertainty or pending work: mode-1 physical convergence limitation,
  reliable production timing gains, high-core throughput and Krylov storage
  remain separate work. Preserve both user input edits (31 samples, mode 1).
- Commit/push status: this entry accompanies the requested rank-6 commit.
  No push requested/performed. User input edits remain uncommitted; executable
  and temporary artifacts are preserved outside the commit.
