#pragma once

#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"


#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(std::string vertex_shader, std::string frag_shader);
    void enable() const;
    void disable() const;
protected:
    ShaderProgram m_shader;
};

class CubeShader : public Shader {
public:
    static CubeShader* getShader();

    // Uniforms
    GLint P_uni;
    GLint VM_uni;
    GLint Normal_Matrix_uni;
    GLint depthBiasMVP_uni;
    GLint tex_uni;
    GLint texShadow_uni;
    GLint ambientIntensity_uni;
    GLint light_position_uni;
    GLint light_rgbIntensity_uni;

    // Attributes
    GLint positionAttrib;
    GLint normalAttrib;
private:
    CubeShader();
};

class ShadowShader : public Shader {
public:
    static ShadowShader* getShader();

    // Uniforms
    GLint MVP_uni;

    // Attributes
    GLint positionAttrib;
private:
    ShadowShader();
};

class ParticleShader : public Shader {
public:
    static ParticleShader* getShader();

    // Uniforms
    GLint MVP_uni;

    // Attributes
    GLint particle_positionAttrib;
    GLint vertex_positionAttrib;
    GLint colorAttrib;
private:
    ParticleShader();
};


class Texture {
public:
    Texture(std::string imageUrl);
    Texture(int width, int height, std::string type);
    bool bind(GLint uniform);
    ~Texture();
    int getWidth();
    int getHeight();
    GLuint getTex();
private:
    GLuint tex;
    unsigned width;
    unsigned height;
    bool loaded;

    int textureId;

    static int textureCounter;
};

class FrameBuffer {
public:
    FrameBuffer(Texture* texture);
    ~FrameBuffer();

    void bind();
    void unbind();

    Texture* texture;
private:
    bool loaded;

    GLuint frameBufferName;
};

class Mesh {
public:
    static Mesh* getMeshRender();
    void render(std::string meshId);
    void bindVAO(bool on, bool shadow);
private:
    Mesh();
    ~Mesh();
    unsigned int numVertices;
    GLuint m_vbo_position;
    GLuint m_vbo_normal;
    BatchInfoMap m_batchInfoMap;
    GLuint m_vao;
    GLuint m_vao_shadow;
};

class MeshModel {
public:
    MeshModel(std::string file);
    void render(glm::mat4 view, bool shadow);
    SceneNode* getRoot();
private:
    void renderNode(const SceneNode* node, glm::mat4 trans, const SceneNode* parent, bool shadow);
    Mesh* meshRender;
    std::shared_ptr<SceneNode> m_rootNode;
};

struct Particle {
    glm::vec3 position, velocity;
    glm::vec4 color;
    float size;
    float cameraDist;
    float life; // Remaining life of the particle. if < 0 : dead and unused.
};

// Code From: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
#define MAX_PARTICLES 1000
class Particles {
public:
    Particles();

    void updateParticles(glm::vec3 cameraPos);
    void updateBuffer();
    void render(glm::mat4 view);
    void addParticle(glm::vec3 loc, glm::vec3 v, glm::vec4 c, float size);


    int findFreeParticle();
private:

    Particle particleArr[MAX_PARTICLES];
    int num_particles;
    int last_particle;

    float position_data[4 * MAX_PARTICLES];
    float color_data[4 * MAX_PARTICLES];

    GLuint vbo; // vertex buffer
    GLuint pbo; // position buffer
    GLuint cbo; // color buffer

    GLuint m_vao;
};
