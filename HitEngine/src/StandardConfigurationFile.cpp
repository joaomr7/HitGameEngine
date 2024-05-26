#include "File/StandardConfigurationFile.h"

#include "Core/Log.h"
#include "Core/Assert.h"
#include "Utils/StringHelpers.h"

namespace hit::config
{
	SourceReader::SourceReader(const std::string& source) 
		: m_source(source), m_current(0), m_line(1), m_column(1) { }

	Token SourceReader::get_next(bool reading_source_content)
	{
		ignore_irrelevant();

		ui64 start = m_current;

		if (reading_source_content)
		{
			if (!consume('{'))
			{
				return get_invalid_token(start);
			}
			start += 1;

			ui32 open_block_count = 0;
			while ((!check('}') || open_block_count > 0) && has_next())
			{
				if (check('{'))
				{
					open_block_count++;
				}

				if (check('}') && open_block_count > 0)
				{
					open_block_count--;
				}

				if (check('\n'))
				{
					m_line += 1;
					m_column = 1;
				}

				advance();
			}

			if (consume('}'))
			{
				auto source = m_source.substr(start, m_current - start - 1);
				return Token { Token::Source, m_line, m_column, source };
			}
			else
			{
				return get_invalid_token(start);
			}
		}

		if (is_digit())
		{
			while (is_digit() && has_next()) advance();

			if (consume('.'))
			{
				while (is_digit() && has_next()) advance();
			}

			auto value = m_source.substr(start, m_current - start);
			return Token { Token::Number, m_line, m_column, value };
		}

		if (is_alpha())
		{
			advance();
			while (is_alphanum() && has_next()) advance();

			std::string naming = m_source.substr(start, m_current - start);
			return Token { Token::Naming, m_line, m_column, naming };
		}

		if (consume('"'))
		{
			start++;

			while (!check('"') && has_next())
			{
				if (check('\n'))
				{
					m_line += 1;
					m_column = 1;
				}

				advance();
			}

			if (consume('"'))
			{
				std::string str = m_source.substr(start, m_current - start - 1);
				return Token { Token::String, m_line, m_column, str };
			}
			else
			{
				return get_invalid_token(start);
			}
		}

		if (consume(':'))
		{
			if (consume(':'))
			{
				return Token { Token::ReadSource, m_line, m_column, "::" };
			}
			else
			{
				return Token { Token::Assign, m_line, m_column, ":" };
			}
		}

		if (consume('['))
		{
			return Token { Token::ListOpen, m_line, m_column, "[" };
		}

		if (consume(']'))
		{
			return Token { Token::ListClose, m_line, m_column, "]" };
		}

		if (consume(','))
		{
			return Token { Token::Comma, m_line, m_column, "," };
		}

		if (consume('{'))
		{
			return Token { Token::BlockOpen, m_line, m_column, "{" };
		}

		if (consume('}'))
		{
			return Token { Token::BlockClose, m_line, m_column, "}" };
		}

		if (!has_next())
		{
			return Token { Token::EndOfFile, m_line, m_column, "EOF" };
		}

		return get_invalid_token(start);
	}

	char SourceReader::current() { return m_source[m_current]; }

	bool SourceReader::has_next() { return m_current + 1 < m_source.size(); }

	void SourceReader::advance() { m_current++; m_column++; }

	bool SourceReader::check(char c) { return m_source[m_current] == c; }

	bool SourceReader::match(char c) { return has_next() && m_source[m_current + 1] == c; }

	bool SourceReader::consume(char c)
	{
		if (check(c))
		{
			advance();
			return true;
		}
		else
		{
			return false;
		}
	}

	bool SourceReader::consume(const std::vector<char>& options)
	{
		for (char op : options)
		{
			if (consume(op))
			{
				return true;
			}
		}

		return false;
	}

