#include "Renderer/Buffer.h"

#include "Utils/Serializer.h"
#include "Utils/Deserializer.h"

namespace hit
{
	ui64 ShaderData::size() const
	{
		switch(type)
		{
			case Bool:
			return 1;

			case Int:
			case Float:
			return 4;

			case Int2:
			case Float2:
			return 8;

			case Int3:
			case Float3:
			return 12;

			case Int4:
			case Float4:
			case Mat2:
			return 16;

			case Mat3: return 36;

			case Mat4: return 64;
		}

		return 0;
	}

	bool BufferData::serialize(Serializer& serializer)
	{
		serializer.serialize(size);
		serializer.serialize(offset);
		serializer.serialize(type);
		serializer.serialize(name);

		return true;
	}

	bool BufferData::deserialize(Deserializer& deserializer)
	{
		if(!deserializer.try_deserialize(&size))
		{
			hit_error("Failed to deserealize BufferData size.");
			return false;
		}

		if(!deserializer.try_deserialize(&offset))
		{
			hit_error("Failed to deserealize BufferData offset.");
			return false;
		}

		if(!deserializer.try_deserialize(&type))
		{
			hit_error("Failed to deserealize BufferData data type.");
			return false;
		}

		if(!deserializer.try_deserialize_string(&name))
		{
			hit_error("Failed to deserealize BufferData name.");
			return false;
		}

		return true;
	}

	bool BufferLayout::serialize(Serializer& serializer)
	{
		serializer.serialize((ui64)m_layout.size());

		for(auto& layout : m_layout)
		{
			if(!layout.serialize(serializer))
			{
				return false;
			}
		}

		serializer.serialize(m_total_size);

		return true;
	}

	bool BufferLayout::deserialize(Deserializer& deserializer)
	{
		ui64 layout_count = 0;
		if(!deserializer.try_deserialize(&layout_count))
		{
			hit_error("Failed to deserealize BufferLayout layout elements count.");
			return false;
		}

		m_layout.resize(layout_count);

		for(auto& layout : m_layout)
		{
			if(!layout.deserialize(deserializer))
			{
				return false;
			}
		}

		if(!deserializer.try_deserialize(&m_total_size))
		{
			hit_error("Failed to deserealize BufferLayout total size.");
			return false;
		}

		return true;
	}

	BufferLayout::BufferLayout(const std::vector<std::pair<ShaderData, std::string>>& data)
	{
		ui64 offset = 0;
		for(auto& [shader_data, name] : data)
		{
			BufferData buffer_data;
			buffer_data.size = shader_data.size();
			buffer_data.offset = offset;
			buffer_data.type = shader_data.type;
			buffer_data.name = name;

			offset += buffer_data.size;

			m_layout.push_back(buffer_data);
		}

		m_total_size = offset;
	}

	BufferLayout::BufferLayout(const std::initializer_list<std::pair<ShaderData, std::string>>& data)
	{ 
		ui64 offset = 0;
		for(auto& [shader_data, name] : data)
		{
			BufferData buffer_data;
			buffer_data.size = shader_data.size();
			buffer_data.offset = offset;
			buffer_data.type = shader_data.type;
			buffer_data.name = name;

			offset += buffer_data.size;

			m_layout.push_back(buffer_data);
		}

		m_total_size = offset;
	}

	bool BufferLayout::has_data(const std::string& name)
	{
		for (auto& data : m_layout)
		{
			if (data.name == name)
			{
				return true;
			}
		}

		return false;
	}

	BufferData BufferLayout::get_data(const std::string& name)
	{
		for (auto& data : m_layout)
		{
			if (data.name == name)
			{
				return data;
			}
		}

		return BufferData();
	}
}