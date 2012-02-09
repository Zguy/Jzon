/*
Copyright (c) 2011 Johannes Häggqvist

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

namespace Jzon
{
	class FormatInterpreter
	{
	public:
		FormatInterpreter(const Format &format) : format(format)
		{
			if (format.useTabs)
			{
				indentationChar = '\t';
			}
			else
			{
				for (unsigned int i = 0; i < format.indentSize; ++i)
				{
					indentationChar += ' ';
				}
			}

			spacing = (format.spacing ? " " : "");
			newline = (format.newline ? "\n" : spacing);
		}

		std::string GetIndentation(unsigned int level) const
		{
			if (!format.newline)
				return "";
			else
			{
				std::string ind;
				for (unsigned int i = 0; i < level; ++i)
				{
					ind += indentationChar;
				}
				return ind;
			}
		}
		
		inline std::string GetNewline() const
		{
			return newline;
		}
		inline std::string GetSpacing() const
		{
			return spacing;
		}

	private:
		const Format &format;
		std::string indentationChar;
		std::string newline;
		std::string spacing;
	};

	void RemoveWhitespace(const std::string &json, std::string &freshJson)
	{
		freshJson.clear();

		bool comment = false;
		int multicomment = 0;
		bool inString = false;

		for (std::string::const_iterator it = json.begin(); it != json.end(); ++it)
		{
			char c1 = (*it);
			char c2;
			if (it+1 != json.end())
				c2 = (*(it+1));

			if (c1 == '"')
			{
				inString = !inString;
			}

			if (!inString)
			{
				if (c1 == '/' && c2 == '*')
				{
					++multicomment;
					++it;
					continue;
				}
				else if (c1 == '*' && c2 == '/')
				{
					--multicomment;
					++it;
					continue;
				}
				else if (c1 == '/' && c2 == '/')
				{
					comment = true;
					++it;
					continue;
				}
				else if (c1 == '\n')
				{
					comment = false;
				}
			}

			if (comment || multicomment > 0)
				continue;

			if (inString)
			{
				if (c1 != '\n')
					freshJson += c1;
			}
			else
			{
				if ((c1 != '\n')&&(c1 != ' ')&&(c1 != '\t'))
					freshJson += c1;
			}
		}
	}

	Node::Node()
	{
	}
	Node::~Node()
	{
	}

	Object &Node::AsObject()
	{
		if (IsObject())
			return static_cast<Object&>(*this);
		else
			throw TypeException();
	}
	const Object &Node::AsObject() const
	{
		if (IsObject())
			return static_cast<const Object&>(*this);
		else
			throw TypeException();
	}
	Array &Node::AsArray()
	{
		if (IsArray())
			return static_cast<Array&>(*this);
		else
			throw TypeException();
	}
	const Array &Node::AsArray() const
	{
		if (IsArray())
			return static_cast<const Array&>(*this);
		else
			throw TypeException();
	}
	Value &Node::AsValue()
	{
		if (IsValue())
			return static_cast<Value&>(*this);
		else
			throw TypeException();
	}
	const Value &Node::AsValue() const
	{
		if (IsValue())
			return static_cast<const Value&>(*this);
		else
			throw TypeException();
	}

	Node::Type Node::DetermineType(const std::string &_json)
	{
		std::string json;
		RemoveWhitespace(_json, json);

		switch (json.at(0))
		{
		case '{' : return T_OBJECT; break;
		case '[' : return T_ARRAY; break;
		default : return T_VALUE; break;
		}
	}


	Value::Value()
		: valueStr(""), type(VT_NULL)
	{
	}
	Value::Value(const Value &rhs)
	{
		valueStr = rhs.valueStr;
		type = rhs.type;
	}
	Value::Value(const Node &rhs)
	{
		const Value &value = rhs.AsValue();
		valueStr = value.valueStr;
		type = value.type;
	}
	Value::Value(const std::string &value)
		: valueStr(value), type(VT_STRING)
	{
	}
	Value::Value(const char *value)
		: valueStr(std::string(value)), type(VT_STRING)
	{
	}
	Value::Value(const int value)
		: type(VT_INT)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
	}
	Value::Value(const double value)
		: type(VT_DOUBLE)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
	}
	Value::Value(const bool value)
		: type(VT_BOOL)
	{
		if (value)
			valueStr = "true";
		else
			valueStr = "false";
	}
	Value::~Value()
	{
	}

	Node::Type Value::GetType() const
	{
		return T_VALUE;
	}
	Value::ValueType Value::GetValueType() const
	{
		return type;
	}

	std::string Value::AsString() const
	{
		if (IsNull())
			return "null";
		else
			return valueStr;
	}
	int Value::AsInt() const
	{
		if (IsNull())
			return 0;
		else
		{
			if (IsInt() || IsDouble())
			{
				std::stringstream sstr(valueStr);
				int val;
				sstr >> val;
				return val;
			}
			else
				throw ValueException();
		}
	}
	double Value::AsDouble() const
	{
		if (IsNull())
			return 0.0;
		else
		{
			if (IsDouble() || IsInt())
			{
				std::stringstream sstr(valueStr);
				double val;
				sstr >> val;
				return val;
			}
			else
				throw ValueException();
		}
	}
	bool Value::AsBool() const
	{
		if (IsNull())
			return false;
		else
			if (IsBool())
				return (valueStr == "true");
			else
				throw ValueException();
	}

	void Value::SetNull()
	{
		valueStr = "";
		type = VT_NULL;
	}
	void Value::Set(const Value &value)
	{
		if (this != &value)
		{
			valueStr = value.valueStr;
			type = value.type;
		}
	}
	void Value::Set(const std::string &value)
	{
		valueStr = value;
		type = VT_STRING;
	}
	void Value::Set(const char *value)
	{
		valueStr = std::string(value);
		type = VT_STRING;
	}
	void Value::Set(const int value)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
		type = VT_INT;
	}
	void Value::Set(const double value)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
		type = VT_DOUBLE;
	}
	void Value::Set(const bool value)
	{
		if (value)
			valueStr = "true";
		else
			valueStr = "false";
		type = VT_BOOL;
	}

	Value &Value::operator=(const Value &rhs)
	{
		if (this != &rhs)
			Set(rhs);
		return *this;
	}
	Value &Value::operator=(const Node &rhs)
	{
		if (this != &rhs)
			Set(rhs.AsValue());
		return *this;
	}
	Value &Value::operator=(const std::string &rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const char *rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const int rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const double rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const bool rhs)
	{
		Set(rhs);
		return *this;
	}

	bool Value::operator==(const Value &other) const
	{
		return ((type == other.type)&&(valueStr == other.valueStr));
	}
	bool Value::operator!=(const Value &other) const
	{
		return !(*this == other);
	}

	std::string Value::Write(const Format &format, unsigned int level) const
	{
		std::string value;
		if (type == VT_NULL)
			value += "null";
		else if (type == VT_STRING)
			value += "\""+valueStr+"\"";
		else
			value += valueStr;
		return value;
	}
	void Value::Read(const std::string &_json)
	{
		std::string json;
		RemoveWhitespace(_json, json);

		if (json.at(0) == '"' && json.at(json.size()-1) == '"')
		{
			valueStr = json.substr(1, json.size()-2);
			type = VT_STRING;
		}
		else if (json == "true" || json == "false")
		{
			valueStr = json;
			type = VT_BOOL;
		}
		else
		{
			bool onlyNumbers = true;
			bool point = false;

			for (std::string::const_iterator it = json.begin(); it != json.end(); ++it)
			{
				char c = (*it);

				if (c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9' && c != '.' && c != '-')
				{
					onlyNumbers = false;
				}
				else if (c == '.')
				{
					point = true;
				}
			}

			if (onlyNumbers)
			{
				if (point)
				{
					valueStr = json;
					type = VT_DOUBLE;
				}
				else
				{
					valueStr = json;
					type = VT_INT;
				}
			}
			else
			{
				valueStr = "";
				type = VT_NULL;
			}
		}
	}

	Node *Value::GetCopy() const
	{
		return new Value(*this);
	}


	Object::Object()
	{
	}
	Object::Object(const Object &other)
	{
		for (ChildList::const_iterator it = other.children.begin(); it != other.children.end(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			children.push_back(std::make_pair(name, value.GetCopy()));
		}
	}
	Object::Object(const Node &other)
	{
		const Object &object = other.AsObject();

		for (ChildList::const_iterator it = object.children.begin(); it != object.children.end(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			children.push_back(std::make_pair(name, value.GetCopy()));
		}
	}
	Object::~Object()
	{
		Clear();
	}

	Node::Type Object::GetType() const
	{
		return T_OBJECT;
	}

	void Object::Add(const std::string &name, Node &node)
	{
		children.push_back(std::make_pair(name, node.GetCopy()));
	}
	void Object::Add(const std::string &name, Value node)
	{
		children.push_back(std::make_pair(name, new Value(node)));
	}
	void Object::Remove(const std::string &name)
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			if ((*it).first == name)
			{
				delete (*it).second;
				children.erase(it);
				break;
			}
		}
	}
	void Object::Clear()
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			delete (*it).second;
		}
		children.clear();
	}

	Object::iterator Object::begin()
	{
		if (children.size() > 0)
			return Object::iterator(&children.front());
		else
			return Object::iterator(NULL);
	}
	Object::iterator Object::end()
	{
		if (children.size() > 0)
			return Object::iterator(&children.back()+1);
		else
			return Object::iterator(NULL);
	}

	unsigned int Object::GetCount() const
	{
		return children.size();
	}
	Node &Object::Get(const std::string &name) const
	{
		Value value;
		return Get(name, value);
	}
	Node &Object::Get(const std::string &name, Node &def) const
	{
		for (ChildList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			if ((*it).first == name)
			{
				return *(*it).second;
			}
		}
		return def;
	}

	std::string Object::Write(const Format &format, unsigned int level) const
	{
		FormatInterpreter fi(format);

		std::string json;
		json += "{" + fi.GetNewline();

		for (ChildList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			if (it != children.begin())
				json += "," + fi.GetNewline();
			json += fi.GetIndentation(level+1) + "\""+name+"\"" + ":" + fi.GetSpacing() + value.Write(format, level+1);
		}

		json += fi.GetNewline() + fi.GetIndentation(level) + "}";
		return json;
	}
	void Object::Read(const std::string &_json)
	{
		std::string json;
		RemoveWhitespace(_json, json);

		std::string name;
		std::string value;
		bool atName = true;
		int numOpen = 0;
		bool inString = false;

		for (std::string::const_iterator it = json.begin(); it != json.end(); ++it)
		{
			const char &c = (*it);

			if (c == '{' || c == '[')
			{
				++numOpen;
			}
			else if (c == '}' || c == ']')
			{
				--numOpen;
			}
			else if (c == '"')
			{
				inString = !inString;
			}

			if (atName)
			{
				if (!inString && c == ':')
					atName = false;
				else if (c != '"' && c != '{' && c != '}')
					name += c;
			}
			else
			{
				if (((numOpen == 1 && !inString && c == ','))||(numOpen == 0))
				{
					if (!value.empty())
					{
						Node *node = NULL;
						switch (Node::DetermineType(value))
						{
						case T_VALUE  : node = new Value;  break;
						case T_OBJECT : node = new Object; break;
						case T_ARRAY  : node = new Array;  break;
						}
						node->Read(value);
						Add(name, *node);
						delete node;
					}

					name.clear();
					value.clear();
					atName = true;
				}
				else
					value += c;
			}
		}
	}

	Node *Object::GetCopy() const
	{
		return new Object(*this);
	}


	Array::Array()
	{
	}
	Array::Array(const Array &other)
	{
		for (ChildList::const_iterator it = other.children.begin(); it != other.children.end(); ++it)
		{
			const Node &value = *(*it);

			children.push_back(value.GetCopy());
		}
	}
	Array::Array(const Node &other)
	{
		const Array &array = other.AsArray();

		for (ChildList::const_iterator it = array.children.begin(); it != array.children.end(); ++it)
		{
			const Node &value = *(*it);

			children.push_back(value.GetCopy());
		}
	}
	Array::~Array()
	{
		Clear();
	}

	Node::Type Array::GetType() const
	{
		return T_ARRAY;
	}

	void Array::Add(Node &node)
	{
		children.push_back(node.GetCopy());
	}
	void Array::Add(Value node)
	{
		children.push_back(new Value(node));
	}
	void Array::Remove(unsigned int index)
	{
		if (index < children.size())
		{
			ChildList::iterator it = children.begin()+index;
			delete (*it);
			children.erase(it);
		}
	}
	void Array::Clear()
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			delete (*it);
		}
		children.clear();
	}

	Array::iterator Array::begin()
	{
		if (children.size() > 0)
			return Array::iterator(&children.front());
		else
			return Array::iterator(NULL);
	}
	Array::iterator Array::end()
	{
		if (children.size() > 0)
			return Array::iterator(&children.back()+1);
		else
			return Array::iterator(NULL);
	}

	unsigned int Array::GetCount() const
	{
		return children.size();
	}
	Node &Array::Get(unsigned int index) const
	{
		Value value;
		return Get(index, value);
	}
	Node &Array::Get(unsigned int index, Node &def) const
	{
		if (index < children.size())
			return *children.at(index);
		else
			return def;
	}

	std::string Array::Write(const Format &format, unsigned int level) const
	{
		FormatInterpreter fi(format);

		std::string json;
		json += "[" + fi.GetNewline();

		for (ChildList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			Node &value = *(*it);

			if (it != children.begin())
				json += "," + fi.GetNewline();
			json += fi.GetIndentation(level+1) + value.Write(format, level+1);
		}

		json += fi.GetNewline() + fi.GetIndentation(level) + "]";
		return json;
	}
	void Array::Read(const std::string &_json)
	{
		std::string json;
		RemoveWhitespace(_json, json);

		std::string value;
		int numOpen = 0;
		bool inString = false;

		for (std::string::const_iterator it = json.begin(); it != json.end(); ++it)
		{
			const char &c = (*it);

			if (c == '{' || c == '[')
			{
				++numOpen;
			}
			else if (c == '}' || c == ']')
			{
				--numOpen;
			}
			else if (c == '"')
			{
				inString = !inString;
			}

			if (((numOpen == 1 && !inString && c == ','))||(numOpen == 0))
			{
				if (!value.empty())
				{
					Node *node = NULL;
					switch (Node::DetermineType(value))
					{
					case T_VALUE  : node = new Value;  break;
					case T_OBJECT : node = new Object; break;
					case T_ARRAY  : node = new Array;  break;
					}
					node->Read(value);
					Add(*node);
					delete node;
				}

				value.clear();
			}
			else
			{
				if (numOpen > 1 || (c != '[' && c != ']'))
				value += c;
			}
		}
	}

	Node *Array::GetCopy() const
	{
		return new Array(*this);
	}


	FileWriter::FileWriter()
	{
	}
	FileWriter::~FileWriter()
	{
	}

	void FileWriter::WriteFile(const std::string &filename, Node &root, const Format &format)
	{
		FileWriter writer;
		writer.Write(filename, root, format);
	}

	void FileWriter::Write(const std::string &filename, Node &root, const Format &format)
	{
		std::fstream file(filename.c_str(), std::ios::out | std::ios::trunc);
		file << root.Write(format);
		file.close();
	}


	FileReader::FileReader(const std::string &filename)
	{
		std::fstream file(filename.c_str(), std::ios::in);

		std::string rawjson = "";

		std::string line;
		while (!file.eof())
		{
			std::getline(file, line);
			rawjson += line + '\n';
		}

		RemoveWhitespace(rawjson, json);
	}
	FileReader::~FileReader()
	{
	}

	void FileReader::ReadFile(const std::string &filename, Node &node)
	{
		FileReader reader(filename);
		reader.Read(node);
	}

	void FileReader::Read(Node &node)
	{
		if (DetermineType() == node.GetType())
		{
			node.Read(json);
		}
		else
		{
			throw TypeException();
		}
	}

	Node::Type FileReader::DetermineType()
	{
		return Node::DetermineType(json);
	}
}