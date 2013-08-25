#pragma once

#include <list>
#include <sstream>
#include <ostream>
#include <memory>
#include <algorithm>

using namespace std;

struct jsonBase
{
	virtual wstring toString() const = 0;
	virtual wostream &serialize(wostream &os) const = 0;
};

wostream &operator <<(wostream &os, jsonBase const &j)
{
	return j.serialize(os);
}

template <typename T> struct jsonProperty
	: jsonBase
{
	wstring name;
	T val;

	jsonProperty(wstring propertyName, T const& val)
		: name(propertyName)
		, val(val)
	{}

	wstring toString() const
	{
		wstringstream ss;
		ss << '\"' << name << "\" : " << val;
		return ss.str();
	}

	virtual wostream &serialize(wostream &os) const
	{
		os << toString();
		return os;
	}
};

template<> struct jsonProperty<wstring> : jsonBase
{
	wstring name;
	wstring val;

	jsonProperty(wstring propertyName, wstring const& val)
		: name(propertyName)
		, val(val)
	{}

	wstring toString() const
	{
		wstringstream ss;
		ss << '\"' << name << "\" : \"" << val << '\"';
		return ss.str();
	}

	virtual wostream &serialize(wostream &os) const
	{
		os << toString();
		return os;
	}
};

struct jsonObject : jsonBase
{
	wstring name;
	list<shared_ptr<jsonBase>> properties;

	jsonObject()
	{}

	jsonObject(wstring name)
		: name(name)
	{}

	jsonObject(jsonObject &&other)
	{
		name = std::move(other.name);
		properties = std::move(other.properties);
	}

	jsonObject(jsonObject const& other)
	{
		if (this != &other)
			*this = other;
	}

	jsonObject &operator=(jsonObject const& other)
	{
		if (this != &other)
		{
			name = other.name;
			properties = other.properties;
		}

		return *this;
	}

	template <typename T>
	void addProperty(wstring name, T const& val)
	{
		properties.push_back(shared_ptr<jsonBase>(new jsonProperty<T>(name, val)));
	}

	wstring toString() const
	{
		wstringstream ss;
		ss << "{" << endl;
		if (!properties.empty())
		{
			ss << *properties.front().get();
			auto begin = properties.begin();
			std::for_each(++begin, properties.end(), [&ss](shared_ptr<jsonBase> j) {
				ss << ",\n" << *j.get();
			});
		}

		ss << endl << "}" << endl;
		return ss.str();
	}

	virtual wostream &serialize(wostream &os) const
	{
		os << toString();
		return os;
	}
};

template <typename T>
struct jsonArray : jsonBase
{
	list<T> values;

	jsonArray() {}

	jsonArray(jsonArray<T> const &other)
	{
		if (this != &other)
			*this = other;
	}

	jsonArray<T> &operator=(jsonArray<T> const &other)
	{
		if (this != &other)
			values = other.values;

		return *this;
	}

	void add(T const& val)
	{
		values.push_back(val);
	}

	void add(T const& val1, T const& val2)
	{
		values.push_back(val1);
		values.push_back(val2);
	}

	void add(T const& val1, T const& val2, T const& val3)
	{
		values.push_back(val1);
		values.push_back(val2);
		values.push_back(val3);
	}

	wstring toString() const
	{
		wstringstream ss;
		ss << "[";
		if (!values.empty())
		{
			ss << values.front();
			auto begin = values.begin();
			for_each(++begin, values.end(), [&ss](T const& val) {
				ss << ", " << val;
			});
		}

		ss << "]";
		return ss.str();
	}

	virtual wostream &serialize(wostream &os) const
	{
		os << toString();
		return os;
	}

	static jsonArray<T> from(T const& val)
	{
		jsonArray<T> ja;
		ja.add(val);
		return ja;
	}

	static jsonArray<T> from(T const& val1, T const& val2)
	{
		jsonArray<T> ja;
		ja.add(val1);
		ja.add(val2);
		return ja;
	}

	static jsonArray<T> from(T const& val1, T const& val2, T const& val3)
	{
		jsonArray<T> ja;
		ja.add(val1);
		ja.add(val2);
		ja.add(val3);
		return ja;
	}

	static jsonArray<T> from(T const& val1, T const& val2, T const& val3, T const& val4, T const& val5, T const& val6)
	{
		jsonArray<T> ja;
		ja.add(val1);
		ja.add(val2);
		ja.add(val3);
		ja.add(val4);
		ja.add(val5);
		ja.add(val6);
		return ja;
	}
};