#pragma once

#include "Chunk.hpp"
#include "Perlin.hpp"

#include <vector>

#include <glm/glm.hpp>

#define CHUNK_LIST_X 17
#define CHUNK_LIST_Y 3
#define CHUNK_LIST_Z 17

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void update(glm::vec3& player_position);
    void render(glm::mat4& view, glm::mat4& depth);
    void renderShadow(glm::mat4& VP);

    bool solidBlock(glm::vec3& position);
    bool destroyBlock(glm::vec3& position);

private:
    void updatePlayerPosition();
    void updateLoadList();
    void updateUnloadList();

    Chunk* chunks[CHUNK_LIST_X][CHUNK_LIST_Y][CHUNK_LIST_Z];
    std::vector<glm::vec3> loadList;
    std::vector<Chunk*> unloadList;

    glm::vec3 m_player_position;
    glm::vec3 origin;

    Perlin* perlin;
};
