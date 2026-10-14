#ifndef OBJ_H
#define OBJ_H

#include <SDL3/SDL.h>
#include <cglm/cglm.h>

typedef struct obj_face_idx {
    unsigned int pos;
    unsigned int uv;
} obj_face_idx;

typedef struct obj_data {
    vec3 *pos;
    vec2 *uvs;
    obj_face_idx *faces;   // liste de sommets, 3 par triangle
    size_t pos_count;
    size_t uv_count;
    size_t face_count;     // nombre de sommets dans faces (multiple de 3)
} obj_data;


obj_data load_obj(char *path);

#endif