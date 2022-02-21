#pragma once

#include "../Widgets/Image.h"
#include "../Widgets/Label.h"
#include "../Widgets/Widget.h"

class Display;

class MainScreen final : public Widget
{
public:
    explicit MainScreen(Display* display);

    void setCurrentTemperature(int value);
    void setCurrentSensorTemperature(int value);
    void setCurrentPressure(int value);
    void setCurrentHumidity(int value);
    void setCurrentWindSpeed(int value);
    void setCurrentWindGustSpeed(int value);

protected:
    void paint() override;

private:
    Image _weatherIcon;
    Label _currentTemperatureLabel;
    Label _currentSensorTempLabel;
    Label _currentPressureLabel;
    Label _currentHumidityLabel;
    Label _currentWindSpeedLabel;
    Label _currentWindGustSpeedLabel;

    Label _clockHoursLabel;
    Label _clockMinutesLabel;
    Label _clockDateLabel;

    void setupLayout();
    void setWeatherIcon();

    static void setupLargeNumberLabel(
        Label& label,
        Rect rect
    );

    static void setupMediumNumberLabel(
        Label& label,
        Rect rect
    );

    static void setupSmallNumberLabel(
        Label& label,
        Rect rect
    );

    static std::string clampedValueToString(
        int value,
        int min,
        int max
    );
};