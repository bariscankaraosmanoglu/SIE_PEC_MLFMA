// Synthetic dimensions exercise production overflow paths without large allocations.
#include "main.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <string>
#if __has_include("input_validation.h")
#include "input_validation.h"
#endif
void interpolationweights(int,int,int,int,double*,double*,double*,int*);
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    const std::string mode=argv[1];
    if(mode=="interpolation") {
        double coarse[2]={0.5,2.5}, fine[3]={0.1,1.0,3.0}, weights[18]{};int indices[18]{};
        interpolationweights(2,3,3,0,coarse,fine,weights,indices);
    } else if(mode=="near") {
        num_mlfmalevels=1;num_rwgs=50000;
        int counts[3]={1,0,0}, child[1]={50000}, neighbors[27];
        std::fill_n(neighbors,27,-1);neighbors[0]=0;
        num_filledboxes=counts;num_children=child;boxes_near=neighbors;
        generate_Znearcomponent();
    } else if(mode=="aggregation") {
        num_mlfmalevels=1;num_filledboxesall=1;
        int counts[3]={1,0,0}, terms[1]={32766};
        num_filledboxes=counts;num_truncation=terms;
        generate_Zfarcomponents();
    } else if(mode=="translation") {
        num_mlfmalevels=3;num_filledboxesall=3;
        int counts[9]={0,0,3,0,0,0,0,0,2},terms[3]={32766,32766,32766};
        double positions[9]={0,0,0,1,0,0,2,0,0},sizes[3]={4,2,1};int far[3*189];
        std::fill_n(far,3*189,-1);far[0]=1;far[1]=2;
        num_filledboxes=counts;num_truncation=terms;box_pos=positions;size_boxes=sizes;boxes_far=far;
        generate_translationcomponent();
    }
#if __has_include("input_validation.h")
    else if(mode=="limits") {
        assert(checked_index_product(46340,46340,INT_MAX,"near matrix")==2147395600L);
        assert(checked_index_sum(INT_MAX-1,1,INT_MAX,"translation")==INT_MAX);
        for(const auto& name:{"product","sum","stencil"}) {
            bool rejected=false;
            try {
                if(std::string(name)=="product") checked_index_product(50000,50000,INT_MAX,"near matrix");
                else if(std::string(name)=="sum") checked_index_sum(INT_MAX,1,INT_MAX,"translation");
                else validate_interpolation_dimensions(2,3,3);
            } catch(const InputError&) { rejected=true; }
            assert(rejected);
        }
        validate_interpolation_dimensions(2,3,2);
        assert(maximum_mlfma_levels()>1);
        puts("CHECK representable arithmetic and stencil boundaries passed");
    }
#endif
    else return 2;
    puts("CHECK dimension probe completed");
}
