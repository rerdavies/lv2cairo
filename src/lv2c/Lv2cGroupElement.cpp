#include "lv2c/Lv2cGroupElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"

using namespace lvtk;

Lv2cGroupElement::Lv2cGroupElement()
{
    this->typography = Lv2cTypographyElement::Create();
    typography->Variant(Lv2cTypographyVariant::Caption);
    typography->Style()
        .Margin({8, 0, 8, 0});
    super::AddChild(typography);
    this->flexGrid = Lv2cFlexGridElement::Create();
    flexGrid->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .FlexWrap(Lv2cFlexWrap::Wrap)
        .RowGap(16);
        
    super::AddChild(flexGrid);

    TextProperty.Bind(typography->TextProperty);
}

Lv2cSize Lv2cGroupElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    Lv2cSize result = super::Arrange(available, context);

    // shift the position of the typography element into the border/margin area.
    Lv2cRectangle textBounds = typography->Bounds();
    auto corners = Style().RoundCorners().PixelValue();

    Lv2cRectangle bounds{available};
    Lv2cRectangle borderRect = bounds.Inset(Style().Margin().PixelValue());
    Lv2cRectangle paddingRect = borderRect.Inset(Style().BorderWidth().PixelValue());
    Lv2cRectangle clientRect = paddingRect.Inset(Style().Padding().PixelValue());

    Lv2cPoint offset = {
        bounds.Left() - clientRect.Left() + corners.topLeft + 12,
        bounds.Top() - clientRect.Top()};
    textBounds = textBounds.translate(offset);
    typography->Layout(textBounds);
    return result;
}

void Lv2cGroupElement::OnDraw(Lv2cDrawingContext &dc)
{
    // draw as usual, but clip out the text area.
    if (Text().length() == 0)
    {
        super::OnDraw(dc);
    }
    else
    {
        dc.save();
        {
            Lv2cRectangle marginBounds = this->ScreenBounds().Translate(-ScreenClientBounds().Left(), -ScreenClientBounds().Top());
            auto tFill = dc.get_fill_rule();
            dc.set_fill_rule(cairo_fill_rule_t::CAIRO_FILL_RULE_EVEN_ODD);

            Lv2cRectangle textBounds = typography->ScreenBounds().Translate(-ScreenClientBounds().Left(), -ScreenClientBounds().Top());

            dc.rectangle(marginBounds);
            dc.rectangle(textBounds);
            dc.clip();
            dc.set_fill_rule(tFill);

            super::OnDraw(dc);
        }
        dc.restore();
    }
}

void Lv2cGroupElement::OnMount()
{
    super::OnMount();
    Classes({Theme().portGroupStyle});
}

void Lv2cGroupElement::AddChild(std::shared_ptr<Lv2cElement> child)
{
    flexGrid->AddChild(child);
}



size_t Lv2cGroupElement::ChildCount() const
{
    return flexGrid->ChildCount();
}
void Lv2cGroupElement::AddChild(std::shared_ptr<Lv2cElement> child, size_t position)
{
    flexGrid->AddChild(child);
}

bool Lv2cGroupElement::RemoveChild(std::shared_ptr<Lv2cElement> element)
{
    return flexGrid->RemoveChild(element);
}
void Lv2cGroupElement::RemoveChild(size_t index)
{
    flexGrid->RemoveChild(index);
}

Lv2cElement::ptr Lv2cGroupElement::Child(size_t index)
{
    return flexGrid->Child(index);
}
const Lv2cElement *Lv2cGroupElement::Child(size_t index) const
{
    return flexGrid->Child(index).get();
}

void Lv2cGroupElement::Children(const std::vector<Lv2cElement::ptr> &children)
{
    return flexGrid->Children(children);
}
void Lv2cGroupElement::RemoveAllChildren()
{
    flexGrid->RemoveAllChildren();
}

std::vector<Lv2cElement::ptr> &Lv2cGroupElement::Children() { return flexGrid->Children(); }
