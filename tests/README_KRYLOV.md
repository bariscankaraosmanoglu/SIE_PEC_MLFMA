# Krylov correctness and termination checks

Rank-3 repairs the GMRES/FGMRES arithmetic and makes solver termination observable.
The existing Householder GMRES method is retained. `fgmres` and `fgmres_in`
share the flexible Arnoldi implementation in `src/krylov.cpp`.

`src/krylov.h` defines the result/status API. Compatibility `int` entry points
return zero only on verified convergence; other values are `KrylovStatus` codes.
`KrylovResult` includes completed iterations, matrix products, and the freshly
evaluated residual of the returned iterate. With zero RHS, residuals and the
tolerance are absolute. Invalid inputs return an unavailable residual (infinity).
On arithmetic failure, the previous iterate with a checked finite residual is
retained, when one exists.

The complex rotation uses
`U = [conj(c), conj(s); -s, c]`, with `c=a/hypot(abs(a),abs(b))` and
`s=b/hypot(abs(a),abs(b))`. It is unitary and annihilates the subdiagonal,
including the legacy failure where `a*a + b*b == 0` despite a nonzero pair.
The zero pair uses the identity rotation and goes through guarded back substitution.
Scaled norms, finite checks, checked workspace size products, and owned vectors
cover overflow, bounds, and cleanup. FGMRES uses a second orthogonalization pass
when the first pass loses at least half the vector norm. Both solvers stop on a
tail at most `16*epsilon*||A*v||`, then distinguish convergence from breakdown
using the actual residual. Back substitution rejects pivots at or below
`epsilon` times the remaining row scale and can retain a usable leading subspace.
These conservative guards can report breakdown on very ill-conditioned problems.

`pc_solution` accepts a finite iteration-limited inner GMRES result as a variable
preconditioner. Other inner failures print their status and signal a nonfinite
callback result; outer FGMRES detects it before applying the operator and retains
its last verified iterate. FGMRES's `Z` storage now uses the restart dimension,
bounded by `min(restart, itermax, n)`, rather than total iteration budget.

`stage_solution` saves the original RHS and checks it against `mvm_physical`
(near plus accurate far, without left preconditioning). Every illumination writes
`result_solver_status.txt`, containing overall status, Krylov status, completed
iterations, Krylov matrix-product count, Krylov true residual, physical true
residual, and tolerance. The separate physical check adds one matrix product;
it is excluded from the Krylov product count. Inner products are also excluded
from the outer count. A left-preconditioned solve that passes its own criterion
but fails the physical criterion reports `physical_residual_not_converged`.
The executable exits with status 1 if any illumination fails. Finite approximate
coefficients and fields are retained alongside that status file for diagnosis.
Iteration progress explicitly labels the recurrence residual as an estimate.

The stage's RHS history uses owned vectors, an unused allocation was removed,
and each illumination's RHS is released after use. Initial guesses guard zero
previous RHS entries. General solver/global cleanup outside this stage remains
outside the change.

Run from the repository root:

```sh
python3 -B tests/check_krylov.py
python3 -B tests/check_krylov.py --sanitize
python3 -B tests/check_krylov.py --sanitize --leaks
```

All commands retain binaries, inputs, status files, and logs in their printed
artifact directory. `--output-dir` selects it explicitly. LeakSanitizer requires
an environment without ptrace; in this workspace the leak checks ran outside
the sandbox. Whole-solver sanitizer suites still disable leak detection because
of existing allocations outside this fix; the isolated Krylov and stage drivers
pass with leak detection enabled.

The dense driver checks complex cancellation, one unknown, full dimension,
zero RHS, nonzero and exact initial guesses, happy and unsuccessful breakdown,
singular consistent/inconsistent systems, restart and budget limits, invalid
arguments, nonfinite operators/preconditioners, variable preconditioning,
nonnormal matrices, unattainable tolerances, and scales from `1e-300` to `1e300`.
The preconditioner driver uses the production nested solve. The stage driver
checks production status plumbing and a deliberately misleading left-weighted
residual: Krylov residual `1.414e-6` versus physical residual `0.999999` at
tolerance `0.001`. It must report failure, and does.

Full reference integration reuses fixtures and the independent operator driver
from `check_cfie.py`:

