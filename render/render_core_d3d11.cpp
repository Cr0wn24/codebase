#pragma warning(push, 0)
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"
#pragma warning(pop)

#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define assert_hr(hr) ASSERT(SUCCEEDED(hr))

global R_D3D11_State *r_d3d11_state;

function void
r_init(void)
{
 Arena *arena = arena_alloc();
 r_d3d11_state = push_array<R_D3D11_State>(arena, 1);
 r_d3d11_state->arena = arena;

 HRESULT hr = {};

 // NOTE(hampus): Create D3D11 device & context
 {
  UINT flags = 0;
#ifndef N_DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};
  hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, flags, levels, ARRAYSIZE(levels),
                         D3D11_SDK_VERSION, &r_d3d11_state->device, 0, &r_d3d11_state->context);
  assert_hr(hr);
 }

 // hampus: Enable useful debug break on errors
#ifndef N_DEBUG
 {
  r_d3d11_state->device->QueryInterface(IID_ID3D11InfoQueue, (void **)&r_d3d11_state->info);
  r_d3d11_state->info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
  r_d3d11_state->info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
  r_d3d11_state->info->Release();
 }

 {
  hr = DXGIGetDebugInterface1(0, IID_IDXGIInfoQueue, (void **)&r_d3d11_state->dxgi_info);
  r_d3d11_state->dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
  r_d3d11_state->dxgi_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
  r_d3d11_state->dxgi_info->Release();
 }
#endif
}

function R_Handle
r_make_render_window_context(OS_Handle window)
{
 R_Handle result = {};

 OS_Win32_Window *win32_window = (OS_Win32_Window *)ptr_from_int(window.u64[0]);
 R_D3D11_Window *d3d11_window = push_array<R_D3D11_Window>(r_d3d11_state->arena, 1);
 d3d11_window->window = window;

 HRESULT hr = {};

 hr = r_d3d11_state->device->QueryInterface(IID_IDXGIDevice, (void **)&r_d3d11_state->dxgi_device);
 assert_hr(hr);

 hr = r_d3d11_state->dxgi_device->GetAdapter(&r_d3d11_state->dxgi_adapter);
 assert_hr(hr);

 hr = r_d3d11_state->dxgi_adapter->GetParent(IID_IDXGIFactory2, (void **)&r_d3d11_state->factory);
 assert_hr(hr);

 {
  DXGI_SWAP_CHAIN_DESC1 desc = {};
  {
   desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   desc.Stereo = FALSE;
   desc.SampleDesc.Count = 1;
   desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   desc.BufferCount = 2;
   desc.Scaling = DXGI_SCALING_STRETCH;
   desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
   desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
  };

  hr = r_d3d11_state->factory->CreateSwapChainForHwnd(r_d3d11_state->device, win32_window->hwnd, &desc, 0, 0, &d3d11_window->swap_chain);
  assert_hr(hr);

  r_d3d11_state->factory->MakeWindowAssociation(win32_window->hwnd, DXGI_MWA_NO_ALT_ENTER);
 }

 result.u64[0] = int_from_ptr(d3d11_window);
 return result;
}

