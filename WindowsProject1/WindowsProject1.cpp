// WindowsProject1.cpp : Defines the entry point for the application.
//

//d3d11 hello world source : https://antongerdelan.net/opengl/d3d11.html

#define WIN32_LEAN_AND_MEAN
#include "framework.h"
#include "WindowsProject1.h"
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler
#include <assert.h>

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

#define MAX_LOADSTRING 100
#define IDT_TIMER1 90

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
ID3D11Device* device_ptr = NULL;
ID3D11DeviceContext* device_context_ptr = NULL;
IDXGISwapChain* swap_chain_ptr = NULL;
ID3D11RenderTargetView* render_target_view_ptr = NULL;
HWND hWnd;
unsigned int color_value = 0;
short int color_change_direction = 1;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    SetTimer(hWnd,             // handle to main window 
	    IDT_TIMER1,            // timer identifier
        1000,                 // 10-second interval 
        (TIMERPROC)NULL);     // no timer callback 
    

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_descr.SampleDesc.Count = 1;
    swap_chain_descr.SampleDesc.Quality = 0;
    swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descr.BufferCount = 1;
    swap_chain_descr.OutputWindow = hWnd;
    swap_chain_descr.Windowed = true;

    /*We are going to display to a window, not in full screen, 
    so we don't need to set a width or height value, and we set Windowed to true.
    As you can see in the documentation, the Numerator, and Denominator can be 
    set to synchronise output frame rate to your monitor. Here we say "just draw as fast as possible". 
    DXGI_FORMAT_B8G8R8A8_UNORM is a reasonable default colour output, but does not have gamma correction.
    We could use DXGI_FORMAT_B8G8R8A8_UNORM_SRGB for that. We are not enabling multisampling anti-aliasing yet,
    so SampleDesc parameters are set to defaults. 
    BufferCount is the count of back buffers to add to the swap chain. 
    So, in windowed mode, for a typical double-buffering set-up with 1 front buffer and 1 back buffer,
    we can set this to 1. We want to tie our output buffers to the window,
    so use your Win32 handle, returned by CreateWindow(), for OutputWindow. 
    Next we can give our struct to the function.*/


    D3D_FEATURE_LEVEL feature_level;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swap_chain_descr,
        &swap_chain_ptr,
        &device_ptr,
        &feature_level,
        &device_context_ptr);
    assert(S_OK == hr && swap_chain_ptr && device_ptr && device_context_ptr);

    /*
    Here I have added extra debug output to the function flags when the program is built in debug mode.
    If all went well, you should be able to now compileand run it, without an assertion triggering.
    Otherwise check the parameter values carefully.The output images from Direct3D are called Render Targets.
    We can get a view pointer to ours now, by fetching it from our swap chain.
    */
    ID3D11Texture2D* framebuffer;
    hr = swap_chain_ptr->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        (void**)&framebuffer);
    assert(SUCCEEDED(hr));

    hr = device_ptr->CreateRenderTargetView(
        framebuffer, 0, &render_target_view_ptr);
    assert(SUCCEEDED(hr));
    framebuffer->Release();



    //Create and Compile Shaders
        //Create a new text file called shaders.hlsl.This will contain both our vertexand pixel shader for drawing our triangle.

    flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
    ID3DBlob* vs_blob_ptr = NULL, * ps_blob_ptr = NULL, * error_blob = NULL;

    // COMPILE VERTEX SHADER
    hr = D3DCompileFromFile(
        L"shaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        flags,
        0,
        &vs_blob_ptr,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (vs_blob_ptr) { vs_blob_ptr->Release(); }
        assert(false);
    }

    // COMPILE PIXEL SHADER
    hr = D3DCompileFromFile(
        L"shaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        flags,
        0,
        &ps_blob_ptr,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (ps_blob_ptr) { ps_blob_ptr->Release(); }
        assert(false);
    }


    ID3D11VertexShader* vertex_shader_ptr = NULL;
    ID3D11PixelShader* pixel_shader_ptr = NULL;

    hr = device_ptr->CreateVertexShader(
        vs_blob_ptr->GetBufferPointer(),
        vs_blob_ptr->GetBufferSize(),
        NULL,
        &vertex_shader_ptr);
    assert(SUCCEEDED(hr));

    hr = device_ptr->CreatePixelShader(
        ps_blob_ptr->GetBufferPointer(),
        ps_blob_ptr->GetBufferSize(),
        NULL,
        &pixel_shader_ptr);
    assert(SUCCEEDED(hr));


    ID3D11InputLayout* input_layout_ptr = NULL;
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      /*
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      */
    };
    hr = device_ptr->CreateInputLayout(
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        vs_blob_ptr->GetBufferPointer(),
        vs_blob_ptr->GetBufferSize(),
        &input_layout_ptr);
    assert(SUCCEEDED(hr));

   /* 
   We only have 1 input variable to our vertex shader - the XYZ position.Later you might want to add colours, normals, 
   and texture coordinates.I have left in additional entries, commented out, to show how your might do that.Each entry 
   has a string for its semantic name.This can be anything, but must match the semantic given in the shader(scroll up to 
   the vertex shader input struct, and check that it is called "POS").We also give the typeand number of components in each 
   vertex element - here we have an XYZ position - 3 components.Each element is a 32 - bit float.This corresponds to DXGI_FORMAT_R32G32B32_FLOAT,
   and will appear as a float3 in our shader.A float4 may use DXGI_FORMAT_R32G32B32A32_FLOAT.Note that any following inputs in the vertex buffer
   must say how they are laid out.If we add colours, we may have a typical interleaved layout : XYZRGBXYZRGBXYZRGB.In this case we would 
   specify that the colour element starts on the fourth float.We can put this value, but the element structs also have a handy macro
   D3D11_APPEND_ALIGNED_ELEMENT that means "starts after the previous element" for an interleaved layout.
   */
    typedef struct Vertex {
        float x;
        float y;
        float z;
        float r;
        float g;
        float b;
    } vertex_t;

    vertex_t vertex_data_array[] = {
        {0.0f,  0.5f,  0.0f,1.0,0.0,0.0}, // point at top
        {0.5f, -0.5f,  0.0f,0.0,1.0,0.0}, // point at bottom-right
        {-0.5f, -0.5f, 0.0f,0.0,0.0,1.0}, // point at bottom-left
    };
    vertex_t vertex_data_array2[] = {
        {0.5f,  0.5f,  0.0f,1.0,0.0,0.0}, // point at top
        {1.0f, -0.5f,  0.0f,0.0,1.0,0.0}, // point at bottom-right
        {0.0f, -0.5f, 0.0f,0.0,0.0,1.0}, // point at bottom-left
    };
    UINT vertex_stride = 6 * sizeof(float);
    UINT vertex_offset = 0;
    UINT vertex_count = sizeof(vertex_data_array)/(vertex_stride);
    ID3D11Buffer* vertex_buffer_ptr = NULL;
    { /*** load mesh data into vertex buffer **/
        D3D11_BUFFER_DESC vertex_buff_descr = {};
        vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
        vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sr_data = { 0 };
        sr_data.pSysMem = vertex_data_array;
        HRESULT hr = device_ptr->CreateBuffer(
            &vertex_buff_descr,
            &sr_data,
            &vertex_buffer_ptr);
        assert(SUCCEEDED(hr));
    }

    ID3D11Buffer* vertex_buffer_ptr2 = NULL;
    { /*** load mesh data into vertex buffer **/
        D3D11_BUFFER_DESC vertex_buff_descr = {};
        vertex_buff_descr.ByteWidth = sizeof(vertex_data_array2);
        vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sr_data = { 0 };
        sr_data.pSysMem = vertex_data_array2;
        HRESULT hr = device_ptr->CreateBuffer(
            &vertex_buff_descr,
            &sr_data,
            &vertex_buffer_ptr2);
        assert(SUCCEEDED(hr));
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));
   
    MSG msg;
    bool should_close = false;

    // Main message loop:
    while (!should_close) {
        /** handle user input and other window events **/
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        if (msg.message == WM_QUIT)
            break;
        if (msg.message == WM_TIMER) {
            if (color_value == 255) {
                color_change_direction = -1;
            }
            if (color_value == 0) {
                color_change_direction = 1;
            }

			color_value = (color_value + color_change_direction) % 256;
        }
        /* clear the back buffer to cornflower blue for the new frame */
        float background_colour[4] = {
          color_value / 255.0f, color_value / 255.0f, color_value / 255.0f, 1.0f };
        
        //Pulsating backround color
		
       // const float c = sin(timer.Peak()) / 2.0f + 0.5f;

        device_context_ptr->ClearRenderTargetView(
            render_target_view_ptr, background_colour);
        
        /*
        We use our render target view pointer to access the back buffer and clear it to an RGBA 
        colour of our choice (values between 0 and 1). It's a good idea to use grey or a neutral colour,
        so if your shape is black you still see something. If you add a depth buffer later you will also
        clear that here with ClearDepthStencilView(). 
        We need to set the valid drawing area - the viewport, within our window, or nothing will draw.*/

        RECT winRect;
        GetClientRect(hWnd, &winRect);
        D3D11_VIEWPORT viewport = {
          0.0f,
          0.0f,
          (FLOAT)(winRect.right - winRect.left),
          (FLOAT)(winRect.bottom - winRect.top),
          0.0f,
          1.0f };
        device_context_ptr->RSSetViewports(1, &viewport);

        /*
        We fetch the drawing surface rectangle from our win32 window handle, 
        and use the width and height from this as our viewport dimensions in the struct D3D11_VIEWPORT,
        which we give to the rasteriser state function RSSetViewports().
        */

        /*
        * Set the Output Merger
            We need to tell the Output Merger to use our render target. 
            If you add depth testing later you would do that here too with OMSetDepthStencilState().
        */
        device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);

        /*
        Before drawing, we can update the Input Assembler with the vertex buffer to draw, 
        and the memory layout, so it knows how to feed vertex data from the vertex buffer 
        to vertex shaders.
        */
        device_context_ptr->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context_ptr->IASetInputLayout(input_layout_ptr);
        if(color_change_direction==1)
            device_context_ptr->IASetVertexBuffers(
            0,
            1,
            &vertex_buffer_ptr,
            &vertex_stride,
            &vertex_offset);
        else
            device_context_ptr->IASetVertexBuffers(
                0,
                1,
                &vertex_buffer_ptr2,
                &vertex_stride,
                &vertex_offset);

        /*Set the Shaders
        Before drawing, we also need to tell the pipeline which shaders to use next.*/
        device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0);
        device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0);

        /*Draw Our Triangle
        When we call Draw() the pipeline will use all the states we just set, the vertex buffer and the shaders.
        We just need to tell it how many vertices to draw from our vertex buffer - 3 for 1 triangle. 
        We stored this in a variable when we created our float array.*/
        device_context_ptr->Draw(vertex_count, 0);


        /*Present the Frame (Swap the Buffers)
        At the very end of our drawing loop we need to swap the buffers. DXGI calls this Present() 
        This should be after all of our Draw() calls. We only have 1 so far. 
        The swap chain commands use the DXGI interface.*/
        swap_chain_ptr->Present(1, 0);

    }
    

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        switch (wParam) {
        case IDT_TIMER1:
            color_value = (color_value + 1) % 256;
            break;
        }

    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    //case WM_PAINT:
    //    {
    //        PAINTSTRUCT ps;
    //        HDC hdc = BeginPaint(hWnd, &ps);
    //        // TODO: Add any drawing code that uses hdc here...
    //        EndPaint(hWnd, &ps);
    //    }
    //    break;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
