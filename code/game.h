
#define RLAPI @link("raylib.dll") extern
#include "raylib.h"

#include "math.h"
#include "tokenizer.h"
#include "memory.h"


enum Entity_Type {
    None,
    Player,
    Box,
    Box_Collider,
    Boss_Dragon
}

enum Game_Mode {
    Control_Player,
    Control_Boss_Enemy,
}

#define NUM_ATTACKS 3


struct Entity {
    Entity_Type type;
    
    v2 p;
    v2 size;
    f32 facing_dir;
    
    v2 velocity;
    v2 acceleration;
    v3 max_speed;
    
    Entity* holding;
    
    v2 texture_size;
    bool flip_texture;
    Texture* texture;
    Color color;
    
    int num_frames;
    f32 frame_advance_rate;
    f32 frame_advance;
    
    [NUM_ATTACKS]f32 attack_time;
    [NUM_ATTACKS]f32 attack_cooldown;
    
    s32 health;
    s32 max_health;
    
    bool is_grounded;
    bool is_jumping;
    bool is_attacking;
    bool is_rigidbody;
    
    u32 pad;
}

struct Game_State {
    Game_Mode mode;
    
    Entity* player;
    Entity* boss_enemy;
    
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
    
    Particle_System* ps_fire;
    
    Texture2D texture_tiles;
    Texture2D texture_background;
    Texture2D texture_player;
    Texture2D texture_dragon;
}

