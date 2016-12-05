#pragma once

#include <glm/glm.hpp>

/*
 * Reference: http://flafla2.github.io/2014/08/09/perlinnoise.html
 */

class Perlin {
public:
    double OctavePerlin(glm::vec2 v, int octaves, double persistence);
    static Perlin* instance();
    void setRepeat(int repeat);
private:
    Perlin();
    ~Perlin();
    static Perlin* _instance;
    int repeat;

    double perlin(glm::vec2 v);
};
