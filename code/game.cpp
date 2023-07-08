#define TILE_SIZE 16
#define BUILD_DEBUG 0

#include <basic.sq>

#include "game.h"
#include "format_tmx.cpp"

#define BACKGROUND_COLOR rgb(52, 28, 39)


bool
box_collision(Entity collider, Entity* rigidbody, v2* step_velocity) {
    bool found = false;
    
    v2 step_position = rigidbody.p + *step_velocity;
    if (step_position.y + rigidbody.size.y > collider.p.y && 
        step_position.y < collider.p.y + collider.size.y) {
        
        if (step_velocity.x < 0.0f && rigidbody.p.x >= collider.p.x + collider.size.x) {
            f32 x_overlap = collider.p.x + collider.size.x - step_position.x;
            if (x_overlap > 0.0f) {
                step_velocity.x = collider.p.x + collider.size.x - rigidbody.p.x;
                rigidbody.velocity.x = 0.0f;
                found = true;
            }
        } else if (step_velocity.x > 0.0f && rigidbody.p.x + rigidbody.size.x <= collider.p.x) {
            f32 x_overlap = step_position.x - collider.p.x + rigidbody.size.x;
            if (x_overlap > 0.0f) {
                step_velocity.x = collider.p.x - rigidbody.size.x - rigidbody.p.x;
                rigidbody.velocity.x = 0.0f;
                found = true;
            }
        }
    }
    
    if (rigidbody.p.x + rigidbody.size.x > collider.p.x && 
        rigidbody.p.x < collider.p.x + collider.size.x) {
        f32 tmp = collider.p.x + collider.size.x;
        if (step_velocity.y < 0.0f && rigidbody.p.y < collider.p.y + collider.size.y &&
            rigidbody.p.y + rigidbody.size.y > collider.p.y) {
            f32 y_overlap = step_position.y - collider.p.y + collider.size.y;
            if (y_overlap > 0.0f) {
                step_velocity.y = collider.p.y + collider.size.y - rigidbody.p.y;
                rigidbody.velocity.y = 0.0f;
                found = true;
            }
        } else if (step_velocity.y > 0.0f && rigidbody.p.y + rigidbody.size.y <= collider.p.y) {
            f32 y_overlap = step_position.y - collider.p.y + rigidbody.size.y;
            if (y_overlap > 0.0f) {
                step_velocity.y = collider.p.y - rigidbody.size.y - rigidbody.p.y;
                rigidbody.velocity.y = 0.0f;
                rigidbody.is_grounded = true;
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
        pln("dist = %", result.distance);
        return result.distance >= min_dist && result.distance <= max_dist;
    }
    
    return false
}


Entity*
spawn_entity(Game_State* state, Memory_Arena* arena, Entity_Type type) {
    Entity* entity = push_struct(arena, Entity);
    state->entity_count++;
    *entity = {};
    entity.type = type;
    
    if (!state.entities) {
        state.entities =  entity;
    }
    return entity;
}


#define TILE_SIZE 16


Entity*
init_level(Game_State* state, Memory_Arena* arena) {
    clear(arena);
    
    state->entity_count = 0;
    
    Loaded_Tmx tmx = read_tmx_map_data("interior.tmx", arena);
    pln("%", tmx);
    state->entities = tmx.entities;
    state->entity_count = tmx.entity_count;
    state->tile_map = tmx.tile_map;
    state->tile_map_width = tmx.tile_map_width;
    state->tile_map_height = tmx.tile_map_height;
    
    pln("ent = %", state->entity_count);
    Entity* player = spawn_entity(state, arena, Entity_Type.Player);
    state.player = player;
    player.texture = &state.texture_player;
    player.p = {10.0f, 6.0f};
    player.size = {1.0f, 2.0f};
    player.max_speed.x = 6;
    player.is_rigidbody = true;
    player.facing_dir = 1.0f;
    player.color = SKYBLUE;
    player.max_health = 1000;
    player.health = player.max_health;
    
    Entity* boss_enemy = spawn_entity(state, arena, Entity_Type.Boss_Dragon);
    state.boss_enemy = boss_enemy;
    boss_enemy.flip_texture = true;
    boss_enemy.texture = &state.texture_dragon;
    boss_enemy.num_frames = 8;
    boss_enemy.frame_advance_rate = 2.5f;
    boss_enemy.p = {3.0f, 3.0f};
    boss_enemy.size = {4.0f, 4.0f};
    boss_enemy.max_speed.x = 2.2f;
    boss_enemy.is_rigidbody = true;
    boss_enemy.color = RED;
    boss_enemy.max_health = 1000;
    boss_enemy.health = boss_enemy.max_health;
    return player;
}

inline s32
round_f32_to_s32(f32 value) {
    return (s32) round(value);
}

v2
to_pixel(Game_State* state, v2 world_p) {
    v2 result;
    result.x = round((world_p.x - state.camera_p.x) * state.meters_to_pixels);
    result.y = round((world_p.y - state.camera_p.y) * state.meters_to_pixels);
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
    v2 size = vec2((f32) state.tile_map_width*TILE_SIZE + 1.0f, (f32) state.tile_map_width*TILE_SIZE);
    
    v2 cp;
    cp.x = -state.camera_p.x * state.meters_to_pixels;
    cp.y = -state.camera_p.y * state.meters_to_pixels;
    v2 min_cursor_p = cp;//world_to_screen_space(vec2(0.0f, 0.0f), cp, state.meters_to_pixels);
    v2 max_cursor_p = min_cursor_p + size + vec2(0.0f, 1.0f);
    
    Color background = BACKGROUND_COLOR;
    
    if (min_cursor_p.x > 0.0f) {
        int w = (int) min_cursor_p.x;
        DrawRectangle(0, 0, w, state.game_height, background);
    }
    
    if (max_cursor_p.x < size.y) {
        DrawRectangle((int) max_cursor_p.x, 0, state.game_width - (int) max_cursor_p.x, state.game_height, background);
    }
    
    if (min_cursor_p.y > 0.0f) {
        DrawRectangle(0, 0, state.game_width, (int) min_cursor_p.y, background);
    }
    
    if (max_cursor_p.y < size.y) {
        DrawRectangle(0, (int) max_cursor_p.y, 
                      state.game_width, state.game_height - (int) max_cursor_p.y, background);
        
    }
}

void
draw_health_bar(Game_State* game_state, Entity* entity, Color color, bool right=true) {
    f32 width = (f32) (game_state.game_width/2-8);
    width *= (f32) entity.health / entity.max_health;
    s32 xoffset = 0;
    s32 xoffset_inner = 0;
    if (right) {
        xoffset = game_state.game_width/2 - 7;
    } else {
        xoffset_inner = game_state.game_width/2 - 8 - (int) width;
    }
    DrawRectangle(7 + xoffset, 7, game_state.game_width/2-6, 6, BLACK);
    DrawRectangle(8 + xoffset + xoffset_inner, 8, (int) width, 4, color);
}

struct Particle {
    v2 p;
    v2 v;
    f32 t;
}

struct Particle_System {
    [500]Particle particles;
    int particle_count;
    
    v2 start_p;
    
    f32 speed;
    
    f32 min_angle;
    f32 max_angle;
    
    f32 min_scale;
    f32 max_scale;
    
    f32 spawn_rate;
    f32 delta_t;
}

Particle_System*
init_particle_system() {
    Particle_System* ps = (Particle_System*) calloc(1, sizeof(Particle_System));
    ps.start_p = vec2(5.0f, 5.0f);
    ps.min_angle = PI_F32/4.0f + 0.3f; ps.max_angle = PI_F32/4.0f - 0.3f;
    ps.speed = 0.1f;
    ps.min_scale = 0.1f; ps.max_scale = 0.1f;
    
    ps.spawn_rate = 0.6f;
    ps.delta_t = 0.015f;
    
    return ps;
}

void
update_particle_system(Particle_System* ps, bool spawn_new) {
    // Remove dead particles
    for (int i = 0; i < ps.particle_count; i++) {
        Particle* p = &ps.particles[i];
        if (p.t <= 0.0f) {
            if (ps.particle_count > 0) {
                Particle* last = &ps.particles[ps.particle_count - 1];
                *p = *last;
                i--;
            }
            ps.particle_count--;
        }
    }
    
    if (spawn_new) {
        // Spawn new particles
        for (int i = 0; i < 10; i++) {
            if (ps.particle_count < ps.particles.count && random_f32() < ps.spawn_rate) {
                Particle* p = &ps.particles[ps.particle_count];
                ps.particle_count++;
                p.p = ps.start_p;
                
                f32 a = ps.min_angle + random_f32() * (ps.max_angle - ps.min_angle);
                p.v.x = cos(a)*ps.speed;
                p.v.y = sin(a)*ps.speed;
                
                p.t = 1.0f;
            }
        }
    }
    
    // Update live particles
    for (int i = 0; i < ps.particle_count; i++) {
        Particle* p = &ps.particles[i];
        &p.p += p.v;
        p.t -= ps.delta_t;
    }
}

bool
check_collisions(Game_State* state, Entity* entity, v2* step_velocity) {
    bool result = false;
    
    for (int j = 0; j < state.entity_count; j++) {
        Entity collider = state.entities[j];
        if (&collider != entity && collider.type == Entity_Type.Box_Collider) {
            bool collided = box_collision(collider, entity, step_velocity);
            if (collided) {
                result = true;
            }
        }
    }
    
    return result;
}




int
main() {
    Game_State game_state = {};
    game_state.game_width = 22 * TILE_SIZE;
    game_state.game_height = 15 * TILE_SIZE;
    game_state.game_scale = 4;
    game_state.screen_width = game_state.game_width * game_state.game_scale;
    game_state.screen_height = game_state.game_height * game_state.game_scale;
    
    game_state.meters_to_pixels = TILE_SIZE;
    game_state.pixels_to_meters = 1.0f/game_state.meters_to_pixels;
    
    SetConfigFlags(ConfigFlags.FLAG_WINDOW_RESIZABLE);
    
    InitWindow(game_state.screen_width, game_state.screen_height, "GMTK Game Jam 2023");
    SetTargetFPS(60);
    
    //cstring tiles = (cstring) "tiles.png";
    game_state.texture_tiles = LoadTexture("tiles.png");
    game_state.texture_background = LoadTexture("background.png");
    game_state.texture_dragon = LoadTexture("dragon.png");
    game_state.texture_player = LoadTexture("player.png");
    
    RenderTexture2D render_target = LoadRenderTexture(game_state.game_width, game_state.game_height);
    
    game_state.ps_fire = init_particle_system();
    
    
    Memory_Arena level_arena = {};
    Entity* player = init_level(&game_state, &level_arena);
    
    while (!WindowShouldClose())
    {
        f32 delta_time = GetFrameTime();
        
        if (IsKeyPressed(KeyboardKey.KEY_F11)) {
            ToggleFullscreen();
        }
        
        
        game_state.screen_width = GetScreenWidth();
        game_state.screen_height = GetScreenHeight();
        
        
#if BUILD_DEBUG
        if (IsKeyPressed(KeyboardKey.KEY_R)) {
            player = init_level(&game_state, &level_arena);
        }
        
        if (IsKeyPressed(KeyboardKey.KEY_M)) { 
            game_state.mode = !game_state.mode;
        }
#endif
        
        // Update
        
        const f32 jump_height = 4.8f;
        const f32 time_to_jump_apex = 3.0f * 0.1f;
        const f32 initial_velocity = (-2.0f * jump_height) / time_to_jump_apex;
        const f32 jump_gravity = (2.0f * jump_height) / (time_to_jump_apex * time_to_jump_apex);
        const f32 gravity = jump_gravity*2.0f; // NOTE(Alexander): normal gravity is heavier than jumping gravity
        
        Vector2 origin =  {};
        
        for (int i = 0; i < game_state.entity_count; i++) {
            Entity* entity = &game_state.entities[i];
            
            switch (entity.type) {
                case Entity_Type.Player: {
                    // Player controller
                    entity.acceleration.y = 0.0f;
                    entity.acceleration.x = 0.0f;
                    
                    if (game_state.mode == Game_Mode.Control_Player) {
                        if (IsKeyDown(KeyboardKey.KEY_A)) {
                            entity.acceleration.x = -16;
                            entity.facing_dir = -1.0f;
                        }
                        
                        if (IsKeyDown(KeyboardKey.KEY_D)) {
                            entity.acceleration.x = 16;
                            entity.facing_dir = 1.0f;
                        }
                        
                        if (entity.is_jumping && (entity.velocity.y > 0.0f || !IsKeyDown(KeyboardKey.KEY_SPACE))) {
                            entity.is_jumping = false;
                        }
                        
                        if (entity.is_grounded && IsKeyPressed(KeyboardKey.KEY_SPACE)) {
                            entity.velocity.y = initial_velocity;
                            entity.is_jumping = true;
                        }
                        
                    } else if (game_state.mode == Game_Mode.Control_Boss_Enemy) {
                        // Program an AI
                    }
                    
                    entity.acceleration.y = entity.is_jumping ? jump_gravity : gravity;
                    
                    
#if 0
                    if (IsKeyPressed(KeyboardKey.KEY_E)) {
                        
                        if (player.holding) {
                            // Take out previous item (unless colliding with something)
                            player.holding.p = player.p;
                            v2 step_velocity = vec2(player.facing_dir, 0.0f);
                            if (!check_collisions(&game_state, player.holding, &step_velocity)) {
                                player.holding.type = Entity_Type.Box;
                                &player.holding.p += step_velocity;
                                
                                if (!IsKeyDown(KeyboardKey.KEY_S)) {
                                    player.holding.velocity = vec2(20.0f * player.facing_dir, -20.0f);
                                }
                                player.holding = 0;
                            }
                        } else {
                            f32 closest = 2.0f;
                            Entity* target = 0;
                            
                            for (int k = 0; k < game_state.entity_count; k++) {
                                Entity* box_entity = &game_state.entities[k];
                                
                                if (box_entity.type == Entity_Type.Box) {
                                    v2 p0 = box_entity.p + box_entity.size * 0.5f;
                                    v2 p1 = player.p + player.size * 0.5f;
                                    v2 diff = p0 - p1;
                                    f32 dist = sqrt(diff.x*diff.x + diff.y*diff.y);
                                    
                                    if (dist < closest) {
                                        target = box_entity;
                                        closest = dist;
                                    }
                                }
                            }
                            
                            if (target) {
                                target.type = Entity_Type.None;
                                player.holding = target;
                            }
                        }
                    }
#endif
                }
                
                case Entity_Type.Boss_Dragon: {
                    entity.acceleration.x = 0.0f;
                    entity.acceleration.y = (entity.is_jumping ? jump_gravity : gravity) * 0.01f;
                    
                    
                    //if (entity.is_grounded && abs(entity.acceleration.x) < epsilon32) {
                    //entity.velocity.x *= 0.8f;
                    //}
                    
                    if (entity.is_jumping && (entity.velocity.y > 0.0f || !IsKeyDown(KeyboardKey.KEY_SPACE))) {
                        entity.is_jumping = false;
                    }
                    
                    if (entity.facing_dir > 0.0f) {
                        game_state.ps_fire.start_p = entity.p + vec2(entity.size.width - 0.8f, 0.8f);
                        game_state.ps_fire.min_angle = PI_F32/4.0f + 0.3f; 
                        game_state.ps_fire.max_angle = PI_F32/4.0f - 0.3f;
                    } else {
                        game_state.ps_fire.start_p = entity.p + vec2(0.8f, 0.8f);
                        game_state.ps_fire.min_angle = -PI_F32/4.0f + PI_F32 + 0.3f; 
                        game_state.ps_fire.max_angle = -PI_F32/4.0f + PI_F32 - 0.3f;
                        
                    }
                    
                    if (game_state.mode == Game_Mode.Control_Boss_Enemy) {
                        
                        entity.is_attacking = false;
                        for (int j = 0; j < entity.attack_time.count; j++) {
                            if (entity.attack_time[j] > 0.0f) {
                                entity.is_attacking = true;
                                entity.attack_time[j] -= delta_time;
                            }
                            
                            if (entity.attack_cooldown[j] > 0.0f) {
                                entity.attack_cooldown[j] -= delta_time;
                            }
                        }
                        
                        if (entity.is_attacking) {
                            entity.velocity = vec2_zero;
                            entity.acceleration = vec2_zero;
                        } else {
                            // Initiate a new attack
                            if (IsKeyPressed(KeyboardKey.KEY_F) && entity.attack_cooldown[0] <= 0.0f) {
                                entity.attack_time[0] = 2.0f;
                                entity.attack_cooldown[0] = 0.0f;
                                entity.is_attacking = true;
                            } else {
                                // more attacks
                            }
                            
                            if (IsKeyDown(KeyboardKey.KEY_A)) {
                                entity.acceleration.x = -16;
                                entity.facing_dir = -1.0f;
                            }
                            
                            if (IsKeyPressed(KeyboardKey.KEY_SPACE)) {
                                entity.velocity.y = -2.0f;
                                entity.is_jumping = true;
                            }
                            
                            
                            if (IsKeyDown(KeyboardKey.KEY_D)) {
                                entity.acceleration.x = 16;
                                entity.facing_dir = 1.0f;
                            }
                        }
                        
                    } else {
                        // Control the dragon using AI!
                        
                    }
                    
                    // Fire breathing attack
                    //assert(entity.attack_time[0] == 0.0f);
                    bool fire_breathing =  entity.attack_time[0] > 0.0f;
                    update_particle_system(game_state.ps_fire, fire_breathing);
                    
                    
                    if (fire_breathing) {
                        
                        BoundingBox player_box;
                        player_box.min = { player.p.x, player.p.y, 0.0f };
                        player_box.max = { player.p.x + player.size.width, player.p.y +player.size.height, 0.0f };
                        
                        v2 rpos = game_state.ps_fire.start_p;
                        
                        Ray ray;
                        ray.position = { rpos.x, rpos.y, 0.0f };
                        ray.direction = { 1.0f, 1.0f, 0.0f };
                        
                        f32 min_d = 0.1f;
                        f32 max_d = 4.0f;
                        
                        bool collision = ray_box_collision(ray, min_d, max_d, player_box);
                        
                        ray.direction = { 1.0f, 1.6f, 0.0f };
                        collision = collision || ray_box_collision(ray, min_d, max_d, player_box);
                        
                        ray.direction = { 1.0f, 0.6f, 0.0f };
                        collision = collision || ray_box_collision(ray, min_d, max_d, player_box);
                        
                        if (collision) {
                            player.health -= 1;
                        }
                    }
                }
            }
            
            if (entity.is_rigidbody) {
                // Rigidbody physics
                v2 step_velocity = entity.velocity * delta_time + entity.acceleration * delta_time * delta_time * 0.5f;
                entity.is_grounded = false;
                check_collisions(&game_state, entity, &step_velocity);
                
                &entity.p += step_velocity;
                &entity.velocity += entity.acceleration * delta_time;
                
                if (entity.num_frames > 0) {
                    entity.frame_advance += step_velocity.x * entity.frame_advance_rate;
                    if (entity.frame_advance > entity.num_frames) {
                        entity.frame_advance -= entity.num_frames;
                    }
                    
                    if (entity.frame_advance < 0.0f) {
                        entity.frame_advance += entity.num_frames;
                    }
                }
                
                
                if (abs(entity.velocity.x) > entity.max_speed.x) {
                    entity.velocity.x = sign(entity.velocity.x) * entity.max_speed.x;
                }
                
                if (abs(entity.acceleration.x) > epsilon32 && abs(entity.velocity.x) > epsilon32 &&
                    sign(entity.acceleration.x) != sign(entity.velocity.x)) {
                    entity.acceleration.x *= 2.0f;
                }
                
                if (abs(entity.acceleration.x) < epsilon32) {
                    entity.velocity.x *= 0.8f;
                }
                
            }
        }
        
        // Draw to render texture
        BeginTextureMode(render_target);
        ClearBackground(BACKGROUND_COLOR);
        
        
#if 0
        game_state.camera_p.x = player.p.x * game_state.meters_to_pixels - game_state.game_width/2.0f;
        game_state.camera_p.x = round(game_state.camera_p.x) * game_state->pixels_to_meters;
        game_state.camera_p.y = player.p.y * game_state.meters_to_pixels - game_state.game_height/2.0f;
        game_state.camera_p.y = round(game_state.camera_p.y) * game_state->pixels_to_meters;
#endif
        
        
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 3; x++) {
                int px = (int) round(x*game_state.texture_background.width-game_state.camera_p.x * game_state->meters_to_pixels);
                int py = (int) round(y*game_state.texture_background.height-game_state.camera_p.y * game_state->meters_to_pixels);
                DrawTexture(game_state.texture_background, px, py, WHITE);
            }
        }
        
        int tile_xcount = (int) (game_state.texture_tiles.width/game_state.meters_to_pixels);
        for (int y = 0; y < game_state.tile_map_height; y++) {
            for (int x = 0; x < game_state.tile_map_width; x++) {
                Tile tile = game_state.tile_map[y*game_state.tile_map_width + x];
                if (tile == 0) continue;
                tile--;
                
                Rectangle src = { 0, 0, game_state.meters_to_pixels, game_state.meters_to_pixels };
                src.x = (tile % tile_xcount) * game_state.meters_to_pixels;
                src.y = (tile / tile_xcount) * game_state.meters_to_pixels;
                
                Rectangle dest = { 0, 0, game_state.meters_to_pixels, game_state.meters_to_pixels };
                dest.x = (x - game_state.camera_p.x) * game_state.meters_to_pixels;
                dest.y = (y - game_state.camera_p.y) * game_state.meters_to_pixels;
                DrawTexturePro(game_state.texture_tiles, src, dest, origin, 0.0f, WHITE);
            }
        }
        
        for (int i = 0; i < game_state.entity_count; i++) {
            Entity* entity = &game_state.entities[i];
            if (!entity.type) continue;
            
            if (entity.texture) {
                v2 p = to_pixel(&game_state, entity.p);
                v2 size = entity.size * game_state->meters_to_pixels;
                
                bool facing_right = entity.facing_dir > 0.0f;
                if (entity.flip_texture) {
                    facing_right = !facing_right;
                }
                
                
                Rectangle src = { 0.0f, 0.0f, size.width, size.height };
                if (entity.num_frames > 0 && entity.is_grounded) {
                    int frame_index = (int) entity.frame_advance;
                    src.x += size.width*frame_index;
                }
                
                Rectangle dest = { p.x, p.y, size.width, size.height };
                if (facing_right) {
                    src.width = -src.width;
                }
                DrawTexturePro(*entity.texture, src, dest, origin, 0.0f, WHITE);
                
            } else {
                v2 p = to_pixel(&game_state, entity.p);
                v2 size = entity.size * game_state.meters_to_pixels;
                DrawRectangle((int) p.x, (int) p.y,
                              (int) size.x, (int) size.y, entity.color);
            }
            
            
            switch (entity.type) {
                case Entity_Type.Boss_Dragon: {
                    Particle_System* ps = game_state.ps_fire;
                    BeginBlendMode(BlendMode.BLEND_MULTIPLIED);
                    for (int j = 0; j < ps.particle_count; j++) {
                        Particle* particle = &ps.particles[j];
                        if (particle.t > 0.0f) {
                            v2 p = to_pixel(&game_state, particle.p);
                            Color color = ORANGE;
                            color.r = (u8) (color.r*particle.t);
                            color.g = (u8) (color.g*particle.t);
                            color.b = (u8) (50*particle.t);
                            color.a = (u8) (particle.t*particle.t*particle.t*255.0f);
                            DrawCircle((int) p.x, (int) p.y, (1.0f - particle.t*particle.t)*10.0f, color);
                        }
                    }
                    BeginBlendMode(BlendMode.BLEND_ALPHA);
                    
#if BUILD_DEBUG && 0
                    Ray ray;
                    v2 rpos = to_pixel(&game_state, game_state.ps_fire.start_p);
                    ray.position = { rpos.x, rpos.y, 0.0f };
                    ray.direction = { 1.0f, 1.0f, 0.0f };
                    DrawRay(ray, PURPLE);
                    
                    ray.direction = { 1.0f, 1.6f, 0.0f };
                    DrawRay(ray, PURPLE);
                    
                    ray.direction = { 1.0f, 0.6f, 0.0f };
                    DrawRay(ray, PURPLE);
#endif
                    
                }
                
                
                case Entity_Type.Player: {
                }
            }
            
            
#if 0
            if (entity.holding) {
                v2 size = entity.size * game_state.meters_to_pixels * 0.3f;
                v2 p = entity.p + vec2(entity.size.x/2.0f + 0.4f * entity.facing_dir, 0.5f);
                
                if (IsKeyDown(KeyboardKey.KEY_S)) {
                    p.y += entity.size.y - 0.5f ;
                }
                
                p = to_pixel(&game_state, p) - size.x/2.0f;
                
                Rectangle r = { p.x, p.y, size.x, size.y };
                DrawRectanglePro(r, origin, 0.0f, entity.holding.color);
            }
#endif
        }
        
        draw_level_bounds(&game_state);
        
        
        
        draw_health_bar(&game_state, game_state.boss_enemy, RED, false);
        draw_health_bar(&game_state, game_state.player, SKYBLUE, true);
        
        //DrawRectangle(5, 5, game_state.game_width/2-10, 8, BLACK);
        //DrawRectangle(6, 6, game_state.game_width/2-12, 6, RED);
        
        
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
            Rectangle dest = { 0, 0, 0, (f32) game_state.screen_height };
            dest.width = aspect_ratio*game_state.screen_height;
            dest.x = (game_state.screen_width - dest.width)/2.0f;
            DrawTexturePro(render_texture, src, dest, origin, 0, WHITE);
            
#if BUILD_DEBUG
            {
                string s = string_print("%", game_state.mode);
                cstring cs = string_to_cstring(s);
                DrawText(cs, 8, game_state.screen_height - 38, 18, GREEN);
                cstring_free(cs);
            }
            
            DrawFPS(8, game_state.screen_height - 24);
#endif
            
            EndDrawing();
        }
    }
    
    CloseWindow();        // Close window and OpenGL context
    
    ExitProcess(0);
    return 0;
}