#include "lv2c/Lv2cLampElement.hpp"
#include "lv2c/Lv2cPngElement.hpp"
#include "lv2c/Lv2cDropShadowElement.hpp"
#include "lv2c/Lv2cWindow.hpp"

using namespace lv2c;

namespace lv2c {

    class LampImage: public Lv2cObject   {

    public:
        Lv2cSurface png_surface;

    };
    class ImageCache {
    public:
        static std::shared_ptr<LampImage> GetLampImage(Lv2cWindow*window, Lv2cColor color)
        {
            std::stringstream ss;
            ss << "lamp_ " << color.R() << '_' << color.G() << '_' << color.B() << '_' << color.A() << ".priv";
            std::string key = ss.str();

            std::shared_ptr<Lv2cObject> t = window->GetMemoObject(key);
            if (t)
            {
                return std::static_pointer_cast<LampImage>(t);
            }
            auto image = std::make_shared<LampImage>();
            auto originalSurface = window->GetPngImage("white_led.png");
            Lv2cImageSurface newSurface = Lv2cImageSurface(cairo_format_t::CAIRO_FORMAT_ARGB32, originalSurface.size().Width(), originalSurface.size().Height()); 

            Lv2cDrawingContext dc { newSurface};

            dc.set_source(originalSurface);
            dc.fill();
            dc.paint();

            dc.set_source(color);
            dc.set_operator(cairo_operator_t::CAIRO_OPERATOR_MULTIPLY);
            dc.fill();
            dc.paint();

            image->png_surface = newSurface;

            window->SetMemoObject(key, image);
            return image;
        }

        std::map<Lv2cColor, std::weak_ptr<LampImage>> cache;
    };


    ImageCache gImageCache;
};

Lv2cLampElement::Lv2cLampElement()
{
    // VariantProperty.SetElement(this,&Lv2cLampElement::OnVariantChanged);
    // auto bg = Lv2cPngElement::Create();
    // bg->Source("OffLed3.png");
    // bg->Style()
    //     .HorizontalAlignment(Lv2cAlignment::Stretch)
    //     .VerticalAlignment(Lv2cAlignment::Stretch);
    // AddChild(bg);

    // auto image = Lv2cPngElement::Create();
    // image->Source("BlueLed3.png");
    // image->Style()
    //     .HorizontalAlignment(Lv2cAlignment::Stretch)
    //     .VerticalAlignment(Lv2cAlignment::Stretch);
    // AddChild(image);
    // this->image = image;
    // image->Style().Opacity(Value());

    // this->Style()
    //     .HorizontalAlignment(Lv2cAlignment::Center)
    //     .VerticalAlignment(Lv2cAlignment::Center)
    //     .RoundCorners({8})
    //     .Height(16)
    //     .Width(16)
    //     .Margin(4)
    //     ;

    OnValueChanged(Value());

}

void Lv2cLampElement::OnVariantChanged(Lv2cLampVariant value)
{
    OnValueChanged(Value());
}

void Lv2cLampElement::OnValueChanged(double value)
{
    double adjustedValue;
    if (Variant() == Lv2cLampVariant::OnOff)
    {
        adjustedValue = value >= 0.5 ? 1.0 : 0.0;
    }
    else
    {
        adjustedValue = 1 - (1 - value) * (1 - value);
    }
    if (this->displayValue != adjustedValue)
    {
        this->displayValue = adjustedValue;
        Invalidate();
    }
}

void Lv2cLampElement::UpdateLampColor()
{
    if (!this->IsMounted())
    {
        return;
    }

    Lv2cColor lampColor = Theme().lampOnColor;
    if (this->lampColor) {
        lampColor = *(this->lampColor);
    }
    offImage = gImageCache.GetLampImage(Window(),Theme().lampOffColor);
    onImage = gImageCache.GetLampImage(Window(),lampColor);
    Invalidate();

}

void Lv2cLampElement::OnUnmount()
{
}

void Lv2cLampElement::OnMount()
{
    Classes({Theme().lampStyle});

    UpdateLampColor();
}
void Lv2cLampElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);

    // LampImage*image = nullptr;

    Lv2cSurface surface;

    if (displayValue > 0.5) 
    {
        surface = this->onImage->png_surface;
    } else {
        surface = this->offImage->png_surface;
    }

    Lv2cSize clientSize = ClientSize();

    Lv2cRectangle imageBounds(0, 0,
                               clientSize.Width(),
                               clientSize.Height());
    if (imageBounds.Empty())
    {
        return;
    }
    if (!surface)
    {
        // gray marker if no image.
        dc.set_source(Lv2cColor(0.5, 0.5, 0.5, 0.25));
        dc.rectangle(imageBounds);
        dc.fill();
        return;
    }


    Lv2cSize imageSize = surface.size();
    if (imageSize.Width() <= 0 || imageSize.Height() <= 0)
    {
        return;
    }


    imageBounds = Lv2cRectangle(0, 0, clientSize.Width(), clientSize.Height());

    double rotation = 0;

    if (rotation != 0)
    {
        dc.save();
        dc.translate(clientSize.Width() / 2, clientSize.Height() / 2);
        dc.rotate(rotation * std::numbers::pi / 180.0);
        ;
        dc.translate(-clientSize.Width() / 2, -clientSize.Height() / 2);
    }


    auto savedOperator = dc.get_operator();
    dc.save();
    dc.round_corner_rectangle(Lv2cRectangle(0, 0, clientSize.Width(), clientSize.Height()), 
        Style().RoundCorners().PixelValue());
    dc.clip();

    dc.set_operator(cairo_operator_t::CAIRO_OPERATOR_OVER);
    dc.rectangle(imageBounds);
    dc.translate(imageBounds.Left(), imageBounds.Top());
    dc.scale(imageBounds.Width() / imageSize.Width(), imageBounds.Height() / imageSize.Height());
    dc.set_source(surface, 0, 0);

    dc.fill();
    dc.set_operator(savedOperator);
    dc.restore();
    if (rotation != 0)
    {
        dc.restore();
    }

}
