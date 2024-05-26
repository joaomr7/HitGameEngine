#pragma once

#include "Core/Types.h"
#include "Utils/Serializer.h"
#include "Utils/Deserializer.h"

#include <string>
#include <string_view>

namespace hit
{
	// TODO: improve this API
	class SerialFileReader
	{
	public:
		SerialFileReader(std::string_view filename);

		Deserializer& get_deserializer();

	private:
		Deserializer m_deserializer;
	};

	class SerialFileWriter
	{
	public:
		SerialFileWriter(std::string_view filename);

		Serializer& get_serializer();

		bool save();

	private:
		Serializer m_serializer;
		std::string m_output_filename;
	};
}