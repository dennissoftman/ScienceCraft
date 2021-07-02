#include "shadermanager.hpp"
#include <fstream>
#include <cassert>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader()
    : Shader("", "", "")
{

}

Shader::Shader(const std::string &v_path, const std::string &f_path, const std::string &g_path)
{
    if(v_path.length() == 0 || f_path.length() == 0)
    {
        fprintf(stderr, "Empty shader names\n");
        return;
    }
    bool load_geom = (g_path.length() > 0);

    // load shaders from files
    std::ifstream fin;
    std::string vsh_source, fsh_source, gsh_source;

    fin.open(v_path);
    assert(fin.is_open() && "Failed to load vertex shader");
    std::getline(fin, vsh_source, (char)fin.eof());
    fin.close();

    fin.open(f_path);
    assert(fin.is_open() && "Failed to load fragment shader");
    std::getline(fin, fsh_source, (char)fin.eof());
    fin.close();

    if(load_geom)
    {
        fin.open(g_path);
        assert(fin.is_open() && "Failed to load geometry shader");
        std::getline(fin, gsh_source, (char)fin.eof());
        fin.close();
    }

    // load shaders to memory
    GLuint vsh, fsh, gsh;
    vsh = glCreateShader(GL_VERTEX_SHADER);
    fsh = glCreateShader(GL_FRAGMENT_SHADER);
    if(load_geom)
        gsh = glCreateShader(GL_GEOMETRY_SHADER);

    const GLchar *vsh_raw = vsh_source.c_str();
    glShaderSource(vsh, 1, &vsh_raw, nullptr);
    const GLchar *fsh_raw = fsh_source.c_str();
    glShaderSource(fsh, 1, &fsh_raw, nullptr);
    const GLchar *gsh_raw = nullptr;
    if(load_geom)
    {
        gsh_raw = gsh_source.c_str();
        glShaderSource(gsh, 1, &gsh_raw, nullptr);
    }

    // compile shaders
    int ok = 0;
    char infoBuff[512];

    glCompileShader(vsh);
    glGetShaderiv(vsh, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        glGetShaderInfoLog(vsh, 512, nullptr, infoBuff);
        fprintf(stderr, "[%s] Shader compilation error:\n%s\n", v_path.c_str(), infoBuff);
    }
    glCompileShader(fsh);
    glGetShaderiv(fsh, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        glGetShaderInfoLog(fsh, 512, nullptr, infoBuff);
        fprintf(stderr, "[%s] Shader compilation error:\n%s\n", f_path.c_str(), infoBuff);
    }
    if(load_geom)
    {
        glCompileShader(gsh);
        glGetShaderiv(gsh, GL_COMPILE_STATUS, &ok);
        if(!ok)
        {
            glGetShaderInfoLog(gsh, 512, nullptr, infoBuff);
            fprintf(stderr, "[%s] Shader compilation error:\n%s\n", g_path.c_str(), infoBuff);
        }
    }

    // link shaders
    shp = glCreateProgram();
    glAttachShader(shp, vsh);
    glAttachShader(shp, fsh);
    if(load_geom)
        glAttachShader(shp, gsh);
    glLinkProgram(shp);
    glGetProgramiv(shp, GL_LINK_STATUS, &ok);
    if(!ok)
    {
        glGetProgramInfoLog(shp, 512, nullptr, infoBuff);
        fprintf(stderr, "Shader linking error:\n%s\n", infoBuff);
    }

    // cleanup
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    if(load_geom)
        glDeleteShader(gsh);
}

Shader::~Shader()
{
    glDeleteProgram(shp);
}

void Shader::use()
{
    glUseProgram(shp);
}

void Shader::setInt(const std::string &prop, int a)
{
    glUniform1i(glGetUniformLocation(shp, prop.c_str()), a);
}

void Shader::setFloat(const std::string &prop, float a)
{
    glUniform1f(glGetUniformLocation(shp, prop.c_str()), a);
}

void Shader::setMat4(const std::string &prop, const glm::mat4 &mat)
{
    glUniformMatrix4fv(glGetUniformLocation(shp, prop.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec2(const std::string &prop, const glm::vec2 &vec)
{
    glUniform2fv(glGetUniformLocation(shp, prop.c_str()), 1, glm::value_ptr(vec));
}

void Shader::setVec3(const std::string &prop, const glm::vec3 &vec)
{
    glUniform3fv(glGetUniformLocation(shp, prop.c_str()), 1, glm::value_ptr(vec));
}

void Shader::setIntArray(const std::string &prop, const int *arr, int size)
{
    glUniform1iv(glGetUniformLocation(shp, prop.c_str()), size, arr);
}

ShaderManager::ShaderManager()
{

}

Shader *ShaderManager::get(const std::string &id) const
{
    if(m_shaders.find(id) != m_shaders.end())
        return m_shaders.at(id);
    return nullptr;
}

void ShaderManager::loadShader(const std::vector<std::string> &paths, const std::string &id)
{
    assert(m_shaders.find(id) == m_shaders.end() && "Shader duplicate");
    assert((paths.size() == 2 || paths.size() == 3) && "Invalid shader quantity");

    if(paths.size() == 2)
        m_shaders[id] = new Shader(paths[0], paths[1]);
    else
        m_shaders[id] = new Shader(paths[0], paths[1], paths[2]);
}


