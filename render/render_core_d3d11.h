#ifndef RENDER_CORE_D3D11_H
#define RENDER_CORE_D3D11_H

#define COBJMACROS
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#define R_BACKEND_D3D11 1

struct R_D3D11_Pipeline
{
  R_D3D11_Pipeline *next;
  U64 gen;

  D3D11_PRIMITIVE_TOPOLOGY topology;

  ID3D11VertexShader *vshader;
  ID3D11PixelShader *pshader;
  ID3D11InputLayout *layout;
  ID3D11BlendState *blend_state;
  ID3D11SamplerState *sampler_state;
  ID3D11RasterizerState *rasterizer_state;
  ID3D11DepthStencilState *depth_stencil_state;
};

struct R_D3D11_Buffer
{
  R_D3D11_Buffer *next;
  U64 gen;

  ID3D11Buffer *buffer;
  U64 size;
};

struct R_D3D11_Texture
{
  R_D3D11_Texture *next;
  U64 gen;

  U64 width;
  U64 height;
  ID3D11ShaderResourceView *view;
  ID3D11Texture2D *texture;
};

struct R_D3D11_Window
{
  OS_Handle window;

  IDXGISwapChain1 *swap_chain;
  ID3D11RenderTargetView *render_target_view;
  ID3D11DepthStencilView *depth_stencil_view;
  ID3D11Buffer *uniform_buffer;
};

struct R_D3D11_State
{
  Arena *arena;

  R_D3D11_Pipeline *first_free_pipeline;
  R_D3D11_Buffer *first_free_buffer;
  R_D3D11_Texture *first_free_texture;

  S32 current_width;
  S32 current_height;

  R_D3D11_Window *current_window_context;

  ID3D11Device *device;
  ID3D11DeviceContext *context;
  IDXGIInfoQueue *dxgi_info;
  IDXGIDevice *dxgi_device;
  IDXGIAdapter *dxgi_adapter;
  IDXGIFactory2 *factory;
  ID3D11InfoQueue *info;
};

#endif // RENDER_CORE_D3D11_H
