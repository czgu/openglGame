#include "Objects.hpp"
#include "SceneNode.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define PI 3.141592653f

void Objects::updateRotation(float delta) {
    rotation = wrap(rotation + delta, 0, 2 * PI);
    m_rotate = glm::rotate(glm::mat4(), rotation, glm::vec3(0, 1, 0));
    facing = glm::vec3(m_rotate * glm::vec4(0, 0, 1, 1));
}

void Objects::updatePosition(glm::vec3 pos) {
    position = pos;
}

bool SolidObjects::collideEnvironment(glm::vec3& position, ChunkManager* chunkManager) {
    float halfWidth = width / 2;
    // Check 8 points
    glm::vec3 points[8];

    // Bottom
    points[0] = glm::vec3(position.x + halfWidth, position.y, position.z + halfWidth);
    points[1] = glm::vec3(position.x + halfWidth, position.y, position.z - halfWidth);
    points[2] = glm::vec3(position.x - halfWidth, position.y, position.z + halfWidth );
    points[3] = glm::vec3(position.x - halfWidth, position.y, position.z - halfWidth);

    points[4] = glm::vec3(position.x + halfWidth, position.y + height, position.z + halfWidth);
    points[5] = glm::vec3(position.x + halfWidth, position.y + height, position.z - halfWidth);
    points[6] = glm::vec3(position.x - halfWidth, position.y + height, position.z + halfWidth );
    points[7] = glm::vec3(position.x - halfWidth, position.y + height, position.z - halfWidth);

    for (int i = 0; i < 7; i++) {
        if (chunkManager->solidBlock(points[i])) {
            return true;
        }
    }
    return false;
}

Player::Player() {
    position = glm::vec3(0, 12, 0);
    rotation = 0;
    facing = glm::vec3(0, 0, 1);
    right = glm::vec3(-1, 0, 0);

    width = 1;
    height = 1.8;

    fallingTime = 0;
    smashTime = 0;

    playerModel = NULL;
    leftThigh = rightThigh = leftArm = rightArm = NULL;
}

void Player::loadModel() {
    playerModel = new MeshModel(getAssetFilePath("puppet.lua"));

    SceneNode* root = playerModel->getRoot();
    loadJoints(root);
}

void Player::loadJoints(SceneNode* node) {
    if (node->m_name == "shoulderJoint") {
        if (rightArm == NULL) {
            rightArm = static_cast<JointNode *>(node);
        } else {
            leftArm = static_cast<JointNode *>(node);
        }
    } else if (node->m_name == "thighJoint") {
        if (rightThigh == NULL) {
            rightThigh = static_cast<JointNode *>(node);
        } else {
            leftThigh = static_cast<JointNode *>(node);
        }
    }

    for (SceneNode* child : node->children) {
        loadJoints(child);
    }
}

void Player::updateRotation(float delta) {
    this->Objects::updateRotation(delta);
    right = glm::cross(facing, glm::vec3(0, 1, 0));
}

void Player::updateDelta(glm::vec3 d) {
    if (fallingTime > 0) {
        d.y = 0;
    }

    delta += d;
}

void Player::updatePosition(ChunkManager* chunkManager, Audio* audio) {
    glm::vec3 positionBefore = position;

    delta.y -= 1.0/60.0;
    for (int i = 0; i < 3; i++) {
        glm::vec3 newpos = position;
        if (i == 0) {
            newpos.x += delta.x;
        } else if (i == 1) {
            newpos.y += delta.y;
        } else {
            newpos.z += delta.z;
        }

        if (collideEnvironment(newpos, chunkManager)) {

        } else {
            position = newpos;
        }
    }

    if (positionBefore.y == position.y) {
        fallingTime = 0;
        delta.y = 0;
    } else {
        fallingTime += 1;
    }

    if (positionBefore.x != position.x || positionBefore.z != position.z) {
        if (walkingTime < 0.2 || (walkingTime >= 1.0 && walkingTime < 1.2)) {
            audio->playWalk();
        }
        walkingTime += 1.0/5.0;
    }

    if (smashTime > 0) {
        smashTime += 1.0/5.0;
        if (smashTime >= 2) {
            smashTime = 0;
        }
    }

    animate();
    delta = glm::vec3(0, delta.y, 0);
}

