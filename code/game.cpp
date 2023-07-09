#define TILE_SIZE 16

#include "game.h"
#include "format_tmx.cpp"

#define BACKGROUND_COLOR rgb(52, 28, 39)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define sign(value) ((value) < 0 ? -1 : ((value) > 0 ? 1 : 0 ))

bool
box_collision(Entity* rigidbody, Entity* other, v2* step_velocity, bool resolve) {
    bool found = false;
    
    v2 step_position = rigidbody->p + *step_velocity;
    if (step_position.y + rigidbody->size.y > other->p.y && 
        step_position.y < other->p.y + other->size.y) {
        
        if (step_velocity->x < 0.0f && rigidbody->p.x >= other->p.x + other->size.x) {
            f32 x_overlap = other->p.x + other->size.x - step_position.x;
            if (x_overlap > 0.0f) {
                if (resolve) {
                    step_velocity->x = other->p.x + other->size.x - rigidbody->p.x;
                    rigidbody->velocity.x = 0.0f;
                }
                found = true;
            }
        } else if (step_velocity->x > 0.0f && rigidbody->p.x + rigidbody->size.x <= other->p.x) {
            f32 x_overlap = step_position.x - other->p.x + rigidbody->size.x;
            if (x_overlap > 0.0f) {
                if (resolve) {
                    step_velocity->x = other->p.x - rigidbody->size.x - rigidbody->p.x;
                    rigidbody->velocity.x = 0.0f;
                }
                found = true;
            }
        }
    }
    
    if (rigidbody->p.x + rigidbody->size.x > other->p.x && 
        rigidbody->p.x < other->p.x + other->size.x) {
        if (step_velocity->y < 0.0f && rigidbody->p.y < other->p.y + other->size.y &&
            rigidbody->p.y + rigidbody->size.y > other->p.y) {
            f32 y_overlap = step_position.y - other->p.y + other->size.y;
            if (y_overlap > 0.0f) {
                if (resolve) {
                    step_velocity->y = other->p.y + other->size.y - rigidbody->p.y;
                    rigidbody->velocity.y = 0.0f;
                }
                found = true;
            }
        } else if (step_velocity->y > 0.0f && rigidbody->p.y + rigidbody->size.y <= other->p.y) {
            f32 y_overlap = step_position.y - other->p.y + rigidbody->size.y;
            if (y_overlap > 0.0f) {
                if (resolve) {
                    step_velocity->y = other->p.y - rigidbody->size.y - rigidbody->p.y;
                    rigidbody->velocity.y = 0.0f;
                    rigidbody->is_grounded = true;
                }
                found = true;
            }
        }
    }
    
    return found;
}


bool
ray_box_collision(Ray ray, f32 min_dist, f32 max_dist, BoundingBox box) {
    RayCollision result = GetRayCollisionBox(ray, box);
    if (result.hit) {
        //pln("dist = %", result.distance);
        return result.distance >= min_dist && result.distance <= max_dist;
    }
    
    return false;
}


Entity*
spawn_entity(Game_State* state, Memory_Arena* arena, Entity_Type type) {
    Entity* entity = push_struct(arena, Entity);
    state->entity_count++;
    *entity = {};
    entity->type = type;
    
    if (!state->entities) {
        state->entities =  entity;
    }
    return entity;
}



Entity*
init_level(Game_State* state, Memory_Arena* arena) {
    clear(arena);
    
    state->entity_count = 0;
    
    
#if BUILD_DEBUG
    state->mode = Control_Boss_Enemy;
#else
    state->mode = Intro_Cutscene;
    state->cutscene_time = 0.0f;
#endif
    
    Loaded_Tmx tmx = read_tmx_map_data(string_lit("assets/interior.tmx"), arena);
    state->entities = tmx.entities;
    state->entity_count = tmx.entity_count;
    state->tile_map = tmx.tile_map;
    state->tile_map_width = tmx.tile_map_width;
    state->tile_map_height = tmx.tile_map_height;
    
    for (int i = 0; i < array_count(state->bullets); i++) {
        Entity* bullet = spawn_entity(state, arena, Bullet);
        state->bullets[i] = bullet;
        bullet->texture = &state->texture_bullet;
        bullet->max_speed.x = 3.0f;
        bullet->size = { 0.5f, 0.5f };
        bullet->is_rigidbody = true;
    }
    
    Entity* bullet = spawn_entity(state, arena, Charged_Bullet);
    state->charged_bullet = bullet;
    bullet->texture = &state->texture_charged_bullet;
    bullet->max_speed.x = 3.0f;
    bullet->size = { 0.75f, 0.75f };
    bullet->is_rigidbody = true;
    
    
    Entity* left_door = spawn_entity(state, arena, Door);
    state->left_door = left_door;
    left_door->texture = &state->texture_door;
    left_door->health = 1;
    left_door->p = { 0.0f, 10.0f };
    left_door->size = { 1.0f, 0.0f };
    left_door->is_rigidbody = false;
    
    Entity* right_door = spawn_entity(state, arena, Door);
    state->right_door = right_door;
    right_door->texture = &state->texture_door;
    right_door->health = 1;
    right_door->p = { 21.0f, 10.0f };
    right_door->size = { 1.0f, 0.0f };
    right_door->is_rigidbody = false;
    
    
    Entity* player = spawn_entity(state, arena, Player);
    state->player = player;
    player->texture = &state->texture_player;
#if BUILD_DEBUG
    player->p = {16.0f, 12.0f};
#else 
    player->p = {24.0f, 12.0f};
#endif
    player->size = {1.0f, 2.0f};
    player->max_speed.x = 3.0f;
    player->is_rigidbody = true;
    player->facing_dir = 1.0f;
    player->color = SKYBLUE;
    player->max_health = 1000;
    player->health = player->max_health;
    
    Entity* boss_enemy = spawn_entity(state, arena, Boss_Dragon);
    state->boss_enemy = boss_enemy;
    boss_enemy->flip_texture = true;
    boss_enemy->texture = &state->texture_dragon;
    boss_enemy->num_frames = 8;
    boss_enemy->frame_advance_rate = 2.5f;
#if BUILD_DEBUG
    boss_enemy->p = {3.0f, 10.0f};
#else 
    boss_enemy->p = {-5.0f, 10.0f};
#endif
    boss_enemy->size = {4.0f, 4.0f};
    boss_enemy->max_speed.x = 2.5f;
    boss_enemy->is_rigidbody = true;
    boss_enemy->color = RED;
    boss_enemy->max_health = 1000;
    boss_enemy->health = boss_enemy->max_health;
    
    return player;
}