function R_Handle
r_make_pipeline(R_PipelineDesc desc)
{
 R_Handle result = {};
 TempArena scratch = get_scratch(0, 0);

 R_D3D11_Pipeline *pipeline = r_d3d11_state->first_free_pipeline;
 if(pipeline == 0)
 {
  pipeline = push_array<R_D3D11_Pipeline>(r_d3d11_state->arena, 1);
 }
 else
 {
  sll_stack_pop(r_d3d11_state->first_free_pipeline);
 }

 switch(desc.topology)
 {
  case R_Topology_TriangleList:
  {
   pipeline->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
   break;
  }
  case R_Topology_TriangleStrip:
  {
   pipeline->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  }
  break;
 }

 UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifndef NDEBUG
 flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
 flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

 ID3DBlob *error = 0;

 HRESULT hr = {};

 R_ShaderDesc *shader_desc = &desc.shader;

 ASSERT(shader_desc->vs_entry_point_name.size < 256);
 StaticArray<U8, 256> vs_target_buffer_cstr = {};
 cstr_format(vs_target_buffer_cstr.val, array_count(vs_target_buffer_cstr), (char *)"%S_5_0", shader_desc->vs_entry_point_name);

 char *vs_entry_point_cstr = cstr_from_str8(scratch.arena, shader_desc->vs_entry_point_name);

 ID3DBlob *vblob = 0;
 hr = D3DCompile((LPCSTR)shader_desc->vs_source.data, shader_desc->vs_source.size, 0, 0, 0, (LPCSTR)vs_entry_point_cstr, (LPCSTR)vs_target_buffer_cstr.val, flags, 0, &vblob, &error);
 if(FAILED(hr))
 {
  const char *message = (const char *)error->GetBufferPointer();
  OutputDebugStringA(message);
  ASSERT(!"Failed to compile vertex shader!");
 }

 ASSERT(shader_desc->ps_entry_point_name.size < 256);
 StaticArray<U8, 256> ps_target_buffer_cstr = {};
 cstr_format(ps_target_buffer_cstr.val, array_count(ps_target_buffer_cstr), (char *)"%S_5_0", shader_desc->ps_entry_point_name);

 char *ps_entry_point_cstr = cstr_from_str8(scratch.arena, shader_desc->ps_entry_point_name);

 ID3DBlob *pblob = 0;

 hr = D3DCompile((LPCSTR)shader_desc->ps_source.data, shader_desc->ps_source.size, 0, 0, 0, (LPCSTR)ps_entry_point_cstr, (LPCSTR)ps_target_buffer_cstr.val, flags, 0, &pblob, &error);
 if(FAILED(hr))
 {
  const char *message = (const char *)error->GetBufferPointer();
  OutputDebugStringA(message);
  ASSERT(!"Failed to compile pixel shader!");
 }

 r_d3d11_state->device->CreateVertexShader(vblob->GetBufferPointer(), vblob->GetBufferSize(), 0, &pipeline->vshader);
 r_d3d11_state->device->CreatePixelShader(pblob->GetBufferPointer(), pblob->GetBufferSize(), 0, &pipeline->pshader);

 R_InputLayoutDesc *input_layout_desc = &desc.input_layout;

 D3D11_INPUT_ELEMENT_DESC d3d11_input_layout_desc[32] = {};
 for(U64 attrib_idx = 0; attrib_idx < input_layout_desc->attribs_count; ++attrib_idx)
 {
  R_InputLayoutAttribute *attrib = &input_layout_desc->attribs[attrib_idx];
  DXGI_FORMAT format = {};
  D3D11_INPUT_CLASSIFICATION slot_class = {};
  switch(attrib->kind)
  {
   case R_AttributeKind_Float:
   {
    format = DXGI_FORMAT_R32_FLOAT;
   }
   break;
   case R_AttributeKind_Float2:
   {
    format = DXGI_FORMAT_R32G32_FLOAT;
   }
   break;
   case R_AttributeKind_Float3:
   {
    format = DXGI_FORMAT_R32G32B32_FLOAT;
   }
   break;
   case R_AttributeKind_Float4:
   {
    format = DXGI_FORMAT_R32G32B32A32_FLOAT;
   }
   break;
    invalid_case;
  }
  switch(attrib->slot_class)
  {
   case R_InputSlotClass_PerInstance:
   {
    slot_class = D3D11_INPUT_PER_INSTANCE_DATA;
   }
   break;
   case R_InputSlotClass_PerVertex:
   {
    slot_class = D3D11_INPUT_PER_VERTEX_DATA;
   }
   break;
    invalid_case;
  }
  d3d11_input_layout_desc[attrib_idx].SemanticName = cstr_from_str8(scratch.arena, attrib->name);
  d3d11_input_layout_desc[attrib_idx].Format = format;
  d3d11_input_layout_desc[attrib_idx].AlignedByteOffset = safe_u32_from_u64(attrib->offset);
  d3d11_input_layout_desc[attrib_idx].InputSlotClass = slot_class;
  d3d11_input_layout_desc[attrib_idx].SemanticIndex = safe_u32_from_u64(attrib->semantic_index);
  d3d11_input_layout_desc[attrib_idx].InstanceDataStepRate = safe_u32_from_u64(attrib->step_rate);
 }

 r_d3d11_state->device->CreateInputLayout(d3d11_input_layout_desc, safe_u32_from_u64(input_layout_desc->attribs_count), vblob->GetBufferPointer(), vblob->GetBufferSize(), &pipeline->layout);

 vblob->Release();
 pblob->Release();

 {
  D3D11_SAMPLER_DESC sampler_desc =
  {
    .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
    .MaxAnisotropy = 1,
    .MinLOD = -F32_MAX,
    .MaxLOD = +F32_MAX,
  };

  switch(desc.sample_filter)
  {
   case R_SampleFilter_Nearest:
   {
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
   }
   break;
   case R_SampleFilter_Bilinear:
   {
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   }
   break;
    invalid_case;
  }

  r_d3d11_state->device->CreateSamplerState(&sampler_desc, &pipeline->sampler_state);
 }

 {
  D3D11_BLEND_DESC blend_desc = {};
  blend_desc.RenderTarget[0] =
  {
    .BlendEnable = TRUE,
    .SrcBlend = D3D11_BLEND_SRC_ALPHA,
    .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
    .BlendOp = D3D11_BLEND_OP_ADD,
    .SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA,
    .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
    .BlendOpAlpha = D3D11_BLEND_OP_ADD,
    .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
  };
  r_d3d11_state->device->CreateBlendState(&blend_desc, &pipeline->blend_state);
 }

 {
  D3D11_RASTERIZER_DESC rasterizer_desc =
  {
    .FillMode = D3D11_FILL_SOLID,
    .CullMode = D3D11_CULL_NONE,
  };
  r_d3d11_state->device->CreateRasterizerState(&rasterizer_desc, &pipeline->rasterizer_state);
 }

 {
  D3D11_DEPTH_STENCIL_DESC depth_stencil_desc =
  {
    .DepthEnable = !!desc.enable_depth_buffering,
    .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
    .DepthFunc = D3D11_COMPARISON_LESS,
    .StencilEnable = FALSE,
    .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
  };
  r_d3d11_state->device->CreateDepthStencilState(&depth_stencil_desc, &pipeline->depth_stencil_state);
 }

 result.u64[0] = int_from_ptr(pipeline);
 return result;
}

