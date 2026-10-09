#include "mlfma_workspace.h"
#include <stdio.h>
#include <complex>
#include <chrono>
#include <omp.h>
#include <cmath>

using namespace std;
using namespace std::chrono;

extern double pi;
extern long int *agg_index;
extern long int size_agg;
extern double *box_pos;
extern double *size_boxes;
extern int *children;
extern double val_kO;
extern int *boxes_far;
extern int *num_truncation;
extern int num_interpol;
extern int num_mlfmalevels;
extern int *num_filledboxes;
extern int num_rwgs;
extern int *rwg_ind;
extern double *rwg_lengths;
extern double *node_pos;
extern int *rwg_nodes;
extern int *rwg_tris;
extern double *vec_trinormals;
extern int *trans_index;
extern complex<double> *trans_op;
extern complex<double> *aggregation;
extern complex<double> *disaggregation;
extern int parallel_omp_on;
extern int num_threads;
extern FILE *file_timer;
extern double val_omega;
extern double val_muO;

void phi_locations_weights(int,double*,double*);
void theta_locations_weights(int,double*,double*);
void theta_locations_weights_lin(int, double*, double*);
void calculate_sin_cos(int, int, double*, double*, double*, double*, double*, double*);
void interpolationweights(int,int,int,int,double*,double*,double*,int*);
void interpolation_c2f(int,int,int,int,int,int*,int*,double*,double*,complex<double>*,complex<double>*);
void interpolation_f2c(int,int,int,int,int,int*,int*,double*,double*,complex<double>*,complex<double>*);
void crossProduct3d(double vec1[], double vec2[], double resvec[]);
double dotProduct3d(double vec1[], double vec2[]);

