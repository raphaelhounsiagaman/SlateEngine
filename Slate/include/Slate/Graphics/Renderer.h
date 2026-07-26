#pragma once

#include "Slate/Math/Math.h"

#include "Color.h"
#include "Handles.h"

#include <Windows.h>
#include <memory>

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

		// TODO: Update this to use Vector2
		void Resize(unsigned int width,	unsigned int height);

		void SetClearColor(const Color& color) { m_ClearColor = color; }

	public:

		Mesh2DHandle CreateMesh2D();
		Mesh3DHandle CreateMesh3D();

	public:

		void DrawMesh2D(const Mesh2DHandle& meshHandle, const MaterialHandle& materialHandle, const Transform2D& transform);
		void DrawMesh3D(const Mesh3DHandle& meshHandle, const MaterialHandle& materialHandle, const Transform3D& transform);
		


	private:
		
		class Implementation;
		std::unique_ptr<Implementation> m_Implementation;

		Color m_ClearColor;

	};




}
