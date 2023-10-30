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

#pragma once
#include <vector>
#include <variant>
#include <map>
#include <memory>
#include <type_traits>

#include <string>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <unordered_set>
#include <set>
namespace lv2c
{
    class json_reader;
    class json_writer;
    class json_variant;

    template <typename T>
    concept JsonWritable = requires(const T a, json_variant v) {
        {
            a.Write(v)
        };
    };

    template <typename T>
    concept JsonReadable = requires(T a, json_variant v) {
        {
            a.Read(v)
        };
    };

    template <typename T>
    concept JsonRange = requires(T a) {
        {
            a.begin()
        };
        {
            a.end()
        };
        {
            a.end() - a.begin()
        } -> std::convertible_to<std::size_t>;
    };

    template <typename T>
    struct json_argument_traits
    {
        using arg_t = const T &;
        using return_t = T &;
        using const_return_t = const T &;

        using value_t = T;
        using move_t = T &&;
        static constexpr bool move_disabled = false;
        static constexpr bool move_enabled = true;
    };

    template <typename T>
        requires(std::integral<T> || std::floating_point<T> || std::is_enum_v<T>)
    struct json_argument_traits<T>
    {
        using arg_t = T;
        using value_t = T;
        using return_t = T;
        using const_return_t = T;
        using move_t = struct _disabled
        {
        };
        static constexpr bool move_disabled = true;
        static constexpr bool move_enabled = false;
    };
    template <>
    struct json_argument_traits<bool>
    {
        using arg_t = bool;
        using value_t = bool;
        using return_t = bool;
        using const_return_t = bool;
        using move_t = struct _disabled
        {
        };
        static constexpr bool move_disabled = true;
        static constexpr bool move_enabled = false;
    };

    class json_null
    {
    public:
        static json_null instance;
        bool operator==(const json_null &other) const { return true; }
        bool operator!=(const json_null &other) const { return (!((*this) == other)); }

    };

    class json_object;
    class json_array;

    class json_variant
    {
    public:
        enum class ContentType
        {
            Null,
            Bool,
            Number,
            String,
            Object,
            Array
        };
        using object_ptr = std::shared_ptr<json_object>;
        using array_ptr = std::shared_ptr<json_array>;

    private:
    public:
        ~json_variant();
        json_variant();
        json_variant(json_variant &&);
        json_variant(const json_variant &);

        json_variant(json_null value);
        json_variant(bool value);
        json_variant(double value);
        json_variant(const std::string &value);
        json_variant(std::shared_ptr<json_object> &&value);
        json_variant(const std::shared_ptr<json_object> &value);
        json_variant(std::shared_ptr<json_array> &&value);
        json_variant(const std::shared_ptr<json_array> &value);
        json_variant(json_array &&array);
        json_variant(json_object &&object);
        json_variant(const char *sz);

        template <typename T>
        json_variant(const std::vector<T> &);

        template <typename T>
        json_variant(const std::unordered_set<T> &);

        template <typename T>
        json_variant(const std::set<T> &);

        json_variant(const void *) = delete; // do NOT allow implicit conversion of pointers to bool

        static json_variant array();
        static json_variant object();

        json_variant &operator=(json_variant &&value);
        json_variant &operator=(const json_variant &value);
        json_variant &operator=(bool value);
        json_variant &operator=(double value);
        json_variant &operator=(float value);

        template <typename T>
        json_variant &operator=(const std::vector<T> &value);

        template <typename T>
        json_variant &operator=(const std::unordered_set<T> &value);

        template <typename T>
        json_variant &operator=(const std::set<T> &value);

        template <JsonReadable T>
        json_variant &operator=(const T &v);

        template <typename T>
            requires(std::is_integral_v<T> || std::is_enum_v<T>) // resolves integer type overloads.
        json_variant &operator=(T value)
        {
            return *this = (double)value;
        }

