#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if BUILD_DEBUG
#define pln(format, ...) printf(format##"\n", __VA_ARGS__)
#else
#define pln(format, ...)
#endif

typedef int8_t    s8;
typedef uint8_t   u8;
typedef int16_t   s16;
typedef uint16_t  u16;
typedef int32_t   s32;
typedef uint32_t  u32;
typedef int64_t   s64;
typedef uint64_t  u64;
typedef uintptr_t umm;
typedef intptr_t  smm;
typedef float     f32;
typedef double    f64;
typedef int32_t   b32;
typedef const char* cstring;

struct string {
    u8* data;
    smm count;
};


inline umm
cstring_count(cstring str) {
    if (!str) return 0;
    
    umm count = 0;
    u8* curr = (u8*) str;
    while (*curr) {
        count++;
        curr++;
    }
    return count;
}

inline string
string_lit(cstring str) {
    string result;
    result.data = (u8*) str;
    result.count = cstring_count(str);
    return result;
}

inline cstring
string_to_cstring(string str, u8* dest=0) {
    u8* result = dest ? dest : (u8*) malloc(str.count + 1);
    memcpy(result, str.data, str.count);
    result[str.count] = 0;
    return (cstring) result;
}
inline string
cstring_to_string(cstring str) {
    string result;
    result.data = (u8*) str;
    result.count = cstring_count(str);
    return result;
}

inline void
cstring_free(cstring str) {
    free((void*) str);
}

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#ifdef assert
#undef assert
#endif

#if BUILD_DEBUG
void
__assert(const char* expression, const char* file, int line) {
    // TODO(alexander): improve assertion printing,
    // maybe let platform layer decide how to present this?
    fprintf(stderr, "%s:%d: Assertion failed: %s\n", file, line, expression);
    *(int *)0 = 0; // NOTE(alexander): purposfully trap the program
}

#define assert(expression) (void)((expression) || (__assert(#expression, __FILE__, __LINE__), 0))
#else
#define assert(expression)
#endif

#include "raylib.h"

#include "math.h"
#include "tokenizer.h"
#include "memory.h"


enum Entity_Type {
    None,
    Player,
    Box,
    Box_Collider,
    Door,
    Boss_Dragon,
    Bullet,
    Charged_Bullet,
};

enum Game_Mode {
    Intro_Cutscene,
    Control_Boss_Enemy,
    Control_Player,
};

#define NUM_ATTACKS 3


struct Entity {
    Entity_Type type;
    
    v2 p;
    v2 size;
    f32 facing_dir;
    
    v2 velocity;
    v2 acceleration;
    v3 max_speed;
    
    Entity* collided_with;
    bool collided;
    
    Entity* holding;
    
    v2 texture_size;
    bool flip_texture;
    f32 sprite_rot;
    Texture* texture;
    Color color;
    
    int num_frames;
    f32 frame_advance_rate;
    f32 frame_advance;
    
    f32 attack_time[NUM_ATTACKS];
    f32 attack_cooldown[NUM_ATTACKS];
    
    s32 health;
    s32 max_health;
    
    s32 invincibility_frames;
    
    bool is_grounded;
    bool is_jumping;
    bool is_attacking;
    bool is_rigidbody;
    bool is_cornered;
    
    u32 pad;
};

struct Particle {
    v2 p;
    v2 v;
    f32 t;
};

struct Particle_System {
    Particle* particles;
    int max_particle_count;
    int particle_count;
    
    v2 start_p;
    
    f32 speed;
    
    f32 min_angle;
    f32 max_angle;
    
    f32 min_scale;
    f32 max_scale;
    
    f32 spawn_rate;
    f32 delta_t;
};

struct Game_State {
    Game_Mode mode;
    
    f32 cutscene_time;
    
    Entity* player;
    Entity* boss_enemy;
    Entity* bullets[5];
    Entity* charged_bullet;
    Entity* left_door;
    Entity* right_door;
    
    Entity* entities;
    int entity_count;
    
    u8* tile_map;
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
    Particle_System* ps_charging;
    
    Texture2D texture_tiles;
    Texture2D texture_background;
    Texture2D texture_player;
    Texture2D texture_dragon;
    Texture2D texture_door;
    Texture2D texture_bullet;
    Texture2D texture_charged_bullet;
    
    Sound sound_shoot_bullet;
    Sound sound_explosion;
    Sound sound_hurt;
    Sound sound_player_hurt;
    Sound sound_fire_breathing;
    Sound sound_charging;
    Sound sound_lose;
    Sound sound_win;
    
    Font font;
};

#define cutscene_interval(begin, end) (state->cutscene_time > (begin) && state->cutscene_time < (end))
