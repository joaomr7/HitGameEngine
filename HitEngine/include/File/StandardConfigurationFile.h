#pragma once

#include "Core/Types.h"
#include "Utils/Ref.h"

#include <string>
#include <vector>
#include <variant>
#include <map>

namespace hit::config
{
	class Value
	{
	public:
		enum Type
		{
			String, Number
		};

	public:
		Value() : m_type(Number), m_value(0.0) { }
		Value(const std::string& str) : m_type(String), m_value(str) { }
		Value(f64 number) : m_type(Number), m_value(number) { }

		inline bool is_string() const { return m_type == String; }
		inline bool is_number() const { return m_type == Number; }

		constexpr const std::string& get_string() const { return std::get<String>(m_value); }
		constexpr f64 get_number() const { return std::get<Number>(m_value); }

	private:
		Type m_type;
		std::variant<std::string, f64> m_value;
	};

	class Property
	{
	public:
		enum Type
		{
			Single, List
		};

	public:
		Property() : m_identifier(""), m_type(Single), m_value(Value()) { }
		Property(const std::string& identifier, const Value& value) : m_identifier(identifier), m_type(Single), m_value(value) { }
		Property(const std::string& identifier, const std::vector<Value>& values) : m_identifier(identifier), m_type(List), m_value(values) { }

		inline bool is_single() const { return m_type == Single; }
		inline bool is_list() const { return m_type == List; }

		void set_value(const Value& value) { m_type = Single; m_value = value; }
		void set_value(const std::vector<Value>& values) { m_type = List; m_value = values; }
		constexpr const Value& get_single_value() const { return std::get<Single>(m_value); }
		constexpr const std::vector<Value>& get_list_value() const { return std::get<List>(m_value); }

		void set_identifier(const std::string& identifier) { m_identifier = identifier; }
		const std::string& get_identifier() const { return m_identifier; }

	private:
		Type m_type;
		std::variant <Value, std::vector<Value>> m_value;
		std::string m_identifier;
	};

	class Block
	{
	public:
		Block(const std::string& identifier = "") : m_identifier(identifier) { }

		bool is_source_block() const;

		void set_source_block_content(const std::string& source) { m_source_content = source; }
		std::string get_source_block_content() const;

		bool has_property(const std::string& identifier) const;
		Property get_property(const std::string& identifier) const;

		bool has_inner_block(const std::string& identifier) const;
		const Block* get_inner_block(const std::string& identifier) const;

		void set_identifier(const std::string& identifier) { m_identifier = identifier; }
		const std::string& get_identifier() const { return m_identifier; }

		void add_property(const Property& property);
		void add_inner_block(const Block& block);

		const std::map<std::string, Property>& get_properties() const { return m_properties; }

	private:
		std::string m_identifier;

		std::string m_source_content;
		std::map<std::string, Property> m_properties;
		std::map<std::string, Ref<Block>> m_inner_blocks;
	};

	struct Token
	{
		enum Type
		{
			Naming, Assign,
			BlockOpen, BlockClose,
			String, Number, 
			ListOpen, ListClose,
			Comma,
			ReadSource, Source,
			EndOfFile,
			Invalid
		};

		Type type;
		ui32 line;
		ui32 column;
		std::string value;
	};

	class SourceReader
	{
	public:
		SourceReader(const std::string& source);

		Token get_next(bool reading_source_content = false);

	private:
		char current();

		bool has_next();

		void advance();

		bool check(char c);

		bool match(char c);

		bool consume(char c);

		bool consume(const std::vector<char>& options);

		void ignore_irrelevant();

		bool is_digit();

		bool is_alpha();

		bool is_alphanum();

		Token get_invalid_token(ui64 start);

	private:
		ui32 m_line;
		ui32 m_column;
		ui64 m_current;
		std::string m_source;
	};

	class TokenReader
	{
	public:
		TokenReader(const std::string& source);

		Token get_current();
		Token get_past();

		bool has_next();
		bool has_past();

		void advance();

		bool check(Token::Type type);
		bool check(const std::vector<Token::Type>& types);

		bool match(Token::Type type);
		bool match(const std::vector<Token::Type>& types);

		bool consume(Token::Type type);
		bool consume(const std::vector<Token::Type>& types);

	private:
		ui64 m_current;
		std::vector<Token> m_tokens;
	};

	class Parser
	{
	public:
		Parser(const std::string& source);

		std::vector<Block> build_block_tree();

	private:
		Block parse_block();

		Property parse_property();

		Value parse_value();

	private:
		TokenReader m_reader;
	};

	class StandardConfigurationFile
	{
	public:
		StandardConfigurationFile(const std::string& source);

		bool has_configuration();

		bool has_block(const std::string& identifier);
		Block* get_block(const std::string& identifier);

	private:
		std::map<std::string, Block> m_root_blocks;
	};
}