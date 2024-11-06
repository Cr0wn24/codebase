//////////////////////////////
// NOTE(hampus): Vector functions

static Vec2F32
v2f32(F32 x, F32 y)
{
  Vec2F32 result = {};
  result.x = x;
  result.y = y;
  return result;
}

Vec2F32
Vec2F32::operator+(Vec2F32 other)
{
  Vec2F32 result = {};
  result.x = x + other.x;
  result.y = y + other.y;
  return result;
}

Vec2F32 &
Vec2F32::operator+=(Vec2F32 other)
{
  *this = *this + other;
  return *this;
}

Vec2F32
Vec2F32::operator-(Vec2F32 other)
{
  Vec2F32 result = {};
  result.x = x - other.x;
  result.y = y - other.y;
  return result;
}

Vec2F32 &
Vec2F32::operator-=(Vec2F32 other)
{
  *this = *this - other;
  return *this;
}

Vec2F32
Vec2F32::operator*(F32 t)
{
  Vec2F32 result = {};
  result.x = x * t;
  result.y = y * t;
  return result;
}

Vec2F32 &
Vec2F32::operator*=(F32 t)
{
  *this = *this * t;
  return *this;
}

Vec2F32
Vec2F32::operator/(F32 t)
{
  Vec2F32 result = *this * (1.0f / t);
  return result;
}

Vec2F32 &
Vec2F32::operator/=(F32 t)
{
  *this = *this * (1.0f / t);
  return *this;
}

F32 &
Vec2F32::operator[](U64 idx)
{
  Assert(idx < 2);
  F32 &result = v[idx];
  return result;
}

static F32
distance_2f32(Vec2F32 v)
{
  F32 result = sqrt_f32(square(v.x) + square(v.y));
  return result;
}

static Vec3F32
v3f32(F32 x, F32 y, F32 z)
{
  Vec3F32 result = {};
  result.x = x;
  result.y = y;
  result.z = z;
  return result;
}

Vec3F32
Vec3F32::operator-(Vec3F32 other)
{
  Vec3F32 result = {};
  result.x = x - other.x;
  result.y = y - other.y;
  return result;
}

Vec3F32
Vec3F32::operator*(F32 t)
{
  Vec3F32 result = {};
  result.x = x * t;
  result.y = y * t;
  result.z = z * t;
  return result;
}

Vec3F32 &
Vec3F32::operator*=(F32 t)
{
  *this = *this * t;
  return *this;
}

F32 &
Vec3F32::operator[](U64 idx)
{
  Assert(idx < 3);
  F32 &result = v[idx];
  return result;
}

