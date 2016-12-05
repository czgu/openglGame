#include "Chunk.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Chunk::Chunk(glm::vec3 position) :
    m_position(position) {

    numVertices = 0;
    requireUpdate = true;

    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                blocks[i][j][k] = BlockType::EMPTY;
            }
        }
    }

    m_model = glm::translate(glm::mat4(), m_position);

    cube_shader = CubeShader::getShader();
    shadow_shader = ShadowShader::getShader();
}

Chunk::~Chunk() {
    deleteGraphicsMemory();
}

void Chunk::deleteGraphicsMemory() {
    if (numVertices > 0) {
        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao_cube);
        glDeleteVertexArrays(1, &m_vao_shadow);
    }
}

BlockType Chunk::getBlock(int x, int y, int z) {
    return blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    blocks[x][y][z] = type;
    requireUpdate = true;
}

void Chunk::renderShadow(glm::mat4& VP) {
    if (requireUpdate) {
        updateBlock();
    }

    if (numVertices == 0) {
        return;
    }
    glm::mat4 MVP = VP * m_model;
    glUniformMatrix4fv( shadow_shader->MVP_uni, 1, GL_FALSE, value_ptr( MVP ) );

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindVertexArray(m_vao_shadow);
    glDrawArrays(GL_TRIANGLES, 0, numCubeVertices);

    CHECK_GL_ERRORS;
}

void Chunk::render(glm::mat4& view, glm::mat4& depth) {
    if (requireUpdate) {
        updateBlock();
    }

    if (numVertices == 0) {
        return;
    }


    glm::mat4 VM = view * m_model;
    glm::mat3 N = glm::mat3(transpose(inverse(VM)));
    glm::mat4 depthMVP = depth * m_model;

    glUniformMatrix4fv( cube_shader->VM_uni, 1, GL_FALSE, value_ptr( VM ) );
    glUniformMatrix3fv( cube_shader->Normal_Matrix_uni, 1, GL_FALSE, value_ptr( N ) );
    glUniformMatrix4fv( cube_shader->depthBiasMVP_uni, 1, GL_FALSE, value_ptr( depthMVP ) );

    glBindVertexArray(m_vao_cube);
    glDrawArrays(GL_TRIANGLES, 0, numCubeVertices);

    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, numCubeVertices, numVertices);
    glEnable(GL_CULL_FACE);

    CHECK_GL_ERRORS;
}

void Chunk::setCubeVertex(float* verts ,int& i, float x, float y, float z, int type, int face) {
    // Position
    verts[i++] = x;
    verts[i++] = y;
    verts[i++] = z;
    verts[i++] = type;
    // Normal
    glm::vec3 faceNormal = getFaceNormal(face);
    verts[i++] = faceNormal.x;
    verts[i++] = faceNormal.y;
    verts[i++] = faceNormal.z;
}

