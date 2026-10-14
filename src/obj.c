#include "obj.h"


// Agrandit un tableau dynamique (x2) si nécessaire
static void *grow(void *ptr, size_t *cap, size_t count, size_t elem_size) {
    if (count < *cap)
        return ptr;
    size_t new_cap = *cap ? *cap * 2 : 64;
    void *tmp = SDL_realloc(ptr, new_cap * elem_size);
    if (!tmp)
        return NULL;
    *cap = new_cap;
    return tmp;
}

// Parse "v", "v/vt", "v//vn" ou "v/vt/vn". Indices OBJ en base 1, négatifs acceptés.
static bool parse_face_vertex(const char **s, size_t pos_count, size_t uv_count,
                              obj_face_idx *out) {
    char *end;
    long v = SDL_strtol(*s, &end, 10);
    if (end == *s || v == 0)
        return false;
    long p = v > 0 ? v - 1 : (long)pos_count + v;
    if (p < 0 || (size_t)p >= pos_count)
        return false;
    out->pos = (unsigned int)p;
    out->uv = 0;
    *s = end;

    if (**s == '/') {
        (*s)++;
        if (**s != '/') {                       // présence d'un uv
            long t = SDL_strtol(*s, &end, 10);
            if (end != *s) {
                long u = t > 0 ? t - 1 : (long)uv_count + t;
                if (u >= 0 && (size_t)u < uv_count)
                    out->uv = (unsigned int)u;
                *s = end;
            }
        }
        if (**s == '/') {                       // normale : ignorée
            (*s)++;
            SDL_strtol(*s, &end, 10);
            *s = end;
        }
    }
    return true;
}

obj_data load_obj(char *path) {
    obj_data out = {0};
    size_t pos_cap = 0, uv_cap = 0, face_cap = 0;

    size_t size = 0;
    char *file = SDL_LoadFile(path, &size);   // le buffer est terminé par '\0'
    if (!file) {
        SDL_Log("load_obj: impossible de lire '%s': %s", path, SDL_GetError());
        return out;
    }

    const char *p = file;
    while (*p) {
        while (*p == ' ' || *p == '\t')
            p++;

        if (SDL_strncmp(p, "v ", 2) == 0) {
            const char *s = p + 2;
            char *end;
            vec3 v;
            v[0] = (float)SDL_strtod(s, &end); s = end;
            v[1] = (float)SDL_strtod(s, &end); s = end;
            v[2] = (float)SDL_strtod(s, &end);

            vec3 *tmp = grow(out.pos, &pos_cap, out.pos_count, sizeof(vec3));
            if (!tmp) goto fail;
            out.pos = tmp;
            SDL_memcpy(out.pos[out.pos_count++], v, sizeof(vec3));
        }
        else if (SDL_strncmp(p, "vt ", 3) == 0) {
            const char *s = p + 3;
            char *end;
            vec2 t;
            t[0] = (float)SDL_strtod(s, &end); s = end;
            t[1] = (float)SDL_strtod(s, &end);

            vec2 *tmp = grow(out.uvs, &uv_cap, out.uv_count, sizeof(vec2));
            if (!tmp) goto fail;
            out.uvs = tmp;
            SDL_memcpy(out.uvs[out.uv_count++], t, sizeof(vec2));
        }
        else if (SDL_strncmp(p, "f ", 2) == 0) {
            const char *s = p + 2;
            obj_face_idx first, prev, cur;
            int n = 0;

            while (*s && *s != '\n') {
                while (*s == ' ' || *s == '\t' || *s == '\r')
                    s++;
                if (!*s || *s == '\n')
                    break;
                if (!parse_face_vertex(&s, out.pos_count, out.uv_count, &cur))
                    break;

                if (n == 0) first = cur;
                else if (n >= 2) {
                    // triangulation en éventail : (first, prev, cur)
                    for (int k = 0; k < 3; k++) {
                        obj_face_idx *tmp = grow(out.faces, &face_cap,
                                                 out.face_count, sizeof(obj_face_idx));
                        if (!tmp) goto fail;
                        out.faces = tmp;
                        out.faces[out.face_count++] = (k == 0) ? first : (k == 1) ? prev : cur;
                    }
                }
                prev = cur;
                n++;
            }
        }

        // ligne suivante
        const char *nl = SDL_strchr(p, '\n');
        if (!nl) break;
        p = nl + 1;
    }

    SDL_free(file);
    return out;

fail:
    SDL_Log("load_obj: out of memory");
    SDL_free(file);
    SDL_free(out.pos);
    SDL_free(out.uvs);
    SDL_free(out.faces);
    return (obj_data){0};
}