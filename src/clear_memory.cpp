#include "solver_globals.h"
#include "matrixBlock.h"
#include <complex>

using namespace std;

// created in read_simulationinput

// created in read_meshmodel
extern double *node_pos;
extern int *tri_nodes;

// created in generate_edgeconnection
extern int *nodetrisindex;
extern int *rwg_tris;
extern int *rwg_nodes;
extern double *rwg_lengths;
extern int *tri2rwgindex;
extern int *tri2rwgnode;
extern int *tri2rwgsign;
extern double *tri_areas;
extern double *vec_trinormals;

// created in genetare_constants
extern double *ffdir_phi;
extern double *ffdir_theta;

// created in generate_octtree
extern int *num_truncation;
extern double *size_boxes;
extern int *num_filledboxes;
extern int *num_children;
extern double *box_pos;
extern int *children;
extern int *children2parent;
extern int *rwg_ind;

// created in generate_nearfarboxes
extern int *boxes_far;
extern int *boxes_near;

// created in generate_Znearcomponent
extern int *rwg2box;
extern int *rwg_reverseind;
extern int *smallestbox2tri;

// created in generate_Zfarcomponents

// created in generate_translationcomponent
extern int *trans_index;

// created in calculate_translationoperator
extern complex<double> *trans_op;

// created in calculate_Znearmatrix
extern complex<double> *matrix_Znear;

// created for mvm_Zfardotx
extern complex<double> *aggregation;
extern complex<double> *disaggregation;

// created in calculate_farfield
extern complex<double> *vec_farfield;
extern matrixBlock* blockdiagonalmatrix;

void clear_memory(){
    //
    //
    // clear_memory
    // Deallocating the large global arrays in the simulation
    delete[] blockdiagonalmatrix;
    blockdiagonalmatrix = nullptr;
    delete[] node_pos;
    delete[] tri_nodes;
    delete[] rwg_tris;
    delete[] rwg_nodes;
    delete[] rwg_lengths;
    delete[] tri2rwgindex;
    delete[] tri2rwgnode;
    delete[] tri2rwgsign;
    delete[] tri_areas;
    delete[] vec_trinormals;
    // delete[] ffdir_phi;
    // delete[] ffdir_theta;
    delete[] num_truncation;
    delete[] size_boxes;
    delete[] num_filledboxes;
    delete[] num_children;
    delete[] box_pos;
    delete[] children;
    delete[] children2parent;
    delete[] rwg_ind;
    delete[] boxes_far;
    delete[] boxes_near;
    delete[] nearindex;
    delete[] rwg2box;
    delete[] rwg_reverseind;
    delete[] smallestbox2tri;
    delete[] agg_index;
    delete[] trans_index;
    delete[] trans_op;
    delete[] matrix_Znear;
    delete[] aggregation;
    delete[] disaggregation;
    // delete[] vec_farfield;





}