void Player::animate() {
    walkingTime = fmod(walkingTime, 2);
    float frame = walkingTime > 1 ? 2 - walkingTime : walkingTime;


    float leftLegAngle = lerp(-25, 30, frame);
    float rightLegAngle = lerp(-25, 30, 1 - frame);
    float leftArmAngle = lerp(330, 360, 1- frame);
    float rightArmAngle = lerp(330, 360, frame);

    if (smashTime > 0) {
        frame = smashTime > 1 ? 2 - smashTime : smashTime;
        rightArmAngle = lerp(360, 300, frame);
    }

    leftArm->set_x_rotate(leftArmAngle);
    rightArm->set_x_rotate(rightArmAngle);
    leftThigh->set_x_rotate(leftLegAngle);
    rightThigh->set_x_rotate(rightLegAngle);
}

void Player::render(glm::mat4 view) {
    if (playerModel == NULL) {
        return;
    }
    glm::mat4 translate = glm::translate(glm::mat4(), position);
    playerModel->render(view * translate * m_rotate, false);
}

void Player::renderShadow(glm::mat4 view) {
    if (playerModel == NULL) {
        return;
    }
    glm::mat4 translate = glm::translate(glm::mat4(), position);
    playerModel->render(view * translate * m_rotate, true);
}

Camera::Camera() {
    near_plane = 0.1f;
    far_plane = 100.0f;
    fov = glm::radians(60.0f);
    aspect = 1024.0/768.0;

    position = glm::vec3(0, 12, 0);
    camera_rotation = glm::vec2(0, 0);
    facing = glm::vec3(0, 0, 1);

    first_person = false;
    distanceFromPlayer = 10;

    generateProjectionMatrix();
}

void Camera::update(Player& player) {
    if (first_person) {
        this->position = player.position + glm::vec3(0, 1.8f, 0.0f) + 0.1f * player.facing; // player height
    } else {
        float yaw = player.rotation + PI;
        float pitch = camera_rotation.y;

        float height = distanceFromPlayer * glm::sin(pitch);
        float width = distanceFromPlayer * glm::cos(pitch);

        glm::mat4 rotate = glm::rotate(glm::mat4(), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        this->position = player.position + glm::vec3(0, 1.8f, 0.0f) + glm::vec3(rotate * glm::vec4(0, 0, width, 1)) + glm::vec3(0, height, 0);

        this->facing = player.position - this->position;
    }

    generateViewMatrix();
}

void Camera::generateViewMatrix() {
    m_view = glm::lookAt(position, facing + position, glm::vec3(0,1,0));
}

void Camera::generateProjectionMatrix() {
    m_perspective = glm::perspective(fov, aspect, near_plane, far_plane);
}

void Camera::updateCameraRotation(glm::vec2 rotation) {
    camera_rotation += rotation;
    if (!first_person) {
        camera_rotation.y = glm::clamp(camera_rotation.y, 0.0f, (float)PI / 2.2f);
        return;
    }

    camera_rotation.x = (float)wrap(camera_rotation.x, 0, 2 * PI);
    camera_rotation.y = glm::clamp(camera_rotation.y, (float)-PI/3.0f, (float)PI / 3.0f);

    glm::mat4 rotate = glm::rotate(glm::mat4(), camera_rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
    rotate = glm::rotate(rotate, camera_rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));

    facing = glm::vec3(rotate * glm::vec4(0, 0, 1, 1));
}

void Camera::updateZoom(float delta) {
    distanceFromPlayer = glm::clamp(distanceFromPlayer + delta, 3.0f, 15.0f);
}

