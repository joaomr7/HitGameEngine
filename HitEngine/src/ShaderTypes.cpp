#include "Renderer/ShaderTypes.h"

#include "Utils/Serializer.h"
#include "Utils/Deserializer.h"

namespace hit
{
	bool ShaderUniform::serialize(Serializer& serializer)
	{
		serializer.serialize(type);

		if(!layout.serialize(serializer))
		{
			return false;
		}

		serializer.serialize(name);

		return true;
	}

	bool ShaderUniform::deserialize(Deserializer& deserializer)
	{
		if(!deserializer.try_deserialize(&type))
		{
			hit_error("Failed to deserealize uniform type.");
			return false;
		}

		if(!layout.deserialize(deserializer))
		{
			hit_error("Failed to deserealize uniform layout.");
			return false;
		}

		if(!deserializer.try_deserialize_string(&name))
		{
			hit_error("Failed to deserealize uniform name.");
			return false;
		}

		return true;
	}

	bool ShaderProgram::serialize(Serializer& serializer)
	{
		// serialize type
		serializer.serialize(type);

		// serialize attributes
		if(!attributes.serialize(serializer))
		{
			hit_error("Failed to serealize shader attributes!");
			return false;
		}

		// serialize max uniforms
		serializer.serialize(max_uniforms);

		// serialize uniforms
		serializer.serialize((ui64)uniforms.size());
		for(auto& uniform : uniforms)
		{
			if(!uniform.serialize(serializer))
			{
				hit_error("Failed to serialize shader uniform!");
				return false;
			}
		}

		// serialize compiled code
		serializer.serialize((ui64)source.size());
		serializer.serialize(source.data(), source.size());

		return true;
	}

	bool ShaderProgram::deserialize(Deserializer& deserializer)
	{
		if(!deserializer.try_deserialize(&type))
		{
			hit_error("Failed to deserealize shader type!");
			return false;
		}

		if(!attributes.deserialize(deserializer))
		{
			return false;
		}

		if (!deserializer.try_deserialize(&max_uniforms))
		{
			hit_error("Failed to deserealize max uniforms.");
			return false;
		}

		ui64 uniforms_count = 0;
		if(!deserializer.try_deserialize(&uniforms_count))
		{
			hit_error("Failed to deserealize uniforms.");
			return false;
		}

		uniforms.resize(uniforms_count);
		for(auto& uniform : uniforms)
		{
			if(!uniform.deserialize(deserializer))
			{
				return false;
			}
		}

		ui64 source_size = 0;
		if(!deserializer.try_deserialize(&source_size))
		{
			hit_error("Failed to deserealize shader source size.");
			return false;
		}

		source.resize(source_size);
		if(!deserializer.try_deserialize_array(source.data(), source_size))
		{
			hit_error("Failed to deserealize shader source.");
			return false;
		}

		return true;
	}
}