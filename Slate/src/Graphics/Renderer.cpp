#include "Slate/Graphics/Renderer.h"

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <stdexcept>

namespace Slate
{

    class Renderer::Implementation
    {
    public:
       
		void CreateDeviceAndSwapChain(HWND windowHandle, unsigned int width, unsigned int height);
		void CreateRenderTarget();
		void SetViewport(unsigned int width, unsigned int height);
		void ResizeRenderer(unsigned int width, unsigned int height);

        void BeginFrame();
        void Present();


		void ClearRenderTarget(const Color& color);

    private:
        void ThrowIfFailed(HRESULT result, const char* message);

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_D3D11Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_D3D11DeviceContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_D3D11SwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_D3D11RenderTargetView;

    };


    Renderer::Renderer()
        : m_Implementation(std::make_unique<Implementation>())
    {}

    Renderer::~Renderer()
    {}

    void Renderer::Create(HWND windowHandle, unsigned int width, unsigned int height)
    {
        
        m_Implementation->CreateDeviceAndSwapChain(windowHandle, width, height);
        m_Implementation->CreateRenderTarget();
        m_Implementation->SetViewport(width, height);
    }

    void Renderer::Destroy()
    {

    }

    void Renderer::BeginFrame()
    {

        m_Implementation->BeginFrame();
        m_Implementation->ClearRenderTarget(m_ClearColor);

    }

    void Renderer::Present()
    {
        m_Implementation->Present();
    }

    void Renderer::Resize(unsigned int width, unsigned int height)
    {
        m_Implementation->ResizeRenderer(width, height);
    }

    Mesh2DHandle Renderer::CreateMesh2D()
    {
        return Mesh2DHandle();
    }

    Mesh3DHandle Renderer::CreateMesh3D()
    {
        return Mesh3DHandle();
    }

    void Renderer::DrawMesh2D(const Mesh2DHandle& meshHandle, const MaterialHandle& materialHandle, const Transform2D& transform)
    {

    }

    void Renderer::DrawMesh3D(const Mesh3DHandle& meshHandle, const MaterialHandle& materialHandle, const Transform3D& transform)
    {}




    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //
	// RENDERER IMPLEMENTATION
    //
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////



    void Renderer::Implementation::CreateDeviceAndSwapChain(HWND windowHandle, unsigned int width, unsigned int height)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDescription{};
        swapChainDescription.BufferDesc.Width = width;
        swapChainDescription.BufferDesc.Height = height;
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;

        swapChainDescription.SampleDesc.Count = 1;
        swapChainDescription.SampleDesc.Quality = 0;

        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 2;
        swapChainDescription.OutputWindow = windowHandle;
        swapChainDescription.Windowed = TRUE;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDescription.Flags = 0;

        UINT deviceFlags = 0;

#ifdef _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        constexpr D3D_FEATURE_LEVEL requestedFeatureLevels[]
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        D3D_FEATURE_LEVEL selectedFeatureLevel{};

        HRESULT result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            deviceFlags,
            requestedFeatureLevels,
            static_cast<UINT>(std::size(requestedFeatureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDescription,
            m_D3D11SwapChain.GetAddressOf(),
            m_D3D11Device.GetAddressOf(),
            &selectedFeatureLevel,
            m_D3D11DeviceContext.GetAddressOf()
        );

        if (result == E_INVALIDARG)
        {
            constexpr D3D_FEATURE_LEVEL fallbackFeatureLevels[]
            {
                D3D_FEATURE_LEVEL_11_0
            };

            result = D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                deviceFlags,
                fallbackFeatureLevels,
                static_cast<UINT>(std::size(fallbackFeatureLevels)),
                D3D11_SDK_VERSION,
                &swapChainDescription,
                m_D3D11SwapChain.GetAddressOf(),
                m_D3D11Device.GetAddressOf(),
                &selectedFeatureLevel,
                m_D3D11DeviceContext.GetAddressOf()
            );
        }

        ThrowIfFailed(result, "Failed to create the Direct3D 11 device.");

        

    }

    void Renderer::Implementation::CreateRenderTarget()
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

        ThrowIfFailed(
            m_D3D11SwapChain->GetBuffer(
                0,
                IID_PPV_ARGS(backBuffer.GetAddressOf())
            ),
            "Failed to get the swap-chain back buffer."
        );

        ThrowIfFailed(
            m_D3D11Device->CreateRenderTargetView(
                backBuffer.Get(),
                nullptr,
                m_D3D11RenderTargetView.GetAddressOf()
            ),
            "Failed to create the render-target view."
        );
    }

    void Renderer::Implementation::SetViewport(unsigned int width, unsigned int height)
    {
        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        m_D3D11DeviceContext->RSSetViewports(
            1,
            &viewport
        );
    }

    void Renderer::Implementation::ResizeRenderer(unsigned int width, unsigned int height)
    {
		if (!m_D3D11SwapChain)
		{
			return;
		}

        if (width == 0 || height == 0)
        {
            return;
        }

        m_D3D11DeviceContext->OMSetRenderTargets(
            0,
            nullptr,
            nullptr
        );

        m_D3D11RenderTargetView.Reset();

        ThrowIfFailed(
            m_D3D11SwapChain->ResizeBuffers(
                0,
                width,
                height,
                DXGI_FORMAT_UNKNOWN,
                0
            ),
            "Failed to resize the swap-chain buffers."
        );

        CreateRenderTarget();

        SetViewport(width, height);
    }

    void Renderer::Implementation::BeginFrame()
    {
        ID3D11RenderTargetView* renderTargets[]
        {
            m_D3D11RenderTargetView.Get()
        };

        m_D3D11DeviceContext->OMSetRenderTargets(
            1,
            renderTargets,
            nullptr
        );
    }

    void Renderer::Implementation::Present()
    {
        ThrowIfFailed(
            m_D3D11SwapChain->Present(1, 0),
            "Failed to present the swap chain."
        );
    
    }

    

    void Renderer::Implementation::ClearRenderTarget(const Color& color)
    {
        m_D3D11DeviceContext->ClearRenderTargetView(
            m_D3D11RenderTargetView.Get(),
            color.GetFloatArray().data()
        );
    }

    void Renderer::Implementation::ThrowIfFailed(HRESULT result, const char* message)
    {
        if (FAILED(result))
        {
            throw std::runtime_error(message);
        }
    }




}

