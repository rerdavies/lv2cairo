// Copyright (c) 2023 Robin E. R. Davies
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
#include <limits>
#include <cmath>
#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <optional>

struct _cairo;
typedef struct _cairo cairo_t;

namespace lvtk
{

    class Lv2cDrawingContext;

    class Lv2cSize
    {
    public:
        Lv2cSize();
        Lv2cSize(double width, double height);

        static const Lv2cSize Zero;
        
        bool operator==(const Lv2cSize &other) const { return width == other.width && height == other.height; }
        double Width() const { return width; }
        double Height() const { return height; }
        Lv2cSize &Width(double value)
        {
            width = value;
            return *this;
        }
        Lv2cSize &Height(double value)
        {
            height = value;
            return *this;
        }
        std::string toString() const
        {
            std::stringstream s;

            s << '{' << width << "," << height << '}';
            return s.str();
        }


        Lv2cSize operator+(const Lv2cSize&other) { return Lv2cSize(width+other.width,height+other.height);}
        Lv2cSize operator-(const Lv2cSize&other) { return Lv2cSize(width-other.width,height-other.height);}
        Lv2cSize operator/(double value) const { return Lv2cSize(width/value,height/value); }
        Lv2cSize operator*(double value) const { return Lv2cSize(width*value,height*value); }
        Lv2cSize ceil() const { return Lv2cSize(std::ceil(width),std::ceil(height));}
        Lv2cSize floor() const { return Lv2cSize(std::floor(width),std::floor(height));}

    private:
        double width, height;
    };


    class Lv2cColor;
    class Lv2cHsvColor;

    // Color with linear intensity. r,g,b values are pre-multiplied.
    class Lv2cLinearColor {
    public:
        Lv2cLinearColor() : r(0.0f),g(0.0f),b(0.0f),a(0.0f) { }
        Lv2cLinearColor(float r, float g, float b) : r(r),g(g),b(b),a(1){}
        Lv2cLinearColor(float r, float g, float b, float a) : r(r),g(g),b(b),a(a){}
        explicit Lv2cLinearColor(const Lv2cColor&color);
        explicit Lv2cLinearColor(const Lv2cHsvColor&color);

        static Lv2cLinearColor FromImageSurfaceColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        Lv2cLinearColor FromSrgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

        /// @brief Bulk conversion of CarioLinearColors to sRGB. Private Use.
        static void ToImageSurface(size_t count, const Lv2cLinearColor *source, uint8_t*dest, float scale);
        static void ToImageSurface(const std::vector<Lv2cLinearColor> &source, uint8_t*dest);
        static void FromImageSurface(size_t count, const uint8_t*source, Lv2cLinearColor *dest);
        static void FromImageSurface(const uint8_t*source, std::vector<Lv2cLinearColor> &dest);

        float r,g,b,a;

        Lv2cLinearColor operator+(const Lv2cLinearColor &other) const;
        Lv2cLinearColor operator-(const Lv2cLinearColor &other) const;
        Lv2cLinearColor& operator+=(const Lv2cLinearColor &other);
        Lv2cLinearColor& operator-=(const Lv2cLinearColor &other);
        Lv2cLinearColor operator*(float value) const;
    };

    class Lv2cHsvColor {
    public:
        Lv2cHsvColor(float hue, float saturation, float brightness, float alpha = 1.0f);
        explicit Lv2cHsvColor(const Lv2cLinearColor &linearColor);
        explicit Lv2cHsvColor(const Lv2cColor &linearColor);

        float Hue() const { return hue; }
        float Saturation() const { return saturation; }
        float Brightness() const { return brightness; }
        float Alpha() const { return alpha; }
        bool operator==(const Lv2cHsvColor&other) const;
    private:
        float hue;
        float saturation;
        float brightness;
        float alpha;
    };
    /// Color in sRGB space.
    class Lv2cColor
    {
    public:
        Lv2cColor() : r(0), g(0), b(0), a(1) {}
        Lv2cColor(float r, float g, float b) : r(r), g(g), b(b), a(1) {}
        Lv2cColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        Lv2cColor(const Lv2cColor &color, float a) : r(color.R()), g(color.G()), b(color.B()), a(color.A() * a) {}
        Lv2cColor(const std::string &webColor);
        explicit Lv2cColor(const Lv2cLinearColor &color);
        explicit Lv2cColor(const Lv2cHsvColor&color);

