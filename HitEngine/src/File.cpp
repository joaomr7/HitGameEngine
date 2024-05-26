#include "File/File.h"
#include "Core/Log.h"

#include <fstream>

namespace hit
{
	bool file_read(std::string_view filename, File::Type type, File& out_file)
	{
		std::ios::openmode open_mode = type == File::Binary ? std::ios::binary : 1;

		std::ifstream read(filename.data(), open_mode);

		if(!read.is_open())
		{
			hit_error("Failed to read file '{}'.", filename);
			return false;
		}

		auto begin = read.tellg();
		read.seekg(0, std::ios::end);
		auto end = (ui64)read.tellg();

		if(!out_file.data.create(end - begin, MemoryUsage::Any))
		{
			read.close();
			hit_error("Failed to allocate file data buffer!");

			return false;
		}

		auto data_ptr = out_file.data.push_array(end - begin);

		read.seekg(0, std::ios::beg);
		read.read((char*)data_ptr, end - begin);

		read.close();

		out_file.type = type;

		return true;
	}

	bool file_save(std::string_view filename, const File& file)
	{
		std::ios::openmode open_mode = file.type == File::Binary ? std::ios::binary : 1;

		std::ofstream write(filename.data(), open_mode);

		if(!write.is_open())
		{
			hit_error("Failed to open file '{}'.", filename);
			return false;
		}

		write.write((char*)file.data.data().data(), file.data.size() * sizeof(ui8));

		write.close();

		return true;
	}

	std::string File::read_as_text()
	{
		return std::string((char*)data.data().data());
	}
}