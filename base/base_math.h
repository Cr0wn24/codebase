#ifndef BASE_MATH_H
#define BASE_MATH_H

#include <math.h>

//////////////////////////////
// NOTE(hampus): Basic macros

#ifndef min
#  define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#  define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define clamp(a, val, b) max((a), min((val), (b)))
#define abs(x) ((x) < 0 ? (-(x)) : (x))

#define cos_f32(x) cosf(x)
#define sin_f32(x) sinf(x)
#define tan_f32(x) tanf(x)
#define round_f32(x) roundf(x)
#define floor_f32(x) floorf(x)
#define ceil_f32(x) ceilf(x)
#define mod_f32(a, b) fmodf(a, b)
#define pow_f32(a, b) powf(a, b)
#define sqrt_f32(a) sqrtf(a)

#define cos_f64(x) cos(x)
#define sin_f64(x) sin(x)
#define tan_f64(x) tan(x)
#define round_f64(x) round(x)
#define floor_f64(x) floor(x)
#define ceil_f64(x) ceil(x)
#define mod_f64(a, b) modf(a, b)
#define pow_f64(a, b) pow(a, b)
#define sqrt_f64(a) sqrt(a)

#define pi64 (3.1415926535897932384626433832795028841971)
#define pi32 ((F32)pi64)

#define radians_from_degrees(v) ((v / 180.0f) * pi32)

#define square(x) ((x) * (x))

//////////////////////////////
// NOTE(hampus): Vector types

// hampus: F32

union Vec2F32
{
  struct
  {
    F32 x, y;
  };
  struct
  {
    F32 width, height;
  };
  struct
  {
    F32 min, max;
  };
  F32 v[2];

  Vec2F32 operator+(Vec2F32 v);
  Vec2F32 &operator+=(Vec2F32 v);
  Vec2F32 operator-(Vec2F32 v);
  Vec2F32 operator*(F32 t);
  Vec2F32 &operator*=(F32 t);
  Vec2F32 operator/(F32 t);
  Vec2F32 &operator/=(F32 t);

  F32 &operator[](U64 idx);
};

union Vec3F32
{
  struct
  {
    F32 x, y, z;
  };
  struct
  {
    F32 r, g, b;
  };
  F32 v[3];

  Vec3F32 operator-(Vec3F32 v);

  F32 &operator[](U64 idx);
};

union Vec4F32
{
  struct
  {
    F32 x, y, z, w;
  };
  struct
  {
    F32 r, g, b, a;
  };
  struct
  {
    F32 ignored[2];
    Vec2F32 zw;
  };
  struct
  {
    Vec3F32 rgb;
  };
  F32 v[4];

  Vec4F32 operator+(Vec4F32 v);
  Vec4F32 operator*(F32 t);
  Vec4F32 operator/(F32 t);
};

// hampus: F64

union Vec2F64
{
  struct
  {
    F64 x, y;
  };
  struct
  {
    F64 width, height;
  };
  struct
  {
    F64 min, max;
  };
  F64 v[2];
};

union Vec3F64
{
  struct
  {
    F64 x, y, z;
  };
  struct
  {
    F64 r, g, b;
  };
  F64 v[3];
};

union Vec4F64
{
  struct
  {
    F64 x, y, z, w;
  };
  struct
  {
    F64 r, g, b, a;
  };
  struct
  {
    Vec3F64 rgb;
  };
  F64 v[4];
};

// hampus: S8

union Vec2S8
{
  struct
  {
    S8 x, y;
  };
  struct
  {
    S8 width, height;
  };
  S8 v[2];
};

union Vec3S8
{
  struct
  {
    S8 x, y, z;
  };
  S8 v[3];
};

union Vec4S8
{
  struct
  {
    S8 x, y, z, w;
  };
  S8 v[4];
};

// hampus: S16

union Vec2S16
{
  struct
  {
    S16 x, y;
  };
  struct
  {
    S16 width, height;
  };
  S16 v[2];
};

union Vec3S16
{

  struct
  {
    S16 x, y, z;
  };
  S16 v[3];
};

union Vec4S16
{
  struct
  {
    S16 x, y, z, w;
  };
  S16 v[4];
};

// hampus: S32

union Vec2S32
{
  struct
  {
    S32 x, y;
  };
  struct
  {
    S32 width, height;
  };
  S32 v[2];
};

union Vec3S32
{
  struct
  {
    S32 x, y, z;
  };
  S32 v[3];
};

union Vec4S32
{
  struct
  {
    S32 x, y, z, w;
  };
  S32 v[4];
};

