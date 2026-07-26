#pragma once

#include "UIElement.h"

namespace Slate
{
	class Image : public UIElement
	{
	public:
		Image(
			const Texture2DHandle& texture,
			const Rectangle2D& bounds,
			float opacity = 1.0f
		);

		void SetTexture(const Texture2DHandle& texture)
		{
			m_Texture = texture;
		}
		const Texture2DHandle& GetTexture() const { return m_Texture; }

		void SetOpacity(float opacity);
		float GetOpacity() const { return m_Opacity; }

		void Render(Renderer& renderer) const override;

	private:
		Texture2DHandle m_Texture;
		float m_Opacity = 1.0f;
	};
}
