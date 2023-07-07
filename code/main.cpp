#include <basic.sq>

#define TILE_SIZE 16

#define RLAPI @link("raylib.dll") extern
#include "raylib.h"

#include "tokenizer.h"
#include "memory.h"

#include "math.cpp"

#include "format_tmx.cpp"



enum Entity_Type {
    None,
    Player,
    Box,
    Box_Collider,
}


struct Entity {
    Entity_Type type;
    
    v2 p;
    v2 size;
    f32 facing_dir;
    
    v2 velocity;
    v2 acceleration;
    
    Entity* holding;
    
    Color color;
    
    bool is_grounded;
    bool is_jumping;
    bool is_rigidbody;
    bool pad0;
}

struct Game_State {
    Entity* player;
    
    Entity* entities;
    int entity_count;
    
    []Tile tile_map;
    int tile_map_width;
    int tile_map_height;
    
    int game_width;
    int game_height;
    int game_scale;
    int screen_width;
    int screen_height;
    
    f32 meters_to_pixels;
    f32 pixels_to_meters;
    
    v2 camera_p;
}


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


Entity*
spawn_entity(Game_State* state, Memory_Arena* arena, Entity_Type type) {
    Entity* entity = push_struct(arena, Entity);
    state->entity_count++;
    *entity = {};
    entity.type = type;
    return entity;
}


#define TILE_SIZE 16


Entity*
initialize_level(Game_State* state, Memory_Arena* arena) {
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
    player.p = {3.0f, 2.0f};
    player.size = {1.0f, 2.0f};
    player.is_rigidbody = true;
    player.facing_dir = 1.0f;
    player.color = SKYBLUE;
    
    Entity* box = spawn_entity(state, arena, Entity_Type.Box);
    box.p = {3.0f, 2.0f};
    box.size = {1.0f, 1.0f};
    box.is_rigidbody = true;
    box.color = RED;
    
    box = spawn_entity(state, arena, Entity_Type.Box);
    box.p = {14.0f, 2.0f};
    box.size = {1.0f, 1.0f};
    box.is_rigidbody = true;
    box.color = RED;
    
    return player;
}

inline s32
round_f32_to_s32(f32 value) {
    return (s32) round(value);
}

