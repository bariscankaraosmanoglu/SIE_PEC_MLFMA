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