inline s32
round_f32_to_s32(f32 value) {
    return (s32) round(value);
}

v2
to_pixel(Game_State* state, v2 world_p) {
    v2 result;
    result.x = roundf((world_p.x - state->camera_p.x) * state->meters_to_pixels);
    result.y = roundf((world_p.y - state->camera_p.y) * state->meters_to_pixels);
    return result;
}

inline v2
world_to_screen_space(v2 world_p, v2s camera_p, f32 scale) {
    v2 result;
    result.x = (f32) (round_f32_to_s32(world_p.x * scale) - camera_p.x);
    result.y = (f32) (round_f32_to_s32(world_p.y * scale) - camera_p.y);
    return result;
}

Color
rgb(u8 r, u8 g, u8 b, u8 a=255) {
    Color result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;
    return result;
}

void
draw_level_bounds(Game_State* state) {
    
    // Level bounds
    v2 size = vec2((f32) state->tile_map_width*TILE_SIZE + 1.0f, (f32) state->tile_map_width*TILE_SIZE);
    
    v2 cp;
    cp.x = -state->camera_p.x * state->meters_to_pixels;
    cp.y = -state->camera_p.y * state->meters_to_pixels;
    v2 min_cursor_p = cp;//world_to_screen_space(vec2(0.0f, 0.0f), cp, state->meters_to_pixels);
    v2 max_cursor_p = min_cursor_p + size + vec2(0.0f, 1.0f);
    
    Color background = BACKGROUND_COLOR;
    
    if (min_cursor_p.x > 0.0f) {
        int w = (int) min_cursor_p.x;
        DrawRectangle(0, 0, w, state->game_height, background);
    }
    
    if (max_cursor_p.x < size.y) {
        DrawRectangle((int) max_cursor_p.x, 0, state->game_width - (int) max_cursor_p.x, state->game_height, background);
    }
    
    if (min_cursor_p.y > 0.0f) {
        DrawRectangle(0, 0, state->game_width, (int) min_cursor_p.y, background);
    }
    
    if (max_cursor_p.y < size.y) {
        DrawRectangle(0, (int) max_cursor_p.y, 
                      state->game_width, state->game_height - (int) max_cursor_p.y, background);
        
    }
}

void
draw_health_bar(Game_State* game_state, Entity* entity, Color color, bool right=true) {
    f32 width = (f32) (game_state->game_width/2-8);
    width *= (f32) entity->health / entity->max_health;
    s32 xoffset = 0;
    s32 xoffset_inner = 0;
    if (right) {
        xoffset = game_state->game_width/2 - 7;
    } else {
        xoffset_inner = game_state->game_width/2 - 8 - (int) width;
    }
    DrawRectangle(7 + xoffset, 7, game_state->game_width/2-6, 6, BLACK);
    DrawRectangle(8 + xoffset + xoffset_inner, 8, (int) width, 4, color);
}

Particle_System*
init_particle_system(int max_particle_count) {
    Particle_System* ps = (Particle_System*) calloc(1, sizeof(Particle_System));
    ps->particles = (Particle*) malloc(max_particle_count * sizeof(Particle));
    ps->max_particle_count = max_particle_count;
    return ps;
}

void
update_particle_system(Particle_System* ps, bool spawn_new) {
    // Remove dead particles
    for (int i = 0; i < ps->particle_count; i++) {
        Particle* p = &ps->particles[i];
        if (p->t <= 0.0f) {
            if (ps->particle_count > 0) {
                Particle* last = &ps->particles[ps->particle_count - 1];
                *p = *last;
                i--;
            }
            ps->particle_count--;
        }
    }
    
    if (spawn_new) {
        // Spawn new particles
        for (int i = 0; i < 10; i++) {
            if (ps->particle_count < ps->max_particle_count && random_f32() < ps->spawn_rate) {
                Particle* p = &ps->particles[ps->particle_count];
                ps->particle_count++;
                p->p = ps->start_p;
                
                f32 a = ps->min_angle + random_f32() * (ps->max_angle - ps->min_angle);
                p->v.x = cosf(a)*ps->speed;
                p->v.y = sinf(a)*ps->speed;
                
                p->t = 1.0f;
            }
        }
    }
    
    // Update live particles
    for (int i = 0; i < ps->particle_count; i++) {
        Particle* p = &ps->particles[i];
        p->p += p->v;
        p->t -= ps->delta_t;
    }
}

