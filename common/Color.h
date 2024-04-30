#ifndef CG_SOLAR_SYSTEM_COLOR_H
#define CG_SOLAR_SYSTEM_COLOR_H


struct Color
{
    float r, g, b, a;
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};


#endif // CG_SOLAR_SYSTEM_COLOR_H
