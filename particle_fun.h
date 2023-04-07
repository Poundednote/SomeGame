#ifndef PHYSICS_H
#include <stdint.h>
#include "particle_math.h"
#include "renderer.cpp"

#define kilobytes(value) (value)*1024LL
#define megabytes(value) (kilobytes(value))*1024LL
#define gigabytes(value) (megabytes(value))*1024LL

#define assert(expression) if(!(expression)) {*(int *)0 = 0;}

#define arraysize(array) (sizeof(array) / sizeof((array)[0]))

#define PI 3.14159265359f
#define GAME_UPDATE_HZ 60
#define TIME_FOR_FRAME (1.0f / GAME_UPDATE_HZ)
#define MAX_PARTICLES (1024*7)

#define WORLD_WIDTH 10000
#define WORLD_HEIGHT 10000
#define WORLD_DEPTH 10000
#define WORLD_LEFT (-WORLD_WIDTH/2.0f)
#define WORLD_RIGHT (WORLD_WIDTH/2.0f)
#define WORLD_BOTTOM (-WORLD_HEIGHT/2.0f)
#define WORLD_TOP (WORLD_HEIGHT/2.0f)
#define WORLD_FORWARD (WORLD_DEPTH/2.0F)
#define WORLD_BACK (-WORLD_DEPTH/2.0F)

struct Particle {
    float mass;
    float radius; // used for collisons
 
    V3 pos;
    V3 vel;
    V3 f_accumulator;
    
    int spatial_mask;
};

struct ParticleSystem {
    float mass[MAX_PARTICLES];
    float radius[MAX_PARTICLES];
    V3 pos[MAX_PARTICLES];
    V3 vel[MAX_PARTICLES];
    V3 f_accumulator[MAX_PARTICLES];
    int spatial_mask[MAX_PARTICLES];
};

struct Spring {
    Particle *p1;
    Particle *p2;

    float spring_const;
    float damping_const;
    
    float rest_length;
};

struct GameInput {

    V2Screen mouse_pos;

    bool mouse_lbutton_down;
    bool mouse_lclickdrag;
    bool mouse_lbutton_click;

    bool left;
    bool right;
    bool up;
    bool down;

    bool action;

    bool camleft;
    bool camright;
    bool camdown;
    bool camup;

    bool camzoomin;
    bool camzoomout;


    V2Screen start_click_pos;
    
};

struct GameMemory {
    bool is_initialised;
    uint32_t permanent_stoarage_size;

    void* permanent_storage;
};

struct GameState {
    float time;

    ParticleSystem particles;
    int particle_count;

    Spring springs[256];
    int spring_count;

    Spring mouse_spring;

    GameCamera camera;

    bool gravity;

    Vertex4Cube particle_vert[MAX_PARTICLES];
    V2Screen4 reference_cubes_s[6*2];
    Vertex4Cube reference_cubes_v[6];
    V2Screen4 particle_screen[MAX_PARTICLES*2];

#if DEBUG_MODE
    uint32_t physics_update_counter;
    uint32_t frame_counter;
    float splat_time;
    bool is_splat;
#endif
};

#define PHYSICS_H
#endif // !PHYSICS_H