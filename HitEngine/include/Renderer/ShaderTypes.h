#pragma once

#include "Core/Types.h"
#include "Utils/SerializableObject.h"
#include "Buffer.h"

#include <string>

namespace hit
{
    struct ShaderUniform : public SerializableObject
    {
        enum Type
        {
            PushConstant, ImageSampler, UniformBuffer
        };

        Type type;
        BufferLayout layout;

        std::string name;

        inline bool is_push_constant() const { return type == PushConstant; }
        inline bool is_image_sampler() const { return type == ImageSampler; }
        inline bool is_uniform_buffer() const { return type == UniformBuffer; }

        bool serialize(Serializer& serializer) override;
        bool deserialize(Deserializer& deserializer) override;
    };

    struct ShaderProgram : public SerializableObject
    {
        enum Type
        {
            Vertex,
            Fragment
        };

        Type type;
        std::vector<ui32> source;
        BufferLayout attributes;

        ui64 max_uniforms;
        std::vector<ShaderUniform> uniforms;

        bool serialize(Serializer& serializer) override;
        bool deserialize(Deserializer& deserializer) override;
    };
}