void Chunk::updateBlock() {
    requireUpdate = false;
    // remove previous allocated memory if there is any
    deleteGraphicsMemory();

    size_t vertex_size = 7; // each vertex takes 5 floats
    size_t vertex_per_cube = 6 * 6;
    size_t numCubes = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    size_t verts_sz = vertex_size * vertex_per_cube * numCubes;
    float * verts = new float [verts_sz];

    bool skipCheck[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    for (int k = 0; k < CHUNK_SIZE; k++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int i = 0; i < CHUNK_SIZE; i++) {
                if (blocks[i][j][k] == BlockType::GRASS_BLADE) {
                    if (blocks[i][j-1][k] != BlockType::GRASS) {
                        blocks[i][j][k] = BlockType::EMPTY;
                    }
                    skipCheck[i][j][k] = true;
                } else if (blocks[i][j][k] == BlockType::EMPTY || surrounded(i, j, k)) {
                    skipCheck[i][j][k] = true;
                } else {
                    skipCheck[i][j][k] = false;
                }
            }
        }
    }

    numVertices = 0;
    numCubeVertices = 0;
    int x = 0;

    int face = 0;
    for (int k = 0; k < CHUNK_SIZE; k++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            bool visible = false;
            for (int i = 0; i < CHUNK_SIZE; i++) {
                if (skipCheck[i][j][k]) {
                    visible = false;
                    continue;
                }
                // Front
                unsigned int faceType = getBlockFace(blocks[i][j][k], face);

                if (visible && blocks[i][j][k] == blocks[i - 1][j][k]) {
                    int xx = x - 6 * vertex_size; // rewind
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j + 1, k, faceType, face);
                    setCubeVertex(verts, xx, i + 1, j, k, faceType, face);
                    xx += vertex_size;
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j + 1, k, faceType, face);
                }
                else if (!(k > 0 && !transparentBlock(blocks[i][j][k - 1]))) {
                    setCubeVertex(verts, x, i, j, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k, faceType, face);
                    setCubeVertex(verts, x, i, j, k, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k, faceType, face);

                    visible = true;
                } else {
                    visible = false;
                }
            }
        }
    }

    face = 1;
    for (int j = 0; j < CHUNK_SIZE; j++) {
        for (int k = 0; k < CHUNK_SIZE; k++) {
            bool visible = false;
            for (int i = 0; i < CHUNK_SIZE; i++) {
                if (skipCheck[i][j][k]) {
                    visible = false;
                    continue;
                }

                // Back
                unsigned int faceType = getBlockFace(blocks[i][j][k], face);
                if (visible && blocks[i][j][k] == blocks[i - 1][j][k]) {
                    int xx = x - 6 * vertex_size; // rewind
                    setCubeVertex(verts, xx, i + 1, j, k + 1, faceType, face);
                    xx += vertex_size;
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j, k + 1, faceType, face);
                    setCubeVertex(verts, xx, i + 1, j + 1, k + 1, faceType, face);
                    xx += vertex_size;
                } else if (!(k < CHUNK_SIZE - 1  && !transparentBlock(blocks[i][j][k + 1]))) {
                    setCubeVertex(verts, x, i + 1, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k + 1, faceType, face);
                    visible = true;
                } else {
                    visible = false;
                }
            }
        }
    }

    face = 2;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            bool visible = false;
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (skipCheck[i][j][k]) {
                    visible = false;
                    continue;
                }

                // Left
                unsigned int faceType = getBlockFace(blocks[i][j][k], face);
                if (visible && blocks[i][j][k] == blocks[i][j][k - 1]) {
                    int xx = x - 6 * vertex_size; // rewind
                    setCubeVertex(verts, xx, i, j, k + 1, faceType, face);
                    xx += vertex_size;
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i, j, k + 1, faceType, face);
                    setCubeVertex(verts, xx, i, j + 1, k + 1, faceType, face);
                    xx += vertex_size;
                } else if (!(i > 0 && !transparentBlock(blocks[i - 1][j][k]))) {
                    setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i, j, k, faceType, face);
                    setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k, faceType, face);
                    visible = true;
                } else {
                    visible = false;
                }
            }
        }
    }

    face = 3;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            bool visible = false;
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (skipCheck[i][j][k]) {
                    visible = false;
                    continue;
                }
                // Right
                unsigned int faceType = getBlockFace(blocks[i][j][k], face);
                if (visible && blocks[i][j][k] == blocks[i][j][k - 1]) {
                    int xx = x - 6 * vertex_size; // rewind
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, xx, i + 1, j, k + 1, faceType, face);
                    xx += vertex_size;
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j + 1 , k + 1, faceType, face);
                } else if (!(i < CHUNK_SIZE - 1  && !transparentBlock(blocks[i + 1][j][k]))) {
                    setCubeVertex(verts, x, i + 1, j, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1 , k + 1, faceType, face);

                    visible = true;
                } else {
                    visible = false;
                }
            }
        }
    }

    face = 4;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            bool visible = false;
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (skipCheck[i][j][k]) {
                    visible = false;
                    continue;
                }

                // Bottom
                int faceType = -getBlockFace(blocks[i][j][k], face);
                if (visible && blocks[i][j][k] == blocks[i][j][k - 1]) {
                    int xx = x - 6 * vertex_size; // rewind
                    setCubeVertex(verts, xx, i, j, k + 1, faceType, face);
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j, k + 1, faceType, face);
                    setCubeVertex(verts, xx, i, j, k + 1, faceType, face);
                    xx += vertex_size;
                    xx += vertex_size;
                } else if (!(j > 0 && !transparentBlock(blocks[i][j - 1][k]))) {
                    setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                    setCubeVertex(verts, x, i, j, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j, k, faceType, face);
                    visible = true;
                } else {
                    visible = false;
                }
            }
        }
    }

    face = 5;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            bool visible = false;
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (skipCheck[i][j][k]) {
                    visible = false;
                    continue;
                }
                // Top
                int faceType = -getBlockFace(blocks[i][j][k], face);
                if (visible && blocks[i][j][k] == blocks[i][j][k - 1]) {
                    int xx = x - 6 * vertex_size; // rewind
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i + 1, j + 1, k + 1, faceType, face);
                    xx += vertex_size;
                    xx += vertex_size;
                    setCubeVertex(verts, xx, i, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, xx, i + 1, j + 1, k + 1, faceType, face);
                } else if (!(j < CHUNK_SIZE - 1  && !transparentBlock(blocks[i][j + 1][k]))) {
                    setCubeVertex(verts, x, i, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k, faceType, face);
                    setCubeVertex(verts, x, i, j + 1, k + 1, faceType, face);
                    setCubeVertex(verts, x, i + 1, j + 1, k + 1, faceType, face);
                    visible = true;
                } else {
                    visible = false;
                }
            }
        }
    }

    numCubeVertices = x / vertex_size;

    face = 3;
    int faceType = 39;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if (blocks[i][j][k] != BlockType::GRASS_BLADE) {
                    continue;
                }
                setCubeVertex(verts, x, i, j, k, faceType, face);
                setCubeVertex(verts, x, i + 1, j, k + 1, faceType, face);
                setCubeVertex(verts, x, i + 1, j + 1, k + 1, faceType, face);
                setCubeVertex(verts, x, i, j, k, faceType, face);
                setCubeVertex(verts, x, i, j + 1, k, faceType, face);
                setCubeVertex(verts, x, i + 1, j + 1, k + 1, faceType, face);

                faceType = -39;

                setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                setCubeVertex(verts, x, i + 1, j, k, faceType, face);
                setCubeVertex(verts, x, i + 1, j + 1, k, faceType, face);
                setCubeVertex(verts, x, i, j, k + 1, faceType, face);
                setCubeVertex(verts, x, i, j + 1, k + 1, faceType, face);
                setCubeVertex(verts, x, i + 1, j + 1, k, faceType, face);
            }
        }
    }


    numVertices = x / vertex_size;

    if (numVertices == 0) {
        delete [] verts;
        return;
    } else {
        // Create the cube vertex buffer
        glGenBuffers( 1, &m_vbo );
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
        glBufferData( GL_ARRAY_BUFFER, x * sizeof(float), verts, GL_STATIC_DRAW );

        glGenVertexArrays( 1, &m_vao_cube );
        glBindVertexArray( m_vao_cube );

        glEnableVertexAttribArray( cube_shader->positionAttrib );
        glVertexAttribPointer( cube_shader->positionAttrib, 4, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), nullptr );

        glEnableVertexAttribArray( cube_shader->normalAttrib );
        glVertexAttribPointer( cube_shader->normalAttrib, 3, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), (void*)(4 * sizeof(float)));

        glGenVertexArrays( 1, &m_vao_shadow );
        glBindVertexArray( m_vao_shadow );

        glEnableVertexAttribArray( shadow_shader->positionAttrib );
        glVertexAttribPointer( shadow_shader->positionAttrib, 4, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), nullptr );

        delete [] verts;
        CHECK_GL_ERRORS;
    }
}

