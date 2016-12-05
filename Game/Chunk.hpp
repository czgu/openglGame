#ifndef _CHUNK_H_
#define _CHUNK_H_

#include "cs488-framework/OpenGLImport.hpp"
#include <glm/glm.hpp>

#include "Perlin.hpp"
#include "GLUtils.hpp"

#define CHUNK_SIZE 16
#define WATER_LEVEL 3

enum BlockType {
    EMPTY = 0,
    GRASS,
    GRASS_BLADE,
    DIRT,
    SAND,
    WATER,
    ROCK,
    TREE,
    SNOW,
    NUM_BLOCKS
};

bool transparentBlock(BlockType block);
unsigned int getBlockFace(BlockType type, unsigned face);

class Chunk {
public:
    Chunk(glm::vec3 position);
    ~Chunk();
    BlockType getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, BlockType type);
    void render(glm::mat4& view, glm::mat4& depth);
    void renderShadow(glm::mat4& VP);
    glm::vec3 getPosition();

    void createTerrain(Perlin* perlin);

private:
    void deleteGraphicsMemory();
    void updateBlock();
    void setCubeVertex(float* verts ,int& i, float x, float y, float z, int type, int face);
    bool requireUpdate;
    bool surrounded(int x, int y, int z);
    glm::vec3 getFaceNormal(int face);

    // Open Gl Variables
    unsigned int numVertices;
    unsigned int numCubeVertices;
    GLuint m_vbo;
    GLuint m_vao_cube;
    GLuint m_vao_shadow;

    glm::vec3 m_position;
    glm::mat4 m_model;

    BlockType blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    CubeShader* cube_shader;
    ShadowShader* shadow_shader;
};

#endif