```sh
rank3_artifacts=/tmp/mlfma-rank3/implementation-52f49r1l
python3 -B tests/check_krylov_reference.py \
  --solver run_mlfma \
  --operator-driver "$rank3_artifacts/cfie/cfie_operators" \
  --reference-input "$rank3_artifacts/cfie/reference/input.pnu" \
  --output-dir /tmp/krylov-reference-new-run
```

It checks modes 0, 1, and 2, repeated illuminations, limited outer solves,
limited inner solves, and invalid inner parameters. Exported coefficients are
independently applied to the accurate unpreconditioned operator and, for this
small fixture, the direct MoM matrix. Successful reference residuals:

| Preconditioner | Physical MLFMA residual | Direct MoM residual |
| --- | ---: | ---: |
| 0 | 0.000234871 | 0.000258225 |
| 1 | 0.000954788 | 0.000988028 |
| 2 | 0.000225538 | 0.000235476 |

All are within the input tolerance `0.001`. The larger coarse CFIE fixtures
still have their pre-existing MLFMA/direct-MoM discrepancies (about 15% for
one product); they check Krylov convergence against the implemented operator,
not that unrelated approximation error.

Baseline sources, tests, and executable were saved before editing. ASan
confirmed GMRES's read of `v[n]` on one unknown. The legacy FGMRES probe on
`[[i,1],[1,i]]`, RHS `[1,0]`, produced NaNs; the corrected solve has residual
zero. Legacy GMRES returned success with residual `0.707107` at a one-step
limit; both corrected methods report `iteration_limit` at that residual.
Legacy restart zero hung; the corrected method reports `invalid_input`.
Original probes remain in `/tmp/mlfma-rank3/baseline`, and repeated probes,
snapshots, reference results, and benchmarks are in the artifact directory above.

The AGENTS.md rebuild commands were followed, after copying each existing
executable into the retained artifact directory:

```sh
rm run_mlfma
g++ -Wall -fexceptions -O3 -std=gnu++17 -fopenmp src/*.cpp -o run_mlfma
```

Additional validation commands executed (with output directories under the
artifact directory):

```sh
python3 -B tests/check_cfie.py --full-solver "$PWD/run_mlfma"
python3 -B tests/check_cfie.py --sanitize
python3 -B tests/check_thread_parallel.py --input src/test_29_sphere_0p03m.pnu
python3 -B tests/check_thread_parallel.py --sanitize --input src/test_29_sphere_0p03m.pnu
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

All passed, including isolated ASan/UBSan/LSan. Existing compiler warnings
outside the changed solver files remain. Modified existing sources retain CRLF.
Accurate/approximate operators, the near matrix, and the thread fixture far
fields remain byte-identical to the saved rank-2 operator baseline. Requested
teams through 32, a two-worker limit, and dynamic adjustment pass.

Compared with the saved rank-3 full-solver baseline, maximum coefficient
relative L2 change among the reference/open/uniform cases is `5.654e-8`, and
maximum complex far-field relative L2 change is `4.620e-8`, both below `0.001`.
The local 4,080-RWG sphere's coefficient change is `4.372e-14` and its checked
physical residual is `0.000868663`.

Three sequential before/after trials, interleaved by version, used
`/usr/bin/time -f '%e %M' -o resources.txt <executable>` with
`OMP_DYNAMIC=FALSE`. Median full-process results:

| Case | Seconds before / after | Peak RSS KiB before / after |
| --- | ---: | ---: |
| Uniform 192-RWG sphere, GMRES, 1 worker | 1.08 / 1.21 | 15,608 / 15,844 |
| Mixed 24-RWG reference, nested FGMRES, 1 worker | 0.17 / 0.20 | 10,628 / 10,752 |
| Local 4,080-RWG sphere, GMRES, 8 workers | 1.46 / 1.58 | 33,492 / 33,596 |
| Dense identity, n=20,000, restart=8, budget=1,000 | 0.15 / <0.01 | 320,312 / 9,972 |

The added residual products impose a measurable cost on these small solver
cases; this is principally a correctness change. The memory benchmark isolates
bounded restart allocation and does not establish a comparable whole-MLFMA speedup.
Its legacy identity solution was already accurate (maximum error `1.02e-13`),
and the corrected solution's maximum error is zero. Production still sets
restart equal to the outer iteration budget; smaller restart sizes are supported
by the solver API but have not been added as a new input option.