function void
r_destroy_pipeline(R_Handle handle)
{
 R_D3D11_Pipeline *d3d11_pipeline = (R_D3D11_Pipeline *)ptr_from_int(handle.u64[0]);
 d3d11_pipeline->blend_state->Release();
 d3d11_pipeline->depth_stencil_state->Release();
 d3d11_pipeline->rasterizer_state->Release();
 d3d11_pipeline->sampler_state->Release();
 d3d11_pipeline->layout->Release();
 d3d11_pipeline->pshader->Release();
 d3d11_pipeline->vshader->Release();
 sll_stack_push(r_d3d11_state->first_free_pipeline, d3d11_pipeline);
}

function R_Handle
r_make_buffer(R_BufferDesc desc)
{
 R_Handle result = {};
 R_D3D11_Buffer *buffer = r_d3d11_state->first_free_buffer;
 if(buffer == 0)
 {
  buffer = push_array<R_D3D11_Buffer>(r_d3d11_state->arena, 1);
 }
 else
 {
  sll_stack_pop(r_d3d11_state->first_free_buffer);
 }
 D3D11_BUFFER_DESC d3d11_desc = {};
 UINT size = safe_u32_from_u64(desc.size);
 switch(desc.kind)
 {
  case R_BufferKind_Vertex:
  {
   d3d11_desc.ByteWidth = size;
   d3d11_desc.Usage = D3D11_USAGE_DYNAMIC;
   d3d11_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   d3d11_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  }
  break;
  case R_BufferKind_Uniform:
  {
   d3d11_desc.ByteWidth = size + 15 - (size + 15) % 16;
   d3d11_desc.Usage = D3D11_USAGE_DYNAMIC;
   d3d11_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   d3d11_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  }
  break;
 }
 r_d3d11_state->device->CreateBuffer(&d3d11_desc, 0, &buffer->buffer);
 buffer->size = desc.size;
 result.u64[0] = int_from_ptr(buffer);
 return result;
}

