#pragma once

#include "Widget.h"

class Image : public Widget
{
public:
    explicit Image(Widget* parent);
    Image(
        const unsigned char* imageData,
        int width,
        int height,
        Widget* parent
    );

    void setImage(
        const unsigned char* imageData,
        int width,
        int height
    );

    void setInverted(bool inverted);

    [[nodiscard]] Size imageSize() const;

    [[nodiscard]] bool isNull() const;

    void paint() override;

private:
    const unsigned char* _imageData = nullptr;
    Size _imageSize;
    bool _inverted = false;
};