#include "GLUtils.hpp"

#include <lodepng/lodepng.h>
#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Utils.hpp"

#include "scene_lua.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

CubeShader* CubeShader::getShader() {
    static CubeShader cube_shader;
    return &cube_shader;
}

ShadowShader* ShadowShader::getShader() {
    static ShadowShader shadow_shader;
    return &shadow_shader;
}

ParticleShader* ParticleShader::getShader() {
    static ParticleShader particle_shader;
    return &particle_shader;
}

Shader::Shader(std::string vertex_shader, std::string frag_shader) {
	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( vertex_shader ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( frag_shader ).c_str() );
	m_shader.link();
}

void Shader::enable() const {
    m_shader.enable();
}

void Shader::disable() const {
    m_shader.disable();
}

CubeShader::CubeShader() : Shader("VertexShader.vs", "FragmentShader.fs") {
    P_uni = m_shader.getUniformLocation("P");
    VM_uni = m_shader.getUniformLocation("VM");
    Normal_Matrix_uni = m_shader.getUniformLocation("NormalMatrix");
    depthBiasMVP_uni = m_shader.getUniformLocation("depthBiasMVP");
    tex_uni = m_shader.getUniformLocation("tex");
    texShadow_uni = m_shader.getUniformLocation("texShadow");
    ambientIntensity_uni = m_shader.getUniformLocation("ambientIntensity");
    light_position_uni = m_shader.getUniformLocation("light.position");
    light_rgbIntensity_uni = m_shader.getUniformLocation("light.rgbIntensity");

    positionAttrib = m_shader.getAttribLocation("position");
    normalAttrib = m_shader.getAttribLocation("normal");
}

ShadowShader::ShadowShader() : Shader("shadow_VertexShader.vs", "shadow_FragmentShader.fs") {
    MVP_uni = m_shader.getUniformLocation("MVP");

    positionAttrib = m_shader.getAttribLocation("position");
}

ParticleShader::ParticleShader() : Shader("particle_VertexShader.vs", "particle_FragmentShader.fs") {
    MVP_uni = m_shader.getUniformLocation("MVP");

    particle_positionAttrib = m_shader.getAttribLocation("particle_position");
    vertex_positionAttrib = m_shader.getAttribLocation("vertex_position");
    colorAttrib = m_shader.getAttribLocation("color");
}

int Texture::textureCounter = 0;

Texture::Texture(std::string imageUrl) {
    loaded = false;

    std::vector<unsigned char> image;
    unsigned error = lodepng::decode(image, width, height, imageUrl);

    if(error != 0) {
        std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
        return;
    }

    glGenTextures(1, &tex);
    textureId = Texture::textureCounter;

    glActiveTexture(GL_TEXTURE0 + textureId);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    size_t u = 1; while(u < width) u *= 2;
    size_t v = 1; while(v < height) v *= 2;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, u, v, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    Texture::textureCounter += 1;
    loaded = true;

    textureType = "color";

	CHECK_GL_ERRORS;
}

Texture::Texture(int width, int height, std::string type) {
    loaded = false;

    glGenTextures(1, &tex);
    textureId = Texture::textureCounter;

    glActiveTexture(GL_TEXTURE0 + textureId);
    glBindTexture(GL_TEXTURE_2D, tex);

    textureType = type;
    if (type == "depth") {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    } else { // color
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, 0);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    Texture::textureCounter += 1;
    loaded = true;
}

bool Texture::bind(GLint uniform) {
    if (loaded) {
        glActiveTexture(GL_TEXTURE0 + textureId);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(uniform, textureId);
	    CHECK_GL_ERRORS;
    }
    return loaded;
}

Texture::~Texture() {
    if (loaded) {
        glDeleteTextures(1, &tex);
    }
}

int Texture::getWidth() {
    return width;
}

int Texture::getHeight() {
    return height;
}

GLuint Texture::getTex() {
    return tex;
}


