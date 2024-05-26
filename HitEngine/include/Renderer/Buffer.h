#pragma once

#include "Core/Types.h"
#include "Utils/SerializableObject.h"
#include "Utils/Freelist.h"
#include "Utils/Ref.h"

#include <vector>
#include <initializer_list>
#include <string>

namespace hit
{
    struct ShaderData
    {
        enum Type
        {
            None,
            Bool,
            Int, Int2, Int3, Int4,
            Float, Float2, Float3, Float4,
            Mat2, Mat3, Mat4
        };

        ui64 size() const;

        Type type = None;
    };

    struct BufferData : public SerializableObject
    {
        ui64 size;
        ui64 offset;
        ShaderData::Type type;

        std::string name;

        bool serialize(Serializer& serializer) override;
        bool deserialize(Deserializer& deserializer) override;
    };

    class BufferLayout : public SerializableObject
    {
    public:
        BufferLayout() = default;
        BufferLayout(const std::vector<std::pair<ShaderData, std::string>>& data);
        BufferLayout(const std::initializer_list<std::pair<ShaderData, std::string>>& data);

        bool has_data(const std::string& name);
        BufferData get_data(const std::string& name);

        inline ui64 elem_count() const { return m_layout.size(); }
        inline ui64 stride() const { return m_total_size; }

        inline std::vector<BufferData>::iterator begin() { return m_layout.begin(); }
        inline std::vector<BufferData>::iterator end() { return m_layout.end(); }

        inline std::vector<BufferData>::const_iterator begin() const { return m_layout.begin(); }
        inline std::vector<BufferData>::const_iterator end() const { return m_layout.end(); }

        inline const BufferData& get(ui64 index) const { return m_layout[index]; }
        inline const BufferData& operator[](ui64 index) const { return m_layout[index]; }

        bool serialize(Serializer& serializer) override;
        bool deserialize(Deserializer& deserializer) override;

    private:
        std::vector<BufferData> m_layout;
        ui64 m_total_size = 0;
    };

    enum class BufferType : ui8
    {
        Unknown,
        Vertex,
        Index,
        Uniform,
        Staging,
        Read,
        Storage
    };

    enum class BufferAllocationType : ui8
    {
        None,
        FreeList
    };

    class Buffer
    {
    public:
        virtual bool create(ui64 initial_size, BufferType type, BufferAllocationType allocation) = 0;
        virtual void destroy() = 0;

        virtual bool bind(ui64 offset = 0) = 0;
        virtual bool unbind() = 0;

        virtual bool resize(ui64 new_size) = 0;

        virtual void* map_memory(ui64 offset, ui64 size) = 0;
        virtual void unmap_memory(ui64 offset, ui64 size) = 0;

        virtual bool flush(ui64 offset, ui64 size) = 0;

        virtual bool read(ui64 offset, ui64 size, void** out_memory) = 0;
        virtual bool load(ui64 offset, ui64 size, void* data) = 0;

        virtual bool draw(ui64 offset, ui32 elem_count, bool bind_only) = 0;

        virtual bool copy(Buffer& other, ui64 size, ui64 dest_offset = 0, ui64 src_offset = 0) = 0;

        inline ui64 get_total_size() { return m_total_size; }

        inline bool is_valid() { return m_type != BufferType::Unknown; }
        inline bool is_vertex() { return m_type == BufferType::Vertex; }
        inline bool is_index() { return m_type == BufferType::Index; }
        inline bool is_uniform() { return m_type == BufferType::Uniform; }
        inline bool is_staging() { return m_type == BufferType::Staging; }
        inline bool is_read() { return m_type == BufferType::Read; }
        inline bool is_storage() { return m_type == BufferType::Storage; }

        inline bool use_freelist() { return m_allocation_type == BufferAllocationType::FreeList; }

    public:
        Buffer() = default;
        virtual ~Buffer() = default;

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&&) noexcept = default;
        Buffer& operator=(Buffer&&) noexcept = default;

    protected:
        BufferType m_type;
        BufferAllocationType m_allocation_type;

        ui64 m_total_size;
        Scope<FreelistCore> m_freelist = nullptr;

        bool m_is_locked;
    };
}