	void SourceReader::ignore_irrelevant()
	{
		while (check(' ') || check('\n') || check('\t') || check('\r') || check('#'))
		{
			consume(' ');

			if (consume('\n'))
			{
				m_line += 1;
				m_column = 1;
			}

			if (consume('\t'))
			{
				m_column += 4;
			}

			consume('\r');

			// ignore comment
			if (consume('#'))
			{
				while (has_next() && !check('\n')) advance();
				advance();
			}
		}
	}

	bool SourceReader::is_digit() { return std::isdigit(m_source[m_current]); }

	bool SourceReader::is_alpha() { return std::isalpha(m_source[m_current]) || m_source[m_current] == '_'; }

	bool SourceReader::is_alphanum() { return std::isalnum(m_source[m_current]) || m_source[m_current] == '_'; }

	Token SourceReader::get_invalid_token(ui64 start)
	{
		return Token { Token::Invalid, m_line, m_column, m_source.substr(start, m_current - start) };
	}

	TokenReader::TokenReader(const std::string& source) : m_current(0)
	{ 
		SourceReader reader(source);

		bool read_source_block = false;
		while (true)
		{
			auto token = reader.get_next(read_source_block);
			read_source_block = false;

			if (token.type == Token::ReadSource)
			{
				read_source_block = true;
				continue;
			}

			m_tokens.push_back(token);

			if (token.type == Token::ReadSource)
			{
				read_source_block = true;
			}

			if (token.type == Token::EndOfFile)
			{
				break;
			}

			if (token.type == Token::Invalid)
			{
				hit_error("Configuration file syntax error at line {}, colum {}, value: {}",
						  token.line, token.column, token.value);

				m_tokens = { token };
				break;
			}
		}
	}

	Token TokenReader::get_current() { return m_tokens[m_current]; }

	Token TokenReader::get_past() { return m_current == 0 ? get_current() : m_tokens[m_current - 1]; }

	bool TokenReader::has_next() { return m_current + 1 < m_tokens.size(); }

	bool TokenReader::has_past() { return m_current > 0; }

	void TokenReader::advance() { m_current++; }

	bool TokenReader::check(Token::Type type) { return m_tokens[m_current].type == type; }

	bool TokenReader::check(const std::vector<Token::Type>& types)
	{
		for (auto type : types)
		{
			if (check(type))
			{
				return true;
			}
		}

		return false;
	}

	bool TokenReader::match(Token::Type type) { return has_next() && m_tokens[m_current + 1].type == type; }

	bool TokenReader::match(const std::vector<Token::Type>& types)
	{
		for (auto type : types)
		{
			if (match(type))
			{
				return true;
			}
		}

		return false;
	}

	bool TokenReader::consume(Token::Type type)
	{
		if (check(type))
		{
			advance();
			return true;
		}

		return false;
	}

	bool TokenReader::consume(const std::vector<Token::Type>& types)
	{
		for (auto type : types)
		{
			if (consume(type))
			{
				return true;
			}
		}

		return false;
	}

	Parser::Parser(const std::string& source) : m_reader(source) { }

	std::vector<Block> Parser::build_block_tree()
	{
		if (m_reader.get_current().type == Token::Invalid)
		{
			hit_error("Can't parse onfiguration file source.");
			return { };
		}

		std::vector<Block> blocks;

		while (m_reader.has_next())
		{
			blocks.push_back(parse_block());
		}

		return blocks;
	}

	Block Parser::parse_block()
	{
		if (!m_reader.consume(Token::Naming))
		{
			auto current = m_reader.get_current();
			hit_error("Expected identifier at line {}, column {}: {}",
					  current.line, current.column, current.value);

			return Block();
		}

		auto token = m_reader.get_past();

		Block block(token.value);

		if (m_reader.consume(Token::Source))
		{
			block.set_source_block_content(m_reader.get_past().value);
			return block;
		}
		else if(m_reader.consume(Token::BlockOpen))
		{
			while (!m_reader.check(Token::BlockClose) && m_reader.has_next())
			{
				if (m_reader.check(Token::Naming))
				{
					if (m_reader.match(Token::Assign))
					{
						block.add_property(parse_property());
						continue;
					}
					else if (m_reader.match(Token::BlockOpen) || m_reader.match(Token::Source))
					{
						block.add_inner_block(parse_block());
						continue;
					}
				}

				auto current = m_reader.get_current();
				hit_error("Invalid syntax at line {}, column {}: {}",
						  current.line, current.column, current.value);
			}

			if (!m_reader.consume(Token::BlockClose))
			{
				auto current = m_reader.get_current();
				hit_error("Expected '}}' at line {}, column {}: {}",
						  current.line, current.column, current.value);
			}
		}
		else
		{
			auto current = m_reader.get_current();
			hit_error("Expected '{{' at line {}, column {}: {}",
					  current.line, current.column, current.value);
		}

		return block;
	}