// hampus: S64

union Vec2S64
{
  struct
  {
    S64 x, y;
  };
  struct
  {
    S64 width, height;
  };
  struct
  {
    S64 min, max;
  };
  S64 v[2];
};

union Vec3S64
{
  struct
  {
    S64 x, y, z;
  };
  S64 v[3];
};

union Vec4S64
{
  struct
  {
    S64 x, y, z, w;
  };
  S64 v[4];
};

// hampus: U8

union Vec2U8
{
  struct
  {
    U8 x, y;
  };
  struct
  {
    U8 width, height;
  };
  U8 v[2];
};

union Vec3U8
{
  struct
  {
    U8 x, y, z;
  };
  U8 v[3];
};

union Vec4U8
{
  struct
  {
    U8 x, y, z, w;
  };
  U8 v[4];
};

// hampus: U16

union Vec2U16
{
  struct
  {
    U16 x, y;
  };
  struct
  {
    U16 width, height;
  };
  U16 v[2];
};

union Vec3U16
{
  struct
  {
    U16 x, y, z;
  };
  U16 v[3];
};

union Vec4U16
{
  struct
  {
    U16 x, y, z, w;
  };
  U16 v[4];
};

// hampus: U32

union Vec2U32
{
  struct
  {
    U32 x, y;
  };
  struct
  {
    U32 width, height;
  };
  U32 v[2];
};

union Vec3U32
{
  struct
  {
    U32 x, y, z;
  };
  U32 v[3];
};

union Vec4U32
{
  struct
  {
    U32 x, y, z, w;
  };
  U32 v[4];
};

// hampus: U64

union Vec2U64
{
  struct
  {
    U64 x, y;
  };
  struct
  {
    U64 width, height;
  };
  struct
  {
    U64 min, max;
  };
  U64 v[2];

  B32 operator==(Vec2U64 other);
  B32 operator!=(Vec2U64 other);
  Vec2U64
  operator+(Vec2U64 v);
};

union Vec3U64
{
  struct
  {
    U64 x, y, z;
  };
  U64 v[3];
};

union Vec4U64
{
  struct
  {
    U64 x, y, z, w;
  };
  U64 v[4];
};

// hampus: Matrices

struct Mat2F32
{
  F32 m[2][2];
};

struct Mat3F32
{
  F32 m[3][3];
  Mat3F32 operator*(Mat3F32 m);
};

struct Mat4F32
{
  F32 m[4][4];
  Mat4F32 operator*(Mat4F32 m);
};

//////////////////////////////
// NOTE(hampus): Compound math types

// hampus: Floating point

union RectF32
{
  struct
  {
    Vec2F32 min, max;
  };
  struct
  {
    F32 x0, y0;
    F32 x1, y1;
  };
  Vec2F32 s[2];
};

union RectF64
{
  struct
  {
    Vec2F64 min, max;
  };
  struct
  {
    F64 x0, y0;
    F64 x1, y1;
  };
  Vec2F64 s[2];
};

// hampus: Signed integers

union RectS8
{
  struct
  {
    Vec2S8 min, max;
  };
  struct
  {
    S8 x0, y0;
    S8 x1, y1;
  };
  Vec2S8 v[2];
};

union RectS16
{
  struct
  {
    Vec2S16 min, max;
  };
  struct
  {
    S16 x0, y0;
    S16 x1, y1;
  };
  Vec2S16 v[2];
};

union RectS32
{
  struct
  {
    Vec2S32 min, max;
  };
  struct
  {
    S32 x0, y0;
    S32 x1, y1;
  };
  Vec2S32 v[2];
};

union RectS64
{
  struct
  {
    Vec2S64 min, max;
  };
  struct
  {
    S64 x0, y0;
    S64 x1, y1;
  };
  Vec2S64 v[2];
};

// hampus: Unsigned integers

union RectU8
{
  struct
  {
    Vec2U8 min, max;
  };
  struct
  {
    U8 x0, y0;
    U8 x1, y1;
  };
  Vec2U8 v[2];
};

union RectU16
{
  struct
  {
    Vec2U16 min, max;
  };
  struct
  {
    U16 x0, y0;
    U16 x1, y1;
  };
  Vec2U16 v[2];
};

union RectU32
{
  struct
  {
    Vec2U32 min, max;
  };
  struct
  {
    U32 x0, y0;
    U32 x1, y1;
  };
  Vec2U32 v[2];
};

union RectU64
{
  struct
  {
    Vec2U64 min, max;
  };
  struct
  {
    U64 x0, y0;
    U64 x1, y1;
  };
  Vec2U64 v[2];
};

