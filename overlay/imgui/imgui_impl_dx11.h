#pragma once

#include "imgui.h"

#ifndef IMGUI_DISABLE

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

IMGUI_IMPL_API bool     ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void     ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API bool     ImGui_ImplDX11_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplDX11_InvalidateDeviceObjects();

// Kept compatible with the supplied current backend header. The local ImGui
// core is 1.89.7 WIP and does not expose ImTextureData yet, so texture atlas
// uploads are handled internally by CreateDeviceObjects().
struct ImGui_ImplDX11_RenderState
{
    ID3D11Device*           Device;
    ID3D11DeviceContext*    DeviceContext;
    ID3D11Buffer*            VertexConstantBuffer;
};

#endif // #ifndef IMGUI_DISABLE
// cromekk
