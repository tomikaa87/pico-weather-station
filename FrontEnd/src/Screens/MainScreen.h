#pragma once

#include "../Widgets/Image.h"
#include "../Widgets/Label.h"
#include "../Widgets/Widget.h"

#include <array>
#include <deque>

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

    void setInternalSensorTemperature(float value);
    void setInternalSensorHumidity(float value);

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

    Label _internalSensorTempLabel;
    Label _internalSensorHumidityLabel;

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
        Rect rect,
        bool monospaced = true
    );

    class ForecastWidget final : public Widget
    {
    public:
        explicit ForecastWidget(Widget* parent);

        inline void setDate(std::string date);
        inline void setMinimumTemperature(int value);
        inline void setMaximumTemperature(int value);
        inline void setWindSpeed(int value);

    protected:
        void onResize() override;

    private:
        Label _dateLabel;
        Label _minTempLabel;
        Label _maxTempLabel;
        Label _windSpeedLabel;

        static void setupLabel(
            Label& label,
            Label::Align alignment,
            bool numbersOnly = true
        );
    };

    class GraphAxisLabelsWidget final : public Widget
    {
    public:
        explicit GraphAxisLabelsWidget(Widget* parent);

        void setRange(int min, int max);

    protected:
        void onResize() override;

    private:
        Label _maxLabel;
        Label _minLabel;

        static void setupLabel(Label& label);
    };

    class GraphWidget final : public Widget
    {
    public:
        explicit GraphWidget(Widget* parent);

        void addSample(int sample);

        inline int sampleMin() const
        {
            return _sampleMin;
        }

        inline int sampleMax() const
        {
            return _sampleMax;
        }

    protected:
        void paint() override;
        void onResize() override;

    private:
        std::deque<int> _samples;

        int _sampleMin = 0;
        int _sampleMax = 0;

        void trimSamples();
        void updateRanges();
    };

    std::array<ForecastWidget, 3> _forecastWidgets;

    GraphAxisLabelsWidget _temperatureGraphLabels;
    GraphAxisLabelsWidget _pressureGraphLabels;

    GraphWidget _temperatureGraph;
    GraphWidget _pressureGraph;
};