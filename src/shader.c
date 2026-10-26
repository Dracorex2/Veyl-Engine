#include "renderer.h"

static char *concat(char *str1, char *str2) {
    int len = SDL_strlen(str1) + SDL_strlen(str2);
    char *res = malloc(len + 1);
    SDL_snprintf(res, len + 1, "%s%s", str1, str2);
    return res;
}

SDL_GPUShader *load_shader(SDL_GPUDevice *gpu, char *shader_file, Uint32 num_uniform_buf, Uint32 num_samplers) {
    SDL_GPUShaderStage stage;
    if (SDL_strstr(shader_file, ".vert"))
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    else if (SDL_strstr(shader_file, ".frag"))
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    char *shader_path = concat(shader_file, ".spv");
    size_t code_size = 0;
    void *code = SDL_LoadFile(shader_path, &code_size);
    if (code == NULL) {
        SDL_Log("Impossible de lire %s : %s", shader_path, SDL_GetError());
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code = (const Uint8 *)code,
        .code_size = code_size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_uniform_buffers = num_uniform_buf,
        .num_samplers = num_samplers,
    };

    SDL_GPUShader *shader = SDL_CreateGPUShader(gpu, &info);

    if (shader == NULL) {
        SDL_Log("Impossible de créer le shader %s : %s", shader_file, SDL_GetError());
    }

    SDL_free(code);

    return shader;
}