v2
to_pixel(Game_State state, v2 world_p) {
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
    
    
    Color background = rgb(52, 28, 39);
    
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
    game_state.game_width = 16 * TILE_SIZE;
    game_state.game_height = 10 * TILE_SIZE;
    game_state.game_scale = 4;
    game_state.screen_width = game_state.game_width * game_state.game_scale;
    game_state.screen_height = game_state.game_height * game_state.game_scale;
    
    game_state.meters_to_pixels = TILE_SIZE;
    game_state.pixels_to_meters = 1.0f/game_state.meters_to_pixels;
    
    
    
    SetConfigFlags(ConfigFlags.FLAG_WINDOW_RESIZABLE);
    
    InitWindow(game_state.screen_width, game_state.screen_height, "GMTK Game Jam 2023");
    SetTargetFPS(60);
    
    //cstring tiles = (cstring) "tiles.png";
    Texture2D texture_tiles = LoadTexture("tiles.png");
    Texture2D texture_background = LoadTexture("background.png");
    
    RenderTexture2D render_target = LoadRenderTexture(game_state.game_width, game_state.game_height);
    
    
    Memory_Arena level_arena = {};
    Entity* player = initialize_level(&game_state, &level_arena);
    
    
    while (!WindowShouldClose())
    {
        f32 delta_time = GetFrameTime();
        
        if (IsKeyPressed(KeyboardKey.KEY_F11)) {
            ToggleFullscreen();
        }
        
        
        game_state.screen_width = GetScreenWidth();
        game_state.screen_height = GetScreenHeight();
        
        
        if (IsKeyPressed(KeyboardKey.KEY_R)) {
            player = initialize_level(&game_state, &level_arena);
        }
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
                    if (IsKeyDown(KeyboardKey.KEY_A)) {
                        entity.acceleration.x = -16;
                        entity.facing_dir = -1.0f;
                    }
                    
                    if (IsKeyDown(KeyboardKey.KEY_D)) {
                        entity.acceleration.x = 16;
                        entity.facing_dir = 1.0f;
                    }
                    
                    const f32 entity_max_speed = 6;
                    if (abs(entity.velocity.x) > entity_max_speed) {
                        entity.velocity.x = sign(entity.velocity.x) * entity_max_speed;
                    }
                    
                    if (abs(entity.acceleration.x) > epsilon32 && abs(entity.velocity.x) > epsilon32 &&
                        sign(entity.acceleration.x) != sign(entity.velocity.x)) {
                        entity.acceleration.x *= 2.0f;
                    }
                    
                    if (abs(entity.acceleration.x) < epsilon32) {
                        entity.velocity.x *= 0.8f;
                    }
                    
                    //if (entity.is_jumping && (entity.velocity.y > 0.0f || !IsKeyDown(KeyboardKey.KEY_SPACE))) {
                    //entity.is_jumping = false;
                    //}
                    
                    
                    entity.acceleration.y = entity.is_jumping ? jump_gravity : gravity;
                    
                    if (entity.is_grounded && IsKeyPressed(KeyboardKey.KEY_SPACE)) {
                        entity.velocity.y = initial_velocity;
                        entity.is_jumping = true;
                    }
                    
                    
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
                }
                
                case Entity_Type.Box: {
                    entity.acceleration.y = entity.is_jumping ? jump_gravity : gravity;
                    
                    
                    if (entity.is_grounded && abs(entity.acceleration.x) < epsilon32) {
                        entity.velocity.x *= 0.8f;
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
                
            }
        }
        
        // Draw to render texture
        BeginTextureMode(render_target);
        ClearBackground(DARKGRAY);
        
        
        
        game_state.camera_p.x = player.p.x * game_state.meters_to_pixels - game_state.game_width/2.0f;
        game_state.camera_p.x = round(game_state.camera_p.x) * game_state->pixels_to_meters;
        game_state.camera_p.y = player.p.y * game_state.meters_to_pixels - game_state.game_height/2.0f;
        game_state.camera_p.y = round(game_state.camera_p.y) * game_state->pixels_to_meters;
        
        
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 3; x++) {
                int px = (int) round(x*texture_background.width-game_state.camera_p.x * game_state->meters_to_pixels);
                int py = (int) round(y*texture_background.height-game_state.camera_p.y * game_state->meters_to_pixels);
                DrawTexture(texture_background, px, py, WHITE);
            }
        }
        
        int tile_xcount = (int) (texture_tiles.width/game_state.meters_to_pixels);
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
                DrawTexturePro(texture_tiles, src, dest, origin, 0.0f, WHITE);
            }
        }
        
        for (int i = 0; i < game_state.entity_count; i++) {
            Entity* entity = &game_state.entities[i];
            if (entity.type) {
                {
                    v2 p = to_pixel(game_state, entity.p);
                    v2 size = entity.size * game_state.meters_to_pixels;
                    DrawRectangle((int) p.x, (int) p.y,
                                  (int) size.x, (int) size.y, entity.color);
                }
                
                if (entity.holding) {
                    v2 size = entity.size * game_state.meters_to_pixels * 0.3f;
                    v2 p = entity.p + vec2(entity.size.x/2.0f + 0.4f * entity.facing_dir, 0.5f);
                    
                    if (IsKeyDown(KeyboardKey.KEY_S)) {
                        p.y += entity.size.y - 0.5f ;
                    }
                    
                    p = to_pixel(game_state, p) - size.x/2.0f;
                    
                    Rectangle r = { p.x, p.y, size.x, size.y };
                    DrawRectanglePro(r, origin, 0.0f, entity.holding.color);
                }
            }
        }
        
        draw_level_bounds(&game_state);
        
        //cstring text = "Congrats! You created your first window!";
        //DrawText((s8*) text, 190, 200, 20, LIGHTGRAY);
        EndTextureMode();
        
        // Render to screen
        
        BeginDrawing();
        
        Texture render_texture = render_target.texture;
        Rectangle src = { 0, 0, (f32) render_texture.width, (f32) (-render_texture.height) };
        Rectangle dest = { 0, 0, (f32) game_state.screen_width, (f32) game_state.screen_height };
        DrawTexturePro(render_texture, src, dest, origin, 0, WHITE);
        
        DrawFPS(8, game_state.screen_height - 24);
        EndDrawing();
    }
    
    CloseWindow();        // Close window and OpenGL context
    
    ExitProcess(0);
    return 0;
}