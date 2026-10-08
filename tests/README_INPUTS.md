# Input and dimension validation

The PEC executable keeps the shared `input.pnu` parameter names and mode
identifiers. `Formulations` and `source_type` remain in the file. This executable
supports formulation 0, plane-wave source 0, and a single frequency. Selecting
an unsupported formulation, source, or frequency sweep exits with status 1 and
an `Input error` diagnostic instead of silently running a different problem.
With frequency count 1, differing endpoints remain accepted: the start frequency
is used and the retained endpoint is identified in the log.

## Implementation

- `read_simulationinput.cpp::read_simulationinput` parses complete records,
  verifies labels and numeric conversions, validates finite values and ranges,
  and publishes mesh arrays only after parsing succeeds. Point/material indices,
  repeated vertices, zero or unrepresentable triangle areas, duplicate triangles,
  material count (1–50), and name lengths (199 bytes maximum) are checked. Named
  metadata before mesh sections and after the cells remains supported.
- `input_validation.cpp::validate_mlfma_dimensions` preserves the existing
  truncation formula while checking conversions, sample counts, and the
  interpolation stencils actually used by the requested operator/field paths.
  Mode 2 accepts `0 <= val_approx <= 1`; zero-order approximate grids remain valid
  where interpolation does not require more samples. Inactive approximation and
  inner iteration settings are retained rather than constrained as active ones.
- `generate_octtree` rejects meshes without full RWGs and bounds Morton/tree
  indices. The current signed-index implementation supports up to 20 levels on
  this 64-bit Linux build. This is an index bound, not a CPU count restriction.
- `generate_Znearcomponent`, `generate_Zfarcomponents`, and
  `generate_translationcomponent` check products and accumulated array sizes
  before overflowing their existing integer indices. Near setup uses an owned
  temporary vector instead of a potentially large stack array.
- `interpolationweights` checks stencil/workspace sizes before indexing and
  bounds its location scan before reading the extended grid.
- `validate_farfield_mode` retains cuts, backscatter, and accelerated-grid
  selectors. For direct grids (`dir_ff >= 5`), it rejects steps whose existing
  allocation is smaller than the number of executed samples. For example, 7
  is rejected; 5 and 7.5 remain supported. The direct-field algorithm is unchanged.

No quadrature, tolerance, physical coefficient, solver, or truncation algorithm
has changed. Threads are not capped at 16. Validation is limited to conditions
required by the current implementation; it is not a general mesh-quality audit
or a guarantee that every representable problem fits available RAM.

## Reproduction

Run from the repository root, with test execution authorized under `AGENTS.md`:

```sh
python3 -B tests/check_inputs.py --output-dir /tmp/chosen-inputs
python3 -B tests/check_inputs.py --sanitize --output-dir /tmp/chosen-inputs-sanitize
```

Each suite runs 99 cases: 14 accepted boundary/compatibility cases and 85 expected
rejections. The driver uses the production reader and tree setup. Separate
synthetic probes exercise the actual interpolation, near-matrix, aggregation,
and translation dimension code without allocating production-size matrices.
Sanitizers cover addresses, undefined behavior, and floating-to-integer overflow.
Leak detection is disabled for these drivers because global solver storage has
existing lifetime issues outside this change.

To collect a baseline from an archived source directory, add
`--source-dir /tmp/archive/src --baseline`; failures are recorded without being
treated as unexpected. `--baseline-dir /tmp/saved-baseline` compares accepted
mesh/material arrays byte for byte where a baseline artifact exists. Every
input, binary, log, and result is retained. Use a fresh output directory for
independent runs.

## Confirmed findings and validation

Artifacts for the rank-4 implementation are in
`/tmp/mlfma-rank4/implementation-xjqpan3a`, including untouched source/test
snapshots and the baseline executable under `before/`.

The baseline reader accepted malformed labels, unsupported formulations, and
frequency sweeps. Sanitizer probes confirmed a long-name buffer overflow,
material-table overflow at 51 parts, invalid-stencil access, near-block integer
overflow, and translation-total integer overflow. The full baseline solver
accepted an impedance selection and a frequency sweep, solving the wrong
supported problem. It aborted with heap corruption for `dir_ff = 7`.
These cases now report explicit input errors. Other capacity guards follow
from the existing array/index arithmetic; not every limit was reproduced using
an enormous physical mesh.

