#include "MainScreen.h"

#include "../Icons.h"

#include "../Widgets/Display.h"
#include "../Widgets/Font.h"

#include <algorithm>
#include <string>

MainScreen::MainScreen(Display* const display)
    : Widget{ display }
    , _weatherIcon{ this }
    , _currentTemperatureLabel{ this }
    , _currentSensorTempLabel{ this }
    , _currentPressureLabel{ this }
{
    setName("MainScreen");
    setRect({
        Point{ 0, 0 },
        display->size()
    });

    _weatherIcon.setName("WeatherIcon");
    _currentTemperatureLabel.setName("CurrentTemperatureLabel");
    _currentSensorTempLabel.setName("CurrentSensorTempLabel");
    _currentPressureLabel.setName("CurrentPressureLabel");

    setupLayout();
    setWeatherIcon();
}

void MainScreen::setCurrentTemperature(const int value)
{
    _currentTemperatureLabel.setText(clampedValueToString(value, -99, 99));
}

void MainScreen::setCurrentSensorTemperature(const int value)
{
    _currentSensorTempLabel.setText(clampedValueToString(value, -99, 99));
}

void MainScreen::setCurrentPressure(const int value)
{
    _currentPressureLabel.setText(clampedValueToString(value, 0, 9999));
}

void MainScreen::paint()
{
    _display->setDrawColor(Display::Color::Black);
    _display->drawBitmap(
        mapToGlobal({ 0, 0 }),
        Graphics::MainScreenWidth,
        Graphics::MainScreenHeight,
        Graphics::MainScreenBackground
    );

    Widget::paint();
}

void MainScreen::setupLayout()
{
    _weatherIcon.setPos({ 2, 2 });

    setupLargeNumberLabel(_currentTemperatureLabel, { 81, 2, 52, 33 });
    setupMediumNumberLabel(_currentSensorTempLabel, { 136, 18, 26, 17 });
    setupMediumNumberLabel(_currentPressureLabel, { 185 + 1, 3 - 1, 35 + 1, 16 + 1 });
}

void MainScreen::setWeatherIcon()
{
    _weatherIcon.setImage(
        Graphics::Icons::Weather::CloudsWithSnowflakes,
        Graphics::Icons::Weather::Width,
        Graphics::Icons::Weather::Height
    );
}

void MainScreen::setupLargeNumberLabel(Label& label, Rect rect)
{
    label.setFont(Font{ Font::Family::Pxl16x8_Mono_x2 });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setRect(std::move(rect));
    label.setAlignment(Label::Align::Right);
    // label.setBackgroundEnabled(false);
}

void MainScreen::setupMediumNumberLabel(Label& label, Rect rect)
{
    label.setFont(Font{ Font::Family::Pxl16x8_Mono });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setRect(std::move(rect));
    label.setAlignment(Label::Align::Right);
    // label.setBackgroundEnabled(false);
}

std::string MainScreen::clampedValueToString(
    const int value,
    const int min,
    const int max
) {
    return std::to_string(
        std::max(min, std::min(max, value))
    );
}