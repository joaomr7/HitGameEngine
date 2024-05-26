#include "VulkanBuffer.h"

#include "VulkanRenderer.h"
#include "VulkanCommand.h"

#include "Core/Memory.h"
#include "Core/Log.h"

namespace hit
{
    VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
    {
        m_type = other.m_type;
        m_allocation_type = other.m_allocation_type;

        m_total_size = other.m_total_size;
        m_freelist = std::move(other.m_freelist);

        m_is_locked = other.m_is_locked;

        m_context = other.m_context;

        m_buffer = other.m_buffer;
        other.m_buffer = nullptr;

        m_memory = other.m_memory;
        other.m_memory = nullptr;

        m_memory_requirements = other.m_memory_requirements;
        m_memory_flags = other.m_memory_flags;
        m_memory_type = other.m_memory_type;
        m_buffer_flags = other.m_buffer_flags;
    }

    VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer && other) noexcept
    {
        m_type = other.m_type;
        m_allocation_type = other.m_allocation_type;

        m_total_size = other.m_total_size;
        m_freelist = std::move(other.m_freelist);

        m_is_locked = other.m_is_locked;

        m_context = other.m_context;

        m_buffer = other.m_buffer;
        other.m_buffer = nullptr;

        m_memory = other.m_memory;
        other.m_memory = nullptr;

        m_memory_requirements = other.m_memory_requirements;
        m_memory_flags = other.m_memory_flags;
        m_memory_type = other.m_memory_type;
        m_buffer_flags = other.m_buffer_flags;

        return *this;
    }

    bool VulkanBuffer::create(ui64 initial_size, BufferType type, BufferAllocationType allocation)
    {
        auto device = m_context->get_device();

        // handle buffer type
        m_type = type;
        switch (m_type)
        {
            case BufferType::Vertex:
            {
                m_buffer_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                m_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;
            }

            case BufferType::Index:
            {
                m_buffer_flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                m_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;
            }

            case BufferType::Uniform:
            {
                m_buffer_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;
            }

            case BufferType::Staging:
            {
                m_buffer_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
            }

            case BufferType::Read:
            {
                m_buffer_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;;
                break;
            }

            case BufferType::Storage:
            default:
            {
                hit_error("Unsupported buffer type.");
                return false;
            }
        }

        // create buffer
        VkBufferCreateInfo buffer_info { };
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = initial_size;
        buffer_info.usage = m_buffer_flags;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(!check_vk_result(vkCreateBuffer(device->get_device(), &buffer_info, device->get_alloc_callback(), &m_buffer)))
        {
            hit_error("Failed to create buffer.");
            return false;
        }

        // create buffer memory
        // get mem requirements
        vkGetBufferMemoryRequirements(device->get_device(), m_buffer, &m_memory_requirements);
        m_memory_type = device->get_memory_type(m_memory_requirements.memoryTypeBits, m_memory_flags);

        if (m_memory_type == -1)
        {
            hit_error("Failed to create buffer memory, because the required memory type wasn't find.");
            return false;
        }

        // update size
        m_total_size = m_memory_requirements.size;
        m_is_locked = true;

        VkMemoryAllocateInfo memory_info { };
        memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_info.allocationSize = m_total_size;
        memory_info.memoryTypeIndex = (ui32)m_memory_type;

        // allocate memory
        if (!check_vk_result(vkAllocateMemory(device->get_device(), &memory_info, device->get_alloc_callback(), &m_memory)))
        {
            hit_error("Failed to allocate buffer memory.");
            return false;
        }

        // handle allocation type
        m_allocation_type = allocation;
        switch (m_allocation_type)
        {
            case BufferAllocationType::FreeList:
            {
                m_freelist = create_scope<FreelistCore>(m_total_size);
                break;
            }
        }

        return true;
    }

    void VulkanBuffer::destroy()
    {
        auto device = m_context->get_device();

        device->wait_idle();

        if (m_memory)
        {
            vkFreeMemory(device->get_device(), m_memory, device->get_alloc_callback());
            m_memory = VK_NULL_HANDLE;
        }

        if (m_buffer)
        {
            vkDestroyBuffer(device->get_device(), m_buffer, device->get_alloc_callback());
            m_buffer = VK_NULL_HANDLE;
        }

        if (m_freelist)
        {
            m_freelist = nullptr;
        }
    }

    bool VulkanBuffer::bind(ui64 offset)
    {
        if (m_is_bind) return true;

        auto device = m_context->get_device();

        if (!check_vk_result(vkBindBufferMemory(device->get_device(), m_buffer, m_memory, offset)))
        {
            hit_error("Can't bind buffer at offset {}", offset);
            return false;
        }

        m_is_bind = true;

        return true;
    }

    bool VulkanBuffer::unbind()
    {
        return true;
    }

    bool VulkanBuffer::resize(ui64 new_size)
    {
        if (m_freelist && !m_freelist->resize(new_size))
        {
            hit_error("Can't resize buffer freelist.");
            return false;
        }

        auto new_buffer = VulkanBuffer(m_context);
        if (!new_buffer.create(new_size, m_type, BufferAllocationType::None))
        {
            hit_error("Can't resize buffer to {} bytes", new_size);
            return false;
        }

        if (!new_buffer.copy(*this, std::min(m_total_size, new_size)))
        {
            new_buffer.destroy();

            hit_error("Failed to copy old buffer to resized buffer!");
            return false;
        }

        // save current freelist
        auto freelist_backup = std::move(m_freelist);

        // destroy current buffer
        m_context->get_device()->wait_idle();
        destroy();

        // move new buffer to old buffer
        *this = std::move(new_buffer);
        m_freelist = std::move(freelist_backup);

        return true;
    }

    void* VulkanBuffer::map_memory(ui64 offset, ui64 size)
    {
        auto device = m_context->get_device();
        void* data;

        if (!check_vk_result(vkMapMemory(device->get_device(), m_memory, offset, size, 0, &data)))
        {
            hit_error("Failed to map buffer memory!");
            return nullptr;
        }

        return data;
    }

    void VulkanBuffer::unmap_memory(ui64 offset, ui64 size)
    { 
        auto device = m_context->get_device();
        vkUnmapMemory(device->get_device(), m_memory);
    }

    bool VulkanBuffer::flush(ui64 offset, ui64 size)
    {
        if (!is_host_coeherent())
        {
            auto device = m_context->get_device();

            VkMappedMemoryRange range;
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = m_memory;
            range.offset = offset;
            range.size = size;

            if (!check_vk_result(vkFlushMappedMemoryRanges(device->get_device(), 1, &range)))
            {
                hit_error("Failed to flush buffer.");
                return false;
            }
        }

        return true;
    }

    bool VulkanBuffer::read(ui64 offset, ui64 size, void** out_memory)
    {
        if (!out_memory || !*out_memory)
        {
            hit_fatal("Can't read buffer to a null memory!");
            return false;
        }

        if (is_device_local() && !is_host_visible())
        {
            VulkanBuffer staging(m_context);
            if (!staging.create(size, BufferType::Staging, BufferAllocationType::None))
            {
                hit_error("Failed to create staging buffer.");
                return false;
            }

            staging.bind();
            if (!staging.copy(*this, size, 0, offset))
            {
                staging.destroy();

                hit_error("Failed to copy buffer.");
                return false;
            }

            // map and copy buffer memory
            auto mapped_memory = staging.map_memory(0, size);

            if (!mapped_memory)
            {
                staging.destroy();

                hit_error("Failed to map buffer memory.");
                return false;
            }

            Memory::copy_memory((ui8*)*out_memory, (ui8*)mapped_memory, size);

            // unmap and destroy staging buffer
            staging.unmap_memory(0, size);
            staging.unbind();
            staging.destroy();
        }
        else
        {
            // map and copy buffer memory
            auto mapped_memory = map_memory(0, size);
            if (!mapped_memory)
            {
                hit_error("Failed to map buffer memory.");
                return false;
            }

            Memory::copy_memory((ui8*)*out_memory, (ui8*)mapped_memory, size);

            // unmap and destroy staging buffer
            unmap_memory(0, size);
        }

        return true;
    }

    bool VulkanBuffer::load(ui64 offset, ui64 size, void* data)
    {
        if (!data || !size)
        {
            hit_fatal("Can't load a null data to buffer.");
            return false;
        }

        if (is_device_local() && !is_host_visible())
        {
            VulkanBuffer staging(m_context);
            if (!staging.create(size, BufferType::Staging, BufferAllocationType::None))
            {
                hit_error("Failed to create staging buffer.");
                return false;
            }

            staging.bind();

            if (!staging.load(0, size, data))
            {
                staging.destroy();
                return false;
            }

            if (!copy(staging, size, offset, 0))
            {
                staging.destroy();

                hit_error("Failed to copy staging buffer.");
                return false;
            }

            staging.destroy();
        }
        else
        {
            auto mem_data = map_memory(offset, size);
            if (!mem_data)
            {
                hit_error("Failed to map buffer memory.");
                return false;
            }

            Memory::copy_memory((ui8*)mem_data, (ui8*)data, size);
            unmap_memory(offset, size);
        }

        return true;
    }

    bool VulkanBuffer::draw(ui64 offset, ui32 elem_count, bool bind_only)
    {
        auto& working_command = m_context->get_graphics_command();

        if (is_vertex())
        {
            vkCmdBindVertexBuffers(working_command.get_command_buffer(), 0, 1, &m_buffer, &offset);

            if (!bind_only)
            {
                vkCmdDraw(working_command.get_command_buffer(), elem_count, 1, 0, 0);
            }
        }
        else if(is_index())
        {
            vkCmdBindIndexBuffer(working_command.get_command_buffer(), m_buffer, offset, VK_INDEX_TYPE_UINT32);
        
            if (!bind_only)
            {
                vkCmdDrawIndexed(working_command.get_command_buffer(), elem_count, 1, 0, 0, 0);
            }
        }
        else
        {
            hit_fatal("Attempting to draw invalid buffer type.");
            return false;
        }

        return true;
    }

    bool VulkanBuffer::copy(Buffer& other, ui64 size, ui64 dest_offset, ui64 src_offset)
    {
        // TODO: check transfer queue support, and use it
        // TODO: add frame queue support

        auto device = m_context->get_device();
        device->wait_graphics_queue();

        auto other_buffer = ((VulkanBuffer*)&other)->m_buffer;

        if (!bind())
        {
            hit_error("Failed to bind destination buffer.");
            return false;
        }

        if (!run_single_graphics_command((VulkanDevice*)device,
            [&](VkCommandBuffer command)
        {
            VkBufferCopy copy_region;
            copy_region.dstOffset = dest_offset;
            copy_region.srcOffset = src_offset;
            copy_region.size = size;

            vkCmdCopyBuffer(command, other_buffer, m_buffer, 1, &copy_region);
        }))
        {
            hit_error("Failed to run graphics command.");
            return false;
        }

        unbind();

        return true;
    }

    bool VulkanBuffer::is_device_local()
    {
        return m_memory_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    bool VulkanBuffer::is_host_visible()
    {
        return m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    bool VulkanBuffer::is_host_coeherent()
    {
        return m_memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
}