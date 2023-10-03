#include "lvtk/LvtkGroupElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"

using namespace lvtk;

LvtkGroupElement::LvtkGroupElement()
{
    this->typography = LvtkTypographyElement::Create();
    typography->Variant(LvtkTypographyVariant::Caption);
    typography->Style()
        .Margin({8, 0, 8, 0});
    super::AddChild(typography);
    this->flexGrid = LvtkFlexGridElement::Create();
    flexGrid->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .FlexWrap(LvtkFlexWrap::Wrap)
        .RowGap(16);
        
    super::AddChild(flexGrid);

    TextProperty.Bind(typography->TextProperty);
}

LvtkSize LvtkGroupElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    LvtkSize result = super::Arrange(available, context);

    // shift the position of the typography element into the border/margin area.
    LvtkRectangle textBounds = typography->Bounds();
    auto corners = Style().RoundCorners().PixelValue();

    LvtkRectangle bounds{available};
    LvtkRectangle borderRect = bounds.Inset(Style().Margin().PixelValue());
    LvtkRectangle paddingRect = borderRect.Inset(Style().BorderWidth().PixelValue());
    LvtkRectangle clientRect = paddingRect.Inset(Style().Padding().PixelValue());

    LvtkPoint offset = {
        bounds.Left() - clientRect.Left() + corners.topLeft + 12,
        bounds.Top() - clientRect.Top()};
    textBounds = textBounds.translate(offset);
    typography->Layout(textBounds);
    return result;
}

void LvtkGroupElement::OnDraw(LvtkDrawingContext &dc)
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
            LvtkRectangle marginBounds = this->ScreenBounds().Translate(-ScreenClientBounds().Left(), -ScreenClientBounds().Top());
            auto tFill = dc.get_fill_rule();
            dc.set_fill_rule(cairo_fill_rule_t::CAIRO_FILL_RULE_EVEN_ODD);

            LvtkRectangle textBounds = typography->ScreenBounds().Translate(-ScreenClientBounds().Left(), -ScreenClientBounds().Top());

            dc.rectangle(marginBounds);
            dc.rectangle(textBounds);
            dc.clip();
            dc.set_fill_rule(tFill);

            super::OnDraw(dc);
        }
        dc.restore();
    }
}

void LvtkGroupElement::OnMount()
{
    super::OnMount();
    Classes({Theme().portGroupStyle});
}

void LvtkGroupElement::AddChild(std::shared_ptr<LvtkElement> child)
{
    flexGrid->AddChild(child);
}



size_t LvtkGroupElement::ChildCount() const
{
    return flexGrid->ChildCount();
}
void LvtkGroupElement::AddChild(std::shared_ptr<LvtkElement> child, size_t position)
{
    flexGrid->AddChild(child);
}

bool LvtkGroupElement::RemoveChild(std::shared_ptr<LvtkElement> element)
{
    return flexGrid->RemoveChild(element);
}
void LvtkGroupElement::RemoveChild(size_t index)
{
    flexGrid->RemoveChild(index);
}

LvtkElement::ptr LvtkGroupElement::Child(size_t index)
{
    return flexGrid->Child(index);
}
const LvtkElement *LvtkGroupElement::Child(size_t index) const
{
    return flexGrid->Child(index).get();
}

void LvtkGroupElement::Children(const std::vector<LvtkElement::ptr> &children)
{
    return flexGrid->Children(children);
}
void LvtkGroupElement::RemoveAllChildren()
{
    flexGrid->RemoveAllChildren();
}

std::vector<LvtkElement::ptr> &LvtkGroupElement::Children() { return flexGrid->Children(); }
