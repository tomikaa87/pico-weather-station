#pragma once

class Point
{
public:
    Point() = default;
    Point(const int x, const int y);
    Point(const Point&) = default;
    Point(Point&&) = default;

    [[nodiscard]] int x() const;
    [[nodiscard]] int y() const;

    Point& operator=(const Point&) = default;
    Point& operator=(Point&&) = default;
    [[nodiscard]] bool operator==(const Point& p) const;
    [[nodiscard]] bool operator!=(const Point& p) const;

    friend Point operator+(const Point& p1, const Point& p2);

private:
    int _x = 0;
    int _y = 0;
};