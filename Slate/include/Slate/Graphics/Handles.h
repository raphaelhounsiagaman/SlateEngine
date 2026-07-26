#pragma once

namespace Slate
{

	struct Mesh3DHandle
	{
		unsigned int Index = 0;
		unsigned int Generation = 0;

		explicit operator bool() const
		{
			return Generation != 0;
		}
	};

	struct MaterialHandle
	{
		unsigned int Index = 0;
		unsigned int Generation = 0;

		explicit operator bool() const
		{
			return Generation != 0;
		}
	};

	struct Texture2DHandle
	{
		unsigned int Index = 0;
		unsigned int Generation = 0;

		explicit operator bool() const
		{
			return Generation != 0;
		}
	};

}




