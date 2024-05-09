#ifndef CG_SOLAR_SYSTEM_COLOR_H
#define CG_SOLAR_SYSTEM_COLOR_H


struct Color
{
    float r, g, b, a;
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    bool operator==(const Color &other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
};


#endif // CG_SOLAR_SYSTEM_COLOR_H
