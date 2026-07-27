#include "Slate/UI/Label.h"

#include "UIElementVisitor.h"

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

	void Label::Accept(UIElementVisitor& visitor) const
	{
		visitor.Visit(*this);
	}
}
