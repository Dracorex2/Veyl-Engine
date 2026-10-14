
#include <SDL3/SDL.h>
#include <cglm/cglm.h>
#include "renderer.h"

typedef struct Camera {
    vec3 pos;
    vec3 target;
} Camera;
typedef struct Look {
    float yaw;
    float pitch;
} Look;


#define EYE_HEIGH 1.8
#define MOVE_SPEED 0.1
#define MOUSE_SENCITIVITY 0.3

Camera camera = {
    .pos = {0, EYE_HEIGH, 4},
    .target = {0, EYE_HEIGH, 0},
};
Look look;

bool keys_down[SDL_SCANCODE_COUNT] = {false};
vec2 mouse_mouve;

void update_camera() {
    vec2 move_input = {0, 0};
    if (keys_down[SDL_SCANCODE_W])
        move_input[1] = 1;
    if (keys_down[SDL_SCANCODE_S])
        move_input[1] = -1;
    if (keys_down[SDL_SCANCODE_D])
        move_input[0] = 1;
    if (keys_down[SDL_SCANCODE_A])
        move_input[0] = -1;

    vec3 foreward = {0, 0, -1};
    vec3 right = {1, 0 ,0};
    
    vec2 look_input;
    glm_vec2_scale(mouse_mouve, MOUSE_SENCITIVITY, look_input);
    look.yaw = SDL_fmodf(look.yaw - look_input[0], 360);
    look.pitch  = glm_clamp(look.pitch - look_input[1], -89, 89);

    mat4 rot4;
    mat3 look_mat;
    vec3 angles = {glm_rad(look.pitch), glm_rad(look.yaw), 0};
    int order[3] = { 1, 0, 2 };
    glm_euler_by_order(angles, glm_euler_order(order), rot4);
    glm_mat4_pick3(rot4, look_mat);
    glm_mat3_mulv(look_mat, foreward, foreward);
    glm_mat3_mulv(look_mat, right, right);
    
    vec3 move_dir;
    vec3 res[2];
    glm_vec3_scale(foreward, move_input[1], res[0]);
    glm_vec3_scale(right, move_input[0], res[1]);
    glm_vec3_add(res[0], res[1], move_dir);
    glm_normalize(move_dir);

    vec3 motion;
    glm_vec3_scale(move_dir, MOVE_SPEED, motion);
    
    glm_vec3_add(camera.pos, motion, camera.pos);
    glm_vec3_add(camera.pos, foreward, camera.target);
    //SDL_Log("[%f, %f]", look.yaw, look.pitch);
}

int main(void) {

    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Erreur SDL : %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Mon moteur voxel", 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_SetWindowRelativeMouseMode(window, true);

    if (!window) {
        SDL_Log("Erreur fenêtre : %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Renderer *renderer = renderer_create(window);

    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        mouse_mouve[0] = 0;
        mouse_mouve[1] = 0;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            else if (event.type == SDL_EVENT_KEY_DOWN) 
                keys_down[event.key.scancode] = true;
            else if (event.type == SDL_EVENT_KEY_UP)
                keys_down[event.key.scancode] = false;
            else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mouse_mouve[0] += event.motion.xrel;
                mouse_mouve[1] += event.motion.yrel;
            }
        }
        update_camera();
        mat4 view_mat;
        vec3 up = {0, 1, 0};
        glm_lookat(camera.pos, camera.target, up, view_mat);
        renderer_draw(renderer, view_mat);
    }

    renderer_destroy(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}