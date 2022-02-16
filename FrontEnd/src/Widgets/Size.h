#pragma once

class Size
{
public:
    Size() = default;
    Size(int w, int h);
    Size(const Size&) = default;
    Size(Size&&) = default;

    [[nodiscard]] bool isNull() const;

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

    Size& operator=(const Size&) = default;
    Size& operator=(Size&&) = default;
    [[nodiscard]] bool operator==(const Size& p) const;
    [[nodiscard]] bool operator!=(const Size& p) const;

private:
    int _w = 0;
    int _h = 0;
};