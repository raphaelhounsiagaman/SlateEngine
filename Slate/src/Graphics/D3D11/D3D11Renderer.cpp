#include "D3D11Renderer.h"

#include "Default3DShader.h"

#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <objbase.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#if defined(_MSC_VER)
    #pragma comment(lib, "d2d1.lib")
    #pragma comment(lib, "d3d11.lib")
    #pragma comment(lib, "d3dcompiler.lib")
    #pragma comment(lib, "dwrite.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "windowscodecs.lib")
#endif

namespace Slate
{
    namespace
    {
        D2D1_COLOR_F ToD2DColor(const Color& color)
        {
            const std::array<float, 4> channels = color.GetFloatArray();
            return D2D1::ColorF(
                channels[0],
                channels[1],
                channels[2],
                channels[3]
            );
        }

        D2D1_RECT_F ToD2DRectangle(const Rectangle2D& rectangle)
        {
            return D2D1::RectF(
                rectangle.X,
                rectangle.Y,
                rectangle.X + rectangle.Width,
                rectangle.Y + rectangle.Height
            );
        }
    }

    void Renderer::Implementation::Create(
        HWND windowHandle,
        unsigned int width,
        unsigned int height
    )
    {
        CreateDeviceAndSwapChain(windowHandle, width, height);
        CreateRenderTarget();
        CreateDepthBuffer(width, height);
        Create3DPipeline();
        Create2DResources();
        SetViewport(width, height);
        m_Canvas2DCommands.reserve(64);
    }

