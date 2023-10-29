/*
Copyright (c) 2023 Robin E. R. Davies

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once 

#include "lvtk/LvtkValueElement.hpp"


// forward declarations
typedef struct _PangoLayout PangoLayout;
typedef struct _PangoFontDescriptor PangoFontDescriptor;
typedef struct _PangoFontMap PangoFontMap;
typedef struct _PangoFontDescription PangoFontDescription;


namespace lvtk {
    class LvtkDrawingContext;
}

namespace lvtk::ui {

    class Lv2TunerElement : public LvtkValueElement {
    public:
        using self = Lv2TunerElement;
        using super = LvtkValueElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2TunerElement();
        virtual ~Lv2TunerElement();

        BINDING_PROPERTY(ReferenceFrequency,double,440.0)      
        BINDING_PROPERTY(ValueIsMidiNote,bool ,true)        

    protected:
        virtual bool WillDraw() const override { return true; }
        virtual void OnMount() override;
        virtual void OnValueChanged(double value) override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;
    private:
        void DrawText(LvtkDrawingContext&dc, double midiNote);
        void DrawDial(LvtkDrawingContext&c, double midiNote);
        void PreparePangoContext();
        void FreePangoContext();
        PangoLayout *pangoLayout = nullptr;
    };

}