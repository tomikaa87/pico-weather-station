#include "WeatherStation.h"

#include "../Icons.h"

#include "../Widgets/Display.h"
#include "../Widgets/Font.h"

#include <algorithm>
#include <string>

#include <fmt/core.h>

namespace Screens
{

WeatherStation::WeatherStation(Display* const display)
    : Widget{ display }
    , _weatherIcon{ this }
    , _currentTemperatureLabel{ this }
    , _currentSensorTempLabel{ this }
    , _currentMinTempLabel{ this }
    , _currentMaxTempLabel{ this }
    , _currentPressureLabel{ this }
    , _currentHumidityLabel{ this }
    , _currentWindSpeedLabel{ this }
    , _currentWindGustSpeedLabel{ this }

    ,_clockHoursLabel{ this }
    ,_clockMinutesLabel{ this }
    ,_clockDateLabel{ this }

    , _internalSensorTempLabel{ this }
    , _internalSensorHumidityLabel{ this }

    , _forecastWidgets{
        ForecastWidget{ this },
        ForecastWidget{ this },
        ForecastWidget{ this }
    }

    , _temperatureGraphLabels{ this }
    , _pressureGraphLabels{ this }

    , _temperatureGraph{ this }
    , _pressureGraph{ this }
{
    setName("WeatherStation");
    setRect({
        Point{ 0, 0 },
        display->size()
    });

    _weatherIcon.setName("WeatherIcon");
    _currentTemperatureLabel.setName("CurrentTemperatureLabel");
    _currentSensorTempLabel.setName("CurrentSensorTempLabel");
    _currentMinTempLabel.setName("CurrentMinTempLabel");
    _currentMaxTempLabel.setName("CurrentMaxTempLabel");
    _currentPressureLabel.setName("CurrentPressureLabel");
    _currentHumidityLabel.setName("CurrentHumidityLabel");
    _currentWindSpeedLabel.setName("CurrentWindSpeedLabel");
    _currentWindGustSpeedLabel.setName("CurrentWindGustSpeedLabel");

    _clockHoursLabel.setName("ClockHoursLabel");
    _clockMinutesLabel.setName("ClockMinutesLabel");
    _clockDateLabel.setName("ClockDateLabel");

    _temperatureGraphLabels.setName("TemperatureGraphLabels");
    _pressureGraphLabels.setName("PressureGraphLabels");
    _temperatureGraph.setName("TemperatureGraph");
    _pressureGraph.setName("PressureGraph");

    setupLayout();
    setWeatherIcon();
}

void WeatherStation::setCurrentTemperature(const int value)
{
    const auto clampedValue = Utils::clamp(value, -99, 99);
    _currentTemperatureLabel.setText(std::to_string(clampedValue));
    _temperatureGraph.addSample(clampedValue);
    _temperatureGraphLabels.setRange(_temperatureGraph.sampleMin(), _temperatureGraph.sampleMax());
}

void WeatherStation::setCurrentSensorTemperature(const int value)
{
    _currentSensorTempLabel.setText(std::to_string(Utils::clamp(value, -99, 99)));
}

void WeatherStation::setCurrentMinimumTemperature(const int value)
{
    _currentMinTempLabel.setText(std::to_string(Utils::clamp(value, -99, 99)));
}

void WeatherStation::setCurrentMaximumTemperature(const int value)
{
    _currentMaxTempLabel.setText(std::to_string(Utils::clamp(value, -99, 99)));
}

void WeatherStation::setCurrentPressure(const int value)
{
    const auto clampedValue = Utils::clamp(value, 0, 9999);
    _currentPressureLabel.setText(std::to_string(clampedValue));
    _pressureGraph.addSample(clampedValue);
    _pressureGraphLabels.setRange(_pressureGraph.sampleMin(), _pressureGraph.sampleMax());
}

void WeatherStation::setCurrentHumidity(const int value)
{
    _currentHumidityLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void WeatherStation::setCurrentWindSpeed(const int value)
{
    _currentWindSpeedLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void WeatherStation::setCurrentWindGustSpeed(const int value)
{
    _currentWindGustSpeedLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void WeatherStation::setInternalSensorTemperature(const float value)
{
    _internalSensorTempLabel.setText(
        fmt::format(
            "{:.1f}",
            Utils::clamp(value, -99.f, 99.f)
        )
    );
}

void WeatherStation::setInternalSensorHumidity(const float value)
{
    _internalSensorHumidityLabel.setText(
        fmt::format(
            "{:.1f}",
            Utils::clamp(value, 0.f, 100.f)
        )
    );
}

void WeatherStation::setClockTime(const int hours, const int minutes)
{
    _clockHoursLabel.setText(std::to_string(Utils::clamp(hours, 0, 23)));
    _clockMinutesLabel.setText(std::to_string(Utils::clamp(minutes, 0, 59)));
}

void WeatherStation::setClockDate(const int day, const std::string& dayOfWeek)
{
    _clockDateLabel.setText(
        fmt::format("{:02d} {}", Utils::clamp(day, 1, 31), dayOfWeek)
    );
}

void WeatherStation::paint()
{
    _display->setDrawColor(Display::Color::Black);
    _display->drawBitmap(
        mapToGlobal({ 0, 0 }),
        Graphics::MainScreenWidth,
        Graphics::MainScreenHeight,
        Graphics::WeatherStationBackground
    );

    Widget::paint();
}

void WeatherStation::setupLayout()
{
    _weatherIcon.setPos({ 2, 2 });

    static const auto fromPhotoshopRectForLarge = [](Rect rect) {
        auto r = std::move(rect);
        r.moveTopLeft({ r.x(), r.y() - 1 });
        r.setSize({ r.width() + 1, r.height() + 1 });
        return r;
    };

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
        fromPhotoshopRectForLarge({ 80, 3, 52, 32 })
    );
    setupMediumNumberLabel(
        _currentSensorTempLabel,
        fromPhotoshopRect({ 136, 19, 26, 16 })
    );
    _currentSensorTempLabel.setAlignment(Label::Align::Left);
    setupMediumNumberLabel(
        _currentMinTempLabel,
        fromPhotoshopRect({ 86, 41, 26, 16 })
    );
    setupMediumNumberLabel(
        _currentMaxTempLabel,
        fromPhotoshopRect({ 136, 41, 26, 16 })
    );
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

    // Internal sensor
    setupSmallNumberLabel(
        _internalSensorTempLabel,
        fromPhotoshopRectForSmall({ 10, 137, 24, 7 }),
        false
    );
    setupSmallNumberLabel(
        _internalSensorHumidityLabel,
        fromPhotoshopRectForSmall({ 10, 150, 24, 7 }),
        false
    );

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
    _clockDateLabel.setAlignment(Label::Align::Left);
    _clockDateLabel.setHeightCalculation(Label::HeightCalculation::NoDescent);
    _clockDateLabel.setRect(fromPhotoshopRectForSmall({ 202, 153, 38, 7 }));

    // Forecast widgets
    _forecastWidgets[0].setRect({ 99, 73, 47, 60 });
    _forecastWidgets[1].setRect({ 147, 73, 47, 60 });
    _forecastWidgets[2].setRect({ 195, 73, 47, 60 });

    for (auto& fw : _forecastWidgets) {
        fw.setDate("88 WWW");
        fw.setMinimumTemperature(-88);
        fw.setMaximumTemperature(-88);
        fw.setWindSpeed(888);
    }

    // Graph widgets
    _temperatureGraphLabels.setRect({ 15, 75, 23, 25 });
    _pressureGraphLabels.setRect({ 15, 105, 23, 25 });
    _temperatureGraph.setRect({ 44, 77, 49, 19 });
    _pressureGraph.setRect({ 44, 107, 49, 19 });
}

void WeatherStation::setWeatherIcon()
{
    _weatherIcon.setImage(
        Graphics::Icons::Weather::CloudsWithSnowflakes,
        Graphics::Icons::Weather::Width,
        Graphics::Icons::Weather::Height
    );
}

void WeatherStation::setupLargeNumberLabel(Label& label, Rect rect)
{
    label.setFont(Font{ Font::Family::Pxl16x8_Mono_x2 });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setRect(std::move(rect));
    label.setAlignment(Label::Align::Right);
}

void WeatherStation::setupMediumNumberLabel(Label& label, Rect rect)
{
    label.setFont(Font{ Font::Family::Pxl16x8_Mono });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setRect(std::move(rect));
    label.setAlignment(Label::Align::Right);
}

void WeatherStation::setupSmallNumberLabel(Label& label, Rect rect, const bool monospaced)
{
    label.setFont(Font{
        monospaced
            ? Font::Family::BitCellMonoNumbers
            : Font::Family::BitCell
    });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setRect(std::move(rect));
    label.setAlignment(Label::Align::Right);
}

WeatherStation::ForecastWidget::ForecastWidget(Widget* const parent)
    : Widget{ parent }
    , _dateLabel{ this }
    , _minTempLabel{ this }
    , _maxTempLabel{ this }
    , _windSpeedLabel{ this }
{
    setBackgroundEnabled(false);

    setupLabel(_dateLabel, Label::Align::Center, false);
    setupLabel(_minTempLabel, Label::Align::Right);
    setupLabel(_maxTempLabel, Label::Align::Right);
    setupLabel(_windSpeedLabel, Label::Align::Right);
}

void WeatherStation::ForecastWidget::setDate(std::string date)
{
    _dateLabel.setText(std::move(date));
}

void WeatherStation::ForecastWidget::setMinimumTemperature(const int value)
{
    _minTempLabel.setText(std::to_string(Utils::clamp(value, -99, 99)));
}

void WeatherStation::ForecastWidget::setMaximumTemperature(const int value)
{
    _maxTempLabel.setText(std::to_string(Utils::clamp(value, -99, 99)));
}

void WeatherStation::ForecastWidget::setWindSpeed(const int value)
{
    _windSpeedLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void WeatherStation::ForecastWidget::onResize()
{
    _dateLabel.setRect({ 0, 1, _rect.width(), _dateLabel.rect().height() });
    _minTempLabel.setRect({ 9, 33, 16, _minTempLabel.rect().height() });
    _maxTempLabel.setRect({ 30, 33, 16, _maxTempLabel.rect().height() });
    _windSpeedLabel.setRect({ 14, 47, 18, _windSpeedLabel.rect().height() });

    Widget::onResize();
}

void WeatherStation::ForecastWidget::setupLabel(
    Label& label,
    const Label::Align alignment,
    const bool numbersOnly
) {
    label.setFont(Font{ numbersOnly ? Font::Family::BitCellMonoNumbers : Font::Family::BitCell });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setAlignment(alignment);
    label.setHeight(8);
}

WeatherStation::GraphAxisLabelsWidget::GraphAxisLabelsWidget(Widget* const parent)
    : Widget{ parent }
    , _maxLabel{ this }
    , _minLabel{ this }
{
    setupLabel(_minLabel);
    setupLabel(_maxLabel);
}

void WeatherStation::GraphAxisLabelsWidget::setRange(const int min, const int max)
{
    _minLabel.setText(std::to_string(min));
    _maxLabel.setText(std::to_string(max));
}

void WeatherStation::GraphAxisLabelsWidget::onResize()
{
    auto clientRect = _rect;
    clientRect.moveTopLeft(Point{ 0, 0 });

    auto maxLabelRect = _maxLabel.rect();
    maxLabelRect.setWidth(clientRect.width());
    maxLabelRect.moveTopRight(clientRect.topRight() - Point{ 0, 1 });
    _maxLabel.setRect(maxLabelRect);

    auto minLabelRect = _minLabel.rect();
    minLabelRect.setWidth(clientRect.width());
    minLabelRect.moveBottomRight(clientRect.bottomRight());
    _minLabel.setRect(minLabelRect);

    Widget::onResize();
}

void WeatherStation::GraphAxisLabelsWidget::setupLabel(Label& label)
{
    label.setFont(Font{ Font::Family::BitCell });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setAlignment(Label::Align::Right);
    label.setHeight(8);
}

WeatherStation::GraphWidget::GraphWidget(Widget* const parent)
    : Widget{ parent }
{}

void WeatherStation::GraphWidget::addSample(const int sample)
{
    _samples.push_back(sample);
    trimSamples();
    updateRanges();

    _needsRepaint = true;
}

void WeatherStation::GraphWidget::paint()
{
    // Clear the background
    _display->setDrawColor(Display::Color::White);
    _display->fillRect(mapToGlobal(_rect));

    // Draw the bars
    if (_sampleMin != _sampleMax && !_samples.empty()) {
        _display->setDrawColor(Display::Color::Black);
        int x = 0;
        const auto globalRect = mapToGlobal(_rect);
        for (const auto sample : _samples) {
            const int lineHeight = (sample - _sampleMin) * _rect.height() / (_sampleMax - _sampleMin);

            // std::cout << __FUNCTION__ << ": sample=" << sample << ", lineHeight=" << lineHeight << '\n';

            if (lineHeight > 0) { 
                _display->drawLine(
                    Point{ globalRect.x() + x, globalRect.bottom() - lineHeight - 1 },
                    Point{ globalRect.x() + x, globalRect.bottom() }
                );
            }

            ++x;
        }
    }

    Widget::paint();
}

void WeatherStation::GraphWidget::onResize()
{
    trimSamples();
    updateRanges();
    Widget::onResize();
}

void WeatherStation::GraphWidget::trimSamples()
{
    while (!_samples.empty() && _samples.size() > _rect.width()) {
        _samples.pop_front();
    }
}
void WeatherStation::GraphWidget::updateRanges()
{
    const auto& [minIt, maxIt] = std::minmax_element(
        std::cbegin(_samples),
        std::cend(_samples)
    );

    _sampleMin = minIt != std::cend(_samples) ? *minIt : 0;
    _sampleMax = maxIt != std::cend(_samples) ? *maxIt : 0;

    std::cout << __FUNCTION__ << ": min=" << _sampleMin << ", max=" << _sampleMax << '\n';
}

}