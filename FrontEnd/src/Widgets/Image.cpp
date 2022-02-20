#include "Image.h"

#include "Display.h"

Image::Image(Widget* parent)
    : Widget{ parent }
{}

Image::Image(
    const unsigned char* imageData,
    const int width,
    const int height,
    Widget* parent
)
    : Widget{ parent }
{
    setImage(imageData, width, height);
}

void Image::setImage(
    const unsigned char* imageData,
    const int width,
    const int height
) {
    if (!imageData || width == 0 || height == 0) {
        return;
    }

    _imageData = imageData;
    _imageSize = Size{ width, height };

    setSize(_imageSize);
}

void Image::setInverted(const bool inverted)
{
    _inverted = inverted;
    _needsRepaint = true;
}

Size Image::imageSize() const
{
    return _imageSize;
}

bool Image::isNull() const
{
    return !_imageData || !_imageSize.isValid();
}

void Image::paint()
{
    if (!isNull()) {
        const auto globalRect = mapToGlobal(_rect);

        // _display->setClipRect(calculateClipRect());

        _display->setDrawColor(
            _inverted
                ? Display::Color::White
                : Display::Color::Black
        );

        _display->drawBitmap(
            globalRect.pos(),
            globalRect.w(),
            globalRect.h(),
            reinterpret_cast<const uint8_t*>(_imageData)
        );

        // _display->resetClipRect();
    }

    Widget::paint();
}