bool
check_collisions(Game_State* state, Entity* entity, v2* step_velocity) {
    bool result = false;
    entity->collided = false;
    entity->collided_with = 0;
    
    for (int j = 0; j < state->entity_count; j++) {
        Entity* other = &state->entities[j];
        if ((other != entity && other->is_rigidbody) || 
            other->type == Box_Collider ||
            other->type == Door) {
            
            // Some collision exceptions
            if ((entity->type == Player && (other->type == Bullet || other->type == Charged_Bullet)) ||
                ((entity->type == Bullet || entity->type == Charged_Bullet) &&  other->type == Player) ||
                //(entity->type == Player && other->type == Boss_Dragon) ||
                //(entity->type == Boss_Dragon && other->type == Player) ||
                (entity->is_rigidbody && entity->health <= 0) ||
                (other->is_rigidbody && other->health <= 0)) {
                continue;
            }
            
            bool collided = box_collision(entity, other, step_velocity, !other->is_rigidbody);
            if (collided) {
                entity->collided = true;
                entity->collided_with = other;
                result = true;
            }
        }
    }
    
    return result;
}


void
shoot_bullet(Game_State* state, Entity* entity, Entity* bullet, bool upward) {
    
    PlaySound(state->sound_shoot_bullet);
    
    bullet->health = bullet->type == Charged_Bullet ? 100 : 30;
    bullet->p = entity->p + vec2((upward && entity->facing_dir == 1.0f) ? 1.3f : 0.0f, 0.75f);
    
    if (upward) {
        bullet->sprite_rot = 90.0f;//PI_F32 / 2.0f;
        bullet->facing_dir = 0.0f;
    } else {
        bullet->facing_dir = entity->facing_dir;
        bullet->sprite_rot = 0.0f;
    }
}



