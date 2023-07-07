union v2 {
    struct {
        f32 x, y;
    };
    struct {
        f32 u, v;
    };
    struct {
        f32 width, height;
    };
    [2]f32 data;
};

const v2 vec2_zero  = {  0.0f,  0.0f };
const v2 vec2_left  = { -1.0f,  0.0f };
const v2 vec2_right = {  1.0f,  0.0f };
const v2 vec2_up    = {  0.0f, -1.0f };
const v2 vec2_down  = {  0.0f,  1.0f };

inline v2
vec2(f32 x, f32 y) {
    v2 result;
    result.x = x;
    result.y = y;
    return result;
}

union v2s {
    struct {
        s32 x, y;
    };
    struct {
        s32 u, v;
    };
    struct {
        s32 width, height;
    };
    [2]s32 data;
};

const v2 vec2s_zero  = {  0,  0 };
const v2 vec2s_left  = { -1,  0 };
const v2 vec2s_right = {  1,  0 };
const v2 vec2s_up    = {  0, -1 };
const v2 vec2s_down  = {  0,  1 };

inline v2s
vec2s(s32 x, s32 y) {
    v2s result;
    result.x = x;
    result.y = y;
    return result;
}

union v3 {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    struct {
        f32 u;
        f32 v;
        f32 w;
    };
    struct {
        f32 r;
        f32 g;
        f32 b;
    };
    struct {
        v2 xy;
        f32 z;
    };
    [3]f32 data;
};

const v3 vec3_zero  = {  0.0f,  0.0f, 0.0f };
const v3 vec3_left  = { -1.0f,  0.0f, 0.0f };
const v3 vec3_right = {  1.0f,  0.0f, 0.0f };
const v3 vec3_up    = {  0.0f, -1.0f, 0.0f };
const v3 vec3_down  = {  0.0f,  1.0f, 0.0f };

inline v3
vec3(f32 x, f32 y, f32 z) {
    v3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

inline v3
vec3(v2 xy, f32 z) {
    v3 result;
    result.x = xy.x;
    result.y = xy.y;
    result.z = z;
    return result;
}

union v4 {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    [4]f32 data;
};

const v4 vec4_zero  = {  0.0f,  0.0f, 0.0f, 0.0f };
const v4 vec4_left  = { -1.0f,  0.0f, 0.0f, 0.0f };
const v4 vec4_right = {  1.0f,  0.0f, 0.0f, 0.0f };
const v4 vec4_up    = {  0.0f, -1.0f, 0.0f, 0.0f };
const v4 vec4_down  = {  0.0f,  1.0f, 0.0f, 0.0f };

inline v4
vec4(f32 x, f32 y, f32 z, f32 w) {
    v4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

union m3x3 {
    struct {
        v3 row0, row1, row2;
    };
    //f32 elem[3][3];
    [9]f32 data;
};

global const m3x3 m3x3_identity = { 
    data: {
        1.0f, 0.0f, 0.0f, 
        0.0f, 1.0f, 0.0f, 
        0.0f, 0.0f, 1.0f
    }
};

union rect2 {
    struct {
        v2 min;
        v2 max;
    };
    struct {
        v2 p0;
        v2 p1;
    };
    struct {
        f32 min_x; 
        f32 min_y;
        f32 max_x; 
        f32 max_y;
    };
    struct {
        f32 left;
        f32 top;
        f32 right;
        f32 bottom;
    };
    [4]f32 data;
};

union rect2s {
    struct {
        v2s min;
        v2s max;
    };
    struct {
        v2s p0;
        v2s p1;
    };
    struct {
        s32 min_x; 
        s32 min_y;
        s32 max_x; 
        s32 max_y;
    };
    struct {
        s32 left;
        s32 top;
        s32 right;
        s32 bottom;
    };
    [4]s32 data;
};

union line2 {
    struct {
        v2 p;
        v2 r;
    };
    struct {
        f32 x;
        f32 y;
        f32 dx;
        f32 dy;
    };
    [4]f32 data;
};


const f32 epsilon32 = 0.0001f;
const f32 pi32 = 3.14159265359f;
const f32 half_pi32 = 0.5f * pi32;
const f32 two_pi32 = 2.0f * pi32;
const f32 deg_to_rad = two_pi32 / 360.0f;
const f32 rad_to_deg = 1.0f / deg_to_rad;


inline f32
cubic_bezier(f32 u, f32 v, f32 t) {
    f32 a = u + v - 2.0f;
    f32 b = -2.0f*u - v + 3.0f;
    f32 c = u;
    return a*t*t*t + b*t*t + c*t;
}

inline f32
lerp(f32 a, f32 b, f32 t) {
    return a*(1.0f - t) + b*t;
}


/***************************************************************************
 * 2D vector functions
 ***************************************************************************/

#define DEF_UNOP_OVERLOADS \
UNOP(-)

#define UNOP(op) \
inline v2 \
operator##op(v2 a) { \
v2 result; \
result.x = op a.x; \
result.y = op a.y; \
return result; \
}


#define DEF_BINOP_OVERLOADS \
BINOP(+) \
BINOP(-) \
BINOP(*) \
BINOP(/)

#define BINOP(op) \
inline v2 \
operator##op(v2 a, v2 b) { \
v2 result; \
result.x = a.x op b.x; \
result.y = a.y op b.y; \
return result; \
} \
\
inline void \
operator##op##=(v2* a, v2 b) { \
a.x op##= b.x; \
a.y op##= b.y; \
} \
inline v2 \
\
operator##op(v2 a, f32 v) { \
v2 result; \
result.x = a.x op v; \
result.y = a.y op v; \
return result; \
} \
\
inline void \
operator##op##=(v2* a, f32 v) { \
a.x op##= v; \
a.y op##= v; \
}


