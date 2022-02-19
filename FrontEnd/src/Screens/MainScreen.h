#pragma once

#include "../Widgets/Image.h"
#include "../Widgets/Label.h"
#include "../Widgets/Widget.h"

class Display;

class MainScreen final : public Widget
{
public:
    explicit MainScreen(Display* display);

private:
    Image _weatherIcon;
    Label _currentTemperatureLabel;

    void setupLayout();
    void setWeatherIcon();
};