    void Renderer::Implementation::CreateDeviceAndSwapChain(
        HWND windowHandle,
        unsigned int width,
        unsigned int height
    )
    {
        m_IsTearingSupported = QueryTearingSupport();

        DXGI_SWAP_CHAIN_DESC swapChainDescription{};
        swapChainDescription.BufferDesc.Width = width;
        swapChainDescription.BufferDesc.Height = height;
        swapChainDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;

        swapChainDescription.SampleDesc.Count = 1;
        swapChainDescription.SampleDesc.Quality = 0;

        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 2;
        swapChainDescription.OutputWindow = windowHandle;
        swapChainDescription.Windowed = TRUE;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDescription.Flags = m_IsTearingSupported
            ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
            : 0;

        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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

    bool Renderer::Implementation::QueryTearingSupport() const
    {
        Microsoft::WRL::ComPtr<IDXGIFactory5> factory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()))))
        {
            return false;
        }

        BOOL isTearingSupported = FALSE;
        const HRESULT result = factory->CheckFeatureSupport(
            DXGI_FEATURE_PRESENT_ALLOW_TEARING,
            &isTearingSupported,
            sizeof(isTearingSupported)
        );

        return SUCCEEDED(result) && isTearingSupported == TRUE;
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
        const Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode =
            CompileShader(Default3DShaderSource, "VSMain", "vs_5_0");

        const Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode =
            CompileShader(Default3DShaderSource, "PSMain", "ps_5_0");

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

    void Renderer::Implementation::Create2DResources()
    {
        const HRESULT comResult = CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED
        );
        if (SUCCEEDED(comResult))
        {
            m_DidInitializeCom = true;
        }
        else if (comResult != RPC_E_CHANGED_MODE)
        {
            ThrowIfFailed(comResult, "Failed to initialize COM for 2D rendering.");
        }

        ThrowIfFailed(
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                m_D2DFactory.GetAddressOf()
            ),
            "Failed to create the Direct2D factory."
        );

        ThrowIfFailed(
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(
                    m_DWriteFactory.GetAddressOf()
                )
            ),
            "Failed to create the DirectWrite factory."
        );

        ThrowIfFailed(
            CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(m_WICFactory.GetAddressOf())
            ),
            "Failed to create the Windows Imaging Component factory."
        );

        Create2DRenderTarget();
    }

    void Renderer::Implementation::Create2DRenderTarget()
    {
        Microsoft::WRL::ComPtr<IDXGISurface> backBufferSurface;
        ThrowIfFailed(
            m_D3D11SwapChain->GetBuffer(
                0,
                IID_PPV_ARGS(backBufferSurface.GetAddressOf())
            ),
            "Failed to get the 2D back-buffer surface."
        );

        const D2D1_RENDER_TARGET_PROPERTIES properties =
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_PREMULTIPLIED
                ),
                96.0f,
                96.0f
            );

        ThrowIfFailed(
            m_D2DFactory->CreateDxgiSurfaceRenderTarget(
                backBufferSurface.Get(),
                &properties,
                m_D2DRenderTarget.GetAddressOf()
            ),
            "Failed to create the Direct2D render target."
        );

        ThrowIfFailed(
            m_D2DRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                m_D2DSolidBrush.GetAddressOf()
            ),
            "Failed to create the Direct2D color brush."
        );

        for (Texture2DResource& texture : m_Textures2D)
        {
            CreateTextureBitmap(texture);
        }
    }

    void Renderer::Implementation::Destroy2DRenderTarget()
    {
        m_D2DSolidBrush.Reset();
        for (Texture2DResource& texture : m_Textures2D)
        {
            texture.Bitmap.Reset();
        }
        m_D2DRenderTarget.Reset();
    }

    void Renderer::Implementation::CreateTextureBitmap(
        Texture2DResource& texture)
    {
        if (!m_D2DRenderTarget || !m_WICFactory)
        {
            return;
        }

        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        ThrowIfFailed(
            m_WICFactory->CreateDecoderFromFilename(
                texture.FilePath.c_str(),
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                decoder.GetAddressOf()
            ),
            "Failed to open the 2D texture file."
        );

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        ThrowIfFailed(
            decoder->GetFrame(0, frame.GetAddressOf()),
            "Failed to decode the first texture frame."
        );

        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        ThrowIfFailed(
            m_WICFactory->CreateFormatConverter(converter.GetAddressOf()),
            "Failed to create the texture format converter."
        );
        ThrowIfFailed(
            converter->Initialize(
                frame.Get(),
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0f,
                WICBitmapPaletteTypeMedianCut
            ),
            "Failed to convert the texture to premultiplied BGRA."
        );

        ThrowIfFailed(
            m_D2DRenderTarget->CreateBitmapFromWicBitmap(
                converter.Get(),
                nullptr,
                texture.Bitmap.GetAddressOf()
            ),
            "Failed to upload the 2D texture."
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

        m_ViewportWidth = width;
        m_ViewportHeight = height;
        UpdateCameraMatrices();
    }

    void Renderer::Implementation::Resize(unsigned int width, unsigned int height)
    {
        if (!m_D3D11SwapChain || width == 0 || height == 0)
        {
            return;
        }

        if (width == m_ViewportWidth && height == m_ViewportHeight)
        {
            return;
        }

        Destroy2DRenderTarget();

        // ResizeBuffers requires every reference to the old back buffer to be
        // released. ClearState also removes bindings outside the output merger.
        m_D3D11DeviceContext->ClearState();
        m_D3D11DeviceContext->Flush();

        m_D3D11RenderTargetView.Reset();
        m_D3D11DepthStencilView.Reset();
        m_D3D11DepthBuffer.Reset();

        ThrowIfFailed(
            m_D3D11SwapChain->ResizeBuffers(
                0,
                width,
                height,
                DXGI_FORMAT_UNKNOWN,
                m_IsTearingSupported
                    ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
                    : 0
            ),
            "Failed to resize the swap-chain buffers."
        );

        CreateRenderTarget();
        CreateDepthBuffer(width, height);
        Create2DRenderTarget();

        SetViewport(width, height);
    }

    void Renderer::Implementation::Destroy()
    {
        Destroy2DRenderTarget();

        if (m_D3D11DeviceContext)
        {
            m_D3D11DeviceContext->ClearState();
            m_D3D11DeviceContext->Flush();
        }

        m_Materials.clear();
        m_Meshes3D.clear();
        m_Textures2D.clear();
        m_TextFormats.clear();
        m_Canvas2DCommands.clear();

        m_D3D11RasterizerState3D.Reset();
        m_D3D11ObjectConstantBuffer.Reset();
        m_D3D11InputLayout3D.Reset();
        m_D3D11PixelShader3D.Reset();
        m_D3D11VertexShader3D.Reset();

        m_D3D11DepthStencilView.Reset();
        m_D3D11DepthBuffer.Reset();
        m_D3D11RenderTargetView.Reset();

        m_D3D11SwapChain.Reset();
        m_D3D11DeviceContext.Reset();
        m_D3D11Device.Reset();

        m_WICFactory.Reset();
        m_DWriteFactory.Reset();
        m_D2DFactory.Reset();

        if (m_DidInitializeCom)
        {
            CoUninitialize();
            m_DidInitializeCom = false;
        }
    }

    void Renderer::Implementation::BeginFrame(const Color& clearColor)
    {
        m_Canvas2DCommands.clear();

        ID3D11RenderTargetView* renderTargets[]
        {
            m_D3D11RenderTargetView.Get()
        };

        m_D3D11DeviceContext->OMSetRenderTargets(
            1,
            renderTargets,
            m_D3D11DepthStencilView.Get()
        );

        Bind3DPipeline3D();
        ClearRenderTarget(clearColor);
    }

    void Renderer::Implementation::Present(bool isVSyncEnabled)
    {
        Render2DCommands();

        ThrowIfFailed(
            m_D3D11SwapChain->Present(
                isVSyncEnabled ? 1u : 0u,
                !isVSyncEnabled && m_IsTearingSupported
                    ? DXGI_PRESENT_ALLOW_TEARING
                    : 0
            ),
            "Failed to present the swap chain."
        );
    }


    void Renderer::Implementation::ClearRenderTarget(const Color& color)
    {
        m_D3D11DeviceContext->ClearRenderTargetView(
            m_D3D11RenderTargetView.Get(),
            color.GetFloatArray().data()
        );

        m_D3D11DeviceContext->ClearDepthStencilView(
            m_D3D11DepthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0
        );
    }

    void Renderer::Implementation::SetCamera3D(const Camera3D& camera)
    {
        m_Camera3D = camera;
        UpdateCameraMatrices();
    }

    void Renderer::Implementation::UpdateCameraMatrices()
    {
        if (m_ViewportWidth == 0 || m_ViewportHeight == 0)
        {
            return;
        }

        const float aspectRatio =
            static_cast<float>(m_ViewportWidth) /
            static_cast<float>(m_ViewportHeight);

        m_ViewMatrix = Matrix4x4::View(m_Camera3D.Transform);
        m_ProjectionMatrix = Matrix4x4::Perspective(
            m_Camera3D.VerticalFieldOfViewRadians,
            aspectRatio,
            m_Camera3D.NearPlaneMetres,
            m_Camera3D.FarPlaneMetres
        );
    }

    Mesh3DHandle Renderer::Implementation::CreateMesh3D(
        std::span<const Vertex3D> vertices,
        std::span<const unsigned int> indices
    )
    {
        if (vertices.empty() || indices.empty())
        {
            throw std::invalid_argument("A 3D mesh needs vertices and indices.");
        }

        if (vertices.size_bytes() > std::numeric_limits<UINT>::max() ||
            indices.size_bytes() > std::numeric_limits<UINT>::max())
        {
            throw std::invalid_argument("The 3D mesh is too large for a Direct3D 11 buffer.");
        }

        for (const unsigned int index : indices)
        {
            if (index >= vertices.size())
            {
                throw std::invalid_argument("A 3D mesh index refers to a missing vertex.");
            }
        }

        D3D11_BUFFER_DESC vertexBufferDescription{};
        vertexBufferDescription.ByteWidth = static_cast<UINT>(vertices.size_bytes());
        vertexBufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexData{};
        vertexData.pSysMem = vertices.data();

        Mesh3DResource mesh{};

        ThrowIfFailed(
            m_D3D11Device->CreateBuffer(
                &vertexBufferDescription,
                &vertexData,
                mesh.VertexBuffer.GetAddressOf()
            ),
            "Failed to create a 3D vertex buffer."
        );

        D3D11_BUFFER_DESC indexBufferDescription{};
        indexBufferDescription.ByteWidth = static_cast<UINT>(indices.size_bytes());
        indexBufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData{};
        indexData.pSysMem = indices.data();

        ThrowIfFailed(
            m_D3D11Device->CreateBuffer(
                &indexBufferDescription,
                &indexData,
                mesh.IndexBuffer.GetAddressOf()
            ),
            "Failed to create a 3D index buffer."
        );

        if (m_Meshes3D.size() >= std::numeric_limits<unsigned int>::max())
        {
            throw std::runtime_error("The renderer has run out of 3D mesh handles.");
        }

        mesh.IndexCount = static_cast<unsigned int>(indices.size());
        mesh.Generation = m_NextMeshGeneration++;

        if (mesh.Generation == 0)
        {
            mesh.Generation = m_NextMeshGeneration++;
        }

        const Mesh3DHandle handle
        {
            static_cast<unsigned int>(m_Meshes3D.size()),
            mesh.Generation
        };

        m_Meshes3D.push_back(std::move(mesh));

        return handle;
    }

    MaterialHandle Renderer::Implementation::CreateMaterial(const Color& color)
    {
        if (m_Materials.size() >= std::numeric_limits<unsigned int>::max())
        {
            throw std::runtime_error("The renderer has run out of material handles.");
        }

        MaterialResource material{};
        material.Albedo = color;
        material.Generation = m_NextMaterialGeneration++;

        if (material.Generation == 0)
        {
            material.Generation = m_NextMaterialGeneration++;
        }

        const MaterialHandle handle
        {
            static_cast<unsigned int>(m_Materials.size()),
            material.Generation
        };

        m_Materials.push_back(material);

        return handle;
    }

    Texture2DHandle Renderer::Implementation::CreateTexture2D(
        const std::filesystem::path& filePath)
    {
        if (filePath.empty() || !std::filesystem::is_regular_file(filePath))
        {
            throw std::invalid_argument("A 2D texture needs an existing file.");
        }

        if (m_Textures2D.size() >= std::numeric_limits<unsigned int>::max())
        {
            throw std::runtime_error("The renderer has run out of 2D texture handles.");
        }

        Texture2DResource texture;
        texture.FilePath = std::filesystem::absolute(filePath);
        texture.Generation = m_NextTextureGeneration++;
        if (texture.Generation == 0)
        {
            texture.Generation = m_NextTextureGeneration++;
        }

        CreateTextureBitmap(texture);

        const Texture2DHandle handle
        {
            static_cast<unsigned int>(m_Textures2D.size()),
            texture.Generation
        };
        m_Textures2D.push_back(std::move(texture));
        return handle;
    }

    void Renderer::Implementation::DrawMesh3D(
        const Mesh3DHandle& meshHandle,
        const MaterialHandle& materialHandle,
        const Transform3D& transform
    )
    {
        const Mesh3DResource& mesh = GetMesh3D(meshHandle);
        const MaterialResource& material = GetMaterial(materialHandle);

        if (m_ViewportWidth == 0 || m_ViewportHeight == 0)
        {
            return;
        }

        UpdateObjectConstants(transform, material);
        BindMesh3D(mesh);

        m_D3D11DeviceContext->DrawIndexed(mesh.IndexCount, 0, 0);
    }

    void Renderer::Implementation::DrawRectangle2D(
        const Rectangle2D& rectangle,
        const Color& color,
        float cornerRadiusPixels)
    {
        if (rectangle.Width <= 0.0f || rectangle.Height <= 0.0f)
        {
            return;
        }

        m_Canvas2DCommands.emplace_back(
            Rectangle2DCommand
            {
                rectangle,
                color,
                std::max(cornerRadiusPixels, 0.0f)
            }
        );
    }

    void Renderer::Implementation::DrawText2D(
        std::wstring_view text,
        const Rectangle2D& bounds,
        const TextStyle& style)
    {
        if (text.empty() ||
            bounds.Width <= 0.0f ||
            bounds.Height <= 0.0f ||
            style.FontSizePixels <= 0.0f)
        {
            return;
        }

        m_Canvas2DCommands.emplace_back(
            Text2DCommand
            {
                std::wstring(text),
                bounds,
                style
            }
        );
    }

    void Renderer::Implementation::DrawTexture2D(
        const Texture2DHandle& texture,
        const Rectangle2D& bounds,
        float opacity)
    {
        GetTexture2D(texture);
        if (bounds.Width <= 0.0f || bounds.Height <= 0.0f)
        {
            return;
        }

        m_Canvas2DCommands.emplace_back(
            Texture2DCommand
            {
                texture,
                bounds,
                std::clamp(opacity, 0.0f, 1.0f)
            }
        );
    }

    void Renderer::Implementation::UpdateObjectConstants(
        const Transform3D& transform,
        const MaterialResource& material
    )
    {
        const std::array<float, 4> albedo = material.Albedo.GetFloatArray();

        const ObjectConstants constants
        {
            Matrix4x4::World(transform),
            m_ViewMatrix,
            m_ProjectionMatrix,
            {
                albedo[0],
                albedo[1],
                albedo[2],
                albedo[3]
            }
        };

        m_D3D11DeviceContext->UpdateSubresource(
            m_D3D11ObjectConstantBuffer.Get(),
            0,
            nullptr,
            &constants,
            0,
            0
        );
    }

    void Renderer::Implementation::BindMesh3D(const Mesh3DResource& mesh)
    {
        ID3D11Buffer* vertexBuffers[]
        {
            mesh.VertexBuffer.Get()
        };

        constexpr UINT strides[]
        {
            sizeof(Vertex3D)
        };

        constexpr UINT offsets[]
        {
            0
        };

        m_D3D11DeviceContext->IASetVertexBuffers(
            0,
            1,
            vertexBuffers,
            strides,
            offsets
        );

        m_D3D11DeviceContext->IASetIndexBuffer(
            mesh.IndexBuffer.Get(),
            DXGI_FORMAT_R32_UINT,
            0
        );
    }

    void Renderer::Implementation::Bind3DPipeline3D()
    {
        m_D3D11DeviceContext->IASetInputLayout(m_D3D11InputLayout3D.Get());
        m_D3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11Buffer* constantBuffers[]
        {
            m_D3D11ObjectConstantBuffer.Get()
        };

        m_D3D11DeviceContext->VSSetShader(m_D3D11VertexShader3D.Get(), nullptr, 0);
        m_D3D11DeviceContext->VSSetConstantBuffers(0, 1, constantBuffers);

        m_D3D11DeviceContext->PSSetShader(m_D3D11PixelShader3D.Get(), nullptr, 0);
        m_D3D11DeviceContext->PSSetConstantBuffers(0, 1, constantBuffers);

        m_D3D11DeviceContext->RSSetState(m_D3D11RasterizerState3D.Get());
    }

    void Renderer::Implementation::Render2DCommands()
    {
        if (!m_D2DRenderTarget || m_Canvas2DCommands.empty())
        {
            return;
        }

        m_D2DRenderTarget->BeginDraw();
        m_D2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        for (const Canvas2DCommand& command : m_Canvas2DCommands)
        {
            std::visit(
                [this](const auto& typedCommand)
                {
                    using TCommand = std::decay_t<decltype(typedCommand)>;

                    if constexpr (std::is_same_v<TCommand, Rectangle2DCommand>)
                    {
                        m_D2DSolidBrush->SetColor(
                            ToD2DColor(typedCommand.FillColor)
                        );
                        const D2D1_ROUNDED_RECT roundedRectangle
                        {
                            ToD2DRectangle(typedCommand.Rectangle),
                            typedCommand.CornerRadiusPixels,
                            typedCommand.CornerRadiusPixels
                        };
                        m_D2DRenderTarget->FillRoundedRectangle(
                            &roundedRectangle,
                            m_D2DSolidBrush.Get()
                        );
                    }
                    else if constexpr (std::is_same_v<TCommand, Text2DCommand>)
                    {
                        m_D2DSolidBrush->SetColor(
                            ToD2DColor(typedCommand.Style.TextColor)
                        );
                        const D2D1_RECT_F textBounds =
                            ToD2DRectangle(typedCommand.Bounds);
                        m_D2DRenderTarget->DrawTextW(
                            typedCommand.Text.c_str(),
                            static_cast<UINT32>(typedCommand.Text.size()),
                            GetTextFormat(typedCommand.Style),
                            &textBounds,
                            m_D2DSolidBrush.Get(),
                            D2D1_DRAW_TEXT_OPTIONS_CLIP
                        );
                    }
                    else if constexpr (std::is_same_v<TCommand, Texture2DCommand>)
                    {
                        const Texture2DResource& texture =
                            GetTexture2D(typedCommand.Texture);
                        const D2D1_RECT_F textureBounds =
                            ToD2DRectangle(typedCommand.Bounds);
                        m_D2DRenderTarget->DrawBitmap(
                            texture.Bitmap.Get(),
                            &textureBounds,
                            typedCommand.Opacity,
                            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
                        );
                    }
                },
                command
            );
        }

        const HRESULT result = m_D2DRenderTarget->EndDraw();
        if (result == D2DERR_RECREATE_TARGET)
        {
            Destroy2DRenderTarget();
            Create2DRenderTarget();
            return;
        }
        ThrowIfFailed(result, "Failed to render the 2D canvas.");
    }

    IDWriteTextFormat* Renderer::Implementation::GetTextFormat(
        const TextStyle& style)
    {
        for (TextFormatResource& resource : m_TextFormats)
        {
            if (resource.FontFamily == style.FontFamily &&
                resource.FontSizePixels == style.FontSizePixels &&
                resource.IsBold == style.IsBold &&
                resource.HorizontalAlignment == style.HorizontalAlignment &&
                resource.VerticalAlignment == style.VerticalAlignment)
            {
                return resource.Format.Get();
            }
        }

        TextFormatResource resource;
        resource.FontFamily = style.FontFamily;
        resource.FontSizePixels = style.FontSizePixels;
        resource.IsBold = style.IsBold;
        resource.HorizontalAlignment = style.HorizontalAlignment;
        resource.VerticalAlignment = style.VerticalAlignment;

        ThrowIfFailed(
            m_DWriteFactory->CreateTextFormat(
                resource.FontFamily.c_str(),
                nullptr,
                resource.IsBold
                    ? DWRITE_FONT_WEIGHT_BOLD
                    : DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                resource.FontSizePixels,
                L"",
                resource.Format.GetAddressOf()
            ),
            "Failed to create the DirectWrite text format."
        );

        DWRITE_TEXT_ALIGNMENT horizontalAlignment =
            DWRITE_TEXT_ALIGNMENT_LEADING;
        if (style.HorizontalAlignment == HorizontalTextAlignment::Center)
        {
            horizontalAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
        }
        else if (style.HorizontalAlignment == HorizontalTextAlignment::Right)
        {
            horizontalAlignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
        }
        resource.Format->SetTextAlignment(horizontalAlignment);

        DWRITE_PARAGRAPH_ALIGNMENT verticalAlignment =
            DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
        if (style.VerticalAlignment == VerticalTextAlignment::Center)
        {
            verticalAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
        }
        else if (style.VerticalAlignment == VerticalTextAlignment::Bottom)
        {
            verticalAlignment = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
        }
        resource.Format->SetParagraphAlignment(verticalAlignment);

        m_TextFormats.push_back(std::move(resource));
        return m_TextFormats.back().Format.Get();
    }

    const Renderer::Implementation::Mesh3DResource&
        Renderer::Implementation::GetMesh3D(const Mesh3DHandle& handle) const
    {
        if (!handle ||
            handle.Index >= m_Meshes3D.size() ||
            m_Meshes3D[handle.Index].Generation != handle.Generation)
        {
            throw std::invalid_argument("The 3D mesh handle is invalid.");
        }

        return m_Meshes3D[handle.Index];
    }

    const Renderer::Implementation::MaterialResource&
        Renderer::Implementation::GetMaterial(const MaterialHandle& handle) const
    {
        if (!handle ||
            handle.Index >= m_Materials.size() ||
            m_Materials[handle.Index].Generation != handle.Generation)
        {
            throw std::invalid_argument("The material handle is invalid.");
        }

        return m_Materials[handle.Index];
    }

    const Renderer::Implementation::Texture2DResource&
        Renderer::Implementation::GetTexture2D(
            const Texture2DHandle& handle) const
    {
        if (!handle ||
            handle.Index >= m_Textures2D.size() ||
            m_Textures2D[handle.Index].Generation != handle.Generation)
        {
            throw std::invalid_argument("The 2D texture handle is invalid.");
        }

        return m_Textures2D[handle.Index];
    }

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
