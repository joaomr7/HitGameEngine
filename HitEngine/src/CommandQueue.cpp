#include "Renderer/CommandQueue.h"

#include "Core/Log.h"
#include "Core/Assert.h"

namespace hit
{
	void ShaderCommand::initialize(void* memory)
	{
		new (memory) ShaderCommand(std::move(*this));
	}

	bool ShaderCommand::execute()
	{
		hit_assert(shader, "Can't execute shader command with null shader!");

		switch (action)
		{
			case Action::Bind:
			{
				return shader->bind();
			}
			case Action::Unbind:
			{
				return shader->unbind();
			}
		}
	}

	void ShaderAttributeCommand::initialize(void* memory)
	{
		new (memory) ShaderAttributeCommand(std::move(*this));
	}

	bool ShaderAttributeCommand::execute()
	{
		hit_assert(shader, "Can't execute shader attribute command with null shader!");
		hit_assert(attribute, "Can't execute shader attribute command with null attribute!");

		switch (action)
		{
			case Action::Bind:
			{
				return shader->bind_attribure(attribute);
			}
			case Action::Unbind:
			{
				return shader->unbind_attribure(attribute);
			}
		}
	}

	void ShaderWriteConstantCommand::initialize(void* memory)
	{
		new (memory) ShaderWriteConstantCommand(std::move(*this));
	}

	bool ShaderWriteConstantCommand::execute()
	{
		hit_assert(shader, "Can't execute shader write constant command with null shader!");

		return shader->write_constant(stage, constant_size, constant);
	}

	void BufferDrawCommand::initialize(void* memory)
	{
		new (memory) BufferDrawCommand(std::move(*this));
	}

	bool BufferDrawCommand::execute()
	{
		hit_assert(buffer, "Can't execute buffer draw command with null buffer!");

		return buffer->draw(offset, elem_count, bind_only);
	}

	CommandQueue::CommandQueue()
	{
		const ui64 initial_queue_size = sizeof(ui64) * 1000;
		auto queue_creation_result = m_command_queue.create(initial_queue_size, MemoryUsage::Renderer);

		m_command_count = 0;
	}

	CommandQueue::~CommandQueue()
	{
		ui8* queue = (ui8*)m_command_queue.memory();

		// Free pending commands
		const ui64 type_size = sizeof(ui16);
		for (ui64 i = 0; i < m_command_count; i++)
		{
			ui16 command_size = *(ui16*)queue;
			queue += type_size;

			Command* command = (Command*)queue;

			queue += command_size;

			// free command
			command->~Command();
		}

		m_command_queue.destroy();
	}

	void CommandQueue::add_command(Command&& command)
	{
		m_command_count += 1;
		
		const ui64 type_size = sizeof(ui16);
		const ui16 command_size = command.get_command_size();

		auto size = (ui16*)m_command_queue.push_memory(type_size);
		*size = command_size;
		
		auto memory = m_command_queue.push_memory(command_size);
		command.initialize(memory);
	}

	bool CommandQueue::execute_and_flush()
	{
		ui8* queue = (ui8*)m_command_queue.memory();

		const ui64 type_size = sizeof(ui16);
		for (ui64 i = 0; i < m_command_count; i++)
		{
			ui16 command_size = *(ui16*)queue;
			queue += type_size;

			Command* command = (Command*)queue;

			queue += command_size;

			if (!command->execute()) [[unlikely]]
			{
				return false;
			}

			// free command
			command->~Command();
		}

		// flush queue
		m_command_count = 0;
		m_command_queue.reset();

		return true;
	}
}