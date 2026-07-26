#pragma once

#include "Slate/Graphics/Renderer.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <vector>

namespace Slate
{

	class Renderer::Implementation
	{
	public:

		void Create(HWND windowHandle, unsigned int width, unsigned int height);
		void Destroy();
		void BeginFrame(const Color& clearColor);
		void Present();
		void Resize(unsigned int width, unsigned int height);

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
		void CreateRenderTarget();
		void CreateDepthBuffer(unsigned int width, unsigned int height);
		void Create3DPipeline();
		void SetViewport(unsigned int width, unsigned int height);

		void ClearRenderTarget(const Color& color);

		void UpdateObjectConstants(
			const Transform3D& transform,
			const MaterialResource& material
		);
		void BindMesh3D(const Mesh3DResource& mesh);
		void Bind3DPipeline3D();

		const Mesh3DResource& GetMesh3D(const Mesh3DHandle& handle) const;
		const MaterialResource& GetMaterial(const MaterialHandle& handle) const;

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

		std::vector<Mesh3DResource> m_Meshes3D;
		std::vector<MaterialResource> m_Materials;

		Camera3D m_Camera3D{};

		unsigned int m_ViewportWidth = 0;
		unsigned int m_ViewportHeight = 0;

		unsigned int m_NextMeshGeneration = 1;
		unsigned int m_NextMaterialGeneration = 1;
	};

}
