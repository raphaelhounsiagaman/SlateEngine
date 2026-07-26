#pragma once

#include "Slate/Graphics/Renderer.h"

#include <d2d1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dwrite.h>
#include <dxgi1_5.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace Slate
{

	class Renderer::Implementation
	{
	public:

		void Create(HWND windowHandle, unsigned int width, unsigned int height);
		void Destroy();
		void BeginFrame(const Color& clearColor);
		void Present(bool isVSyncEnabled);
		void Resize(unsigned int width, unsigned int height);
		bool IsTearingSupported() const { return m_IsTearingSupported; }

		void SetCamera3D(const Camera3D& camera);

		Mesh3DHandle CreateMesh3D(
			std::span<const Vertex3D> vertices,
			std::span<const unsigned int> indices
		);
		MaterialHandle CreateMaterial(const Color& color);
		Texture2DHandle CreateTexture2D(
			const std::filesystem::path& filePath
		);

		void DrawMesh3D(
			const Mesh3DHandle& meshHandle,
			const MaterialHandle& materialHandle,
			const Transform3D& transform
		);
		void DrawRectangle2D(
			const Rectangle2D& rectangle,
			const Color& color,
			float cornerRadiusPixels
		);
		void DrawText2D(
			std::wstring_view text,
			const Rectangle2D& bounds,
			const TextStyle& style
		);
		void DrawTexture2D(
			const Texture2DHandle& texture,
			const Rectangle2D& bounds,
			float opacity
		);

	private:

		struct Mesh3DResource
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
			unsigned int IndexCount = 0;
			unsigned int Generation = 0;
		};

		struct MaterialResource
		{
			Color Albedo;
			unsigned int Generation = 0;
		};

		struct Texture2DResource
		{
			std::filesystem::path FilePath;
			Microsoft::WRL::ComPtr<ID2D1Bitmap> Bitmap;
			unsigned int Generation = 0;
		};

		struct Rectangle2DCommand
		{
			Rectangle2D Rectangle;
			Color FillColor;
			float CornerRadiusPixels = 0.0f;
		};

		struct Text2DCommand
		{
			std::wstring Text;
			Rectangle2D Bounds;
			TextStyle Style;
		};

		struct Texture2DCommand
		{
			Texture2DHandle Texture;
			Rectangle2D Bounds;
			float Opacity = 1.0f;
		};

		using Canvas2DCommand = std::variant<
			Rectangle2DCommand,
			Text2DCommand,
			Texture2DCommand
		>;

		struct TextFormatResource
		{
			std::wstring FontFamily;
			float FontSizePixels = 0.0f;
			bool IsBold = false;
			HorizontalTextAlignment HorizontalAlignment =
				HorizontalTextAlignment::Left;
			VerticalTextAlignment VerticalAlignment =
				VerticalTextAlignment::Top;
			Microsoft::WRL::ComPtr<IDWriteTextFormat> Format;
		};

		struct ObjectConstants
		{
			Matrix4x4 World;
			Matrix4x4 View;
			Matrix4x4 Projection;
			float Albedo[4];
		};

		static_assert(sizeof(ObjectConstants) % 16 == 0);

		void CreateDeviceAndSwapChain(
			HWND windowHandle,
			unsigned int width,
			unsigned int height
		);
		bool QueryTearingSupport() const;
		void CreateRenderTarget();
		void CreateDepthBuffer(unsigned int width, unsigned int height);
		void Create3DPipeline();
		void Create2DResources();
		void Create2DRenderTarget();
		void Destroy2DRenderTarget();
		void CreateTextureBitmap(Texture2DResource& texture);
		void SetViewport(unsigned int width, unsigned int height);
		void UpdateCameraMatrices();

		void ClearRenderTarget(const Color& color);

		void UpdateObjectConstants(
			const Transform3D& transform,
			const MaterialResource& material
		);
		void BindMesh3D(const Mesh3DResource& mesh);
		void Bind3DPipeline3D();
		void Render2DCommands();
		IDWriteTextFormat* GetTextFormat(const TextStyle& style);

		const Mesh3DResource& GetMesh3D(const Mesh3DHandle& handle) const;
		const MaterialResource& GetMaterial(const MaterialHandle& handle) const;
		const Texture2DResource& GetTexture2D(
			const Texture2DHandle& handle
		) const;

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

		Microsoft::WRL::ComPtr<ID2D1Factory> m_D2DFactory;
		Microsoft::WRL::ComPtr<ID2D1RenderTarget> m_D2DRenderTarget;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_D2DSolidBrush;
		Microsoft::WRL::ComPtr<IDWriteFactory> m_DWriteFactory;
		Microsoft::WRL::ComPtr<IWICImagingFactory> m_WICFactory;

		std::vector<Mesh3DResource> m_Meshes3D;
		std::vector<MaterialResource> m_Materials;
		std::vector<Texture2DResource> m_Textures2D;
		std::vector<TextFormatResource> m_TextFormats;
		std::vector<Canvas2DCommand> m_Canvas2DCommands;

		Camera3D m_Camera3D{};
		Matrix4x4 m_ViewMatrix = Matrix4x4::Identity();
		Matrix4x4 m_ProjectionMatrix = Matrix4x4::Identity();

		unsigned int m_ViewportWidth = 0;
		unsigned int m_ViewportHeight = 0;

		unsigned int m_NextMeshGeneration = 1;
		unsigned int m_NextMaterialGeneration = 1;
		unsigned int m_NextTextureGeneration = 1;
		bool m_DidInitializeCom = false;
		bool m_IsTearingSupported = false;
	};

}
