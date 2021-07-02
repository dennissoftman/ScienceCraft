#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <glm/glm.hpp>
#include <set>
#include <vector>
#include <unordered_map>
#include <mutex>

#define CHUNK_WIDTH (4)
#define CHUNK_HEIGHT (4)
#define CHUNK_DEPTH (4)

class Chunk
{
public:
    bool setBlock(const glm::ivec3 &rpos, int id);
    int getBlock(const glm::ivec3 &rpos);

    // TODO: Chunk::update()
    void update(uint64_t curTick);

    const std::set<int> &getTextures() const;

    const glm::ivec3 &getPos() const;

    const uint8_t *data() const;

    // heightmap is 4x4 elements array
    static Chunk *createChunk(const glm::ivec3 &pos, int *heightmap);
    static void generateChunk(glm::ivec2 pos, std::unordered_map<std::string, Chunk*> &outPtr);

    static std::mutex *chunkMutex;
private:
    glm::ivec3 pos;        // pos in chunks
    std::vector<uint8_t> cdata;
//    uint8_t cdata[CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_DEPTH];
    std::set<int> textures;
//    uint8_t textures[16];
};

#endif // CHUNK_HPP
