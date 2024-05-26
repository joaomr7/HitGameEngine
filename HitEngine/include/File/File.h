#pragma once

#include "Core/Types.h"
#include "Utils/FastTypedArena.h"

#include <string>
#include <string_view>

namespace hit
{
	struct File
	{
		enum Type
		{
			Text, Binary
		};

		std::string read_as_text();

		Type type;
		FastTypedArena<ui8> data;
	};

	bool file_read(std::string_view filename, File::Type type, File& out_file);
	bool file_save(std::string_view filename, const File& file);
}