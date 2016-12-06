#include "ChunkManager.hpp"
#include <cstdlib>
#include <iostream>
#include <cmath>

#include <glm/gtc/noise.hpp>

ChunkManager::ChunkManager() {
    m_player_position = glm::vec3(999, 999, 999);
    perlin = Perlin::instance();

    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                chunks[x][y][z] = NULL;
            }
        }
    }
}

ChunkManager::~ChunkManager() {
    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                if (chunks[x][y][z] != NULL) {
                    delete chunks[x][y][z];
                }
            }
        }
    }
    for (Chunk* chunk : unloadList) {
        delete chunk;
    }
}

void ChunkManager::render(glm::mat4& view, glm::mat4& depth) {
    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                if (chunks[x][y][z] != NULL) {
                    chunks[x][y][z]->render(view, depth);
                }
            }
        }
    }
}

void ChunkManager::renderShadow(glm::mat4& VP) {
    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                if (chunks[x][y][z] != NULL) {
                    chunks[x][y][z]->renderShadow(VP);
                }
            }
        }
    }
}

inline glm::vec3 toChunkCoord(glm::vec3 v) {
    return glm::vec3(
        floor(v.x / CHUNK_SIZE),
        floor(v.y / CHUNK_SIZE),
        floor(v.z / CHUNK_SIZE)
    );
}

inline glm::vec3 toNormalCoord(glm::vec3 v) {
    return glm::vec3(
        ((int)v.x) * CHUNK_SIZE,
        ((int)v.y) * CHUNK_SIZE,
        ((int)v.z) * CHUNK_SIZE
    );
}

void ChunkManager::update(glm::vec3& player_position) {
    glm::vec3 chunk_player_position = toChunkCoord(player_position);

    if (chunk_player_position != m_player_position) {
        m_player_position = chunk_player_position;

        origin = m_player_position -
            glm::vec3(CHUNK_LIST_X / 2, CHUNK_LIST_Y / 2, CHUNK_LIST_Z / 2);
        updatePlayerPosition();
    }

    updateLoadList();
    updateUnloadList();
}

void ChunkManager::updatePlayerPosition() {
    Chunk* chunkLoaded[CHUNK_LIST_X][CHUNK_LIST_Y][CHUNK_LIST_Z];
    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                chunkLoaded[x][y][z] = NULL;
            }
        }
    }

    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                Chunk* chunk = chunks[x][y][z];
                if (chunk == NULL)
                    continue;

                glm::vec3 chunkPos = toChunkCoord(chunk->getPosition()) - origin;
                if (chunkPos.x < 0 ||
                    chunkPos.y < 0 ||
                    chunkPos.z < 0 ||
                    chunkPos.x >= CHUNK_LIST_X ||
                    chunkPos.y >= CHUNK_LIST_Y ||
                    chunkPos.z >= CHUNK_LIST_Z) {
                    unloadList.push_back(chunk);
                } else {
                    chunkLoaded[(int)chunkPos.x][(int)chunkPos.y][(int)chunkPos.z] = chunk;
                }
            }
        }
    }

    for (int x = 0; x < CHUNK_LIST_X; x++) {
        for (int y = 0; y < CHUNK_LIST_Y; y++) {
            for (int z = 0; z < CHUNK_LIST_Z; z++) {
                if (chunkLoaded[x][y][z] == NULL) {
                    loadList.push_back(glm::vec3(x, y, z));
                }
                chunks[x][y][z] = chunkLoaded[x][y][z];
            }
        }
    }
}

void ChunkManager::updateLoadList() {
    /*
    if (loadList.empty()) {
        return;
    }

    glm::vec3 coord = loadList[0];
    loadList.erase(loadList.begin());
    */
    for (glm::vec3& coord : loadList) {
        glm::vec3 normalCoord = toNormalCoord(coord + origin);
        Chunk* chunk = new Chunk(normalCoord);
        chunk->createTerrain(perlin);
        chunks[(int)coord.x][(int)coord.y][(int)coord.z] = chunk;
    }
    loadList.clear();
}

void ChunkManager::updateUnloadList() {
    /*
    if (unloadList.empty()) {
        return;
    }

    Chunk* chunk = unloadList[0];
    unloadList.erase(unloadList.begin());
    */
    for (Chunk* chunk : unloadList) {
        delete chunk;
    }
    unloadList.clear();
}

bool ChunkManager::solidBlock(glm::vec3& position) {
    // Find which chunk this belongs to
    glm::vec3 playerChunkPos = toChunkCoord(position);
    glm::vec3 chunkPos = playerChunkPos - origin;
    if (chunkPos.x < 0 ||
        chunkPos.y < 0 ||
        chunkPos.z < 0 ||
        chunkPos.x >= CHUNK_LIST_X ||
        chunkPos.y >= CHUNK_LIST_Y ||
        chunkPos.z >= CHUNK_LIST_Z) {
        return true;
    }

    Chunk* chunk = chunks[(int)chunkPos.x][(int)chunkPos.y][(int)chunkPos.z];
    // unloaded chunk
    if (chunk == NULL) {
        return false;
    }

    glm::vec3 localCoord = position - toNormalCoord(playerChunkPos);

    BlockType type = chunk->getBlock((int)localCoord.x, (int)localCoord.y, (int)localCoord.z);
    if ( passableBlock(type) ) {
        return false;
    }

    return true;
}

bool ChunkManager::destroyBlock(glm::vec3& position) {
    // Find which chunk this belongs to
    glm::vec3 playerChunkPos = toChunkCoord(position);
    glm::vec3 chunkPos = playerChunkPos - origin;
    if (chunkPos.x < 0 ||
        chunkPos.y < 0 ||
        chunkPos.z < 0 ||
        chunkPos.x >= CHUNK_LIST_X ||
        chunkPos.y >= CHUNK_LIST_Y ||
        chunkPos.z >= CHUNK_LIST_Z) {
        return false;
    }

    Chunk* chunk = chunks[(int)chunkPos.x][(int)chunkPos.y][(int)chunkPos.z];
    // unloaded chunk
    if (chunk == NULL) {
        return false;
    }

    glm::vec3 localCoord = position - toNormalCoord(playerChunkPos);

    BlockType type = chunk->getBlock((int)localCoord.x, (int)localCoord.y, (int)localCoord.z);
    if ( transparentBlock(type) ) {
        return false;
    }

    chunk->setBlock((int)localCoord.x, (int)localCoord.y, (int)localCoord.z, BlockType::EMPTY);
    return true;
}


