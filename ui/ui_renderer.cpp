#include "ui/generated/ui_renderer_meta.cpp"

static void
ui_renderer_init()
{
  ui_state->renderer = push_array<UI_RendererState>(ui_state->arena, 1);
  UI_RendererState *renderer = ui_state->renderer;

  R_ShaderDesc shader_desc = {};
#if R_BACKEND_GLES
  META_EMBED_FILE("ui_vertex_shader_gles.glsl", vertex_shader_source);
  META_EMBED_FILE("ui_fragment_shader_gles.glsl", fragment_shader_source);
  shader_desc.vs_source = vertex_shader_source;
  shader_desc.ps_source = fragment_shader_source;
#elif R_BACKEND_D3D11
  META_EMBED_FILE("ui_shader_d3d11.hlsl", hlsl_shader_source);
  String8 hlsl = hlsl_shader_source;
  shader_desc.vs_source = hlsl;
  shader_desc.ps_source = hlsl;
  shader_desc.vs_entry_point_name = str8_lit("vs");
  shader_desc.ps_entry_point_name = str8_lit("ps");
#endif
  R_InputLayoutDesc input_layout_desc = {};
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("MIN"),
                                                     .kind = R_AttributeKind_Float2,
                                                     .offset = member_offset(UI_RectInstance, dst_min),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });

  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("MAX"),
                                                     .kind = R_AttributeKind_Float2,
                                                     .offset = member_offset(UI_RectInstance, dst_max),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("MIN_UV"),
                                                     .kind = R_AttributeKind_Float2,
                                                     .offset = member_offset(UI_RectInstance, src_min),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("MAX_UV"),
                                                     .kind = R_AttributeKind_Float2,
                                                     .offset = member_offset(UI_RectInstance, src_max),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("COLORS"),
                                                     .kind = R_AttributeKind_Float4,
                                                     .offset = member_offset(UI_RectInstance, colors[0]),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("COLORS"),
                                                     .kind = R_AttributeKind_Float4,
                                                     .offset = member_offset(UI_RectInstance, colors[1]),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .semantic_index = 1,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("COLORS"),
                                                     .kind = R_AttributeKind_Float4,
                                                     .offset = member_offset(UI_RectInstance, colors[2]),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .semantic_index = 2,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("COLORS"),
                                                     .kind = R_AttributeKind_Float4,
                                                     .offset = member_offset(UI_RectInstance, colors[3]),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .semantic_index = 3,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("CORNER_RADIUS"),
                                                     .kind = R_AttributeKind_Float4,
                                                     .offset = member_offset(UI_RectInstance, corner_radius),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });
  r_add_input_layout_attribute(&input_layout_desc, (R_AttributeDesc){
                                                     .semantic_name = str8_lit("EXTRA"),
                                                     .kind = R_AttributeKind_Float4,
                                                     .offset = member_offset(UI_RectInstance, extra),
                                                     .slot_class = R_InputSlotClass_PerInstance,
                                                     .step_rate = 1,
                                                   });

  R_PipelineDesc pipeline_desc = {};
  pipeline_desc.shader = shader_desc;
  pipeline_desc.input_layout = input_layout_desc;
  pipeline_desc.topology = R_Topology_TriangleStrip;
  renderer->pipeline = r_make_pipeline(pipeline_desc);

  renderer->vertex_buffer = r_make_buffer((R_BufferDesc){
    .size = 4096 * sizeof(UI_RectInstance),
    .kind = R_BufferKind_Vertex,
    .stride = sizeof(UI_RectInstance),
  });

  renderer->uniform_buffer = r_make_buffer((R_BufferDesc){
    .size = sizeof(Mat4F32) * 2,
    .kind = R_BufferKind_Uniform,
  });
  U32 white = 0xffffffff;
  renderer->white_texture = r_make_tex2d_from_bitmap(&white, 1, 1);
}

