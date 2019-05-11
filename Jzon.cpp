/*
Copyright (c) 2015 Johannes Häggqvist

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifdef JZON_DLL
#	if defined _WIN32 || defined __CYGWIN__
#		define JZON_API __declspec(dllexport)
#		define JZON_STL_EXTERN
#	endif
#endif

#include "Jzon.h"

#include <fstream>
#include <stack>
#include <algorithm>
#include <cassert>

namespace Jzon
{
	namespace
	{
		inline bool isWhitespace(char c)
		{
			return (c == '\n' || c == ' ' || c == '\t' || c == '\r' || c == '\f');
		}

		const char charsUnescaped[] = { '\\'  , '/'  , '\"'  , '\n' , '\t' , '\b' , '\f' , '\r' };
		const char *charsEscaped[]  = { "\\\\", "\\/", "\\\"", "\\n", "\\t", "\\b", "\\f", "\\r" };
		const unsigned int numEscapeChars = 8;
		const char nullUnescaped = '\0';
		const char *nullEscaped  = "\0\0";
		const char *getEscaped(const char c)
		{
			for (unsigned int i = 0; i < numEscapeChars; ++i)
			{
				const char &ue = charsUnescaped[i];

				if (c == ue)
				{
					return charsEscaped[i];
				}
			}
			return nullEscaped;
		}
		char getUnescaped(const char c1, const char c2)
		{
			for (unsigned int i = 0; i < numEscapeChars; ++i)
			{
				const char *e = charsEscaped[i];

				if (c1 == e[0] && c2 == e[1])
				{
					return charsUnescaped[i];
				}
			}
			return nullUnescaped;
		}
	}

	Node::Node() : data(NULL)
	{
	}
	Node::Node(Type type) : data(NULL)
	{
		if (type != T_INVALID)
		{
			data = std::make_shared<Data>(type);
		}
	}
	Node::Node(const Node &other) : data(other.data)
	{
	}
	Node::Node(Node &&other) : data(std::move(other.data))
	{
	}
	Node::Node(Type type, const std::string &value) : data(std::make_shared<Data>(T_NULL)) { set(type, value); }
	Node::Node(const std::string &value) : data(std::make_shared<Data>(T_STRING)) { set(value); }
	Node::Node(std::string value) : data(std::make_shared<Data>(T_STRING)) { set(std::move(value)); }
	Node::Node(const char *value) : data(std::make_shared<Data>(T_STRING)) { set(value); }
	Node::Node(int value) : data(std::make_shared<Data>(T_NUMBER)) { set(value); }
	Node::Node(unsigned int value) : data(std::make_shared<Data>(T_NUMBER)) { set(value); }
	Node::Node(long long value) : data(std::make_shared<Data>(T_NUMBER)) { set(value); }
	Node::Node(unsigned long long value) : data(std::make_shared<Data>(T_NUMBER)) { set(value); }
	Node::Node(float value) : data(std::make_shared<Data>(T_NUMBER)) { set(value); }
	Node::Node(double value) : data(std::make_shared<Data>(T_NUMBER)) { set(value); }
	Node::Node(bool value) : data(std::make_shared<Data>(T_BOOL)) { set(value); }
	Node::~Node()
	{
	}

	void Node::detach()
	{
		if (data != NULL)
		{
			data = std::make_shared<Data>(*data);
		}
	}

	std::string Node::toString(const std::string &def) const
	{
		if (isValue())
		{
			if (isNull())
			{
				return std::string("null");
			}
			else
			{
				return data->valueStr;
			}
		}
		else
		{
			return def;
		}
	}
#define GET_NUMBER(T) \
	if (isNumber())\
	{\
		std::stringstream sstr(data->valueStr);\
		T val;\
		sstr >> val;\
		return val;\
	}\
	else\
	{\
		return def;\
	}
	int Node::toInt(int def) const { GET_NUMBER(int) }
	float Node::toFloat(float def) const { GET_NUMBER(float) }
	double Node::toDouble(double def) const { GET_NUMBER(double) }
#undef GET_NUMBER
	bool Node::toBool(bool def) const
	{
		if (isBool())
		{
			return (data->valueStr == "true");
		}
		else
		{
			return def;
		}
	}

	void Node::setNull()
	{
		if (isValue())
		{
			detach();
			data->type = T_NULL;
			data->valueStr.clear();
		}
	}
	void Node::set(Type type, const std::string &value)
	{
		set(type, value.c_str());
	}
	void Node::set(Type type, std::string&& value)
	{
		if (isValue() && (type == T_NULL || type == T_STRING || type == T_NUMBER || type == T_BOOL))
		{
			detach();
			data->type = type;
			if (type == T_STRING)
			{
				data->valueStr = unescapeString(value.c_str());
			}
			else
			{
				data->valueStr = std::move(value);
			}
		}
	}
	void Node::set(Type type, const char *value)
	{
		if (isValue() && (type == T_NULL || type == T_STRING || type == T_NUMBER || type == T_BOOL))
		{
			detach();
			data->type = type;
			if (type == T_STRING)
			{
				data->valueStr = unescapeString(value);
			}
			else
			{
				data->valueStr = value;
			}
		}
	}
	void Node::set(const std::string &value)
	{
		set(value.c_str());
	}
	void Node::set(const char *value)
	{
		if (isValue())
		{
			detach();
			data->type = T_STRING;
			data->valueStr = unescapeString(value);
		}
	}

	void Node::set(int value) { set_number(value); }
	void Node::set(unsigned int value) { set_number(value); }
	void Node::set(long value) { set_number(value); }
	void Node::set(unsigned long value) { set_number(value); }
	void Node::set(long long value) { set_number(value); }
	void Node::set(unsigned long long value) { set_number(value); }
	void Node::set(float value) { set_number(value); }
	void Node::set(double value) { set_number(value); }

	void Node::set(bool value)
	{
		if (isValue())
		{
			detach();
			data->type = T_BOOL;
			data->valueStr = (value ? "true" : "false");
		}
	}

	Node &Node::operator=(const Node &rhs)
	{
		if (this != &rhs)
		{
			data = rhs.data;
		}
		return *this;
	}
	Node &Node::operator=(Node &&rhs)
	{
		if (this != &rhs)
		{
			data = std::move(rhs.data);
		}
		return *this;
	}
	Node &Node::operator=(const std::string &rhs) { set(rhs); return *this; }
	Node &Node::operator=(std::string &&rhs) { set(std::move(rhs)); return *this; }
	Node &Node::operator=(const char *rhs) { set(rhs); return *this; }
	Node &Node::operator=(int rhs) { set(rhs); return *this; }
	Node &Node::operator=(unsigned int rhs) { set(rhs); return *this; }
	Node &Node::operator=(long rhs) { set(rhs); return *this; }
	Node &Node::operator=(unsigned long rhs) { set(rhs); return *this; }
	Node &Node::operator=(long long rhs) { set(rhs); return *this; }
	Node &Node::operator=(unsigned long long rhs) { set(rhs); return *this; }
	Node &Node::operator=(float rhs) { set(rhs); return *this; }
	Node &Node::operator=(double rhs) { set(rhs); return *this; }
	Node &Node::operator=(bool rhs) { set(rhs); return *this; }

	void Node::add(const Node &node)
	{
		if (isArray())
		{
			detach();
			data->children.emplace_back(std::make_pair(std::string(), node));
		}
	}
	void Node::add(const std::string &name, const Node &node)
	{
		if (isObject())
		{
			detach();
			data->children.emplace_back(std::make_pair(name, node));
		}
	}
	void Node::add(std::string&& name, const Node &node)
	{
		if (isObject())
		{
			detach();
			data->children.emplace_back(std::make_pair(std::move(name), node));
		}
	}
	void Node::append(const Node &node)
	{
		if ((isObject() && node.isObject()) || (isArray() && node.isArray()))
		{
			detach();
			data->children.insert(data->children.end(), node.data->children.begin(), node.data->children.end());
		}
	}
	void Node::remove(size_t index)
	{
		if (isContainer() && index < data->children.size())
		{
			detach();
			NamedNodeList::iterator it = data->children.begin()+index;
			data->children.erase(it);
		}
	}
	void Node::remove(const char *name)
	{
		if (isObject())
		{
			detach();
			NamedNodeList &children = data->children;
			for (NamedNodeList::iterator it = children.begin(); it != children.end(); ++it)
			{
				if ((*it).first == name)
				{
					children.erase(it);
					break;
				}
			}
		}
	}
	void Node::remove(const std::string &name)
	{
		remove(name.c_str());
	}
	void Node::clear()
	{
		if (data != NULL && !data->children.empty())
		{
			detach();
			data->children.clear();
		}
	}
	bool Node::has(const char *name) const
	{
		if (isObject())
		{
			NamedNodeList &children = data->children;
			for (NamedNodeList::const_iterator it = children.begin(); it != children.end(); ++it)
			{
				if ((*it).first == name)
				{
					return true;
				}
			}
		}
		return false;
	}
	bool Node::has(const std::string &name) const
	{
		return has(name.c_str());
	}
	size_t Node::getCount() const
	{
		return data != NULL ? data->children.size() : 0;
	}
	Node Node::get(const char *name) const
	{
		if (isObject())
		{
			NamedNodeList &children = data->children;
			for (NamedNodeList::const_iterator it = children.begin(); it != children.end(); ++it)
			{
				if ((*it).first == name)
				{
					return (*it).second;
				}
			}
		}
		return Node(T_INVALID);
	}
	Node Node::get(const std::string &name) const
	{
		return get(name.c_str());
	}
	Node Node::get(size_t index) const
	{
		if (isContainer() && index < data->children.size())
		{
			return data->children.at(index).second;
		}
		return Node(T_INVALID);
	}

	Node::iterator Node::begin()
	{
		if (data != NULL && !data->children.empty())
			return Node::iterator(&data->children.front());
		else
			return Node::iterator(NULL);
	}
	Node::const_iterator Node::begin() const
	{
		if (data != NULL && !data->children.empty())
			return Node::const_iterator(&data->children.front());
		else
			return Node::const_iterator(NULL);
	}
	Node::iterator Node::end()
	{
		if (data != NULL && !data->children.empty())
			return Node::iterator(&data->children.back()+1);
		else
			return Node::iterator(NULL);
	}
	Node::const_iterator Node::end() const
	{
		if (data != NULL && !data->children.empty())
			return Node::const_iterator(&data->children.back()+1);
		else
			return Node::const_iterator(NULL);
	}

	bool Node::operator==(const Node &other) const
	{
		return (
			(data == other.data) ||
			(isValue() && (data->type == other.data->type)&&(data->valueStr == other.data->valueStr)));
	}
	bool Node::operator!=(const Node &other) const
	{
		return !(*this == other);
	}

	Node::Data::Data(Type type) : type(type)
	{
	}
	Node::Data::Data(const Data &other) : type(other.type), valueStr(other.valueStr), children(other.children)
	{
	}
	Node::Data::Data(Data &&other) : type(other.type), valueStr(std::move(other.valueStr)), children(std::move(other.children))
	{
	}
	Node::Data::~Data()
	{
	}


	std::string escapeString(const std::string &value)
	{
		std::string escaped;
		escaped.reserve(value.length());

		for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
		{
			const char &c = (*it);

			const char *a = getEscaped(c);
			if (a[0] != '\0')
			{
				escaped += a[0];
				escaped += a[1];
			}
			else
			{
				escaped += c;
			}
		}

		return escaped;
	}
	std::string unescapeString(const char *value)
	{
		std::ostringstream unescaped;

		while (*value != '\0')
		{
			const char c = (*value);
			const char c2 = (*(value + 1) != '\0')
				? '\0'
				: *(value + 1);

			const char a = getUnescaped(c, c2);
			if (a != '\0')
			{
				unescaped << a;
				if (*(value + 1) != '\0')
					++value;
			}
			else
			{
				unescaped << c;
			}
		}

		return unescaped.str();
	}
	std::string unescapeString(const std::string &value)
	{
		return unescapeString(value.c_str());
	}

	Node invalid()
	{
		return Node(Node::T_INVALID);
	}
	Node null()
	{
		return Node(Node::T_NULL);
	}
	Node object()
	{
		return Node(Node::T_OBJECT);
	}
	Node array()
	{
		return Node(Node::T_ARRAY);
	}


	Writer::Writer(const Format &format)
	{
		setFormat(format);
	}
	Writer::~Writer()
	{
	}

	void Writer::setFormat(const Format &format)
	{
		this->format = format;
		indentationChar = (format.useTabs ? '\t' : ' ');
		spacing = (format.spacing ? " " : "");
		newline = (format.newline ? "\n" : spacing);
	}

	void Writer::writeStream(const Node &node, std::ostream &stream) const
	{
		writeNode(node, 0, stream);
	}
	void Writer::writeString(const Node &node, std::string &json) const
	{
		std::ostringstream stream(json);
		writeStream(node, stream);
		json = stream.str();
	}
	void Writer::writeFile(const Node &node, const std::string &filename) const
	{
		std::ofstream stream(filename.c_str(), std::ios::out | std::ios::trunc);
		writeStream(node, stream);
	}

	void Writer::writeNode(const Node &node, unsigned int level, std::ostream &stream) const
	{
		switch (node.getType())
		{
		case Node::T_INVALID: break;
		case Node::T_OBJECT: writeObject(node, level, stream); break;
		case Node::T_ARRAY: writeArray(node, level, stream); break;
		case Node::T_NULL: // Fallthrough
		case Node::T_STRING: // Fallthrough
		case Node::T_NUMBER: // Fallthrough
		case Node::T_BOOL: writeValue(node, stream); break;
		}
	}
	void Writer::writeObject(const Node &node, unsigned int level, std::ostream &stream) const
	{
		stream << "{" << newline;

		for (Node::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			const std::string &name = (*it).first;
			const Node &value = (*it).second;

			if (it != node.begin())
				stream << "," << newline;
			stream << getIndentation(level+1) << "\""<<name<<"\"" << ":" << spacing;
			writeNode(value, level+1, stream);
		}

		stream << newline << getIndentation(level) << "}";
	}
	void Writer::writeArray(const Node &node, unsigned int level, std::ostream &stream) const
	{
		stream << "[" << newline;

		for (Node::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			const Node &value = (*it).second;

			if (it != node.begin())
				stream << "," << newline;
			stream << getIndentation(level+1);
			writeNode(value, level+1, stream);
		}

		stream << newline << getIndentation(level) << "]";
	}
	void Writer::writeValue(const Node &node, std::ostream &stream) const
	{
		if (node.isString())
		{
			stream << "\""<<escapeString(node.toString())<<"\"";
		}
		else
		{
			stream << node.toString();
		}
	}

	std::string Writer::getIndentation(unsigned int level) const
	{
		if (!format.newline)
		{
			return "";
		}
		else
		{
			return std::string(format.indentSize * level, indentationChar);
		}
	}


	Parser::Parser()
	{
	}
	Parser::~Parser()
	{
	}

	Node Parser::parseStream(std::istream &stream)
	{
		TokenQueue tokens;
		DataQueue data;

		tokenize(stream, tokens, data);
		Node node = assemble(tokens, data);

		return node;
	}
	Node Parser::parseString(const std::string &json)
	{
		std::istringstream stream(json);
		return parseStream(stream);
	}
	Node Parser::parseFile(const std::string &filename)
	{
		std::ifstream stream(filename.c_str(), std::ios::in);
		return parseStream(stream);
	}

	const std::string &Parser::getError() const
	{
		return error;
	}

	void Parser::tokenize(std::istream &stream, TokenQueue &tokens, DataQueue &data)
	{
		Token token = T_UNKNOWN;
		std::string valueBuffer;
		bool saveBuffer;

		char c = '\0';
		while (stream.peek() != std::char_traits<char>::eof())
		{
			stream.get(c);

			if (isWhitespace(c))
				continue;

			saveBuffer = true;

			switch (c)
			{
			case '{':
				{
					token = T_OBJ_BEGIN;
					break;
				}
			case '}':
				{
					token = T_OBJ_END;
					break;
				}
			case '[':
				{
					token = T_ARRAY_BEGIN;
					break;
				}
			case ']':
				{
					token = T_ARRAY_END;
					break;
				}
			case ',':
				{
					token = T_SEPARATOR_NODE;
					break;
				}
			case ':':
				{
					token = T_SEPARATOR_NAME;
					break;
				}
			case '"':
				{
					token = T_VALUE;
					readString(stream, data);
					break;
				}
			case '/':
				{
					char p = static_cast<char>(stream.peek());
					if (p == '*')
					{
						jumpToCommentEnd(stream);
						saveBuffer = false;
						break;
					}
					else if (p == '/')
					{
						jumpToNext('\n', stream);
						saveBuffer = false;
						break;
					}
					// Intentional fallthrough
				}
			default:
				{
					valueBuffer += c;
					saveBuffer = false;
					break;
				}
			}

			if ((saveBuffer || stream.peek() == std::char_traits<char>::eof()) && (!valueBuffer.empty())) // Always save buffer on the last character
			{
				if (interpretValue(valueBuffer, data))
				{
					tokens.push(T_VALUE);
				}
				else
				{
					// Store the unknown token, so we can show it to the user
					data.push(std::make_pair(Node::T_STRING, valueBuffer));
					tokens.push(T_UNKNOWN);
				}

				valueBuffer.clear();
			}

			// Push the token last so that any data
			// will get pushed first from above.
			// If saveBuffer is false, it means that
			// we are in the middle of a value, so we
			// don't want to push any tokens now.
			if (saveBuffer)
			{
				tokens.push(token);
			}
		}
	}
	Node Parser::assemble(TokenQueue &tokens, DataQueue &data)
	{
		std::stack<NamedNode> nodeStack;
		Node root(Node::T_INVALID);

		std::string nextName = "";

		Token token;
		while (!tokens.empty())
		{
			token = tokens.front();
			tokens.pop();

			switch (token)
			{
			case T_UNKNOWN:
				{
					const std::string &unknownToken = data.front().second;
					error = "Unknown token: "+unknownToken;
					data.pop();
					return Node(Node::T_INVALID);
				}
			case T_OBJ_BEGIN:
				{
					nodeStack.push(std::make_pair(nextName, object()));
					nextName.clear();
					break;
				}
			case T_ARRAY_BEGIN:
				{
					nodeStack.push(std::make_pair(nextName, array()));
					nextName.clear();
					break;
				}
			case T_OBJ_END:
			case T_ARRAY_END:
				{
					if (nodeStack.empty())
					{
						error = "Found end of object or array without beginning";
						return Node(Node::T_INVALID);
					}
					if (token == T_OBJ_END && !nodeStack.top().second.isObject())
					{
						error = "Mismatched end and beginning of object";
						return Node(Node::T_INVALID);
					}
					if (token == T_ARRAY_END && !nodeStack.top().second.isArray())
					{
						error = "Mismatched end and beginning of array";
						return Node(Node::T_INVALID);
					}

					std::string nodeName(std::move(nodeStack.top().first));
					Node node = nodeStack.top().second;
					nodeStack.pop();

					if (!nodeStack.empty())
					{
						Node &stackTop = nodeStack.top().second;
						if (stackTop.isObject())
						{
							stackTop.add(nodeName, node);
						}
						else if (stackTop.isArray())
						{
							stackTop.add(node);
						}
						else
						{
							error = "Can only add elements to objects and arrays";
							return Node(Node::T_INVALID);
						}
					}
					else
					{
						root = node;
					}
					break;
				}
			case T_VALUE:
				{
					if (data.empty())
					{
						error = "Missing data for value";
						return Node(Node::T_INVALID);
					}

					std::pair<Node::Type, std::string> &dataPair = data.front();
					if (!tokens.empty() && tokens.front() == T_SEPARATOR_NAME)
					{
						tokens.pop();
						if (dataPair.first != Node::T_STRING)
						{
							error = "A name has to be a string";
							return Node(Node::T_INVALID);
						}
						else
						{
							nextName = dataPair.second;
							data.pop();
						}
					}
					else
					{
						Node node(dataPair.first, std::move(dataPair.second));
						data.pop();

						if (!nodeStack.empty())
						{
							Node &stackTop = nodeStack.top().second;
							if (stackTop.isObject())
								stackTop.add(nextName, node);
							else if (stackTop.isArray())
								stackTop.add(node);

							nextName.clear();
						}
						else
						{
							error = "Outermost node must be an object or array";
							return Node(Node::T_INVALID);
						}
					}
					break;
				}
			case T_SEPARATOR_NAME:
				break;
			case T_SEPARATOR_NODE:
				{
					if (!tokens.empty() && tokens.front() == T_ARRAY_END) {
						error = "Extra comma in array";
						return Node(Node::T_INVALID);
					}
					break;
				}
			}
		}

		return root;
	}

	void Parser::jumpToNext(char c, std::istream &stream)
	{
		while (!stream.eof() && static_cast<char>(stream.get()) != c);
		stream.unget();
	}
	void Parser::jumpToCommentEnd(std::istream &stream)
	{
		stream.ignore(1);
		char c1 = '\0', c2 = '\0';
		while (stream.peek() != std::char_traits<char>::eof())
		{
			stream.get(c2);

			if (c1 == '*' && c2 == '/')
				break;

			c1 = c2;
		}
	}

	void Parser::readString(std::istream &stream, DataQueue &data)
	{
		std::ostringstream str;

		char c1 = '\0', c2 = '\0';
		while (stream.peek() != std::char_traits<char>::eof())
		{
			stream.get(c2);

			if (c1 != '\\' && c2 == '"')
			{
				break;
			}

			str << c2;

			c1 = c2;
		}

		data.push(std::make_pair(Node::T_STRING, str.str()));
	}
	bool Parser::interpretValue(const std::string &value, DataQueue &data)
	{
		std::string upperValue(value.size(), '\0');

		std::transform(value.begin(), value.end(), upperValue.begin(), toupper);

		if (upperValue == "NULL")
		{
			data.push(std::make_pair(Node::T_NULL, std::string()));
		}
		else if (upperValue == "TRUE")
		{
			data.push(std::make_pair(Node::T_BOOL, std::string("true")));
		}
		else if (upperValue == "FALSE")
		{
			data.push(std::make_pair(Node::T_BOOL, std::string("false")));
		}
		else
		{
			bool number = true;
			bool negative = false;
			bool fraction = false;
			bool scientific = false;
			bool scientificSign = false;
			bool scientificNumber = false;
			for (std::string::const_iterator it = upperValue.begin(); number && it != upperValue.end(); ++it)
			{
				char c = (*it);
				switch (c)
				{
				case '-':
					{
						if (scientific)
						{
							if (scientificSign) // Only one - allowed after E
								number = false;
							else
								scientificSign = true;
						}
						else
						{
							if (negative) // Only one - allowed before E
								number = false;
							else
								negative = true;
						}
						break;
					}
				case '+':
					{
						if (!scientific || scientificSign)
							number = false;
						else
							scientificSign = true;
						break;
					}
				case '.':
					{
						if (fraction) // Only one . allowed
							number = false;
						else
							fraction = true;
						break;
					}
				case 'E':
					{
						if (scientific)
							number = false;
						else
							scientific = true;
						break;
					}
				default:
					{
						if (c >= '0' && c <= '9')
						{
							if (scientific)
								scientificNumber = true;
						}
						else
						{
							number = false;
						}
						break;
					}
				}
			}

			if (scientific && !scientificNumber)
				number = false;

			if (number)
			{
				data.push(std::make_pair(Node::T_NUMBER, value));
			}
			else
			{
				return false;
			}
		}

		return true;
	}
}
