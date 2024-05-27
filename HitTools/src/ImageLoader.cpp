#include "Image/ImageLoader.h"

#include "Core/Types.h"
#include "Core/Log.h"
#include "File/SerialFile.h"
#include "Utils/Serializer.h"

#include "../vendor/stb_image/stb_image.h"

#include <filesystem>

namespace hit
{
	// TODO: add compression to output file
	int image_loader_entry_point(const std::vector<std::string>& options)
	{
		if (options.size() < 2)
		{
			hit_error("Invalid image_loader options!");
			hit_trace("image_loader options: (output_directoy) (list of images files to load)");
			return -1;
		}

		const auto& output_directoy = options[0];
		const auto& files = std::vector(options.begin() + 1, options.end());

		stbi_set_flip_vertically_on_load(1);

		for (auto& file : files)
		{
			i32 image_width;
			i32 image_height;
			i32 image_channels;
			ui8* image = stbi_load(file.c_str(), &image_width, &image_height, &image_channels, 4);

			if (!image)
			{
				hit_error("Failed to load image '{}'.", file);
				return -1;
			}

			std::string output_file_name = output_directoy + "/" + std::filesystem::path(file).stem().string() + ".hit_texture";
			SerialFileWriter writer(output_file_name);

			auto& serializer = writer.get_serializer();

			// serialize loaded image
			serializer.serialize(image_width);
			serializer.serialize(image_height);
			serializer.serialize(image_channels);
			serializer.serialize(image, image_width * image_height * image_channels);

			if (!writer.save())
			{
				stbi_image_free(image);
				hit_error("Failed to save loaded texture file '{}'!", output_file_name);
				return -1;
			}

			stbi_image_free(image);
		}

		return 0;
	}
}