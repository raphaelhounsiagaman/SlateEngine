#pragma once

#include "UIElement.h"

#include <functional>
#include <string>

namespace Slate
{
	struct ButtonStyle
	{
		Color NormalColor{ 45, 50, 62 };
		Color HoveredColor{ 62, 70, 88 };
		Color PressedColor{ 30, 34, 43 };
		Color DisabledColor{ 70, 70, 70 };
		float CornerRadiusPixels = 8.0f;
		TextStyle Text
		{
			L"Segoe UI",
			22.0f,
			Color::WHITE,
			true,
			HorizontalTextAlignment::Center,
			VerticalTextAlignment::Center
		};
	};

	class Button : public UIElement
	{
	public:
		using ClickHandler = std::function<void()>;

		Button(
			std::wstring text,
			const Rectangle2D& bounds,
			ClickHandler onClick = {},
			const ButtonStyle& style = {}
		);

		void SetText(std::wstring text);
		const std::wstring& GetText() const { return m_Text; }

		void SetOnClick(ClickHandler onClick);
		void SetStyle(const ButtonStyle& style) { m_Style = style; }
		const ButtonStyle& GetStyle() const { return m_Style; }

		bool IsHovered() const { return m_IsHovered; }
		bool IsPressed() const { return m_IsPressed; }

		bool OnMouseMoved(const Vector2i& positionPixels) override;
		bool OnMouseButtonPressed(
			MouseButton button,
			const Vector2i& positionPixels
		) override;
		bool OnMouseButtonReleased(
			MouseButton button,
			const Vector2i& positionPixels
		) override;
		void CancelInteraction() override;

	private:
		void Accept(UIElementVisitor& visitor) const override;

		std::wstring m_Text;
		ClickHandler m_OnClick;
		ButtonStyle m_Style;
		bool m_IsHovered = false;
		bool m_IsPressed = false;
	};
}
