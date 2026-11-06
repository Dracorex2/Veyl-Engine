#include "renderer.h"
#include "mesh.h"
#include "obj.h"

#include <stddef.h>
#include <string.h>


#include <SDL3_image/SDL_image.h>
const SDL_FColor WHITE = {1, 1, 1, 1};


#include <SDL3/SDL.h>

#include <SDL3/SDL.h>

/* Concatène `count` chaînes.
 * Retourne une chaîne allouée avec SDL_malloc (à libérer avec SDL_free),
 * ou NULL en cas d'erreur. Les entrées NULL sont traitées comme "". */
#include <SDL3/SDL.h>

/* Concatène s1 et s2 dans une nouvelle chaîne allouée avec SDL_malloc
 * (à libérer avec SDL_free). Retourne NULL en cas d'erreur.
 * Une entrée NULL est traitée comme "". */
char *my_strjoin(const char *s1, const char *s2)
{
    const size_t len1 = s1 ? SDL_strlen(s1) : 0;
    const size_t len2 = s2 ? SDL_strlen(s2) : 0;

    char *result = (char *)SDL_malloc(len1 + len2 + 1);
    if (!result) {
        return NULL;
    }

    if (len1) {
        SDL_memcpy(result, s1, len1);
    }
    if (len2) {
        SDL_memcpy(result + len1, s2, len2);
    }
    result[len1 + len2] = '\0';

    return result;
}

