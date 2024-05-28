#pragma once

#include "Core/Types.h"
#include "Utils/Ref.h"
#include "Utils/Arena.h"

#include "Shader.h"
#include "Buffer.h"

namespace hit
{
	struct Command
	{
		inline Command() = default;
		inline Command(const Command&) = default;
		inline Command(Command&&) noexcept = default;

		Command& operator=(const Command&) = default;
		Command& operator=(Command&&) noexcept = default;

		virtual ~Command() = default;

		virtual ui16 get_command_size() const = 0;

		virtual void initialize(void* memory) = 0;

		virtual bool execute() = 0;
	};

	struct ShaderCommand : public Command
	{
		enum class Action { Bind, Unbind };

		Ref<Shader> shader;
		Action action;

		inline ShaderCommand(const Ref<Shader>& shader = nullptr, Action action = Action::Bind)
			: shader(shader), action(action) { }

		inline ShaderCommand(const ShaderCommand&) = default;
		inline ShaderCommand(ShaderCommand&&) noexcept = default;

		ShaderCommand& operator=(const ShaderCommand&) = default;
		ShaderCommand& operator=(ShaderCommand&&) noexcept = default;

		inline constexpr ui16 get_command_size() const override { return sizeof(ShaderCommand); }

		void initialize(void* memory) override;

		bool execute() override;
	};

	struct ShaderAttributeCommand : public Command
	{
		enum class Action { Bind, Unbind };

		Ref<Shader> shader;
		Ref<ShaderAttribute> attribute;
		Action action;

		inline ShaderAttributeCommand(
			const Ref<Shader>& shader = nullptr,
			Ref<ShaderAttribute> attribute = nullptr,
			Action action = Action::Bind)
			: shader(shader), attribute(attribute), action(action) { }

		inline ShaderAttributeCommand(const ShaderAttributeCommand&) = default;
		inline ShaderAttributeCommand(ShaderAttributeCommand&&) noexcept = default;

		ShaderAttributeCommand& operator=(const ShaderAttributeCommand&) = default;
		ShaderAttributeCommand& operator=(ShaderAttributeCommand&&) noexcept = default;

		inline constexpr ui16 get_command_size() const override { return sizeof(ShaderAttributeCommand); }

		void initialize(void* memory) override;

		bool execute() override;
	};

	struct ShaderWriteConstantCommand : public Command
	{
		Ref<Shader> shader;

		void* constant;
		ui64 constant_size;

		ShaderProgram::Type stage;

		inline ShaderWriteConstantCommand(
			const Ref<Shader>& shader = nullptr,
			ShaderProgram::Type stage = ShaderProgram::Vertex,
			ui64 constant_size = 0,
			void* constant = nullptr)
			: shader(shader), stage(stage), constant_size(constant_size), constant(constant) { }

		inline ShaderWriteConstantCommand(const ShaderWriteConstantCommand&) = default;
		inline ShaderWriteConstantCommand(ShaderWriteConstantCommand&&) noexcept = default;

		ShaderWriteConstantCommand& operator=(const ShaderWriteConstantCommand&) = default;
		ShaderWriteConstantCommand& operator=(ShaderWriteConstantCommand&&) noexcept = default;

		inline constexpr ui16 get_command_size() const override { return sizeof(ShaderWriteConstantCommand); }

		void initialize(void* memory) override;

		bool execute() override;
	};

	struct BufferDrawCommand : public Command
	{
		Ref<Buffer> buffer;

		ui64 offset;
		ui32 elem_count;
		bool bind_only;

		inline BufferDrawCommand(
			const Ref<Buffer>& buffer = nullptr,
			ui64 offset = 0,
			ui32 elem_count = 0,
			bool bind_only = false)
			: buffer(buffer), offset(offset), elem_count(elem_count), bind_only(bind_only) { }

		inline BufferDrawCommand(const BufferDrawCommand&) = default;
		inline BufferDrawCommand(BufferDrawCommand&&) noexcept = default;

		BufferDrawCommand& operator=(const BufferDrawCommand&) = default;
		BufferDrawCommand& operator=(BufferDrawCommand&&) noexcept = default;

		inline constexpr ui16 get_command_size() const override { return sizeof(BufferDrawCommand); }

		void initialize(void* memory) override;

		bool execute() override;
	};

	// TODO:
	// It flush reset queue at each execution, because it's dynamic
	// To avoid this behavior create a static command queue version
	class CommandQueue
	{
	public:
		CommandQueue();
		~CommandQueue();

		void add_command(Command&& command);

		bool execute_and_flush();

	private:
		ui64 m_command_count;
		Arena m_command_queue;
	};
}