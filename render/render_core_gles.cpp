#define STBI_ONLY_PNG
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#define GL_CALL(call)                                \
 call;                                               \
 {                                                   \
  GLenum error = glGetError();                       \
  if(error != GL_NO_ERROR)                           \
   r_gles_debug_callback(error, __FILE__, __LINE__); \
 }

static R_GLES_State *r_gles_state;

static void
r_gles_debug_callback(GLenum error, char *file, U64 line)
{
#define gl_errors_table  \
 X(GL_INVALID_ENUM)      \
 X(GL_INVALID_VALUE)     \
 X(GL_INVALID_OPERATION) \
 X(GL_STACK_OVERFLOW)    \
 X(GL_STACK_UNDERFLOW)   \
 X(GL_OUT_OF_MEMORY)     \
 X(GL_INVALID_FRAMEBUFFER_OPERATION)

 switch(error)
 {

#define X(enum)                                                                    \
 case enum:                                                                        \
 {                                                                                 \
  os_print_debug_string("OpenGL error: %s in %s:%d", Stringify(enum), file, line); \
  Assert(false);                                                                   \
 }                                                                                 \
 break;

  gl_errors_table;
#undef X
 }
}

static void
r_init()
{
 Arena *arena = arena_alloc();
 r_gles_state = push_array<R_GLES_State>(arena, 1);
 r_gles_state->arena = arena;
 r_gles_state->frame_arena = arena_alloc();
}

static R_Handle
r_make_render_window_context(OS_Handle window_os)
{
 R_Handle result = {};
 R_GLES_Window *window = r_gles_state->first_free_window;
 if(window == 0)
 {
  window = push_array<R_GLES_Window>(r_gles_state->arena, 1);
 }
 else
 {
  SLLStackPop(r_gles_state->first_free_window);
  MemoryZeroStruct(window);
 }
 window->window_os = window_os;
 result.u64[0] = IntFromPtr(window);
 return result;
}

static void
r_destroy_render_window_context(R_Handle handle)
{
 if(!r_handle_match(handle, r_handle_zero()))
 {
  R_GLES_Window *window = (R_GLES_Window *)PtrFromInt(handle.u64[0]);
  SLLStackPush(r_gles_state->first_free_window, window);
 }
 else
 {
  os_print_debug_string(Str8Lit("Tried to destroy a null render window context"));
 }
}

