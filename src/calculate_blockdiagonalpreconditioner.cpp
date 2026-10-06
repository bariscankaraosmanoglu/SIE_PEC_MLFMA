#include "matrixBlock.h"
#include "matrixoperations.h"
#include "matrix_inverse.h"
#include <iostream>

extern int *children;
extern int *boxes_near;
extern complex<double> *matrix_Znear;
extern long int *nearindex;
extern int num_mlfmalevels;
extern int *num_filledboxes;
extern int num_rwgs;
extern int *rwg_ind;
extern int num_filledboxesall;
extern int *num_children;
extern matrixBlock* blockdiagonalmatrix;

void calculate_blockdiagonalpreconditioner(){
    complex<double>** mattest;
    complex<double>** result;
    int matsize;

    printf("num_filledboxesall: %d \n", num_filledboxesall);

    blockdiagonalmatrix = new matrixBlock[num_filledboxes[num_mlfmalevels-1]];

    int box_start=num_filledboxes[num_mlfmalevels*2-1];
    int box_end=num_filledboxes[num_mlfmalevels*3-1]+1;

    for(int box_test=box_start;box_test<box_end;box_test++){
        for(int ind_nearbox=0;ind_nearbox<27;ind_nearbox++){
            if(boxes_near[box_test*27+ind_nearbox]!=-1){
                int box_basis=boxes_near[box_test*27+ind_nearbox];
                if(box_test==box_basis){
                    int num_rwgs_inbox = num_children[box_test];
                    blockdiagonalmatrix[box_test-box_start].initmatrixBlock(num_rwgs_inbox,num_rwgs_inbox);
                    int rwg_test_start=children[box_test*2+0];
                    int rwg_test_end=children[box_test*2+1]+1;
                    long int ind_Znear=nearindex[(box_test-box_start)*27+ind_nearbox];


                    for(int ind_rwg_test=rwg_test_start;ind_rwg_test<rwg_test_end;ind_rwg_test++){
                        int testfunction=rwg_ind[ind_rwg_test];
                        blockdiagonalmatrix[box_test-box_start].indrows[ind_rwg_test-rwg_test_start]=testfunction; 
                        blockdiagonalmatrix[box_test-box_start].indcols[ind_rwg_test-rwg_test_start]=testfunction; //Test and basis functions are the same for self-box

                        // printf("%d  %d \n", box_test, testfunction);
                    }


                    for(int ind_rwg_test=rwg_test_start;ind_rwg_test<rwg_test_end;ind_rwg_test++){
                        int testfunction=rwg_ind[ind_rwg_test];
                        int rwg_basis_start=children[box_basis*2+0];
                        int rwg_basis_end=children[box_basis*2+1]+1;
                        for(int ind_rwg_basis=rwg_basis_start;ind_rwg_basis<rwg_basis_end;ind_rwg_basis++){
                            int basisfunction=rwg_ind[ind_rwg_basis];
                            blockdiagonalmatrix[box_test-box_start].assignvalue(ind_rwg_test-rwg_test_start,ind_rwg_basis-rwg_basis_start,matrix_Znear[ind_Znear]);

                            //matrix_Znear[ind_Znear]=complex<double> (0.0, 0.0);
                            //if(basisfunction==testfunction)
                            //    matrix_Znear[ind_Znear]=complex<double> (1.0, 0.0);

                            ind_Znear++;
                        }
                    }
                }
            }
        }
    }



    printf("copying finished \n");

    for(int box_test=box_start;box_test<box_end;box_test++){
        mattest = blockdiagonalmatrix[box_test-box_start].submatrix;  
        matsize = blockdiagonalmatrix[box_test-box_start].sizerow;
        complex<double>** result = matrix_inverse(mattest, matsize);
        for(int indr=0; indr<matsize; indr++){
            for(int indc=0; indc<matsize; indc++){
                blockdiagonalmatrix[box_test-box_start].assignvalue(indr,indc,result[indr][indc]);
            }
        }
        for(int indr=0; indr<matsize; indr++)
            delete[] result[indr];
    }

    printf("inverse operation finished \n");

    //delete[] blockdiagonalmatrix;

    std::cout.flush();
    
}