//////////////////////////////
// NOTE(hampus): Vector functions

// hampus: F32

function Vec2F32 v2f32(F32 x, F32 y);
function F32 distance_2f32(Vec2F32 v);

function Vec3F32 v3f32(F32 x, F32 y, F32 z);
function Vec3F32 cross_3f32(Vec3F32 a, Vec3F32 b);
function Vec3F32 negate_3f32(Vec3F32 v);
function F32 length_3f32(Vec3F32 v);
function Vec3F32 norm_3f32(Vec3F32 v);

function Vec4F32 v4f32(F32 x, F32 y, F32 z, F32 w);
function Vec4F32 v4f32(F32 x, F32 y, Vec2F32 zw);

// hampus: F64

function Vec2F64 v2f64(F64 x, F64 y);

function Vec3F64 v3f64(F64 x, F64 y, F64 z);

function Vec4F64 v4f64(F64 x, F64 y, F64 z, F64 w);

// hampus: S32

function Vec2S32 v2s32(S32 x, S32 y);

function Vec3S32 v3s32(S32 x, S32 y, S32 z);

function Vec4S32 v4s32(S32 x, S32 y, S32 z, S32 w);

// hampus: S64

function Vec2S64 v2s64(S64 x, S64 y);

function Vec3S64 v3s64(S64 x, S64 y, S64 z);

function Vec4S64 v4s64(S64 x, S64 y, S64 z, S64 w);

// hampus: U32

function Vec2U32 v2u32(U32 x, U32 y);

function Vec3U32 v3u32(U32 x, U32 y, U32 z);

function Vec4U32 v4u32(U32 x, U32 y, U32 z, U32 w);

// hampus: U64

function Vec2U64 v2u64(U64 x, U64 y);

function Vec3U64 v3u64(U64 x, U64 y, U64 z);

function Vec4U64 v4u64(U64 x, U64 y, U64 z, U64 w);

//////////////////////////////
// NOTE(hampus): Mat4 functions

function Mat4F32 make_4x4f32(F32 x);
function Mat4F32 make_perspective_4x4f32(F32 fovy, F32 aspect, F32 front, F32 back);
function Mat4F32 make_ortho_4x4f32(F32 left, F32 right, F32 bottom, F32 top, F32 n, F32 f);
function Mat4F32 make_look_at_4x4f32(Vec3F32 pos, Vec3F32 target, Vec3F32 up);
function Mat4F32 make_translate_4x4f32(Vec3F32 trans);
function Mat4F32 make_scale_4x4f32(Vec3F32 scale);
function Mat4F32 make_rotate_4x4f32(F32 radians, Vec3F32 axis);

//////////////////////////////
// NOTE(hampus): Mat3 functions

function Mat3F32 make_3x3f32(F32 x);
function Mat3F32 make_translate_3x3f32(Vec2F32 p);
function Mat3F32 make_scale_3x3f32(Vec2F32 scale);
function Mat3F32 make_rotate_3x3f32(F32 radians);

//////////////////////////////
// NOTE(hampus): Rect functions

// hampus: F32

function RectF32 r4f32(Vec2F32 min, Vec2F32 max);
function RectF32 r4f32(F32 x0, F32 y0, F32 x1, F32 y1);
function Vec2F32 center_r4f32(RectF32 rect);
function Vec2F32 dim_r4f32(RectF32 rect);
function B32 r4f32_contains_2f32(RectF32 rect, Vec2F32 v);
function RectF32 r4f32_intersect_r4f32(RectF32 cutter, RectF32 cookie);

// hampus: S32

function RectS32 r4s32(Vec2S32 min, Vec2S32 max);
function Vec2S32 center_r4s32(RectS32 rect);
function Vec2S32 dim_r4s32(RectS32 rect);
function B32 r4s32_contains_2f32(RectS32 rect, Vec2S32 v);

// hampus: U32

function RectU32 r4u32(Vec2U32 min, Vec2U32 max);
function Vec2U32 center_r4u32(RectU32 rect);
function Vec2U32 dim_r4u32(RectU32 rect);
function B32 r4u32_contains_2u32(RectU32 rect, Vec2U32 v);

// hampus: U64

function RectU64 r4u64(Vec2U64 min, Vec2U64 max);
function Vec2U64 center_r4u64(RectU64 rect);
function Vec2U64 dim_r4u64(RectU64 rect);
function B32 r4u64_contains_2u64(RectU64 rect, Vec2U64 v);

//////////////////////////////
// NOTE(hampus): Constants

