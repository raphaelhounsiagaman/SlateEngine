#pragma once

namespace Slate
{
	class Button;
	class Image;
	class Label;

	class UIElementVisitor
	{
	public:
		virtual ~UIElementVisitor() = default;

		virtual void Visit(const Button& button) = 0;
		virtual void Visit(const Image& image) = 0;
		virtual void Visit(const Label& label) = 0;
	};
}