        // @brief Blend two colors.
        // Blend is a linear value. Colors are interpolated in linear RGB color-space.
        static Lv2cColor Blend(double blend, const Lv2cColor&c0, const Lv2cColor&c2);
        /// @brief Blend using un-gamma-corrected values. Deprecated.
        static Lv2cColor LinearBlend(double blend, const Lv2cColor &c0, const Lv2cColor &c1);
        Lv2cColor &operator=(const Lv2cColor &other);
        Lv2cColor &operator=(const std::string &webColor);

        /// @brief Produce an palette color variant from the current color.
        /// @param level A level from 50 to 900. 500 is the current color. Smaller leves are lighter; larger levels are darker.
        ////@returns A Lv2c color that is lighter or darker than the current color, depending on the suppled level.
        /// Analagous to Android M2 Material color palettes. The color of the object is treated as color 500. 
        /// Levels less than 500 generate lighter, less saturated results, trending to white. Levels greater than 500 
        /// generate darker, more saturated results. Level 200 is suitable for button faces, and lightly-tinted control decorations. Level 700
        /// would be suitable for toolbars, if Lv2cairo had such a concept. Level 800 would be suitable for status/title bars
        /// if Lv2c had such a concept.
        ///
        /// Colors are selected in CIE Lab space in order to provide colors with very roughly perceptualy-equal spacing. The CIE Lab
        /// color model is known to provide only a first-order approximation of human hue perception, with notable problems with
        /// blue hues.
        ///
        /// Palette colors do not match Google M2 Material Palette colors, although they provide equivalent functionality.
        ///
        /// Color computations are moderately (but not prohibitively) expensive. You may want to pre-compute palette colors outside of 
        /// the main drawing loop.


        Lv2cColor PaletteColor(int level);

        /// @brief A measure of the contrast between two colors.
        /// @param c1 
        /// @param c2 
        /// @return A positive number indicating the perceptual difference between two colors.
        /// ColorDifference uses the CIEDE2000 metric to generate a measure that is roughly perceptually
        /// linear across the entire color-space (but in fact is not quite linear).
        ///
        /// In common use, one would compare relative contrast values, for example: to decide whether to use light- or dark-colored text 
        /// on a given background background color based on which has the larger color difference.
        ///
        /// See lvtk/Lv2cCieColors.hpp for a more general selection of color-space and color-management functions.
        static double ColorDifference(const Lv2cColor &c1, const Lv2cColor &c2);
        

        float R() const { return r; }
        float G() const { return g; }
        float B() const { return b; }
        float A() const { return a; }

        std::string toString() const;
        std::string ToWebString() const;

        // #rrggbbaa
        std::string toPangoString() const;

        bool operator==(const Lv2cColor &other) const;

        // @brief sRGB value [0..255] to linear intensity in range [0..1].
        static double RgbToI(double value);
        // @brief Linear intensity value in range [0..1] to sRGB value [0..255]
        static double IToRgb(double value);
    private:
        float r, g, b, a;
    };

    class Lv2cPoint
    {
    public:
        Lv2cPoint() : x(0), y(0) {}
        Lv2cPoint(double x, double y) : x(x), y(y) {}

        bool operator==(const Lv2cPoint &other) const { return x == other.x && y == other.y; }
        Lv2cPoint operator+(const Lv2cPoint &other) { return Lv2cPoint(x + other.x, y + other.y); }
        Lv2cPoint operator-(const Lv2cPoint &other) { return Lv2cPoint(x - other.x, y - other.y); }
        Lv2cPoint operator-() { return Lv2cPoint(-x, -y); }
        Lv2cPoint operator/(double value) const { return Lv2cPoint(x/value,y/value); }
        Lv2cPoint operator*(double value) const { return Lv2cPoint(x*value,y*value); }
        Lv2cPoint ceil() const { return Lv2cPoint(std::ceil(x),std::ceil(y));}
        Lv2cPoint floor() const { return Lv2cPoint(std::floor(x),std::floor(y));}