static Vec3F32
cross_3f32(Vec3F32 a, Vec3F32 b)
{
  Vec3F32 result = {};
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

static Vec3F32
negate_3f32(Vec3F32 v)
{
  Vec3F32 result = {};
  result.x = -v.x;
  result.y = -v.y;
  result.z = -v.z;
  return result;
}

static F32
length_3f32(Vec3F32 v)
{
  F32 result = sqrt_f32(square(v.x) + square(v.y) + square(v.z));
  return result;
}

static Vec3F32
norm_3f32(Vec3F32 v)
{
  Vec3F32 result = {};
  F32 length = length_3f32(v);
  result.x = v.x / length;
  result.y = v.y / length;
  result.z = v.z / length;
  return result;
}

static Vec4F32
v4f32(F32 x, F32 y, F32 z, F32 w)
{
  return ((Vec4F32){x, y, z, w});
}

static Vec4F32
v4f32(F32 x, F32 y, Vec2F32 zw)
{
  return v4f32(x, y, zw.x, zw.y);
}

Vec4F32
Vec4F32::operator+(Vec4F32 other)
{
  Vec4F32 result = {};
  result.x = x + other.x;
  result.y = y + other.y;
  result.z = z + other.z;
  result.w = w + other.w;
  return result;
}

Vec4F32
Vec4F32::operator*(F32 t)
{
  Vec4F32 result = {};
  result.x = x * t;
  result.y = y * t;
  result.z = z * t;
  result.w = w * t;
  return result;
}

Vec4F32 &
Vec4F32::operator*=(F32 t)
{
  *this = *this * t;
  return *this;
}

Vec4F32
Vec4F32::operator/(F32 t)
{
  Vec4F32 result = *this * t;
  return result;
}

// hampus: F64

static Vec2F64
v2f64(F64 x, F64 y)
{
  return ((Vec2F64){x, y});
}

static Vec3F64
v3f64(F64 x, F64 y, F64 z)
{
  return ((Vec3F64){x, y, z});
}

static Vec4F64
v4f64(F64 x, F64 y, F64 z, F64 w)
{
  return ((Vec4F64){x, y, z, w});
}

// hampus: S32

static Vec2S32
v2s32(S32 x, S32 y)
{
  return ((Vec2S32){x, y});
}

static Vec3S32
v3s32(S32 x, S32 y, S32 z)
{
  return ((Vec3S32){x, y, z});
}

static Vec4S32
v4s32(S32 x, S32 y, S32 z, S32 w)
{
  return ((Vec4S32){x, y, z, w});
}

// hampus: S64

static Vec2S64
v2s64(S64 x, S64 y)
{
  return ((Vec2S64){x, y});
}

static Vec3S64
v3s64(S64 x, S64 y, S64 z)
{
  return ((Vec3S64){x, y, z});
}

static Vec4S64
v4s64(S64 x, S64 y, S64 z, S64 w)
{
  return ((Vec4S64){x, y, z, w});
}

// hampus: U32

static Vec2U32
v2u32(U32 x, U32 y)
{
  return ((Vec2U32){x, y});
}

static Vec3U32
v3u32(U32 x, U32 y, U32 z)
{
  return ((Vec3U32){x, y, z});
}

static Vec4U32
v4u32(U32 x, U32 y, U32 z, U32 w)
{
  return ((Vec4U32){x, y, z, w});
}

// hampus: U64

static Vec2U64
v2u64(U64 x, U64 y)
{
  return ((Vec2U64){x, y});
}

B32
Vec2U64::operator==(Vec2U64 other)
{
  B32 result = x == other.x && y == other.y;
  return result;
}

B32
Vec2U64::operator!=(Vec2U64 other)
{
  B32 result = !(*this == other);
  return result;
}

Vec2U64
Vec2U64::operator+(Vec2U64 other)
{
  Vec2U64 result = {};
  result.x = x + other.x;
  result.y = y + other.y;
  return result;
}

static Vec3U64
v3u64(U64 x, U64 y, U64 z)
{
  return ((Vec3U64){x, y, z});
}

static Vec4U64
v4u64(U64 x, U64 y, U64 z, U64 w)
{
  return ((Vec4U64){x, y, z, w});
}

//////////////////////////////
// NOTE(hampus): Matrix functions

static Mat4F32
make_4x4f32(F32 x)
{
  Mat4F32 result =
  {
    x,
    0,
    0,
    0,
    0,
    x,
    0,
    0,
    0,
    0,
    x,
    0,
    0,
    0,
    0,
    x,
  };
  return result;
}

Mat4F32
Mat4F32::operator*(Mat4F32 v)
{
  Mat4F32 result = {};

  result.m[0][0] = m[0][0] * v.m[0][0] + m[0][1] * v.m[1][0] + m[0][2] * v.m[2][0] + m[0][3] * v.m[3][0];
  result.m[0][1] = m[0][0] * v.m[0][1] + m[0][1] * v.m[1][1] + m[0][2] * v.m[2][1] + m[0][3] * v.m[3][1];
  result.m[0][2] = m[0][0] * v.m[0][2] + m[0][1] * v.m[1][2] + m[0][2] * v.m[2][2] + m[0][3] * v.m[3][2];
  result.m[0][3] = m[0][0] * v.m[0][3] + m[0][1] * v.m[1][3] + m[0][2] * v.m[2][3] + m[0][3] * v.m[3][3];

  result.m[1][0] = m[1][0] * v.m[0][0] + m[1][1] * v.m[1][0] + m[1][2] * v.m[2][0] + m[1][3] * v.m[3][0];
  result.m[1][1] = m[1][0] * v.m[0][1] + m[1][1] * v.m[1][1] + m[1][2] * v.m[2][1] + m[1][3] * v.m[3][1];
  result.m[1][2] = m[1][0] * v.m[0][2] + m[1][1] * v.m[1][2] + m[1][2] * v.m[2][2] + m[1][3] * v.m[3][2];
  result.m[1][3] = m[1][0] * v.m[0][3] + m[1][1] * v.m[1][3] + m[1][2] * v.m[2][3] + m[1][3] * v.m[3][3];

  result.m[2][0] = m[2][0] * v.m[0][0] + m[2][1] * v.m[1][0] + m[2][2] * v.m[2][0] + m[2][3] * v.m[3][0];
  result.m[2][1] = m[2][0] * v.m[0][1] + m[2][1] * v.m[1][1] + m[2][2] * v.m[2][1] + m[2][3] * v.m[3][1];
  result.m[2][2] = m[2][0] * v.m[0][2] + m[2][1] * v.m[1][2] + m[2][2] * v.m[2][2] + m[2][3] * v.m[3][2];
  result.m[2][3] = m[2][0] * v.m[0][3] + m[2][1] * v.m[1][3] + m[2][2] * v.m[2][3] + m[2][3] * v.m[3][3];

  result.m[3][0] = m[3][0] * v.m[0][0] + m[3][1] * v.m[1][0] + m[3][2] * v.m[2][0] + m[3][3] * v.m[3][0];
  result.m[3][1] = m[3][0] * v.m[0][1] + m[3][1] * v.m[1][1] + m[3][2] * v.m[2][1] + m[3][3] * v.m[3][1];
  result.m[3][2] = m[3][0] * v.m[0][2] + m[3][1] * v.m[1][2] + m[3][2] * v.m[2][2] + m[3][3] * v.m[3][2];
  result.m[3][3] = m[3][0] * v.m[0][3] + m[3][1] * v.m[1][3] + m[3][2] * v.m[2][3] + m[3][3] * v.m[3][3];

  return result;
}

static Mat4F32
make_perspective_4x4f32(F32 fov, F32 aspect, F32 near_z, F32 far_z)
{
  Mat4F32 result = make_4x4f32(1);

  F32 tangent = tanf(fov / 2);
  F32 top = near_z * tangent;
  F32 right = top * aspect;

  result.m[0][0] = near_z / right;
  result.m[1][1] = near_z / top;
  result.m[2][2] = -(far_z + near_z) / (far_z - near_z);
  result.m[3][2] = -1;
  result.m[2][3] = -(2 * far_z * near_z) / (far_z - near_z);
  result.m[3][3] = 0;
  return result;
}

static Mat4F32
make_ortho_4x4f32(F32 left, F32 right, F32 bottom, F32 top, F32 n, F32 f)
{
  Mat4F32 result = {};

  result.m[0][0] = 2 / (right - left);
  result.m[0][1] = 0;
  result.m[0][2] = 0;
  result.m[0][3] = 0;

  result.m[1][0] = 0;
  result.m[1][1] = 2 / (top - bottom);
  result.m[1][2] = 0;
  result.m[1][3] = 0;

  result.m[2][0] = 0;
  result.m[2][1] = 0;
  result.m[2][2] = 1 / (n - f);
  result.m[2][3] = 0;

  result.m[3][0] = (left + right) / (left - right);
  result.m[3][1] = (top + bottom) / (bottom - top);
  result.m[3][2] = n / (n - f);
  result.m[3][3] = 1;

  return result;
}

static Mat4F32
make_look_at_4x4f32(Vec3F32 pos, Vec3F32 target, Vec3F32 up)
{
  Mat4F32 result = make_4x4f32(1);

  Vec3F32 dir = norm_3f32(pos - target);
  Vec3F32 right = norm_3f32(cross_3f32(up, dir));
  Vec3F32 up2 = norm_3f32(cross_3f32(dir, right));

  Mat4F32 rotation = make_4x4f32(1);
  rotation.m[0][0] = right.x;
  rotation.m[0][1] = right.y;
  rotation.m[0][2] = right.z;

  rotation.m[1][0] = up2.x;
  rotation.m[1][1] = up2.y;
  rotation.m[1][2] = up2.z;

  rotation.m[2][0] = dir.x;
  rotation.m[2][1] = dir.y;
  rotation.m[2][2] = dir.z;

  result = rotation * make_translate_4x4f32(negate_3f32(pos));

  return result;
}

static Mat4F32
make_translate_4x4f32(Vec3F32 trans)
{
  Mat4F32 result = make_4x4f32(1);
  result.m[0][3] = trans.x;
  result.m[1][3] = trans.y;
  result.m[2][3] = trans.z;
  return result;
}

static Mat4F32
make_scale_4x4f32(Vec3F32 scale)
{
  Mat4F32 result = make_4x4f32(1);
  result.m[0][0] = scale.x;
  result.m[1][1] = scale.y;
  result.m[2][2] = scale.z;
  return result;
}

static Mat4F32
make_rotate_4x4f32(F32 radians, Vec3F32 axis)
{
  F32 cos = cos_f32(radians);
  F32 sin = sin_f32(radians);

  F32 inv_cos = 1.0f - cos;

  Mat4F32 result = make_4x4f32(1);

  result.m[0][0] = square(axis.x) * inv_cos + cos;
  result.m[0][1] = axis.x * axis.y * inv_cos - axis.z * sin;
  result.m[0][2] = axis.x * axis.z * inv_cos + axis.y * sin;
  result.m[0][3] = 0;

  result.m[1][0] = axis.x * axis.y * inv_cos + axis.z * sin;
  result.m[1][1] = square(axis.y) * inv_cos + cos;
  result.m[1][2] = axis.y * axis.z * inv_cos - axis.x * sin;
  result.m[1][3] = 0;

  result.m[2][0] = axis.x * axis.z * inv_cos - axis.y * sin;
  result.m[2][1] = axis.y * axis.z * inv_cos + axis.x * sin;
  result.m[2][2] = square(axis.z) * inv_cos + cos;
  result.m[2][3] = 0;

  result.m[3][0] = 0;
  result.m[3][1] = 0;
  result.m[3][2] = 0;
  result.m[3][3] = 1;

  return result;
}

//////////////////////////////
// NOTE(hampus): Mat3 functions

static Mat3F32
make_3x3f32(F32 x)
{
  Mat3F32 result =
  {
    x,
    0,
    0,
    0,
    x,
    0,
    0,
    0,
    x,
  };
  return result;
}

Mat3F32
Mat3F32::operator*(Mat3F32 v)
{
  Mat3F32 result = {};
  result.m[0][0] = m[0][0] * v.m[0][0] + m[0][1] * v.m[1][0] + m[0][2] * v.m[2][0];
  result.m[1][0] = m[1][0] * v.m[0][0] + m[1][1] * v.m[1][0] + m[1][2] * v.m[2][0];
  result.m[2][0] = m[1][0] * v.m[0][0] + m[1][1] * v.m[1][0] + m[1][2] * v.m[2][0];

  result.m[0][1] = m[0][0] * v.m[0][1] + m[0][1] * v.m[1][1] + m[0][2] * v.m[2][1];
  result.m[1][1] = m[1][0] * v.m[0][1] + m[1][1] * v.m[1][1] + m[1][2] * v.m[2][1];
  result.m[2][1] = m[1][0] * v.m[0][1] + m[1][1] * v.m[1][1] + m[1][2] * v.m[2][1];

  result.m[0][2] = m[0][0] * v.m[0][2] + m[0][1] * v.m[1][2] + m[0][2] * v.m[2][2];
  result.m[1][2] = m[1][0] * v.m[0][2] + m[1][1] * v.m[1][2] + m[1][2] * v.m[2][2];
  result.m[2][2] = m[1][0] * v.m[0][2] + m[1][1] * v.m[1][2] + m[1][2] * v.m[2][2];
  return result;
}

static Mat3F32
make_translate_3x3f32(Vec2F32 p)
{
  Mat3F32 result =
  {
    1,
    0,
    p.x,
    0,
    1,
    p.y,
    0,
    0,
    1,
  };
  return result;
}

static Mat3F32
make_scale_3x3f32(Vec2F32 scale)
{
  Mat3F32 result =
  {
    scale.x,
    0,
    0,
    0,
    scale.y,
    0,
    0,
    0,
    1,
  };
  return result;
}

static Mat3F32
make_rotate_3x3f32(F32 radians)
{
  F32 a = cos_f32(radians);
  F32 b = sin_f32(radians);
  Mat3F32 result =
  {
    a,
    -b,
    0,
    b,
    a,
    0,
    0,
    0,
    1,
  };
  return result;
}

//////////////////////////////
// NOTE(hampus): Rect functions

static RectF32
r4f32(Vec2F32 min, Vec2F32 max)
{
  return ((RectF32){min, max});
}

static RectF32
r4f32(F32 x0, F32 y0, F32 x1, F32 y1)
{
  return r4f32(v2f32(x0, y0), v2f32(x1, y1));
}

static Vec2F32
center_r4f32(RectF32 rect)
{
  Vec2F32 result = {};
  result.x = (rect.x1 + rect.x0) / 2;
  result.y = (rect.y1 + rect.y0) / 2;
  return result;
}

static Vec2F32
dim_r4f32(RectF32 rect)
{
  Vec2F32 result = {};
  result.x = rect.x1 - rect.x0;
  result.y = rect.y1 - rect.y0;
  return result;
}

static B32
r4f32_contains_2f32(RectF32 rect, Vec2F32 v)
{
  B32 result = v.x >= rect.x0 && v.x < rect.x1 && v.y >= rect.y0 && v.y < rect.y1;
  return result;
}

static RectF32
r4f32_intersect_r4f32(RectF32 cutter, RectF32 cookie)
{
  RectF32 result = {};
  result.x0 = clamp(cutter.x0, cookie.x0, cutter.x1);
  result.x1 = clamp(cutter.x0, cookie.x1, cutter.x1);
  result.y0 = clamp(cutter.y0, cookie.y0, cutter.y1);
  result.y1 = clamp(cutter.y0, cookie.y1, cutter.y1);
  return result;
}

static RectS32
r4s32(Vec2S32 min, Vec2S32 max)
{
  return ((RectS32){min, max});
}

static Vec2S32
center_r4s32(RectS32 rect)
{
  Vec2S32 result = {};
  result.x = (rect.x1 + rect.x0) / 2;
  result.y = (rect.y1 + rect.y0) / 2;
  return result;
}

static Vec2S32
dim_r4s32(RectS32 rect)
{
  Vec2S32 result = {};
  result.x = rect.x1 - rect.x0;
  result.y = rect.y1 - rect.y0;
  return result;
}

static B32
r4s32_contains_2f32(RectS32 rect, Vec2S32 v)
{
  B32 result = v.x >= rect.x0 && v.x < rect.x1 && v.y >= rect.y0 && v.y < rect.y1;
  return result;
}

static RectU32
r4u32(Vec2U32 min, Vec2U32 max)
{
  return ((RectU32){min, max});
}

static Vec2U32
center_r4u32(RectU32 rect)
{
  Vec2U32 result = {};
  result.x = (rect.x1 + rect.x0) / 2;
  result.y = (rect.y1 + rect.y0) / 2;
  return result;
}

static Vec2U32
dim_r4u32(RectU32 rect)
{
  Vec2U32 result = {};
  result.x = rect.x1 - rect.x0;
  result.y = rect.y1 - rect.y0;
  return result;
}

static B32
r4u32_contains_2u32(RectU32 rect, Vec2U32 v)
{
  B32 result = v.x >= rect.x0 && v.x < rect.x1 && v.y >= rect.y0 && v.y < rect.y1;
  return result;
}

static RectU64
r4u64(Vec2U64 min, Vec2U64 max)
{
  return ((RectU64){min, max});
}

static Vec2U64
center_r4u64(RectU64 rect)
{
  Vec2U64 result = {};
  result.x = (rect.x1 + rect.x0) / 2;
  result.y = (rect.y1 + rect.y0) / 2;
  return result;
}

static Vec2U64
dim_r4u64(RectU64 rect)
{
  Vec2U64 result = {};
  result.x = rect.x1 - rect.x0;
  result.y = rect.y1 - rect.y0;
  return result;
}

static B32
r4u64_contains_2u64(RectU64 rect, Vec2U64 v)
{
  B32 result = v.x >= rect.x0 && v.x < rect.x1 && v.y >= rect.y0 && v.y < rect.y1;
  return result;
}

//////////////////////////////
// NOTE(hampus): Conversion static

static U32
u32_from_s64(S64 s64)
{
  U32 result = 0;
  result = (U32)s64;
  return result;
}

static U64
u64_from_s64(S64 s64)
{
  U64 result = 0;
  result = (U64)s64;
  return result;
}

static U32
u32_from_u64(U64 u64)
{
  U32 result = 0;
  result = (U32)u64;
  return result;
}

static S64
s64_from_u64(U64 u64)
{
  S64 result = 0;
  result = (S64)u64;
  return result;
}

static S32
s32_from_u64(U64 u64)
{
  S32 result = 0;
  result = (S32)u64;
  return result;
}

static U32
u32_from_s32(S32 s32)
{
  U32 result = 0;
  result = (U32)s32;
  return result;
}

static S32
s32_from_u32(U32 u32)
{
  S32 result = 0;
  result = (S32)u32;
  return result;
}

static U64
u64_from_s32(S32 s32)
{
  U64 result = 0;
  result = (U64)s32;
  return result;
}

//////////////////////////////
// NOTE(hampus): Integer functions

static U8
round_up_to_power_2_u8(U8 value, U8 power)
{
  U8 result = (U8)((value + power - 1) & ~(power - 1));
  return result;
}

static U16
round_up_to_power_2_u16(U16 value, U16 power)
{
  U16 result = (U16)((value + power - 1) & ~(power - 1));
  return result;
}

static U32
round_up_to_power_2_u32(U32 value, U32 power)
{
  U32 result = (U32)((value + power - 1) & ~(power - 1));
  return result;
}

static U64
round_up_to_power_2_u64(U64 value, U64 power)
{
  U64 result = (U64)((value + power - 1) & ~(power - 1));
  return result;
}

//////////////////////////////
// NOTE(hampus): F32 functions

static F32
lerp_f32(F32 a, F32 b, F32 t)
{
  F32 result = (1.0f - t) * a + t * b;
  return result;
}

//////////////////////////////
// NOTE(hampus): Misc functions

static Vec3F32
hsv_from_rgb(Vec3F32 rgb)
{
  F32 c_max = max(rgb.x, max(rgb.y, rgb.z));
  F32 c_min = Min(rgb.x, Min(rgb.y, rgb.z));
  F32 delta = c_max - c_min;
  F32 h = ((delta == 0.f) ? 0.f : (c_max == rgb.x)     ? mod_f32((rgb.y - rgb.z) / delta + 6.f, 6.f)
                                  : (c_max == rgb.y)   ? (rgb.z - rgb.x) / delta + 2.f
                                    : (c_max == rgb.z) ? (rgb.x - rgb.y) / delta + 4.f
                                                       : 0.f);
  F32 s = (c_max == 0.f) ? 0.f : (delta / c_max);
  F32 v = c_max;
  Vec3F32 hsv = {h / 6.f, s, v};
  return hsv;
}

static Vec3F32
rgb_from_hsv(Vec3F32 hsv)
{
  F32 h = mod_f32(hsv.x * 360.f, 360.f);
  F32 s = hsv.y;
  F32 v = hsv.z;

  F32 c = v * s;
  F32 x = c * (1.f - abs(mod_f32(h / 60.f, 2.f) - 1.f));
  F32 m = v - c;

  F32 r = 0;
  F32 g = 0;
  F32 b = 0;

  if((h >= 0.f && h < 60.f) || (h >= 360.f && h < 420.f))
  {
    r = c;
    g = x;
    b = 0;
  }
  else if(h >= 60.f && h < 120.f)
  {
    r = x;
    g = c;
    b = 0;
  }
  else if(h >= 120.f && h < 180.f)
  {
    r = 0;
    g = c;
    b = x;
  }
  else if(h >= 180.f && h < 240.f)
  {
    r = 0;
    g = x;
    b = c;
  }
  else if(h >= 240.f && h < 300.f)
  {
    r = x;
    g = 0;
    b = c;
  }
  else if((h >= 300.f && h <= 360.f) || (h >= -60.f && h <= 0.f))
  {
    r = c;
    g = 0;
    b = x;
  }

  Vec3F32 rgb = {r + m, g + m, b + m};
  return (rgb);
}

static F32
srgb_from_linear_f32(F32 value)
{
  F32 result = 0.0f;
  if(value < 0.0031308f)
  {
    result = value * 12.92f;
  }
  else
  {
    result = 1.055f * pow_f32(value, 1.0f / 2.4f) - 0.055f;
  }
  return result;
}

static Vec4F32
srgb_from_linear_4f32(Vec4F32 linear)
{
  Vec4F32 result = v4f32(srgb_from_linear_f32(linear.r),
                         srgb_from_linear_f32(linear.g),
                         srgb_from_linear_f32(linear.b),
                         linear.a);
  return result;
}

static F32
linear_from_srgb_f32(F32 value)
{
  F32 result = 0.0f;
  if(value < 0.04045f)
  {
    result = value / 12.92f;
  }
  else
  {
    result = pow_f32((value + 0.055f) / 1.055f, 2.4f);
  }
  return result;
}

static Vec4F32
linear_from_srgb_4f32(Vec4F32 srgb)
{
  Vec4F32 result = v4f32(linear_from_srgb_f32(srgb.r),
                         linear_from_srgb_f32(srgb.g),
                         linear_from_srgb_f32(srgb.b),
                         srgb.a);
  return result;
}

static Vec4F32
rgba_from_u32(U32 u32)
{
  Vec4F32 result = {};
  result.a = (F32)((u32 >> 24) & 0xff) / 255.0f;
  result.r = (F32)((u32 >> 16) & 0xff) / 255.0f;
  result.g = (F32)((u32 >> 8) & 0xff) / 255.0f;
  result.b = (F32)((u32 >> 0) & 0xff) / 255.0f;
  return result;
}

static U32
u32_from_rgba(Vec4F32 v)
{
  U32 result = 0;
  result = (U32)(v.a * 255.0f) << 24 |
           (U32)(v.r * 255.0f) << 16 |
           (U32)(v.g * 255.0f) << 8 |
           (U32)(v.b * 255.0f);
  return result;
}

static U64
num_digits(U64 x)
{
  U64 result = 0;
  U64 accumulator = 10;

  U64 i = 0;
  for(; i < 19; ++i)
  {
    if(x < accumulator)
    {
      result = i + 1;
      break;
    }
    accumulator *= 10;
  }
  if(i == 18)
  {
    result = 20;
  }
  return result;
}
