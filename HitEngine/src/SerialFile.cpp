#include "File/SerialFile.h"
#include "File/File.h"

namespace hit
{
	SerialFileReader::SerialFileReader(std::string_view filename)
	{ 
		File file;
		if(!file_read(filename, File::Binary, file))
		{
			return;
		}

		m_deserializer = Deserializer(file.data);
	}

	Deserializer& SerialFileReader::get_deserializer()
	{
		return m_deserializer;
	}

	SerialFileWriter::SerialFileWriter(std::string_view filename) : m_output_filename(filename) { }

	Serializer& SerialFileWriter::get_serializer()
	{
		return m_serializer;
	}

	bool SerialFileWriter::save()
	{
		File file;
		file.type = File::Binary;
		file.data = m_serializer.get_serial_buffer();

		return file_save(m_output_filename, file);
	}
}