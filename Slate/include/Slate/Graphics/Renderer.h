#pragma once

#include "Slate/Math/Math.h"

#include "Camera3D.h"
#include "Canvas2D.h"
#include "Color.h"
#include "Handles.h"
#include "Vertex.h"

#include <Windows.h>
#include <filesystem>
#include <memory>
#include <span> 
#include <string_view>

namespace Slate
{

	class Renderer
	{

	public:
		Renderer();
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void Create(HWND windowHandle, unsigned int width, unsigned int height);
		void Destroy();

		void BeginFrame();
		void Present();

		void Resize(
			unsigned int widthPixels,
			unsigned int heightPixels
		);

		void SetClearColor(const Color& color) { m_ClearColor = color; }
		void SetCamera3D(const Camera3D& camera);
		void SetVSyncEnabled(bool isEnabled)
		{
			m_IsVSyncEnabled = isEnabled;
		}
		bool IsVSyncEnabled() const { return m_IsVSyncEnabled; }
		bool IsTearingSupported() const;

	public:

		Mesh3DHandle CreateMesh3D(
			std::span<const Vertex3D> vertices,
			std::span<const unsigned int> indices
		);

		MaterialHandle CreateMaterial(const Color& color);
		Texture2DHandle CreateTexture2D(
			const std::filesystem::path& filePath
		);

	public:

		void DrawMesh3D(const Mesh3DHandle& meshHandle, const MaterialHandle& materialHandle, const Transform3D& transform);
		void DrawRectangle2D(
			const Rectangle2D& rectangle,
			const Color& color,
			float cornerRadiusPixels = 0.0f
		);
		void DrawText2D(
			std::wstring_view text,
			const Rectangle2D& bounds,
			const TextStyle& style = {}
		);
		void DrawTexture2D(
			const Texture2DHandle& texture,
			const Rectangle2D& bounds,
			float opacity = 1.0f
		);


	private:
		
		class Implementation;
		std::unique_ptr<Implementation> m_Implementation;

		Color m_ClearColor;
		bool m_IsVSyncEnabled = false;

	};




}