	Property Parser::parse_property()
	{
		auto token = m_reader.get_current();
		m_reader.advance();

		Property property;
		property.set_identifier(token.value);

		if (m_reader.consume(Token::Assign))
		{
			if (m_reader.consume(Token::ListOpen))
			{
				std::vector<Value> values;

				while (!m_reader.check(Token::ListClose) && m_reader.has_next())
				{
					values.push_back(parse_value());

					if (!m_reader.check(Token::ListClose) && !m_reader.consume(Token::Comma))
					{
						auto current = m_reader.get_current();
						hit_error("Expected ']' at line {}, column {}: {}",
								  current.line, current.column, current.value);

						break;
					}
				}

				if (!m_reader.consume(Token::ListClose))
				{
					auto current = m_reader.get_current();
					hit_error("Expected ']' at line {}, column {}: {}",
							  current.line, current.column, current.value);
				}

				property.set_value(values);
			}
			else
			{
				property.set_value(parse_value());
			}
		}

		return property;
	}

	Value Parser::parse_value()
	{
		if (m_reader.consume(Token::Number))
		{
			return Value(str_to_f64(m_reader.get_past().value));
		}

		if (m_reader.consume(Token::String) || m_reader.consume(Token::Naming))
		{
			return Value(m_reader.get_past().value);
		}

		auto current = m_reader.get_current();
		hit_error("Expected 'number' or 'string' at line {}, column {}: {}",
				  current.line, current.column, current.value);

		return Value();
	}

	bool Block::is_source_block() const
	{
		return !m_source_content.empty();
	}

	std::string Block::get_source_block_content() const
	{
		return m_source_content;
	}

	bool Block::has_property(const std::string& identifier) const
	{
		return m_properties.find(identifier) != m_properties.end();
	}

	Property Block::get_property(const std::string& identifier) const
	{
		auto it = m_properties.find(identifier);
		return it != m_properties.end() ? it->second : Property();
	}

	bool Block::has_inner_block(const std::string& identifier) const
	{
		return m_inner_blocks.find(identifier) != m_inner_blocks.end();
	}

	const Block* Block::get_inner_block(const std::string& identifier) const
	{
		auto it = m_inner_blocks.find(identifier);
		return it != m_inner_blocks.end() ? it->second.get() : nullptr;
	}

	void Block::add_property(const Property& property)
	{
		m_properties[property.get_identifier()] = property;
	}

	void Block::add_inner_block(const Block& block)
	{ 
		m_inner_blocks[block.get_identifier()] = create_ref<Block>(block);
	}

	StandardConfigurationFile::StandardConfigurationFile(const std::string& source)
	{ 
		Parser parser(source);

		auto blocks = parser.build_block_tree();

		if (blocks.empty())
		{
			hit_error("Failed to create configuration file.");
		}

		for (auto& block : blocks)
		{
			m_root_blocks[block.get_identifier()] = block;
		}
	}

	bool StandardConfigurationFile::has_configuration()
	{
		return !m_root_blocks.empty();
	}

	bool StandardConfigurationFile::has_block(const std::string& identifier)
	{
		return m_root_blocks.find(identifier) != m_root_blocks.end();
	}

	Block* StandardConfigurationFile::get_block(const std::string& identifier)
	{
		auto it = m_root_blocks.find(identifier);
		return it != m_root_blocks.end() ? &it->second : nullptr;
	}
}