int
main() {
    Game_State game_state = {};
    Game_State* state = &game_state;
    
    state->game_width = 22 * TILE_SIZE;
    state->game_height = 15 * TILE_SIZE;
    state->game_scale = 4;
    state->screen_width = state->game_width * state->game_scale;
    state->screen_height = state->game_height * state->game_scale;
    
    state->meters_to_pixels = TILE_SIZE;
    state->pixels_to_meters = 1.0f/state->meters_to_pixels;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    InitWindow(state->screen_width, state->screen_height, "GMTK Game Jam 2023");
    SetTargetFPS(60);
    
    InitAudioDevice();
    
    //cstring tiles = (cstring) "tiles.png";
    state->texture_tiles = LoadTexture("assets/tiles.png");
    state->texture_background = LoadTexture("assets/background.png");
    state->texture_dragon = LoadTexture("assets/dragon.png");
    state->texture_player = LoadTexture("assets/player.png");
    state->texture_door = LoadTexture("assets/door.png");
    state->texture_bullet = LoadTexture("assets/bullet.png");
    state->texture_charged_bullet = LoadTexture("assets/charged_bullet.png");
    
    state->sound_shoot_bullet = LoadSound("assets/shoot_bullet.wav");
    state->sound_explosion = LoadSound("assets/explosion.wav");
    state->sound_hurt = LoadSound("assets/hurt.wav");
    state->sound_player_hurt = LoadSound("assets/player_hurt.wav");
    state->sound_fire_breathing = LoadSound("assets/fire_breath.wav");
    state->sound_charging = LoadSound("assets/charging.wav");
    
    RenderTexture2D render_target = LoadRenderTexture(state->game_width, state->game_height);
    
    // Fire attack
    state->ps_fire = init_particle_system(500);
    state->ps_fire->start_p = vec2(5.0f, 5.0f);
    
    state->ps_fire->min_angle = PI_F32/4.0f + 0.3f; 
    state->ps_fire->max_angle = PI_F32/4.0f - 0.3f;
    
    state->ps_fire->speed = 0.1f;
    
    state->ps_fire->spawn_rate = 0.6f;
    state->ps_fire->delta_t = 0.015f;
    
    // Charging attack
    state->ps_charging = init_particle_system(100);
    state->ps_charging->start_p = vec2(5.0f, 5.0f);
    
    state->ps_charging->min_angle = 0;
    state->ps_charging->max_angle = PI_F32*2.0f;
    
    state->ps_charging->speed = 0.05f;
    
    state->ps_charging->spawn_rate = 0.5f;
    state->ps_charging->delta_t = 0.04f;
    
    
    Memory_Arena level_arena = {};
    Entity* player = init_level(state, &level_arena);
    
    while (!WindowShouldClose())
    {
        f32 delta_time = GetFrameTime();
        
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }
        
        
        state->screen_width = GetScreenWidth();
        state->screen_height = GetScreenHeight();
        
        
#if BUILD_DEBUG
        if (IsKeyPressed(KEY_R)) {
            player = init_level(state, &level_arena);
        }
        
        if (IsKeyPressed(KEY_M)) { 
            state->mode = state->mode == Control_Boss_Enemy ?
                Control_Player : Control_Boss_Enemy;
        }
#endif
        
        // Update
        
        const f32 jump_height = 4.8f;
        const f32 time_to_jump_apex = 3.0f * 0.3f;
        const f32 initial_velocity = (-2.0f * jump_height) / time_to_jump_apex;
        const f32 jump_gravity = (2.0f * jump_height) / (time_to_jump_apex * time_to_jump_apex);
        const f32 gravity = jump_gravity*2.0f; // NOTE(Alexander): normal gravity is heavier than jumping gravity
        
        
        if (state->mode == Intro_Cutscene) {
            state->cutscene_time += delta_time;
            
            if (state->cutscene_time > 8.0f) {
                state->mode = Control_Boss_Enemy;
            }
        }
        
        Vector2 origin =  {};
        
        for (int i = 0; i < state->entity_count; i++) {
            Entity* entity = &state->entities[i];
            
            switch (entity->type) {
                case Player: {
                    // Player controller
                    entity->acceleration.y = 0.0f;
                    entity->acceleration.x = 0.0f;
                    
                    if (state->mode == Intro_Cutscene) {
                        if (cutscene_interval(0.4, 2.5f)) {
                            entity->acceleration.x = -16;
                            entity->facing_dir = -1.0f;
                        } else if (cutscene_interval(3.5f, 5.5f)) {
                            entity->facing_dir = 1.0f;
                        } else {
                            entity->facing_dir = -1.0f;
                        }
                        
                    } else if (state->mode == Control_Player) {
                        if (IsKeyDown(KEY_A)) {
                            entity->acceleration.x = -16;
                            entity->facing_dir = -1.0f;
                        }
                        
                        if (IsKeyDown(KEY_D)) {
                            entity->acceleration.x = 16;
                            entity->facing_dir = 1.0f;
                        }
                        
                        if (entity->is_jumping && (entity->velocity.y > 0.0f || !IsKeyDown(KEY_SPACE))) {
                            entity->is_jumping = false;
                        }
                        
                        if (entity->is_grounded && IsKeyPressed(KEY_SPACE)) {
                            entity->velocity.y = initial_velocity;
                            entity->is_jumping = true;
                        }
                        
                    } else if (state->mode == Control_Boss_Enemy) {
                        
                        Entity* target = state->boss_enemy;
                        
                        v2 dist = ((entity->p + entity->size/2.0f) -
                                   (target->p + (target->size/2.0f)));
                        
                        bool attack = false;
                        bool jump = false;
                        f32 move_x = 0.0f;
                        
                        f32 accuracy = 2.0f - min(fabsf(dist.x), fabsf(dist.y));
                        f32 charge_accuracy = 4.0f - min(fabsf(dist.x), fabsf(dist.y));
                        
                        // Too far to hit
                        if (max(fabsf(dist.x), fabsf(dist.y)) > 7.0f) {
                            accuracy = -1.0f;
                        }
                        
                        // no accuracy if looking the other way
                        if (fabsf(dist.y) < 2.0f) {
                            if ((int) entity->facing_dir == sign(dist.x)) {
                                accuracy = -1.0f;
                            }
                        }
                        
                        //pln("%f, %f", dist.x, dist.y);
                        
                        
                        bool go_to_attack = true;
                        if (player->invincibility_frames > 0 || state->boss_enemy->is_attacking) {
                            go_to_attack = false;
                            
                            if (state->boss_enemy->is_attacking && sign(dist.x) != (int) state->boss_enemy->facing_dir) {
                                go_to_attack = true;
                            }
                        }
                        
                        // if safe distance away attack anyways
                        if (fabsf(dist.x) > 7.0f || fabsf(dist.y) > 7.0f) {
                            go_to_attack = true;
                        }
                        
                        if (go_to_attack) {
                            entity->is_cornered = false;
                        }
                        
                        bool shoot_upwards = false;
                        if (go_to_attack) {
                            attack = accuracy > 0.0f;
                            
                            if (fabsf(dist.y) > 5.0f) {
                                shoot_upwards = true;
                                
                                if (fabsf(dist.y) > 7.0f) {
                                    jump = random_f32() <= 0.01f;
                                }
                                
                                if (fabsf(dist.x) > 0.5f) {
                                    move_x = -1.0f*sign(dist.x);
                                }
                            } else {
                                if (fabsf(dist.x) > 3.0f) {
                                    
                                    if (fabsf(dist.y) > 2.0f) {
                                        jump = true;
                                    }
                                    
                                    if (fabsf(dist.x) > 7.0f) {
                                        move_x = -1.0f*sign(dist.x);
                                    }
                                    
                                } else {
                                    move_x = 1.0f*sign(dist.x);
                                }
                            }
                        } else {
                            // Move away from danger
                            // TODO: better corner handling
                            if (entity->p.x < 3.0f || entity->p.x > 19.0f || entity->is_cornered) {
                                // Avoid getting cornered (move to center of screen 
                                entity->is_cornered = true;
                                f32 escape_x = entity->p.x - 11.0f;
                                move_x = -1.0f*sign(escape_x);
                            } else {
                                move_x = 1.0f*sign(dist.x);
                            }
                        }
                        
                        
                        entity->is_attacking = false;
                        for (int j = 0; j < array_count(entity->attack_time); j++) {
                            if (entity->attack_time[j] > 0.0f) {
                                entity->is_attacking = true;
                                entity->attack_time[j] -= delta_time;
                                
                                if (j == 1) {
                                    if (entity->invincibility_frames <= 0) {
                                        if ((int) (entity->attack_time[j] * 80.0f) % 40 == 39) {
                                            PlaySound(state->sound_charging);
                                        }
                                    } else {
                                        entity->attack_time[j] = 0.0f;
                                        continue;
                                    }
                                }
                                
                                if (j == 1 && entity->attack_time[1] <= 0.0f) {
                                    PlaySound(state->sound_explosion);
                                    shoot_bullet(state, entity, state->charged_bullet, fabsf(dist.y) > 3.0f);
                                }
                            }
                            
                            if (entity->attack_cooldown[j] > 0.0f) {
                                entity->attack_cooldown[j] -= delta_time;
                            }
                        }
                        
                        for (int bi = 0; bi < array_count(state->bullets); bi++) {
                            Entity* bullet = state->bullets[bi];
                            if (bullet->health > 0) {
                                bullet->health -= 1;
                            }
                        }
                        
                        
                        bool is_charging =  entity->attack_time[1] > 0.0f;
                        if (is_charging) {
                            state->ps_charging->start_p = entity->p +
                                vec2(entity->facing_dir > 0.0f ? 1.0f : 0.0f, 1.0f);
                        }
                        update_particle_system(state->ps_charging, is_charging);
                        
                        
                        if (!entity->is_attacking) {
                            // Initiate a new attack
                            
                            if (entity->invincibility_frames <= 0) {
                                if (attack && entity->attack_cooldown[0] <= 0.0f) {
                                    entity->attack_time[0] = 0.3f;
                                    entity->attack_cooldown[0] = 0.5f;
                                    entity->is_attacking = true;
                                    
                                    f32 far_dist = max(fabsf(dist.x), fabsf(dist.y));
                                    
                                    // Try use a charged bullet if there is a good chance
                                    if (charge_accuracy > 0.25f &&
                                        (state->boss_enemy->is_attacking || far_dist > 4.0f) &&
                                        state->charged_bullet->health <= 0 &&
                                        entity->is_grounded &&
                                        entity->attack_cooldown[1] <= 0.0f) {
                                        
                                        entity->is_attacking = true;
                                        entity->attack_time[1] = random_f32()*0.6f + 0.5f;
                                        entity->attack_cooldown[0] = 2.0f;
                                        entity->attack_cooldown[1] = 3.0f;
                                        PlaySound(state->sound_charging);
                                    } else {
                                        
                                        // Find available bullet
                                        for (int bi = 0; bi < array_count(state->bullets); bi++) {
                                            Entity* bullet = state->bullets[bi];
                                            if (bullet->health <= 0) {
                                                entity->is_attacking = true;
                                                shoot_bullet(state, entity, bullet, shoot_upwards);
                                                entity->attack_cooldown[0] = random_f32()*2.0f;
                                                break;
                                            }
                                        }
                                    }
                                    
                                } else {
                                    // more attacks
                                }
                            }
                            
                            entity->acceleration.x = move_x*16;
                            if (move_x != 0.0f) {
                                entity->facing_dir = (f32) sign(move_x);
                            } else {
                                entity->facing_dir = (f32) -sign(dist.x);
                            }
                            
                            if (entity->is_grounded && jump) {
                                entity->velocity.y = initial_velocity;
                                entity->is_jumping = true;
                            }
                        }
                    }
                    
                    entity->acceleration.y = entity->is_jumping ? jump_gravity : gravity;
                    
                    
#if 0
                    if (IsKeyPressed(KEY_E)) {
                        
                        if (player->holding) {
                            // Take out previous item (unless colliding with something)
                            player->holding.p = player->p;
                            v2 step_velocity = vec2(player->facing_dir, 0.0f);
                            if (!check_collisions(state, player->holding, &step_velocity)) {
                                player->holding.type = Box;
                                &player->holding.p += step_velocity;
                                
                                if (!IsKeyDown(KEY_S)) {
                                    player->holding.velocity = vec2(20.0f * player->facing_dir, -20.0f);
                                }
                                player->holding = 0;
                            }
                        } else {
                            f32 closest = 2.0f;
                            Entity* target = 0;
                            
                            for (int k = 0; k < state->entity_count; k++) {
                                Entity* box_entity = state->entities[k];
                                
                                if (box_entity->type == Box) {
                                    v2 p0 = box_entity->p + box_entity->size * 0.5f;
                                    v2 p1 = player->p + player->size * 0.5f;
                                    v2 diff = p0 - p1;
                                    f32 dist = sqrt(diff.x*diff.x + diff.y*diff.y);
                                    
                                    if (dist < closest) {
                                        target = box_entity;
                                        closest = dist;
                                    }
                                }
                            }
                            
                            if (target) {
                                target.type = None;
                                player->holding = target;
                            }
                        }
                    }
#endif
                } break;
                
                case Door: {
                    if (state->mode == Intro_Cutscene) {
                        if (entity == state->right_door) {
                            if (cutscene_interval(3.0f, 3.5f)) {
                                entity->size.y += delta_time*8.0f;
                            } else if (cutscene_interval(3.5f, 10.0f)) {
                                entity->size.y = 4.0f;
                                
                                if (entity->health == 1) {
                                    PlaySound(state->sound_explosion);
                                    entity->health = 2;
                                }
                            }
                        } else if (entity == state->left_door) {
                            if (cutscene_interval(6.0f, 6.5f)) {
                                entity->size.y += delta_time*8.0f;
                            } else if (cutscene_interval(6.5f, 10.0f)) {
                                entity->size.y = 4.0f;
                                
                                if (entity->health == 1) {
                                    PlaySound(state->sound_explosion);
                                    entity->health = 2;
                                }
                            }
                        }
                    } else {
                        entity->size.y = 4.0f;
                    }
                } break;
                
                case Bullet:
                case Charged_Bullet: {
                    entity->velocity.x = 0.0f;
                    entity->velocity.y = 0.0f;
                    
                    f32 bullet_speed = entity->type == Charged_Bullet ? 20.0f : 12.0f;
                    if (entity->facing_dir == 0.0f) {
                        entity->velocity.y = -bullet_speed;
                    } else {
                        entity->velocity.x = bullet_speed*entity->facing_dir;
                    }
                    if (entity->collided) {
                        entity->health = 0;
                        
                        if (entity->collided_with == state->boss_enemy) {
                            if (state->boss_enemy->invincibility_frames <= 0) {
                                state->boss_enemy->health -= entity->type == Charged_Bullet ? 100 : 10;
                                state->boss_enemy->invincibility_frames = 40;
                                
                                PlaySound(state->sound_hurt);
                                if (entity->facing_dir == 0.0f) {
                                    state->boss_enemy->velocity = vec2(0.0f, -3.0f);
                                } else {
                                    state->boss_enemy->velocity = vec2(entity->facing_dir*3.0f, 0.0f);
                                }
                            }
                        }
                    }
                } break;
                
                
                case Boss_Dragon: {
                    f32 fly_upward_gravity = 4.25f;
                    f32 fly_gravity = fly_upward_gravity*0.5f;
                    entity->acceleration.x = 0.0f;
                    entity->acceleration.y = (entity->is_jumping ? fly_upward_gravity : fly_gravity);
                    
                    //if (entity->is_grounded && abs(entity->acceleration.x) < epsilon32) {
                    //entity->velocity.x *= 0.8f;
                    //}
                    
                    if (entity->is_jumping && (entity->velocity.y > 0.0f || !IsKeyDown(KEY_SPACE))) {
                        entity->is_jumping = false;
                    }
                    
                    if (entity->facing_dir > 0.0f) {
                        state->ps_fire->start_p = entity->p + vec2(entity->size.width - 0.8f, 0.8f);
                        state->ps_fire->min_angle = PI_F32/4.0f + 0.3f; 
                        state->ps_fire->max_angle = PI_F32/4.0f - 0.3f;
                    } else {
                        state->ps_fire->start_p = entity->p + vec2(0.8f, 0.8f);
                        state->ps_fire->min_angle = -PI_F32/4.0f + PI_F32 + 0.3f; 
                        state->ps_fire->max_angle = -PI_F32/4.0f + PI_F32 - 0.3f;
                        
                    }
                    
                    
                    if (state->mode == Intro_Cutscene) {
                        if (cutscene_interval(3.5f, 6.5f)) {
                            entity->acceleration.x = 16;
                            entity->facing_dir = 1.0f;
                        }
                        
                    } else if (state->mode == Control_Boss_Enemy) {
                        
                        
                        if (entity->collided) {
                            if (entity->collided_with == state->player) {
                                if (player->invincibility_frames <= 0) {
                                    player->invincibility_frames = 30;
                                    player->health -= 10;
                                    player->velocity.x = -entity->facing_dir*2.0f;
                                    PlaySound(state->sound_player_hurt);
                                }
                            }
                        }
                        
                        entity->is_attacking = false;
                        for (int j = 0; j < array_count(entity->attack_time); j++) {
                            if (entity->attack_time[j] > 0.0f) {
                                entity->is_attacking = true;
                                entity->attack_time[j] -= delta_time;
                            }
                            
                            if (entity->attack_cooldown[j] > 0.0f) {
                                entity->attack_cooldown[j] -= delta_time;
                            }
                        }
                        
                        if (entity->is_attacking) {
                            entity->velocity = vec2_zero;
                            entity->acceleration = vec2_zero;
                        } else {
                            
                            if (entity->invincibility_frames <= 0) {
                                // Initiate a new attack
                                if (IsKeyPressed(KEY_F) && entity->attack_cooldown[0] <= 0.0f) {
                                    entity->attack_time[0] = 2.5f;
                                    entity->attack_cooldown[0] = 5.0f;
                                    entity->is_attacking = true;
                                    PlaySound(state->sound_fire_breathing);
                                    
                                } else if (IsKeyPressed(KEY_E) && entity->attack_cooldown[1] <= 0.0f) {
                                    //entity->attack_time[1] = 2.0f;
                                    //entity->attack_cooldown[1] = 10.0f;
                                    //entity->is_attacking = true;
                                    
                                }
                            }
                            
                            if (IsKeyPressed(KEY_S)) { 
                                entity->velocity.y = 1.5f;
                            }
                            if (IsKeyDown(KEY_S)) {
                                entity->acceleration.y *= 2.0f;
                            }
                            
                            if (IsKeyDown(KEY_A)) {
                                entity->acceleration.x = -16;
                                entity->facing_dir = -1.0f;
                            }
                            
                            if (IsKeyPressed(KEY_SPACE)) {
                                entity->velocity.y = -3.0f;
                                entity->is_jumping = true;
                            }
                            
                            
                            if (IsKeyDown(KEY_D)) {
                                entity->acceleration.x = 16;
                                entity->facing_dir = 1.0f;
                            }
                        }
                        
                    } else {
                        // Control the dragon using AI!
                        
                    }
                    
                    // Fire breathing attack
                    //assert(entity->attack_time[0] == 0.0f);
                    bool fire_breathing = entity->attack_time[0] > 0.0f;
                    update_particle_system(state->ps_fire, entity->attack_time[0] > 0.5f);
                    
                    if (fire_breathing) {
                        
                        BoundingBox player_box;
                        player_box.min = { player->p.x, player->p.y, 0.0f };
                        player_box.max = { player->p.x + player->size.width, player->p.y +player->size.height, 0.0f };
                        
                        v2 rpos = state->ps_fire->start_p;
                        
                        Ray ray;
                        ray.position = { rpos.x, rpos.y, 0.0f };
                        
                        v2 dir = { entity->facing_dir, 1.0f };
                        dir = normalize(dir);
                        ray.direction = { dir.x, dir.y, 0.0f };
                        
                        f32 t = (2.0f - entity->attack_time[0]) * 2.0f;
                        if (t >= 1.0f) t = 1.0f;
                        f32 min_d = 0.0f;
                        f32 max_d = t*5.0f;
                        pln("%f", max_d);
                        
                        bool collision = ray_box_collision(ray, min_d, max_d, player_box);
                        
                        dir = { entity->facing_dir, 1.6f };
                        dir = normalize(dir);
                        ray.direction = { dir.x, dir.y, 0.0f };
                        
                        collision = collision || ray_box_collision(ray, min_d, max_d, player_box);
                        
                        dir = { entity->facing_dir, 0.6f };
                        dir = normalize(dir);
                        ray.direction = { dir.x, dir.y, 0.0f };
                        collision = collision || ray_box_collision(ray, min_d, max_d, player_box);
                        
                        if (collision) {
                            if (player->invincibility_frames <= 0) {
                                player->health -= 40;
                                player->invincibility_frames = 40;
                                SetSoundPitch(state->sound_player_hurt, random_f32()*0.3f + 1.0f);
                                PlaySound(state->sound_player_hurt);
                            }
                        }
                    }
                    
                    
                    bool is_charging = entity->attack_time[1] > 0.0f;
                    if (is_charging) {
                        player->acceleration.x = player->facing_dir*30.0f;
                    } else {
                        
                    }
                } break;
            }
            
            if (entity->invincibility_frames > 0) {
                entity->invincibility_frames--;
            }
            
            if (entity->is_rigidbody) {
                // Rigidbody physics
                v2 step_velocity = entity->velocity * delta_time + entity->acceleration * delta_time * delta_time * 0.5f;
                entity->is_grounded = false;
                check_collisions(state, entity, &step_velocity);
                
                entity->p += step_velocity;
                entity->velocity += entity->acceleration * delta_time;
                
                if (entity->num_frames > 0) {
                    entity->frame_advance += step_velocity.x * entity->frame_advance_rate;
                    if (fabsf(step_velocity.x) <= 0.01f) {
                        entity->frame_advance = 0.0f;
                    }
                    
                    if (entity->frame_advance > entity->num_frames) {
                        entity->frame_advance -= entity->num_frames;
                    }
                    
                    if (entity->frame_advance < 0.0f) {
                        entity->frame_advance += entity->num_frames;
                    }
                }
                
                
                if (fabsf(entity->velocity.x) > entity->max_speed.x) {
                    entity->velocity.x = sign(entity->velocity.x) * entity->max_speed.x;
                }
                
                if (fabsf(entity->acceleration.x) > epsilon32 && fabsf(entity->velocity.x) > epsilon32 &&
                    sign(entity->acceleration.x) != sign(entity->velocity.x)) {
                    entity->acceleration.x *= 2.0f;
                }
                
                if (fabsf(entity->acceleration.x) < epsilon32) {
                    entity->velocity.x *= 0.8f;
                }
                
            }
        }
        
        // Draw to render texture
        BeginTextureMode(render_target);
        ClearBackground(BACKGROUND_COLOR);
        
        
#if 0
        state->camera_p.x = player->p.x * state->meters_to_pixels - state->game_width/2.0f;
        state->camera_p.x = round(state->camera_p.x) * state->pixels_to_meters;
        state->camera_p.y = player->p.y * state->meters_to_pixels - state->game_height/2.0f;
        state->camera_p.y = round(state->camera_p.y) * state->pixels_to_meters;
#endif
        
        
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 3; x++) {
                int px = (int) round(x*state->texture_background.width-state->camera_p.x * state->meters_to_pixels);
                int py = (int) round(y*state->texture_background.height-state->camera_p.y * state->meters_to_pixels);
                DrawTexture(state->texture_background, px, py, WHITE);
            }
        }
        
        int tile_xcount = (int) (state->texture_tiles.width/state->meters_to_pixels);
        for (int y = 0; y < state->tile_map_height; y++) {
            for (int x = 0; x < state->tile_map_width; x++) {
                u8 tile = state->tile_map[y*state->tile_map_width + x];
                if (tile == 0) continue;
                tile--;
                
                Rectangle src = { 0, 0, state->meters_to_pixels, state->meters_to_pixels };
                src.x = (tile % tile_xcount) * state->meters_to_pixels;
                src.y = (tile / tile_xcount) * state->meters_to_pixels;
                
                Rectangle dest = { 0, 0, state->meters_to_pixels, state->meters_to_pixels };
                dest.x = (x - state->camera_p.x) * state->meters_to_pixels;
                dest.y = (y - state->camera_p.y) * state->meters_to_pixels;
                DrawTexturePro(state->texture_tiles, src, dest, origin, 0.0f, WHITE);
            }
        }
        
        for (int i = 0; i < state->entity_count; i++) {
            Entity* entity = &state->entities[i];
            if (!entity->type) continue;
            if (entity->health <= 0) continue;
            
            if (entity->texture) {
                v2 p = to_pixel(state, entity->p);
                v2 size = entity->size * state->meters_to_pixels;
                
                bool facing_right = entity->facing_dir > 0.0f;
                if (entity->flip_texture) {
                    facing_right = !facing_right;
                }
                
                
                Rectangle src = { 0.0f, 0.0f, size.width, size.height };
                if (entity->num_frames > 0 && entity->is_grounded) {
                    int frame_index = (int) entity->frame_advance;
                    src.x += size.width*frame_index;
                }
                
                Rectangle dest = { p.x, p.y, size.width, size.height };
                if (facing_right) {
                    src.width = -src.width;
                }
                
                if (entity->invincibility_frames % 2 == 0) {
                    DrawTexturePro(*entity->texture, src, dest, origin, entity->sprite_rot, WHITE);
                }
                
            } else {
                v2 p = to_pixel(state, entity->p);
                v2 size = entity->size * state->meters_to_pixels;
                DrawRectangle((int) p.x, (int) p.y,
                              (int) size.x, (int) size.y, entity->color);
            }
            
#if 0
            {
                v2 p = to_pixel(state, entity->p);
                v2 size = entity->size * state->meters_to_pixels;
                Rectangle r = { p.x, p.y, size.x, size.y };
                DrawRectanglePro(r, origin, 0.0f, RED);
            }
#endif
            
            switch (entity->type) {
                case Boss_Dragon: {
                    Particle_System* ps = state->ps_fire;
                    BeginBlendMode(BLEND_MULTIPLIED);
                    for (int j = 0; j < ps->particle_count; j++) {
                        Particle* particle = &ps->particles[j];
                        if (particle->t > 0.0f) {
                            v2 p = to_pixel(state, particle->p);
                            Color color = ORANGE;
                            color.r = (u8) (color.r*particle->t);
                            color.g = (u8) (color.g*particle->t);
                            color.b = (u8) (50*particle->t);
                            color.a = (u8) (particle->t*particle->t*particle->t*255.0f);
                            DrawCircle((int) p.x, (int) p.y, (1.0f - particle->t*particle->t)*10.0f, color);
                        }
                    }
                    BeginBlendMode(BLEND_ALPHA);
                    
#if BUILD_DEBUG && 0
                    Ray ray;
                    v2 rpos = to_pixel(state, state->ps_fire->start_p);
                    ray.position = { rpos.x, rpos.y, 0.0f };
                    
                    v2 dir = { entity->facing_dir, 1.0f };
                    dir = normalize(dir);
                    ray.direction = { dir.x, dir.y, 0.0f };
                    DrawRay(ray, PURPLE);
                    
                    dir = { entity->facing_dir, 1.6f };
                    dir = normalize(dir);
                    ray.direction = { dir.x, dir.y, 0.0f };
                    DrawRay(ray, PURPLE);
                    
                    dir = { entity->facing_dir, 0.6f };
                    dir = normalize(dir);
                    ray.direction = { dir.x, dir.y, 0.0f };
                    DrawRay(ray, PURPLE);
#endif
                    
                } break;
                
                
                case Player: {
                    Particle_System* ps = state->ps_charging;
                    BeginBlendMode(BLEND_ADDITIVE);
                    for (int j = 0; j < ps->particle_count; j++) {
                        Particle* particle = &ps->particles[j];
                        if (particle->t > 0.0f) {
                            v2 cp = to_pixel(state, ps->start_p);
                            v2 p1 = to_pixel(state, particle->p);
                            v2 p0 = cp + (p1 - cp)*(1.0f - particle->t);
                            Color color = YELLOW;
                            //color.r = (u8) (color.r*particle->t);
                            //color.g = (u8) (color.g*particle->t);
                            //color.b = (u8) (50*particle->t);
                            color.a = 25;//(u8) (particle->t*particle->t*particle->t*255.0f);
                            DrawLine((int) p0.x, (int) p0.y, (int) p1.x, (int) p1.y, color);
                        }
                    }
                    BeginBlendMode(BLEND_ALPHA);
                    
                } break;
            }
            
            
#if 0
            if (entity->holding) {
                v2 size = entity->size * state->meters_to_pixels * 0.3f;
                v2 p = entity->p + vec2(entity->size.x/2.0f + 0.4f * entity->facing_dir, 0.5f);
                
                if (IsKeyDown(KEY_S)) {
                    p.y += entity->size.y - 0.5f ;
                }
                
                p = to_pixel(state, p) - size.x/2.0f;
                
                Rectangle r = { p.x, p.y, size.x, size.y };
                DrawRectanglePro(r, origin, 0.0f, entity->holding.color);
            }
#endif
        }
        
        draw_level_bounds(state);
        
        
        if (state->mode == Control_Boss_Enemy) {
            draw_health_bar(state, state->boss_enemy, RED, false);
            draw_health_bar(state, state->player, SKYBLUE, true);
        }
        
        //DrawRectangle(5, 5, state->game_width/2-10, 8, BLACK);
        //DrawRectangle(6, 6, state->game_width/2-12, 6, RED);
        
        
        //cstring text = "Congrats! You created your first window!";
        //DrawText((s8*) text, 190, 200, 20, LIGHTGRAY);
        EndTextureMode();
        
        {
            // Render to screen
            BeginDrawing();
            
            ClearBackground(BACKGROUND_COLOR);
            Texture render_texture = render_target.texture;
            Rectangle src = { 0, 0, (f32) render_texture.width, (f32) (-render_texture.height) };
            f32 aspect_ratio = (f32) render_texture.width / render_texture.height;
            Rectangle dest = { 0, 0, 0, (f32) state->screen_height };
            dest.width = aspect_ratio*state->screen_height;
            dest.x = (state->screen_width - dest.width)/2.0f;
            
            //dest.x += dest.width;
            if (state->mode == Intro_Cutscene) {
                if (cutscene_interval(7.0f, 7.5f)) {
                    f32 t = (state->cutscene_time - 7.0f)*4.0f;
                    
                    f32 swap = fabsf(1.0f - t);
                    dest.x = dest.x + (dest.width / 2.0f) * (1.0f - swap);
                    dest.width = dest.width * swap;
                    
                }
                
                
                if (cutscene_interval(0.0f, 7.25f)) {
                    src.width = -src.width;
                }
            }
            DrawTexturePro(render_texture, src, dest, origin, 0, WHITE);
            
#if BUILD_DEBUG
            
            DrawFPS(8, state->screen_height - 24);
#endif
            
            EndDrawing();
        }
    }
    
    CloseWindow();        // Close window and OpenGL context
    
    return 0;
}