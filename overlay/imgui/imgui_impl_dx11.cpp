// dear imgui: Renderer Backend for DirectX11
// This needs to be used along with a Platform Backend (e.g. Win32).
//
// This is the supplied DX11 renderer adapted to the project's local
// Dear ImGui 1.89.7 WIP core. The later ImTextureData/PlatformIO APIs are
// intentionally not referenced because they do not exist in that core.

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstddef>
#include <cstring>

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler.lib")
#endif

struct ImGui_ImplDX11_Data
{
    ID3D11Device*               pd3dDevice;
    ID3D11DeviceContext*        pd3dDeviceContext;
    ID3D11Buffer*               pVB;
    ID3D11Buffer*               pIB;
    ID3D11VertexShader*         pVertexShader;
    ID3D11InputLayout*          pInputLayout;
    ID3D11Buffer*               pVertexConstantBuffer;
    ID3D11PixelShader*          pPixelShader;
    ID3D11SamplerState*         pTexSamplerLinear;
    ID3D11RasterizerState*      pRasterizerState;
    ID3D11BlendState*            pBlendState;
    ID3D11DepthStencilState*     pDepthStencilState;
    ID3D11ShaderResourceView*    FontTexture;
    int                         VertexBufferSize;
    int                         IndexBufferSize;
    ImGui_ImplDX11_RenderState* RenderState;

    ImGui_ImplDX11_Data()
    {
        std::memset(this, 0, sizeof(*this));
        VertexBufferSize = 5000;
        IndexBufferSize = 10000;
    }
};

struct VERTEX_CONSTANT_BUFFER_DX11
{
    float mvp[4][4];
};

static ImGui_ImplDX11_Data* ImGui_ImplDX11_GetBackendData()
{
    return ImGui::GetCurrentContext()
        ? static_cast<ImGui_ImplDX11_Data*>(ImGui::GetIO().BackendRendererUserData)
        : nullptr;
}

static void ImGui_ImplDX11_SetupRenderState(ImDrawData* draw_data, ID3D11DeviceContext* device_context)
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();

    D3D11_VIEWPORT viewport{};
    viewport.Width = draw_data->DisplaySize.x * draw_data->FramebufferScale.x;
    viewport.Height = draw_data->DisplaySize.y * draw_data->FramebufferScale.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    device_context->RSSetViewports(1, &viewport);

    D3D11_MAPPED_SUBRESOURCE mapped_resource{};
    if (device_context->Map(bd->pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) == S_OK)
    {
        auto* constant_buffer = static_cast<VERTEX_CONSTANT_BUFFER_DX11*>(mapped_resource.pData);
        const float left = draw_data->DisplayPos.x;
        const float right = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        const float top = draw_data->DisplayPos.y;
        const float bottom = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const float projection[4][4] =
        {
            { 2.0f / (right - left), 0.0f, 0.0f, 0.0f },
            { 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.5f, 0.0f },
            { (right + left) / (left - right), (top + bottom) / (bottom - top), 0.5f, 1.0f },
        };
        std::memcpy(constant_buffer->mvp, projection, sizeof(projection));
        device_context->Unmap(bd->pVertexConstantBuffer, 0);
    }

    const UINT stride = sizeof(ImDrawVert);
    const UINT offset = 0;
    device_context->IASetInputLayout(bd->pInputLayout);
    device_context->IASetVertexBuffers(0, 1, &bd->pVB, &stride, &offset);
    device_context->IASetIndexBuffer(
        bd->pIB,
        sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
        0
    );
    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->VSSetShader(bd->pVertexShader, nullptr, 0);
    device_context->VSSetConstantBuffers(0, 1, &bd->pVertexConstantBuffer);
    device_context->PSSetShader(bd->pPixelShader, nullptr, 0);
    device_context->PSSetSamplers(0, 1, &bd->pTexSamplerLinear);
    device_context->GSSetShader(nullptr, nullptr, 0);
    device_context->HSSetShader(nullptr, nullptr, 0);
    device_context->DSSetShader(nullptr, nullptr, 0);
    device_context->CSSetShader(nullptr, nullptr, 0);

    const float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    device_context->OMSetBlendState(bd->pBlendState, blend_factor, 0xffffffff);
    device_context->OMSetDepthStencilState(bd->pDepthStencilState, 0);
    device_context->RSSetState(bd->pRasterizerState);
}

