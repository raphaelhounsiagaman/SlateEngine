#pragma once

#include "Slate/Math/Math.h"

#include "Camera3D.h"
#include "Color.h"
#include "Handles.h"
#include "Vertex.h"

#include <Windows.h>
#include <filesystem>
#include <memory>
#include <span>

namespace Slate
{
	class UICanvas;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void Create(HWND windowHandle, Vector2iu windowSize);
		void Destroy();

		void BeginFrame();
		void Present();

		void Resize(Vector2iu windowSize);

		void SetClearColor(const Color& color) { m_ClearColor = color; }
		void SetCamera3D(const Camera3D& camera);
		void SetVSyncEnabled(bool isEnabled)
		{
			m_IsVSyncEnabled = isEnabled;
		}

		bool IsVSyncEnabled() const { return m_IsVSyncEnabled; }

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
		void DrawCanvas(const UICanvas& canvas);

	private:
		class Implementation;
		class CanvasVisitor;

		std::unique_ptr<Implementation> m_Implementation;

		Color m_ClearColor;
		bool m_IsVSyncEnabled = false;
	};
}
