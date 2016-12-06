#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "Chunk.hpp"
#include "ChunkManager.hpp"
#include "GLUtils.hpp"
#include "Objects.hpp"
#include "Utils.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>


#define NUM_JOINT 32

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
    glm::vec3 ambientIntensity;
};

class Game : public CS488Window {
public:
	Game();
	virtual ~Game();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
    void initShader();
    void initGameWorld();
	void initCamera();
    void initTexture();
    void initLightSources();

    // -- Update methods
    void updateViewMatrix();
    void uploadCommonSceneUniforms();
    LightSource getSunLight();

    // -- Open GL variables
    CubeShader* cube_shader;
    ShadowShader* shadow_shader;
    ParticleShader* particle_shader;

    FrameBuffer* shadowFrameBuffer;
    FrameBuffer* waterFrameBuffer;

    // -- Render variables
    glm::mat4 m_shadow_perspective;
    glm::mat4 m_shadow_view;

	LightSource m_light;
    Particles* particle_system;
    bool enablePlayerParticle;

    glm::vec2 moveFactor;

    // Control variables
    bool mouse_button_pressed[3];
    glm::vec2 mouse_position;

    ChunkManager* worldManager;
    Texture* cubeTexture;
    Texture* dudvTexture;
    Texture* shadowTexture;
    Texture* waterTexture;

    // Game logic variables
    float timeOfDay;

    // Control Variables
    Player player;
    Camera camera;

    Audio audio;

    bool msaa;
};
