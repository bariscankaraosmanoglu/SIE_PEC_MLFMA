#include "krylov.h"
#include <algorithm>
#include <cstdio>
#include <limits>

using namespace std;
extern int val_pc;
extern int val_itermax_pc;

void pc_solution(complex<double>* b, complex<double>* x, int n) {
    if (val_pc != 2) {
        copy_n(b, n, x);
        return;
    }
    fill_n(x, n, complex<double>{});
    const KrylovResult result = gmres_solve(n, b, val_itermax_pc, 0.1, x, 1);
    // A limited, finite inner solve is a valid variable preconditioner for FGMRES.
    if (result.status == KrylovStatus::Converged || result.status == KrylovStatus::IterationLimit) return;
    fprintf(stderr, "Inner GMRES failed: %s, iterations=%d, true relative residual=%.6e\n",
            krylov_status_name(result.status), result.iterations, result.relative_residual);
    // The legacy callback cannot return a status; the outer solver detects this marker
    // before an operator application and retains its last verified finite iterate.
    fill_n(x, n, complex<double>{numeric_limits<double>::quiet_NaN(), 0.0});
}
