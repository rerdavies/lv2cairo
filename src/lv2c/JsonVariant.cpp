/*
 * MIT License
 *
 * Copyright (c) 2023 Robin E. R. Davies
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "lv2c/JsonVariant.hpp"
#include "lv2c/JsonIo.hpp"
#include <limits>
#include <cmath>
#include <cstddef>
#include <memory>
#include <sstream>

using namespace lvtk;

json_variant::~json_variant()
{
    free();
}

void json_variant::free()
{
    // in-place deletion.
    switch (content_type)
    {
    case ContentType::String:
        memString().std::string::~string();
        break;
    case ContentType::Object:
        memObject().std::shared_ptr<json_object>::~shared_ptr<json_object>();
        break;
    case ContentType::Array:
        memArray().std::shared_ptr<json_array>::~shared_ptr<json_array>();
        break;
    default:
        break;
    }
    content_type = ContentType::Null;
}

json_variant::json_variant(json_variant &&other)
{
    this->content_type = ContentType::Null;
    switch (other.content_type)
    {
    case ContentType::Null:
        break;
    case ContentType::Bool:
        this->content.bool_value = other.content.bool_value;
        break;
    case ContentType::Number:
        this->content.double_value = other.content.double_value;
        break;
    case ContentType::String:
        new (content.mem) std::string();
        std::swap(this->memString(),other.memString());
        this->content_type = ContentType::String;
        return;
    case ContentType::Object:
        new (content.mem) std::shared_ptr<json_object>();
        std::swap(this->memObject(),other.memObject());
        this->content_type = ContentType::Object;
        return;
    case ContentType::Array:
        new (content.mem) std::shared_ptr<json_array>();
        std::swap(this->memArray(),other.memArray());
        this->content_type = ContentType::Array;
        return;
    }
    this->content_type = other.content_type;
    other.content_type = ContentType::Null;
}

json_variant &json_variant::operator=(const json_variant &other)
{
    free();
    switch (other.content_type)
    {
    case ContentType::Null:
        break;
    case ContentType::Bool:
        this->content.bool_value = other.content.bool_value;
        break;
    case ContentType::Number:
        this->content.double_value = other.content.double_value;
        break;
    case ContentType::String:
        new (content.mem) std::string(other.memString());
        break;
    case ContentType::Object:
        new (content.mem) std::shared_ptr<json_object>{other.memObject()};
        break;
    case ContentType::Array:
        new (content.mem) std::shared_ptr<json_array>{other.memArray()};
        this->content_type = ContentType::Array;
        break;
    }
    this->content_type = other.content_type;
    return *this;
}

json_variant::json_variant(const json_variant &other)
{
    this->content_type = ContentType::Null;
    switch (other.content_type)
    {
    case ContentType::Null:
        break;
    case ContentType::Bool:
        this->content.bool_value = other.content.bool_value;
        break;
    case ContentType::Number:
        this->content.double_value = other.content.double_value;
        break;
    case ContentType::String:
        new (content.mem) std::string(other.memString());
        this->content_type = ContentType::String;
        return;
    case ContentType::Object:
        new (content.mem) std::shared_ptr<json_object>{other.memObject()};
        this->content_type = ContentType::Object;
        return;
    case ContentType::Array:
        new (content.mem) std::shared_ptr<json_array>{other.memArray()};
        this->content_type = ContentType::Array;
        return;
    }
    this->content_type = other.content_type;
}

bool json_array::operator==(const json_array &other) const
{
    if (!(this->size() == other.size()))
        return false;
    for (size_t i = 0; i < this->size(); ++i)
    {
        if (!((*this)[i] == other[i]))
            return false;
    }
    return true;
}

void json_array::read(json_reader &reader)
{
    
    reader.consume('[');
    if (reader.peek() != ']')
    {
        while (true) {
            json_variant v;
            v.read(reader);
            values.push_back(std::move(v));
            if (reader.peek() != ',')
            {
                break;
            }
            reader.consume(',');
        }
    }
    reader.consume(']');
}

json_object::iterator json_object::find(const std::string &key)
{
    for (auto i = begin(); i != end(); ++i)
    {
        if (i->first == key)
        {
            return i;
        }
    }
    return end();
}
json_object::const_iterator json_object::find(const std::string &key) const
{
    for (auto i = begin(); i != end(); ++i)
    {
        if (i->first == key)
        {
            return i;
        }
    }
    return end();
}

bool json_object::operator==(const json_object &other) const
{

    for (const auto &pair : this->values)
    {
        auto index = other.find(pair.first);
        if (index == other.end())
            return false;
        if (!(index->second == pair.second))
            return false;
    }
    for (const auto &pair : other.values)
    {
        auto index = this->find(pair.first);
        if (index == this->end())
            return false;
        if (!(index->second == pair.second))
            return false;
    }
    return true;
}

void json_object::read(json_reader &reader)
{
    reader.start_object();

    if (reader.peek() != '}')
    {

        while (true)
        {
            std::string key;
            json_variant value;
            reader.read(&key);
            reader.consume(':');
            value.read(reader);

            (*this)[key] = value;
            if (reader.peek() == ',')
            {
                reader.consume(',');
            } else {
                break;
            }
        }
    }

    reader.end_object();
}

json_variant &json_array::at(size_t index)
{
    check_index(index);
    return values.at(index);
}
const json_variant &json_array::at(size_t index) const
{
    check_index(index);
    return values.at(index);
}

size_t json_variant::size() const
{
    if (content_type == ContentType::Array)
        return as_array()->size();
    if (content_type == ContentType::Object)
    {
        return as_object()->size();
    }
    throw std::runtime_error("Not supported.");
}

json_variant &json_object::at(const std::string &index)
{
    for (auto &entry : values)
    {
        if (entry.first == index)
        {
            return entry.second;
        }
    }
    throw std::runtime_error("Not found.");
    values.push_back(std::pair(index, json_variant()));
    return values[values.size() - 1].second;
}
const json_variant &json_object::at(const std::string &index) const
{
    for (const auto &entry : values)
    {
        if (entry.first == index)
        {
            return entry.second;
        }
    }
    throw std::runtime_error("Not found.");
}

json_variant &json_object::operator[](const std::string &index)
{
    for (auto &entry : values)
    {
        if (entry.first == index)
        {
            return entry.second;
        }
    }
    values.push_back(std::pair<std::string, json_variant>(index, json_variant()));
    return values[values.size() - 1].second;
}
const json_variant &json_object::operator[](const std::string &index) const
{
    return at(index);
}
bool json_object::contains(const std::string &index) const
{
    return find(index) != end();
}

json_variant &json_variant::operator=(bool value)
{
    free();
    this->content_type = ContentType::Bool;
    this->content.bool_value = value;
    return *this;
}

json_variant &json_variant::operator=(double value)
{
    free();
    this->content_type = ContentType::Number;
    this->content.double_value = value;
    return *this;
}
json_variant &json_variant::operator=(const std::string &value)
{
    free();
    this->content_type = ContentType::String;
    new (this->content.mem) std::string(value); // in-place constructor.
    return *this;
}
json_variant &json_variant::operator=(std::string &&value)
{
    free();
    this->content_type = ContentType::String;
    new (this->content.mem) std::string(std::move(value)); // in-place constructor.
    return *this;
}
json_variant &json_variant::operator=(json_object &&value)
{
    free();
    new (this->content.mem) std::shared_ptr<json_object>{new json_object(std::move(value))};
    this->content_type = ContentType::Object;
    return *this;
}
json_variant &json_variant::operator=(json_array &&value)
{
    free();
    this->content_type = ContentType::Array;
    new (this->content.mem) std::shared_ptr<json_array>{new json_array(std::move(value))};
    return *this;
}
json_variant &json_variant::operator=(json_variant &&value)
{
    if (this->content_type == value.content_type)
    {
        switch (this->content_type)
        {
        case ContentType::String:
            std::swap(this->memString(), value.memString());
            break;
        case ContentType::Array:
            std::swap(this->memArray(), value.memArray());
            break;
        case ContentType::Object:
            std::swap(this->memObject(), value.memObject());
            break;
        case ContentType::Null:
            break;
        default:
            // undifferentiated copy of POD types.
            *(uint64_t *)(this->content.mem) = *(uint64_t *)(value.content.mem);
            break;
        }
        return *this;
    }
    free();
    switch (value.content_type)
    {
    case ContentType::String:
        new (content.mem) std::string(std::move(value.memString()));
        break;
    case ContentType::Object:
        new (content.mem) std::shared_ptr<json_object>(std::move(value.memObject()));
        break;
    case ContentType::Array:
        new (content.mem) std::shared_ptr<json_array>(std::move(value.memArray()));
        break;
    default:
        // undifferentiated copy of POD types.
        *(uint64_t *)(this->content.mem) = *(uint64_t *)(value.content.mem);
        break;
    }
    this->content_type = value.content_type;
    return *this;
}

bool json_variant::operator==(const json_variant &other) const
{
    if (this->content_type != other.content_type)
        return false;

    switch (this->content_type)
    {
    case ContentType::Null:
        return true;
    case ContentType::Bool:
        return as_bool() == other.as_bool();
        break;
    case ContentType::Number:
        return as_number() == other.as_number();
    case ContentType::String:
        return as_string() == other.as_string();
    case ContentType::Array:
        return *(as_array().get()) == *(other.as_array().get());
    case ContentType::Object:
        return *(as_object().get()) == *(other.as_object().get());
        ;
    default:
        throw std::runtime_error("Invalid content_type.");
    }
}
json_array::json_array(json_array &&other)
    : values(std::move(other.values))
{
    ++allocation_count_;
}
json_object::json_object(json_object &&other)
    : values(std::move(other.values))
{
    ++allocation_count_;
}

bool json_variant::contains(const std::string &index) const
{
    if (is_object())
    {
        return as_object()->contains(index);
    }
    return false;
}


json_variant::json_variant(const char *sz)
    : json_variant(std::string(sz))
{
}

void json_variant::read(json_reader &reader)
{
    int v = reader.peek();
    if (v == '[')
    {
        json_array array;
        array.read(reader);
        (*this) = std::move(array);
    }
    else if (v == '{')
    {
        json_object object;
        object.read(reader);
        (*this) = std::move(object);
    }
    else if (v == '\"')
    {
        std::string s;
        reader.read(&s);
        (*this) = std::move(s);
    }
    else if (v == 'n')
    {
        reader.read_null();
        (*this) = json_null();
    }
    else if (v == 't' || v == 'f')
    {
        bool b;
        reader.read(&b);
        (*this) = b;
    }
    else
    {
        // it's a number.
        double v;
        reader.read(&v);
        (*this) = v;
    }
}
void json_variant::write(json_writer &writer) const
{
    switch (content_type)
    {
    case ContentType::Null:
        writer.write_null();
        break;
    case ContentType::Bool:
        writer.write(as_bool());
        break;
    case ContentType::Number:
        writer.write(as_number());
        break;
    case ContentType::String:
        writer.write(as_string());
        break;
    case ContentType::Object:
        memObject()->write(writer);
        break;
    case ContentType::Array:
        memArray()->write(writer);
        break;
    default:
        throw std::runtime_error("Invalid variant type");
    }
}

std::string json_variant::to_string() const
{
    std::stringstream ss;
    json_writer writer(ss);
    this->write(writer);
    return ss.str();
}


static bool is_all_simple_types(const json_array*array)
{
    for (size_t i= 0; i < array->size(); ++i)
    {
        const json_variant &v = array->at(i);
        if (v.is_number()
        || v.is_bool() 
        || v.is_string()
        || v.is_null())
        {
            // then we're good.
        } else {
            return false;
        }
    }
    return true;
}

void json_array::write(json_writer &writer) const
{
    // zero length arrays get written on one line.
    if (size() == 0)
    {
        writer.write_raw("[]");
        return;
    }
    // arrays with no structured elements get written on one line.
    if (is_all_simple_types(this) )
    {
        // write on a single line.
        writer.write_raw("[");
        for (size_t i = 0; i < size(); ++i)
        {
            if (i != 0)
            {
                writer.write_raw(",");
                writer.needs_space(true);
            }
            values[i].write(writer);
        }
        writer.write_raw("]");
        return;
    }
    writer.start_array();
    bool first = true;
    if (size() != 0)
    {
        for (auto &value : values)
        {
            if (!first) {
                writer.write_raw(",");
                writer.endl();
            }
            first = false;
            value.write(writer);
        }
        writer.endl();
    }
    writer.end_array();
}

void json_object::write(json_writer &writer) const
{
    writer.start_object();
    bool first = true;
    if (values.size() != 0)
    {
        for (auto &value : values)
        {
            if (!first)
            {
                writer.write_raw(",");
                writer.endl();
            }
            first = false;
            writer.check_indent();
            writer.write(value.first);
            writer.write_raw(":");
            writer.needs_space(true);
            value.second.write(writer);
        }
        writer.endl();
    }
    writer.end_object();
}


/*static*/
 json_variant  json_variant::array()
 {
    return json_variant(std::make_shared<json_array>());
 }
/*static*/
 json_variant  json_variant::object()
 {
    return json_variant(std::make_shared<json_object>());
 }


/*static*/ json_null json_null::instance;
/*static*/ int64_t json_array::allocation_count_ = 0;  // strictly for testing purposes. not thread safe.
/*static*/ int64_t json_object::allocation_count_ = 0; // strictly for testing purposes. not thread safe.