void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data)
{
    if (!draw_data || draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    if (!bd || !bd->pd3dDevice || !bd->pd3dDeviceContext || draw_data->TotalVtxCount <= 0)
        return;

    ID3D11DeviceContext* device_context = bd->pd3dDeviceContext;

    if (!bd->pVB || bd->VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (bd->pVB)
        {
            bd->pVB->Release();
            bd->pVB = nullptr;
        }

        bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = static_cast<UINT>(bd->VertexBufferSize * sizeof(ImDrawVert));
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(bd->pd3dDevice->CreateBuffer(&desc, nullptr, &bd->pVB)))
            return;
    }

    if (!bd->pIB || bd->IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (bd->pIB)
        {
            bd->pIB->Release();
            bd->pIB = nullptr;
        }

        bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = static_cast<UINT>(bd->IndexBufferSize * sizeof(ImDrawIdx));
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(bd->pd3dDevice->CreateBuffer(&desc, nullptr, &bd->pIB)))
            return;
    }

    D3D11_MAPPED_SUBRESOURCE vertex_resource{};
    D3D11_MAPPED_SUBRESOURCE index_resource{};
    if (device_context->Map(bd->pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertex_resource) != S_OK)
        return;
    if (device_context->Map(bd->pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &index_resource) != S_OK)
    {
        device_context->Unmap(bd->pVB, 0);
        return;
    }

    auto* vertex_destination = static_cast<ImDrawVert*>(vertex_resource.pData);
    auto* index_destination = static_cast<ImDrawIdx*>(index_resource.pData);
    for (int list_index = 0; list_index < draw_data->CmdListsCount; ++list_index)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[list_index];
        std::memcpy(
            vertex_destination,
            draw_list->VtxBuffer.Data,
            static_cast<size_t>(draw_list->VtxBuffer.Size) * sizeof(ImDrawVert)
        );
        std::memcpy(
            index_destination,
            draw_list->IdxBuffer.Data,
            static_cast<size_t>(draw_list->IdxBuffer.Size) * sizeof(ImDrawIdx)
        );
        vertex_destination += draw_list->VtxBuffer.Size;
        index_destination += draw_list->IdxBuffer.Size;
    }
    device_context->Unmap(bd->pVB, 0);
    device_context->Unmap(bd->pIB, 0);

    struct BackupState
    {
        UINT scissor_rects_count;
        UINT viewports_count;
        D3D11_RECT scissor_rects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ID3D11RasterizerState* rasterizer_state;
        ID3D11BlendState* blend_state;
        FLOAT blend_factor[4];
        UINT sample_mask;
        UINT stencil_ref;
        ID3D11DepthStencilState* depth_stencil_state;
        ID3D11ShaderResourceView* pixel_shader_resource;
        ID3D11SamplerState* pixel_sampler;
        ID3D11PixelShader* pixel_shader;
        ID3D11VertexShader* vertex_shader;
        ID3D11GeometryShader* geometry_shader;
        UINT pixel_instances_count;
        UINT vertex_instances_count;
        UINT geometry_instances_count;
        ID3D11ClassInstance* pixel_instances[256];
        ID3D11ClassInstance* vertex_instances[256];
        ID3D11ClassInstance* geometry_instances[256];
        D3D11_PRIMITIVE_TOPOLOGY primitive_topology;
        ID3D11Buffer* index_buffer;
        ID3D11Buffer* vertex_buffer;
        ID3D11Buffer* vertex_constant_buffer;
        UINT index_buffer_offset;
        UINT vertex_buffer_stride;
        UINT vertex_buffer_offset;
        DXGI_FORMAT index_buffer_format;
        ID3D11InputLayout* input_layout;
    };

    BackupState old{};
    old.scissor_rects_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    old.viewports_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    device_context->RSGetScissorRects(&old.scissor_rects_count, old.scissor_rects);
    device_context->RSGetViewports(&old.viewports_count, old.viewports);
    device_context->RSGetState(&old.rasterizer_state);
    device_context->OMGetBlendState(&old.blend_state, old.blend_factor, &old.sample_mask);
    device_context->OMGetDepthStencilState(&old.depth_stencil_state, &old.stencil_ref);
    device_context->PSGetShaderResources(0, 1, &old.pixel_shader_resource);
    device_context->PSGetSamplers(0, 1, &old.pixel_sampler);
    old.pixel_instances_count = old.vertex_instances_count = old.geometry_instances_count = 256;
    device_context->PSGetShader(&old.pixel_shader, old.pixel_instances, &old.pixel_instances_count);
    device_context->VSGetShader(&old.vertex_shader, old.vertex_instances, &old.vertex_instances_count);
    device_context->VSGetConstantBuffers(0, 1, &old.vertex_constant_buffer);
    device_context->GSGetShader(&old.geometry_shader, old.geometry_instances, &old.geometry_instances_count);
    device_context->IAGetPrimitiveTopology(&old.primitive_topology);
    device_context->IAGetIndexBuffer(&old.index_buffer, &old.index_buffer_format, &old.index_buffer_offset);
    device_context->IAGetVertexBuffers(
        0,
        1,
        &old.vertex_buffer,
        &old.vertex_buffer_stride,
        &old.vertex_buffer_offset
    );
    device_context->IAGetInputLayout(&old.input_layout);

    ImGui_ImplDX11_SetupRenderState(draw_data, device_context);

    ImGui_ImplDX11_RenderState render_state{};
    render_state.Device = bd->pd3dDevice;
    render_state.DeviceContext = bd->pd3dDeviceContext;
    render_state.VertexConstantBuffer = bd->pVertexConstantBuffer;
    bd->RenderState = &render_state;

    int global_index_offset = 0;
    int global_vertex_offset = 0;
    const ImVec2 clip_offset = draw_data->DisplayPos;
    const ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int list_index = 0; list_index < draw_data->CmdListsCount; ++list_index)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[list_index];
        for (int command_index = 0; command_index < draw_list->CmdBuffer.Size; ++command_index)
        {
            const ImDrawCmd* command = &draw_list->CmdBuffer[command_index];
            if (command->UserCallback)
            {
                if (command->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplDX11_SetupRenderState(draw_data, device_context);
                else
                    command->UserCallback(draw_list, command);
                continue;
            }

            const ImVec2 clip_min(
                (command->ClipRect.x - clip_offset.x) * clip_scale.x,
                (command->ClipRect.y - clip_offset.y) * clip_scale.y
            );
            const ImVec2 clip_max(
                (command->ClipRect.z - clip_offset.x) * clip_scale.x,
                (command->ClipRect.w - clip_offset.y) * clip_scale.y
            );
            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                continue;

            const D3D11_RECT scissor_rect{
                static_cast<LONG>(clip_min.x),
                static_cast<LONG>(clip_min.y),
                static_cast<LONG>(clip_max.x),
                static_cast<LONG>(clip_max.y)
            };
            device_context->RSSetScissorRects(1, &scissor_rect);

            ID3D11ShaderResourceView* texture =
                reinterpret_cast<ID3D11ShaderResourceView*>(command->GetTexID());
            device_context->PSSetShaderResources(0, 1, &texture);
            device_context->DrawIndexed(
                command->ElemCount,
                command->IdxOffset + global_index_offset,
                static_cast<INT>(command->VtxOffset + global_vertex_offset)
            );
        }

        global_index_offset += draw_list->IdxBuffer.Size;
        global_vertex_offset += draw_list->VtxBuffer.Size;
    }
    bd->RenderState = nullptr;

    device_context->RSSetScissorRects(old.scissor_rects_count, old.scissor_rects);
    device_context->RSSetViewports(old.viewports_count, old.viewports);
    device_context->RSSetState(old.rasterizer_state);
    device_context->OMSetBlendState(old.blend_state, old.blend_factor, old.sample_mask);
    device_context->OMSetDepthStencilState(old.depth_stencil_state, old.stencil_ref);
    device_context->PSSetShaderResources(0, 1, &old.pixel_shader_resource);
    device_context->PSSetSamplers(0, 1, &old.pixel_sampler);
    device_context->PSSetShader(old.pixel_shader, old.pixel_instances, old.pixel_instances_count);
    device_context->VSSetShader(old.vertex_shader, old.vertex_instances, old.vertex_instances_count);
    device_context->VSSetConstantBuffers(0, 1, &old.vertex_constant_buffer);
    device_context->GSSetShader(old.geometry_shader, old.geometry_instances, old.geometry_instances_count);
    device_context->IASetPrimitiveTopology(old.primitive_topology);
    device_context->IASetIndexBuffer(old.index_buffer, old.index_buffer_format, old.index_buffer_offset);
    device_context->IASetVertexBuffers(
        0,
        1,
        &old.vertex_buffer,
        &old.vertex_buffer_stride,
        &old.vertex_buffer_offset
    );
    device_context->IASetInputLayout(old.input_layout);

    if (old.rasterizer_state) old.rasterizer_state->Release();
    if (old.blend_state) old.blend_state->Release();
    if (old.depth_stencil_state) old.depth_stencil_state->Release();
    if (old.pixel_shader_resource) old.pixel_shader_resource->Release();
    if (old.pixel_sampler) old.pixel_sampler->Release();
    if (old.pixel_shader) old.pixel_shader->Release();
    if (old.vertex_shader) old.vertex_shader->Release();
    if (old.vertex_constant_buffer) old.vertex_constant_buffer->Release();
    if (old.geometry_shader) old.geometry_shader->Release();
    if (old.index_buffer) old.index_buffer->Release();
    if (old.vertex_buffer) old.vertex_buffer->Release();
    if (old.input_layout) old.input_layout->Release();
    for (UINT i = 0; i < old.pixel_instances_count; ++i)
        if (old.pixel_instances[i]) old.pixel_instances[i]->Release();
    for (UINT i = 0; i < old.vertex_instances_count; ++i)
        if (old.vertex_instances[i]) old.vertex_instances[i]->Release();
    for (UINT i = 0; i < old.geometry_instances_count; ++i)
        if (old.geometry_instances[i]) old.geometry_instances[i]->Release();
}