void calculate_farfield_mlfma(int ind_sol, complex<double> x[]){
    auto& workspace = mlfma_workspace();
    //
    //
    // calculate_farfield_mlfma
    // Calculate the far-zone scattered electic fields via MLFMA.
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    int box_start, box_end;
    complex<double> complex0=complex<double>(0.0, 0.0);
    complex<double> i=complex<double>(0.0, 1.0);
    complex<double> y_constant=(i*val_kO/(4.0*pi))*(i*val_kO/(4.0*pi));

    for(int ind0=0; ind0<(size_agg*2);ind0++){
        aggregation[ind0]=complex0;
    }


    //
    //
    // Aggregation operation at the last level
    // Aggregation is performed on every basis function at each box.
    // Radiation patterns of each basis function is shifted from sample position to the box center.
    // Shifted patterns are multiplied with the given complex coefficients.
    box_start=num_filledboxes[num_mlfmalevels*2-1];
    box_end=num_filledboxes[num_mlfmalevels*3-1]+1;
    const auto& leaf_grid = workspace.grid(num_truncation[num_mlfmalevels-1]+1,2*(num_truncation[num_mlfmalevels-1]+1));
    omp_set_num_threads(num_threads);

    #pragma omp parallel if(parallel_omp_on) shared(aggregation,x)
    {
        double shiftvector[3];
        double khat[3]{0.0, 0.0, 0.0};
        double dotval;
        complex<double> expfactor;
        int numberofterms=num_truncation[num_mlfmalevels-1];
        int numberofthetas=numberofterms+1;
        int numberofphis=numberofthetas*2;
        const double* sintheta = leaf_grid.sin_theta.data();
        const double* costheta = leaf_grid.cos_theta.data();
        const double* sinphi = leaf_grid.sin_phi.data();
        const double* cosphi = leaf_grid.cos_phi.data();
        int basisnode1,basisnode2,basisnode3;
        double basisnode1loc[3],basisnode2loc[3],basisnode3loc[3],basislocation[3];
        double basislength,basissign,basisvector[3];
        double basisradiationtheta,basisradiationphi;
        double rwgsign[2]{1.0,-1.0};
        int rwglastnodeindex[2]{2,3};
        int indtheta,indphi;
        complex<double> val_x;


        int tid=omp_get_thread_num();
        double timer_start = omp_get_wtime();
        double timer_total = 0.0;

        #pragma omp for schedule(dynamic,5)
        for(int box_basis=box_start;box_basis<box_end;box_basis++){
            double timer_in = omp_get_wtime();
            int start_basis=children[box_basis*2+0];
            int end_basis=children[box_basis*2+1]+1;
            for(int ind_basis=start_basis;ind_basis<end_basis;ind_basis++){
                int basisfunction=rwg_ind[ind_basis];
                val_x=x[basisfunction];
                double basisboxcenter[3]={box_pos[box_basis*3+0],box_pos[box_basis*3+1],box_pos[box_basis*3+2]};
                for(int rwgtri=0; rwgtri<2; rwgtri++){
                    basisnode1=rwg_nodes[basisfunction*4+0];
                    basisnode2=rwg_nodes[basisfunction*4+1];
                    basisnode3=rwg_nodes[basisfunction*4+rwglastnodeindex[rwgtri]];
                    basisnode1loc[0]=node_pos[basisnode1*3+0];
                    basisnode1loc[1]=node_pos[basisnode1*3+1];
                    basisnode1loc[2]=node_pos[basisnode1*3+2];
                    basisnode2loc[0]=node_pos[basisnode2*3+0];
                    basisnode2loc[1]=node_pos[basisnode2*3+1];
                    basisnode2loc[2]=node_pos[basisnode2*3+2];
                    basisnode3loc[0]=node_pos[basisnode3*3+0];
                    basisnode3loc[1]=node_pos[basisnode3*3+1];
                    basisnode3loc[2]=node_pos[basisnode3*3+2];
                    basislocation[0]=(basisnode1loc[0]+basisnode2loc[0]+basisnode3loc[0])/3;
                    basislocation[1]=(basisnode1loc[1]+basisnode2loc[1]+basisnode3loc[1])/3;
                    basislocation[2]=(basisnode1loc[2]+basisnode2loc[2]+basisnode3loc[2])/3;
                    basislength=rwg_lengths[basisfunction];
                    basissign=rwgsign[rwgtri];
                    shiftvector[0]=basislocation[0]-basisboxcenter[0];
                    shiftvector[1]=basislocation[1]-basisboxcenter[1];
                    shiftvector[2]=basislocation[2]-basisboxcenter[2];
                    basisvector[0]=basislocation[0]-basisnode3loc[0];
                    basisvector[1]=basislocation[1]-basisnode3loc[1];
                    basisvector[2]=basislocation[2]-basisnode3loc[2];
                    long int indagg=agg_index[box_basis]*2;
                    for(indtheta=0;indtheta<numberofthetas;indtheta++){
                        for(indphi=0;indphi<numberofphis;indphi++){
                            khat[0]=sintheta[indtheta]*cosphi[indphi];
                            khat[1]=sintheta[indtheta]*sinphi[indphi];
                            khat[2]=costheta[indtheta];
                            dotval=shiftvector[0]*khat[0]+shiftvector[1]*khat[1]+shiftvector[2]*khat[2];
                            expfactor=exp(-i*val_kO*dotval);
                            basisradiationtheta=costheta[indtheta]*cosphi[indphi]*basisvector[0]+costheta[indtheta]*sinphi[indphi]*basisvector[1]-sintheta[indtheta]*basisvector[2];
                            basisradiationphi=-sinphi[indphi]*basisvector[0]+cosphi[indphi]*basisvector[1];
                            aggregation[indagg]+=val_x*basissign*basislength*expfactor*basisradiationtheta/2.0;
                            indagg++;
                            aggregation[indagg]+=val_x*basissign*basislength*expfactor*basisradiationphi/2.0;
                            indagg++;
                        }
                    }
                }
            }
            timer_total+=omp_get_wtime()-timer_in;
        }
        double timer_end = omp_get_wtime();
//        fprintf(file_timer,"MVMFarAgg%d %d %1.5f %1.5f\n", num_mlfmalevels-1, omp_get_thread_num(), timer_end-timer_start, timer_total );
    }
    
    


    //
    //
    // Aggregation operation at the higher levels
    // Aggregation is performed on every child of each parent box.
    // Fields of the boxes are interpolated (upsampled) and shifted to the parent box center
    for(int indlev=(num_mlfmalevels-2); indlev>=0; indlev--){
        int numberofterms=num_truncation[indlev];
        int num_finesample_theta=numberofterms+1;
        int num_finesample_phi=num_finesample_theta*2;
        numberofterms=num_truncation[indlev+1];
        int num_coarsesample_theta=numberofterms+1;
        int num_coarsesample_phi=num_coarsesample_theta*2;
        auto& fine_grid = workspace.grid(num_finesample_theta,num_finesample_phi);
        auto& weights = workspace.table(num_coarsesample_theta,num_coarsesample_phi,
                                        num_finesample_theta,num_finesample_phi,num_interpol);
        double* sintheta = fine_grid.sin_theta.data();
        double* costheta = fine_grid.cos_theta.data();
        double* sinphi = fine_grid.sin_phi.data();
        double* cosphi = fine_grid.cos_phi.data();
        double* inter_weights_theta = weights.theta_weights.data();
        double* inter_weights_phi = weights.phi_weights.data();
        int* indexloc_theta = weights.theta_indices.data();
        int* indexloc_phi = weights.phi_indices.data();
        complex<double> *temp_fine_theta, *temp_fine_phi;
        complex<double> *temp_coarse_theta, *temp_coarse_phi;

        box_start=num_filledboxes[num_mlfmalevels*1+indlev];
        box_end=num_filledboxes[num_mlfmalevels*2+indlev]+1;

        omp_set_num_threads(num_threads);
        #pragma omp parallel if(parallel_omp_on) shared(aggregation) private(temp_fine_theta,temp_fine_phi,\
                                                                             temp_coarse_theta,temp_coarse_phi)
        {
            workspace.prepare_team();
            double shiftvector[3];
            double khat[3]{0.0, 0.0, 0.0};
            double dotval;
            complex<double> expfactor;
            double timer_start = omp_get_wtime();
            double timer_total = 0.0;
            auto& scratch = workspace.worker();
            scratch.prepare(num_coarsesample_theta,num_coarsesample_phi,num_finesample_theta,num_finesample_phi);
            temp_fine_theta = scratch.fine_theta.data();
            temp_fine_phi = scratch.fine_phi.data();
            temp_coarse_theta = scratch.coarse_theta.data();
            temp_coarse_phi = scratch.coarse_phi.data();
            #pragma omp for schedule(dynamic)
            for(int box_parent=box_start;box_parent<box_end;box_parent++){
                double timer_in = omp_get_wtime();
                double parentbasisboxcenter[3]={box_pos[box_parent*3+0],box_pos[box_parent*3+1],box_pos[box_parent*3+2]};
                int box_child_start=children[box_parent*2+0];
                int box_child_end=children[box_parent*2+1]+1;
                for(int box_child=box_child_start;box_child<box_child_end;box_child++){
                    double childbasisboxcenter[3]={box_pos[box_child*3+0],box_pos[box_child*3+1],box_pos[box_child*3+2]};
                    shiftvector[0]=childbasisboxcenter[0]-parentbasisboxcenter[0];
                    shiftvector[1]=childbasisboxcenter[1]-parentbasisboxcenter[1];
                    shiftvector[2]=childbasisboxcenter[2]-parentbasisboxcenter[2];
                    long int ind_child=agg_index[box_child];
                    int ind_temp=0;
                    for(int ind=0;ind<(num_finesample_theta*num_finesample_phi);ind++){
                        temp_fine_theta[ind]=complex0;
                        temp_fine_phi[ind]=complex0;
                    }
                    for(int ind=0;ind<(num_coarsesample_theta*num_coarsesample_phi);ind++){
                        temp_coarse_theta[ind]=complex0;
                        temp_coarse_phi[ind]=complex0;
                    }
                    for(int indtheta=0;indtheta<num_coarsesample_theta;indtheta++){
                        for(int indphi=0;indphi<num_coarsesample_phi;indphi++){
                            temp_coarse_theta[ind_temp]=aggregation[ind_child*2+0];
                            temp_coarse_phi[ind_temp]=aggregation[ind_child*2+1];
                            ind_temp++;
                            ind_child++;
                        }
                    }
                    interpolation_c2f(num_coarsesample_theta,num_coarsesample_phi,num_finesample_theta,num_finesample_phi,num_interpol,indexloc_theta,indexloc_phi,inter_weights_theta,inter_weights_phi,temp_coarse_theta,temp_fine_theta,scratch.interpolation);
                    interpolation_c2f(num_coarsesample_theta,num_coarsesample_phi,num_finesample_theta,num_finesample_phi,num_interpol,indexloc_theta,indexloc_phi,inter_weights_theta,inter_weights_phi,temp_coarse_phi,temp_fine_phi,scratch.interpolation);
                    long int ind_parent=agg_index[box_parent];

                    ind_temp=0;
                    for(int indtheta=0;indtheta<num_finesample_theta;indtheta++){
                        for(int indphi=0;indphi<num_finesample_phi;indphi++){
                            khat[0]=sintheta[indtheta]*cosphi[indphi];
                            khat[1]=sintheta[indtheta]*sinphi[indphi];
                            khat[2]=costheta[indtheta];
                            dotval=dotProduct3d(shiftvector,khat);
                            expfactor=exp(-i*val_kO*dotval);
                            aggregation[ind_parent*2+0]+=expfactor*temp_fine_theta[ind_temp];
                            aggregation[ind_parent*2+1]+=expfactor*temp_fine_phi[ind_temp];
                            ind_temp++;
                            ind_parent++;
                        }
                    }
                }

                timer_total+=omp_get_wtime()-timer_in;
            }

            double timer_end = omp_get_wtime();
//            fprintf(file_timer,"MVMFarAgg%d %d %1.5f %1.5f \n", indlev, omp_get_thread_num(), timer_end-timer_start, timer_total );
        }
    } 



    {
        int num_finesample_theta=181;
        int num_finesample_phi=360;
        int num_coarsesample_theta=num_truncation[0]+1;
        int num_coarsesample_phi=num_coarsesample_theta*2;
        auto& weights = workspace.table(num_coarsesample_theta,num_coarsesample_phi,
                                        num_finesample_theta,num_finesample_phi,num_interpol,true);
        double* inter_weights_theta = weights.theta_weights.data();
        double* inter_weights_phi = weights.phi_weights.data();
        int* indexloc_theta = weights.theta_indices.data();
        int* indexloc_phi = weights.phi_indices.data();
        workspace.prepare_team();
        auto& scratch = workspace.worker();
        scratch.prepare(num_coarsesample_theta,num_coarsesample_phi,num_finesample_theta,num_finesample_phi);
        complex<double>* temp_fine_theta = scratch.fine_theta.data();
        complex<double>* temp_fine_phi = scratch.fine_phi.data();
        complex<double>* temp_coarse_theta = scratch.coarse_theta.data();
        complex<double>* temp_coarse_phi = scratch.coarse_phi.data();

        long int ind_child=agg_index[0];
        int ind_temp=0;
        for(int ind=0;ind<(num_finesample_theta*num_finesample_phi);ind++){
            temp_fine_theta[ind]=complex0;
            temp_fine_phi[ind]=complex0;
        }
        for(int ind=0;ind<(num_coarsesample_theta*num_coarsesample_phi);ind++){
            temp_coarse_theta[ind]=complex0;
            temp_coarse_phi[ind]=complex0;
        }
        for(int indtheta=0;indtheta<num_coarsesample_theta;indtheta++){
            for(int indphi=0;indphi<num_coarsesample_phi;indphi++){
                temp_coarse_theta[ind_temp]=aggregation[ind_child*2+0];
                temp_coarse_phi[ind_temp]=aggregation[ind_child*2+1];
                ind_temp++;
                ind_child++;
            }
        }


        interpolation_c2f(num_coarsesample_theta,num_coarsesample_phi,num_finesample_theta,num_finesample_phi,num_interpol,indexloc_theta,indexloc_phi,inter_weights_theta,inter_weights_phi,temp_coarse_theta,temp_fine_theta,scratch.interpolation);
        interpolation_c2f(num_coarsesample_theta,num_coarsesample_phi,num_finesample_theta,num_finesample_phi,num_interpol,indexloc_theta,indexloc_phi,inter_weights_theta,inter_weights_phi,temp_coarse_phi,temp_fine_phi,scratch.interpolation);


        printf("num_coarsesample_theta: %d \n", num_coarsesample_theta);
        printf("num_coarsesample_phi: %d \n", num_coarsesample_phi);

        /*
        {
 
            int ind=0;
            for(int indt=0; indt<num_coarsesample_theta; indt++){
                for(int indp=0; indp<num_coarsesample_phi; indp++ ){
                    double dir_theta=double(indt)/(num_coarsesample_theta)*180.0;
                    double dir_phi=double(indp)/(num_coarsesample_phi)*360.0;
                    printf("%1.4e %1.4e %1.8e %1.8e %1.8e %1.8e \n", dir_theta, dir_phi, \
                                abs(temp_coarse_theta[ind]), arg(temp_coarse_theta[ind]), \
                                abs(temp_coarse_phi[ind]), arg(temp_coarse_phi[ind]));
                    ind++;
                }                    
            }
        }
        */




        // Output staging has a local owner and is released after writing.
        vector<complex<double>> farfield_theta(num_finesample_theta*(num_finesample_phi+1));
        vector<complex<double>> farfield_phi(num_finesample_theta*(num_finesample_phi+1));

        complex<double> far_constant=i*val_omega*val_muO/(4*pi);
        int ind,inds;
        for(int indt=0; indt<num_finesample_theta; indt++){
            for(int indp=0; indp<num_finesample_phi; indp++ ){
                inds = indt*(num_finesample_phi) + indp;
                ind = indt*(num_finesample_phi+1) + indp;
                farfield_theta[ind]=temp_fine_theta[inds]*far_constant;
                farfield_phi[ind]=temp_fine_phi[inds]*far_constant;
            }
            inds = indt*(num_finesample_phi) + 0;
            ind = indt*(num_finesample_phi+1) + num_finesample_phi;
            farfield_theta[ind]=temp_fine_theta[inds]*far_constant;
            farfield_phi[ind]=temp_fine_phi[inds]*far_constant;
        }




        FILE *fptr;
        char filename[100];
        sprintf(filename, "result_farefield_mlfma_%d.txt", ind_sol);
        fptr=fopen(filename,"w");
        fprintf(fptr,"%%Theta       Phi         Abs(E_theta)    Phase(E_theta)  Abs(E_phi)      Phase(E_phi)\n");
        ind=0;
        for(int indt=0; indt<num_finesample_theta; indt++){
            for(int indp=0; indp<(num_finesample_phi+1); indp++ ){
                double dir_theta=double(indt)/(num_finesample_theta-1)*180.0;
                double dir_phi=double(indp)/(num_finesample_phi)*360.0;
                fprintf(fptr,"%1.4e %1.4e %1.8e %1.8e %1.8e %1.8e \n", dir_theta, dir_phi, \
                        abs(farfield_theta[ind]), arg(farfield_theta[ind]), \
                        abs(farfield_phi[ind]), arg(farfield_phi[ind]));
                ind++;
            }
        }
        fclose(fptr);

    }

















    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
//    printf("Zfar MVM Calculation Takes %f s \n", time_span.count());

}