        double x;
        double y;

        std::string ToString() const
        {
            std::stringstream s;
            s << '{' << x << "," << y << '}';
            return s.str();
        }
        static double Distance(Lv2cPoint p1, Lv2cPoint p2);
    };

    struct Lv2cThickness {
        double left = 0, top = 0, right = 0, bottom = 0;
    };

    class Lv2cRectangle
    {
    public:
        Lv2cRectangle();
        Lv2cRectangle(double x, double y, double width, double height);
        Lv2cRectangle(Lv2cSize size);
        double X() const { return x; }
        double Y() const { return y; }
        double Width() const { return width; }
        double Height() const { return height; }
        double Left() const { return x; }
        double Right() const { return x + width; }
        double Top() const { return y; }
        double Bottom() const { return y + height; }
        Lv2cSize Size() const { return Lv2cSize(width, height); }
        double Area() const { return width * height; }

        Lv2cRectangle &Left(double value)
        {
            this->x = value;
            return *this;
        }
        Lv2cRectangle &X(double value)
        {
            this->x = value;
            return *this;
        }
        Lv2cRectangle &Top(double value)
        {
            this->y = value;
            return *this;
        }
        Lv2cRectangle &Y(double value)
        {
            this->y = value;
            return *this;
        }
        Lv2cRectangle &Width(double value)
        {
            this->width = value;
            return *this;
        }
        Lv2cRectangle &Height(double value)
        {
            this->height = value;
            return *this;
        }

        Lv2cRectangle Intersect(const Lv2cRectangle &other) const;
        // The outer bounds of this rectangle and the other.
        Lv2cRectangle Union(const Lv2cRectangle &other) const;
        Lv2cRectangle Inflate(double value) const;
        Lv2cRectangle Inflate(double left,double top, double right, double bottom) const;
        Lv2cRectangle Inset(const Lv2cThickness &thickness) const;

        // Returns floor() of left and top, and ceiling() of right and bottom. 
        Lv2cRectangle Ceiling() const;
        // Returns ceil() of left and top, and floor() of right and bottom.
        Lv2cRectangle Floor() const;


        bool Contains(double x, double y) const;
        bool Contains(Lv2cPoint point) const;

        bool operator==(const Lv2cRectangle &other) const;

        Lv2cRectangle Translate(double tx, double ty) const;
        Lv2cRectangle translate(Lv2cPoint point) const { return Translate(point.x, point.y); }

        bool Intersects(const Lv2cRectangle &other) const;
        bool Empty() const;


        std::string ToString() const
        {
            std::stringstream s;
            s << '{' << x << "," << y << "," << width << "," << height << '}';
            return s.str();
        }

    private:
        double x, y, width, height;
    };

    enum class Lv2cWindowState
    {
        Withdrawn,
        Normal,
        Maximized,
        Minimized
    };
    class Lv2cWindowPosition 
    {
    public:
        Lv2cWindowPosition();

        Lv2cWindowPosition(int x,
                       int y,
                       int width,
                       int height,
                       Lv2cWindowState windowState = Lv2cWindowState::Normal);
        Lv2cWindowState WindowState() const { return windowState; }

    public:
        Lv2cWindowState windowState;
        int x,y,width,height;
    };

    ///////////////////////////////

    inline Lv2cSize::Lv2cSize()
        : width(0), height(0)
    {
    }

    inline Lv2cSize::Lv2cSize(double width, double height)
        : width(width), height(height)
    {
    }

    inline Lv2cRectangle::Lv2cRectangle(double x, double y, double width, double height)
        : x(x), y(y), width(width), height(height)
    {
    }

    inline Lv2cRectangle::Lv2cRectangle(Lv2cSize size)
        : x(0), y(0), width(size.Width()), height(size.Height())
    {
    }

    inline Lv2cRectangle::Lv2cRectangle()
        : x(0), y(0), width(0), height(0)
    {
    }


    enum class ModifierState
    {
        Empty = 0,
        Shift = 1,
        Control = 2,
        Alt = 4,
        Super = 8,
    };

