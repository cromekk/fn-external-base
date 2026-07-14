#pragma once

#include <Windows.h>
#include <cstdint>
#include <utility>

#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

void gameCache( );
void renderVisuals( );
void renderMenu( );

inline ID3D11Device* p_device = nullptr;
inline ID3D11DeviceContext* p_device_context = nullptr;
inline IDXGISwapChain* p_swap_chain = nullptr;
inline ID3D11RenderTargetView* p_render_target = nullptr;
inline MSG messager{};
inline HWND my_wnd = nullptr;
inline HWND game_wnd = nullptr;
inline MARGINS m_window_margin{ -1 };

inline void shutdown_overlay( );

namespace overlay_detail {
    inline void apply_window_surface( HWND hwnd )
    {
        if ( !hwnd )
            return;

        m_window_margin = { -1 };
        DwmExtendFrameIntoClientArea( hwnd,&m_window_margin );

        const LONG_PTR extended_style = GetWindowLongPtrA( hwnd,GWL_EXSTYLE );
        SetWindowLongPtrA( hwnd,GWL_EXSTYLE,extended_style | WS_EX_LAYERED | WS_EX_TOOLWINDOW );
        SetLayeredWindowAttributes( hwnd,RGB( 0,0,0 ),255,LWA_ALPHA );

        UpdateWindow( hwnd );
        ShowWindow( hwnd,SW_SHOW );
    }

    inline void cleanup_render_target( )
    {
        if ( p_render_target )
        {
            p_render_target->Release( );
            p_render_target = nullptr;
        }
    }

    inline bool create_render_target( )
    {
        if ( !p_swap_chain || !p_device )
            return false;

        ID3D11Texture2D* back_buffer = nullptr;
        const HRESULT buffer_result = p_swap_chain->GetBuffer(
            0,
            IID_PPV_ARGS( &back_buffer )
        );
        if ( FAILED( buffer_result ) )
            return false;

        const HRESULT view_result = p_device->CreateRenderTargetView(
            back_buffer,
            nullptr,
            &p_render_target
        );
        back_buffer->Release( );
        return SUCCEEDED( view_result );
    }

    inline bool reset_device( int width,int height )
    {
        if ( !p_swap_chain || width <= 0 || height <= 0 )
            return false;

        if ( settings.ScreenWidth == width && settings.ScreenHeight == height && p_render_target )
            return true;

        ImGui_ImplDX11_InvalidateDeviceObjects( );
        cleanup_render_target( );

        const HRESULT resize_result = p_swap_chain->ResizeBuffers(
            0,
            static_cast< UINT >( width ),
            static_cast< UINT >( height ),
            DXGI_FORMAT_UNKNOWN,
            0
        );
        if ( FAILED( resize_result ) )
            return false;

        if ( !create_render_target( ) )
            return false;

        return ImGui_ImplDX11_CreateDeviceObjects( );
    }

    inline void update_target_rect( )
    {
        if ( !my_wnd || !game_wnd )
            return;

        RECT client_rect{};
        POINT client_origin{};
        if ( !GetClientRect( game_wnd,&client_rect ) || !ClientToScreen( game_wnd,&client_origin ) )
            return;

        const int width = client_rect.right - client_rect.left;
        const int height = client_rect.bottom - client_rect.top;
        if ( width <= 0 || height <= 0 )
            return;

        const bool size_changed = settings.ScreenWidth != width || settings.ScreenHeight != height;
        if ( size_changed )
            reset_device( width,height );

        settings.ScreenWidth = width;
        settings.ScreenHeight = height;
        settings.ScreenCenterX = width / 2;
        settings.ScreenCenterY = height / 2;

        SetWindowPos(
            my_wnd,
            HWND_TOP,
            client_origin.x,
            client_origin.y,
            width,
            height,
            SWP_NOACTIVATE | SWP_SHOWWINDOW
        );
    }

