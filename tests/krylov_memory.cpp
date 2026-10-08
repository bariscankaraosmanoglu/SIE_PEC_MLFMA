// Bounded-restart allocation benchmark; can also link against the saved legacy sources.
#include <algorithm>
#include <complex>
#include <cstdio>
#include <vector>
using C=std::complex<double>;
int parallel_omp_on=0, val_pc=0;
static constexpr int n=20000;
void mvm(C* x,C* y) { std::copy_n(x,n,y); }
void mvm_in(C* x,C* y) { mvm(x,y); }
void pc_solution(C* b,C* x,int count) { std::copy_n(b,count,x); }
int fgmres(int,C*,int,int,double,C*);
int main() {
    std::vector<C> b(n,1.0), x(n);
    int status=fgmres(n,b.data(),1000,8,1e-12,x.data());
    double residual=0;
    for (int i=0;i<n;++i) residual=std::max(residual,std::abs(b[i]-x[i]));
    std::printf("CHECK memory_benchmark return=%d max_error=%.17g\n",status,residual);
    return residual<=1e-12 ? 0 : 1;
}