glm::vec3 Chunk::getPosition() {
    return m_position;
}

void Chunk::createTerrain(Perlin* perlin) {
    requireUpdate = true;
    if (m_position.y == 0) {
        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int k = 0; k <  CHUNK_SIZE; k++) {
                glm::vec2 pos = glm::vec2(
                    (m_position.x + i) / CHUNK_SIZE,
                    (m_position.z + k) / CHUNK_SIZE);

                int height = perlin->OctavePerlin(pos, 4, 2) * CHUNK_SIZE;
                int maxHeight = height > WATER_LEVEL ? height : WATER_LEVEL;

                for (int j = 0; j < maxHeight; j++) {
                    if (j == height - 1 && height  == maxHeight) {
                        blocks[i][j][k] = GRASS;
                        if (j < CHUNK_SIZE - 1 && j > 10 && i % 2 == 0 && k % 2 == 1) {
                            blocks[i][j + 1][k] = GRASS_BLADE;
                        }
                    } else if ((height == 0 && j == 0) || (height < maxHeight && j == height - 1)) {
                        blocks[i][j][k] = SAND;
                    } else if (j >= height) {
                        blocks[i][j][k] = WATER;
                    } else if (j >= height - 2) {
                        blocks[i][j][k] = DIRT;
                    } else {
                        blocks[i][j][k] = ROCK;
                    }
                }
            }
        }
    }
}