    inline void update_mouse_input( )
    {
        if ( !my_wnd )
            return;

        ImGuiIO& io = ImGui::GetIO( );
        POINT cursor{};
        if ( GetCursorPos( &cursor ) )
        {
            ScreenToClient( my_wnd,&cursor );
            io.MousePos = ImVec2(
                static_cast< float >( cursor.x ),
                static_cast< float >( cursor.y )
            );
        }

        static bool previous_left_down = false;
        const bool left_down = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
        io.MouseDown[ 0 ] = left_down;
        io.MouseClicked[ 0 ] = left_down && !previous_left_down;
        if ( io.MouseClicked[ 0 ] )
            io.MouseClickedPos[ 0 ] = io.MousePos;
        previous_left_down = left_down;
    }

    inline void render_frame( )
    {
        if ( !p_device_context || !p_swap_chain || !p_render_target )
            return;

        const float clear_color[ 4 ] = { 0.0f,0.0f,0.0f,0.0f };
        p_device_context->OMSetRenderTargets( 1,&p_render_target,nullptr );
        p_device_context->ClearRenderTargetView( p_render_target,clear_color );

        ImGui::Render( );
        ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

        const HRESULT present_result = p_swap_chain->Present( 1,0 );
        if ( present_result == DXGI_ERROR_DEVICE_REMOVED ||
            present_result == DXGI_ERROR_DEVICE_RESET )
        {
            ImGui_ImplDX11_InvalidateDeviceObjects( );
            cleanup_render_target( );
        }
    }
}

inline void hijack_window( )
{
    if ( my_wnd && IsWindow( my_wnd ) )
        return;

    my_wnd = FindWindowA( "Chrome_WidgetWin_1","Discord Overlay" );
    if ( !my_wnd )
        return;

    overlay_detail::apply_window_surface( my_wnd );
}

inline HRESULT directx_init( )
{
    if ( !my_wnd || !IsWindow( my_wnd ) )
        return E_HANDLE;

    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = my_wnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL feature_levels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL feature_level{};

    const HRESULT device_result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        feature_levels,
        static_cast< UINT >( sizeof( feature_levels ) / sizeof( feature_levels[ 0 ] ) ),
        D3D11_SDK_VERSION,
        &swap_chain_desc,
        &p_swap_chain,
        &p_device,
        &feature_level,
        &p_device_context
    );
    if ( FAILED( device_result ) )
        return device_result;

    if ( !overlay_detail::create_render_target( ) )
    {
        shutdown_overlay( );
        return E_FAIL;
    }

    if ( !ImGui::GetCurrentContext( ) )
        ImGui::CreateContext( );

    if ( !ImGui_ImplWin32_Init( my_wnd ) )
    {
        shutdown_overlay( );
        ImGui::DestroyContext( );
        return E_FAIL;
    }

    if ( !ImGui_ImplDX11_Init( p_device,p_device_context ) )
    {
        ImGui_ImplWin32_Shutdown( );
        shutdown_overlay( );
        ImGui::DestroyContext( );
        return E_FAIL;
    }

    ImGuiIO& io = ImGui::GetIO( );
    io.IniFilename = nullptr;

    ImGuiStyle& style = ImGui::GetStyle( );
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowTitleAlign = ImVec2( 0.5f,0.5f );
    style.Colors[ ImGuiCol_BorderShadow ] = ImColor( 0,0,0,0 );
    style.Colors[ ImGuiCol_Border ] = ImColor( 8,8,8,250 );
    style.Colors[ ImGuiCol_TitleBg ] = ImColor( 38,38,38,250 );
    style.Colors[ ImGuiCol_TitleBgActive ] = ImColor( 38,38,38,250 );
    style.Colors[ ImGuiCol_WindowBg ] = ImColor( 38,38,38,250 );
    style.Colors[ ImGuiCol_FrameBg ] = ImColor( 38,38,38,245 );
    style.Colors[ ImGuiCol_FrameBgActive ] = ImColor( 38,38,38,245 );
    style.Colors[ ImGuiCol_FrameBgHovered ] = ImColor( 38,38,38,245 );
    style.Colors[ ImGuiCol_Button ] = ImColor( 38,38,38,245 );
    style.Colors[ ImGuiCol_ButtonActive ] = ImColor( 38,38,38,245 );
    style.Colors[ ImGuiCol_ButtonHovered ] = ImColor( 38,38,38,245 );
    style.Colors[ ImGuiCol_SliderGrab ] = ImColor( 68,68,68,250 );
    style.Colors[ ImGuiCol_SliderGrabActive ] = ImColor( 68,68,68,250 );
    style.Colors[ ImGuiCol_CheckMark ] = ImColor( 68,68,68,250 );
    style.Colors[ ImGuiCol_Header ] = ImColor( 68,68,68,250 );
    style.Colors[ ImGuiCol_HeaderActive ] = ImColor( 0,0,0,0 );
    style.Colors[ ImGuiCol_HeaderHovered ] = ImColor( 68,68,68,250 );
    style.Colors[ ImGuiCol_PopupBg ] = ImColor( 38,38,38,250 );

    return S_OK;
}

