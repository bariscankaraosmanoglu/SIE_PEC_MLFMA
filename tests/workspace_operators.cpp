// Repeated products exercise scratch lifetimes independently of Krylov storage.
#include "main.h"
#if __has_include("mlfma_workspace.h")
#include "mlfma_workspace.h"
#define HAS_WORKSPACE 1
#else
#define HAS_WORKSPACE 0
#endif
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <omp.h>
#include <string>
#include <sys/resource.h>
#include <vector>

void mvm_Zfardotx(complex<double>*, complex<double>*);
void mvm_Zfardotx_in(complex<double>*, complex<double>*);
void calculate_farfield_mlfma(int, complex<double>*);

static void require(bool ok, const char* what) {
    if (!ok) { fprintf(stderr,"FAIL %s\n",what); exit(1); }
}
static void dump(const string& name, const vector<complex<double>>& values) {
    ofstream f(name,ios::binary);
    f.write(reinterpret_cast<const char*>(values.data()),values.size()*sizeof(values[0]));
    require(bool(f),"write operator output");
}
static string field(int index) {
    ifstream f("result_farefield_mlfma_"+to_string(index)+".txt",ios::binary);
    require(bool(f),"open accelerated field output");
    return {istreambuf_iterator<char>(f),istreambuf_iterator<char>()};
}
static double compare(const vector<complex<double>>& a, const vector<complex<double>>& b) {
    double error=0, scale=0;
    for (size_t j=0;j<a.size();++j) {
        require(isfinite(real(b[j])) && isfinite(imag(b[j])),"finite repeated product");
        error+=norm(a[j]-b[j]); scale+=norm(a[j]);
    }
    return sqrt(error/(scale ? scale : 1));
}
static bool owners_empty() {
    return !node_pos && !tri_nodes && !tri_media && !rwg_media && !dir_PW && !vec_sol &&
        !vec_RHS && !num_truncation_pc && !agg_index_pc && !trans_index_pc && !trans_op_pc &&
        !ffdir_theta && !ffdir_phi && !vec_farfield && !nodetrisindex && !aggregation &&
        !disaggregation && !blockdiagonalmatrix;
}
#if HAS_WORKSPACE
// SFINAE keeps this driver usable with the archived rank-6 workspace.
template<class Workspace>
static auto check_tree_shifts(Workspace& workspace, int)
    -> decltype(workspace.child_shifts(1,2,1.0,1.0),void()) {
    size_t hits=0, fallbacks=0;
    double maximum=0;
    for (int approximate=0;approximate<2;++approximate) {
        for (int level=(approximate ? 2 : 0);level<num_mlfmalevels-1;++level) {
            const int nt=(approximate ? num_truncation_pc[level] : num_truncation[level])+1, np=2*nt;
            const auto& table=workspace.child_shifts(nt,np,size_boxes[level],val_kO);
            const auto& grid=workspace.grid(nt,np);
            const int begin=num_filledboxes[num_mlfmalevels+level];
            const int end=num_filledboxes[2*num_mlfmalevels+level]+1;
            for (int parent=begin;parent<end;++parent)
                for (int child=children[2*parent];child<=children[2*parent+1];++child) {
                    double shift[3];
                    for (int a=0;a<3;++a) shift[a]=box_pos[3*child+a]-box_pos[3*parent+a];
                    const auto* phase=table.for_shift(shift);
                    if (!phase) { ++fallbacks; continue; }
                    ++hits;
                    for (int t=0;t<nt;++t) for (int p=0;p<np;++p) {
                        const double dot=shift[0]*(grid.sin_theta[t]*grid.cos_phi[p])+
                                         shift[1]*(grid.sin_theta[t]*grid.sin_phi[p])+shift[2]*grid.cos_theta[t];
                        const auto direct=exp(complex<double>(0,-val_kO)*dot);
                        maximum=max(maximum,abs(phase[t*np+p]-direct));
                        maximum=max(maximum,abs(conj(phase[t*np+p])-conj(direct)));
                    }
                }
        }
    }
    require(hits>0 && maximum<=1e-12,"actual tree phase agreement");
    printf("CHECK tree_shift_cache hits %zu fallbacks %zu tables %zu max_phase_error %.17g\n",
           hits,fallbacks,workspace.shift_count(),maximum);
}
template<class Workspace> static void check_tree_shifts(Workspace&, long) {}
#endif
int main(int argc,char** argv) {
    if (argc<2) return 2;
    if (string(argv[1])=="cleanup") {
        // Exercise a partially initialized simulation, including previously omitted owners.
        node_pos=new double[3]; tri_media=new int[1]; rwg_media=new double[1];
        dir_PW=new double[4]; vec_sol=new complex<double>[1]; vec_RHS=new complex<double>[1];
        num_truncation_pc=new int[1]; agg_index_pc=new long int[1];
        trans_index_pc=new int[1]; trans_op_pc=new complex<double>[1];
        ffdir_theta=new double[1]; ffdir_phi=new double[1]; vec_farfield=new complex<double>[1];
        nodetrisindex=new int[1];
        clear_memory();
        printf("CHECK partial_cleanup_owners_empty %d workspace %d\n",owners_empty(),HAS_WORKSPACE);
#if HAS_WORKSPACE
        require(owners_empty(),"partial cleanup owners");
        clear_memory();
        require(owners_empty(),"repeated partial cleanup");
        require(num_source!=nullptr,"static source count storage survives cleanup");
#endif
        return 0;
    }
    if (string(argv[1])!="repeat" || argc!=4) return 2;
    const int threads=atoi(argv[2]), repetitions=atoi(argv[3]);
    require(threads>0 && repetitions>0 && repetitions<=100,"bounded repeat settings");
    read_simulationinput(); parallel_omp_on=1; num_threads=threads;
    generate_edgeconnection(); generate_octtree(); generate_nearfarboxes();
    generate_Znearcomponent(); generate_Zfarcomponents(); generate_translationcomponent();
    calculate_loadbalance(); calculate_Znearmatrix(); calculate_translationoperator();
    aggregation=new complex<double>[2*size_agg]; disaggregation=new complex<double>[2*size_agg];
    vector<complex<double>> x[2], accurate[2], approximate[2], actual(num_rwgs), inner(num_rwgs);
    string fields[2];
    for (int k=0;k<2;++k) {
        x[k].resize(num_rwgs); accurate[k].resize(num_rwgs); approximate[k].resize(num_rwgs);
        for (int j=0;j<num_rwgs;++j) x[k][j]={sin(.13*j+.7*k),cos(.17*j-.4*k)};
        mvm_Zfardotx(x[k].data(),accurate[k].data());
        mvm_Zfardotx_in(x[k].data(),approximate[k].data());
        dump("accurate"+to_string(k)+".bin",accurate[k]);
        dump("approximate"+to_string(k)+".bin",approximate[k]);
        calculate_farfield_mlfma(k,x[k].data()); fields[k]=field(k);
    }
#if HAS_WORKSPACE
    check_tree_shifts(mlfma_workspace(),0);
#endif
    // Warm every requested team before checking storage stability. Runtime team
    // limits/dynamic adjustment remain authoritative; no requested-count indexing.
    for (int team : {1,2,8,32}) {
        num_threads=team;
        fill(actual.begin(),actual.end(),complex<double>{});
        fill(inner.begin(),inner.end(),complex<double>{});
        mvm_Zfardotx(x[0].data(),actual.data()); mvm_Zfardotx_in(x[0].data(),inner.data());
        require(compare(accurate[0],actual)<=1e-12 && compare(approximate[0],inner)<=1e-12,
                "changing teams preserves products");
    }
    num_threads=threads;
#if HAS_WORKSPACE
    const auto capacity=mlfma_workspace().retained_bytes();
    const auto grids=mlfma_workspace().grid_count(), tables=mlfma_workspace().table_count();
    printf("CHECK workspace_warm bytes %zu grids %zu tables %zu workers %zu\n",capacity,grids,tables,
           mlfma_workspace().worker_count());
#endif
    const auto start=steady_clock::now();
    double maximum=0;
    for (int i=0;i<repetitions;++i) {
        const int k=i%2;
        fill(actual.begin(),actual.end(),complex<double>{});
        fill(inner.begin(),inner.end(),complex<double>{});
        mvm_Zfardotx(x[k].data(),actual.data()); mvm_Zfardotx_in(x[k].data(),inner.data());
        maximum=max(maximum,max(compare(accurate[k],actual),compare(approximate[k],inner)));
        if (i<4) {
            calculate_farfield_mlfma(i+2,x[k].data());
            require(fields[k]==field(i+2),"repeated accelerated field output");
        }
#if HAS_WORKSPACE
        require(capacity==mlfma_workspace().retained_bytes() && grids==mlfma_workspace().grid_count() &&
                tables==mlfma_workspace().table_count(),"storage stable after warmup");
#endif
    }
    require(maximum<=1e-12,"repeated operator agreement");
    printf("CHECK repetitions %d threads %d max_relative_error %.17g steady_seconds %.9g\n",
           repetitions,threads,maximum,duration<double>(steady_clock::now()-start).count());
    struct rusage usage{}; getrusage(RUSAGE_SELF,&usage);
    printf("CHECK peak_rss_kib %ld\n",usage.ru_maxrss);
    clear_memory();
#if HAS_WORKSPACE
    require(owners_empty(),"full simulation cleanup owners");
    require(mlfma_workspace().retained_bytes()==0 && mlfma_workspace().grid_count()==0 &&
            mlfma_workspace().table_count()==0 && mlfma_workspace().worker_count()==0,"workspace released");
    clear_memory(); require(owners_empty(),"repeated full cleanup");
    // Metadata must rebuild identically after release (new simulation lifetime).
    auto& grid=mlfma_workspace().grid(5,10);
    const auto saved=grid.theta;
    mlfma_workspace().clear();
    require(mlfma_workspace().grid(5,10).theta==saved,"metadata rebuild");
    mlfma_workspace().clear();
    printf("CHECK cleanup_and_rebuild passed\n");
#endif
    return 0;
}
