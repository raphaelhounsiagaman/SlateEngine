#include "Slate/UI/Image.h"

#include "UIElementVisitor.h"

#include <algorithm>

namespace Slate
{
	Image::Image(
		const Texture2DHandle& texture,
		const Rectangle2D& bounds,
		float opacity)
		: m_Texture(texture)
	{
		m_Bounds = bounds;
		SetOpacity(opacity);
	}

	void Image::SetOpacity(float opacity)
	{
		m_Opacity = std::clamp(opacity, 0.0f, 1.0f);
	}

	void Image::Accept(UIElementVisitor& visitor) const
	{
		visitor.Visit(*this);
	}
}
