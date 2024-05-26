#pragma once

#include "Core/Types.h"
#include "Core/Log.h"
#include "Serializer.h"

#include <vector>

namespace hit
{
	class Deserializer
	{
	public:
		Deserializer() = default;
		Deserializer(const SerialBuffer& buffer);
		Deserializer(const Serializer& serializer);
		Deserializer(const Deserializer& other);
		Deserializer(Deserializer&& other) noexcept;

		Deserializer& operator=(const Serializer& serializer);
		Deserializer& operator=(const Deserializer& other);
		Deserializer& operator=(Deserializer&& other) noexcept;

		bool go_to(ui64 position);
		ui64 get_left_size() const;
		ui64 get_current_position() const;

		template<typename T>
		bool try_deserialize(T* out_data);
		template<typename T>
		bool try_deserialize_array(T* out_array, ui64 count);
		bool try_deserialize_string(std::string* str);

		template<typename T>
		T deserialize();
		template<typename T>
		std::vector<T> deserialize_array(ui64 count);
		std::string deserialize_string();

		ui64 get_total_size() const;
		const ui8* get_serialized_data() const;

		const SerialBuffer& get_serial_buffer() const;

	private:
		SerialBuffer m_buffer;
		ui64 m_current_position;
	};

	inline Deserializer::Deserializer(const SerialBuffer& buffer) : m_buffer(buffer), m_current_position(0) { }

	inline Deserializer::Deserializer(const Serializer& serializer) : m_buffer(serializer.get_serial_buffer()), m_current_position(0) { }

	inline Deserializer::Deserializer(const Deserializer& other) : m_buffer(other.m_buffer), m_current_position(other.m_current_position) { }

	inline Deserializer::Deserializer(Deserializer&& other) noexcept : m_buffer(std::move(other.m_buffer)), m_current_position(other.m_current_position) { }

	inline Deserializer& Deserializer::operator=(const Serializer& serializer)
	{
		m_buffer = serializer.get_serial_buffer();
		m_current_position = 0;
		return *this;
	}

	inline Deserializer& Deserializer::operator=(const Deserializer& other)
	{
		m_buffer = other.m_buffer;
		m_current_position = other.m_current_position;
		return *this;
	}

	inline Deserializer& Deserializer::operator=(Deserializer&& other) noexcept
	{
		m_buffer = std::move(other.m_buffer);
		m_current_position = other.m_current_position;
		return *this;
	}

	inline bool Deserializer::go_to(ui64 position)
	{
		if(position >= m_buffer.size())
		{
			return false;
		}

		m_current_position = position;
		return true;
	}

	inline ui64 Deserializer::get_left_size() const
	{
		return m_buffer.size() - m_current_position;
	}

	inline ui64 Deserializer::get_current_position() const
	{
		return m_current_position;
	}

	inline ui64 Deserializer::get_total_size() const
	{
		return m_buffer.size() * sizeof(ui8);
	}

	inline const ui8* Deserializer::get_serialized_data() const
	{
		return m_buffer.data().data();
	}

	inline const SerialBuffer& Deserializer::get_serial_buffer() const
	{
		return m_buffer;
	}

	template<typename T>
	bool Deserializer::try_deserialize(T* out_data)
	{
		if(!out_data)
		{
			hit_error("Deserializing nullptr!");
			return false;
		}

		const ui64 data_size = sizeof(T);
		const ui64 left_size = get_left_size();

		if(data_size > left_size)
		{
			return false;
		}

		Memory::copy_memory((ui8*)out_data, m_buffer[m_current_position], data_size);

		m_current_position += data_size;

		return true;
	}

	template<typename T>
	bool Deserializer::try_deserialize_array(T* out_array, ui64 count)
	{
		if(!out_array)
		{
			hit_error("Deserializing nullptr!");
			return false;
		}

		const ui64 array_size = count * sizeof(T);
		const ui64 left_size = get_left_size();

		if(array_size > left_size)
		{
			return false;
		}

		Memory::copy_memory((ui8*)out_array, m_buffer[m_current_position], array_size);

		m_current_position += array_size;

		return true;
	}

	inline bool Deserializer::try_deserialize_string(std::string* str)
	{
		if(!str)
		{
			hit_error("Deserializing nullptr string!");
			return false;
		}

		ui64 str_length = 0;
		if(!try_deserialize(&str_length))
		{
			return false;
		}

		const ui64 left_size = get_left_size();
		if(str_length > left_size)
		{
			return false;
		}

		str->resize(str_length);
		Memory::copy_memory((ui8*)str->data(), m_buffer[m_current_position], str_length);

		m_current_position += str_length;

		return true;
	}

	template<typename T>
	T Deserializer::deserialize()
	{
		T out_data;

		if(!try_deserialize(&out_data))
		{
			hit_assert(false, "Failed to deserialize data!");
		}

		return out_data;
	}

	template<typename T>
	std::vector<T> Deserializer::deserialize_array(ui64 count)
	{
		std::vector<T> out_array(count);

		if(!try_deserialize_array(out_array.data(), count))
		{
			hit_assert(false, "Failed to deserialize array!");
		}

		return out_array;
	}

	inline std::string Deserializer::deserialize_string()
	{
		std::string out_str;

		if(!try_deserialize_string(&out_str))
		{
			hit_assert(false, "Failed to deserialize data!");
		}

		return out_str;
	}
}