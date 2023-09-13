#include "lvtk/LvtkPngStripElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include <numbers>

using namespace lvtk;

void LvtkPngStripElement::OnSourceChanged(const std::string &source)
{
    sourceChanged = true;
    if (this->IsMounted())
    {
        Load();
    }
}

void LvtkPngStripElement::Load()
{
    if (sourceChanged && IsMounted())
    {
        sourceChanged = false;
        surface = this->Window()->GetPngImage(Source());
        if (surface)
        {
            LvtkSize imageSize = surface.size();

            LvtkRectangle tileSize = TileSize();
            if (tileSize.Empty())
            {
                tileSize = LvtkSize(imageSize.Height(), imageSize.Height());
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

void LvtkPngStripElement::OnMount()
{
    super::OnMount();
    Load();
}

void LvtkPngStripElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);
    if (surface && !tileSize.Empty())
    {
        int tile = std::round((tileCount-1) * Value());

        LvtkRectangle sourceRectangle = LvtkRectangle(0,0,tileSize.Width(),tileSize.Height());
        sourceRectangle = sourceRectangle.Translate(tileSize.Width()*tile,0);

        LvtkSize clientSize = ClientSize();
        LvtkRectangle destRect = LvtkRectangle(0,0,clientSize.Width(),clientSize.Height());
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
LvtkSize LvtkPngStripElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &context)
{
    LvtkSize result;
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
    return LvtkSize(width,height);

}

void LvtkPngStripElement::OnValueChanged(double value)
{
    Invalidate();
}


LvtkPngStripElement::LvtkPngStripElement()
{
    SourceProperty.SetElement(this,&LvtkPngStripElement::OnSourceChanged);
}
