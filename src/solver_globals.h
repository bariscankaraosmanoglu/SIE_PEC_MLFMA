#ifndef SOLVER_GLOBALS_H
#define SOLVER_GLOBALS_H

// Material coefficients and the final testing coefficient for each RWG.
// Open surface components always receive an EFIE coefficient of one.
extern double val_cfiealpha[50];
extern double* rwg_media;
extern double source_PW[4];

// Indices shared by assembly, multiplication, and cleanup.
extern long int* nearindex;
extern long int* agg_index;

#endif
