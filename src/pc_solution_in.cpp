#include <stdio.h>
#include <complex>
#include <chrono>

using namespace std;
using namespace std::chrono;

extern int num_rwgs;

void pc_solution_in(complex<double> vec_RHS_i[], complex<double> vec_sol_i[], int size_vecRHS){


    for(int ind=0;ind<size_vecRHS;ind++){
        vec_sol_i[ind]=vec_RHS_i[ind];
    }


}

