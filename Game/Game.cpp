#include "Game.hpp"
#include "scene_lua.hpp"

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/norm.hpp>


#include <cstdlib>

using namespace glm;
using namespace std;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

const int button_types[] = {GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_MOUSE_BUTTON_RIGHT};

//----------------------------------------------------------------------------------------
// Constructor
Game::Game() : msaa(false), enablePlayerParticle(false)
{
    moveFactor = glm::vec2(0,0);
}

//----------------------------------------------------------------------------------------
// Destructor
Game::~Game()
{
    delete cubeTexture;
    delete worldManager;

    delete shadowFrameBuffer;
    delete particle_system;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void Game::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

    initShader();
	initCamera();
    initGameWorld();
    initTexture();
	initLightSources();

    audio.playBackground();
    particle_system = new Particles();
}

void Game::initGameWorld() {
    this->worldManager = new ChunkManager();
    this->player.loadModel();
    timeOfDay = 0;
}

//----------------------------------------------------------------------------------------
void Game::initShader() {
    cube_shader = CubeShader::getShader();
    shadow_shader = ShadowShader::getShader();
    particle_shader = ParticleShader::getShader();
}

//----------------------------------------------------------------------------------------
void Game::initCamera()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
    camera.aspect = aspect;
    camera.generateProjectionMatrix();

    m_shadow_perspective = glm::ortho<float>(-30,30,-30,30,-10,200);
    //m_perspective = m_shadow_perspective;
    //m_shadow_perspective = m_perspective;
}

//----------------------------------------------------------------------------------------

void Game::initTexture() {
    shadowTexture = new Texture(1024, 768, "depth");
    waterTexture = new Texture(1024, 768, "color");

    cubeTexture = new Texture(getAssetFilePath("tileSet.png"));
    dudvTexture = new Texture(getAssetFilePath("dudv.png"));
    shadowFrameBuffer = new FrameBuffer(shadowTexture);
    waterFrameBuffer = new FrameBuffer(waterTexture);

    cube_shader->enable();
    cubeTexture->bind(cube_shader->tex_uni);
    waterTexture->bind(cube_shader->texWater_uni);
    dudvTexture->bind(cube_shader->texDUDV_uni);
    shadowTexture->bind(cube_shader->texShadow_uni);
    cube_shader->disable();
}

//----------------------------------------------------------------------------------------
void Game::initLightSources() {
    m_light.position = vec3(0.0f, 1.0f, 0.0f);
    m_light.rgbIntensity = vec3(0.8f);
}

//----------------------------------------------------------------------------------------
void Game::updateViewMatrix() {
    camera.update(player);
    m_shadow_view = glm::lookAt(m_light.position * 10, m_light.position * 9, vec3(0, 1, 0));
    // m_shadow_view = m_view;
}

