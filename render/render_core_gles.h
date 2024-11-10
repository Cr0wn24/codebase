#ifndef RENDER_GLES_H
#define RENDER_GLES_H

#include <GLES3/gl32.h>

#define R_BACKEND_GLES 1

struct R_GLES_Pipeline
{
  R_GLES_Pipeline *next;
  GLenum draw_mode;
  GLuint shader;
  GLuint vao;
  GLint sample_filter;
  R_InputLayoutDesc input_layout_desc;
};

struct R_GLES_Buffer
{
  R_GLES_Buffer *next;
  GLuint vbo;
};

struct R_GLES_Tex2D
{
  R_GLES_Tex2D *next;
  GLuint texture;
  U32 width;
  U32 height;
};

struct R_GLES_Window
{
  R_GLES_Window *next;
  OS_Handle window_os;
};

struct R_GLES_State
{
  Arena *arena;
  Arena *frame_arena;

  R_GLES_Window *first_free_window;
  R_GLES_Pipeline *first_free_pipeline;
  R_GLES_Buffer *first_free_buffer;
  R_GLES_Tex2D *first_free_tex2d;

  R_GLES_Window *active_window_context;
  R_GLES_Pipeline *active_pipeline;
  R_GLES_Buffer *active_buffer;
  R_GLES_Tex2D *active_tex2d;
};

static void r_gles_set_uniform_4x4f32(String8 name, U8 *data);

#endif // RENDER_GLES_H