        json_variant &operator=(const std::string &value);
        json_variant &operator=(std::string &&value);
        json_variant &operator=(json_object &&value);
        json_variant &operator=(json_array &&value);

        json_variant &operator=(const char *sz) { return (*this) = std::string(sz); }

        json_variant &operator=(void *) = delete; // do NOT allow implicit conversion of pointers to bool

        void require_type(ContentType content_type) const
        {
            if (this->content_type != content_type)
            {
                throw std::runtime_error("Content type is not valid.");
            }
        }
        bool is_null() const { return content_type == ContentType::Null; }
        bool is_bool() const { return content_type == ContentType::Bool; }
        bool is_number() const { return content_type == ContentType::Number; }
        bool is_string() const { return content_type == ContentType::String; }
        bool is_object() const { return content_type == ContentType::Object; }
        bool is_array() const { return content_type == ContentType::Array; }

        bool as_bool() const
        {
            require_type(ContentType::Bool);
            return content.bool_value;
        }

        double as_number() const
        {
            require_type(ContentType::Number);
            return content.double_value;
        }

        const std::string &as_string() const;
        std::string &as_string();

        const std::shared_ptr<json_object> &as_object() const;
        std::shared_ptr<json_object> &as_object();

        const std::shared_ptr<json_array> &as_array() const;
        std::shared_ptr<json_array> &as_array();


        void get(bool *value) const { *value = as_bool(); }
        void get(float *value) const { *value = (float)as_number(); }
        void get(double *value) const { *value = as_number(); }
        void get(std::string *value) const { *value = as_string(); }

        template <typename T>
            requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
        void get(T *value) const
        {
            *value = (T)as_number();
        }

        template <typename T>
        void get(std::vector<T> *value) const;

        template <typename T>
        void get(std::unordered_set<T> *value) const;

        template <typename T>
        void get(std::set<T> *value) const;


        template <typename U>
        U as() const
        {
            U value;
            get(&value);
            return value;
        }

        template <typename U>
        typename json_argument_traits<U>::value_t as(
            typename json_argument_traits<U>::arg_t defaultValue) const
        {
            if (is_null())
                return defaultValue;
            return as<U>();
        }

        template <typename T>
            requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
        inline typename json_argument_traits<T>::const_return_t as() const
        {
            return (T)as_number();
        }

        // convenience methods for object and array manipulation.
        static json_variant make_object();
        static json_variant make_array();

        void resize(size_t size);
        size_t size() const;

        bool contains(const std::string &index) const;

        json_variant &at(size_t index);
        const json_variant &at(size_t index) const;

        json_variant &at(const std::string &index);
        const json_variant &at(const std::string &index) const;

        json_variant &operator[](size_t index);
        const json_variant &operator[](size_t index) const;

        json_variant &operator[](const std::string &index);
        const json_variant &operator[](const std::string &index) const;

        bool operator==(const json_variant &other) const;
        bool operator!=(const json_variant &other) const;

        void read(json_reader &reader);
        void write(json_writer &writer) const;

        std::string to_string() const;

    private:
        void free();

        static constexpr size_t stringSize = sizeof(std::string);
        static constexpr size_t objectSize = sizeof(std::shared_ptr<json_variant>);
        static constexpr size_t memSize = stringSize > objectSize ? stringSize : objectSize;
        union Content
        {
            bool bool_value;
            double double_value;
            float float_value;
            int32_t int32_value;
            uint8_t mem[memSize];
        };

        ContentType content_type = ContentType::Null;
        Content content;

        std::string &memString();
        object_ptr &memObject();
        array_ptr &memArray();

        const std::string &memString() const;
        const object_ptr &memObject() const;
        const array_ptr &memArray() const;
    };
    class json_array
    {
    private:
        json_array(const json_array &) {} // deleted.
    public:
        using ptr = std::shared_ptr<json_array>;

        json_array() { ++allocation_count_; }
        json_array(json_array &&other);
        ~json_array() { --allocation_count_; }

