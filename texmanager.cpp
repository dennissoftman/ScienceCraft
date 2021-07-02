#include "texmanager.hpp"
#include <cassert>

Texture::Texture()
    : Texture(TEX_NONE)
{

}

Texture::Texture(const std::string &path)
    : data(nullptr)
{
    if(path == TEX_NONE)
    {
        unsigned char *tdata = (unsigned char*)malloc(64*64*3);
        int flip = 0;
        for(int i=0; i < 64; i++)
        {
            for(int j=0; j < 64; j++)
            {
                tdata[3*(j + i*64) + 0] = flip ? 0xff : 0;
                tdata[3*(j + i*64) + 1] = 0x00;
                tdata[3*(j + i*64) + 2] = flip ? 0xff : 0;

                if(j != 63)
                    flip = !flip;
            }
        }
        width = height = 64;
        depth = 24;
        data = tdata;
    }
    else
    {
        SDL_Surface *tdata = IMG_Load(path.c_str());
        if(tdata == NULL)
        {
            fprintf(stderr, "Failed to load '%s'\n", path.c_str());
            Texture(TEX_NONE);
            return;
        }

        tdata = SDL_ConvertSurface(tdata, tdata->format, 0);
        if(tdata == NULL)
        {
            fprintf(stderr, "Failed to optimize texture\n");
            Texture(TEX_NONE);
            return;
        }
        data = (uint32_t*)malloc(tdata->w * tdata->h * sizeof(uint32_t));
        memcpy(data, tdata->pixels, tdata->h * tdata->pitch);
        width = tdata->w;
        height = tdata->h;
        depth = tdata->format->BitsPerPixel;
        SDL_FreeSurface(tdata);
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, (depth == 32) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTextureParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1);

        float maxA = 0.f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxA);
        float a_amount = std::min(4.f, maxA);
        glTextureParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, a_amount);
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
    glDeleteTextures(1, &id);
    free(data);
}

GLuint Texture::getID() const
{
    return id;
}

void *Texture::getData() const
{
    return data;
}

int Texture::getWidth() const
{
    return width;
}

int Texture::getHeight() const
{
    return height;
}

int Texture::getDepth() const
{
    return depth;
}

TexManager::TexManager()
{
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF | IMG_INIT_WEBP);
}

TexManager::~TexManager()
{
    for(auto &p : m_textureArrays)
        glDeleteTextures(1, &p.second);
    for(auto &p : m_textures)
        delete p.second;
    IMG_Quit();
}

GLuint TexManager::get(const std::string &id) const
{
    if(m_textures.find(id) != m_textures.end())
        return m_textures.at(id)->getID();
    return 0;
}

void TexManager::loadTex(const std::string &path, const std::string &id)
{
    assert(m_textures.find(id) == m_textures.end() && "Texture duplicate");

    m_textures[id] = new Texture(path);
}

void TexManager::createArray(const std::string &id,
                             const std::unordered_map<int, std::string> &textures,
                             const glm::ivec2 &tileSize)
{
    GLuint arr_id = 0;
    glGenTextures(1, &arr_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, arr_id);
    glTexImage3D(GL_TEXTURE_2D_ARRAY,
                 0,
                 GL_RGBA,
                 tileSize.x, tileSize.y, textures.size(),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    int i = 0;
    for(auto &texPair : textures)
    {
        if(m_textures.find(texPair.second) == m_textures.end())
            continue;
        const Texture *tex = m_textures[texPair.second];
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                        0,
                        0, 0, i,
                        tileSize.x, tileSize.y, 1,
                        (tex->getDepth() == 32) ? GL_RGBA : GL_RGB,
                        GL_UNSIGNED_BYTE,
                        tex->getData());
        i += 1;
    }
    glTextureParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTextureParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTextureParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -1);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    m_textureArrays[id] = arr_id;
}

GLuint TexManager::getArray(const std::string &id) const
{
    if(m_textureArrays.find(id) != m_textureArrays.end())
        return m_textureArrays.at(id);
    return 0;
}