static bool ImGui_ImplDX11_CreateFontsTexture()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    if (!bd || !bd->pd3dDevice)
        return false;

    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    D3D11_TEXTURE2D_DESC texture_desc{};
    texture_desc.Width = static_cast<UINT>(width);
    texture_desc.Height = static_cast<UINT>(height);
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subresource{};
    subresource.pSysMem = pixels;
    subresource.SysMemPitch = static_cast<UINT>(width * 4);

    ID3D11Texture2D* texture = nullptr;
    if (FAILED(bd->pd3dDevice->CreateTexture2D(&texture_desc, &subresource, &texture)))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
    view_desc.Format = texture_desc.Format;
    view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    view_desc.Texture2D.MipLevels = 1;

    const HRESULT result = bd->pd3dDevice->CreateShaderResourceView(
        texture,
        &view_desc,
        &bd->FontTexture
    );
    texture->Release();
    if (FAILED(result))
        return false;

    io.Fonts->TexID = static_cast<ImTextureID>(bd->FontTexture);
    return true;
}

bool ImGui_ImplDX11_CreateDeviceObjects()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    if (!bd || !bd->pd3dDevice)
        return false;

    ImGui_ImplDX11_InvalidateDeviceObjects();

    static const char* vertex_shader =
        "cbuffer vertexBuffer : register(b0) {"
        "  float4x4 ProjectionMatrix;"
        "};"
        "struct VS_INPUT {"
        "  float2 pos : POSITION;"
        "  float4 col : COLOR0;"
        "  float2 uv : TEXCOORD0;"
        "};"
        "struct PS_INPUT {"
        "  float4 pos : SV_POSITION;"
        "  float4 col : COLOR0;"
        "  float2 uv : TEXCOORD0;"
        "};"
        "PS_INPUT main(VS_INPUT input) {"
        "  PS_INPUT output;"
        "  output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));"
        "  output.col = input.col;"
        "  output.uv = input.uv;"
        "  return output;"
        "}";

    ID3DBlob* vertex_blob = nullptr;
    if (FAILED(D3DCompile(
        vertex_shader,
        std::strlen(vertex_shader),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_4_0",
        0,
        0,
        &vertex_blob,
        nullptr
    )))
        return false;

    if (FAILED(bd->pd3dDevice->CreateVertexShader(
        vertex_blob->GetBufferPointer(),
        vertex_blob->GetBufferSize(),
        nullptr,
        &bd->pVertexShader
    )))
    {
        vertex_blob->Release();
        return false;
    }

    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(offsetof(ImDrawVert, pos)), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(offsetof(ImDrawVert, uv)), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, static_cast<UINT>(offsetof(ImDrawVert, col)), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    if (FAILED(bd->pd3dDevice->CreateInputLayout(
        layout,
        static_cast<UINT>(IM_ARRAYSIZE(layout)),
        vertex_blob->GetBufferPointer(),
        vertex_blob->GetBufferSize(),
        &bd->pInputLayout
    )))
    {
        vertex_blob->Release();
        return false;
    }
    vertex_blob->Release();

    D3D11_BUFFER_DESC constant_buffer_desc{};
    constant_buffer_desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER_DX11);
    constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(bd->pd3dDevice->CreateBuffer(
        &constant_buffer_desc,
        nullptr,
        &bd->pVertexConstantBuffer
    )))
        return false;

    static const char* pixel_shader =
        "struct PS_INPUT {"
        "  float4 pos : SV_POSITION;"
        "  float4 col : COLOR0;"
        "  float2 uv : TEXCOORD0;"
        "};"
        "sampler sampler0;"
        "Texture2D texture0;"
        "float4 main(PS_INPUT input) : SV_Target {"
        "  return input.col * texture0.Sample(sampler0, input.uv);"
        "}";

    ID3DBlob* pixel_blob = nullptr;
    if (FAILED(D3DCompile(
        pixel_shader,
        std::strlen(pixel_shader),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_4_0",
        0,
        0,
        &pixel_blob,
        nullptr
    )))
        return false;

    const HRESULT pixel_shader_result = bd->pd3dDevice->CreatePixelShader(
        pixel_blob->GetBufferPointer(),
        pixel_blob->GetBufferSize(),
        nullptr,
        &bd->pPixelShader
    );
    pixel_blob->Release();
    if (FAILED(pixel_shader_result))
        return false;

    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(bd->pd3dDevice->CreateBlendState(&blend_desc, &bd->pBlendState)))
        return false;

    D3D11_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_NONE;
    rasterizer_desc.ScissorEnable = TRUE;
    rasterizer_desc.DepthClipEnable = TRUE;
    if (FAILED(bd->pd3dDevice->CreateRasterizerState(&rasterizer_desc, &bd->pRasterizerState)))
        return false;

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
    depth_stencil_desc.DepthEnable = FALSE;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    if (FAILED(bd->pd3dDevice->CreateDepthStencilState(
        &depth_stencil_desc,
        &bd->pDepthStencilState
    )))
        return false;

    D3D11_SAMPLER_DESC sampler_desc{};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.MinLOD = 0.0f;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(bd->pd3dDevice->CreateSamplerState(&sampler_desc, &bd->pTexSamplerLinear)))
        return false;

    return ImGui_ImplDX11_CreateFontsTexture();
}

