// Synthetic self blocks test the production builder, RWG maps, and cleanup.
#include "main.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
void mvm_BDPmatrix(std::complex<double>*,std::complex<double>*);
using Values = std::vector<std::complex<double>>;
static double error(const Values& expected,const Values& actual) {
    double d=0,s=0;
    for(std::size_t i=0;i<expected.size();++i) {
        if (!std::isfinite(std::abs(actual[i]))) return INFINITY;
        d=std::max(d,std::abs(actual[i]-expected[i]));s=std::max(s,std::abs(expected[i]));
    }
    return d/(s ? s : 1);
}
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    const std::string mode=argv[1];
    val_pc=mode=="mode0" ? 0 : mode=="mode2" ? 2 : 1;
    num_rwgs=6;num_mlfmalevels=1;
    int counts[3]={3,0,2},child_counts[3]={3,2,1},ranges[6]={0,2,3,4,5,5};
    int order[6]={2,0,4,1,3,5},neighbors[81];long offsets[81]{};
    std::fill_n(neighbors,81,-1);neighbors[0]=0;neighbors[27]=1;neighbors[54]=2;
    offsets[0]=0;offsets[27]=9;offsets[54]=13;
    Values matrix={1,1,0,1,1,1,0,1,1, std::complex<double>(2,1),1,0,std::complex<double>(3,-1), std::complex<double>(2,-1)};
    if(mode=="well-conditioned" || val_pc!=1)
        matrix={4,1,0,1,5,1,0,1,4, std::complex<double>(2,1),1,0,std::complex<double>(3,-1), std::complex<double>(2,-1)};
    if(mode=="singular-block") { matrix[0]=matrix[1]=matrix[2]=0; }
    if(mode=="nonfinite-block") matrix[2]={INFINITY,0};
    if(mode=="missing-self") neighbors[27]=-1;
    if(mode=="duplicate-index") order[2]=order[0];
    num_filledboxes=counts;num_children=child_counts;children=ranges;rwg_ind=order;
    boxes_near=neighbors;nearindex=offsets;matrix_Znear=matrix.data();size_Znear=matrix.size();
    const Values original=matrix;
    calculate_blockdiagonalpreconditioner();
    bool passed=matrix==original;
    if(val_pc!=1) {
        passed &= blockdiagonalmatrix==nullptr;
        std::printf("CHECK unused_mode_has_no_blocks %d\n",blockdiagonalmatrix==nullptr);
    } else {
        Values expected(6),rhs(6),actual(6);
        for(int i=0;i<6;++i) expected[i]={0.3+i*0.1,-0.1+i*0.03};
        for(int box=0;box<3;++box) {
            const int n=child_counts[box],first=ranges[2*box];
            for(int i=0;i<n;++i) for(int j=0;j<n;++j)
                rhs[order[first+i]]+=original[offsets[27*box]+i*n+j]*expected[order[first+j]];
        }
        mvm_BDPmatrix(rhs.data(),actual.data());
        const double separate=error(expected,actual);
        mvm_BDPmatrix(rhs.data(),rhs.data());
        const double alias=error(expected,rhs);
        std::printf("CHECK mapped_solve %.17g in_place %.17g physical_matrix_unchanged %d\n",separate,alias,matrix==original);
        passed &= separate<=1e-12 && alias<=1e-12;
    }
    // Reset borrowed synthetic arrays before invoking the actual production
    // cleanup. The factors must remain independently owned by the blocks.
    num_filledboxes=nullptr;num_children=nullptr;children=nullptr;rwg_ind=nullptr;
    boxes_near=nullptr;nearindex=nullptr;matrix_Znear=nullptr;
    clear_memory();
    const bool cleared=blockdiagonalmatrix==nullptr;
    std::printf("CHECK factors_cleared %d\n",cleared);passed &= cleared;
    // Keep baseline probes safe after recording the old cleanup deficiency.
    delete[] blockdiagonalmatrix;blockdiagonalmatrix=nullptr;
    return passed ? 0 : 1;
}
