# Angular, interpolation, and child shift caches

Rank 7 extends the rank-6 workspace. `calculate_translationoperator()` now
borrows the existing angular locations, weights, and trigonometric tables.
Interpolation caching already covered accurate and approximate operators and
the distinct uniform far-field output grid; its construction is unchanged.

`MlfmaWorkspace::child_shifts()` retains eight upward phase vectors for each
required angular shape, sampling convention, parent width, and real wavenumber.
The bit positions x/y/z identify the positive halves of an octant. The
displacement is `(+-h/4, +-h/4, +-h/4)` for parent width `h`, and the upward
phase is `exp(-i*k*dot(displacement,khat))`. Downward shifts use the conjugate.
Both polarizations use the same phase. Approximate operators share a table
with accurate operators only when all keys match.

Tables are created before parallel regions; workers only read them. No per-box,
per-RWG, or per-worker phase vectors are retained. One table's complex payload
costs `8 * Ntheta * Nphi * sizeof(complex<double>)` bytes (128 bytes per angular
direction with the existing complex doubles), plus small map/metadata overhead.
Only used level/grid combinations are prepared. `clear_memory()` and new tree
construction release them through the existing workspace lifetime. Keys also
prevent accidental reuse when width, frequency, or sampling changes.

Child selection uses the signs of the actual child-minus-parent displacement,
not the position in the occupied-child list. Missing octants therefore require
no special ordering. Center subtraction may differ from the ideal offset due
to floating-point rounding. `ChildShiftTable::for_shift()` bounds the resulting
phase-argument error by `k * sum(abs(actual_offset - canonical_offset))`.
It uses the cache only when this is at most 64 double epsilons in radians.
Otherwise the caller uses its original dot product and complex exponential.
Zero/nonfinite components also select this direct fallback. This protects
strongly translated coordinates from an incorrect canonical-offset assumption.
The bound describes displacement rounding; evaluation of dot products and
exponentials also incurs ordinary floating-point rounding.

Leaf radiation/receiving patterns, near/far partitioning, translation formulas,
interpolation order, accumulation order, quadrature, truncation, solver settings,
and precision are unchanged. Outputs can differ slightly from rank 6 because
canonical shifts replace rounded center subtraction. Whole-solver global state
is still sequential and nonreentrant. Full radiation-pattern caching and Krylov
storage changes remain outside this work.

## Bounded checks

Execute only with the approval required by root `AGENTS.md`. Keep output paths
fresh and retain all artifacts. Focused commands are:

```sh
python3 -B tests/check_angular_cache.py --output-dir /tmp/chosen-angular
python3 -B tests/check_angular_cache.py --sanitize --output-dir /tmp/chosen-angular-san
```

The focused check independently evaluates both phase signs and all eight
octants, sparse translated children, center-cancellation fallback, grid/width/
wavenumber/sampling keys, stencil construction, shared reads with 32 requested
workers, invalid keys, cleanup, and rebuilding. No solver geometry is required.
The phase comparison threshold is `1e-14`.

For ptrace environments where LSan fails, retain the failure and rerun the built
check outside ptrace with the necessary sandbox permission, or use `--skip-leaks`
for ASan/UBSan and arrange a separate approved leak check. A ptrace diagnostic
does not count as a passing leak check.

The existing `check_workspace.py` fixture additionally checks phase lookup
against actual tree offsets, reports hits/fallbacks, and checks repeated
accurate/approximate products, accelerated fields, retained capacity, and
cleanup. Its template-based diagnostic also compiles against the archived
rank-6 workspace, allowing unchanged baseline products to be recorded.

Broader checks use the existing CFIE suite, thread suite, and full Krylov
reference suite. Before/after operator comparisons retain the existing `1e-12`
relative L2 threshold; independently verified physical residuals must satisfy
the unchanged input tolerance. See the development log and handoff for the
approved commands, actual outcomes, resource measurements, and retained paths.
