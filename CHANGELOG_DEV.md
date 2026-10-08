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
