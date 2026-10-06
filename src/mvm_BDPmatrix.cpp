#include <complex>
#include "matrixBlock.h"

extern matrixBlock* blockdiagonalmatrix;
extern int num_rwgs;
extern int *num_filledboxes;
extern int num_mlfmalevels;

extern int *children;
extern int *boxes_near;
extern complex<double> *matrix_Znear;
extern long int *nearindex;
extern int num_mlfmalevels;
extern int *num_filledboxes;
extern int num_rwgs;
extern int *rwg_ind;

void mvm_BDPmatrix(std::complex<double> x[], std::complex<double> y[]){

std::complex<double>* y2 = new std::complex<double>[num_rwgs];
for(int i=0; i<num_rwgs; i++)
    y2[i]=complex<double> (0.0, 0.0);

    
    // Apply Block-Diagonal Preconditioner
    int num_boxes=num_filledboxes[num_mlfmalevels-1];
    for(int box_test=0;box_test<num_boxes;box_test++){
        int num_test = blockdiagonalmatrix[box_test].sizerow;
        int num_basis = blockdiagonalmatrix[box_test].sizecol;
        for(int i=0; i<num_test; i++){
            int indr=blockdiagonalmatrix[box_test].indrows[i];
            for(int j=0; j<num_basis; j++){
                int indc=blockdiagonalmatrix[box_test].indcols[j];
                y2[indr]+=blockdiagonalmatrix[box_test].submatrix[i][j]*x[indc];
            }
        }
    }


    for(int i=0; i<num_rwgs; i++)
        y[i]=y2[i];

    delete[] y2;
    

}