    // Are all the of the test flags set in value?
    inline bool operator&&(ModifierState value, ModifierState testFlags)
    {
        return (int(value) & int(testFlags)) == int(testFlags);
    }

    // Are any of the test flags set in value?
    inline bool operator||(ModifierState value, ModifierState testFlags)
    {
        return (int(value) & int(testFlags)) != 0;
    }

    // Add flag bit(s) to value.
    inline ModifierState operator+(ModifierState value, ModifierState flags)
    {
        return (ModifierState)(int(value) | int(flags));
    }
    // Remove flag bit(s) from value.
    inline ModifierState operator-(ModifierState value, ModifierState flags)
    {
        return (ModifierState)(int(value) & ~int(flags));
    }
    inline ModifierState &operator+=(ModifierState &value, ModifierState flags)
    {
        value = value + flags;
        return value;
    }
    inline ModifierState &operator-=(ModifierState &value, ModifierState flags)
    {
        value = value - flags;
        return value;
    }

    class WindowHandle
    {
    public:
        WindowHandle() : nativeHandle(0) {}
        WindowHandle(uint64_t nativeHandle) : nativeHandle(nativeHandle) {}
        uint64_t getHandle() const { return nativeHandle; }
        bool operator==(const WindowHandle &other) { return nativeHandle == other.nativeHandle; }

    private:
        uint64_t nativeHandle;
    };

    class AnimationHandle
    {
        friend class Lv2cWindow;

    private:
        static AnimationHandle Next();
        static uint64_t nextHandle;

    public:
        AnimationHandle();
        static const AnimationHandle InvalidHandle;
        bool isValid() const;

        operator bool() const { return isValid(); }

        bool operator==(const AnimationHandle &other) const;
        bool operator<(const AnimationHandle &other) const;

    private:
        uint64_t nativeHandle;
    };

    /* EVENTS*/

    class Lv2cElement;

    
    class EventHandle
    {
    public:
        EventHandle()
        {
            handle = 0;
        }
        operator bool() const { return handle != 0; }
        static const EventHandle InvalidHandle;

        static EventHandle Next() { return EventHandle(++nextHandle); }
        bool isValid() const { return handle != 0; }
        void Clear() { this->handle = 0; }
        uint64_t getHandle() { return this->handle; }

    private:
        uint64_t handle = 0;
        EventHandle(uint64_t v) { handle = v; }
        static uint64_t nextHandle;
    };
    inline uint64_t EventHandle::nextHandle = 1;
    inline const EventHandle EventHandle::InvalidHandle = EventHandle(0);

    template <typename EVENT_ARGS_TYPE>
    class Lv2cEvent
    {
    public:
        using EventArgs = EVENT_ARGS_TYPE;
        using EventListener = std::function<bool (const EventArgs&e)>;

        bool Fire(const EventArgs &event) requires (!std::is_same_v<EVENT_ARGS_TYPE,void>)
        {
            for (auto &listener : eventHandlers)
            {
                if (listener.second(event))
                {
                    return true;
                }
            }
            return false;
        }

        EventHandle AddListener(const EventListener &listener)
        {
            EventHandle h = EventHandle::Next();
            eventHandlers[h.getHandle()] = listener;
            return h;
        }
        bool RemoveListener(EventHandle h)
        {
            auto f = eventHandlers.find(h.getHandle());
            if (f != eventHandlers.end())
            {
                eventHandlers.erase(f);
                return true;
            }
            return false;
        }

    private:
        std::map<uint64_t, EventListener> eventHandlers;
    };

    template <>
    class Lv2cEvent<void>
    {
    public:
        using EventArgs = void;
        using EventListener = std::function<bool (void)>;

        bool Fire() 
        {
            for (auto &listener : eventHandlers)
            {
                if (listener.second())
                {
                    return true;
                }
            }
            return false;
        }

        EventHandle AddListener(EventListener &&listener)
        {
            EventHandle h = EventHandle::Next();
            eventHandlers[h.getHandle()] = std::move(listener);
            return h;
        }

