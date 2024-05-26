#pragma once

#include "Core/Types.h"
#include "Core/Memory.h"
#include "FastTypedArena.h"

#include <string>
#include <vector>

namespace hit
{
	using SerialBuffer = FastTypedArena<ui8>;

	class Serializer
	{
	public:
		Serializer();
		Serializer(const SerialBuffer& buffer);
		Serializer(const Serializer& other);
		Serializer(Serializer&& other) noexcept;

		Serializer& operator=(const Serializer& other);
		Serializer& operator=(Serializer&& other) noexcept;

		template<typename T>
		void serialize(const T& data);
		template<typename T>
		void serialize(const T* data, ui64 count);
		template<typename T>
		void serialize(const std::vector<T>& vec);

		void reset();

		ui64 get_total_size() const;
		const ui8* get_serialized_data() const;

		const SerialBuffer& get_serial_buffer() const;

	private:
		SerialBuffer m_buffer;
	};

	inline Serializer::Serializer() : m_buffer(sizeof(ui8)) {}

	inline Serializer::Serializer(const SerialBuffer& buffer) : m_buffer(buffer) { }

	inline Serializer::Serializer(const Serializer& other) : m_buffer(other.m_buffer) { }

	inline Serializer::Serializer(Serializer&& other) noexcept : m_buffer(std::move(other.m_buffer)) { }

	inline Serializer& Serializer::operator=(const Serializer& other)
	{
		m_buffer = other.m_buffer;
		return *this;
	}

	inline Serializer& Serializer::operator=(Serializer&& other) noexcept
	{
		m_buffer = std::move(other.m_buffer);
		return *this;
	}

	inline void Serializer::reset()
	{
		m_buffer.clear();
	}

	inline ui64 Serializer::get_total_size() const
	{
		return m_buffer.size() * sizeof(ui8);
	}

	inline const ui8* Serializer::get_serialized_data() const
	{
		return m_buffer.data().data();
	}

	inline const SerialBuffer& Serializer::get_serial_buffer() const
	{
		return m_buffer;
	}

	template<typename T>
	void Serializer::serialize(const T& data)
	{
		const ui64 data_size = sizeof(data);
		auto buffer_location = m_buffer.push_array(data_size);

		Memory::copy_memory(buffer_location, (ui8*)&data, data_size);
	}

	template<>
	inline void Serializer::serialize(const std::string& data)
	{
		const ui64 data_size = data.length() * sizeof(char);
		serialize(data_size);

		auto buffer_location = m_buffer.push_array(data_size);
		Memory::copy_memory(buffer_location, (ui8*)data.c_str(), data_size);
	}

	template<typename T>
	void Serializer::serialize(const T* data, ui64 count)
	{
		const ui64 data_size = count * sizeof(T);
		auto buffer_location = m_buffer.push_array(data_size);
		Memory::copy_memory(buffer_location, (ui8*)data, data_size);
	}

	template<typename T>
	void Serializer::serialize(const std::vector<T>& vec)
	{
		serialize(vec.data(), vec.size());
	}
}