        json_variant &at(size_t index);
        const json_variant &at(size_t index) const;

        json_variant &operator[](size_t index);
        const json_variant &operator[](size_t &index) const;

        void resize(size_t size) { values.resize(size); }
        size_t size() const { return values.size(); }
        void push_back(json_variant &&value) { values.push_back(std::move(value)); }
        template <typename U>
        void push_back(U &&value) { values.push_back(value); }
        void push_back(double value) { values.push_back(json_variant{value}); }
        void push_back(const std::string &value) { values.push_back(json_variant{value}); }
        void push_back(bool value) { values.push_back(json_variant{value}); }
        void push_back(const std::shared_ptr<json_array> &value) { values.push_back(json_variant(value)); }
        void push_back(const std::shared_ptr<json_object> &value) { values.push_back(json_variant(value)); }

        bool operator==(const json_array &other) const;
        bool operator!=(const json_array &other) const { return (!((*this) == other)); }

        // Strictly for testing purposes. Not thread-safe.
        static int64_t allocation_count()
        {
            return allocation_count_;
        }
        using iterator = std::vector<json_variant>::iterator;
        using const_iterator = std::vector<json_variant>::const_iterator;

        iterator begin() { return values.begin(); }
        iterator end() { return values.end(); }
        const_iterator begin() const { return values.begin(); }
        const_iterator end() const { return values.end(); }

        void read(json_reader &reader);
        void write(json_writer &writer) const;

    private:
        static int64_t allocation_count_; // strictly for testing purposes. not thread safe.

        void check_index(size_t size) const;

        std::vector<json_variant> values;
    };
    class json_object
    {
    private:
        json_object(const json_object &) {} // deleted.
    public:
        using ptr = std::shared_ptr<json_object>;

        json_object() { ++allocation_count_; }
        json_object(json_object &&other);
        ~json_object() { --allocation_count_; }

        size_t size() const { return values.size(); }
        json_variant &at(const std::string &index);
        const json_variant &at(const std::string &index) const;

        json_variant &operator[](const std::string &index);
        const json_variant &operator[](const std::string &index) const;

        bool operator==(const json_object &other) const;
        bool operator!=(const json_object &other) const { return (!((*this) == other)); }
        bool contains(const std::string &index) const;

        using values_t = std::vector<std::pair<std::string, json_variant>>;
        using iterator = values_t::iterator;
        using const_iterator = values_t::const_iterator;

        iterator begin() { return values.begin(); }
        iterator end() { return values.end(); }
        const_iterator begin() const { return values.begin(); }
        const_iterator end() const { return values.end(); }

        iterator find(const std::string &key);
        const_iterator find(const std::string &key) const;

        // strictly for testing purposes. Not thread-safe.
        static int64_t allocation_count()
        {
            return allocation_count_;
        }

        void read(json_reader &reader);
        void write(json_writer &writer) const;

    private:
        static int64_t allocation_count_;
        values_t values;
    };

    ////////////////////////////////////////////////

    inline std::string &json_variant::memString() { return *(std::string *)content.mem; }
    inline const std::string &json_variant::memString() const { return *(const std::string *)content.mem; }

    template <>
    inline json_argument_traits<bool>::const_return_t json_variant::as<bool>() const { return as_bool(); }

    template <>
    inline json_argument_traits<float>::const_return_t
    json_variant::as<float>() const
    {
        return (float)as_number();
    }

    template <>
    inline double json_variant::as<double>() const
    {
        return as_number();
    }

    template <>
    inline std::string json_variant::as<std::string>() const { return as_string(); }

    // template <>
    // inline std::string &json_variant::as<std::string>() { return as_string(); }
    // template <>
    // inline json_variant &json_variant::as<json_variant>() { return *this; }