FrameBuffer::FrameBuffer(Texture* texture): texture(texture) {
    glGenFramebuffers(1, &frameBufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->getTex(), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "FBO creation failed" << std::endl;
        loaded = false;
    } else {
        loaded = true;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
    if (loaded) {
        glDeleteFramebuffers(1, &frameBufferName);
        delete texture;
    }
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
}

void FrameBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Mesh* Mesh::getMeshRender() {
    static Mesh mesh;
    return &mesh;
}

Mesh::Mesh() {
    CubeShader* cube_shader = CubeShader::getShader();
    ShadowShader* shadow_shader = ShadowShader::getShader();

    std::unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
        getAssetFilePath("cube.obj"),
        getAssetFilePath("sphere.obj"),
        getAssetFilePath("suzanne.obj")
    });

    // Acquire the BatchInfoMap from the MeshConsolidator.
    meshConsolidator->getBatchInfoMap(m_batchInfoMap);

    const int numVertex = meshConsolidator->getNumVertexPositionBytes() / sizeof(float);
    const float* vertexPositions = meshConsolidator->getVertexPositionDataPtr();
    float* vertexPositionsVec4 = new float[(numVertex/3) * 4];
    int x = 0;
    for (int i = 0; i < numVertex/3; i++) {
        for (int j = 0; j < 3; j++) {
            vertexPositionsVec4[x++] = vertexPositions[i * 3 + j];
        }
        vertexPositionsVec4[x++] = 120;
    }

    // Generate VBO
    glGenBuffers(1, &m_vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_position);

    glBufferData(GL_ARRAY_BUFFER, x * sizeof(float), vertexPositionsVec4, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_normal);

    glBufferData(GL_ARRAY_BUFFER, meshConsolidator->getNumVertexNormalBytes(),
        meshConsolidator->getVertexNormalDataPtr(), GL_STATIC_DRAW);

    delete [] vertexPositionsVec4;

	glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(cube_shader->positionAttrib);
    glEnableVertexAttribArray(cube_shader->normalAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_position);
	glVertexAttribPointer(cube_shader->positionAttrib, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_normal);
	glVertexAttribPointer(cube_shader->normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenVertexArrays( 1, &m_vao_shadow );
    glBindVertexArray( m_vao_shadow );
    glEnableVertexAttribArray( shadow_shader->positionAttrib );

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_position);
    glVertexAttribPointer( shadow_shader->positionAttrib, 4, GL_FLOAT, GL_FALSE, 0, nullptr );

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vbo_position);
    glDeleteBuffers(1, &m_vbo_position);
    glDeleteVertexArrays(1, &m_vao);

}

void Mesh::bindVAO(bool on, bool shadow) {
    if (on) {
        if (shadow) {
            glBindVertexArray(m_vao_shadow);
        } else {
            glBindVertexArray(m_vao);
        }
    } else {
	    glBindVertexArray(0);
    }
}

void Mesh::render(std::string meshId) {
    BatchInfo batchInfo = m_batchInfoMap[meshId];
    glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
}

MeshModel::MeshModel (std::string file) : meshRender(Mesh::getMeshRender()) {
    m_rootNode = std::shared_ptr<SceneNode>(import_lua(file));
	if (!m_rootNode) {
		std::cerr << "Could not open " << file << std::endl;
	}
}

void MeshModel::renderNode(const SceneNode* node, glm::mat4 trans, const SceneNode* parent, bool shadow) {
    trans = trans * node->trans;

    if (node->m_nodeType == NodeType::SceneNode) {
    }
    else if (node->m_nodeType == NodeType::GeometryNode) {
        const GeometryNode* geometryNode = static_cast<const GeometryNode *>(node);
        if (shadow) {
            glUniformMatrix4fv(ShadowShader::getShader()->MVP_uni, 1, GL_FALSE, value_ptr(trans));
        } else {
            glUniformMatrix4fv(CubeShader::getShader()->VM_uni, 1, GL_FALSE, value_ptr(trans));
        }
		//-- Now render the mesh:
        meshRender->render(geometryNode->meshId);
    } else { // JointNode
        const JointNode* jointNode = static_cast<const JointNode *>(node);
        trans = trans * jointNode->get_x_rotate() * jointNode->get_y_rotate();
    }

    for (const SceneNode* children : node->children) {
        renderNode(children, trans, node, shadow);
    }
}
void MeshModel::render(glm::mat4 view, bool shadow) {
    meshRender->bindVAO(true, shadow);
    this->renderNode(&*m_rootNode, view, NULL, shadow);
    meshRender->bindVAO(false, shadow);
}

