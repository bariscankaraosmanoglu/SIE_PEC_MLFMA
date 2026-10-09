#include "mlfma_workspace.h"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <limits>
#include <omp.h>
#include <stdexcept>
#include <utility>

void theta_locations_weights(int,double*,double*);
void theta_locations_weights_lin(int,double*,double*);
void phi_locations_weights(int,double*,double*);
void calculate_sin_cos(int,int,double*,double*,double*,double*,double*,double*);
void interpolationweights(int,int,int,int,double*,double*,double*,int*);

namespace {
std::size_t product(int a, int b, int multiplier = 1) {
    if (a <= 0 || b <= 0 || multiplier <= 0 ||
        std::size_t(a) > std::numeric_limits<std::size_t>::max()/b/multiplier)
        throw std::length_error("MLFMA workspace exceeds integer index capacity");
    const std::size_t count = std::size_t(a)*b*multiplier;
    if (count > INT_MAX) throw std::length_error("MLFMA workspace exceeds integer index capacity");
    return count;
}
template<class T> std::size_t bytes(const std::vector<T>& values) {
    return values.capacity()*sizeof(T);
}
}

AngularGrid& MlfmaWorkspace::grid(int theta, int phi, bool linear) {
    const GridKey key{theta,phi,linear};
    auto found = grids_.find(key);
    if (found != grids_.end()) return found->second;
    product(theta,phi);
    if (linear && theta < 2) throw std::invalid_argument("linear theta grid requires two samples");
    AngularGrid value;
    value.theta.resize(theta); value.theta_weights.resize(theta);
    value.phi.resize(phi); value.phi_weights.resize(phi);
    value.sin_theta.resize(theta); value.cos_theta.resize(theta);
    value.sin_phi.resize(phi); value.cos_phi.resize(phi);
    if (linear) theta_locations_weights_lin(theta,value.theta_weights.data(),value.theta.data());
    else theta_locations_weights(theta,value.theta_weights.data(),value.theta.data());
    phi_locations_weights(phi,value.phi_weights.data(),value.phi.data());
    calculate_sin_cos(theta,phi,value.theta.data(),value.phi.data(),value.sin_theta.data(),
                      value.cos_theta.data(),value.sin_phi.data(),value.cos_phi.data());
    return grids_.emplace(key,std::move(value)).first->second;
}

InterpolationTable& MlfmaWorkspace::table(int ct, int cp, int ft, int fp, int order, bool linear) {
    const TableKey key{ct,cp,ft,fp,order,linear};
    auto found = tables_.find(key);
    if (found != tables_.end()) return found->second;
    auto& coarse = grid(ct,cp);
    auto& fine = grid(ft,fp,linear);
    InterpolationTable value;
    value.theta_weights.resize(product(ft,order,2));
    value.theta_indices.resize(value.theta_weights.size());
    value.phi_weights.resize(product(fp,order,2));
    value.phi_indices.resize(value.phi_weights.size());
    interpolationweights(ct,ft,order,0,coarse.theta.data(),fine.theta.data(),
                         value.theta_weights.data(),value.theta_indices.data());
    interpolationweights(cp,fp,order,1,coarse.phi.data(),fine.phi.data(),
                         value.phi_weights.data(),value.phi_indices.data());
    return tables_.emplace(key,std::move(value)).first->second;
}

void InterpolationScratch::prepare(int ct, int cp, int ft) {
    inflated_theta.resize(product(ct,cp,3));
    half.resize(product(ft,cp));
    inflated_phi.resize(product(ft,cp,3));
}
void MlfmaScratch::prepare(int ct, int cp, int ft, int fp) {
    fine_theta.resize(product(ft,fp)); fine_phi.resize(fine_theta.size());
    coarse_theta.resize(product(ct,cp)); coarse_phi.resize(coarse_theta.size());
    interpolation.prepare(ct,cp,ft);
}

void MlfmaWorkspace::prepare_team() {
    #pragma omp single
    {
        try {
            const std::size_t count = omp_get_num_threads();
            if (workers_.size() < count) workers_.resize(count);
        } catch (const std::exception& error) {
            std::fprintf(stderr,"MLFMA workspace: %s\n",error.what());
            std::exit(EXIT_FAILURE);
        }
    }
}
MlfmaScratch& MlfmaWorkspace::worker() {
    return workers_.at(omp_get_thread_num());
}
void MlfmaWorkspace::clear() {
    grids_.clear(); tables_.clear();
    std::vector<MlfmaScratch>().swap(workers_);
}
std::size_t MlfmaWorkspace::retained_bytes() const {
    std::size_t total = bytes(workers_);
    for (const auto& entry : grids_) {
        const auto& g = entry.second;
        total += sizeof(entry)+bytes(g.theta)+bytes(g.theta_weights)+bytes(g.phi)+bytes(g.phi_weights)
                 +bytes(g.sin_theta)+bytes(g.cos_theta)+bytes(g.sin_phi)+bytes(g.cos_phi);
    }
    for (const auto& entry : tables_) {
        const auto& t = entry.second;
        total += sizeof(entry)+bytes(t.theta_weights)+bytes(t.phi_weights)+bytes(t.theta_indices)+bytes(t.phi_indices);
    }
    for (const auto& w : workers_)
        total += bytes(w.fine_theta)+bytes(w.fine_phi)+bytes(w.coarse_theta)+bytes(w.coarse_phi)
                 +bytes(w.interpolation.inflated_theta)+bytes(w.interpolation.half)+bytes(w.interpolation.inflated_phi);
    // Payload/capacity accounting for regression checks, not allocator RSS.
    return total;
}
MlfmaWorkspace& mlfma_workspace() {
    static MlfmaWorkspace value;
    return value;
}