        EventHandle AddListener(const EventListener &listener)
        {
            EventHandle h = EventHandle::Next();
            eventHandlers[h.getHandle()] = listener;
            return h;
        }
        bool RemoveListener(EventHandle h)
        {
            auto f = eventHandlers.find(h.getHandle());
            if (f != eventHandlers.end())
            {
                eventHandlers.erase(f);
                return true;
            }
            return false;
        }

    private:
        std::map<uint64_t, EventListener> eventHandlers;
    };

    struct Lv2cFocusEventArgs
    {
        Lv2cFocusEventArgs();
        Lv2cFocusEventArgs(Lv2cElement *oldFocus, Lv2cElement *newFocus);

        const Lv2cElement *oldFocus = nullptr;
        const Lv2cElement *newFocus = nullptr;
    };

    struct Lv2cKeyboardEventArgs
    {
        WindowHandle h;
        Lv2cElement* target; // original target of the event.
        uint64_t rawKeyCode = 0;
        bool textValid = false;
        bool keysymValid = false;
        char text[32]{};
        // currently an x11 keysym from X11/keysymdefs.h
        unsigned int keysym = 0;
        ModifierState modifierState;
    };
    enum class Lv2cScrollDirection {
        Up,
        Down,
        Left,
        Right
    };
    struct Lv2cScrollWheelEventArgs
    {
    public:
        Lv2cScrollWheelEventArgs();
        Lv2cScrollWheelEventArgs(WindowHandle h, Lv2cScrollDirection scrollDirection, double x, double y, ModifierState modifierState);

        WindowHandle h;
        Lv2cScrollDirection scrollDirection;

        // Mouse location in element coordinates.
        Lv2cPoint point;
        // Mouse location in screeen coordinates.
        Lv2cPoint screenPoint;
        ModifierState modifierState;
    };


    struct Lv2cMouseEventArgs
    {
    public:
        Lv2cMouseEventArgs();
        Lv2cMouseEventArgs(WindowHandle h, uint64_t button, double x, double y, ModifierState modifierState);

        WindowHandle h;
        uint64_t button;

        // Mouse location in element coordinates.
        Lv2cPoint point;
        // Mouse location in screeen coordinates.
        Lv2cPoint screenPoint;
        ModifierState modifierState;
    };

    class Lv2cMouseOverEventArgs
    {
    public:
        Lv2cMouseOverEventArgs(bool mouseOver);

        bool mouseOver;
    };

    ////////////////////////////////////////

    inline Lv2cMouseOverEventArgs::Lv2cMouseOverEventArgs(bool mouseOver) : mouseOver(mouseOver) {}

    inline Lv2cMouseEventArgs::Lv2cMouseEventArgs(WindowHandle h, uint64_t button, double x, double y, ModifierState modifierState)
        : h(h), button(button), point({-1, -1}), screenPoint({double(x), double(y)}), modifierState(modifierState)
    {
    }
    inline Lv2cMouseEventArgs::Lv2cMouseEventArgs()
        : h(WindowHandle(0)), button(0), point({0, 0}), screenPoint({0, 0}), modifierState(ModifierState::Empty)
    {
    }
    inline Lv2cScrollWheelEventArgs::Lv2cScrollWheelEventArgs(WindowHandle h, Lv2cScrollDirection scrollDirection, double x, double y, ModifierState modifierState)
        : h(h), scrollDirection(scrollDirection), point({-1, -1}), screenPoint({double(x), double(y)}), modifierState(modifierState)
    {
    }
    inline Lv2cScrollWheelEventArgs::Lv2cScrollWheelEventArgs()
        : h(WindowHandle(0)), scrollDirection(Lv2cScrollDirection::Up), point({0, 0}), screenPoint({0, 0}), modifierState(ModifierState::Empty)
    {
    }

    inline bool Lv2cRectangle::operator==(const Lv2cRectangle &other) const
    {
        return this->x == other.x && this->y == other.y && this->width == other.width && this->height == other.height;
    }

    inline Lv2cRectangle Lv2cRectangle::Translate(double tx, double ty) const
    {
        return Lv2cRectangle(x + tx, y + ty, width, height);
    }

