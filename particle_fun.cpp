#include <stdint.h>
#include <math.h>
#include <emmintrin.h>

#include "particle_fun.h"

#define PARTICLE_MASS 1
#define COEFFICIENT_OF_DRAG 0.01f

static V3 gravity; 

static uint32_t parkmiller_rand(uint32_t *state) {
	const uint32_t A = 48271;

	uint32_t low  = (*state & 0x7fff) * A;			// max: 32,767 * 48,271 = 1,581,695,857 = 0x5e46c371
	uint32_t high = (*state >> 15)    * A;			// max: 65,535 * 48,271 = 3,163,439,985 = 0xbc8e4371
	uint32_t x = low + ((high & 0xffff) << 15) + (high >> 16);	// max: 0x5e46c371 + 0x7fff8000 + 0xbc8e = 0xde46ffff

	x = (x & 0x7fffffff) + (x >> 31);
	*state = x;
    return *state;
}


#if DEBUG_MODE

static void create_random_particles(GameState *state, 
                                    int number_of_particles, 
                                    uint32_t seed) {

    for (int i = 0; i < number_of_particles; i++) { 
        //assert(state->particle_count < arraysize(state->particles));
        state->particles.mass[state->particle_count] = (float)(parkmiller_rand(&seed) % 30) + 10;
        state->particles.radius[state->particle_count] = state->particles.mass[state->particle_count];
        state->particles.pos[state->particle_count].x = (float)(parkmiller_rand(&seed)%WORLD_WIDTH)-WORLD_RIGHT;
        state->particles.pos[state->particle_count].y = (float)(parkmiller_rand(&seed)%WORLD_HEIGHT)-WORLD_TOP;
        state->particles.pos[state->particle_count].z = (float)(parkmiller_rand(&seed)%WORLD_DEPTH)-WORLD_FORWARD;
#if 1
        state->particles.vel[state->particle_count].x = (float)(parkmiller_rand(&seed)%20)-10;
        state->particles.vel[state->particle_count].y = (float)(parkmiller_rand(&seed)%20)-10;
        state->particles.vel[state->particle_count].z = 0;
#endif
        state->particles.f_accumulator[state->particle_count] = {};

        state->particle_count++;
    }
}

static void create_side_by_side_particles(GameState *state, 
                                    int number_of_particles, 
                                    V3 pad,
                                    V3 start) {

    for (int i = 0; i < number_of_particles; i++) { 
        //assert(state->particle_count < arraysize(state->particles));
        state->particles.mass[state->particle_count] = 1;
        state->particles.radius[state->particle_count] = state->particles.mass[state->particle_count];
        state->particles.pos[state->particle_count] = ((float)i*pad)+start;
        state->particles.f_accumulator[state->particle_count] = {};

        state->particle_count++;
    }
}

