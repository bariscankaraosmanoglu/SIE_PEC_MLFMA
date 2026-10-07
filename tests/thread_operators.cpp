// Operator regression on a deterministic vector, independent of the Krylov solvers.
#include "main.h"
#include <cassert>
#include <omp.h>
#include <vector>
#include <cmath>
#include <cstdlib>
void mvm_Zneardotx(complex<double>*, complex<double>*);
void mvm_Zfardotx(complex<double>*, complex<double>*);
void mvm_Zfardotx_in(complex<double>*, complex<double>*);
void calculate_farfield_mlfma(int,complex<double>*);
static void dump(const char* name,const complex<double>* values,size_t count){
    FILE* f=fopen(name,"wb"); if(!f || fwrite(values,sizeof(*values),count,f)!=count) exit(2); fclose(f);
}
int main(int argc,char** argv){
    if(argc!=3) return 2;
    read_simulationinput(); parallel_omp_on=atoi(argv[1]); num_threads=atoi(argv[2]);
    generate_edgeconnection(); generate_octtree(); generate_nearfarboxes();
    generate_Znearcomponent(); generate_Zfarcomponents(); generate_translationcomponent();
    calculate_loadbalance(); calculate_Znearmatrix(); calculate_translationoperator();
    aggregation=new complex<double>[2*size_agg]; disaggregation=new complex<double>[2*size_agg];
    vector<complex<double>> x(num_rwgs), near(num_rwgs), reference(num_rwgs), far(num_rwgs), approximate(num_rwgs);
    for(int j=0;j<num_rwgs;++j) x[j]={sin(0.13*j),cos(0.17*j)};
    mvm_Zneardotx(x.data(),near.data());
    int actual_team=0;
    #pragma omp parallel if(parallel_omp_on)
    {
        #pragma omp single
        actual_team=omp_get_num_threads();
    }
    printf("CHECK actual_team %d requested %d\n",actual_team,num_threads);
    int start=num_filledboxes[2*num_mlfmalevels-1], end=num_filledboxes[3*num_mlfmalevels-1]+1;
    for(int b=start;b<end;++b) for(int n=0;n<27;++n){
        int c=boxes_near[b*27+n]; if(c<0) continue;
        long int offset=nearindex[(b-start)*27+n];
        for(int row=children[2*b];row<=children[2*b+1];++row){
            complex<double> sum{};
            for(int col=children[2*c];col<=children[2*c+1];++col) sum+=matrix_Znear[offset++]*x[rwg_ind[col]];
            reference[rwg_ind[row]]+=sum;
        }
    }
    double error=0,norm=0;
    for(int j=0;j<num_rwgs;++j){error+=std::norm(near[j]-reference[j]); norm+=std::norm(reference[j]);}
    printf("CHECK near_relative_error %.17g\n",sqrt(error/norm));
    mvm_Zfardotx(x.data(),far.data());
    if(val_pc==2) mvm_Zfardotx_in(x.data(),approximate.data());
    dump("near_matrix.bin",matrix_Znear,size_Znear); dump("near.bin",near.data(),num_rwgs);
    dump("far.bin",far.data(),num_rwgs); dump("approximate.bin",approximate.data(),num_rwgs);
    calculate_farfield_mlfma(0,x.data());
    return sqrt(error/norm)<1e-12?0:1;
}