    inline json_variant::object_ptr &json_variant::memObject()
    {
        return *(object_ptr *)content.mem;
    }
    inline json_variant::array_ptr &json_variant::memArray()
    {
        return *(array_ptr *)content.mem;
    }

    inline const json_variant::object_ptr &json_variant::memObject() const
    {
        return *(const object_ptr *)content.mem;
    }
    inline const json_variant::array_ptr &json_variant::memArray() const
    {
        return *(const array_ptr *)content.mem;
    }

    inline json_variant::json_variant(json_array &&array)
    {
        this->content_type = ContentType::Null;
        new (content.mem) std::shared_ptr<json_array>{new json_array(std::move(array))};
        this->content_type = ContentType::Array;
    }
    inline json_variant::json_variant(json_object &&object)
    {
        this->content_type = ContentType::Null;
        new (content.mem) std::shared_ptr<json_object>{new json_object(std::move(object))};
        this->content_type = ContentType::Object;
    }

    inline json_variant::json_variant(const std::string &value)
    {
        this->content_type = ContentType::Null;
        new (content.mem) std::string(value); // placement new.
        content_type = ContentType::String;
    }

    inline json_variant::json_variant(std::shared_ptr<json_object> &&value)
    {
        this->content_type = ContentType::Null;
        new (content.mem) std::shared_ptr<json_object>(std::move(value)); // placement new.
        content_type = ContentType::Object;
    }
    inline json_variant::json_variant(const std::shared_ptr<json_object> &value)
    {
        // don't deep copy!
        std::shared_ptr<json_object> t = const_cast<std::shared_ptr<json_object> &>(value);
        this->content_type = ContentType::Null;
        new (content.mem) std::shared_ptr<json_object>(t); // placement new.
        content_type = ContentType::Object;
    }

    inline json_variant::json_variant(const std::shared_ptr<json_array> &value)
    {
        // Make sure we don't deep copy!
        std::shared_ptr<json_array> t = const_cast<std::shared_ptr<json_array> &>(value);
        this->content_type = ContentType::Null;
        new (content.mem) std::shared_ptr<json_array>(t); // placement new.
        content_type = ContentType::Array;
    }
    inline json_variant::json_variant(array_ptr &&value)
    {
        this->content_type = ContentType::Null;
        new (content.mem) std::shared_ptr<json_array>(std::move(value)); // placement new.
        content_type = ContentType::Array;
    }
    inline json_variant::json_variant()
    {
        content_type = ContentType::Null;
    }
    inline json_variant::json_variant(json_null value)
    {
        content_type = ContentType::Null;
    }
    inline json_variant::json_variant(bool value)
    {
        content_type = ContentType::Bool;
        content.bool_value = value;
    }

    inline json_variant::json_variant(double value)
    {
        content_type = ContentType::Number;
        content.double_value = value;
    }
    inline std::string &json_variant::as_string()
    {
        require_type(ContentType::String);
        return memString();
    }

    inline const std::string &json_variant::as_string() const
    {
        require_type(ContentType::String);
        return memString();
    }

    inline const json_variant::object_ptr &json_variant::as_object() const
    {
        require_type(ContentType::Object);
        return memObject();
    }

    inline json_variant::object_ptr &json_variant::as_object()
    {
        require_type(ContentType::Object);
        return memObject();
    }

    inline const json_variant::array_ptr &json_variant::as_array() const
    {
        require_type(ContentType::Array);
        return memArray();
    }

    inline json_variant::array_ptr &json_variant::as_array()
    {
        require_type(ContentType::Array);
        return memArray();
    }

    inline /*static*/ json_variant json_variant::make_object()
    {
        return json_variant{std::make_shared<json_object>()};
    }
    inline /*static */ json_variant json_variant::make_array()
    {
        return json_variant{std::make_shared<json_array>()};
    }

    inline void json_variant::resize(size_t size)
    {
        as_array()->resize(size);
    }

