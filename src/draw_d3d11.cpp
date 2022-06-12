#ifdef RENDER_D3D11

#include "display.h"
#include "array.h"
#include "geometry.h"
#include "mesh.h"
#include "draw.h"
#include "os.h"

#include <d3d11_1.h>
#include <string.h>

#include "compiled/color_vs.h"
#include "compiled/color_ps.h"
#include "compiled/texture_vs.h"
#include "compiled/texture_ps.h"
#include "compiled/basic_3d_vs.h"
#include "compiled/basic_3d_ps.h"
#include "compiled/msaa_2x_vs.h"
#include "compiled/msaa_2x_ps.h"
#include "compiled/msaa_4x_vs.h"
#include "compiled/msaa_4x_ps.h"
#include "compiled/msaa_8x_vs.h"
#include "compiled/msaa_8x_ps.h"
#include "compiled/text_vs.h"
#include "compiled/text_ps.h"

#define SafeRelease(ptr) do { if (ptr) { ptr->Release(); ptr = nullptr; } } while (false)

struct Immediate_Vertex {
    Vector3 position;
    u32 color;
    Vector2 uv;
};

struct Shader {
    ID3D11VertexShader *vertex_shader;
    ID3D11PixelShader *pixel_shader;
    
    bool diffuse_texture_clamped = false;
    bool textures_point_sample = false;
    bool depth_test = true;
    bool depth_write = true;
    bool alpha_blend = true;
};

Texture_Map *the_back_buffer;
Texture_Map *the_back_depth_buffer;

Texture_Map *the_offscreen_buffer;
Texture_Map *the_offscreen_depth_buffer;

int render_target_width = 0;
int render_target_height = 0;

Matrix4 view_to_proj_matrix;
Matrix4 world_to_view_matrix;
Matrix4 object_to_world_matrix;
Matrix4 object_to_proj_matrix;

bool draw_is_initted = false;

static Shader *current_shader;
static Texture_Map *current_diffuse_map;
static Texture_Map *current_render_target;
static Texture_Map *current_depth_target;

static bool should_vsync;
bool multisampling;
int num_samples;

static ID3D11Device1 *device;
static ID3D11DeviceContext1 *device_context;
static IDXGISwapChain1 *swap_chain;

static ID3D11RenderTargetView *back_buffer_rtv;
static ID3D11DepthStencilView *back_buffer_dsv;

static ID3D11RasterizerState1 *rasterizer_state;

static ID3D11DepthStencilState *depth_test_enabled_depth_write_enabled;
static ID3D11DepthStencilState *depth_test_enabled_depth_write_disabled;
static ID3D11DepthStencilState *depth_test_disabled_depth_write_enabled;
static ID3D11DepthStencilState *depth_test_disabled_depth_write_disabled;

static ID3D11BlendState *blend_enabled;
static ID3D11BlendState *blend_disabled;

static ID3D11SamplerState *sampler_point_wrap;
static ID3D11SamplerState *sampler_linear_wrap;
static ID3D11SamplerState *sampler_point_clamp;
static ID3D11SamplerState *sampler_linear_clamp;

static ID3D11Buffer *transform_cbo;

static ID3D11InputLayout *mesh_input_layout;
static ID3D11InputLayout *immediate_input_layout;

static const int MAX_IMMEDIATE_VERTICES = 2400;
static Immediate_Vertex *immediate_vertices;
static int num_immediate_vertices;

static ID3D11Buffer *immediate_vbo;

static Shader *compile_shader(char *file_path, u32 num_vs_bytes, const u8 *vs_bytes, u32 num_ps_bytes, const u8 *ps_bytes) {
    Shader *result = new Shader();

    char *data = os_read_entire_file(file_path);
    defer { delete [] data; };

    if (strstr(data, "@DiffuseTextureClamped")) result->diffuse_texture_clamped = true;
    if (strstr(data, "@TexturesPointSample")) result->textures_point_sample = true;
    if (strstr(data, "@NoDepthTest")) result->depth_test = false;
    if (strstr(data, "@NoDepthWrite")) result->depth_write = false;
    if (strstr(data, "@NoBlend")) result->alpha_blend = false;
    
    device->CreateVertexShader(vs_bytes, num_vs_bytes, nullptr, &result->vertex_shader);
    device->CreatePixelShader(ps_bytes, num_ps_bytes, nullptr, &result->pixel_shader);
    
    return result;
}

