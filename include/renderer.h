#ifndef RENDERER_H
#define RENDERER_H

#include "mesh.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdbool.h>
#include <cglm/cglm.h>


typedef struct vertex_data {
    vec3 pos;
    SDL_FColor color;
    vec2 uv;
} vertex_data;

//typedef struct Model {
//    SDL_GPUBuffer *vertex_buffer;
//    SDL_GPUBuffer *index_buffer;
//    Uint32 num_indecies;
//    SDL_GPUTexture *texture;
//} Model;

typedef struct Renderer {
    SDL_Window *window;
    int window_size_w;
    int window_size_h;
    SDL_GPUDevice *gpu;
    SDL_GPUGraphicsPipeline *pipeline;
    Model *models;
    SDL_GPUSampler *sampler;
    SDL_GPUTexture *depth_texture;
    SDL_GPUTextureFormat depth_texture_format;
} Renderer;

//SDL_GPUShader *load_shader(SDL_GPUDevice *gpu, char *shader_file, Uint32 num_uniform_buf, Uint32 num_samplers);

Model *load_model(Renderer *renderer, char *obj_path, char *tx_path);

Renderer *renderer_create(SDL_Window *window);

bool create_pipeline(Renderer *renderer);

void renderer_draw(Renderer *renderer, mat4 view_mat);
void renderer_destroy(Renderer *renderer);

#endif