    inline json_variant &json_variant::at(size_t index)
    {
        return as_array()->at(index);
    }
    inline const json_variant &json_variant::at(size_t index) const
    {
        return as_array()->at(index);
    }

    inline json_variant &json_variant::at(const std::string &index)
    {
        return as_object()->at(index);
    }
    inline const json_variant &json_variant::at(const std::string &index) const
    {
        return as_object()->at(index);
    }

    inline json_variant &json_variant::operator[](size_t index)
    {
        return as_array()->at(index);
    }
    inline const json_variant &json_variant::operator[](size_t index) const
    {
        return as_array()->at(index);
    }

    inline const json_variant &json_variant::operator[](const std::string &index) const
    {
        return (*as_object())[index];
    }
    inline json_variant &json_variant::operator[](const std::string &index)
    {
        return (*as_object())[index];
    }

    inline const json_variant &json_array::operator[](size_t &index) const
    {
        check_index(index);
        return values[index];
    }

    inline json_variant &json_array::operator[](size_t index)
    {
        return at(index);
    }

    inline void json_array::check_index(size_t size) const
    {
        if (size >= values.size())
        {
            throw std::out_of_range("index out of range.");
        }
    }

    inline bool json_variant::operator!=(const json_variant &other) const
    {
        return !(*this == other);
    }

    inline json_variant &json_variant::operator=(float value)
    {
        return *this = (double)value;
    }

    /////////////////////////////////////////
    template <typename T>
    json_variant::json_variant(const std::vector<T> &values)
    {
        *this = std::make_shared<json_array>();
        resize(values.size());
        for (size_t i = 0; i < values.size(); ++i)
        {
            (*this)[i] = values[i];
        }
    }
    template <typename T>
    json_variant::json_variant(const std::unordered_set<T> &values)
    {
        *this = std::make_shared<json_array>();
        resize(values.size());
        int ix = 0;
        for (auto &entry : values)
        {
            (*this)[ix++] = json_variant(entry);
        }
    }

    template <typename T>
    json_variant::json_variant(const std::set<T> &values)
    {
        *this = std::make_shared<json_array>();
        resize(values.size());
        int ix = 0;
        for (auto &entry : values)
        {
            (*this)[ix++] = json_variant(entry);
        }
    }

    template <typename T>
    json_variant &json_variant::operator=(const std::vector<T> &values)
    {
        *this = std::make_shared<json_array>();
        resize(values.size());
        for (size_t i = 0; i < values.size(); ++i)
        {
            (*this)[i] = values[i];
        }
        return *this;
    }

    template <typename T>
    json_variant &json_variant::operator=(const std::unordered_set<T> &values)
    {
        *this = std::make_shared<json_array>();
        resize(values.size());
        int ix = 0;
        for (auto &entry : values)
        {
            (*this)[ix++] = entry;
        }
        return *this;
    }

    template <typename T>
    json_variant &json_variant::operator=(const std::set<T> &values)
    {
        *this = std::make_shared<json_array>();
        resize(values.size());
        int ix = 0;
        for (auto &entry : values)
        {
            (*this)[ix++] = entry;
        }
        return *this;
    }

    template <JsonReadable T>
    json_variant &json_variant::operator=(const T &v)
    {
        v.Write(*this);
        return *this;
    }

    template <typename T>
    void json_variant::get(std::vector<T> *value) const
    {
        value->resize(0);
        value->reserve(size());
        for (size_t i = 0; i < size(); ++i)
        {
            value->push_back((*this)[i].as<T>());
        }
    }

    template <typename T>
    void json_variant::get(std::unordered_set<T> *value) const
    {
        value->clear();
        for (size_t i = 0; i < size(); ++i)
        {
            value->insert((*this)[i].as<T>());
        }
    }

    template <typename T>
    void json_variant::get(std::set<T> *value) const
    {
        value->clear();
        for (size_t i = 0; i < size(); ++i)
        {
            value->insert((*this)[i]);
        }
    }

} // namespace