Both final 99-case input suites passed (`inputs-final-v2` and
`inputs-final-sanitize-v2`). Existing CFIE and thread operator suites passed in
ordinary and sanitizer configurations. Thread checks requested 1, 2, 8, 16, and
32 workers, also exercising disabled, limited, and dynamic teams; they matched
the archived serial operator outputs. The CFIE topology leak checks initially
failed because LeakSanitizer cannot operate under sandbox ptrace, then passed
when the same suite was rerun outside that restriction (`cfie-sanitize-reviewed`).
This does not establish a clean whole-program leak audit.

The final full-solver Krylov reference suite passed for preconditioners 0, 1,
and 2, multiple illuminations, iteration limits, and early invalid-inner-limit
rejection (`reference-final-v2`). Converged reference physical residuals were
approximately `2.35e-4`, `9.55e-4`, and `2.26e-4`, below the unchanged `1e-3`
tolerance. Full closed/open cases and safe zero-order approximation cases had
byte-identical result text versus baseline (`final/`, `zero-order-full/`, and
`final-comparisons.json`). All three local meshes, including SOM, produced
byte-identical parsed arrays (`large-inputs/`). No full SOM solve was run.

The final production build followed `AGENTS.md` after archiving each existing
executable:

```sh
rm run_mlfma
g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma
```

It succeeded; existing warnings remain (`final-v2-build.log`).

## Measured cost

Three interleaved baseline/current trials used `/usr/bin/time -f '%e %M'`.
The host is an Intel i7-7700HQ with eight logical CPUs. These are small-case
measurements, not evidence of a speed improvement. One sphere trial overlapped
small reference validation work, increasing timing uncertainty.

| Scope | Median seconds before / after | Median peak RSS KiB before / after |
| --- | --- | --- |
| Sphere, full setup/solve/field, 4,080 RWGs, 8 workers | 1.70 / 1.62 | 33,432 / 33,484 |
| Closed reference, full setup/solve/field, mode 2, 1 worker | 0.19 / 0.18 | 10,728 / 10,748 |
| SOM, parsing and binary dump only, 183,842 triangles | 0.11 / 0.30 | 8,872 / 20,448 |

The sphere uses 1 GHz, two accuracy digits, interpolation width 2, mode 0,
and tolerance `0.001`; its 26 iterations and results are unchanged. The closed
reference uses 1 GHz, six digits, width 4, approximation 0.8, and tolerance
`0.001`; its three iterations and results are unchanged. SOM is parsed with
its original 5 GHz parameters; no assembly or solve is included. Validation
adds parsing work and temporary duplicate-triangle storage, which is released
before assembly. Raw resources and comparison data are under `benchmarks/`.

## Additional full-sphere comparison

An additional user-requested full before/after pair used the unchanged local
`src/test_23_sphere_0p01m.pnu`: approximately 0.3 m sphere radius, 25,058 triangles,
37,587 RWGs, 3 GHz, all 181 plane-wave illuminations, eight requested workers,
preconditioner 2, explicit six levels, two digits, approximation 0.3, interpolation
width 2, inner/outer limits 10/100, tolerance `0.001`, and backscatter selector 0.
Both archived/current production executables exited 0; all five result files
were byte-identical. Both reported 727 outer iterations, and the maximum physical
residual was `9.33977e-4`, below the unchanged tolerance.

| Full sphere23 pair | Before | After |
| --- | --- | --- |
| Wall seconds, setup/solve/output | 1,099.11 | 1,128.97 |
| Peak RSS KiB | 352,796 | 353,564 |
| Reported solution-stage seconds | 1,097.840165 | 1,127.691979 |

This single pair observed +2.72% wall time and +0.22% peak RSS (0.75 MiB).
Almost all of the timing difference was in the solution stage with unchanged
reported numerical work; its cause is not established by this pair. Artifacts,
exact commands, host/compiler metadata, hashes, and comparisons are retained
at `/tmp/mlfma-rank4/sphere23-before-after-8mm2uv5c`. The proposed parser
optimizations were not applied, and no repeated full runs or full SOM solve
were performed for this additional comparison.

## Remaining limits

Other executables' impedance and source implementations have not been examined.
Their identifiers are preserved without claiming numerical support here.
Orientation consistency, self-intersections, near-degenerate mesh quality, and
the unrelated matrix-inversion compiler warnings remain outside this change.
More-core throughput scaling has not been measured on this eight-CPU host.
