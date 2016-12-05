#include "Perlin.hpp"
#include <cmath>
#include <iostream>
#include <glm/gtc/noise.hpp>

double Perlin::OctavePerlin(glm::vec2 v, int octaves, double persistence) {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;

    for (int i = 0; i < octaves; i++) {
        total += perlin(v) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2;
    }
    return total/maxValue;
}

Perlin* Perlin::instance() {
    if (_instance == 0) {
        _instance = new Perlin();
    }

    return _instance;
}

void Perlin::setRepeat(int repeat) {
    this->repeat = repeat;
}

double Perlin::perlin(glm::vec2 v) {
    double ret = glm::perlin(v);
    ret = (ret + 0.707) / 1.414;
    if (ret < 0) return 0;
    if (ret > 1) return 1;
    return ret;
}

Perlin::Perlin() {
    repeat = 0;
}

Perlin::~Perlin() {
}

Perlin* Perlin::_instance = 0;
