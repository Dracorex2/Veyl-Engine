#include "mesh.h"

static void add_vertex(
    Vertex *v,
    float x, float y, float z,
    float r, float g, float b
) {
    v->x = x;
    v->y = y;
    v->z = z;
    v->r = r;
    v->g = g;
    v->b = b;
    v->a = 1.0f;
}

/* Ajoute un rectangle composé de deux triangles.
   Les quatre coins sont donnés dans l'ordre : a, b, c, d. */
static void add_face(
    Vertex *v,
    int *index,
    float ax, float ay, float az,
    float bx, float by, float bz,
    float cx, float cy, float cz,
    float dx, float dy, float dz,
    float r, float g, float b
) {
    /* Triangle 1 : a, b, c */
    add_vertex(&v[(*index)++], ax, ay, az, r, g, b);
    add_vertex(&v[(*index)++], bx, by, bz, r, g, b);
    add_vertex(&v[(*index)++], cx, cy, cz, r, g, b);

    /* Triangle 2 : a, c, d */
    add_vertex(&v[(*index)++], ax, ay, az, r, g, b);
    add_vertex(&v[(*index)++], cx, cy, cz, r, g, b);
    add_vertex(&v[(*index)++], dx, dy, dz, r, g, b);
}

void mesh_create_cube(Vertex vertices[CUBE_VERTEX_COUNT])
{
    int i = 0;

    /* Face avant : Z positif, rouge */
    add_face(vertices, &i,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        1.0f, 0.2f, 0.2f);

    /* Face arrière : Z négatif, bleu */
    add_face(vertices, &i,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        0.2f, 0.3f, 1.0f);

    /* Face gauche : vert */
    add_face(vertices, &i,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        0.2f, 0.8f, 0.3f);

    /* Face droite : jaune */
    add_face(vertices, &i,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
        1.0f, 0.8f, 0.2f);

    /* Face du dessus : violet */
    add_face(vertices, &i,
        -0.5f, 0.5f,  0.5f,
         0.5f, 0.5f,  0.5f,
         0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        0.8f, 0.3f, 1.0f);

    /* Face du dessous : gris */
    add_face(vertices, &i,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        0.5f, 0.5f, 0.5f);
}