LightSource Game::getSunLight() {
    struct LightSource ret;
    float Day = (timeOfDay / 1440.0f);
    // 0 Sun Rise, 0.25 Noon, 0.5 Sun Set, 0.75 MidNight, 1.0 Sun Rise
    if (Day < 0.5) { // Day
        ret.rgbIntensity = getSunLightColor(Day);
        ret.ambientIntensity = ret.rgbIntensity * 0.5;
        float angle = Day * 2 * PI;
        glm::mat4 rotate = glm::rotate(mat4(), angle, vec3(0.0f, 0.0f, 1.0f));
        // X+ is east
        ret.position = vec3(rotate * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    } else { // Night
        ret.rgbIntensity = getSunLightColor(Day);
        ret.ambientIntensity = ret.rgbIntensity * 0.4;
        float angle = (1.0 - Day) * 2 * PI;
        glm::mat4 rotate = glm::rotate(mat4(), angle, vec3(0.0f, 0.0f, 1.0f));
        // X+ is east
        ret.position = vec3(rotate * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    }
    return ret;
}

void Game::uploadCommonSceneUniforms() {
    glUniformMatrix4fv( cube_shader->P_uni, 1, GL_FALSE, value_ptr( camera.m_perspective ) );

    GLint location;
    {
        glClearColor(m_light.rgbIntensity.x, m_light.rgbIntensity.y, m_light.rgbIntensity.z, 1.0);
        glUniform3fv(cube_shader->light_position_uni, 1, value_ptr(m_light.position));
        glUniform3fv(cube_shader->light_rgbIntensity_uni, 1, value_ptr(m_light.rgbIntensity));
        //-- Set background light ambient intensity
        glUniform3fv(cube_shader->ambientIntensity_uni, 1, value_ptr(m_light.ambientIntensity));
        glUniform2fv(cube_shader->moveFactor_uni, 1, value_ptr(moveFactor));
        CHECK_GL_ERRORS;
    }

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void Game::appLogic()
{
    if (enablePlayerParticle) {
        for (int i = 0; i < 5; i++) {
            float randx = ((float)rand() / RAND_MAX) * 2 - 1;
            float randz = ((float)rand() / RAND_MAX) * 2 - 1;
            float randsize = ((float)rand() / RAND_MAX) / 2;
            float r = ((float)rand() / RAND_MAX);
            float g = ((float)rand() / RAND_MAX);
            float b = ((float)rand() / RAND_MAX);

            particle_system->addParticle(player.position, vec3(randx, 4, randz), vec4(r,g,b,1), randsize);
        }
    }

    moveFactor += vec2(0.001, 0.001);
    moveFactor.x = fmod(moveFactor.x, 1.0);
    moveFactor.y = fmod(moveFactor.y, 1.0);


    player.updatePosition(worldManager, &audio);
	// Place per frame, application logic here ...
    timeOfDay = wrap(timeOfDay + 1.0/60, 0, 1440);

    worldManager->update(player.position);

    m_light = getSunLight();

    updateViewMatrix();

    particle_system->updateParticles(camera.position);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void Game::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Application", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit (Q)" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

        if ( ImGui::SliderFloat( "Time ", &timeOfDay, 0, 1440, "%.0f")) {
        }

        ImGui::Checkbox("First Person", &camera.first_person);
        ImGui::Checkbox("Enable Particles", &enablePlayerParticle);

        if (ImGui::Checkbox("msaa", &msaa)) {
            if (msaa) {
                glEnable(GL_MULTISAMPLE);
            } else {
                glDisable(GL_MULTISAMPLE);
            }
        }

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );


		ImGui::Text( "Position %f %f %f", player.position.x, player.position.y, player.position.z);
		// ImGui::Text( "Solid Block %d", player.collideEnvironment(player.position, worldManager));
		ImGui::Text( "Solid Block 2 %d", worldManager->solidBlock(player.position));

	ImGui::End();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void Game::draw() {
    glm::mat4 VP = m_shadow_perspective * m_shadow_view;

    shadowFrameBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shadow_shader->enable();
        glEnable( GL_DEPTH_TEST );
        glDisable( GL_CULL_FACE );
        worldManager->renderShadow(VP);
        player.renderShadow(VP);

    shadow_shader->disable();
    shadowFrameBuffer->unbind();

    // Render world
    glm::mat4 biasDepthVP = getBiasMatrix(VP);

    Camera waterCamera = camera;
    float distanceToWater = 2 * (camera.position.y - WATER_LEVEL);
    waterCamera.position.y -= distanceToWater;
    waterCamera.facing.y = -waterCamera.facing.y;
    waterCamera.generateViewMatrix();

    waterFrameBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cube_shader->enable();
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_CULL_FACE );
        glEnable(GL_CLIP_DISTANCE0);
        uploadCommonSceneUniforms();
        //glCullFace( GL_FRONT );
        worldManager->render(waterCamera.m_view, biasDepthVP);
        player.render(waterCamera.m_view);
    cube_shader->disable();
    waterFrameBuffer->unbind();

    // Draw the actual
    cube_shader->enable();
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_CULL_FACE );
        glDisable(GL_CLIP_DISTANCE0);
        //glCullFace( GL_FRONT );
        worldManager->render(camera.m_view, biasDepthVP);
        player.render(camera.m_view);
    cube_shader->disable();
 




    particle_shader->enable();
        glEnable( GL_DEPTH_TEST );
        particle_system->render(camera.m_perspective * camera.m_view);
    particle_shader->disable();

	CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void Game::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool Game::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool Game::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

    vec2 new_mouse_position = vec2(xPos, yPos);
	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
        if (this->mouse_button_pressed[0]) {
            vec2 delta = 2 * PI * (new_mouse_position - mouse_position) / m_windowWidth;
            camera.updateCameraRotation(delta);
            player.updateRotation(delta.x);
        }
    }
    mouse_position = new_mouse_position;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool Game::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
    if (!ImGui::IsMouseHoveringAnyWindow()) {
        if (actions == GLFW_PRESS || actions == GLFW_RELEASE) {
            for (int i = 0; i < 3; i++) {
                if (button_types[i] == button) {
                    mouse_button_pressed[i] = (actions == GLFW_PRESS);
                    eventHandled = true;
                }
            }

            if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {

                glm::vec3 blockPos;
                bool deletedBlock = false;

                for (int i = 0; i < 2; i++) {
                    blockPos = player.position + player.facing + vec3(0, i, 0);
                    deletedBlock =  worldManager->destroyBlock(blockPos);

                    if (deletedBlock) {
                        break;
                    }
                }

                if (deletedBlock) {
                    for (int i = 0; i < 20; i++) {
                        float randx = ((float)rand() / RAND_MAX) * 2 - 1;
                        float randz = ((float)rand() / RAND_MAX) * 2 - 1;
                        float randsize = ((float)rand() / RAND_MAX) / 2;
                        float r = ((float)rand() / RAND_MAX) / 2 + 0.5f;
                        float g = ((float)rand() / RAND_MAX) / 2;
                        float b = ((float)rand() / RAND_MAX) / 4;

                        particle_system->addParticle(blockPos, vec3(randx, 4, randz), vec4(r,g,b,1), randsize);
                    }
                    audio.playRockBreak();
                }
                player.smashTime = 0.01;
            }
        }
    }

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool Game::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...
    camera.updateZoom(yOffSet);

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool Game::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initCamera();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool Game::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
        if (key == GLFW_KEY_SPACE && player.fallingTime == 0) {
            // Player up
            player.updateDelta(glm::vec3(0, 0.3, 0));
            audio.playJump();
        }

        if (key == GLFW_KEY_LEFT_SHIFT) {
            // Player up
            player.updateDelta(glm::vec3(0, -0.3, 0));
        }

        if (key == GLFW_KEY_A) {
            // Player up
            player.updateDelta(-player.right * 0.5);
            //eventHandled = true;
        }


        if (key == GLFW_KEY_D) {
            // Player up
            player.updateDelta(player.right * 0.5);
            //eventHandled = true;
        }

        if (key == GLFW_KEY_W) {
            // Player forward
            player.updateDelta(player.facing * 0.5);
        }

        if (key == GLFW_KEY_S) {
            // Player backward
            player.updateDelta(-player.facing * 0.5);
        }

        if (key == GLFW_KEY_Q) {
            glfwSetWindowShouldClose(m_window, GL_TRUE);
            eventHandled = true;
        }
	}
	// Fill in with event handling code...

	return eventHandled;
}