function void
r_destroy_buffer(R_Handle handle)
{
 R_D3D11_Buffer *buffer = r_d3d11_state->first_free_buffer;
 buffer->buffer->Release();
 sll_stack_push(r_d3d11_state->first_free_buffer, buffer);
}

function R_Handle
r_make_tex2d_from_bitmap(void *data, U32 width, U32 height)
{
 R_Handle result = {};
 TempArena scratch = get_scratch(0, 0);
 R_D3D11_Texture *d3d11_texture = (R_D3D11_Texture *)r_d3d11_state->first_free_texture;
 if(d3d11_texture != 0)
 {
  sll_stack_pop(r_d3d11_state->first_free_texture);
 }
 else
 {
  d3d11_texture = push_array<R_D3D11_Texture>(r_d3d11_state->arena, 1);
 }

 D3D11_TEXTURE2D_DESC desc =
 {
   .Width = width,
   .Height = height,
   .MipLevels = 1,
   .ArraySize = 1,
   .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
   .SampleDesc = {1, 0},
   .Usage = D3D11_USAGE_DEFAULT,
   .BindFlags = D3D11_BIND_SHADER_RESOURCE,
   .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
 };

 UINT mem_pitch = width * sizeof(U32);

 D3D11_SUBRESOURCE_DATA resource_data =
 {
   .pSysMem = data,
   .SysMemPitch = mem_pitch,
 };

 ID3D11Texture2D *texture = 0;
 ID3D11ShaderResourceView *texture_view = 0;
 r_d3d11_state->device->CreateTexture2D(&desc, &resource_data, &texture);
 r_d3d11_state->device->CreateShaderResourceView((ID3D11Resource *)texture, 0, &texture_view);
 d3d11_texture->view = texture_view;
 d3d11_texture->texture = texture;

 d3d11_texture->width = width;
 d3d11_texture->height = height;

 result.u64[0] = int_from_ptr(d3d11_texture);
 return result;
}

function R_Handle
r_make_tex2d_from_memory(String8 data)
{
 R_Handle result = {};
 int width = 0;
 int height = 0;
 int channels = 0;
 unsigned char *texture_data = stbi_load_from_memory(data.data, safe_s32_from_u64(data.size), &width, &height, &channels, 4);
 result = r_make_tex2d_from_bitmap(texture_data, safe_u32_from_s32(width), safe_u32_from_s32(height));
 stbi_image_free(texture_data);
 return result;
}

function void
r_destroy_tex2d(R_Handle texture)
{
 R_D3D11_Texture *d3d11_texture = (R_D3D11_Texture *)ptr_from_int(texture.u64[0]);
 d3d11_texture->view->Release();
 sll_stack_push(r_d3d11_state->first_free_texture, d3d11_texture);
}

function void
r_fill_tex2d_region(R_Handle texture, RectU64 region, void *memory)
{
 profile_function();
 R_D3D11_Texture *d3d11_texture = (R_D3D11_Texture *)ptr_from_int(texture.u64[0]);
 U32 bytes_per_pixel = 4;
 Vec2S32 dim = v2s32((S32)(region.x1 - region.x0), (S32)(region.y1 - region.y0));
 D3D11_BOX dst_box =
 {
   (UINT)region.x0,
   (UINT)region.y0,
   0,
   (UINT)region.x1,
   (UINT)region.y1,
   1,
 };
 r_d3d11_state->context->UpdateSubresource((ID3D11Resource *)d3d11_texture->texture, 0, &dst_box, memory, dim.x * bytes_per_pixel, 0);
}