static R_Handle
r_make_pipeline(R_PipelineDesc desc)
{
 R_Handle result = {};

 R_GLES_Pipeline *pipeline = r_gles_state->first_free_pipeline;

 if(pipeline == 0)
 {
  pipeline = push_array<R_GLES_Pipeline>(r_gles_state->arena, 1);
 }
 else
 {
  SLLStackPop(r_gles_state->first_free_pipeline);
  MemoryZeroStruct(pipeline);
 }

 GL_CALL(GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER));
 GL_CALL(glShaderSource(vertex_shader, 1, (const GLchar *const *)&desc.shader.vs_source.data, (GLint *)&desc.shader.vs_source.size));
 GL_CALL(glCompileShader(vertex_shader));

 S32 success = 0;
 GL_CALL(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success));
 if(success != GL_TRUE)
 {
  StaticArray<U8, 512> info_log = {};
  S32 info_log_length = 0;
  GL_CALL(glGetShaderInfoLog(vertex_shader, array_count(info_log), &info_log_length, (GLchar *)info_log.val));
  os_print_debug_string("Vertex shader failed to compile: %S", str8((U8 *)info_log.val, (U64)info_log_length));
  Assert(false);
 }

 GL_CALL(GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
 GL_CALL(glShaderSource(fragment_shader, 1, (const GLchar *const *)&desc.shader.ps_source.data, (GLint *)&desc.shader.ps_source.size));
 GL_CALL(glCompileShader(fragment_shader));

 GL_CALL(glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success));
 if(success != GL_TRUE)
 {
  StaticArray<U8, 512> info_log = {};
  S32 info_log_length = 0;
  GL_CALL(glGetShaderInfoLog(fragment_shader, array_count(info_log), &info_log_length, (GLchar *)info_log.val));
  os_print_debug_string("Fragment shader failed to compile: %S", str8(info_log.val, (U64)info_log_length));
  Assert(false);
 }

 GL_CALL(pipeline->shader = glCreateProgram());
 GL_CALL(glAttachShader(pipeline->shader, vertex_shader));
 GL_CALL(glAttachShader(pipeline->shader, fragment_shader));
 GL_CALL(glLinkProgram(pipeline->shader));

 GL_CALL(glGetProgramiv(pipeline->shader, GL_LINK_STATUS, &success));
 if(success != GL_TRUE)
 {
  StaticArray<U8, 512> info_log = {};
  S32 info_log_length = 0;
  GL_CALL(glGetProgramInfoLog(pipeline->shader, array_count(info_log), &info_log_length, (GLchar *)info_log.val));
  GL_CALL(os_print_debug_string("Shader program failed to link: %S", str8(info_log.val, (U64)info_log_length)));
  Assert(false);
 }

 GL_CALL(glDeleteShader(vertex_shader));
 GL_CALL(glDeleteShader(fragment_shader));
 GL_CALL(glUseProgram(pipeline->shader));

 GL_CALL(glGenVertexArrays(1, &pipeline->vao));
 GL_CALL(glBindVertexArray(pipeline->vao));

 pipeline->input_layout_desc = desc.input_layout;

 GL_CALL(glBindVertexArray(0));

 switch(desc.topology)
 {
  case R_Topology_TriangleStrip:
  {
   pipeline->draw_mode = GL_TRIANGLE_STRIP;
  }
  break;
  case R_Topology_TriangleList:
  {
   pipeline->draw_mode = GL_TRIANGLES;
  }
  break;
   invalid_case;
 }

 switch(desc.sample_filter)
 {
  case R_SampleFilter_Bilinear:
  {
   pipeline->sample_filter = GL_LINEAR;
  }
  break;
  case R_SampleFilter_Nearest:
  {
   pipeline->sample_filter = GL_NEAREST;
  }
  break;
   invalid_case;
 }
 result.u64[0] = IntFromPtr(pipeline);
 return result;
}

static void
r_destroy_pipeline(R_Handle handle)
{
 if(!r_handle_match(handle, r_handle_zero()))
 {
  R_GLES_Pipeline *pipeline = (R_GLES_Pipeline *)PtrFromInt(handle.u64[0]);
  GL_CALL(glDeleteProgram(pipeline->shader));
  SLLStackPush(r_gles_state->first_free_pipeline, pipeline);
 }
 else
 {
  // TODO(hampus): logging
 }
}

static R_Handle
r_make_buffer(R_BufferDesc desc)
{
 R_Handle result = {};
 R_GLES_Buffer *buffer = r_gles_state->first_free_buffer;
 if(buffer == 0)
 {
  buffer = push_array<R_GLES_Buffer>(r_gles_state->arena, 1);
 }
 else
 {
  SLLStackPop(r_gles_state->first_free_buffer);
  MemoryZeroStruct(buffer);
 }
 GL_CALL(glGenBuffers(1, &buffer->vbo));
 GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo));
 GL_CALL(glBufferData(GL_ARRAY_BUFFER, desc.size, 0, GL_DYNAMIC_DRAW));
 GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
 result.u64[0] = IntFromPtr(buffer);
 return result;
}

static void
r_destroy_buffer(R_Handle handle)
{
 if(!r_handle_match(handle, r_handle_zero()))
 {
  R_GLES_Buffer *buffer = (R_GLES_Buffer *)PtrFromInt(handle.u64[0]);
  GL_CALL(glDeleteBuffers(1, &buffer->vbo));
  SLLStackPush(r_gles_state->first_free_buffer, buffer);
 }
}

