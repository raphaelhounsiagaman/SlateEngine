#pragma once

#include "Slate/Graphics/Canvas2D.h"
#include "Slate/Graphics/Renderer.h"
#include "Slate/Input/MouseCodes.h"
#include "Slate/Math/Vector.h"

namespace Slate
{
	class UIElement
	{
	public:
		virtual ~UIElement() = default;

		void SetBounds(const Rectangle2D& bounds) { m_Bounds = bounds; }
		const Rectangle2D& GetBounds() const { return m_Bounds; }

		void SetVisible(bool isVisible) { m_IsVisible = isVisible; }
		bool IsVisible() const { return m_IsVisible; }

		void SetEnabled(bool isEnabled) { m_IsEnabled = isEnabled; }
		bool IsEnabled() const { return m_IsEnabled; }

		virtual void Render(Renderer& renderer) const = 0;

		virtual bool OnMouseMoved(const Vector2i&) { return false; }
		virtual bool OnMouseButtonPressed(
			MouseButton,
			const Vector2i&)
		{
			return false;
		}
		virtual bool OnMouseButtonReleased(
			MouseButton,
			const Vector2i&)
		{
			return false;
		}
		virtual void CancelInteraction() {}

	protected:
		Rectangle2D m_Bounds;
		bool m_IsVisible = true;
		bool m_IsEnabled = true;
	};
}