function void
r_begin_pass(Vec4F32 clear_color)
{
 R_D3D11_Window *d3d11_window = r_d3d11_state->current_window_context;

 Vec2U64 client_area_dim = os_client_area_from_window(d3d11_window->window);
 HRESULT hr;
 if(d3d11_window->render_target_view == 0 ||
    r_d3d11_state->current_width != (S32)client_area_dim.width ||
    r_d3d11_state->current_height != (S32)client_area_dim.height)
 {
  if(d3d11_window->render_target_view)
  {
   r_d3d11_state->context->ClearState();
   d3d11_window->render_target_view->Release();
   d3d11_window->depth_stencil_view->Release();
   d3d11_window->render_target_view = 0;
  }

  if(client_area_dim.width != 0 && client_area_dim.height != 0)
  {
   hr = d3d11_window->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
   if(FAILED(hr))
   {
    ASSERT(!"Failed to resize swap chain!");
   }

   // NOTE(hampus): Create RenderTarget view for new backbuffer texture
   ID3D11Texture2D *backbuffer = 0;
   d3d11_window->swap_chain->GetBuffer(0, IID_ID3D11Texture2D, (void **)&backbuffer);
   D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
   {
    render_target_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   }
   r_d3d11_state->device->CreateRenderTargetView((ID3D11Resource *)backbuffer, &render_target_view_desc, &d3d11_window->render_target_view);
   backbuffer->Release();

   D3D11_TEXTURE2D_DESC depth_desc = {};
   {
    depth_desc.Width = safe_u32_from_u64(client_area_dim.width);
    depth_desc.Height = safe_u32_from_u64(client_area_dim.height);
    depth_desc.MipLevels = 1;
    depth_desc.ArraySize = 1;
    depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_desc.SampleDesc = {1, 0};
    depth_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   };

   // NOTE(hampus): Create new depth stencil texture & DepthStencil view
   ID3D11Texture2D *depth = 0;
   r_d3d11_state->device->CreateTexture2D(&depth_desc, 0, &depth);
   r_d3d11_state->device->CreateDepthStencilView((ID3D11Resource *)depth, 0, &d3d11_window->depth_stencil_view);
   depth->Release();
  }
  r_d3d11_state->current_width = (S32)client_area_dim.width;
  r_d3d11_state->current_height = (S32)client_area_dim.height;
 }

 D3D11_VIEWPORT viewport =
 {
   .TopLeftX = 0,
   .TopLeftY = 0,
   .Width = (FLOAT)client_area_dim.width,
   .Height = (FLOAT)client_area_dim.height,
   .MinDepth = 0,
   .MaxDepth = 1,
 };

 r_d3d11_state->context->RSSetViewports(1, &viewport);

 if(d3d11_window->render_target_view)
 {
  r_d3d11_state->context->ClearRenderTargetView(d3d11_window->render_target_view, clear_color.v);
  r_d3d11_state->context->ClearDepthStencilView(d3d11_window->depth_stencil_view,
                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
 }
}

function void
r_end_pass(void)
{
}

function void
r_draw(U64 vertex_count)
{
 r_d3d11_state->context->Draw(safe_u32_from_u64(vertex_count), 0);
}

function void
r_instanced_draw(U64 vertex_count_per_instance, U64 instance_count)
{
 r_d3d11_state->context->DrawInstanced(safe_u32_from_u64(vertex_count_per_instance), safe_u32_from_u64(instance_count), 0, 0);
}

function void
r_commit(void)
{
 profile_function();

 HRESULT hr;
 R_D3D11_Window *d3d11_window = r_d3d11_state->current_window_context;
 if(d3d11_window->render_target_view)
 {
  BOOL vsync = TRUE;
  hr = d3d11_window->swap_chain->Present((UINT)(vsync ? 1 : 0), (UINT)0);
  if(hr == DXGI_STATUS_OCCLUDED)
  {
   // NOTE(hampus): Window is minimized, cannot vsync - instead sleep a bit
   if(vsync)
   {
    Sleep(10);
   }
  }
  else if(FAILED(hr))
  {
   ASSERT(!"Failed to present swap chain! Device lost?");
  }
 }
}

function void
r_apply_pipeline(R_Handle pipeline)
{
 R_D3D11_Pipeline *d3d11_pipeline = (R_D3D11_Pipeline *)ptr_from_int(pipeline.u64[0]);
 R_D3D11_Window *d3d11_window = r_d3d11_state->current_window_context;

 r_d3d11_state->context->VSSetShader(d3d11_pipeline->vshader, 0, 0);

 r_d3d11_state->context->IASetInputLayout(d3d11_pipeline->layout);
 r_d3d11_state->context->IASetPrimitiveTopology(d3d11_pipeline->topology);

 r_d3d11_state->context->RSSetState(d3d11_pipeline->rasterizer_state);

 r_d3d11_state->context->PSSetSamplers(0, 1, &d3d11_pipeline->sampler_state);
 r_d3d11_state->context->PSSetShader(d3d11_pipeline->pshader, 0, 0);

 r_d3d11_state->context->OMSetBlendState(d3d11_pipeline->blend_state, 0, ~0U);
 r_d3d11_state->context->OMSetDepthStencilState(d3d11_pipeline->depth_stencil_state, 0);
 r_d3d11_state->context->OMSetRenderTargets(1, &d3d11_window->render_target_view, d3d11_window->depth_stencil_view);
}

function void
r_apply_vertex_buffer(R_Handle buffer, U64 stride)
{
 R_D3D11_Buffer *d3d11_buffer = (R_D3D11_Buffer *)ptr_from_int(buffer.u64[0]);
 ID3D11Buffer *vertex_buffer = d3d11_buffer->buffer;
 UINT offset = 0;
 UINT stride_u32 = safe_u32_from_u64(stride);
 r_d3d11_state->context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride_u32, &offset);
}