    inline bool Lv2cRectangle::Intersects(const Lv2cRectangle &other) const
    {
        if (Right() <= other.Left())
            return false;
        if (Left() >= other.Right())
            return false;
        if (Bottom() <= other.Top())
            return false;
        if (Top() >= other.Bottom())
            return false;
        return true;
    }

    class Lv2cWindow;
    class Lv2cElement;

    enum class LayoutValueType
    {
        Empty,
        Pixels,
        Percent,
        Point
    };

    class StyleContext
    {
    public:
        double length;          // total length, including extraSpace.
        // double extraSpace;      // total space that can be occupied by n* elements.
        // double extraSpaceCount; // sum of n for all n* elements.
    };

    class Lv2cMeasurement
    {
    public:
        using Type = LayoutValueType;
        Lv2cMeasurement() : type_(Type::Empty), value(0) {}
        Lv2cMeasurement(double value) : type_(Type::Pixels), value(value) {}
        Lv2cMeasurement(double value, Type type_) : type_(type_), value(value) {}
        Lv2cMeasurement(int value) : type_(Type::Pixels), value((double)value) {}

        static Lv2cMeasurement Empty() { return Lv2cMeasurement(); }
        static Lv2cMeasurement Percent(double value) { return Lv2cMeasurement(value, Type::Percent); }
        static Lv2cMeasurement Pixels(double value) { return Lv2cMeasurement(value, Type::Pixels); }
        static Lv2cMeasurement Point(double value) { return Lv2cMeasurement(value, Type::Point); }

        bool isPixels() const { return type_ == Type::Pixels; }
        bool isPercent() const { return type_ == Type::Percent; }
        bool isEmpty() const { return type_ == Type::Empty; }
        bool iPoint() const { return type_ == Type::Point; }

        Type getType() const { return type_; }
        /// @brief Get the original value supplied to the constructor.
        /// @return
        double getBaseValue() const { return value; }
        /// @brief Calculate the effective value in pixels.
        /// @return
        double PixelValue() const;


        Lv2cMeasurement &operator=(double value)
        {
            this->value = value;
            this->type_ = Type::Pixels;
            return *this;
        }

        void ResolvePercent(double size);

    private:
        Type type_;
        double value;
    };

    class Lv2cThicknessMeasurement
    {
    public:
        Lv2cThicknessMeasurement() : left(0), top(0), right(0), bottom(0) {}
        Lv2cThicknessMeasurement(const Lv2cMeasurement &thickness) : left(thickness), top(thickness), right(thickness), bottom(thickness) {}
        Lv2cThicknessMeasurement(double thickness) : left(thickness), top(thickness), right(thickness), bottom(thickness) {}
        Lv2cThicknessMeasurement(
            const Lv2cMeasurement &left,
            const Lv2cMeasurement &top,
            const Lv2cMeasurement &right,
            const Lv2cMeasurement &bottom) : left(left), top(top), right(right), bottom(bottom) {}

        Lv2cThickness PixelValue() const;

        void ResolvePercent(Lv2cSize size)
        {
            left.ResolvePercent(size.Width());
            right.ResolvePercent(size.Width());
            top.ResolvePercent(size.Height());
            bottom.ResolvePercent(size.Height());
        }

        Lv2cThicknessMeasurement &Left(const Lv2cMeasurement &measurement)
        {
            left = measurement;
            return *this;
        }
        Lv2cThicknessMeasurement &Right(const Lv2cMeasurement &measurement)
        {
            right = measurement;
            return *this;
        }
        Lv2cThicknessMeasurement &Top(const Lv2cMeasurement &measurement)
        {
            top = measurement;
            return *this;
        }
        Lv2cThicknessMeasurement &Bottom(const Lv2cMeasurement &measurement)
        {
            bottom = measurement;
            return *this;
        }

        const Lv2cMeasurement &Left() const { return left; }
        const Lv2cMeasurement &Right() const { return right; }
        const Lv2cMeasurement &Top() const { return top; }
        const Lv2cMeasurement &Bottom() const { return bottom; }

        bool isEmpty() const;

    private:
        Lv2cMeasurement left, top, right, bottom;
    };

