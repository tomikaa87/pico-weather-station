#include "MainScreen.h"

#include "../Icons.h"

#include "../Widgets/Display.h"
#include "../Widgets/Font.h"

#include <algorithm>
#include <string>

#include <fmt/core.h>

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

    , _internalSensorTempLabel{ this }
    , _internalSensorHumidityLabel{ this }

    , _temperatureGraphLabels{ this }
    , _pressureGraphLabels{ this }

    , _temperatureGraph{ this }
    , _pressureGraph{ this }
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

    _temperatureGraphLabels.setName("TemperatureGraphLabels");
    _pressureGraphLabels.setName("PressureGraphLabels");
    _temperatureGraph.setName("TemperatureGraph");
    _pressureGraph.setName("PressureGraph");

    setupLayout();
    setWeatherIcon();

    _clockHoursLabel.setText("88");
    _clockMinutesLabel.setText("88");
    _clockDateLabel.setText("88 WWW");
}

void MainScreen::setCurrentTemperature(const int value)
{
    const auto clampedValue = Utils::clamp(value, -99, 99);
    _currentTemperatureLabel.setText(std::to_string(clampedValue));
    _temperatureGraph.addSample(clampedValue);
    _temperatureGraphLabels.setRange(_temperatureGraph.sampleMin(), _temperatureGraph.sampleMax());
}

void MainScreen::setCurrentSensorTemperature(const int value)
{
    _currentSensorTempLabel.setText(std::to_string(Utils::clamp(value, -99, 99)));
}

void MainScreen::setCurrentPressure(const int value)
{
    const auto clampedValue = Utils::clamp(value, 0, 9999);
    _currentPressureLabel.setText(std::to_string(clampedValue));
    _pressureGraph.addSample(clampedValue);
    _pressureGraphLabels.setRange(_pressureGraph.sampleMin(), _pressureGraph.sampleMax());
}

void MainScreen::setCurrentHumidity(const int value)
{
    _currentHumidityLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void MainScreen::setCurrentWindSpeed(const int value)
{
    _currentWindSpeedLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void MainScreen::setCurrentWindGustSpeed(const int value)
{
    _currentWindGustSpeedLabel.setText(std::to_string(Utils::clamp(value, 0, 999)));
}

void MainScreen::setInternalSensorTemperature(const float value)
{
    _internalSensorTempLabel.setText(
        fmt::format(
            "{:.1f}",
            Utils::clamp(value, 0.f, 99.f)
        )
    );
}

void MainScreen::setInternalSensorHumidity(const float value)
{
    _internalSensorHumidityLabel.setText(
        fmt::format(
            "{:.1f}",
            Utils::clamp(value, 0.f, 99.f)
        )
    );
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
        fromPhotoshopRectForSmall({ 14, 137, 20, 7 }),
        false
    );
    setupSmallNumberLabel(
        _internalSensorHumidityLabel,
        fromPhotoshopRectForSmall({ 14, 150, 20, 7 }),
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

    // Graph widgets
    _temperatureGraphLabels.setRect({ 15, 75, 23, 25 });
    _pressureGraphLabels.setRect({ 15, 105, 23, 25 });
    _temperatureGraph.setRect({ 44, 77, 49, 19 });
    _pressureGraph.setRect({ 44, 107, 49, 19 });
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
}

void MainScreen::setupMediumNumberLabel(Label& label, Rect rect)
{
    label.setFont(Font{ Font::Family::Pxl16x8_Mono });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setRect(std::move(rect));
    label.setAlignment(Label::Align::Right);
}

void MainScreen::setupSmallNumberLabel(Label& label, Rect rect, const bool monospaced)
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

MainScreen::GraphAxisLabelsWidget::GraphAxisLabelsWidget(Widget* parent)
    : Widget{ parent }
    , _minLabel{ this }
    , _maxLabel{ this }
{
    setupLabel(_minLabel);
    setupLabel(_maxLabel);
}

void MainScreen::GraphAxisLabelsWidget::setRange(const int min, const int max)
{
    _minLabel.setText(std::to_string(min));
    _maxLabel.setText(std::to_string(max));
}

void MainScreen::GraphAxisLabelsWidget::onResize()
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

void MainScreen::GraphAxisLabelsWidget::setupLabel(Label& label)
{
    label.setFont(Font{ Font::Family::BitCell });
    label.setHeightCalculation(Label::HeightCalculation::NoDescent);
    label.setAlignment(Label::Align::Right);
    label.setHeight(8);
}

MainScreen::GraphWidget::GraphWidget(Widget* parent)
    : Widget{ parent }
{}

void MainScreen::GraphWidget::addSample(const int sample)
{
    _samples.push_back(sample);
    trimSamples();
    updateRanges();

    _needsRepaint = true;
}

void MainScreen::GraphWidget::paint()
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

void MainScreen::GraphWidget::onResize()
{
    trimSamples();
    updateRanges();
    Widget::onResize();
}

void MainScreen::GraphWidget::trimSamples()
{
    while (!_samples.empty() && _samples.size() > _rect.width()) {
        _samples.pop_front();
    }
}
void MainScreen::GraphWidget::updateRanges()
{
    const auto& [minIt, maxIt] = std::minmax_element(
        std::cbegin(_samples),
        std::cend(_samples)
    );

    _sampleMin = minIt != std::cend(_samples) ? *minIt : 0;
    _sampleMax = maxIt != std::cend(_samples) ? *maxIt : 0;

    std::cout << __FUNCTION__ << ": min=" << _sampleMin << ", max=" << _sampleMax << '\n';
}