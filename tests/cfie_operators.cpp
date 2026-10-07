// CFIE row-weighting regression. Endpoint responses are established with
// uniform material coefficients, then combined independently for each row.
#include "main.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <string>
#include <vector>
void generate_cfiealpha();
void mvm_Zneardotx(complex<double>*, complex<double>*);
void mvm_Zfardotx(complex<double>*, complex<double>*);
void mvm_Zfardotx_in(complex<double>*, complex<double>*);
void calculate_rhsvector(double*);
void calculate_farfield_mlfma(int, complex<double>*);
int gmres(int, complex<double>*, int, double, complex<double>*, int);
using Values = vector<complex<double>>;
static void dump(const string& name, const Values& values) {
    FILE* f = fopen(name.c_str(), "wb");
    if (!f || fwrite(values.data(), sizeof(values[0]), values.size(), f) != values.size()) exit(2);
    fclose(f);
}
static double error(const Values& expected, const Values& actual) {
    double diff = 0, norm = 0;
    for (size_t j = 0; j < actual.size(); ++j) {
        if (!isfinite(abs(actual[j]))) return INFINITY;
        diff += std::norm(actual[j] - expected[j]); norm += std::norm(expected[j]);
    }
    return sqrt(diff / (norm ? norm : 1));
}
struct Response { Values near, far, approximate, rhs; vector<double> alpha; };
static Response evaluate(const Values& x, const vector<double>& coefficients, const string& tag) {
    copy(coefficients.begin(), coefficients.end(), val_cfiealpha);
    delete[] matrix_Znear; matrix_Znear = nullptr;
    delete[] rwg_media; rwg_media = nullptr;
    calculate_Znearmatrix();
    Response r{Values(num_rwgs), Values(num_rwgs), Values(num_rwgs), {}, {}};
    r.alpha.assign(rwg_media, rwg_media + num_rwgs);
    mvm_Zneardotx(const_cast<complex<double>*>(x.data()), r.near.data());
    mvm_Zfardotx(const_cast<complex<double>*>(x.data()), r.far.data());
    mvm_Zfardotx_in(const_cast<complex<double>*>(x.data()), r.approximate.data());
    double direction[4] = {37, 19, 90, 109};
    calculate_rhsvector(direction); r.rhs.assign(vec_RHS, vec_RHS + num_rwgs);
    delete[] vec_RHS; vec_RHS = nullptr;
    dump(tag + "-near.bin", r.near); dump(tag + "-far.bin", r.far);
    dump(tag + "-approximate.bin", r.approximate); dump(tag + "-rhs.bin", r.rhs);
    return r;
}
static Values blend(const Values& electric, const Values& magnetic, const vector<double>& alpha) {
    Values result(alpha.size());
    for (size_t j = 0; j < alpha.size(); ++j)
        result[j] = alpha[j] * electric[j] + (1 - alpha[j]) * magnetic[j];
    return result;
}
int main(int argc, char** argv) {
    if (argc != 3 && argc != 4) return 2;
    read_simulationinput(); num_threads = atoi(argv[2]);
    generate_edgeconnection();
    if (string(argv[1]) == "topology") {
        generate_cfiealpha();
        FILE* f = fopen("coefficients.txt", "w");
        if (!f) return 2;
        for (int j = 0; j < num_rwgs; ++j)
            fprintf(f, "%d %d %.17g\n", rwg_tris[2*j], rwg_tris[2*j+1], rwg_media[j]);
        fclose(f); return 0;
    }
    generate_octtree(); generate_nearfarboxes(); generate_Znearcomponent();
    generate_Zfarcomponents(); generate_translationcomponent(); calculate_loadbalance();
    calculate_translationoperator();
    aggregation = new complex<double>[2*size_agg]; disaggregation = new complex<double>[2*size_agg];
    vector<double> configured(val_cfiealpha, val_cfiealpha + num_media);
    Values x(num_rwgs);
    for (int j = 0; j < num_rwgs; ++j) x[j] = {sin(0.13*j), cos(0.17*j)};
    Response magnetic = evaluate(x, vector<double>(num_media, 0), "magnetic");
    Response electric = evaluate(x, vector<double>(num_media, 1), "electric");
    Response mixed = evaluate(x, configured, "configured");
    bool passed = true;
    for (auto entry : {pair<const char*, pair<Values*, Values*>>{"near", {&electric.near, &magnetic.near}},
                       {"far", {&electric.far, &magnetic.far}},
                       {"approximate", {&electric.approximate, &magnetic.approximate}},
                       {"rhs", {&electric.rhs, &magnetic.rhs}}}) {
        Values* actual = string(entry.first) == "near" ? &mixed.near : string(entry.first) == "far" ? &mixed.far :
                         string(entry.first) == "rhs" ? &mixed.rhs : &mixed.approximate;
        double e = error(blend(*entry.second.first, *entry.second.second, mixed.alpha), *actual);
        printf("CHECK %s_row_blend %.17g\n", entry.first, e); passed &= e <= 1e-12;
    }
    if (string(argv[1]) == "open") {
        passed &= all_of(mixed.alpha.begin(), mixed.alpha.end(), [](double alpha) { return alpha == 1; });
        double e = error(electric.far, magnetic.far);
        double p = error(electric.approximate, magnetic.approximate);
        printf("CHECK open_forces_efie far %.17g approximate %.17g\n", e, p);
        passed &= e <= 1e-12 && p <= 1e-12;
    } else {
        int pivot = num_rwgs / 2;
        rwg_media[pivot] = mixed.alpha[pivot] == 1 ? 0.3 : 1;
        vector<double> changed(rwg_media, rwg_media + num_rwgs);
        Values far(num_rwgs), approximate(num_rwgs);
        mvm_Zfardotx(x.data(), far.data()); mvm_Zfardotx_in(x.data(), approximate.data());
        double e = error(blend(electric.far, magnetic.far, changed), far);
        double p = error(blend(electric.approximate, magnetic.approximate, changed), approximate);
        printf("CHECK single_row_far %.17g approximate %.17g\n", e, p);
        passed &= e <= 1e-12 && p <= 1e-12;
    }
    copy(mixed.alpha.begin(), mixed.alpha.end(), rwg_media);
    Values solution(num_rwgs), product(num_rwgs);
    val_pc = 0;
    if (argc == 4) {
        FILE* f = fopen(argv[3], "r");
        if (!f) return 2;
        for (auto& value : solution) {
            double real, imag;
            if (fscanf(f, "%lf %lf", &real, &imag) != 2) return 2;
            value = {real, imag};
        }
        fclose(f);
    } else {
        gmres(num_rwgs, mixed.rhs.data(), min(num_rwgs, val_itermax), val_soltol, solution.data(), 0);
    }
    mvm_Zneardotx(solution.data(), product.data()); mvm_Zfardotx(solution.data(), product.data());
    double residual = error(mixed.rhs, product);
    printf("CHECK independent_residual %.17g tolerance %.17g\n", residual, val_soltol);
    passed &= residual <= val_soltol;
    dump("solution.bin", solution); dump("solution-product.bin", product);
    calculate_farfield_mlfma(0, solution.data());
    // Direct MoM reference: assemble every pair with the existing singular
    // integration kernel, using a single artificial leaf and no MLFMA.
    num_mlfmalevels = 1;
    int counts[3] = {1, 0, 0}, ranges[2] = {0, num_rwgs-1}, child_count[1] = {num_rwgs};
    int neighbors[27]; fill(neighbors, neighbors+27, -1); neighbors[0] = 0;
    long int offsets[27]{};
    vector<int> order(num_rwgs), reverse(num_rwgs), row_box(num_rwgs, 0), triangles(num_tris);
    iota(order.begin(), order.end(), 0); reverse = order; iota(triangles.begin(), triangles.end(), 0);
    num_filledboxes = counts; children = ranges; num_children = child_count; boxes_near = neighbors;
    nearindex = offsets; rwg_ind = order.data(); rwg_reverseind = reverse.data(); rwg2box = row_box.data();
    smallestbox2tri = triangles.data(); val_maxtri = num_tris; size_Znear = long(num_rwgs)*num_rwgs;
    calculate_loadbalance(); delete[] matrix_Znear; delete[] rwg_media;
    matrix_Znear = nullptr; rwg_media = nullptr; calculate_Znearmatrix();
    Values direct(num_rwgs); mvm_Zneardotx(x.data(), direct.data());
    Values accelerated = mixed.near;
    for (int j = 0; j < num_rwgs; ++j) accelerated[j] += mixed.far[j];
    double direct_error = error(direct, accelerated);
    printf("CHECK direct_mom_product %.17g\n", direct_error);
    Values direct_solution(num_rwgs); mvm_Zneardotx(solution.data(), direct_solution.data());
    double direct_residual = error(mixed.rhs, direct_solution);
    printf("CHECK direct_mom_residual %.17g\n", direct_residual);
    if (string(argv[1]) == "reference")
        passed &= direct_error <= val_soltol && direct_residual <= val_soltol;
    dump("direct.bin", direct); dump("accelerated.bin", accelerated);
    return passed ? 0 : 1;
}