static UI_RectInstance *
ui_draw_rect(Vec2F32 min, Vec2F32 max, UI_DrawRectParams params)
{
  UI_RendererState *renderer = ui_state->renderer;
  if(params.slice.tex.u64[0] == 0)
  {
    params.slice.tex = renderer->white_texture;
  }
  UI_RectBatchNode *node = renderer->last_batch_node;
  UI_RectBatch *batch = 0;
  if(node != 0)
  {
    batch = node->v;
    if(batch->count == batch->max_count)
    {
      node = 0;
    }

    if(!r_handle_match(params.slice.tex, renderer->white_texture) &&
       !r_handle_match(params.slice.tex, batch->params.texture))
    {
      if(!r_handle_match(batch->params.texture, renderer->white_texture))
      {
        node = 0;
      }
      else
      {
        batch->params.texture = params.slice.tex;
      }
    }
  }

  if(node != 0)
  {
    if(renderer->clip_rect_stack != batch->params.clip_rect_node)
    {
      node = 0;
    }
  }

  if(node == 0)
  {
    node = push_array<UI_RectBatchNode>(ui_frame_arena(), 1);
    dll_push_back(renderer->first_batch_node, renderer->last_batch_node, node);
    node->v = push_array<UI_RectBatch>(ui_frame_arena(), 1);
    batch = node->v;
    batch->max_count = 4096;
    batch->base = push_array_no_zero<UI_RectInstance>(ui_frame_arena(), batch->max_count);
    batch->params.texture = params.slice.tex;
    batch->params.clip_rect_node = renderer->clip_rect_stack;
  }

  UI_RectInstance *instance = &batch->base[batch->count];
  instance->dst_min = v2f32(round_f32(min.x), round_f32(min.y));
  instance->dst_max = v2f32(round_f32(max.x), round_f32(max.y));
  instance->src_min = params.slice.uv.min;
  instance->src_max = params.slice.uv.max;
  instance->colors[0] = params.color;
  instance->colors[1] = params.color;
  instance->colors[2] = params.color;
  instance->colors[3] = params.color;
  instance->corner_radius.x = params.corner_radius;
  instance->corner_radius.y = params.corner_radius;
  instance->corner_radius.z = params.corner_radius;
  instance->corner_radius.w = params.corner_radius;
  instance->extra.x = params.softness;
  instance->extra.y = params.border_thickness;
  instance->extra.z = (F32) !!r_handle_match(params.slice.tex, renderer->white_texture);
  batch->count += 1;

  return instance;
}

static void
ui_renderer_destroy()
{
  UI_RendererState *renderer = ui_state->renderer;
  renderer->first_batch_node = renderer->last_batch_node = 0;
  renderer->clip_rect_stack = 0;
  r_destroy_pipeline(renderer->pipeline);
  renderer->pipeline = r_handle_zero();
  r_destroy_buffer(renderer->uniform_buffer);
  renderer->uniform_buffer = r_handle_zero();
  r_destroy_buffer(renderer->vertex_buffer);
  renderer->vertex_buffer = r_handle_zero();
  r_destroy_tex2d(renderer->white_texture);
  renderer->white_texture = r_handle_zero();
}

static F32
ui_draw_text(Vec2F32 pos, F_Tag tag, U32 size, String8 string, Vec4F32 color)
{
  TempArena scratch = get_scratch(0, 0);
  F_GlyphRun glyph_run = f_make_glyph_run(scratch.arena, tag, size, string);
  F32 advance = pos.x;
  F_Atlas *atlas = f_atlas();

  F_GlyphRunNode *second_node = glyph_run.first->next;

  for(F_GlyphRunNode *node = glyph_run.first; node != 0; node = node->next)
  {
    F32 xpos = advance + node->metrics.left_bearing;
    F32 ypos = pos.y;
    F32 width = (F32)node->bitmap_size.x;
    F32 height = (F32)node->bitmap_size.y;

    R_Tex2DSlice slice = {};
    slice.tex = atlas->handle;
    slice.uv = node->region_uv;
    UI_DrawRectParams params = {};
    params.color = color;
    params.slice = slice;
    ui_draw_rect(v2f32(xpos, ypos), v2f32(xpos + width, ypos + height), params);

    advance += node->metrics.advance;
  }

  return (advance - pos.x);
}

static RectF32
ui_clip_rect_top()
{
  UI_RendererState *renderer = ui_state->renderer;
  RectF32 result = renderer->clip_rect_stack->v;
  return result;
}

static void
ui_clip_rect_push(RectF32 rect)
{
  UI_RendererState *renderer = ui_state->renderer;
  UI_ClipRectNode *n = push_array<UI_ClipRectNode>(ui_frame_arena(), 1);
  n->v = rect;
  sll_stack_push(renderer->clip_rect_stack, n);
}

static void
ui_clip_rect_pop()
{
  UI_RendererState *renderer = ui_state->renderer;
  sll_stack_pop(renderer->clip_rect_stack);
}