static R_Handle
r_make_tex2d_from_bitmap(void *data, U32 width, U32 height)
{
 R_Handle result = {};
 R_GLES_Tex2D *tex2d = r_gles_state->first_free_tex2d;
 if(tex2d == 0)
 {
  tex2d = push_array<R_GLES_Tex2D>(r_gles_state->arena, 1);
 }
 else
 {
  SLLStackPop(r_gles_state->first_free_tex2d);
  MemoryZeroStruct(tex2d);
 }

 GL_CALL(glGenTextures(1, &tex2d->texture));
 GL_CALL(glBindTexture(GL_TEXTURE_2D, tex2d->texture));

 GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
 GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
 GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
 GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

 GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

 tex2d->width = width;
 tex2d->height = height;

 result.u64[0] = IntFromPtr(tex2d);

 return result;
}

static R_Handle
r_make_tex2d_from_memory(String8 data)
{
 R_Handle result = {};
 S32 width = 0;
 S32 height = 0;
 S32 channels = 0;
 U8 *bitmap_data = stbi_load_from_memory(data.data, data.size, &width, &height, &channels, 0);
 result = r_make_tex2d_from_bitmap(bitmap_data, safe_u32_from_s32(width), safe_u32_from_s32(height));
 Assert(channels == 4);
 return result;
}

static void
r_destroy_tex2d(R_Handle handle)
{
 if(!r_handle_match(handle, r_handle_zero()))
 {
  R_GLES_Tex2D *texture = (R_GLES_Tex2D *)PtrFromInt(handle.u64[0]);
  GL_CALL(glDeleteTextures(1, &texture->texture));
  SLLStackPush(r_gles_state->first_free_tex2d, texture);
 }
 else
 {
  // TODO(hampus): logging
 }
}

static void
r_update_tex2d_contents(R_Handle handle, void *memory)
{
 if(!r_handle_match(handle, r_handle_zero()))
 {
  R_GLES_Tex2D *texture = (R_GLES_Tex2D *)PtrFromInt(handle.u64[0]);
  GL_CALL(glBindTexture(GL_TEXTURE_2D, texture->texture));
  GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, memory));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
 }
 else
 {
  // TODO(hampus): logging
 }
}

static void
r_begin_pass(Vec4F32 clear_color)
{
 ArenaClear(r_gles_state->frame_arena);
 GL_CALL(glDisable(GL_SCISSOR_TEST));
 GL_CALL(glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a));
 GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
 GL_CALL(glEnable(GL_SCISSOR_TEST));
}

static void
r_end_pass()
{
 r_gles_state->active_buffer = 0;
 r_gles_state->active_tex2d = 0;
 r_gles_state->active_pipeline = 0;
}

static void
r_draw(U64 vertex_count)
{
 if(r_gles_state->active_tex2d != 0)
 {
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_gles_state->active_pipeline->sample_filter));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_gles_state->active_pipeline->sample_filter));
 }
 GL_CALL(glDrawArrays(r_gles_state->active_pipeline->draw_mode, 0, vertex_count));
}

static void
r_instanced_draw(U64 vertex_count_per_instance, U64 instance_count)
{
 if(r_gles_state->active_tex2d != 0)
 {
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_gles_state->active_pipeline->sample_filter));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_gles_state->active_pipeline->sample_filter));
 }
 GL_CALL(glDrawArraysInstanced(r_gles_state->active_pipeline->draw_mode, 0, vertex_count_per_instance, instance_count));
}

static void
r_commit()
{
 if(r_gles_state->active_window_context == 0)
 {
  os_print_debug_string(Str8Lit("Tried to commit the renderer without an active window context"));
 }
 else
 {
  os_swap_buffers(r_gles_state->active_window_context->window_os);
 }

 r_gles_state->active_window_context = 0;
}

static void
r_apply_pipeline(R_Handle pipeline)
{
 if(!r_handle_match(pipeline, r_handle_zero()))
 {
  r_gles_state->active_pipeline = (R_GLES_Pipeline *)PtrFromInt(pipeline.u64[0]);
  GL_CALL(glEnable(GL_BLEND));
  GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL_CALL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO));
  GL_CALL(glUseProgram(r_gles_state->active_pipeline->shader));
  GL_CALL(glBindVertexArray(r_gles_state->active_pipeline->vao));
 }
 else
 {
  os_print_debug_string(Str8Lit("Tried to apply a null pipeline"));
  // TODO(hampus): logging
 }
}

