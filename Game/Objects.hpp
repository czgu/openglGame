#pragma once

#include <glm/glm.hpp>
#include "GLUtils.hpp"
#include "Utils.hpp"
#include "ChunkManager.hpp"
#include "SceneNode.hpp"
#include "JointNode.hpp"

struct Objects {
    glm::vec3 position;
    float rotation; // [0, 2PI]
    glm::vec3 facing;
    glm::mat4 m_rotate;

    virtual void updateRotation(float delta);
    virtual void updatePosition(glm::vec3 pos);
};

struct SolidObjects: public Objects {
    float width;
    float height;
    virtual bool collideEnvironment(glm::vec3& position, ChunkManager* chunkManager);
};

struct Player : public SolidObjects {
    glm::vec3 delta;
    glm::vec3 right;
    float fallingTime;
    float walkingTime;
    float smashTime;

    MeshModel* playerModel;
    JointNode* leftThigh, *rightThigh;
    JointNode* leftArm, *rightArm;

    Player();

    void loadModel();
    void loadJoints(SceneNode* node);

    void render(glm::mat4 view);
    void renderShadow(glm::mat4 view);

    virtual void updateRotation(float delta) override;
    virtual void updatePosition(ChunkManager* chunkManager, Audio* audio);
    void updateDelta(glm::vec3 d);

    void animate();
};

struct Enemy : public Objects {

};

struct Camera : public Objects {
    glm::vec2 camera_rotation;

    float near_plane, far_plane;
    float fov;
    float aspect;

    glm::mat4 m_perspective;
    glm::mat4 m_view;

    bool first_person;
    float distanceFromPlayer;

    Camera();
    void update(Player& player);
    void updateZoom(float delta);

    void updateCameraRotation(glm::vec2 rotation);
    void generateViewMatrix();
    void generateProjectionMatrix();
};


