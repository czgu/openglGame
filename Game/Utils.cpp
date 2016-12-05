#include "Utils.hpp"
#include <iostream>

std::string getAssetFilePath(std::string fileName) {
    return "Assets/" + fileName;
}

glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t) {
    return (1-t)*a + t*b;
}

double lerp(double a, double b, double t) {
    return (1-t)*a + t*b;
}

double wrap(double val, double low, double high) {
    double range = high - low;
    while (val < low) {
        val += range;
    }
    while (val > high) {
        val -= range;
    }
    return val;
}

// From: http://www.joshondesign.com/2013/03/01/improvedEasingEquations
float easeIn(float x) {
    return glm::pow(x, 5);
}
float easeOut(float x) {
    return 1 - easeIn(1-x);
}

static glm::vec3 sunColors[4] = {
    glm::vec3(253.0/255.0, 125.0/255.0, 1.0/255.0), // Sun Rise
    glm::vec3(235.0/255.0, 235.0/255.0, 235.0/255.0), // Noon
    glm::vec3(253.0/255.0, 125.0/255.0, 1.0/255.0), // Sun Set
    glm::vec3(30.0/255.0, 30.0/255.0, 30.0/255.0) // Moon
};

glm::vec3 getSunLightColor(float time) {
    if (time < 0.25) {
        float t = (time) / 0.25;
        t = easeOut(t);
        return lerp(sunColors[0], sunColors[1], t);
    } else if (time < 0.5) {
        float t = (time - 0.25) / 0.25;
        t = easeIn(t);
        return lerp(sunColors[1], sunColors[2], t);
    } else if (time < 0.75) {
        float t = (time - 0.5) / 0.25;
        t = easeOut(t);
        return lerp(sunColors[2], sunColors[3], t);
    }
    float t = (time - 0.75) / 0.25;
    t = easeIn(t);
    return lerp(sunColors[3], sunColors[0], t);
}


glm::mat4 getBiasMatrix(glm::mat4& m) {
    static glm::mat4 biasMatrix(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0
    );
    return biasMatrix * m;
}

Audio::Audio() {
    static bool initializeMixer = false;
    if (initializeMixer == false) {
        Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC | MIX_INIT_FLUIDSYNTH | MIX_INIT_MOD | MIX_INIT_MODPLUG);

        int audio_rate = 44100;
        Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
        int audio_channels = 2;
        int audio_buffers = 512;

        if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
            std::cout << "Cannot open audio" << std::endl;
            exit(1);
        }

        int result = Mix_AllocateChannels(4);
        if( result < 0 ) {
            std::cout << "Unable to allocate mixing channels: " << SDL_GetError() << std::endl;
        }

        Mix_Volume(-1, MIX_MAX_VOLUME);
    }

    backgroundMusic = Mix_LoadMUS("Assets/hyrule-field.mid");
    if (backgroundMusic == NULL) {
        std::cout << "cannot load music" << std::endl;
    } else {
        Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    }

    jumpSound = Mix_LoadWAV("Assets/jumping.wav");
    if (jumpSound == NULL) {
        std::cout << "cannot load sound" << std::endl;
    } else {
        Mix_VolumeChunk(jumpSound, MIX_MAX_VOLUME);
    }

    walkSound = Mix_LoadWAV("Assets/walking.wav");
    if (walkSound == NULL) {
        std::cout << "cannot load sound" << std::endl;
    }

    rockSound = Mix_LoadWAV("Assets/rock_shatter.wav");
    if (rockSound == NULL) {
        std::cout << "cannot load sound" << std::endl;
    }
}

void Audio::playBackground() {
    if (backgroundMusic != NULL) {
        Mix_PlayMusic(backgroundMusic, -1); //loop forever
    }
}

void Audio::playJump() {
    if (jumpSound != NULL) {
        Mix_PlayChannel( -1, jumpSound, 0 );
    }
}

void Audio::playWalk() {
    if (walkSound != NULL) {
        Mix_PlayChannel( -1, walkSound, 0 );
    }
}

void Audio::playRockBreak() {
    if (rockSound != NULL) {
        Mix_PlayChannel( -1, rockSound, 0 );
    }
}

Audio::~Audio() {
    if (jumpSound != NULL) {
        Mix_FreeChunk( jumpSound );
    }
    if (walkSound != NULL) {
        Mix_FreeChunk( walkSound );
    }
    if (rockSound != NULL) {
        Mix_FreeChunk( rockSound );
    }
}

