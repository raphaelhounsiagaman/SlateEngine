#include "Slate/Graphics/Renderer.h"

#include "D3D11/D3D11Renderer.h"

namespace Slate
{

	Renderer::Renderer()
		: m_Implementation(std::make_unique<Implementation>())
	{}

	Renderer::~Renderer()
	{}

	void Renderer::Create(HWND windowHandle, Vector2iu windowSize)
	{
		m_Implementation->Create(windowHandle, windowSize);
	}

	void Renderer::Destroy()
	{
		m_Implementation->Destroy();
	}

	void Renderer::BeginFrame()
	{
		m_Implementation->BeginFrame(m_ClearColor);
	}

	void Renderer::Present()
	{
		m_Implementation->Present(m_IsVSyncEnabled);
	}

	void Renderer::Resize(Vector2iu windowSize)
	{
		m_Implementation->Resize(windowSize);
	}

	void Renderer::SetCamera3D(const Camera3D& camera)
	{
		m_Implementation->SetCamera3D(camera);
	}

	bool Renderer::IsTearingSupported() const
	{
		return m_Implementation->IsTearingSupported();
	}

	Mesh3DHandle Renderer::CreateMesh3D(
		std::span<const Vertex3D> vertices,
		std::span<const unsigned int> indices
	)
	{
		return m_Implementation->CreateMesh3D(vertices, indices);
	}

	MaterialHandle Renderer::CreateMaterial(const Color& color)
	{
		return m_Implementation->CreateMaterial(color);
	}

	Texture2DHandle Renderer::CreateTexture2D(
		const std::filesystem::path& filePath)
	{
		return m_Implementation->CreateTexture2D(filePath);
	}

	void Renderer::DrawMesh3D(
		const Mesh3DHandle& meshHandle,
		const MaterialHandle& materialHandle,
		const Transform3D& transform
	)
	{
		m_Implementation->DrawMesh3D(meshHandle, materialHandle, transform);
	}

	void Renderer::DrawRectangle2D(
		const Rectangle2D& rectangle,
		const Color& color,
		float cornerRadiusPixels)
	{
		m_Implementation->DrawRectangle2D(
			rectangle,
			color,
			cornerRadiusPixels
		);
	}

	void Renderer::DrawText2D(
		std::wstring_view text,
		const Rectangle2D& bounds,
		const TextStyle& style)
	{
		m_Implementation->DrawText2D(text, bounds, style);
	}

	void Renderer::DrawTexture2D(
		const Texture2DHandle& texture,
		const Rectangle2D& bounds,
		float opacity)
	{
		m_Implementation->DrawTexture2D(texture, bounds, opacity);
	}

}
