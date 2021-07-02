#include "chunk.hpp"
#include "gamewindow.hpp"

#include "PerlinNoise.hpp"

std::mutex *Chunk::chunkMutex = nullptr;

__attribute__((constructor)) void chunk_init()
{
    Chunk::chunkMutex = new std::mutex();
}

__attribute__((destructor)) void chunk_deinit()
{
    delete Chunk::chunkMutex;
}

Chunk *Chunk::createChunk(const glm::ivec3 &pos, int *heightmap)
{
    Chunk *res = new Chunk();
    res->pos = pos;
    res->cdata.resize(CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_DEPTH, 0);

    for(int i=0; i < CHUNK_WIDTH; i++)
    {
        for(int j=0; j < CHUNK_DEPTH; j++)
        {
            const int py = heightmap[j + i*4];

            for(int h=pos.y; h < py; h++)
            {
                int bid;
                if(h < 2)
                    bid = 7;
                else if(h < (py / 2))
                    bid = 1;
                else if(h < (py-1))
                    bid = 3;

                if(h == py-1)
                    bid = 2;

                int k = h - pos.y;
                res->setBlock(glm::ivec3(i, k, j), bid);
            }
        }
    }
    return res;
}

void Chunk::generateChunk(glm::ivec2 pos, std::unordered_map<std::string, Chunk*> &outPtr)
{
    std::vector<Chunk*> buffer;

    int hmap[CHUNK_WIDTH * CHUNK_DEPTH];
    siv::PerlinNoise noiseGen(GameWindow::m_seed);
    for(int i=0; i < CHUNK_WIDTH; i++)
        for(int j=0; j < CHUNK_DEPTH; j++)
            hmap[j + i*4] = 1 + (63) * (noiseGen.accumulatedOctaveNoise2D((4*pos.x + i)/128.0, (4*pos.y + j)/128.0, 16) + 1.0) / 2.0;

    for(int i=0; i < 16; i++)
        buffer.push_back(Chunk::createChunk(glm::ivec3(pos.x, i, pos.y), hmap));

    Chunk::chunkMutex->lock();
    for(int i=0; i < 16; i++)
        outPtr[asString(glm::ivec3(pos.x, i, pos.y))] = buffer[i];
    Chunk::chunkMutex->unlock();
}

bool Chunk::setBlock(const glm::ivec3 &rpos, int id)
{
    if(rpos.x < 0 || rpos.y < 0 || rpos.z < 0)
        return false;
    if(abs(rpos.x) >= CHUNK_WIDTH || abs(rpos.y) >= CHUNK_HEIGHT || abs(rpos.z) >= CHUNK_DEPTH)
        return false;

    int bpos = rpos.x*CHUNK_DEPTH*CHUNK_HEIGHT + rpos.y*CHUNK_DEPTH + rpos.z;
    cdata[bpos] = id;
    return true;
}

int Chunk::getBlock(const glm::ivec3 &rpos)
{
    if(rpos.x < 0 || rpos.y < 0 || rpos.z < 0)
        return 0;
    if(abs(rpos.x) >= CHUNK_WIDTH || abs(rpos.y) >= CHUNK_HEIGHT || abs(rpos.z) >= CHUNK_DEPTH)
        return 0;

    int bpos = rpos.x*CHUNK_DEPTH*CHUNK_HEIGHT + rpos.y*CHUNK_DEPTH + rpos.z;
    return cdata[bpos];
}

void Chunk::update(uint64_t curTick)
{
    (void)curTick;
}

const std::set<int> &Chunk::getTextures() const
{
    return textures;
}

const glm::ivec3 &Chunk::getPos() const
{
    return pos;
}

const uint8_t *Chunk::data() const
{
    return cdata.data();
}
