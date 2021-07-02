#ifndef TEXMANAGER_HPP
#define TEXMANAGER_HPP

#include <string>
#include <unordered_map>

#include <SDL2/SDL_image.h>
#define TEX_NONE ":none\x0b"

#include "dist.hpp"

class Texture
{
public:
    Texture();
    Texture(const std::string &path);
    ~Texture();

    GLuint getID() const;
    void *getData() const;
    int getWidth() const;
    int getHeight() const;
    int getDepth() const;

private:
    int width, height, depth;
    GLuint id;
    void *data;
};

class TexManager
{
public:
    TexManager();
    ~TexManager();

    GLuint get(const std::string &id) const;

    void loadTex(const std::string &path, const std::string &id);

    void createArray(const std::string &id,
                     const std::unordered_map<int, std::string> &textures,
                     const glm::ivec2 &tileSize=glm::ivec2(16, 16));
    GLuint getArray(const std::string &id) const;
private:
    std::unordered_map<std::string, Texture*> m_textures;

    std::unordered_map<std::string, GLuint> m_textureArrays;
};

#endif // TEXMANAGER_HPP