#define CompileShader(path, name) compile_shader(path, ArrayCount(name##_vs_shader_bytes), name##_vs_shader_bytes, ArrayCount(name##_ps_shader_bytes), name##_ps_shader_bytes)

#include "compiled/init.h"

static void destroy_offscreen_buffer() {
    if (the_offscreen_buffer->texture) {
        ((ID3D11Texture2D *)the_offscreen_buffer->texture)->Release();
        the_offscreen_buffer->texture = nullptr;
    }

    if (the_offscreen_buffer->rtv) {
        ((ID3D11RenderTargetView *)the_offscreen_buffer->rtv)->Release();
        the_offscreen_buffer->rtv = nullptr;
    }

    if (the_offscreen_buffer->srv) {
        ((ID3D11Texture2D *)the_offscreen_buffer->srv)->Release();
        the_offscreen_buffer->srv = nullptr;
    }

    if (the_offscreen_depth_buffer->texture) {
        ((ID3D11Texture2D *)the_offscreen_depth_buffer->texture)->Release();
        the_offscreen_buffer->texture = nullptr;
    }

    if (the_offscreen_depth_buffer->rtv) {
        ((ID3D11DepthStencilView *)the_offscreen_depth_buffer->rtv)->Release();
        the_offscreen_buffer->rtv = nullptr;
    }

    if (the_offscreen_depth_buffer->srv) {
        ((ID3D11Texture2D *)the_offscreen_depth_buffer->srv)->Release();
        the_offscreen_buffer->srv = nullptr;
    }
}

static void create_offscreen_buffer(int width, int height) {
    the_offscreen_buffer = create_texture_rendertarget(width, height, multisampling, num_samples);
    the_offscreen_depth_buffer = create_texture_depthtarget(the_offscreen_buffer);
}

void set_shader(Shader *shader) {
    if (current_shader == shader) return;
    assert(shader);

    immediate_flush();
    
    current_shader = shader;

    device_context->VSSetShader(shader->vertex_shader, nullptr, 0);
    device_context->PSSetShader(shader->pixel_shader, nullptr, 0);

    if (shader->depth_test && shader->depth_write) {
        device_context->OMSetDepthStencilState(depth_test_enabled_depth_write_enabled, 0);
    } else if (shader->depth_test && !shader->depth_write) {
        device_context->OMSetDepthStencilState(depth_test_enabled_depth_write_disabled, 0);
    } else if (!shader->depth_test && shader->depth_write) {
        device_context->OMSetDepthStencilState(depth_test_disabled_depth_write_enabled, 0);
    } else if (!shader->depth_test && !shader->depth_write) {
        device_context->OMSetDepthStencilState(depth_test_disabled_depth_write_disabled, 0);
    }

    if (shader->diffuse_texture_clamped && shader->textures_point_sample) {
        device_context->PSSetSamplers(0, 1, &sampler_point_clamp);
    } else if (shader->diffuse_texture_clamped && !shader->textures_point_sample) {
        device_context->PSSetSamplers(0, 1, &sampler_linear_clamp);
    } else if (!shader->diffuse_texture_clamped && shader->textures_point_sample) {
        device_context->PSSetSamplers(0, 1, &sampler_point_wrap);
    } else if (!shader->diffuse_texture_clamped && !shader->textures_point_sample) {
        device_context->PSSetSamplers(0, 1, &sampler_linear_wrap);
    }
    
    if (shader->alpha_blend) {
        device_context->OMSetBlendState(blend_enabled, nullptr, 0xffffffff);
    } else {
        device_context->OMSetBlendState(blend_disabled, nullptr, 0xffffffff);
    }

    device_context->VSSetConstantBuffers(0, 1, &transform_cbo);
}