inline HWND get_process_wnd( std::uint32_t pid )
{
    std::pair<HWND,std::uint32_t> params{ nullptr,pid };
    const BOOL enum_result = EnumWindows(
        [] ( HWND hwnd,LPARAM parameter ) -> BOOL
        {
            auto* window_params = reinterpret_cast< std::pair<HWND,std::uint32_t>* >( parameter );
            std::uint32_t process_id = 0;
            if ( GetWindowThreadProcessId( hwnd,reinterpret_cast< LPDWORD >( &process_id ) ) &&
                process_id == window_params->second )
            {
                window_params->first = hwnd;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast< LPARAM >( &params )
    );

    return enum_result == FALSE ? params.first : nullptr;
}

inline void shutdown_overlay( )
{
    if ( ImGui::GetCurrentContext( ) )
    {
        ImGuiIO& io = ImGui::GetIO( );
        if ( io.BackendRendererUserData )
            ImGui_ImplDX11_Shutdown( );
        if ( io.BackendPlatformUserData )
            ImGui_ImplWin32_Shutdown( );
        ImGui::DestroyContext( );
    }

    overlay_detail::cleanup_render_target( );
    if ( p_swap_chain )
    {
        p_swap_chain->Release( );
        p_swap_chain = nullptr;
    }
    if ( p_device_context )
    {
        p_device_context->Release( );
        p_device_context = nullptr;
    }
    if ( p_device )
    {
        p_device->Release( );
        p_device = nullptr;
    }

    my_wnd = nullptr;
}

inline WPARAM render_loop( )
{
    if ( !my_wnd || !p_device || !p_device_context || !p_swap_chain )
        return 0;

    ZeroMemory( &messager,sizeof( messager ) );
    while ( messager.message != WM_QUIT )
    {
        while ( PeekMessage( &messager,nullptr,0,0,PM_REMOVE ) )
        {
            TranslateMessage( &messager );
            DispatchMessage( &messager );
        }

        if ( !game_wnd || !IsWindow( game_wnd ) )
        {
            game_wnd = get_process_wnd( mem::ProcessID );
            Sleep( 25 );
            continue;
        }

        const HWND foreground_window = GetForegroundWindow( );
        if ( foreground_window != game_wnd && foreground_window != my_wnd )
        {
            Sleep( 10 );
            continue;
        }

        overlay_detail::update_target_rect( );

        ImGuiIO& io = ImGui::GetIO( );
        io.DeltaTime = 1.0f / 60.0f;
        overlay_detail::update_mouse_input( );

        ImGui_ImplDX11_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );

        gameCache( );
        renderVisuals( );
        renderMenu( );

        ImGui::EndFrame( );
        overlay_detail::render_frame( );

        const HWND target_window = GetWindow( game_wnd,GW_HWNDPREV );
        SetWindowPos( my_wnd,target_window,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
    }

    shutdown_overlay( );
    return messager.wParam;
}