static SDL_GPUShader *load_shader(SDL_GPUDevice *gpu, char *shader_file, Uint32 num_uniform_buf, Uint32 num_samplers) {
    SDL_GPUShaderStage stage;
    if (SDL_strstr(shader_file, ".vert"))
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    else if (SDL_strstr(shader_file, ".frag"))
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    char *shader_path = my_strjoin(shader_file, ".spv");
    size_t code_size = 0;
    void *code = SDL_LoadFile(shader_path, &code_size);
    if (code == NULL) {
        SDL_Log("Impossible de lire %s : %s", shader_path, SDL_GetError());
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {0};
    info.code = (const Uint8 *)code;
    info.code_size = code_size;
    info.entrypoint = "main";
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage = stage;
    info.num_uniform_buffers = num_uniform_buf;
    info.num_samplers = num_samplers;

    SDL_GPUShader *shader = SDL_CreateGPUShader(gpu, &info);

    if (shader == NULL) {
        SDL_Log("Impossible de créer le shader %s : %s", shader_file, SDL_GetError());
    }

    SDL_free(code);

    return shader;
}

Model *load_model(Renderer *renderer, char *obj_path, char *tx_path) {
    Model *res = SDL_calloc(1, sizeof(Model));
    obj_data data = load_obj(obj_path);
    vertex_data *vertices = SDL_malloc(data.face_count * sizeof(vertex_data));
    Uint16 *indices = SDL_malloc(data.face_count * sizeof(Uint16));
    for (size_t i = 0; i < data.face_count; i++) {
        obj_face_idx face = data.faces[i];

        SDL_memcpy(vertices[i].pos, data.pos[face.pos], sizeof(vec3));
        SDL_memcpy(vertices[i].uv,  data.uvs[face.uv],  sizeof(vec2));
        vertices[i].uv[1] = 1.0f - vertices[i].uv[1];   // flip du V
        vertices[i].color = WHITE;
        indices[i] = (Uint16)i;
    }

    res->mesh.num_indecies = data.face_count;
    
    size_t vertices_byte_size = data.face_count * sizeof(vertices[0]);
    size_t indices_byte_size  = data.face_count * sizeof(indices[0]);


    int height;
    int width;
    SDL_GetWindowSize(renderer->window, &width, &height);

    res->mesh.vertex_buffer = SDL_CreateGPUBuffer(renderer->gpu, &(SDL_GPUBufferCreateInfo){.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = (Uint32)vertices_byte_size});
    res->mesh.index_buffer = SDL_CreateGPUBuffer(renderer->gpu, &(SDL_GPUBufferCreateInfo){.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = (Uint32)indices_byte_size});
    SDL_GPUTransferBuffer *transfer_buf = SDL_CreateGPUTransferBuffer(renderer->gpu, &(SDL_GPUTransferBufferCreateInfo){.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = (Uint32)(vertices_byte_size + indices_byte_size)});
    void *transfer_mem = SDL_MapGPUTransferBuffer(renderer->gpu, transfer_buf, false);
    memcpy(transfer_mem, vertices, vertices_byte_size);
    memcpy(transfer_mem + vertices_byte_size, indices, indices_byte_size);
    SDL_UnmapGPUTransferBuffer(renderer->gpu, transfer_buf);


    SDL_GPUCommandBuffer *copy_cmd_buf = SDL_AcquireGPUCommandBuffer(renderer->gpu);
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(copy_cmd_buf);
    SDL_UploadToGPUBuffer(copy_pass, &(SDL_GPUTransferBufferLocation){.transfer_buffer = transfer_buf}, &(SDL_GPUBufferRegion){.buffer = res->mesh.vertex_buffer, .size = vertices_byte_size}, false);
    SDL_UploadToGPUBuffer(copy_pass, &(SDL_GPUTransferBufferLocation){.transfer_buffer = transfer_buf, .offset = (Uint32)vertices_byte_size}, &(SDL_GPUBufferRegion){.buffer = res->mesh.index_buffer, .size = indices_byte_size}, false);
    res->texture = IMG_LoadGPUTexture(renderer->gpu, copy_pass, tx_path, NULL, NULL);
    if (!res->texture) {
        SDL_Log("Texture error");
        return 0;
    }
    SDL_EndGPUCopyPass(copy_pass);
    if (!SDL_SubmitGPUCommandBuffer(copy_cmd_buf)) {
        SDL_Log("Impossible de soumettre le rendu : %s", SDL_GetError());
    }
    SDL_ReleaseGPUTransferBuffer(renderer->gpu, transfer_buf);
    return res;
}

bool create_pipeline(Renderer *renderer) {   
    SDL_GPUShader *vert_shader = load_shader(renderer->gpu, "./shaders/build/shader.vert", 1, 0);
    SDL_GPUShader *frag_shader = load_shader(renderer->gpu, "./shaders/build/shader.frag", 0, 1);

    SDL_GPUVertexAttribute vertex_attrs[] = {
        {.location = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = (Uint32)offsetof(vertex_data, pos)},
        {.location = 1, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = (Uint32)offsetof(vertex_data, color)},
        {.location = 2, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = (Uint32)offsetof(vertex_data, uv)},
    };

    renderer->pipeline = SDL_CreateGPUGraphicsPipeline(renderer->gpu, &(SDL_GPUGraphicsPipelineCreateInfo){
        .vertex_shader = vert_shader,
        .fragment_shader = frag_shader,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_input_state = (SDL_GPUVertexInputState){
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = &(SDL_GPUVertexBufferDescription){
                .slot = 0,
                .pitch = sizeof(vertex_data)
            },
            .num_vertex_attributes = (Uint32)(sizeof(vertex_attrs) / sizeof(vertex_attrs[0])),
            .vertex_attributes = vertex_attrs
        },
        .depth_stencil_state = (SDL_GPUDepthStencilState){
            .enable_depth_test = true,
            .enable_depth_write = true,
            .compare_op = SDL_GPU_COMPAREOP_LESS
        },
        .rasterizer_state = (SDL_GPURasterizerState){
            .cull_mode = SDL_GPU_CULLMODE_BACK
        },
        .target_info = (SDL_GPUGraphicsPipelineTargetInfo){
            .num_color_targets = 1,
            .color_target_descriptions = &(SDL_GPUColorTargetDescription){
                .format = SDL_GetGPUSwapchainTextureFormat(renderer->gpu, renderer->window),
            },
            .has_depth_stencil_target = true,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM
        }
    });
    
    if (renderer->pipeline == NULL) {
        SDL_Log("Impossible de créer le pipeline : %s", SDL_GetError());
        return false;
    }
    
    SDL_ReleaseGPUShader(renderer->gpu, vert_shader);
    SDL_ReleaseGPUShader(renderer->gpu, frag_shader);

    return true;
}


Renderer *renderer_create(SDL_Window *window)
{
    Renderer *renderer = SDL_calloc(1, sizeof(*renderer));
    if (renderer == NULL) {
        SDL_Log("Mémoire insuffisante pour Renderer.");
        return NULL;
    }

    renderer->window = window;

    renderer->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (renderer->gpu == NULL) {
        SDL_Log("Impossible de créer le GPU : %s", SDL_GetError());
        //renderer_destroy(renderer);
        return NULL;
    }

    if (!SDL_ClaimWindowForGPUDevice(renderer->gpu, window)) {
        SDL_Log("Impossible d'associer la fenêtre au GPU : %s", SDL_GetError());
        //renderer_destroy(renderer);
        return NULL;
    }

    if (!SDL_SetGPUSwapchainParameters(renderer->gpu, renderer->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR, SDL_GPU_PRESENTMODE_VSYNC)) {
        SDL_Log("Impossible de mettre les paramettre du swapchain : %s", SDL_GetError());
        //renderer_destroy(renderer);
        return NULL;
    }

    SDL_GetWindowSize(renderer->window, &renderer->window_size_w, &renderer->window_size_h);
    renderer->depth_texture_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    renderer->depth_texture = SDL_CreateGPUTexture(renderer->gpu, &(SDL_GPUTextureCreateInfo){
        .format = SDL_GPU_TEXTUREFORMAT_D24_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .height = renderer->window_size_h, .width = renderer->window_size_w,
        .layer_count_or_depth = 1,
        .num_levels = 1
    });
    if (renderer->depth_texture == NULL) {
        SDL_Log("Depth_Texture error : %s", SDL_GetError());
        return NULL;
    }
    

    return renderer;
}

void renderer_draw(Renderer *renderer, mat4 view_mat) {
    
    SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(renderer->gpu);
    if (cmd_buf == NULL) {
        SDL_Log("Impossible d'acquérir un command buffer : %s", SDL_GetError());
        return;
    }
    
    SDL_GPUTexture *swapchain_tex = NULL;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, renderer->window, &swapchain_tex, NULL, NULL)) {
        SDL_CancelGPUCommandBuffer(cmd_buf);
        return;
    }
    
    struct UBO {
        mat4 mvp;
    };
    struct UBO ubo;

    int height;
    int width;
    SDL_GetWindowSize(renderer->window, &width, &height);
    mat4 projection;
    glm_perspective(glm_rad(70.0f), (float)width / (float)height, 0.0001f, 10000.0f, projection);
    mat4 model_mat;
    glm_mat4_identity(model_mat);
    glm_translate(model_mat, (float[]){0, 0, 0});
    static double angle = 0;
    //angle += 0.01;
    glm_rotate(model_mat, angle, (float[]){0, 1, 0});
    glm_mat4_mul(view_mat, model_mat, ubo.mvp);
    glm_mat4_mul(projection, ubo.mvp, ubo.mvp);

    SDL_GPUColorTargetInfo color_target = {0};
    color_target.texture = swapchain_tex;
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.clear_color = (SDL_FColor){ 0.08f, 0.10f, 0.16f, 1.0f };
    color_target.store_op = SDL_GPU_STOREOP_STORE;


    SDL_GPUDepthStencilTargetInfo depth_info = {
        .texture = renderer->depth_texture,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .clear_depth = 1,
        .store_op = SDL_GPU_STOREOP_DONT_CARE,
    };
    SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(cmd_buf, &color_target, 1, &depth_info);
    SDL_BindGPUGraphicsPipeline(render_pass, renderer->pipeline);
    SDL_BindGPUVertexBuffers(render_pass, 0, &(SDL_GPUBufferBinding){.buffer = renderer->models->mesh.vertex_buffer}, 1);
    SDL_BindGPUIndexBuffer(render_pass, &(SDL_GPUBufferBinding){.buffer = renderer->models->mesh.index_buffer}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    SDL_PushGPUVertexUniformData(cmd_buf, 0, &ubo, sizeof(ubo));
    SDL_BindGPUFragmentSamplers(render_pass, 0, &(SDL_GPUTextureSamplerBinding){.texture = renderer->models->texture, .sampler = renderer->sampler}, 1);
    SDL_DrawGPUIndexedPrimitives(render_pass, renderer->models->mesh.num_indecies, 1, 0, 0, 0);
    SDL_EndGPURenderPass(render_pass);

    if (!SDL_SubmitGPUCommandBuffer(cmd_buf)) {
        SDL_Log("Impossible de soumettre le rendu : %s", SDL_GetError());
    }
}

void renderer_destroy(Renderer *renderer)
{
    if (renderer == NULL) {
        return;
    }

    if (renderer->gpu != NULL) {
        if (renderer->pipeline != NULL) {
            SDL_ReleaseGPUGraphicsPipeline(renderer->gpu, renderer->pipeline);
        }
        SDL_ReleaseWindowFromGPUDevice(renderer->gpu, renderer->window);
        SDL_DestroyGPUDevice(renderer->gpu);
    }

    SDL_free(renderer);
}
