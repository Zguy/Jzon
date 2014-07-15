/*
Copyright (c) 2014 Johannes Häggqvist

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
#include "Jzon.h"

#include <sstream>
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
		inline bool isNumber(char c)
		{
			return ((c >= '0' && c <= '9') || c == '.' || c == '-');
		}

		const char charsUnescaped[] = { '\\'  , '/'  , '\"'  , '\n' , '\t' , '\b' , '\f' , '\r' };
		const char *charsEscaped[]  = { "\\\\", "\\/", "\\\"", "\\n", "\\t", "\\b", "\\f", "\\r" };
		const unsigned int numEscapeChars = 8;
		const char nullUnescaped = '\0';
		const char *nullEscaped  = "\0\0";
		const char *&getEscaped(const char &c)
		{
			for (unsigned int i = 0; i < numEscapeChars; ++i)
			{
				const char &ue = charsUnescaped[i];

				if (c == ue)
				{
					const char *&e = charsEscaped[i];
					return e;
				}
			}
			return nullEscaped;
		}
		const char &getUnescaped(const char &c1, const char &c2)
		{
			for (unsigned int i = 0; i < numEscapeChars; ++i)
			{
				const char *&e = charsEscaped[i];

				if (c1 == e[0] && c2 == e[1])
				{
					const char &ue = charsUnescaped[i];
					return ue;
				}
			}
			return nullUnescaped;
		}
	}

	Node::Node(Type type) : data(NULL)
	{
		if (type != T_INVALID)
		{
			data = new Data(type);
		}
	}
	Node::Node(const Node &other) : data(other.data)
	{
		if (data != NULL)
		{
			data->addRef();
		}
	}
	Node::Node(const std::string &value) : data(new Data(T_STRING))
	{
		set(value);
	}
	Node::Node(const char *value) : data(new Data(T_STRING))
	{
		set(value);
	}
	Node::Node(int value) : data(new Data(T_NUMBER))
	{
		set(value);
	}
	Node::Node(float value) : data(new Data(T_NUMBER))
	{
		set(value);
	}
	Node::Node(double value) : data(new Data(T_NUMBER))
	{
		set(value);
	}
	Node::Node(bool value) : data(new Data(T_BOOL))
	{
		set(value);
	}
	Node::~Node()
	{
		if (data != NULL && data->release())
		{
			delete data;
			data = NULL;
		}
	}

	void Node::detach()
	{
		if (data != NULL && data->refCount > 1)
		{
			Data *newData = new Data(*data);
			if (data->release())
			{
				delete data;
			}
			data = newData;
		}
	}

	Node::Type Node::getType() const
	{
		return (data == NULL ? T_INVALID : data->type);
	}

	std::string Node::toString(const std::string &default) const
	{
		if (!isValue() || isNull())
		{
			return default;
		}
		else
		{
			return data->valueStr;
		}
	}
	int Node::toInt(int default) const
	{
		if (isNumber())
		{
			std::stringstream sstr(data->valueStr);
			int val;
			sstr >> val;
			return val;
		}
		else
		{
			return default;
		}
	}
	float Node::toFloat(float default) const
	{
		if (isNumber())
		{
			std::stringstream sstr(data->valueStr);
			float val;
			sstr >> val;
			return val;
		}
		else
		{
			return default;
		}
	}
	double Node::toDouble(double default) const
	{
		if (isNumber())
		{
			std::stringstream sstr(data->valueStr);
			double val;
			sstr >> val;
			return val;
		}
		else
		{
			return default;
		}
	}
	bool Node::toBool(bool default) const
	{
		if (isBool())
		{
			return (data->valueStr == "true");
		}
		else
		{
			return default;
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
		if (isValue() && (type == T_NULL || type == T_STRING || type == T_NUMBER || type == T_BOOL))
		{
			detach();
			data->type = type;
			data->valueStr = value;
		}
	}
	void Node::set(const std::string &value)
	{
		if (isValue())
		{
			detach();
			data->type = T_STRING;
			data->valueStr = unescapeString(value);
		}
	}
	void Node::set(const char *value)
	{
		if (isValue())
		{
			detach();
			data->type = T_STRING;
			data->valueStr = unescapeString(std::string(value));
		}
	}
	void Node::set(int value)
	{
		if (isValue())
		{
			detach();
			data->type = T_NUMBER;
			std::stringstream sstr;
			sstr << value;
			data->valueStr = sstr.str();
		}
	}
	void Node::set(float value)
	{
		if (isValue())
		{
			detach();
			data->type = T_NUMBER;
			std::stringstream sstr;
			sstr << value;
			data->valueStr = sstr.str();
		}
	}
	void Node::set(double value)
	{
		if (isValue())
		{
			detach();
			data->type = T_NUMBER;
			std::stringstream sstr;
			sstr << value;
			data->valueStr = sstr.str();
		}
	}
	void Node::set(bool value)
	{
		if (isValue())
		{
			data->type = T_BOOL;
			if (value)
				data->valueStr = "true";
			else
				data->valueStr = "false";
		}
	}

	Node &Node::operator=(const Node &rhs)
	{
		if (this != &rhs)
		{
			if (data != NULL && data->release())
			{
				delete data;
			}
			data = rhs.data;
			if (data != NULL)
			{
				data->addRef();
			}
		}
		return *this;
	}
	Node &Node::operator=(const std::string &rhs)
	{
		set(rhs);
		return *this;
	}
	Node &Node::operator=(const char *rhs)
	{
		set(rhs);
		return *this;
	}
	Node &Node::operator=(int rhs)
	{
		set(rhs);
		return *this;
	}
	Node &Node::operator=(float rhs)
	{
		set(rhs);
		return *this;
	}
	Node &Node::operator=(double rhs)
	{
		set(rhs);
		return *this;
	}
	Node &Node::operator=(bool rhs)
	{
		set(rhs);
		return *this;
	}

	void Node::add(const Node &node)
	{
		if (isArray())
		{
			detach();
			data->children.push_back(std::make_pair(std::string(), node));
		}
	}
	void Node::add(const std::string &name, const Node &node)
	{
		if (isObject())
		{
			detach();
			data->children.push_back(std::make_pair(name, node));
		}
	}
	void Node::remove(size_t index)
	{
		if (isList() && index < data->children.size())
		{
			detach();
			NamedNodeList::iterator it = data->children.begin()+index;
			data->children.erase(it);
		}
	}
	void Node::remove(const std::string &name)
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
	void Node::clear()
	{
		if (data != NULL && !data->children.empty())
		{
			detach();
			data->children.clear();
		}
	}

	bool Node::has(const std::string &name) const
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
	size_t Node::getCount() const
	{
		return data->children.size();
	}
	Node Node::get(const std::string &name) const
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
	Node Node::get(size_t index) const
	{
		if (isList() && index < data->children.size())
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

	Node::Data::Data(Type type) : refCount(1), type(type)
	{
	}
	Node::Data::Data(const Data &other) : refCount(1), type(other.type), valueStr(other.valueStr), children(other.children)
	{
	}
	Node::Data::~Data()
	{
		assert(refCount == 0);
	}
	void Node::Data::addRef()
	{
		++refCount;
	}
	bool Node::Data::release()
	{
		return (--refCount == 0);
	}


	std::string escapeString(const std::string &value)
	{
		std::string escaped;

		for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
		{
			const char &c = (*it);

			const char *&a = getEscaped(c);
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
	std::string unescapeString(const std::string &value)
	{
		std::string unescaped;

		for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
		{
			const char &c = (*it);
			char c2 = '\0';
			if (it+1 != value.end())
				c2 = *(it+1);

			const char &a = getUnescaped(c, c2);
			if (a != '\0')
			{
				unescaped += a;
				if (it+1 != value.end())
					++it;
			}
			else
			{
				unescaped += c;
			}
		}

		return unescaped;
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
	const std::string &Writer::write(const Node &node)
	{
		result.clear();
		writeNode(node, 0);
		return result;
	}

	const std::string &Writer::getResult() const
	{
		return result;
	}

	void Writer::writeNode(const Node &node, unsigned int level)
	{
		switch (node.getType())
		{
		case Node::T_INVALID: break;
		case Node::T_OBJECT : writeObject(node, level); break;
		case Node::T_ARRAY  : writeArray(node, level);  break;
		default             : writeValue(node);         break;
		}
	}
	void Writer::writeObject(const Node &node, unsigned int level)
	{
		result += "{" + newline;

		for (Node::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			const std::string &name = (*it).first;
			const Node &value = (*it).second;

			if (it != node.begin())
				result += "," + newline;
			result += getIndentation(level+1) + "\""+name+"\"" + ":" + spacing;
			writeNode(value, level+1);
		}

		result += newline + getIndentation(level) + "}";
	}
	void Writer::writeArray(const Node &node, unsigned int level)
	{
		result += "[" + newline;

		for (Node::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			const Node &value = (*it).second;

			if (it != node.begin())
				result += "," + newline;
			result += getIndentation(level+1);
			writeNode(value, level+1);
		}

		result += newline + getIndentation(level) + "]";
	}
	void Writer::writeValue(const Node &node)
	{
		if (node.isString())
		{
			result += "\""+escapeString(node.toString("null"))+"\"";
		}
		else
		{
			result += node.toString("null");
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


	Parser::Parser() : jsonSize(0), cursor(0)
	{
	}
	Parser::Parser(const std::string &json) : cursor(0)
	{
		setJson(json);
	}
	Parser::~Parser()
	{
	}

	void Parser::setJson(const std::string &json)
	{
		this->json = json;
		jsonSize   = json.size();
	}
	Node Parser::parse()
	{
		cursor = 0;

		tokenize();
		Node node = assemble();

		return node;
	}

	const std::string &Parser::getError() const
	{
		return error;
	}

	void Parser::tokenize()
	{
		Token token = T_UNKNOWN;
		std::string valueBuffer;
		bool saveBuffer;

		char c = '\0';
		for (; cursor < jsonSize; ++cursor)
		{
			c = json.at(cursor);

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
					readString();
					break;
				}
			case '/':
				{
					char p = peek();
					if (p == '*')
					{
						jumpToCommentEnd();
						saveBuffer = false;
						break;
					}
					else if (p == '/')
					{
						jumpToNext('\n');
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

			if ((saveBuffer || cursor == jsonSize-1) && (!valueBuffer.empty())) // Always save buffer on the last character
			{
				if (interpretValue(valueBuffer))
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

			// Push the token last so that any
			// value token will get pushed first
			// from above.
			// If saveBuffer is false, it means that
			// we are in the middle of a value, so we
			// don't want to push any tokens now.
			if (saveBuffer)
			{
				tokens.push(token);
			}
		}
	}
	Node Parser::assemble()
	{
		std::stack<Node::NamedNode> nodeStack;
		Node root(Node::T_INVALID);

		std::string name = "";

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
					nodeStack.push(std::make_pair(name, object()));
					name.clear();
					break;
				}
			case T_ARRAY_BEGIN:
				{
					nodeStack.push(std::make_pair(name, array()));
					name.clear();
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

					std::string name = nodeStack.top().first;
					Node node = nodeStack.top().second;
					nodeStack.pop();

					if (!nodeStack.empty())
					{
						Node &stackTop = nodeStack.top().second;
						if (stackTop.isObject())
						{
							stackTop.add(name, node);
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

					const std::pair<Node::Type, std::string> &dataPair = data.front();
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
							name = dataPair.second;
							data.pop();
						}
					}
					else
					{
						Node node = null();

						if (dataPair.first == Node::T_STRING)
						{
							node.set(dataPair.second); // This method calls unescapeString()
						}
						else
						{
							node.set(dataPair.first, dataPair.second);
						}
						data.pop();

						if (!nodeStack.empty())
						{
							Node &stackTop = nodeStack.top().second;
							if (stackTop.isObject())
								stackTop.add(name, node);
							else if (stackTop.isArray())
								stackTop.add(node);

							name.clear();
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
			case T_SEPARATOR_NODE: break;
			}
		}

		return root;
	}

	char Parser::peek()
	{
		if (cursor < jsonSize-1)
		{
			return json.at(cursor+1);
		}
		else
		{
			return '\0';
		}
	}
	void Parser::jumpToNext(char c)
	{
		++cursor;
		while (cursor < jsonSize && json.at(cursor) != c)
			++cursor;
	}
	void Parser::jumpToCommentEnd()
	{
		cursor += 2;
		char c1 = '\0', c2 = '\0';
		for (; cursor < jsonSize; ++cursor)
		{
			c2 = json.at(cursor);

			if (c1 == '*' && c2 == '/')
				break;

			c1 = c2;
		}
	}

	void Parser::readString()
	{
		if (json.at(cursor) != '"')
			return;

		std::string str;

		++cursor;

		char c1 = '\0', c2 = '\0';
		for (; cursor < jsonSize; ++cursor)
		{
			c2 = json.at(cursor);

			if (c1 != '\\' && c2 == '"')
			{
				break;
			}

			str += c2;

			c1 = c2;
		}

		data.push(std::make_pair(Node::T_STRING, str));
	}
	bool Parser::interpretValue(const std::string &value)
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
			for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
			{
				if (!isNumber(*it))
				{
					number = false;
					break;
				}
			}

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


	FileWriter::FileWriter(const Format &format) : writer(format)
	{
	}
	FileWriter::~FileWriter()
	{
	}

	void FileWriter::setFormat(const Format &format)
	{
		writer.setFormat(format);
	}
	void FileWriter::write(const std::string &filename, const Node &node)
	{
		writer.write(node);

		std::fstream file(filename.c_str(), std::ios::out | std::ios::trunc);
		file << writer.getResult();
		file.close();
	}


	FileParser::FileParser()
	{
	}
	FileParser::~FileParser()
	{
	}

	Node FileParser::parse(const std::string &filename)
	{
		std::string json;
		if (!loadFile(filename, json))
		{
			return Node(Node::T_INVALID);
		}

		parser.setJson(json);
		Node node = parser.parse();
		return node;
	}

	const std::string &FileParser::getError() const
	{
		return parser.getError();
	}

	bool FileParser::loadFile(const std::string &filename, std::string &json)
	{
		std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);

		if (!file.is_open())
		{
			return false;
		}

		file.seekg(0, std::ios::end);
		std::ios::pos_type size = file.tellg();
		file.seekg(0, std::ios::beg);

		json.resize(static_cast<std::string::size_type>(size), '\0');
		file.read(&json[0], size);

		return true;
	}
}