void ImGui_ImplDX11_InvalidateDeviceObjects()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    if (!bd)
        return;

    if (ImGui::GetCurrentContext() && ImGui::GetIO().Fonts)
        ImGui::GetIO().Fonts->TexID = nullptr;

    if (bd->FontTexture)            { bd->FontTexture->Release(); bd->FontTexture = nullptr; }
    if (bd->pTexSamplerLinear)      { bd->pTexSamplerLinear->Release(); bd->pTexSamplerLinear = nullptr; }
    if (bd->pIB)                    { bd->pIB->Release(); bd->pIB = nullptr; }
    if (bd->pVB)                    { bd->pVB->Release(); bd->pVB = nullptr; }
    if (bd->pBlendState)            { bd->pBlendState->Release(); bd->pBlendState = nullptr; }
    if (bd->pDepthStencilState)     { bd->pDepthStencilState->Release(); bd->pDepthStencilState = nullptr; }
    if (bd->pRasterizerState)       { bd->pRasterizerState->Release(); bd->pRasterizerState = nullptr; }
    if (bd->pPixelShader)           { bd->pPixelShader->Release(); bd->pPixelShader = nullptr; }
    if (bd->pVertexConstantBuffer)  { bd->pVertexConstantBuffer->Release(); bd->pVertexConstantBuffer = nullptr; }
    if (bd->pInputLayout)           { bd->pInputLayout->Release(); bd->pInputLayout = nullptr; }
    if (bd->pVertexShader)          { bd->pVertexShader->Release(); bd->pVertexShader = nullptr; }
}

bool ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
    if (!device || !device_context)
        return false;

    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    auto* bd = IM_NEW(ImGui_ImplDX11_Data)();
    bd->pd3dDevice = device;
    bd->pd3dDeviceContext = device_context;
    bd->pd3dDevice->AddRef();
    bd->pd3dDeviceContext->AddRef();

    io.BackendRendererUserData = bd;
    io.BackendRendererName = "imgui_impl_dx11";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    return true;
}

void ImGui_ImplDX11_Shutdown()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    if (!bd)
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplDX11_InvalidateDeviceObjects();
    if (bd->pd3dDevice)         bd->pd3dDevice->Release();
    if (bd->pd3dDeviceContext)  bd->pd3dDeviceContext->Release();

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);
}

void ImGui_ImplDX11_NewFrame()
{
    ImGui_ImplDX11_Data* bd = ImGui_ImplDX11_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplDX11_Init()?");
    if (!bd)
        return;

    if (!bd->pVertexShader && !ImGui_ImplDX11_CreateDeviceObjects())
        IM_ASSERT(0 && "ImGui_ImplDX11_CreateDeviceObjects() failed!");
}

#endif // #ifndef IMGUI_DISABLE
// cromekk