static void
ui_draw_box_hierarchy(UI_Box *root)
{
  if(root->flags & UI_BoxFlag_Disabled)
  {
    root->rect_color00.a *= 0.5f;
    root->rect_color10.a *= 0.5f;
    root->rect_color01.a *= 0.5f;
    root->rect_color11.a *= 0.5f;
    root->text_color.a *= 0.5f;
  }

  if(root->custom_draw)
  {
    root->custom_draw(root);
  }
  else
  {
    F32 animation_delta = (F32)(1.0 - pow_f64(2.0, 3.0f * -10 * ui_state->dt));
    if(ui_box_is_active(root))
    {
      root->active_t += (1.0f - root->active_t) * animation_delta / 2;
    }
    else if(root->active_state == 0)
    {
      root->active_t += (0.0f - root->active_t) * animation_delta / 4;
    }

    if(root->active_state == 1)
    {
      root->active_t += (0.4f - root->active_t) * animation_delta / 4;

      if(abs(0.4f - root->active_t) < 0.1f)
      {
        root->active_state = 2;
      }
    }

    if(root->active_state == 2)
    {
      root->active_t += (0.0f - root->active_t) * animation_delta / 2;

      if(abs(0.0f - root->active_t) < 0.1f)
      {
        root->active_state = 0;
      }
    }

    if(ui_box_is_hot(root))
    {
      root->hot_t += (1.0f - root->hot_t) * animation_delta;
    }
    else
    {
      root->hot_t += (0.0f - root->hot_t) * animation_delta / 4;
    }

    root->active_t = clamp(0, root->active_t, 1.0f);
    root->hot_t = clamp(0, root->hot_t, 1.0f);

    if(root->active_t <= 0.01f)
    {
      root->active_t = 0;
    }

    if(root->hot_t <= 0.01f)
    {
      root->hot_t = 0;
    }

    if(root->flags & UI_BoxFlag_DrawDropShadow)
    {
      Vec2F32 dpi = os_window_dpi(ui_state->os_window);
      F32 pixel_density = os_window_dpi(ui_state->os_window).x / 160.0f;
      // NOTE(hampus): This works, don't ask why
      F32 multiplier = 2500 * square(pixel_density) / 2.0f;
      Vec2F32 min = root->fixed_rect.min;
      Vec2F32 max = root->fixed_rect.max + v2f32(multiplier * 1.2f / dpi.x, multiplier / dpi.x * 1.2f);
      F32 scale = max(max.x - min.x, max.y - min.y) / max(root->fixed_rect.x1 - root->fixed_rect.x0, root->fixed_rect.y1 - root->fixed_rect.y0);
      UI_DrawRectParams params = {};
      params.softness = 10 * pixel_density;
      params.color = v4f32(0, 0, 0, root->alpha);
      UI_RectInstance *instance = ui_draw_rect(min, max, params);
      memory_copy_array(instance->corner_radius.v, (root->corner_radius * scale * 0.7f).v);
    }

    B32 should_draw_background = false;
    if(root->flags & UI_BoxFlag_DrawBackgroundOnActiveOrHot)
    {
      should_draw_background = root->active_t != 0 || root->hot_t != 0;
    }

    if(should_draw_background || (root->flags & UI_BoxFlag_DrawBackground))
    {
      // TODO(hampus): Correct darkening/lightening
      F32 d = 0;

      if(root->flags & UI_BoxFlag_ActiveAnimation)
      {
        d += 0.2f * root->active_t;
      }

      if(root->flags & UI_BoxFlag_HotAnimation)
      {
        d += 0.2f * root->hot_t;
      }

      Vec4F32 *color = &root->rect_color00;
#if 0
      color[0] = linear_from_srgb_4f32(color[0]);
      color[1] = linear_from_srgb_4f32(color[1]);
      color[2] = linear_from_srgb_4f32(color[2]);
      color[3] = linear_from_srgb_4f32(color[3]);
#endif
      color[Corner_TopLeft] = color[Corner_TopLeft] + v4f32(d, d, d, 0);
      color[Corner_TopRight] = color[Corner_TopRight] + v4f32(d, d, d, 0);
      color[Corner_BottomRight] = color[Corner_BottomRight] + v4f32(d, d, d, 0);
      color[Corner_BottomLeft] = color[Corner_BottomLeft] + v4f32(d, d, d, 0);

      color[Corner_TopLeft].a *= root->alpha;
      color[Corner_TopRight].a *= root->alpha;
      color[Corner_BottomRight].a *= root->alpha;
      color[Corner_BottomLeft].a *= root->alpha;

      UI_DrawRectParams params = {};
      params.softness = root->softness;
      params.slice = root->slice;
      UI_RectInstance *instance = ui_draw_rect(root->fixed_rect.min, root->fixed_rect.max, params);
      memory_copy(instance->colors, color, sizeof(Vec4F32) * 4);
      memory_copy_array(instance->corner_radius.v, root->corner_radius.v);
    }

    if(root->flags & UI_BoxFlag_DrawBorder)
    {
      Vec4F32 border_color = root->border_color;
      // border_color = linear_from_srgb_4f32(border_color);
      F32 d = 0;
      if(root->flags & UI_BoxFlag_HotAnimation)
      {
        d += 0.4f * root->hot_t;
      }

      border_color = border_color + v4f32(d, d, d, 0);
      if(root->flags & UI_BoxFlag_FocusAnimation)
      {
        if(ui_box_is_focus(root))
        {
          border_color = v4f32(0.8f, 0.6f, 0.0f, 1.0f);
        }
      }

      border_color.a *= root->alpha;

      UI_DrawRectParams params = {};
      params.border_thickness = root->border_thickness;
      params.color = border_color;
      params.softness = root->softness;
      UI_RectInstance *instance = ui_draw_rect(root->fixed_rect.min, root->fixed_rect.max, params);

      memory_copy_array(instance->corner_radius.v, root->corner_radius.v);
    }

    if(root->flags & UI_BoxFlag_DrawText)
    {
      // root->text_color = linear_from_srgb_4f32(root->text_color);

      F32 advance = f_get_advance(
      root->font_tag, root->font_size, root->string);
      F32 height = f_line_height_from_tag_size(root->font_tag, root->font_size);
      Vec2F32 text_pos = {};
      switch(root->text_align)
      {
        case UI_TextAlign_Center:
        {
          F32 width_left = root->fixed_rect.x1 - root->fixed_rect.x0 - advance;
          F32 height_left = root->fixed_rect.y1 - root->fixed_rect.y0 - height;

          text_pos = v2f32(root->fixed_rect.min.x + width_left / 2,
                           root->fixed_rect.min.y + height_left / 2);
        }
        break;
        case UI_TextAlign_Right:
        {
          F32 width_left = root->fixed_rect.x1 - root->fixed_rect.x0 - advance;
          F32 height_left = root->fixed_rect.y1 - root->fixed_rect.y0 - height;

          text_pos = v2f32(root->fixed_rect.min.x + width_left,
                           root->fixed_rect.min.y + height_left / 2);
        }
        break;
        case UI_TextAlign_Left:
        {
          F32 width_left = root->fixed_rect.x1 - root->fixed_rect.x0 - advance;
          F32 height_left = root->fixed_rect.y1 - root->fixed_rect.y0 - height;

          text_pos = v2f32(root->fixed_rect.min.x,
                           root->fixed_rect.min.y + height_left / 2);
        }
        break;
          invalid_case;
      }
      Vec4F32 text_color = root->text_color;
      text_color.a *= root->alpha;
      ui_draw_text(text_pos, root->font_tag, root->font_size, root->string, text_color);
    }
  }

  if(ui_state->draw_debug_lines)
  {
    UI_DrawRectParams params = {};
    params.border_thickness = 1;
    params.color = v4f32(1, 0, 1, 1);
    ui_draw_rect(root->fixed_rect.min, root->fixed_rect.max, params);
  }

  if(root->flags & UI_BoxFlag_Clip)
  {
    RectF32 top_clip_rect = ui_clip_rect_top();
    RectF32 clip_rect = r4f32_intersect_r4f32(root->fixed_rect, top_clip_rect);
    ui_clip_rect_push(clip_rect);
  }

  for(UI_Box *child = root->last; !ui_box_is_nil(child); child = child->prev)
  {
    ui_draw_box_hierarchy(child);
  }

  if(root->flags & UI_BoxFlag_Clip)
  {
    ui_clip_rect_pop();
  }
}

