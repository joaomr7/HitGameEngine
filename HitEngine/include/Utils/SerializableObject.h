#pragma once

#include "Serializer.h"
#include "Deserializer.h"

#include <vector>

namespace hit
{
	struct SerializableObject
	{
		virtual ~SerializableObject() = default;

		virtual bool serialize(Serializer& serializer) { return false; }
		virtual bool deserialize(Deserializer& deserializer) { return false; }
	};

	bool serialize_object(SerializableObject* object, SerialBuffer* out_buffer);

	bool deserialize_object(SerializableObject* object, const SerialBuffer& buffer);

	bool serialize_objects(const std::vector<SerializableObject*>& objects, SerialBuffer* out_buffer);

	bool deserialize_objects(const std::vector<SerializableObject*>& objects, const SerialBuffer& buffer);
}