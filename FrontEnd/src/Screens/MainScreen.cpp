#include "MainScreen.h"

#include "../Icons.h"

#include "../Widgets/Display.h"
#include "../Widgets/Font.h"

MainScreen::MainScreen(Display* const display)
    : Widget{ display }
    , _weatherIcon{ this }
    , _currentTemperatureLabel{ this }
{
    setName("MainScreen");
    setRect({
        Point{ 0, 0 },
        display->size()
    });

    _weatherIcon.setName("WeatherIcon");
    _currentTemperatureLabel.setName("CurrentTemperatureLabel");

    setupLayout();
    setWeatherIcon();

    _currentTemperatureLabel.setText("-23.4");
}

void MainScreen::setupLayout()
{
    _weatherIcon.setPos({ 2, 2 });

    _currentTemperatureLabel.setFont(Font{ Font::Family::Pxl16x8_x2 });
    _currentTemperatureLabel.setHeightCalculation(Label::HeightCalculation::NoDescent);
    _currentTemperatureLabel.setPos({ 74, 2 });
    _currentTemperatureLabel.setWidth(70);
    _currentTemperatureLabel.setAlignment(Label::Align::Right);
    _currentTemperatureLabel.setBackgroundEnabled(false);
}

void MainScreen::setWeatherIcon()
{
    _weatherIcon.setImage(
        Graphics::Icons::Weather::CloudsWithSnowflakes,
        Graphics::Icons::Weather::Width,
        Graphics::Icons::Weather::Height
    );
}