static void create_cube(GameState *state, V3 pos) {

    int vertex0 = state->vertex_count++;
    int vertex1 = state->vertex_count++; 
    int vertex2 = state->vertex_count++;
    int vertex3 = state->vertex_count++;
    int vertex4 = state->vertex_count++;
    int vertex5 = state->vertex_count++; 
    int vertex6 = state->vertex_count++;
    int vertex7 = state->vertex_count++;

    state->vertex_list[vertex0] = v3(pos.x, pos.y, pos.z);
    state->vertex_list[vertex1] = v3(pos.x, pos.y, pos.z+1);
    state->vertex_list[vertex2] = v3(pos.x, pos.y+1, pos.z);
    state->vertex_list[vertex3] = v3(pos.x, pos.y+1, pos.z+1);
    state->vertex_list[vertex4] = v3(pos.x+1, pos.y, pos.z);
    state->vertex_list[vertex5] = v3(pos.x+1, pos.y, pos.z+1);
    state->vertex_list[vertex6] = v3(pos.x+1, pos.y+1, pos.z);
    state->vertex_list[vertex7] = v3(pos.x+1, pos.y+1, pos.z+1);

    Triangle *triangle0 = &state->polygons[state->polygon_count++];
    Triangle *triangle1 = &state->polygons[state->polygon_count++];
    Triangle *triangle2 = &state->polygons[state->polygon_count++];
    Triangle *triangle3 = &state->polygons[state->polygon_count++];
    Triangle *triangle4 = &state->polygons[state->polygon_count++];
    Triangle *triangle5 = &state->polygons[state->polygon_count++];
    Triangle *triangle6 = &state->polygons[state->polygon_count++];
    Triangle *triangle7 = &state->polygons[state->polygon_count++];
    Triangle *triangle8 = &state->polygons[state->polygon_count++];
    Triangle *triangle9 = &state->polygons[state->polygon_count++];
    Triangle *triangle10 = &state->polygons[state->polygon_count++];
    Triangle *triangle11 = &state->polygons[state->polygon_count++];

    triangle0->v1  = vertex0;
    triangle0->v2  = vertex2;
    triangle0->v3  = vertex4;
    triangle0->color = WHITE;

    triangle1->v1  = vertex2;
    triangle1->v2  = vertex6;
    triangle1->v3  = vertex4;
    triangle1->color = WHITE;

    triangle2->v1  = vertex4;
    triangle2->v2  = vertex6;
    triangle2->v3  = vertex5;
    triangle2->color = RED;

    triangle3->v1  = vertex6;
    triangle3->v2  = vertex7;
    triangle3->v3  = vertex5;
    triangle3->color = RED;

    triangle4->v1  = vertex5;
    triangle4->v2  = vertex7;
    triangle4->v3  = vertex1;
    triangle4->color = GREEN;

    triangle5->v1  = vertex7;
    triangle5->v2  = vertex3;
    triangle5->v3  = vertex1;
    triangle5->color = GREEN;

    triangle6->v1  = vertex1;
    triangle6->v2  = vertex3;
    triangle6->v3  = vertex0;
    triangle6->color = BLUE;

    triangle7->v1  = vertex3;
    triangle7->v2  = vertex2;
    triangle7->v3  = vertex0;
    triangle7->color = BLUE;

    triangle8->v1  = vertex2;
    triangle8->v2  = vertex3;
    triangle8->v3  = vertex6;
    triangle8->color = PINK;

    triangle9->v1  = vertex3;
    triangle9->v2  = vertex7;
    triangle9->v3  = vertex6;
    triangle9->color = PINK;

    triangle10->v1  = vertex1;
    triangle10->v2  = vertex0;
    triangle10->v3  = vertex5;
    triangle10->color = CYAN;

    triangle11->v1  = vertex0;
    triangle11->v2  = vertex4;
    triangle11->v3  = vertex5;
    triangle11->color = CYAN;
}
#endif

inline static void spring_apply_force(Spring *spring) {

    V3 l = spring->p1->pos - spring->p2->pos;
    V3 dl = spring->p1->vel - spring->p2->vel;

    V3 force = -((spring->spring_const*(v3_mag(l) - spring->rest_length)) + 
            spring->damping_const*((v3_dot(l, dl))/v3_mag(l))) * (l/v3_mag(l));

    spring->p1->f_accumulator += force;

    spring->p2->f_accumulator += -force;
}

