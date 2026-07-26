#include "Slate/Graphics/Renderer.h"

#include <d3d11.h>
#include <d3dcompiler.h>
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

		void CreateDepthBuffer(unsigned int width, unsigned int height);
        void Create3DPipeline();

		void SetViewport(unsigned int width, unsigned int height);
		void ResizeRenderer(unsigned int width, unsigned int height);

        void Destroy();

        void BeginFrame();
        void Present();

		void ClearRenderTarget(const Color& color);
        void SetCamera3D(const Camera3D& camera);

        Mesh3DHandle CreateMesh3D(
            std::span<const Vertex3D> vertices,
            std::span<const unsigned int> indices
        );
        MaterialHandle CreateMaterial(const Color& color);

        void DrawMesh3D(
            const Mesh3DHandle& meshHandle,
            const MaterialHandle& materialHandle,
            const Transform3D& transform
        );

    private:

        struct ObjectConstants
        {
            Matrix4x4 World;
            Matrix4x4 View;
            Matrix4x4 Projection;
            float Albedo[4];
        };

        Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
            const char* source,
            const char* entryPoint,
            const char* target
        );

        void ThrowIfFailed(HRESULT result, const char* message);

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_D3D11Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_D3D11DeviceContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_D3D11SwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_D3D11RenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_D3D11DepthBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_D3D11DepthStencilView;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_D3D11VertexShader3D;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_D3D11PixelShader3D;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_D3D11InputLayout3D;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_D3D11ObjectConstantBuffer;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_D3D11RasterizerState3D;

        Camera3D m_Camera3D{};

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
		m_Implementation->CreateDepthBuffer(width, height);
        m_Implementation->Create3DPipeline();
        m_Implementation->SetViewport(width, height);
    }

    void Renderer::Destroy()
    {
        m_Implementation->Destroy();
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

    void Renderer::SetCamera3D(const Camera3D& camera)
    {
        m_Implementation->SetCamera3D(camera);
    }

    Mesh2DHandle Renderer::CreateMesh2D()
    {
        return Mesh2DHandle();
    }

    Mesh3DHandle Renderer::CreateMesh3D(std::span<const Vertex3D> vertices, std::span<const unsigned int> indices)
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



    void Renderer::Implementation::CreateDeviceAndSwapChain(
        HWND windowHandle, 
        unsigned int width, 
        unsigned int height
    )
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

#ifdef SLATE_DEBUG
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

    void Renderer::Implementation::CreateDepthBuffer(unsigned int width, unsigned int height)
    {
        D3D11_TEXTURE2D_DESC depthBufferDescription{};
        depthBufferDescription.Width = width;
        depthBufferDescription.Height = height;
        depthBufferDescription.MipLevels = 1;
        depthBufferDescription.ArraySize = 1;
        depthBufferDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthBufferDescription.SampleDesc.Count = 1;
        depthBufferDescription.Usage = D3D11_USAGE_DEFAULT;
        depthBufferDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        ThrowIfFailed(
            m_D3D11Device->CreateTexture2D(
                &depthBufferDescription,
                nullptr,
                m_D3D11DepthBuffer.GetAddressOf()
            ),
            "Failed to create the depth buffer."
        );

        ThrowIfFailed(
            m_D3D11Device->CreateDepthStencilView(
                m_D3D11DepthBuffer.Get(),
                nullptr,
                m_D3D11DepthStencilView.GetAddressOf()
            ),
            "Failed to create the depth-stencil view."
        );
    }

    void Renderer::Implementation::Create3DPipeline()
    {
        static constexpr char shaderSource[] = R"(
            cbuffer ObjectConstants : register(b0)
            {
                row_major float4x4 World;
                row_major float4x4 View;
                row_major float4x4 Projection;
                float4 Albedo;
            };

            struct VertexInput
            {
                float3 Position : POSITION;
                float3 Normal : NORMAL;
            };

            struct VertexOutput
            {
                float4 Position : SV_POSITION;
                float3 WorldNormal : NORMAL;
            };

            VertexOutput VSMain(VertexInput input)
            {
                VertexOutput output;

                float4 worldPosition = mul(float4(input.Position, 1.0f), World);
                float4 viewPosition = mul(worldPosition, View);

                output.Position = mul(viewPosition, Projection);
                output.WorldNormal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);

                return output;
            }

            float4 PSMain(VertexOutput input) : SV_TARGET
            {
                const float3 lightDirection = normalize(float3(-0.5f, 0.8f, -0.6f));
                const float diffuse = saturate(dot(normalize(input.WorldNormal), lightDirection));
                const float brightness = 0.25f + diffuse * 0.75f;

                return float4(Albedo.rgb * brightness, Albedo.a);
            }
        )";

        const Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode =
            CompileShader(shaderSource, "VSMain", "vs_5_0");

        const Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode =
            CompileShader(shaderSource, "PSMain", "ps_5_0");

        ThrowIfFailed(
            m_D3D11Device->CreateVertexShader(
                vertexShaderByteCode->GetBufferPointer(),
                vertexShaderByteCode->GetBufferSize(),
                nullptr,
                m_D3D11VertexShader3D.GetAddressOf()
            ),
            "Failed to create the 3D vertex shader."
        );

        ThrowIfFailed(
            m_D3D11Device->CreatePixelShader(
                pixelShaderByteCode->GetBufferPointer(),
                pixelShaderByteCode->GetBufferSize(),
                nullptr,
                m_D3D11PixelShader3D.GetAddressOf()
            ),
            "Failed to create the 3D pixel shader."
        );

        const D3D11_INPUT_ELEMENT_DESC inputElements[]
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                static_cast<UINT>(offsetof(Vertex3D, Position)),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "NORMAL",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                static_cast<UINT>(offsetof(Vertex3D, Normal)),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };

        ThrowIfFailed(
            m_D3D11Device->CreateInputLayout(
                inputElements,
                static_cast<UINT>(std::size(inputElements)),
                vertexShaderByteCode->GetBufferPointer(),
                vertexShaderByteCode->GetBufferSize(),
                m_D3D11InputLayout3D.GetAddressOf()
            ),
            "Failed to create the 3D input layout."
        );

        D3D11_BUFFER_DESC constantBufferDescription{};
        constantBufferDescription.ByteWidth = sizeof(ObjectConstants);
        constantBufferDescription.Usage = D3D11_USAGE_DEFAULT;
        constantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        ThrowIfFailed(
            m_D3D11Device->CreateBuffer(
                &constantBufferDescription,
                nullptr,
                m_D3D11ObjectConstantBuffer.GetAddressOf()
            ),
            "Failed to create the 3D object constant buffer."
        );

        D3D11_RASTERIZER_DESC rasterizerDescription{};
        rasterizerDescription.FillMode = D3D11_FILL_SOLID;
        rasterizerDescription.CullMode = D3D11_CULL_BACK;
        rasterizerDescription.FrontCounterClockwise = FALSE;
        rasterizerDescription.DepthClipEnable = TRUE;

        ThrowIfFailed(
            m_D3D11Device->CreateRasterizerState(
                &rasterizerDescription,
                m_D3D11RasterizerState3D.GetAddressOf()
            ),
            "Failed to create the 3D rasterizer state."
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

    void Renderer::Implementation::Destroy()
    {
        if (m_D3D11DeviceContext)
        {
            m_D3D11DeviceContext->ClearState();
            m_D3D11DeviceContext->Flush();
        }

        m_D3D11DepthStencilView.Reset();
        m_D3D11DepthBuffer.Reset();
        m_D3D11RenderTargetView.Reset();

        m_D3D11SwapChain.Reset();
        m_D3D11DeviceContext.Reset();
        m_D3D11Device.Reset();
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

    void Renderer::Implementation::SetCamera3D(const Camera3D& camera)
    {
            m_Camera3D = camera;
    }

    Mesh3DHandle Renderer::Implementation::CreateMesh3D(std::span<const Vertex3D> vertices, std::span<const unsigned int> indices)
    {
        return Mesh3DHandle();
    }

    MaterialHandle Renderer::Implementation::CreateMaterial(const Color& color)
    {
        return MaterialHandle();
    }

    void Renderer::Implementation::DrawMesh3D(const Mesh3DHandle& meshHandle, const MaterialHandle& materialHandle, const Transform3D& transform)
    {}

    Microsoft::WRL::ComPtr<ID3DBlob> Renderer::Implementation::CompileShader(
        const char* source, 
        const char* entryPoint,
        const char* target
    )
    {
        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef SLATE_DEBUG
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> shaderByteCode;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;

        const HRESULT result = D3DCompile(
            source,
            std::strlen(source),
            nullptr,
            nullptr,
            nullptr,
            entryPoint,
            target,
            compileFlags,
            0,
            shaderByteCode.GetAddressOf(),
            errors.GetAddressOf()
        );

        if (FAILED(result))
        {
            std::string message = "Failed to compile the Direct3D 11 shader.";

            if (errors)
            {
                message += "\n";
                message.append(
                    static_cast<const char*>(errors->GetBufferPointer()),
                    errors->GetBufferSize()
                );
            }

            throw std::runtime_error(message);
        }

        return shaderByteCode;
    }

    void Renderer::Implementation::ThrowIfFailed(HRESULT result, const char* message)
    {
        if (FAILED(result))
        {
            throw std::runtime_error(message);
        }
    }




}

