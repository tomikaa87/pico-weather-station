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
    , _currentHumidityLabel{ this }
    , _currentWindSpeedLabel{ this }
    , _currentWindGustSpeedLabel{ this }

    ,_clockHoursLabel{ this }
    ,_clockMinutesLabel{ this }
    ,_clockDateLabel{ this }
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
    _currentHumidityLabel.setName("CurrentHumidityLabel");
    _currentWindSpeedLabel.setName("CurrentWindSpeedLabel");
    _currentWindGustSpeedLabel.setName("CurrentWindGustSpeedLabel");

    _clockHoursLabel.setName("ClockHoursLabel");
    _clockMinutesLabel.setName("ClockMinutesLabel");
    _clockDateLabel.setName("ClockDateLabel");

    setupLayout();
    setWeatherIcon();

    _clockHoursLabel.setText("88");
    _clockMinutesLabel.setText("88");
    _clockDateLabel.setText("88 WWW");
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

void MainScreen::setCurrentHumidity(const int value)
{
    _currentHumidityLabel.setText(clampedValueToString(value, 0, 999));
}

void MainScreen::setCurrentWindSpeed(const int value)
{
    _currentWindSpeedLabel.setText(clampedValueToString(value, 0, 999));
}

void MainScreen::setCurrentWindGustSpeed(const int value)
{
    _currentWindGustSpeedLabel.setText(clampedValueToString(value, 0, 999));
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

    static const auto fromPhotoshopRect = [](Rect rect) {
        auto r = std::move(rect);
        r.moveTopLeft({ r.x(), r.y() - 1 });
        r.setSize({ r.width() + 1, r.height() + 1 });
        return r;
    };

    static const auto fromPhotoshopRectForSmall = [](Rect rect) {
        auto r = std::move(rect);
        r.moveTopLeft({ r.x(), r.y() - 1 });
        r.setSize({ r.width() + 1, r.height() + 1 });
        return r;
    };

    // Current conditions
    setupLargeNumberLabel(
        _currentTemperatureLabel,
        fromPhotoshopRect({ 80, 3, 52, 32 })
    );
    setupMediumNumberLabel(
        _currentSensorTempLabel,
        fromPhotoshopRect({ 136, 19, 26, 16 })
    );
    _currentSensorTempLabel.setAlignment(Label::Align::Left);
    setupMediumNumberLabel(
        _currentPressureLabel,
        fromPhotoshopRect({ 186, 3, 35, 16 })
    );
    setupMediumNumberLabel(
        _currentHumidityLabel,
        fromPhotoshopRect({ 186, 22, 26, 16 })
    );
    setupMediumNumberLabel(
        _currentWindSpeedLabel,
        fromPhotoshopRect({ 186, 41, 26, 16 })
    );
    setupSmallNumberLabel(
        _currentWindGustSpeedLabel,
        fromPhotoshopRectForSmall({ 217, 50, 17, 7 })
    );
    _currentWindGustSpeedLabel.setAlignment(Label::Align::Left);

    // Clock
    setupMediumNumberLabel(
        _clockHoursLabel,
        fromPhotoshopRect({ 200, 136, 17, 16 })
    );
    setupMediumNumberLabel(
        _clockMinutesLabel,
        fromPhotoshopRect({ 223, 136, 17, 16 })
    );
    _clockDateLabel.setFont(Font{ Font::Family::BitCell });
    _clockDateLabel.setRect(fromPhotoshopRectForSmall({ 202, 153, 38, 7 }));
    _clockDateLabel.setAlignment(Label::Align::Left);
    _clockDateLabel.setHeightCalculation(Label::HeightCalculation::NoDescent);
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

void MainScreen::setupSmallNumberLabel(Label& label, Rect rect)
{
    label.setFont(Font{ Font::Family::BitCellMonoNumbers });
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