function void
r_apply_uniform_buffer(R_Handle buffer)
{
 R_D3D11_Buffer *d3d11_buffer = (R_D3D11_Buffer *)ptr_from_int(buffer.u64[0]);
 r_d3d11_state->context->VSSetConstantBuffers(0, 1, &d3d11_buffer->buffer);
}

function void
r_apply_tex2d(R_Handle tex2d)
{
 R_D3D11_Texture *d3d11_texture = (R_D3D11_Texture *)ptr_from_int(tex2d.u64[0]);
 r_d3d11_state->context->PSSetShaderResources(0, 1, &d3d11_texture->view);
}

function void
r_apply_window_context(R_Handle window_context)
{
 R_D3D11_Window *d3d11_window = (R_D3D11_Window *)ptr_from_int(window_context.u64[0]);
 r_d3d11_state->current_window_context = d3d11_window;
}

function void
r_apply_scissor_rect(RectF32 rect)
{
 D3D11_RECT d3d11_rect = {};
 d3d11_rect.left = (LONG)rect.x0;
 d3d11_rect.top = (LONG)rect.y0;
 d3d11_rect.right = (LONG)rect.x1;
 d3d11_rect.bottom = (LONG)rect.y1;
 r_d3d11_state->context->RSSetScissorRects(1, &d3d11_rect);
}

function void
r_fill_buffer(R_Handle buffer, R_FillBufferDesc desc)
{
 R_D3D11_Buffer *d3d11_buffer = (R_D3D11_Buffer *)ptr_from_int(buffer.u64[0]);
 U64 clamped_size = min(d3d11_buffer->size, desc.data.size);
 ID3D11Buffer *vertex_buffer = d3d11_buffer->buffer;
 D3D11_MAPPED_SUBRESOURCE mapped = {};
 r_d3d11_state->context->Map((ID3D11Resource *)vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
 memory_copy((U8 *)mapped.pData, (U8 *)desc.data.data, clamped_size);
 r_d3d11_state->context->Unmap((ID3D11Resource *)vertex_buffer, 0);
}