#ifndef MESH_H
#define MESH_H

#include <SDL3/SDL.h>
#include <cglm/cglm.h>


typedef struct Mesh {
    SDL_GPUBuffer *vertex_buffer;
    SDL_GPUBuffer *index_buffer;
    Uint32 num_indecies;
} Mesh;

typedef struct Model {
    Mesh mesh;    
    SDL_GPUTexture *texture;
} Model;


#endif