static U64
r_byte_size_from_attribute_kind(R_AttributeKind kind)
{
 U64 result = 0;
 switch(kind)
 {
  case R_AttributeKind_Float:
  {
   result = 1;
  }
  break;
  case R_AttributeKind_Float2:
  {
   result = 2;
  }
  break;
  case R_AttributeKind_Float3:
  {
   result = 3;
  }
  break;
  case R_AttributeKind_Float4:
  {
   result = 4;
  }
  break;
   invalid_case;
 }
 return result;
}

static void
r_apply_vertex_buffer(R_Handle buffer, U64 stride)
{
 if(!r_handle_match(buffer, r_handle_zero()))
 {
  r_gles_state->active_buffer = (R_GLES_Buffer *)PtrFromInt(buffer.u64[0]);
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r_gles_state->active_buffer->vbo));
  if(r_gles_state->active_pipeline != 0)
  {
   // TODO(hampus): Test the performance of this.
   R_GLES_Pipeline *pipeline = r_gles_state->active_pipeline;
   for(U64 attrib_idx = 0; attrib_idx < pipeline->input_layout_desc.attribs_count; ++attrib_idx)
   {
    R_InputLayoutAttribute *attrib = &pipeline->input_layout_desc.attribs[attrib_idx];
    GLint size = r_byte_size_from_attribute_kind(attrib->kind);
    GL_CALL(glVertexAttribPointer(attrib_idx, size, GL_FLOAT, GL_FALSE, stride, (void *)(attrib->offset)));
    GL_CALL(glVertexAttribDivisor(attrib_idx, 1));
    GL_CALL(glEnableVertexAttribArray(attrib_idx));
   }
  }
  else
  {
   os_print_debug_string(Str8Lit("Need an active pipeline to apply a buffer!"));
  }
 }
 else
 {
  os_print_debug_string(Str8Lit("Tried to apply a null vertex buffer"));
 }
}

static void
r_apply_uniform_buffer(R_Handle buffer)
{
}

static void
r_apply_tex2d(R_Handle tex2d)
{
 if(!r_handle_match(tex2d, r_handle_zero()))
 {
  r_gles_state->active_tex2d = (R_GLES_Tex2D *)PtrFromInt(tex2d.u64[0]);
  GL_CALL(glBindTexture(GL_TEXTURE_2D, r_gles_state->active_tex2d->texture));
 }
 else
 {
  os_print_debug_string(Str8Lit("Tried to apply a null tex2d"));
 }
}

static void
r_apply_window_context(R_Handle window_context)
{
 if(!r_handle_match(window_context, r_handle_zero()))
 {
  r_gles_state->active_window_context = (R_GLES_Window *)PtrFromInt(window_context.u64[0]);
 }
 else
 {
  os_print_debug_string(Str8Lit("Tried to apply a null window context"));
 }
}

static void
r_apply_scissor_rect(RectF32 rect)
{
 R_GLES_Window *window = r_gles_state->active_window_context;
 Vec2U64 viewport_dim = os_client_area_from_window(window->window_os);
 GLint x = (GLint)rect.x0;
 GLint y = (GLint)rect.y1;
 GLint width = (GLint)(rect.x1 - rect.x0);
 GLint height = (GLint)(rect.y1 - rect.y0);
 glScissor(x, viewport_dim.y - y, width, height);
}

static void
r_fill_buffer(R_Handle handle, R_FillBufferDesc desc)
{
 if(!r_handle_match(handle, r_handle_zero()))
 {
  R_GLES_Buffer *buffer = (R_GLES_Buffer *)PtrFromInt(handle.u64[0]);
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo));
  GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, desc.data.size, desc.data.data));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
 }
 else
 {
  os_print_debug_string(Str8Lit("Tried to fill a null buffer"));
 }
}

static void
r_gles_set_uniform_4x4f32(String8 name, U8 *data)
{
 TempArena scratch = GetScratch(0, 0);
 GL_CALL(GLint loc = glGetUniformLocation(r_gles_state->active_pipeline->shader, cstr_from_str8(scratch.arena, name)));
 GL_CALL(glUniformMatrix4fv(loc, 1, GL_FALSE, (F32 *)data));
}