void game_update_and_render(GameMemory *memory, 
                            OffscreenBuffer *buffer, 
                            OffscreenBuffer *zbuffer,
                            GameInput *input) {


    GameState *state = (GameState *)memory->permanent_storage;
    if (!memory->is_initialised) {
        //set GRAVITY
        gravity.y = -9.81f; 
        ReadFileResult sphere_mesh = PlatformReadFile("sphere.obj");
        for (int i = 0; i > sphere_mesh.size; ++i) {
            //find first character begging with v
            if (((char *)sphere_mesh.file)[i]== 'v') {
            }
        }
#if DEBUG_MODE
        create_random_particles(state, 1000, 120093);
        create_side_by_side_particles(state, 10, v3(10,0,0), v3(0,0,0));
        create_side_by_side_particles(state, 10, v3(0,0,10), v3(0,0,0));
#endif
        
#if 0
        Spring *spring = &state->springs[state->spring_count++];
        spring->p1 = &state->particles[state->particle_count++];
        spring->p2 = &state->particles[state->particle_count++];
        spring->spring_const = 3.0f;
            spring->damping_const = 0.2f;
        spring->rest_length = 3;

        spring->p1->mass = 1;
        spring->p1->radius = 1;
        spring->p1->pos.x = 0;
        spring->p1->pos.y = 0; 
        spring->p1->has_drag = true;
        spring->p1->has_grav = true;
        spring->p1->is_anchor = true;

        spring->p2->mass = 1;
        spring->p2->radius = 1;
        spring->p2->pos.x = 0 + 5;
        spring->p2->pos.y = 0;
        spring->p2->has_drag = true;
        spring->p2->has_grav = true;
        spring->p2->is_anchor = false;
#endif

        state->camera.width = 300;
        state->camera.height = 300;
        state->camera.pos.x = 0;
        state->camera.pos.y = 0;
        state->camera.pos.z = 0;
        state->camera.zfar = 5000; 
        state->camera.znear = 1; 
        state->camera.fov = PI/3.0f;
        state->move_speed = 1.0f;
        memory->is_initialised = true;
    }


#if DEBUG_MODE
    if (state->frame_counter >= GAME_UPDATE_HZ) {
        state->frame_counter = 0;
    }
#endif

    state->vertex_count = 0;
    state->screen_vertex_count = 0;
    state->polygon_count = 0;
    state->draw_count = 0;

    renderer_draw_background(buffer, 0xFFFF00FF); 

    //clear z buffer every frame
    uint32_t zbuffer_size = zbuffer->width * zbuffer->height;
    float *depth_value = ((float *)zbuffer->memory);
    for (uint32_t i = 0; i < zbuffer_size; ++i) {
        *depth_value++ = FLT_MAX;
    }


    float timestep = TIME_FOR_FRAME;

    create_cube(state, v3(10,10,-30));

    {
        V3 move = {};
        if (input->action) {
            state->move_speed *= 10.0f;
            if (state->move_speed >= 50) {
                state->move_speed = 0.05f;
            }
        }

        if (input->camleft) {
            state->camera.theta_y -= PI/100.0f;
        }

        if (input->camright) {
            state->camera.theta_y += PI/100.0f;
        }

        if (input->camdown) {
            state->camera.theta_x += PI/100.0f;

        }

        if (input->camup) {
            state->camera.theta_x -= PI/100.0f;
        }

        if (input->left) {
            move -= v3_rotate_on_axis(v3(0,1,0), state->camera.theta_y, v3(1,0,0));
        }

        if (input->right) {
            move += 
                v3_rotate_on_axis(v3(0,1,0), state->camera.theta_y, v3(1,0,0));
        }

        if (input->down) {
            move -= 
                v3_rotate_q4(v3(0,0,1), 
                        (rotation_q4(state->camera.theta_y, v3(0,1,0)) * 
                         rotation_q4(state->camera.theta_x, v3(1,0,0))));

        }

        if (input->up) {
            move += 
                v3_rotate_q4(v3(0,0,1), 
                        (rotation_q4(state->camera.theta_y, v3(0,1,0)) * 
                         rotation_q4(state->camera.theta_x, v3(1,0,0))));

        }

        state->camera.pos += state->move_speed*v3_norm(move);


        if (fabs(state->camera.theta_x) > PI/2) {
            state->camera.theta_x = (state->camera.theta_x > 0) ? PI/2 : -PI/2;
        }

        if (fabs(state->camera.theta_y) >= 2*PI) {
            state->camera.theta_y = 0;
        }



        if (state->camera.width > WORLD_WIDTH) {
            state->camera.width = WORLD_WIDTH;
        }

        if (state->camera.height > WORLD_HEIGHT) {
            state->camera.height = WORLD_HEIGHT;
        }

        if ((state->camera.pos.x + state->camera.width/2.0f) > WORLD_RIGHT) {
            state->camera.pos.x = (WORLD_RIGHT - state->camera.width/2.0f);
        }

        if ((state->camera.pos.x - state->camera.width/2.0f) < WORLD_LEFT) {
            state->camera.pos.x = (WORLD_LEFT + state->camera.width/2.0f);
        }

        if ((state->camera.pos.y + state->camera.height/2.0f) > WORLD_TOP) {
            state->camera.pos.y = (WORLD_TOP - state->camera.height/2.0f);
        }

        if ((state->camera.pos.y - state->camera.height/2.0f) < WORLD_BOTTOM) {
            state->camera.pos.y = (WORLD_BOTTOM + state->camera.height/2.0f);
        }
    }

    // transform particles

    int cube_count = 0;
    int screen_count = 0;
    for (int i = 0;i < state->particle_count; i++) {

#if SSE
        V2Screen4 screen[2];
        Vertex4Cube *cube = &state->particle_vert[cube_count++];
#else
        int vertex0 = state->vertex_count++;
        int vertex1 = state->vertex_count++; 
        int vertex2 = state->vertex_count++;
        int vertex3 = state->vertex_count++;
        int vertex4 = state->vertex_count++;
        int vertex5 = state->vertex_count++; 
        int vertex6 = state->vertex_count++;
        int vertex7 = state->vertex_count++;

        state->vertex_list[vertex0] = v3(state->particles.pos[i].x, state->particles.pos[i].y, state->particles.pos[i].z);
        state->vertex_list[vertex1] = v3(state->particles.pos[i].x, state->particles.pos[i].y, state->particles.pos[i].z+1);
        state->vertex_list[vertex2] = v3(state->particles.pos[i].x, state->particles.pos[i].y+1, state->particles.pos[i].z);
        state->vertex_list[vertex3] = v3(state->particles.pos[i].x, state->particles.pos[i].y+1, state->particles.pos[i].z+1);
        state->vertex_list[vertex4] = v3(state->particles.pos[i].x+1, state->particles.pos[i].y, state->particles.pos[i].z);
        state->vertex_list[vertex5] = v3(state->particles.pos[i].x+1, state->particles.pos[i].y, state->particles.pos[i].z+1);
        state->vertex_list[vertex6] = v3(state->particles.pos[i].x+1, state->particles.pos[i].y+1, state->particles.pos[i].z);
        state->vertex_list[vertex7] = v3(state->particles.pos[i].x+1, state->particles.pos[i].y+1, state->particles.pos[i].z+1);

#endif

        Triangle *triangle0 = &state->polygons[state->polygon_count++];
        Triangle *triangle1 = &state->polygons[state->polygon_count++];
        Triangle *triangle2 = &state->polygons[state->polygon_count++];
        Triangle *triangle3 = &state->polygons[state->polygon_count++];
        Triangle *triangle4 = &state->polygons[state->polygon_count++];
        Triangle *triangle5 = &state->polygons[state->polygon_count++];
        Triangle *triangle6 = &state->polygons[state->polygon_count++];
        Triangle *triangle7 = &state->polygons[state->polygon_count++];
        Triangle *triangle8 = &state->polygons[state->polygon_count++];
        Triangle *triangle9 = &state->polygons[state->polygon_count++];
        Triangle *triangle10 = &state->polygons[state->polygon_count++];
        Triangle *triangle11 = &state->polygons[state->polygon_count++];

        triangle0->v1  = vertex0;
        triangle0->v2  = vertex2;
        triangle0->v3  = vertex4;
        triangle0->color = WHITE;

        triangle1->v1  = vertex2;
        triangle1->v2  = vertex6;
        triangle1->v3  = vertex4;
        triangle1->color = WHITE;

        triangle2->v1  = vertex4;
        triangle2->v2  = vertex6;
        triangle2->v3  = vertex5;
        triangle2->color = RED;

        triangle3->v1  = vertex6;
        triangle3->v2  = vertex7;
        triangle3->v3  = vertex5;
        triangle3->color = RED;

        triangle4->v1  = vertex5;
        triangle4->v2  = vertex7;
        triangle4->v3  = vertex1;
        triangle4->color = GREEN;

        triangle5->v1  = vertex7;
        triangle5->v2  = vertex3;
        triangle5->v3  = vertex1;
        triangle5->color = GREEN;

        triangle6->v1  = vertex1;
        triangle6->v2  = vertex3;
        triangle6->v3  = vertex0;
        triangle6->color = BLUE;

        triangle7->v1  = vertex3;
        triangle7->v2  = vertex2;
        triangle7->v3  = vertex0;
        triangle7->color = BLUE;

        triangle8->v1  = vertex2;
        triangle8->v2  = vertex3;
        triangle8->v3  = vertex6;
        triangle8->color = PINK;

        triangle9->v1  = vertex3;
        triangle9->v2  = vertex7;
        triangle9->v3  = vertex6;
        triangle9->color = PINK;

        triangle10->v1  = vertex1;
        triangle10->v2  = vertex0;
        triangle10->v3  = vertex5;
        triangle10->color = CYAN;

        triangle11->v1  = vertex0;
        triangle11->v2  = vertex4;
        triangle11->v3  = vertex5;
        triangle11->color = CYAN;

#if SSE
        cube->vertices[0].x = _mm_set1_ps(state->particles.pos[i].x);
        cube->vertices[1].x = _mm_set1_ps(state->particles.pos[i].x+1);

        cube->vertices[0].y = _mm_set_ps(state->particles.pos[i].y, 
                                          state->particles.pos[i].y,
                                          state->particles.pos[i].y+1,
                                          state->particles.pos[i].y+1);

        cube->vertices[1].y = _mm_set_ps(state->particles.pos[i].y, 
                                          state->particles.pos[i].y,
                                          state->particles.pos[i].y+1,
                                          state->particles.pos[i].y+1);

        cube->vertices[0].z = _mm_set_ps(state->particles.pos[i].z,
                                         state->particles.pos[i].z+1,
                                         state->particles.pos[i].z,
                                         state->particles.pos[i].z+1);

        cube->vertices[1].z = _mm_set_ps(state->particles.pos[i].z,
                                         state->particles.pos[i].z+1,
                                         state->particles.pos[i].z,
                                         state->particles.pos[i].z+1);

        renderer_vertex4_to_v2screen(cube->vertices, 
                            &state->camera, 
                            buffer->width, 
                            buffer->height, 
                            2, 
                            screen);

        // unpack
        V2Screen unpacked_screen[arraysize(screen)*4];

        unpacked_screen[0].x = ((int *)&screen[0].x)[3];
        unpacked_screen[0].y = ((int *)&screen[0].y)[3];

        unpacked_screen[1].x = ((int *)&screen[0].x)[2];
        unpacked_screen[1].y = ((int *)&screen[0].y)[2];

        unpacked_screen[2].x = ((int *)&screen[0].x)[1];
        unpacked_screen[2].y = ((int *)&screen[0].y)[1];

        unpacked_screen[3].x = ((int *)&screen[0].x)[0];
        unpacked_screen[3].y = ((int *)&screen[0].y)[0];

        unpacked_screen[4].x = ((int *)&screen[1].x)[3];
        unpacked_screen[4].y = ((int *)&screen[1].y)[3];

        unpacked_screen[5].x = ((int *)&screen[1].x)[2];
        unpacked_screen[5].y = ((int *)&screen[1].y)[2];

        unpacked_screen[6].x = ((int *)&screen[1].x)[1];
        unpacked_screen[6].y = ((int *)&screen[1].y)[1];

        unpacked_screen[7].x = ((int *)&screen[1].x)[0];
        unpacked_screen[7].y = ((int *)&screen[1].y)[0];

        renderer_draw_triangles_filled(buffer, 
                                       unpacked_screen, 
                                       triangles, 
                                       colors, 
                                       arraysize(triangles));
#endif

       // V2Screen4_draw_triangles(buffer, screen, triangles, 0xFFFF00FF, 12);
    }

    //draw every spring
    for (int i = 0;i < state->spring_count; ++i) {
        Spring *spring = &state->springs[i];
        //render_line(buffer, start_pos, end_pos);
    }

    // TODO: render mouse_spring
            
    // update the simulation time until it syncs with the time after 1 video frame
    float t = state->time + TIME_FOR_FRAME;
    while (state->time < t) {


        // check for collisions and apply global forces

        for (int i = 0; i < state->particle_count; ++i) {

            float particle_left = state->particles.pos[i].x - (state->particles.radius[i]/2.0f);
            float particle_right = state->particles.pos[i].x + (state->particles.radius[i]/2.0f);
            float particle_top = state->particles.pos[i].y + (state->particles.radius[i]/2.0f);
            float particle_bottom = state->particles.pos[i].y - (state->particles.radius[i]/2.0f);

            float particle_x = state->particles.pos[i].x;
            float particle_y = state->particles.pos[i].y;

            float particle_dx = state->particles.pos[i].x;
            float particle_dy = state->particles.vel[i].y;

            if (particle_right > WORLD_RIGHT) {
                particle_dx = -particle_dx;
            }

            if (particle_left < WORLD_LEFT) {
                particle_dx = -particle_dx;
            }


            if (particle_top > WORLD_TOP) {
                particle_dy = -particle_dy;
            }

            if (particle_bottom < WORLD_BOTTOM) {
                particle_dy = -particle_dy;
            }


            //update spatial mask
            //
#if 0
            int left_grid_pos = (int)floorf((particles.pos.x - 
                        particles.radius) / 
                    (WORLD_WIDTH / 16.0f) + 8);

            int right_grid_pos = (int)floorf((particle->pos.x + 
                        particle->radius) / 
                   (WORLD_WIDTH / 16.0f) + 8);

            particle->spatial_mask = (1 << (left_grid_pos-1));
            particle->spatial_mask = particle->spatial_mask | (1 << (right_grid_pos-1));

            int top_grid_pos = (int)floorf(((particle->pos.y - particle->radius) / (WORLD_HEIGHT / 16.0f))+8);
            int bottom_grid_pos = (int)floorf(((particle->pos.y + particle->radius) / (WORLD_HEIGHT / 16.0f))+8);
            particle->spatial_mask = particle->spatial_mask | (1 << (16+top_grid_pos-1));
            particle->spatial_mask = particle->spatial_mask | (1 << (16+bottom_grid_pos));
#endif

#if 0 
            for (int j = 0; j< state->particle_count; ++j) {
                Particle *potential_collider = &state->particles[j];
                if (!potential_collider->active) {
                    continue;
                }
                
                // particles cant collide with themselves
                if (potential_collider == particle) {
                    continue;
                }
                
                // if not in the same grid then move onto the next
                int particle_x_mask = (particle->spatial_mask & 0x0000FFFF); // low 16
                int particle_y_mask = (particle->spatial_mask & 0xFFFF0000); // high 16
                                                                            //
                int collider_x_mask = (potential_collider->spatial_mask & 0x0000FFFF); // low 16
                int collider_y_mask = (potential_collider->spatial_mask & 0xFFFF0000);// high 16
                // if not colliding

                if (!((particle_x_mask & collider_x_mask) && 
                        (particle_y_mask & collider_y_mask))) {

                    continue;
                }

                
                if ((v3_mag(potential_collider->pos - particle->pos)) >= 
                    (particle->radius + potential_collider->radius)) {
                    continue;
                }

                if(particle == player) {
                    if (player->mass > potential_collider->mass) {
                        player->mass += 0.5f;    
                        potential_collider->active = 0;
                        continue;
                    }
                    
                    else {
                        player->mass *= 0.5f;
                        if (player->mass < 0.1f) {
                            player->mass = 0.1f;
                        }
                    }

                }

                V3 unit_normal = (potential_collider->pos - particle->pos) / 
                                 (v3_mag(potential_collider->pos - particle->pos));
                
                V3 unit_tangent = {};
                unit_tangent.x = -unit_normal.y;
                unit_tangent.y = unit_normal.x;

                float p1_dot = v3_dot((particle->vel-potential_collider->vel),
                                           (particle->pos-potential_collider->pos));

                float p2_dot = v3_dot((potential_collider->vel-particle->vel),
                                           (potential_collider->pos-particle->pos));


                
                V3 p1_after_vel =  ((2.0f*potential_collider->mass)/
                                  (particle->mass+potential_collider->mass)) *
                                 (p1_dot/((v3_mag(particle->pos-potential_collider->pos)) *
                                          (v3_mag(particle->pos-potential_collider->pos)))) *
                                 (particle->pos - potential_collider->pos);

                V3 p2_after_vel =  ((2.0f*particle->mass)/
                                  (particle->mass+potential_collider->mass)) *
                                 (p2_dot/((v3_mag(potential_collider->pos-particle->pos)) * 
                                          (v3_mag(potential_collider->pos-particle->pos)))) *
                                 (potential_collider->pos - particle->pos);

                //rollback for colliding particles
                while ((v3_mag(potential_collider->pos - particle->pos)) <
                        (particle->radius + potential_collider->radius)) {

                    particle->pos += -(timestep) * particle->vel;
                    potential_collider->pos += -(timestep) * potential_collider->vel;
                }

                particle->vel = particle->vel - p1_after_vel;
                potential_collider->vel = potential_collider->vel - p2_after_vel;
            }
#endif

            //zero forces
            state->particles.f_accumulator[i] = {}; 
            state->particles.f_accumulator[i] += -(COEFFICIENT_OF_DRAG*state->particles.vel[i]); 
        }

        // apply springs forces
        for (int i = 0; i < state->spring_count; ++i) {
            Spring *spring = &state->springs[i];
            spring_apply_force(spring);

        }

        if (state->mouse_spring.p1) {
            Spring *spring = &state->mouse_spring; 
            spring_apply_force(spring);
        }

        //update particles
        
        int n_particles = state->particle_count;
#if 1
        //update particles;
        for (int i = 0; i < arraysize(state->particles.pos); ++i) {
            state->particles.vel[i] += timestep * (state->particles.f_accumulator[i]/state->particles.mass[i]);
            state->particles.pos[i] += timestep * state->particles.vel[i];
        }

#endif

       renderer_world_vertices_to_screen_and_cull(state->vertex_list, 
                                                  state->vertex_count, 
                                                  state->polygons,
                                                  state->polygon_count,
                                                  &state->camera, 
                                                  buffer->width, 
                                                  buffer->height, 
                                                  state->polygons_to_draw,
                                                  &state->draw_count,
                                                  state->screen_vertices,
                                                  state->screen_vertex_count);


       renderer_draw_triangles_filled(buffer,
                                      zbuffer,
                                      state->screen_vertices,
                                      state->polygons_to_draw, 
                                      state->draw_count);

        state->time += timestep;
    }
} 
