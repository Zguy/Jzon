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
	}
	Array &Node::AsArray()
	{
		if (IsArray())
			return static_cast<Array&>(*this);
	}
	Value &Node::AsValue()
	{
		if (IsValue())
			return static_cast<Value&>(*this);
	}

	NodePtr Node::Read(const std::string &json)
	{
		if (json.size() == 0)
			return NodePtr(new Value());

		std::string node;
		Type lookFor;
		int numOpen = 0;

		switch (json.at(0))
		{
		case '{' : lookFor = T_OBJECT; break;
		case '[' : lookFor = T_ARRAY; break;
		default : lookFor = T_VALUE; break;
		}

		if (lookFor == T_VALUE)
			return Value::Read(json);

		for (std::string::const_iterator it = json.cbegin(); it != json.cend(); ++it)
		{
			char c = (*it);

			node += c;

			char openToken, closeToken;
			if (lookFor == T_OBJECT)
			{
				openToken = '{';
				closeToken = '}';
			}
			else if (lookFor == T_ARRAY)
			{
				openToken = '[';
				closeToken = ']';
			}

			if (c == openToken)
			{
				++numOpen;
			}
			else if (c == closeToken)
			{
				--numOpen;

				if (numOpen == 0)
				{
					if (lookFor == T_OBJECT)
						return Object::Read(node);
					else if (lookFor == T_ARRAY)
						return Array::Read(node);
				}
			}
		}

		return NodePtr(new Value());
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

	bool Value::IsNull() const
	{
		return (type == VT_NULL);
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
			std::stringstream sstr(valueStr);
			int val;
			sstr >> val;
			return val;
		}
	}
	double Value::AsDouble() const
	{
		if (IsNull())
			return 0.0;
		else
		{
			std::stringstream sstr(valueStr);
			double val;
			sstr >> val;
			return val;
		}
	}
	bool Value::AsBool() const
	{
		if (IsNull())
			return false;
		else
			return (valueStr == "true");
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

	std::string Value::Write() const
	{
		std::string value;
		if (type == VT_NULL)
			value = "null";
		else if (type == VT_STRING)
			value = "\""+valueStr+"\"";
		else
			value = valueStr;
		return value;
	}
	NodePtr Value::Read(const std::string &json)
	{
		ValuePtr value(new Value);

		if (json.front() == '"' && json.back() == '"')
		{
			value->valueStr = json.substr(1, json.size()-2);
			value->type = VT_STRING;
		}
		else if (json == "true" || json == "false")
		{
			value->valueStr = json;
			value->type = VT_BOOL;
		}
		else
		{
			bool onlyNumbers = true;
			bool point = false;

			for (std::string::const_iterator it = json.cbegin(); it != json.cend(); ++it)
			{
				char c = (*it);

				if (c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9' && c != '.')
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
					value->valueStr = json;
					value->type = VT_DOUBLE;
				}
				else
				{
					value->valueStr = json;
					value->type = VT_INT;
				}
			}
			else
			{
				value->valueStr = "";
				value->type = VT_NULL;
			}
		}

		return value;
	}

	NodePtr Value::GetCopy() const
	{
		return NodePtr(new Value(*this));
	}


	Object::Object()
	{
	}
	Object::Object(const Object &other)
	{
		for (ChildList::const_iterator it = other.children.cbegin(); it != other.children.cend(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			children.push_back(std::make_pair<std::string, NodePtr>(name, value.GetCopy()));
		}
	}
	Object::~Object()
	{
	}

	Node::Type Object::GetType() const
	{
		return T_OBJECT;
	}

	void Object::Add(const std::string &name, Node &node)
	{
		children.push_back(std::make_pair<std::string, NodePtr>(name, node.GetCopy()));
	}
	void Object::Add(const std::string &name, NodePtr node)
	{
		children.push_back(std::make_pair<std::string, NodePtr>(name, node->GetCopy()));
	}
	void Object::Add(const std::string &name, Value node)
	{
		children.push_back(std::make_pair<std::string, NodePtr>(name, node.GetCopy()));
	}
	void Object::Remove(const std::string &name)
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			if ((*it).first == name)
			{
				children.erase(it);
				break;
			}
		}
	}

	Object::Iterator Object::Begin()
	{
		if (children.size() > 0)
			return Object::Iterator(&children.front());
		else
			return Object::Iterator(NULL);
	}
	Object::Iterator Object::End()
	{
		if (children.size() > 0)
			return Object::Iterator(&children.back()+1);
		else
			return Object::Iterator(NULL);
	}

	Node &Object::Get(const std::string &name, Node &default) const
	{
		for (ChildList::const_iterator it = children.cbegin(); it != children.cend(); ++it)
		{
			if ((*it).first == name)
			{
				return *(*it).second;
			}
		}
		return default;
	}

	std::string Object::Write() const
	{
		std::string json;
		json += "{";

		for (ChildList::const_iterator it = children.cbegin(); it != children.cend(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			if (it != children.cbegin())
				json += ",";
			json += "\""+name+"\"" + ":" + value.Write();
		}

		json += "}";
		return json;
	}
	NodePtr Object::Read(const std::string &json)
	{
		ObjectPtr object(new Object);
		std::string name;
		std::string value;
		bool atName = true;
		int numOpen = 0;
		bool inString = false;

		for (std::string::const_iterator it = json.cbegin(); it != json.cend(); ++it)
		{
			char c = (*it);

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
						NodePtr node = Node::Read(value);
						object->Add(name, *node);
					}

					name.clear();
					value.clear();
					atName = true;
				}
				else
					value += c;
			}
		}

		return object;
	}

	NodePtr Object::GetCopy() const
	{
		return NodePtr(new Object(*this));
	}


	Array::Array()
	{
	}
	Array::Array(const Array &other)
	{
		for (ChildList::const_iterator it = other.children.cbegin(); it != other.children.cend(); ++it)
		{
			const Node &value = *(*it);

			children.push_back(value.GetCopy());
		}
	}
	Array::~Array()
	{
	}

	Node::Type Array::GetType() const
	{
		return T_ARRAY;
	}

	void Array::Add(Node &node)
	{
		children.push_back(node.GetCopy());
	}
	void Array::Add(NodePtr node)
	{
		children.push_back(node->GetCopy());
	}
	void Array::Add(Value node)
	{
		children.push_back(node.GetCopy());
	}
	void Array::Remove(unsigned int index)
	{
		if (index < children.size())
			children.erase(children.begin()+index);
	}

	Array::Iterator Array::Begin()
	{
		if (children.size() > 0)
			return Array::Iterator(&children.front());
		else
			return Array::Iterator(NULL);
	}
	Array::Iterator Array::End()
	{
		if (children.size() > 0)
			return Array::Iterator(&children.back()+1);
		else
			return Array::Iterator(NULL);
	}

	unsigned int Array::GetCount() const
	{
		return children.size();
	}
	Node &Array::Get(unsigned int index, Node &default) const
	{
		if (index < children.size())
			return *children.at(index);
		else
			return default;
	}

	std::string Array::Write() const
	{
		std::string json;
		json += "[";

		for (ChildList::const_iterator it = children.cbegin(); it != children.cend(); ++it)
		{
			Node &value = *(*it);

			if (it != children.cbegin())
				json += ",";
			json += value.Write();
		}

		json += "]";
		return json;
	}
	NodePtr Array::Read(const std::string &json)
	{
		ArrayPtr array(new Array);
		std::string value;
		int numOpen = 0;
		bool inString = false;

		for (std::string::const_iterator it = json.cbegin(); it != json.cend(); ++it)
		{
			char c = (*it);

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
					NodePtr node = Node::Read(value);
					array->Add(*node);
				}

				value.clear();
			}
			else
			{
				if (numOpen > 1 || (c != '[' && c != ']'))
				value += c;
			}
		}

		return array;
	}

	NodePtr Array::GetCopy() const
	{
		return NodePtr(new Array(*this));
	}


	FileWriter::FileWriter()
	{
	}
	FileWriter::~FileWriter()
	{
	}

	void FileWriter::WriteFile(const std::string &filename, Node &root)
	{
		FileWriter writer;
		writer.Write(filename, root);
	}
	void FileWriter::WriteFile(const std::string &filename, NodePtr root)
	{
		WriteFile(filename, *root);
	}

	void FileWriter::Write(const std::string &filename, Node &root)
	{
		std::fstream file(filename.c_str(), std::ios::out | std::ios::trunc);
		file << root.Write();
		file.close();
	}
	void FileWriter::Write(const std::string &filename, NodePtr root)
	{
		Write(filename, *root);
	}


	FileReader::FileReader()
	{
	}
	FileReader::~FileReader()
	{
	}

	NodePtr FileReader::ReadFile(const std::string &filename)
	{
		FileReader reader;
		return reader.Read(filename);
	}

	NodePtr FileReader::Read(const std::string &filename)
	{
		std::string json;
		std::fstream file(filename.c_str(), std::ios::in);
		if (!file.is_open())
			return NodePtr(new Value());

		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			json += line + '\n';
		}

		RemoveWhitespace(json);

		return Node::Read(json);
	}

	void FileReader::RemoveWhitespace(std::string &json)
	{
		std::string freshJson;

		bool comment = false;
		int multicomment = 0;
		bool inString = false;

		for (std::string::iterator it = json.begin(); it != json.end(); ++it)
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

		json = freshJson;
	}
}