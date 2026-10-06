#include <complex>

using namespace std;

extern int num_rwgs;
extern int val_pc;

void mvm_Zneardotx(complex<double> x[], complex<double> y[]);
void mvm_Zfardotx_in(complex<double> x[], complex<double> y[]);
void mvm_BDPmatrix(std::complex<double> x[], std::complex<double> y[]);

void mvm_in(complex<double> x[], complex<double> y[]){

    mvm_Zneardotx(x,y);
    mvm_Zfardotx_in(x,y);
    if (val_pc==1){mvm_BDPmatrix(y, y);}

}
