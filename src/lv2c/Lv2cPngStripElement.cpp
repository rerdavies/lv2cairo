#include "lv2c/Lv2cPngStripElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include <numbers>

using namespace lvtk;

void Lv2cPngStripElement::OnSourceChanged(const std::string &source)
{
    sourceChanged = true;
    if (this->IsMounted())
    {
        Load();
    }
}

void Lv2cPngStripElement::Load()
{
    if (sourceChanged && IsMounted())
    {
        sourceChanged = false;
        surface = this->Window()->GetPngImage(Source());
        if (surface)
        {
            Lv2cSize imageSize = surface.size();

            Lv2cRectangle tileSize = TileSize();
            if (tileSize.Empty())
            {
                tileSize = Lv2cSize(imageSize.Height(), imageSize.Height());
            }
            if (this->tileSize != tileSize)
            {
                this->tileSize = tileSize;
                InvalidateLayout();
            }
            this->tileCount = imageSize.Width() / tileSize.Width();
            Invalidate();
        }
    }
}

void Lv2cPngStripElement::OnMount()
{
    super::OnMount();
    Load();
}

void Lv2cPngStripElement::OnDraw(Lv2cDrawingContext &dc)
{
    super::OnDraw(dc);
    if (surface && !tileSize.Empty())
    {
        int tile = std::round((tileCount-1) * Value());

        Lv2cRectangle sourceRectangle = Lv2cRectangle(0,0,tileSize.Width(),tileSize.Height());
        sourceRectangle = sourceRectangle.Translate(tileSize.Width()*tile,0);

        Lv2cSize clientSize = ClientSize();
        Lv2cRectangle destRect = Lv2cRectangle(0,0,clientSize.Width(),clientSize.Height());
        dc.save();
        {
            dc.rectangle(destRect);
            dc.clip();

            dc.rectangle(destRect);
            dc.scale(destRect.Width()/sourceRectangle.Width(),destRect.Width()/sourceRectangle.Height());
            dc.translate(-sourceRectangle.Left(),-sourceRectangle.Top());
            dc.set_source(surface,0,0);
            dc.fill();

        }
        dc.restore();
            
    }
}
Lv2cSize Lv2cPngStripElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context)
{
    Lv2cSize result;
    Load();
    double width = clientConstraint.Width();
    double height = clientConstraint.Height();
    if (!tileSize.Empty())
    {
        if (width == 0 && height != 0)
        {
            width = std::ceil(height*tileSize.Width()/tileSize.Height());
        } else if (height == 0 && width != 0)
        {
            height = std::ceil(width*tileSize.Height()/tileSize.Width());
        } else if (width == 0 && height == 0)
        {
            width = tileSize.Width();
            height = tileSize.Height();
        }
    }
    return Lv2cSize(width,height);

}

void Lv2cPngStripElement::OnValueChanged(double value)
{
    Invalidate();
}


Lv2cPngStripElement::Lv2cPngStripElement()
{
    SourceProperty.SetElement(this,&Lv2cPngStripElement::OnSourceChanged);
}