SceneNode* MeshModel::getRoot() {
    return &*m_rootNode;
}

Particles::Particles() {
    static const float vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &pbo);
    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), NULL, GL_STREAM_DRAW);

    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), NULL, GL_STREAM_DRAW);

    ParticleShader* particle_shader = ParticleShader::getShader();

    glGenVertexArrays( 1, &m_vao );
    glBindVertexArray( m_vao );

    glEnableVertexAttribArray( particle_shader->vertex_positionAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(particle_shader->vertex_positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray( particle_shader->particle_positionAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    glVertexAttribPointer(particle_shader->particle_positionAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glEnableVertexAttribArray( particle_shader->colorAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glVertexAttribPointer(particle_shader->colorAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);

    CHECK_GL_ERRORS;
    //
    num_particles = 0;
    last_particle = 0;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        particleArr[i].life = -1;
    }
}

void Particles::updateBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, num_particles * 4 * sizeof(float), position_data);

    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, num_particles * 4 * sizeof(float), color_data);
    CHECK_GL_ERRORS;
}

void Particles::render(glm::mat4 view) {
    ParticleShader* particle_shader = ParticleShader::getShader();

    glBindVertexArray( m_vao );
    glVertexAttribDivisor(particle_shader->vertex_positionAttrib, 0);
    glVertexAttribDivisor(particle_shader->particle_positionAttrib, 1);
    glVertexAttribDivisor(particle_shader->colorAttrib, 1);

    glUniformMatrix4fv( particle_shader->MVP_uni, 1, GL_FALSE, value_ptr( view ) );
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, num_particles);

    CHECK_GL_ERRORS;
}

int Particles::findFreeParticle(){
    for(int i = last_particle; i< MAX_PARTICLES; i++){
        if (particleArr[i].life < 0){
            last_particle = i;
            return i;
        }
    }

    for(int i = 0; i< last_particle; i++){
        if (particleArr[i].life < 0){
            last_particle = i;
            return i;
        }
    }
    return 0;
}

void Particles::updateParticles(glm::vec3 cameraPos) {
    static const float delta = 1.0/60.0; // Represent delta time, TODO:use actual time
    num_particles = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle& p = particleArr[i];
        if (p.life > 0.0f) {
            p.life -= delta;
            if (p.life > 0) {
                // simulate falling
                p.velocity += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
                p.position += p.velocity * (float)delta;

                p.cameraDist = glm::length(p.position - cameraPos);

                position_data[4 * num_particles + 0] = p.position.x;
                position_data[4 * num_particles + 1] = p.position.y;
                position_data[4 * num_particles + 2] = p.position.z;
                position_data[4 * num_particles + 3] = p.size;

                color_data[4 * num_particles + 0] = p.color.r;
                color_data[4 * num_particles + 1] = p.color.g;
                color_data[4 * num_particles + 2] = p.color.b;
                color_data[4 * num_particles + 3] = p.color.a;

                num_particles++;
            } else {
                p.cameraDist = -1;
            }
        }
    }
    updateBuffer();
}

void Particles::addParticle(glm::vec3 loc, glm::vec3 v, glm::vec4 c, float size) {
    int slot = findFreeParticle();

    particleArr[slot].life = 2.0;
    particleArr[slot].position = loc;
    particleArr[slot].velocity = v;
    particleArr[slot].color = c;
    particleArr[slot].size = size;
}