static void create_render_target() {
    ID3D11Texture2D *back_buffer = nullptr;
    defer { SafeRelease(back_buffer); };
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device->CreateRenderTargetView(back_buffer, nullptr, &back_buffer_rtv);

    D3D11_TEXTURE2D_DESC depth_buffer_desc = {};
    back_buffer->GetDesc(&depth_buffer_desc);

    depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D *depth_buffer = nullptr;
    defer { SafeRelease(depth_buffer); };
    device->CreateTexture2D(&depth_buffer_desc, nullptr, &depth_buffer);

    device->CreateDepthStencilView(depth_buffer, nullptr, &back_buffer_dsv);

    the_back_buffer->rtv = back_buffer_rtv;
    the_back_depth_buffer->rtv = back_buffer_dsv;
}

static void destroy_render_target() {
    SafeRelease(back_buffer_rtv);
    SafeRelease(back_buffer_dsv);
}

void init_draw(bool vsync, bool multisample, int sample_count) {
    defer { draw_is_initted = true; };
    
    should_vsync = vsync;
    multisampling = multisample;
    num_samples = sample_count;
    
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

    ID3D11Device *base_device = nullptr;
    defer { SafeRelease(base_device); };
    ID3D11DeviceContext *base_device_context = nullptr;
    defer { SafeRelease(base_device_context); };

    UINT device_create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef DEBUG
    device_create_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_create_flags,
                      feature_levels, ArrayCount(feature_levels), D3D11_SDK_VERSION,
                      &base_device, nullptr, &base_device_context);

    base_device->QueryInterface(IID_PPV_ARGS(&device));
    base_device_context->QueryInterface(IID_PPV_ARGS(&device_context));

    IDXGIDevice1 *dxgi_device = nullptr;
    defer { SafeRelease(dxgi_device); };
    device->QueryInterface(IID_PPV_ARGS(&dxgi_device));
    
    IDXGIAdapter *dxgi_adapter = nullptr;
    defer { SafeRelease(dxgi_adapter); };
    dxgi_device->GetAdapter(&dxgi_adapter);

    IDXGIFactory2 *dxgi_factory = nullptr;
    defer { SafeRelease(dxgi_factory); };
    dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory));

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    HWND window_handle = (HWND)display_get_native_window();
    dxgi_factory->CreateSwapChainForHwnd(device, window_handle, &swap_chain_desc, nullptr, nullptr, &swap_chain);
    dxgi_factory->MakeWindowAssociation(window_handle, DXGI_MWA_NO_ALT_ENTER);

    the_back_buffer = new Texture_Map();
    the_back_buffer->width = display_get_width();
    the_back_buffer->height = display_get_height();

    the_back_depth_buffer = new Texture_Map();
    the_back_depth_buffer->width = the_back_buffer->width;
    the_back_depth_buffer->height = the_back_buffer->height;
    
    create_render_target();
    create_offscreen_buffer(the_back_buffer->width, the_back_buffer->height);

    D3D11_RASTERIZER_DESC1 rasterizer_desc = {};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_BACK;
    rasterizer_desc.FrontCounterClockwise = true;
    if (multisample && num_samples > 1) {
        rasterizer_desc.MultisampleEnable = true;
    }
    device->CreateRasterizerState1(&rasterizer_desc, &rasterizer_state);

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
    device->CreateDepthStencilState(&depth_stencil_desc, &depth_test_enabled_depth_write_enabled);

    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    device->CreateDepthStencilState(&depth_stencil_desc, &depth_test_enabled_depth_write_disabled);
    
    depth_stencil_desc.DepthEnable = FALSE;
    device->CreateDepthStencilState(&depth_stencil_desc, &depth_test_disabled_depth_write_disabled);

    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    device->CreateDepthStencilState(&depth_stencil_desc, &depth_test_disabled_depth_write_enabled);

    init_shaders();
    
    {
        D3D11_INPUT_ELEMENT_DESC ied[3] = {};

        ied[0].SemanticName = "POSITION";
        ied[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        ied[0].AlignedByteOffset = 0;
        ied[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        ied[1].SemanticName = "UV";
        ied[1].Format = DXGI_FORMAT_R32G32_FLOAT;
        ied[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        ied[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        ied[2].SemanticName = "NORMAL";
        ied[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        ied[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        ied[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        device->CreateInputLayout(ied, ArrayCount(ied), basic_3d_vs_shader_bytes, ArrayCount(basic_3d_vs_shader_bytes),
                                  &mesh_input_layout);
    }

    {
        D3D11_INPUT_ELEMENT_DESC ied[3] = {};

        ied[0].SemanticName = "POSITION";
        ied[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        ied[0].AlignedByteOffset = 0;
        ied[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        ied[1].SemanticName = "COLOR";
        ied[1].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        ied[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        ied[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        
        ied[2].SemanticName = "UV";
        ied[2].Format = DXGI_FORMAT_R32G32_FLOAT;
        ied[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        ied[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        device->CreateInputLayout(ied, ArrayCount(ied), texture_vs_shader_bytes, ArrayCount(texture_vs_shader_bytes),
                                  &immediate_input_layout);
    }

    D3D11_BUFFER_DESC transform_cbo_bd = {};
    transform_cbo_bd.ByteWidth = 4 * sizeof(Matrix4);
    transform_cbo_bd.Usage = D3D11_USAGE_DYNAMIC;
    transform_cbo_bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    transform_cbo_bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&transform_cbo_bd, nullptr, &transform_cbo);

    {
        D3D11_SAMPLER_DESC sampler_desc = {};
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        device->CreateSamplerState(&sampler_desc, &sampler_linear_wrap);

        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        device->CreateSamplerState(&sampler_desc, &sampler_point_wrap);

        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState(&sampler_desc, &sampler_point_clamp);

        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        device->CreateSamplerState(&sampler_desc, &sampler_linear_clamp);
    }

    {
        D3D11_BLEND_DESC blend_desc = {};
        blend_desc.RenderTarget[0].BlendEnable =  true;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&blend_desc, &blend_enabled);

        blend_desc.RenderTarget[0].BlendEnable = false;
        device->CreateBlendState(&blend_desc, &blend_disabled);
    }
    
    D3D11_BUFFER_DESC immediate_vbo_bd = {};
    immediate_vbo_bd.ByteWidth = MAX_IMMEDIATE_VERTICES * sizeof(Immediate_Vertex);
    immediate_vbo_bd.Usage = D3D11_USAGE_DYNAMIC;
    immediate_vbo_bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    immediate_vbo_bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    device->CreateBuffer(&immediate_vbo_bd, nullptr, &immediate_vbo);

    immediate_vertices = new Immediate_Vertex[MAX_IMMEDIATE_VERTICES];
    num_immediate_vertices = 0;
    
    device_context->RSSetState(rasterizer_state);
    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    view_to_proj_matrix = matrix4_identity();
    world_to_view_matrix = matrix4_identity();
    object_to_world_matrix = matrix4_identity();
    object_to_proj_matrix = matrix4_identity();
}

void resize_render_targets(int width, int height) {
    destroy_offscreen_buffer();
    destroy_render_target();
    
    swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    
    create_render_target();
    create_offscreen_buffer(width, height);

    the_back_buffer->width = width;
    the_back_buffer->height = height;
}

void make_buffers_for_mesh(Mesh *mesh, u32 num_vertices, Mesh_Vertex *buffer, u32 num_indices, u32 *indices) {
    ID3D11Buffer *vbo = nullptr;
    ID3D11Buffer *ibo = nullptr;
    
    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.ByteWidth = num_vertices * sizeof(Mesh_Vertex);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = buffer;
    device->CreateBuffer(&buffer_desc, &subresource_data, &vbo);

    buffer_desc.ByteWidth = num_indices * sizeof(u32);
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    subresource_data.pSysMem = indices;
    device->CreateBuffer(&buffer_desc, &subresource_data, &ibo);

    mesh->vbo = (void *)vbo;
    mesh->ibo = (void *)ibo;
}

void swap_buffers() {
    swap_chain->Present(should_vsync ? 1 : 0, 0);    
}

void immediate_begin() {
    immediate_flush();
}

void immediate_flush() {
    if (!num_immediate_vertices) return;

    D3D11_MAPPED_SUBRESOURCE msr;
    device_context->Map(immediate_vbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, immediate_vertices, num_immediate_vertices * sizeof(Immediate_Vertex));
    device_context->Unmap(immediate_vbo, 0);
    
    UINT stride = sizeof(Immediate_Vertex);
    UINT offset = 0;
    device_context->IASetVertexBuffers(0, 1, &immediate_vbo, &stride, &offset);
    set_vertex_format_to_immediate();

    device_context->Draw(num_immediate_vertices, 0);
    
    num_immediate_vertices = 0;
}

void immediate_vertex(Vector3 position, u32 color, Vector2 uv) {
    immediate_vertices[num_immediate_vertices].position = position;
    immediate_vertices[num_immediate_vertices].color = color;
    immediate_vertices[num_immediate_vertices].uv = uv;
    num_immediate_vertices++;
}

void immediate_quad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color) {
    if (num_immediate_vertices + 6 > MAX_IMMEDIATE_VERTICES) immediate_flush();

    u32 icolor = abgr_color(color);

    immediate_vertex(p0, icolor, uv0);
    immediate_vertex(p1, icolor, uv1);
    immediate_vertex(p2, icolor, uv2);
    
    immediate_vertex(p0, icolor, uv0);
    immediate_vertex(p2, icolor, uv2);
    immediate_vertex(p3, icolor, uv3);
}

void immediate_quad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector4 color) {
    Vector2 uv0 = make_vector2(0.0f, 0.0f);
    Vector2 uv1 = make_vector2(1.0f, 0.0f);
    Vector2 uv2 = make_vector2(1.0f, 1.0f);
    Vector2 uv3 = make_vector2(0.0f, 1.0f);

    immediate_quad(p0, p1, p2, p3, uv0, uv1, uv2, uv3, color);
}

void immediate_quad(Vector2 _p0, Vector2 _p1, Vector2 _p2, Vector2 _p3, Vector2 uv0, Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector4 color) {
    Vector3 p0 = make_vector3(_p0.x, _p0.y, 0.0f);
    Vector3 p1 = make_vector3(_p1.x, _p1.y, 0.0f);
    Vector3 p2 = make_vector3(_p2.x, _p2.y, 0.0f);
    Vector3 p3 = make_vector3(_p3.x, _p3.y, 0.0f);

    immediate_quad(p0, p1, p2, p3, uv0, uv1, uv2, uv3, color);
}

void immediate_quad(Vector2 _p0, Vector2 _p1, Vector2 _p2, Vector2 _p3, Vector4 color) {
    Vector3 p0 = make_vector3(_p0.x, _p0.y, 0.0f);
    Vector3 p1 = make_vector3(_p1.x, _p1.y, 0.0f);
    Vector3 p2 = make_vector3(_p2.x, _p2.y, 0.0f);
    Vector3 p3 = make_vector3(_p3.x, _p3.y, 0.0f);

    immediate_quad(p0, p1, p2, p3, color);    
}

void set_vertex_format_to_mesh() {
    device_context->IASetInputLayout(mesh_input_layout);
}

void set_vertex_format_to_immediate() {
    device_context->IASetInputLayout(immediate_input_layout);    
}

Texture_Map *create_texture_rendertarget(int width, int height, bool multisample, int num_samples) {
    ID3D11Texture2D *texture = nullptr;
    ID3D11RenderTargetView *rtv = nullptr;
    ID3D11ShaderResourceView *srv = nullptr;

    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    if (multisample && num_samples > 1) {
        texture_desc.SampleDesc.Count = num_samples;
    } else {
        texture_desc.SampleDesc.Count = 1;
    }
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    device->CreateTexture2D(&texture_desc, nullptr, &texture);

    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = texture_desc.Format;
    if (multisample && num_samples > 1) {
        rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        rtv_desc.Texture2DMS = D3D11_TEX2DMS_RTV{0};
    } else {
        rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtv_desc.Texture2D = D3D11_TEX2D_RTV{0};
    }

    device->CreateRenderTargetView(texture, &rtv_desc, &rtv);

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = texture_desc.Format;
    if (multisample && num_samples > 1) {
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    } else {
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    }
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(texture, &srv_desc, &srv);

    Texture_Map *result = new Texture_Map();

    result->width = width;
    result->height = height;

    result->texture = (void *)texture;
    result->rtv = (void *)rtv;
    result->srv = (void *)srv;
    
    return result;
}

Texture_Map *create_texture_depthtarget(Texture_Map *render_target) {
    Texture_Map *result = new Texture_Map();

    result->width = render_target->width;
    result->height = render_target->height;

    D3D11_TEXTURE2D_DESC depth_buffer_desc = {};
    ((ID3D11Texture2D *)render_target->texture)->GetDesc(&depth_buffer_desc);

    depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    device->CreateTexture2D(&depth_buffer_desc, nullptr, (ID3D11Texture2D **)&result->texture);
    device->CreateDepthStencilView((ID3D11Texture2D *)result->texture, nullptr, (ID3D11DepthStencilView **)&result->rtv);
    
    return result;
}

void set_render_target(Texture_Map *map) {
    assert(map);
    
    device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&the_back_buffer->rtv, nullptr);
    
    int width = map->width;
    int height = map->height;

    D3D11_VIEWPORT vp = {};
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MaxDepth = 1.0f;
    device_context->RSSetViewports(1, &vp);

    current_render_target = map;

    render_target_width = map->width;
    render_target_height = map->height;
}

void set_depth_target(Texture_Map *map) {
    assert(map);
    
    device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&current_render_target->rtv, (ID3D11DepthStencilView *)map->rtv);

    current_depth_target = map;
}

void clear_render_target(f32 r, f32 g, f32 b, f32 a) {
    assert(current_render_target);
    
    float clear_color[4] = { r, g, b, a };
    device_context->ClearRenderTargetView((ID3D11RenderTargetView *)current_render_target->rtv, clear_color);

    if (current_depth_target) {
        device_context->ClearDepthStencilView((ID3D11DepthStencilView *)current_depth_target->rtv, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
}

void draw_mesh(Mesh *mesh, Vector3 position, Vector3 rotation, f32 scale) {
    Matrix4 m = matrix4_identity();

    m._11 = scale;
    m._22 = scale;
    m._33 = scale;

    m._14 = position.x;
    m._24 = position.y;
    m._34 = position.z;
    
    Matrix4 rot_x = make_x_rotation(rotation.x * (PI / 180.0f));
    Matrix4 rot_y = make_y_rotation(rotation.y * (PI / 180.0f));
    Matrix4 rot_z = make_z_rotation(rotation.z * (PI / 180.0f));
    Matrix4 r = rot_x * rot_y * rot_z;
    
    object_to_world_matrix = m * r;
    refresh_transform();
    
    UINT stride = sizeof(Mesh_Vertex);
    UINT offset = 0;
    device_context->IASetVertexBuffers(0, 1, (ID3D11Buffer **)&mesh->vbo, &stride, &offset);
    device_context->IASetIndexBuffer((ID3D11Buffer *)mesh->ibo, DXGI_FORMAT_R32_UINT, 0);

    set_vertex_format_to_mesh();
    
    device_context->DrawIndexed(mesh->vertex_count, 0, 0);
}

void refresh_transform() {
    object_to_proj_matrix = view_to_proj_matrix * (world_to_view_matrix * object_to_world_matrix);
    
    Matrix4 matrices[] = {
        transpose(view_to_proj_matrix),
        transpose(world_to_view_matrix),
        transpose(object_to_world_matrix),
        transpose(object_to_proj_matrix),
    };
    
    D3D11_MAPPED_SUBRESOURCE msr;
    device_context->Map(transform_cbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, matrices, sizeof(matrices));
    device_context->Unmap(transform_cbo, 0);

    if (current_shader) {
        device_context->VSSetConstantBuffers(0, 1, &transform_cbo);
    }
}

void rendering_2d_right_handed() {
    view_to_proj_matrix = matrix4_identity();

    f32 w = (f32)render_target_width;
    if (w < 1.0f) w = 1.0f;
    f32 h = (f32)render_target_height;
    if (h < 1.0f) h = 1.0f;
    
    view_to_proj_matrix._11 = 2.0f/w;
    view_to_proj_matrix._22 = 2.0f/h;
    view_to_proj_matrix._14 = -1.0f;
    view_to_proj_matrix._24 = -1.0f;
    
    world_to_view_matrix = matrix4_identity();
    object_to_world_matrix = matrix4_identity();

    refresh_transform();
}

void set_diffuse_texture(Texture_Map *map) {
    if (current_diffuse_map == map) return;

    immediate_flush();
    
    device_context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView **)&map->srv);
    current_diffuse_map = map;
}

Texture_Map *create_texture(Bitmap bitmap) {
    ID3D11Texture2D *texture = nullptr;
    ID3D11ShaderResourceView *srv = nullptr;

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    int num_channels = 0;
    if (bitmap.format == TEXTURE_FORMAT_RGBA8 || bitmap.format == TEXTURE_FORMAT_RGB8) {
        format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        num_channels = 4;
    }

    u8 *data = bitmap.data;
    bool should_free_data_on_exit = false;
    if (bitmap.format == TEXTURE_FORMAT_RGB8) {
        should_free_data_on_exit = true;

        data = new u8[bitmap.width * bitmap.height * 4];
        for (int y = 0; y < bitmap.height; y++) {
            for (int x = 0; x < bitmap.width; x++) {
                u8 *source = &bitmap.data[(y * bitmap.width + x) * 3];
                u8 *dest = &data[(y * bitmap.width + x) * 4];

                dest[0] = source[0];
                dest[1] = source[1];
                dest[2] = source[2];
                dest[3] = 255;
            }
        }
    }
    defer { if (should_free_data_on_exit) delete [] data; };

    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width = bitmap.width;
    texture_desc.Height = bitmap.height;
    texture_desc.MipLevels = 0;
    texture_desc.ArraySize = 1;
    texture_desc.Format = format;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texture_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    
    device->CreateTexture2D(&texture_desc, NULL, &texture);

    if (bitmap.data) {
        device_context->UpdateSubresource(texture, 0, NULL, data, bitmap.width * num_channels * sizeof(u8), 0);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = texture_desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(texture, &srv_desc, &srv);

    device_context->GenerateMips(srv);
    
    Texture_Map *result = new Texture_Map();

    result->width = bitmap.width;
    result->height = bitmap.height;
    result->format = bitmap.format;

    result->texture = (void *)texture;
    result->srv = (void *)srv;
    result->rtv = nullptr;
    
    return result;
}

void update_texture(Texture_Map *map, int x, int y, int width, int height, u8 *data) {
    ID3D11Texture2D *tex = (ID3D11Texture2D *)map->texture;
    int num_channels = 0;
    if (map->format == TEXTURE_FORMAT_RGBA8 || map->format == TEXTURE_FORMAT_RGB8) {
        num_channels = 4;
    }

    D3D11_BOX box;
    box.left = x;
    box.right = box.left + width;
    box.top = y;
    box.bottom = box.top + height;
    box.front = 0;
    box.back = 1;
    
    device_context->UpdateSubresource(tex, 0, &box, data, width * num_channels * sizeof(u8), 0);
}

#endif
