// Copyright (c) 2022 Robin Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <cmath>

#include "JsonVariant.hpp"

namespace lvtk
{

    class json_exception : public std::runtime_error
    {
    public:
        json_exception(const std::string &message) : std::runtime_error(message) {}
        json_exception(const char *message) : std::runtime_error(message) {}
    };

    class json_reader
    {
    public:
        json_reader(std::istream &input);

        bool allowNaN() const { return allowNaN_; }
        void allowNaN(bool allow) { allowNaN_ = allow; }

        void read_object_start();
        void consume(const char *str);

        void start_object() { consume('{'); }
        void end_object() { consume('}'); }
        void consume(char expected);
        int peek();

        bool is_complete();

        std::string read_string();
        bool read_boolean();

    public:
        void read_null();

        void read(bool *value);

        void read(std::string *value);

        void read(double *value);

    private:
        void throw_format_error(const std::string &emssage);
        void throw_format_error();
        void skip_whitespace();
        bool is_whitespace(char c);
        char get();
        uint16_t read_hex();
        char16_t read_u_escape();
        void consume_token(const char*token, const char*errorMessage);
    private:

        bool allowNaN_ = true;
        std::istream &s;
    };

    class json_writer
    {
    public:
        json_writer(std::ostream &s, bool compressed = false);
        void set_allow_nan(bool value);
        void set_precision(int precision);
        void write_null();
        void write(bool value);
        void write(const std::string &value);
        void write(double value);
        void write_raw(const char *text);
        
        void check_indent();
        void check_space();

        void indent();
        void unindent();
        void endl();
        void start_array();
        void end_array();
        void start_object();
        void end_object();
        void needs_space(bool value);

    private:
        int precision = 15;
        int64_t indent_count = 0;
        bool new_line = false;
        bool needs_space_ = true;
        std::ostream &s;
        bool compressed = true;
        bool allow_nan = false;
    };

    inline std::istream&operator >>(std::istream&s,json_variant&variant)
    {
        json_reader reader{s};
        variant.read(reader);
        return s;

    }
    inline std::ostream &operator <<(std::ostream&s,const json_variant&variant)
    {
        json_writer writer{s};
        variant.write(writer);
        return s;

    }

    /////////////////////////////////////////////////////////////

    inline bool json_reader::is_whitespace(char c)
    {
        switch (c)
        {
        case 0x20:
        case 0x0A:
        case 0x0D:
        case 0x09:
            return true;
        default:
            return false;
        }
    }

    inline char json_reader::get()
    {
        int ic = s.get();
        if (ic == -1)
            throw_format_error("Unexpected end of file");
        return (char)ic;
    }

    inline int json_reader::peek()
    {
        skip_whitespace();
        return s.peek();
    }


}