read_only global S8 S8_MIN = (S8)0x80;
read_only global S16 S16_MIN = (S16)0x8000;
read_only global S32 S32_MIN = (S32)0x80000000;
read_only global S64 S64_MIN = (S64)0x8000000000000000ll;

read_only global S8 S8_MAX = 0x7F;
read_only global S16 S16_MAX = 0x7FFF;
read_only global S32 S32_MAX = 0x7FFFFFFF;
read_only global S64 S64_MAX = 0x7FFFFFFFFFFFFFFFll;

read_only global U8 U8_MAX = 0xFF;
read_only global U16 U16_MAX = 0xFFFF;
read_only global U32 U32_MAX = 0xFFFFFFFF;
read_only global U64 U64_MAX = 0xFFFFFFFFFFFFFFFFllu;

read_only global F32 F32_EPSILON = 1.1920929e-7f;
read_only global F32 F32_PI = 3.14159265359f;
read_only global F32 F32_TAU = 6.28318530718f;
read_only global F32 F32_E = 2.71828182846f;
read_only global U32 F32_BIAS = 127;
read_only global F32 F32_MAX = 3.402823466e+38F;
read_only global F32 F32_MIN = 1.175494351e-38F;

read_only global F64 F64_EPSILON = 2.220446e-16;
read_only global F64 F64_PI = 3.141592653589793;
read_only global F64 F64_TAU = 6.28318530718;
read_only global F64 F64_E = 2.71828182846;
read_only global U32 F64_BIAS = 1023;

//////////////////////////////
// NOTE(hampus): Conversion function

#if COMPILER_CLANG || COMPILER_GCC

#  define safe_u32_from_s64(x) ({ ASSERT((x) >= 0 && x <= U32_MAX); u32_from_s64(x); })
#  define safe_u64_from_s64(x) ({ ASSERT((x) >= 0); u64_from_s64(x); })
#  define safe_u32_from_u64(x) ({ ASSERT((x) <= U32_MAX); u32_from_u64(x); })
#  define safe_s64_from_u64(x) ({ ASSERT((x) <= S64_MAX); s64_from_u64(x); })
#  define safe_u32_from_s32(x) ({ ASSERT((x) >= 0); u32_from_s32(x); })
#  define safe_s32_from_u64(x) ({ ASSERT((x) <= S32_MAX); s32_from_u64(x); })
#  define safe_s32_from_u32(x) ({ ASSERT((x) <= S32_MAX); s32_from_u32(x); })
#  define safe_u64_from_s32(x) ({ ASSERT((x) >= 0); u64_from_s32(x); })

#else

#  define safe_u32_from_s64(x) u32_from_s64(x)
#  define safe_u64_from_s64(x) u64_from_s64(x)
#  define safe_u32_from_u64(x) u32_from_u64(x)
#  define safe_s64_from_u64(x) s64_from_u64(x)
#  define safe_u32_from_s32(x) u32_from_s32(x)
#  define safe_s32_from_u64(x) s32_from_u64(x)
#  define safe_s32_from_u32(x) s32_from_u32(x)
#  define safe_u64_from_s32(x) u64_from_s32(x)

#endif

function U32 u32_from_s64(S64 s64);
function U64 u64_from_s64(S64 s64);
function U32 u32_from_u64(U64 u64);
function S64 s64_from_u64(U64 u64);
function U32 u32_from_s32(S32 s32);
function S32 s32_from_u64(U64 u64);

//////////////////////////////
// NOTE(hampus): Integer functions

function U8 round_up_to_power_2_u8(U8 value, U8 power);
function U16 round_up_to_power_2_u16(U16 value, U16 power);
function U32 round_up_to_power_2_u32(U32 value, U32 power);
function U64 round_up_to_power_2_u64(U64 value, U64 power);

//////////////////////////////
// NOTE(hampus): F32 functions

function F32 lerp_f32(F32 a, F32 b, F32 t);

//////////////////////////////
// NOTE(hampus): Misc functions

function Vec3F32 hsv_from_rgb(Vec3F32 rgb);
function Vec3F32 rgb_from_hsv(Vec3F32 hsv);
function F32 srgb_from_linear_f32(F32 value);
function Vec4F32 srgb_from_linear_4f32(Vec4F32 linear);
function F32 linear_from_srgb_f32(F32 value);
function Vec4F32 linear_from_srgb_4f32(Vec4F32 srgb);
function Vec4F32 rgba_from_u32(U32 u32);
function U64 num_digits(U64 x);

#endif // BASE_MATH_H