    enum class Lv2cVisibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    enum class Lv2cFontWeight
    {
        Thin = 100,
        Ultralight = 200,
        Light = 300,
        Semilight = 350,
        Book = 380,
        Normal = 400,
        Medium = 500,
        Semibold = 600,
        Bold = 700,
        Ultrabold = 800,
        Heavy = 900,
        Ultraheavy = 1000
    };

    enum class Lv2cFontStretch
    {
        UltraCondensed,
        ExtraCondensed,
        Condensed,
        SemiCondensed,
        Normal,
        SemiExpanded,
        Expanded,
        ExtraExpanded,
        UltraExpanded
    }; // matches typedef enum {} PangoStretch;

    enum class Lv2cFontStyle
    {
        Normal,
        Oblique,
        Italic
    };

    enum class Lv2cFontVariant
    {
        Normal,
        SmallCaps
    }; // maps onto PangoVariant;

    enum class Lv2cTextTransform {
        Normal,
        Capitalize
    };
    enum class Lv2cHoverState
    {
        Empty = 0,
        Hover = 1,
        Focus = 2,
        Pressed = 4,
        Selected = 8
    };
    inline Lv2cHoverState operator&(Lv2cHoverState v1, Lv2cHoverState v2) { return (Lv2cHoverState)(((int)v1) & ((int)v2)); }
    inline Lv2cHoverState operator|(Lv2cHoverState v1, Lv2cHoverState v2) { return (Lv2cHoverState)(((int)v1) | ((int)v2)); }
    inline Lv2cHoverState operator-(Lv2cHoverState v1, Lv2cHoverState v2) { return (Lv2cHoverState)(((int)v1) & ~((int)v2)); }
    inline Lv2cHoverState operator+(Lv2cHoverState v1, Lv2cHoverState v2) { return (Lv2cHoverState)(((int)v1) | ((int)v2)); }
    inline bool operator&&(Lv2cHoverState v1, Lv2cHoverState v2) { return (((int)v1) & ((int)v2)) != 0; }

    class Lv2cHoverOpacity
    {
    public:
        Lv2cHoverOpacity();
        Lv2cHoverOpacity(double defaultOpacity,double hoverOpacity,double focusOpacity, double pressedOpacity);
        double GetOpacity(Lv2cHoverState hoverState) const;
        bool operator==(const Lv2cHoverOpacity&other) const;
    private:
        static constexpr size_t N_ENTRIES = 8;
        double opacity[N_ENTRIES] = {};

    };
    class Lv2cHoverColors
    {
    public:
        Lv2cHoverColors();
        Lv2cHoverColors(Lv2cColor textColor);
        Lv2cHoverColors(Lv2cColor textColor, double maxOpacity);
        Lv2cHoverColors(Lv2cColor background, Lv2cColor foreground);

        Lv2cColor GetColor(Lv2cHoverState hoverState) const;
        bool operator==(const Lv2cHoverColors&other) const;
    private:
        Lv2cColor colors[8] = {};
    };

    class Lv2cRoundCorners
    {
    public:
        double topLeft;
        double topRight;
        double bottomLeft;
        double bottomRight;

        Lv2cRoundCorners inset(const Lv2cThickness&thickness);
        Lv2cRoundCorners operator*(double scale);
        bool is_empty() const;
    };

    class Lv2cRoundCornersMeasurement
    {
    public:
        Lv2cRoundCornersMeasurement();
        Lv2cRoundCornersMeasurement(const Lv2cMeasurement &value);
        Lv2cRoundCornersMeasurement(
            const Lv2cMeasurement &topLeft,
            const Lv2cMeasurement &topRight,
            const Lv2cMeasurement &bottomleft,
            const Lv2cMeasurement &bottomRight);
        bool isEmpty() const;
        Lv2cMeasurement topLeft, topRight, bottomLeft, bottomRight;
        void ResolvePercent(Lv2cSize elementBounds);
        Lv2cRoundCorners PixelValue() const;
    };

    enum class Lv2cFlexDirection
    {
        Column,
        Row
    };
    enum class Lv2cFlexWrap
    {
        Wrap,
        NoWrap
    };

    enum class Lv2cFlexJustification
    {
        Start,
        End,
        Center,
        SpaceBetween,
        SpaceAround
    };