DEF_UNOP_OVERLOADS
DEF_BINOP_OVERLOADS

#undef BIN_OP
#undef DEF_OP_OVERLOADS

inline bool
operator==(v2 a, v2 b) {
    return (a.x == b.x) && (a.y == b.y);
}

inline v2
abs(v2 v) {
    v2 result = vec2(abs(v.x), abs(v.y));
    return result;
}

#if 0
inline v2
hadamard(v2 a, f32 b) {
    v2 result;
    result.x = a.x * b;
    result.y = a.y * b;
    return result;
}
#endif

inline v2
hadamard(v2 a, v2 b) {
    v2 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}

inline f32
dot_product(v2 a, v2 b) {
    return a.x * b.x + a.y * b.y;
}

inline f32
cross_product(v2 a, v2 b) {
    return a.x * b.y - a.y * b.x; // TODO(alexander): not sure if this should be called cross product?
}

inline f32
length_sq(v2 v) {
    return v.x*v.x + v.y*v.y;
}

inline f32
length(v2 v) {
    return sqrt(length_sq(v)); // TODO(alexander): replace sqrtf with own implementation
}

inline v2
normalize(v2 v) {
    return v / length(v);
}

inline f32
angle(v2 a, v2 b) {
    // NOTE(alexander): is there a better way to calculate this?
    // NOTE(alexander): SPEED: maybe instead take in normalized vectors avoid double normalization
    a = normalize(a);
    b = normalize(b);
    return acos(dot_product(a, b));
}

inline f32
determinant(v2 a, v2 b) {
    // NOTE(alexander): a is left column, b is right column
    return (a.x * b.y) - (b.x * a.y);
}

//
//inline v2s
//round_v2_to_v2s(v2 v) {
//v2s result;
//result.x = round_f32_to_s32(v.x);
//result.y = round_f32_to_s32(v.y);
//return result;
//}
//

/***************************************************************************
 * 3D vector functions
 ***************************************************************************/

inline v3
operator+(v3 a, v3 b) {
    v3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline v3
operator-(v3 a, v3 b) {
    v3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline v3
operator*(v3 a, v3 b) {
    v3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

inline v3
operator*(v3 a, f32 v) {
    v3 result;
    result.x = a.x * v;
    result.y = a.y * v;
    result.z = a.z * v;
    return result;
}

inline v3
operator/(v3 a, v3 b) {
    v3 result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    return result;
}

/***************************************************************************
   * 3 by 3 matrix functions
 ***************************************************************************/

inline void
operator*=(v3* v, m3x3 m) {
    v.x = m.row0.x * v.x + m.row0.y * v.y + m.row0.z * v.z;
    v.y = m.row1.x * v.x + m.row1.y * v.y + m.row1.z * v.z;
    v.z = m.row2.x * v.x + m.row2.y * v.y + m.row2.z * v.z;
}

inline v3
operator*(m3x3 m, v3 v) {
    v3 result = v;
    &result *= m;
    return result;
}


inline m3x3
operator*(m3x3 a, m3x3 b) {
    m3x3 result;
    result.row0.x = a.row0.x * b.row0.x + a.row0.y * b.row1.x + a.row0.z * b.row2.x;
    result.row0.y = a.row0.x * b.row0.y + a.row0.y * b.row1.y + a.row0.z * b.row2.y;
    result.row0.y = a.row0.x * b.row0.z + a.row0.y * b.row1.z + a.row0.z * b.row2.z;
    
    result.row1.x = a.row1.x * b.row0.x + a.row1.y * b.row1.x + a.row1.z * b.row2.x;
    result.row1.y = a.row1.x * b.row0.y + a.row1.y * b.row1.y + a.row1.z * b.row2.y;
    result.row1.y = a.row1.x * b.row0.z + a.row1.y * b.row1.z + a.row1.z * b.row2.z;
    
    result.row2.x = a.row2.x * b.row0.x + a.row2.y * b.row1.x + a.row2.z * b.row2.x;
    result.row2.y = a.row2.x * b.row0.y + a.row2.y * b.row1.y + a.row2.z * b.row2.y;
    result.row2.y = a.row2.x * b.row0.z + a.row2.y * b.row1.z + a.row2.z * b.row2.z;
    return result;
}

inline void
translate(m3x3* mat, v2 v) {
    mat.row0.z += v.x;
    mat.row1.z += v.y;
}


inline void
rotate(m3x3* mat, f32 v) {
    f32 cosv = cos(v);
    f32 sinv = sin(v);
    
    mat.row0.x *= cosv;
    mat.row0.y *= -sinv;
    mat.row1.x *= sinv;
    mat.row1.y *= cosv;
}

inline void
scale(m3x3* mat, v2 v) {
    mat.row0.x *= v.x;
    mat.row1.y *= v.y;
}



/***************************************************************************
 * 4D vector functions
 ***************************************************************************/

inline v4
operator+(v4 a, v4 b) {
    v4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline v4
operator*(v4 a, f32 v) {
    v4 result;
    result.x = a.x * v;
    result.y = a.y * v;
    result.z = a.z * v;
    result.w = a.w * v;
    return result;
}

inline v4
operator*(v4 a, v4 b) {
    v4 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;
    return result;
}