static void
ui_draw()
{
  profile_function();

  UI_RendererState *renderer = ui_state->renderer;
  renderer->first_batch_node = 0;
  renderer->last_batch_node = 0;

  Vec2U64 client_area = os_client_area_from_window(ui_state->os_window);

  RectF32 clip_rect = {};
  clip_rect.max = v2f32((F32)client_area.x, (F32)client_area.y);
  ui_clip_rect_push(clip_rect);
  ui_draw_box_hierarchy(ui_state->root);
  ui_clip_rect_pop();

  Mat4F32 mats[2] = {};
  mats[0] = make_ortho_4x4f32(0, (F32)client_area.x, (F32)client_area.y, 0, 0, 1);
  mats[1] = make_4x4f32(1);

  r_apply_pipeline(renderer->pipeline);
  r_apply_vertex_buffer(renderer->vertex_buffer, sizeof(UI_RectInstance));

#if R_BACKEND_GLES
  r_gles_set_uniform_4x4f32(str8_lit("projection"), (U8 *)mats[0].m);
  r_gles_set_uniform_4x4f32(str8_lit("transform"), (U8 *)mats[1].m);
#elif R_BACKEND_D3D11
  r_fill_buffer(renderer->uniform_buffer, (R_FillBufferDesc){
                                            .data = str8_struct(&mats),
                                          });
  r_apply_uniform_buffer(renderer->uniform_buffer);
#endif

  for(UI_RectBatchNode *n = renderer->first_batch_node; n != 0; n = n->next)
  {
    UI_RectBatch *batch = n->v;
    r_apply_tex2d(batch->params.texture);
    r_fill_buffer(renderer->vertex_buffer, (R_FillBufferDesc){.data = str8((U8 *)batch->base, batch->count * sizeof(UI_RectInstance))});
    r_apply_scissor_rect(batch->params.clip_rect_node->v);
    r_instanced_draw(4, batch->count);
  }
}