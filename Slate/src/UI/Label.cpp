#include "Slate/UI/Label.h"

#include <utility>

namespace Slate
{
	Label::Label(
		std::wstring text,
		const Rectangle2D& bounds,
		const TextStyle& style)
		: m_Text(std::move(text)), m_Style(style)
	{
		m_Bounds = bounds;
	}

	void Label::SetText(std::wstring text)
	{
		m_Text = std::move(text);
	}

	void Label::Render(Renderer& renderer) const
	{
		if (m_IsVisible)
		{
			renderer.DrawText2D(m_Text, m_Bounds, m_Style);
		}
	}
}
