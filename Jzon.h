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
#ifndef Jzon_h__
#define Jzon_h__

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <iterator>
#include <exception>

namespace Jzon
{
	class Node;
	typedef std::shared_ptr<Node> NodePtr;
	typedef std::pair<std::string, NodePtr> NamedNodePtr;
	typedef std::pair<std::string, Node&> NamedNode;
	class Object;
	typedef std::shared_ptr<Object> ObjectPtr;
	class Array;
	typedef std::shared_ptr<Array> ArrayPtr;
	class Value;
	typedef std::shared_ptr<Value> ValuePtr;
	
	class TypeException : public std::exception
	{
	public:
		TypeException() : std::exception("A Node was used as the wrong type")
		{}
	};
	class ValueException : public std::exception
	{
	public:
		ValueException() : std::exception("A Value was used as the wrong type")
		{}
	};

	struct Format
	{
		bool newline;
		bool spacing;
		bool useTabs;
		unsigned int indentSize;
	};
	static const Format StandardFormat = { true, true, true, 0 };
	static const Format NoFormat = { false, false, false, 0 };

	class Node
	{
	public:
		enum Type
		{
			T_OBJECT,
			T_ARRAY,
			T_VALUE
		};

		Node();
		virtual ~Node();

		virtual Type GetType() const = 0;

		Object &AsObject();
		Array &AsArray();
		Value &AsValue();

		inline bool IsObject() const { return (GetType() == T_OBJECT); }
		inline bool IsArray() const { return (GetType() == T_ARRAY); }
		inline bool IsValue() const { return (GetType() == T_VALUE); }

		virtual std::string Write(const Format &format = NoFormat, unsigned int level = 0) const = 0;
		static NodePtr Read(const std::string &json);

		virtual NodePtr GetCopy() const = 0;
	};

	class Value : public Node
	{
	public:
		enum ValueType
		{
			VT_NULL,
			VT_STRING,
			VT_INT,
			VT_DOUBLE,
			VT_BOOL
		};

		Value();
		Value(const Value &rhs);
		Value(const std::string &value);
		Value(const char *value);
		Value(const int value);
		Value(const double value);
		Value(const bool value);
		virtual ~Value();

		virtual Type GetType() const;
		ValueType GetValueType() const;

		bool IsNull() const;
		std::string AsString() const;
		int AsInt() const;
		double AsDouble() const;
		bool AsBool() const;

		void SetNull();
		void Set(const Value &value);
		void Set(const std::string &value);
		void Set(const char *value);
		void Set(const int value);
		void Set(const double value);
		void Set(const bool value);

		Value &operator=(const Value &rhs);
		Value &operator=(const std::string &rhs);
		Value &operator=(const char *rhs);
		Value &operator=(const int rhs);
		Value &operator=(const double rhs);
		Value &operator=(const bool rhs);

		bool operator==(const Value &other) const;
		bool operator!=(const Value &other) const;

		virtual std::string Write(const Format &format = NoFormat, unsigned int level = 0) const;
		static NodePtr Read(const std::string &json);

		virtual NodePtr GetCopy() const;

	private:
		std::string valueStr;
		ValueType type;
	};

	static const Value null;

	class Object : public Node
	{
	public:
		class Iterator : public std::iterator<std::input_iterator_tag, NamedNodePtr>
		{
		public:
			Iterator(NamedNodePtr *o) : p(o) {}
			Iterator(const Iterator &it) : p(it.p) {}

			Iterator &operator++() { ++p; return *this; }
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }

			bool operator==(const Iterator &rhs) { return p == rhs.p; }
			bool operator!=(const Iterator &rhs) { return p != rhs.p; }

			NamedNode operator*() { return NamedNode(p->first, *p->second); }

		private:
			NamedNodePtr *p;
		};

		Object();
		Object(const Object &other);
		virtual ~Object();

		virtual Type GetType() const;

		void Add(const std::string &name, Node &node);
		void Add(const std::string &name, NodePtr node);
		void Add(const std::string &name, Value node);
		void Remove(const std::string &name);

		Iterator Begin();
		Iterator End();

		Node &Get(const std::string &name, Node &default = Value()) const;

		virtual std::string Write(const Format &format = NoFormat, unsigned int level = 0) const;
		static NodePtr Read(const std::string &json);

		virtual NodePtr GetCopy() const;

	private:
		typedef std::vector<NamedNodePtr> ChildList;
		ChildList children;
	};

	class Array : public Node
	{
	public:
		class Iterator : public std::iterator<std::input_iterator_tag, NodePtr>
		{
		public:
			Iterator(NodePtr *o) : p(o) {}
			Iterator(const Iterator &it) : p(it.p) {}

			Iterator &operator++() { ++p; return *this; }
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }

			bool operator==(const Iterator &rhs) { return p == rhs.p; }
			bool operator!=(const Iterator &rhs) { return p != rhs.p; }

			Node &operator*() { return **p; }

		private:
			NodePtr *p;
		};

		Array();
		Array(const Array &other);
		virtual ~Array();

		virtual Type GetType() const;

		void Add(Node &node);
		void Add(NodePtr node);
		void Add(Value node);
		void Remove(unsigned int index);

		Iterator Begin();
		Iterator End();

		unsigned int GetCount() const;
		Node &Get(unsigned int index, Node &default = Value()) const;

		virtual std::string Write(const Format &format = NoFormat, unsigned int level = 0) const;
		static NodePtr Read(const std::string &json);

		virtual NodePtr GetCopy() const;

	private:
		typedef std::vector<NodePtr> ChildList;
		ChildList children;
	};

	class FileWriter
	{
	public:
		FileWriter();
		~FileWriter();

		static void WriteFile(const std::string &filename, Node &root, const Format &format = NoFormat);
		static void WriteFile(const std::string &filename, NodePtr root, const Format &format = NoFormat);

		void Write(const std::string &filename, Node &root, const Format &format = NoFormat);
		void Write(const std::string &filename, NodePtr root, const Format &format = NoFormat);
	};

	class FileReader
	{
	public:
		FileReader();
		~FileReader();

		static NodePtr ReadFile(const std::string &filename);

		NodePtr Read(const std::string &filename);

	private:
		void RemoveWhitespace(std::string &json);
	};
}

#endif // Jzon_h__