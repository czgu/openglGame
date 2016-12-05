#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// Math Functions
glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t);
double lerp(double a, double b, double t);
double wrap(double val, double low, double high);

float easeIn(float x);
float easeOut(float x);

// Utility Functions
glm::vec3 getSunLightColor(float time);
std::string getAssetFilePath(std::string fileName);

glm::mat4 getBiasMatrix(glm::mat4& m);

// Calculation class to get bounding box for shadow
// Original code from ThinMatrix: https://www.youtube.com/watch?v=o6zDfDkOFIc&index=38&list=PLRIWtICgwaX0u7Rf9zkZhLoLuZVfUksDP&t=974s
// TODO: implement this
class ShadowBox {
public:
private:
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;

    float farHeight, farWidth;
    float nearWidth, nearHeight;

    glm::mat4 viewMatrix;

};

class Audio {
public:
    Audio();
    ~Audio();

    void playBackground();
    void playJump();
    void playWalk();
    void playRockBreak();
private:
    Mix_Music* backgroundMusic;
    Mix_Chunk* jumpSound;
    Mix_Chunk* walkSound;
    Mix_Chunk* rockSound;
};
