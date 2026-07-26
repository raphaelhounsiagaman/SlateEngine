#pragma once

#include "UIElement.h"

#include <string>

namespace Slate
{
	class Label : public UIElement
	{
	public:
		Label(
			std::wstring text,
			const Rectangle2D& bounds,
			const TextStyle& style = {}
		);

		void SetText(std::wstring text);
		const std::wstring& GetText() const { return m_Text; }

		void SetStyle(const TextStyle& style) { m_Style = style; }
		const TextStyle& GetStyle() const { return m_Style; }

		void Render(Renderer& renderer) const override;

	private:
		std::wstring m_Text;
		TextStyle m_Style;
	};
}
