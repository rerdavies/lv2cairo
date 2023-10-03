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

    class LvtkDrawingContext;

    class LvtkSize
    {
    public:
        LvtkSize();
        LvtkSize(double width, double height);

        static const LvtkSize Zero;
        
        bool operator==(const LvtkSize &other) const { return width == other.width && height == other.height; }
        double Width() const { return width; }
        double Height() const { return height; }
        LvtkSize &Width(double value)
        {
            width = value;
            return *this;
        }
        LvtkSize &Height(double value)
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


        LvtkSize operator+(const LvtkSize&other) { return LvtkSize(width+other.width,height+other.height);}
        LvtkSize operator-(const LvtkSize&other) { return LvtkSize(width-other.width,height-other.height);}
        LvtkSize operator/(double value) const { return LvtkSize(width/value,height/value); }
        LvtkSize operator*(double value) const { return LvtkSize(width*value,height*value); }
        LvtkSize ceil() const { return LvtkSize(std::ceil(width),std::ceil(height));}
        LvtkSize floor() const { return LvtkSize(std::floor(width),std::floor(height));}

    private:
        double width, height;
    };


    class LvtkColor;
    class LvtkHsvColor;

    // Color with linear intensity. r,g,b values are pre-multiplied.
    class LvtkLinearColor {
    public:
        LvtkLinearColor() : r(0.0f),g(0.0f),b(0.0f),a(0.0f) { }
        LvtkLinearColor(float r, float g, float b) : r(r),g(g),b(b),a(1){}
        LvtkLinearColor(float r, float g, float b, float a) : r(r),g(g),b(b),a(a){}
        explicit LvtkLinearColor(const LvtkColor&color);
        explicit LvtkLinearColor(const LvtkHsvColor&color);

        static LvtkLinearColor FromImageSurfaceColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        LvtkLinearColor FromSrgb(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

        /// @brief Bulk conversion of CarioLinearColors to sRGB. Private Use.
        static void ToImageSurface(size_t count, const LvtkLinearColor *source, uint8_t*dest, float scale);
        static void ToImageSurface(const std::vector<LvtkLinearColor> &source, uint8_t*dest);
        static void FromImageSurface(size_t count, const uint8_t*source, LvtkLinearColor *dest);
        static void FromImageSurface(const uint8_t*source, std::vector<LvtkLinearColor> &dest);

        float r,g,b,a;

        LvtkLinearColor operator+(const LvtkLinearColor &other) const;
        LvtkLinearColor operator-(const LvtkLinearColor &other) const;
        LvtkLinearColor& operator+=(const LvtkLinearColor &other);
        LvtkLinearColor& operator-=(const LvtkLinearColor &other);
        LvtkLinearColor operator*(float value) const;
    };

    class LvtkHsvColor {
    public:
        LvtkHsvColor(float hue, float saturation, float brightness, float alpha = 1.0f);
        explicit LvtkHsvColor(const LvtkLinearColor &linearColor);
        explicit LvtkHsvColor(const LvtkColor &linearColor);

        float Hue() const { return hue; }
        float Saturation() const { return saturation; }
        float Brightness() const { return brightness; }
        float Alpha() const { return alpha; }
        bool operator==(const LvtkHsvColor&other) const;
    private:
        float hue;
        float saturation;
        float brightness;
        float alpha;
    };
    /// Color in sRGB space.
    class LvtkColor
    {
    public:
        LvtkColor() : r(0), g(0), b(0), a(1) {}
        LvtkColor(float r, float g, float b) : r(r), g(g), b(b), a(1) {}
        LvtkColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        LvtkColor(const LvtkColor &color, float a) : r(color.R()), g(color.G()), b(color.B()), a(color.A() * a) {}
        LvtkColor(const std::string &webColor);
        explicit LvtkColor(const LvtkLinearColor &color);
        explicit LvtkColor(const LvtkHsvColor&color);

        // @brief Blend two colors.
        // Blend is a linear value. Colors are interpolated in linear RGB color-space.
        static LvtkColor Blend(double blend, const LvtkColor&c0, const LvtkColor&c2);
        /// @brief Blend using un-gamma-corrected values. Deprecated.
        static LvtkColor LinearBlend(double blend, const LvtkColor &c0, const LvtkColor &c1);
        LvtkColor &operator=(const LvtkColor &other);
        LvtkColor &operator=(const std::string &webColor);

        /// @brief Produce an palette color variant from the current color.
        /// @param level A level from 50 to 900. 500 is the current color. Smaller leves are lighter; larger levels are darker.
        ////@returns A Lvtk color that is lighter or darker than the current color, depending on the suppled level.
        /// Analagous to Android M2 Material color palettes. The color of the object is treated as color 500. 
        /// Levels less than 500 generate lighter, less saturated results, trending to white. Levels greater than 500 
        /// generate darker, more saturated results. Level 200 is suitable for button faces, and lightly-tinted control decorations. Level 700
        /// would be suitable for toolbars, if Lv2cairo had such a concept. Level 800 would be suitable for status/title bars
        /// if Lvtk had such a concept.
        ///
        /// Colors are selected in CIE Lab space in order to provide colors with very roughly perceptualy-equal spacing. The CIE Lab
        /// color model is known to provide only a first-order approximation of human hue perception, with notable problems with
        /// blue hues.
        ///
        /// Palette colors do not match Google M2 Material Palette colors, although they provide equivalent functionality.
        ///
        /// Color computations are moderately (but not prohibitively) expensive. You may want to pre-compute palette colors outside of 
        /// the main drawing loop.


        LvtkColor PaletteColor(int level);

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
        /// See lvtk/LvtkCieColors.hpp for a more general selection of color-space and color-management functions.
        static double ColorDifference(const LvtkColor &c1, const LvtkColor &c2);
        

        float R() const { return r; }
        float G() const { return g; }
        float B() const { return b; }
        float A() const { return a; }

        std::string toString() const;
        std::string ToWebString() const;

        // #rrggbbaa
        std::string toPangoString() const;

        bool operator==(const LvtkColor &other) const;

        // @brief sRGB value [0..255] to linear intensity in range [0..1].
        static double RgbToI(double value);
        // @brief Linear intensity value in range [0..1] to sRGB value [0..255]
        static double IToRgb(double value);
    private:
        float r, g, b, a;
    };

    class LvtkPoint
    {
    public:
        LvtkPoint() : x(0), y(0) {}
        LvtkPoint(double x, double y) : x(x), y(y) {}

        bool operator==(const LvtkPoint &other) const { return x == other.x && y == other.y; }
        LvtkPoint operator+(const LvtkPoint &other) { return LvtkPoint(x + other.x, y + other.y); }
        LvtkPoint operator-(const LvtkPoint &other) { return LvtkPoint(x - other.x, y - other.y); }
        LvtkPoint operator-() { return LvtkPoint(-x, -y); }
        LvtkPoint operator/(double value) const { return LvtkPoint(x/value,y/value); }
        LvtkPoint operator*(double value) const { return LvtkPoint(x*value,y*value); }
        LvtkPoint ceil() const { return LvtkPoint(std::ceil(x),std::ceil(y));}
        LvtkPoint floor() const { return LvtkPoint(std::floor(x),std::floor(y));}

        double x;
        double y;

        std::string ToString() const
        {
            std::stringstream s;
            s << '{' << x << "," << y << '}';
            return s.str();
        }
        static double Distance(LvtkPoint p1, LvtkPoint p2);
    };

    struct LvtkThickness {
        double left = 0, top = 0, right = 0, bottom = 0;
    };

    class LvtkRectangle
    {
    public:
        LvtkRectangle();
        LvtkRectangle(double x, double y, double width, double height);
        LvtkRectangle(LvtkSize size);
        double X() const { return x; }
        double Y() const { return y; }
        double Width() const { return width; }
        double Height() const { return height; }
        double Left() const { return x; }
        double Right() const { return x + width; }
        double Top() const { return y; }
        double Bottom() const { return y + height; }
        LvtkSize Size() const { return LvtkSize(width, height); }
        double Area() const { return width * height; }

        LvtkRectangle &Left(double value)
        {
            this->x = value;
            return *this;
        }
        LvtkRectangle &X(double value)
        {
            this->x = value;
            return *this;
        }
        LvtkRectangle &Top(double value)
        {
            this->y = value;
            return *this;
        }
        LvtkRectangle &Y(double value)
        {
            this->y = value;
            return *this;
        }
        LvtkRectangle &Width(double value)
        {
            this->width = value;
            return *this;
        }
        LvtkRectangle &Height(double value)
        {
            this->height = value;
            return *this;
        }

        LvtkRectangle Intersect(const LvtkRectangle &other) const;
        // The outer bounds of this rectangle and the other.
        LvtkRectangle Union(const LvtkRectangle &other) const;
        LvtkRectangle Inflate(double value) const;
        LvtkRectangle Inflate(double left,double top, double right, double bottom) const;
        LvtkRectangle Inset(const LvtkThickness &thickness) const;

        // Returns floor() of left and top, and ceiling() of right and bottom. 
        LvtkRectangle Ceiling() const;
        // Returns ceil() of left and top, and floor() of right and bottom.
        LvtkRectangle Floor() const;


        bool Contains(double x, double y) const;
        bool Contains(LvtkPoint point) const;

        bool operator==(const LvtkRectangle &other) const;

        LvtkRectangle Translate(double tx, double ty) const;
        LvtkRectangle translate(LvtkPoint point) const { return Translate(point.x, point.y); }

        bool Intersects(const LvtkRectangle &other) const;
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

    enum class LvtkWindowState
    {
        Withdrawn,
        Normal,
        Maximized,
        Minimized
    };
    class LvtkWindowPosition 
    {
    public:
        LvtkWindowPosition();

        LvtkWindowPosition(int x,
                       int y,
                       int width,
                       int height,
                       LvtkWindowState windowState = LvtkWindowState::Normal);
        LvtkWindowState WindowState() const { return windowState; }

    public:
        LvtkWindowState windowState;
        int x,y,width,height;
    };

    ///////////////////////////////

    inline LvtkSize::LvtkSize()
        : width(0), height(0)
    {
    }

    inline LvtkSize::LvtkSize(double width, double height)
        : width(width), height(height)
    {
    }

    inline LvtkRectangle::LvtkRectangle(double x, double y, double width, double height)
        : x(x), y(y), width(width), height(height)
    {
    }

    inline LvtkRectangle::LvtkRectangle(LvtkSize size)
        : x(0), y(0), width(size.Width()), height(size.Height())
    {
    }

    inline LvtkRectangle::LvtkRectangle()
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
        friend class LvtkWindow;

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

    class LvtkElement;

    
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
    class LvtkEvent
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
    class LvtkEvent<void>
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

    struct LvtkFocusEventArgs
    {
        LvtkFocusEventArgs();
        LvtkFocusEventArgs(LvtkElement *oldFocus, LvtkElement *newFocus);

        const LvtkElement *oldFocus = nullptr;
        const LvtkElement *newFocus = nullptr;
    };

    struct LvtkKeyboardEventArgs
    {
        WindowHandle h;
        LvtkElement* target; // original target of the event.
        uint64_t rawKeyCode = 0;
        bool textValid = false;
        bool keysymValid = false;
        char text[32]{};
        // currently an x11 keysym from X11/keysymdefs.h
        unsigned int keysym = 0;
        ModifierState modifierState;
    };
    enum class LvtkScrollDirection {
        Up,
        Down,
        Left,
        Right
    };
    struct LvtkScrollWheelEventArgs
    {
    public:
        LvtkScrollWheelEventArgs();
        LvtkScrollWheelEventArgs(WindowHandle h, LvtkScrollDirection scrollDirection, double x, double y, ModifierState modifierState);

        WindowHandle h;
        LvtkScrollDirection scrollDirection;

        // Mouse location in element coordinates.
        LvtkPoint point;
        // Mouse location in screeen coordinates.
        LvtkPoint screenPoint;
        ModifierState modifierState;
    };


    struct LvtkMouseEventArgs
    {
    public:
        LvtkMouseEventArgs();
        LvtkMouseEventArgs(WindowHandle h, uint64_t button, double x, double y, ModifierState modifierState);

        WindowHandle h;
        uint64_t button;

        // Mouse location in element coordinates.
        LvtkPoint point;
        // Mouse location in screeen coordinates.
        LvtkPoint screenPoint;
        ModifierState modifierState;
    };

    class LvtkMouseOverEventArgs
    {
    public:
        LvtkMouseOverEventArgs(bool mouseOver);

        bool mouseOver;
    };

    ////////////////////////////////////////

    inline LvtkMouseOverEventArgs::LvtkMouseOverEventArgs(bool mouseOver) : mouseOver(mouseOver) {}

    inline LvtkMouseEventArgs::LvtkMouseEventArgs(WindowHandle h, uint64_t button, double x, double y, ModifierState modifierState)
        : h(h), button(button), point({-1, -1}), screenPoint({double(x), double(y)}), modifierState(modifierState)
    {
    }
    inline LvtkMouseEventArgs::LvtkMouseEventArgs()
        : h(WindowHandle(0)), button(0), point({0, 0}), screenPoint({0, 0}), modifierState(ModifierState::Empty)
    {
    }
    inline LvtkScrollWheelEventArgs::LvtkScrollWheelEventArgs(WindowHandle h, LvtkScrollDirection scrollDirection, double x, double y, ModifierState modifierState)
        : h(h), scrollDirection(scrollDirection), point({-1, -1}), screenPoint({double(x), double(y)}), modifierState(modifierState)
    {
    }
    inline LvtkScrollWheelEventArgs::LvtkScrollWheelEventArgs()
        : h(WindowHandle(0)), scrollDirection(LvtkScrollDirection::Up), point({0, 0}), screenPoint({0, 0}), modifierState(ModifierState::Empty)
    {
    }

    inline bool LvtkRectangle::operator==(const LvtkRectangle &other) const
    {
        return this->x == other.x && this->y == other.y && this->width == other.width && this->height == other.height;
    }

    inline LvtkRectangle LvtkRectangle::Translate(double tx, double ty) const
    {
        return LvtkRectangle(x + tx, y + ty, width, height);
    }

    inline bool LvtkRectangle::Intersects(const LvtkRectangle &other) const
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

    class LvtkWindow;
    class LvtkElement;

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

    class LvtkMeasurement
    {
    public:
        using Type = LayoutValueType;
        LvtkMeasurement() : type_(Type::Empty), value(0) {}
        LvtkMeasurement(double value) : type_(Type::Pixels), value(value) {}
        LvtkMeasurement(double value, Type type_) : type_(type_), value(value) {}
        LvtkMeasurement(int value) : type_(Type::Pixels), value((double)value) {}

        static LvtkMeasurement Empty() { return LvtkMeasurement(); }
        static LvtkMeasurement Percent(double value) { return LvtkMeasurement(value, Type::Percent); }
        static LvtkMeasurement Pixels(double value) { return LvtkMeasurement(value, Type::Pixels); }
        static LvtkMeasurement Point(double value) { return LvtkMeasurement(value, Type::Point); }

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


        LvtkMeasurement &operator=(double value)
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

    class LvtkThicknessMeasurement
    {
    public:
        LvtkThicknessMeasurement() : left(0), top(0), right(0), bottom(0) {}
        LvtkThicknessMeasurement(const LvtkMeasurement &thickness) : left(thickness), top(thickness), right(thickness), bottom(thickness) {}
        LvtkThicknessMeasurement(double thickness) : left(thickness), top(thickness), right(thickness), bottom(thickness) {}
        LvtkThicknessMeasurement(
            const LvtkMeasurement &left,
            const LvtkMeasurement &top,
            const LvtkMeasurement &right,
            const LvtkMeasurement &bottom) : left(left), top(top), right(right), bottom(bottom) {}

        LvtkThickness PixelValue() const;

        void ResolvePercent(LvtkSize size)
        {
            left.ResolvePercent(size.Width());
            right.ResolvePercent(size.Width());
            top.ResolvePercent(size.Height());
            bottom.ResolvePercent(size.Height());
        }

        LvtkThicknessMeasurement &Left(const LvtkMeasurement &measurement)
        {
            left = measurement;
            return *this;
        }
        LvtkThicknessMeasurement &Right(const LvtkMeasurement &measurement)
        {
            right = measurement;
            return *this;
        }
        LvtkThicknessMeasurement &Top(const LvtkMeasurement &measurement)
        {
            top = measurement;
            return *this;
        }
        LvtkThicknessMeasurement &Bottom(const LvtkMeasurement &measurement)
        {
            bottom = measurement;
            return *this;
        }

        const LvtkMeasurement &Left() const { return left; }
        const LvtkMeasurement &Right() const { return right; }
        const LvtkMeasurement &Top() const { return top; }
        const LvtkMeasurement &Bottom() const { return bottom; }

        bool isEmpty() const;

    private:
        LvtkMeasurement left, top, right, bottom;
    };

    enum class LvtkVisibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    enum class LvtkFontWeight
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

    enum class LvtkFontStretch
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

    enum class LvtkFontStyle
    {
        Normal,
        Oblique,
        Italic
    };

    enum class LvtkFontVariant
    {
        Normal,
        SmallCaps
    }; // maps onto PangoVariant;

    enum class LvtkTextTransform {
        Normal,
        Capitalize
    };
    enum class LvtkHoverState
    {
        Empty = 0,
        Hover = 1,
        Focus = 2,
        Pressed = 4,
        Selected = 8
    };
    inline LvtkHoverState operator&(LvtkHoverState v1, LvtkHoverState v2) { return (LvtkHoverState)(((int)v1) & ((int)v2)); }
    inline LvtkHoverState operator|(LvtkHoverState v1, LvtkHoverState v2) { return (LvtkHoverState)(((int)v1) | ((int)v2)); }
    inline LvtkHoverState operator-(LvtkHoverState v1, LvtkHoverState v2) { return (LvtkHoverState)(((int)v1) & ~((int)v2)); }
    inline LvtkHoverState operator+(LvtkHoverState v1, LvtkHoverState v2) { return (LvtkHoverState)(((int)v1) | ((int)v2)); }
    inline bool operator&&(LvtkHoverState v1, LvtkHoverState v2) { return (((int)v1) & ((int)v2)) != 0; }

    class LvtkHoverOpacity
    {
    public:
        LvtkHoverOpacity();
        LvtkHoverOpacity(double defaultOpacity,double hoverOpacity,double focusOpacity, double pressedOpacity);
        double GetOpacity(LvtkHoverState hoverState) const;
        bool operator==(const LvtkHoverOpacity&other) const;
    private:
        static constexpr size_t N_ENTRIES = 8;
        double opacity[N_ENTRIES] = {};

    };
    class LvtkHoverColors
    {
    public:
        LvtkHoverColors();
        LvtkHoverColors(LvtkColor textColor);
        LvtkHoverColors(LvtkColor textColor, double maxOpacity);
        LvtkHoverColors(LvtkColor background, LvtkColor foreground);

        LvtkColor GetColor(LvtkHoverState hoverState) const;
        bool operator==(const LvtkHoverColors&other) const;
    private:
        LvtkColor colors[8] = {};
    };

    class LvtkRoundCorners
    {
    public:
        double topLeft;
        double topRight;
        double bottomLeft;
        double bottomRight;

        LvtkRoundCorners inset(const LvtkThickness&thickness);
        LvtkRoundCorners operator*(double scale);
        bool is_empty() const;
    };

    class LvtkRoundCornersMeasurement
    {
    public:
        LvtkRoundCornersMeasurement();
        LvtkRoundCornersMeasurement(const LvtkMeasurement &value);
        LvtkRoundCornersMeasurement(
            const LvtkMeasurement &topLeft,
            const LvtkMeasurement &topRight,
            const LvtkMeasurement &bottomleft,
            const LvtkMeasurement &bottomRight);
        bool isEmpty() const;
        LvtkMeasurement topLeft, topRight, bottomLeft, bottomRight;
        void ResolvePercent(LvtkSize elementBounds);
        LvtkRoundCorners PixelValue() const;
    };

    enum class LvtkFlexDirection
    {
        Column,
        Row
    };
    enum class LvtkFlexWrap
    {
        Wrap,
        NoWrap
    };

    enum class LvtkFlexJustification
    {
        Start,
        End,
        Center,
        SpaceBetween,
        SpaceAround
    };

    enum class LvtkFlexOverflowJustification
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

    enum class LvtkTextAlign
    {
        Start,
        Center,
        End
    }; // matches enum {} PangoAlignment

    enum class LvtkAlignment
    {
        Start,
        End,
        Center,
        Stretch
    };

    enum class LvtkEllipsizeMode
    {
        Disable,
        Start,
        Center,
        End
    }; // must match PangoEllipsizeMode.

    enum LvtkDropShadowVariant
    {
        Empty,
        DropShadow,
        InnerDropShadow
    };

    struct LvtkDropShadow {
        LvtkDropShadowVariant variant = LvtkDropShadowVariant::DropShadow;
        double xOffset = 0;
        double yOffset = 4;
        double radius = 3;
        double opacity = 0.75;
        LvtkColor color = LvtkColor(0,0,0);
        bool operator==(const LvtkDropShadow&other) const;
    };

    struct LvtkVuSettings {
        LvtkColor red;
        LvtkColor yellow;
        LvtkColor green;
        bool hasTicks = false;
        double tickDb = 6;
        double tickWidth = 4;
        double padding = 2;
        std::optional<double> redLevel;
        std::optional<double> yellowLevel;
    };

    enum class LvtkMessageDialogType {
        Info,
        Warning,
        Error,
        Question
    };

    enum class LvtkMessageButtons {
        Ok,
        OkCancel
    };

    ///////////////

    inline bool LvtkRectangle::Empty() const
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


    inline LvtkLinearColor LvtkLinearColor::operator+(const LvtkLinearColor &other) const
    {
        return LvtkLinearColor(
            r + other.r,
            g+other.g,
            b+other.b,
            a+other.a
        );
    }
    inline LvtkLinearColor LvtkLinearColor::operator-(const LvtkLinearColor &other) const
    {
        return LvtkLinearColor(
            r - other.r,
            g -other.g,
            b - other.b,
            a -other.a
        );
    }
    inline LvtkLinearColor& LvtkLinearColor::operator+=(const LvtkLinearColor &other)
    {
        r += other.r;
        g += other.g;
        b += other.b;
        a += other.a;
        return *this;
    }
    inline LvtkLinearColor& LvtkLinearColor::operator-=(const LvtkLinearColor &other)
    {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        a -= other.a;
        return *this;
    }

    inline LvtkLinearColor LvtkLinearColor::operator*(float value) const
    {
        return LvtkLinearColor(
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


