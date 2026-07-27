#include "Slate/Graphics/Renderer.h"

#include "Slate/UI/Button.h"
#include "Slate/UI/Image.h"
#include "Slate/UI/Label.h"
#include "Slate/UI/UICanvas.h"

#include "D3D11/D3D11Renderer.h"
#include "UI/UIElementVisitor.h"

namespace Slate
{
	class Renderer::CanvasVisitor final : public UIElementVisitor
	{
	public:
		explicit CanvasVisitor(Implementation& implementation)
			: m_Implementation(implementation)
		{}

		void Visit(const Button& button) override
		{
			if (!button.IsVisible())
			{
				return;
			}

			const ButtonStyle& style = button.GetStyle();
			const Color* backgroundColor = &style.NormalColor;
			if (!button.IsEnabled())
			{
				backgroundColor = &style.DisabledColor;
			}
			else if (button.IsPressed())
			{
				backgroundColor = &style.PressedColor;
			}
			else if (button.IsHovered())
			{
				backgroundColor = &style.HoveredColor;
			}

			m_Implementation.DrawRectangle2D(
				button.GetBounds(),
				*backgroundColor,
				style.CornerRadiusPixels
			);
			m_Implementation.DrawText2D(
				button.GetText(),
				button.GetBounds(),
				style.Text
			);
		}

		void Visit(const Image& image) override
		{
			if (image.IsVisible())
			{
				m_Implementation.DrawTexture2D(
					image.GetTexture(),
					image.GetBounds(),
					image.GetOpacity()
				);
			}
		}

		void Visit(const Label& label) override
		{
			if (label.IsVisible())
			{
				m_Implementation.DrawText2D(
					label.GetText(),
					label.GetBounds(),
					label.GetStyle()
				);
			}
		}

	private:
		Implementation& m_Implementation;
	};

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

	void Renderer::DrawCanvas(const UICanvas& canvas)
	{
		CanvasVisitor visitor(*m_Implementation);
		canvas.VisitElements(visitor);
	}

}