bool transparentBlock(BlockType block) {
    switch (block) {
        case EMPTY:
        case GRASS_BLADE:
        case WATER:
            return true;
            break;
        default:
            return false;
            break;
    }
    return false;
}

bool Chunk::surrounded(int x, int y, int z) {
    if (x == 0 || y == 0 || z == 0 || x == CHUNK_SIZE - 1 || y == CHUNK_SIZE - 1 || z == CHUNK_SIZE - 1) {
        return false;
    }
    // Check 6 sides
    if (transparentBlock(blocks[x + 1][y][z])) {
        return false;
    }
    if (transparentBlock(blocks[x - 1][y][z])) {
        return false;
    }
    if (transparentBlock(blocks[x][y + 1][z])) {
        return false;
    }
    if (transparentBlock(blocks[x][y - 1][z])) {
        return false;
    }
    if (transparentBlock(blocks[x][y][z + 1])) {
        return false;
    }
    if (transparentBlock(blocks[x][y][z - 1])) {
        return false;
    }
    return true;
}

glm::vec3 Chunk::getFaceNormal(int face) {
    switch (face) {
        case 0: // Front
            return glm::vec3(0,0,-1);
            break;
        case 1: // Back
            return glm::vec3(0,0,1);
            break;
        case 2: // Left
            return glm::vec3(-1,0,0);
            break;
        case 3: // Right
            return glm::vec3(1,0,0);
            break;
        case 4: // Bottom
            return glm::vec3(0,-1,0);
            break;
        case 5: // Top
            return glm::vec3(0,1,0);
            break;
        default:
            // Impossible!
            break;
    }
    return glm::vec3(1,1,1);
}


// Face: 0 front 1 back 2 left 3 right 4 bottom 5 top
unsigned int getBlockFace(BlockType type, unsigned face) {
    switch(type) {
        case GRASS:
            if (face < 4) {
                return 3;
            } else if (face == 4) {
                return 2;
            } else {
                return 40;
            }
            break;
        case GRASS_BLADE:
            if (face < 4) {
                return 39;
            } else {
                return 31;
            }
            break;
        case DIRT:
            return 2;
            break;
        case SAND:
            return 8 * 16 + 14;
            break;
        case WATER:
            return 12 * 16 + 14;
            break;
        case ROCK:
            return 1;
            break;
        case TREE:
            if (face < 4) {
                return 16 + 4;
            } else {
                return 16 + 5;
            }
            break;
        case SNOW:
            if (face < 4) {
                return 4 * 16 + 4;
            } else if (face == 4) {
                return 2;
            } else {
                return 4 * 16 + 2;
            }
            break;
        default:
            break;
    }
    return 0;
}