    enum class Lv2cFlexOverflowJustification
    {
        // Same as FlexJustification.
        Normal, 
        // Align to start.
        Start,
        // Align to end.
        End,
        // Center
        Center
    };

    enum class Lv2cTextAlign
    {
        Start,
        Center,
        End
    }; // matches enum {} PangoAlignment

    enum class Lv2cAlignment
    {
        Start,
        End,
        Center,
        Stretch
    };

    enum class Lv2cEllipsizeMode
    {
        Disable,
        Start,
        Center,
        End
    }; // must match PangoEllipsizeMode.

    enum Lv2cDropShadowVariant
    {
        Empty,
        DropShadow,
        InnerDropShadow
    };

    struct Lv2cDropShadow {
        Lv2cDropShadowVariant variant = Lv2cDropShadowVariant::DropShadow;
        double xOffset = 0;
        double yOffset = 4;
        double radius = 3;
        double opacity = 0.75;
        Lv2cColor color = Lv2cColor(0,0,0);
        bool operator==(const Lv2cDropShadow&other) const;
    };

    struct Lv2cVuSettings {
        Lv2cColor red;
        Lv2cColor yellow;
        Lv2cColor green;
        bool hasTicks = false;
        double tickDb = 6;
        double tickWidth = 4;
        double padding = 2;
        std::optional<double> redLevel;
        std::optional<double> yellowLevel;
    };

    enum class Lv2cMessageDialogType {
        Info,
        Warning,
        Error,
        Question
    };

    enum class Lv2cMessageButtons {
        Ok,
        OkCancel
    };

    ///////////////

    inline bool Lv2cRectangle::Empty() const
    {
        return width <= 0 || height <= 0;
    }

    inline AnimationHandle::AnimationHandle()
    {
        nativeHandle = 0; // invalid.
    }

    inline bool AnimationHandle::isValid() const { return nativeHandle != 0; }

    inline bool AnimationHandle::operator==(const AnimationHandle &other) const { return nativeHandle == other.nativeHandle; }
    inline bool AnimationHandle::operator<(const AnimationHandle &other) const { return nativeHandle < other.nativeHandle; }


    inline Lv2cLinearColor Lv2cLinearColor::operator+(const Lv2cLinearColor &other) const
    {
        return Lv2cLinearColor(
            r + other.r,
            g+other.g,
            b+other.b,
            a+other.a
        );
    }
    inline Lv2cLinearColor Lv2cLinearColor::operator-(const Lv2cLinearColor &other) const
    {
        return Lv2cLinearColor(
            r - other.r,
            g -other.g,
            b - other.b,
            a -other.a
        );
    }
    inline Lv2cLinearColor& Lv2cLinearColor::operator+=(const Lv2cLinearColor &other)
    {
        r += other.r;
        g += other.g;
        b += other.b;
        a += other.a;
        return *this;
    }
    inline Lv2cLinearColor& Lv2cLinearColor::operator-=(const Lv2cLinearColor &other)
    {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        a -= other.a;
        return *this;
    }

    inline Lv2cLinearColor Lv2cLinearColor::operator*(float value) const
    {
        return Lv2cLinearColor(
            r*value,
            g*value,
            b*value,
            a*value
        );
    }
    namespace implementation
    {
        extern float srgb2i[256];

        extern float i2srgb[256];


        inline float SrgbToI(uint8_t value)
        {
            return srgb2i[value];
        }

        inline uint8_t IToSrgb(float value)
        {
            // lerp from a lookup table.
            // Guaranteed to round-trip uint8_t values;
            constexpr size_t inverse_table_size = sizeof(i2srgb)/sizeof(i2srgb[0]);
            constexpr size_t inverse_table_max = inverse_table_size-2;
            constexpr float TABLE_CONVERSION_FACTOR = inverse_table_max;

            if (value > 1) value = 1;
            if (value < 0) value = 0;
            float indexF = value*TABLE_CONVERSION_FACTOR;
            int ix0 = indexF;
            float frac = indexF-ix0;
            return (uint8_t)(i2srgb[ix0]*(1-frac)+i2srgb[ix0+1]*frac);
        }
    }

}


