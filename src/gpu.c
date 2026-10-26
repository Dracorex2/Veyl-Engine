#include "mesh.h"
#include "obj.h"
#include "renderer.h"

const SDL_FColor WHITE = {1, 1, 1, 1};

Model LoadGPUObjMesh(SDL_GPUDevice *device, SDL_GPUCopyPass *copy_pass, char *path) {
    Mesh res;

    obj_data data = load_obj(path);
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

    res.num_indecies = data.face_count;
    
    size_t vertices_byte_size = data.face_count * sizeof(vertices[0]);
    size_t indices_byte_size  = data.face_count * sizeof(indices[0]);
}