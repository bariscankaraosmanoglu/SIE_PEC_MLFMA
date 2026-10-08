// Uses the production reader and geometry setup; also links to archived sources.
#include "main.h"
#include <cstdio>
#include <string>
int main(int argc, char** argv) {
    read_simulationinput();
    std::printf("CHECK accepted frequency=%.17g frequencies=%d materials=%d points=%d triangles=%d threads=%d\n",
                val_freq,num_freqs,num_media,num_nodes,num_tris,num_threads);
    FILE* file=std::fopen("parsed.bin","wb");
    if(!file) return 2;
    std::fwrite(node_pos,sizeof(double),std::size_t(num_nodes)*3,file);
    std::fwrite(tri_nodes,sizeof(int),std::size_t(num_tris)*3,file);
    std::fwrite(tri_media,sizeof(int),num_tris,file);
    std::fwrite(val_cfiealpha,sizeof(double),num_media,file);
    std::fclose(file);
    if(argc>1 && std::string(argv[1])=="tree") {
        generate_edgeconnection();generate_octtree();
        std::printf("CHECK tree levels=%d rwgs=%d\n",num_mlfmalevels,num_rwgs);
    }
}
