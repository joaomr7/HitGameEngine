#include "Utils/SerializableObject.h"

#include "Utils/Serializer.h"
#include "Utils/Deserializer.h"

namespace hit
{
	bool serialize_object(SerializableObject* object, SerialBuffer* out_buffer)
	{
		if(!object)
		{
			hit_error("Serializable object is nullptr!");
			return false;
		}

		if(!out_buffer)
		{
			hit_error("Serial buffer is nullptr!");
			return false;
		}

		Serializer serializer(*out_buffer);

		if(!object->serialize(serializer))
		{
			return false;
		}

		*out_buffer = serializer.get_serial_buffer();

		return true;
	}

	bool deserialize_object(SerializableObject* object, const SerialBuffer& buffer)
	{
		if(!object)
		{
			hit_error("Serializable object is nullptr!");
			return false;
		}

		Deserializer deserializer(buffer);

		return object->deserialize(deserializer);
	}

	bool serialize_objects(const std::vector<SerializableObject*>& objects, SerialBuffer* out_buffer)
	{
		if(!out_buffer)
		{
			hit_error("Serial buffer is nullptr!");
			return false;
		}

		Serializer serializer(*out_buffer);

		for(auto& object : objects)
		{
			if(!object)
			{
				hit_error("Serializable object is nullptr!");
				return false;
			}

			if(!object->serialize(serializer))
			{
				return false;
			}
		}

		*out_buffer = serializer.get_serial_buffer();

		return true;
	}

	bool deserialize_objects(const std::vector<SerializableObject*>& objects, const SerialBuffer& buffer)
	{
		Deserializer deserializer(buffer);

		for(auto& object : objects)
		{
			if(!object)
			{
				hit_error("Serializable object is nullptr!");
				return false;
			}

			if(!object->deserialize(deserializer))
			{
				return false;
			}
		}

		return true;
	}
}