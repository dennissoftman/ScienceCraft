#include "mdlmanager.hpp"
#include "dist.hpp"
#include <fstream>
#include <cassert>

Model3D::Model3D()
    : Model3D("")
{

}

bool is_obj(const std::string &path)
{
    if(path.length() < 4)
        return false;

    static const char extension[3] = {'o', 'b', 'j'};
    for(size_t i=0; i < 3; i++)
    {
        if((path[i+path.length() - 3]|0x20) != extension[i])
            return false;
    }
    return true;
}

Model3D::Model3D(const std::string &path)
    : VAO(0), VBO(0)
{
    if(path.length() == 0)
        return;

    //
    std::vector<vertex_t> mdata;
    if(is_obj(path))
        loadOBJ(path, mdata);
    m_size = mdata.size(); // vertex count
    //

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, mdata.size() * sizeof(vertex_t), mdata.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(0 * sizeof(GLfloat)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, norm));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tex));
    glBindVertexArray(0);

    mdata.clear();
}

Model3D::~Model3D()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

GLuint Model3D::getVAO() const
{
    return VAO;
}

GLuint Model3D::getSize() const
{
    return m_size;
}

void Model3D::loadOBJ(const std::string &path, std::vector<vertex_t> &out)
{
    std::vector<glm::vec3> vertCoords;
    std::vector<glm::vec3> normCoords;
    std::vector<glm::vec2> texCoords;

    std::ifstream fin;
    fin.open(path);
    if(!fin.is_open())
    {
        fprintf(stderr, "Failed to open '%s'\n", path.c_str());
        return;
    }

    out.clear();

    std::string line;
    while(std::getline(fin, line))
    {
        if(line.length() == 0)
            continue;

        if(line[0] == 'o')
        {
            // fprintf(stderr, "Model: %s\n", line.substr(2, line.length()-1).c_str());
            continue;
        }
        else if(line[0] == 'v')
        {
            if(line[1] == 't') // texCoord
            {
                std::vector<std::string> parts;
                splitString(parts, line.substr(3), ' ');
                if(parts.size() != 2)
                {
                    fprintf(stderr, "Failed to load '%s'\n", path.c_str());
                    return;
                }
                texCoords.push_back(glm::vec2(std::strtof(parts[0].c_str(), nullptr),
                                              std::strtof(parts[1].c_str(), nullptr)));
            }
            else if(line[1] == 'n') // normCoord
            {
                std::vector<std::string> parts;
                splitString(parts, line.substr(3), ' ');
                if(parts.size() != 3)
                {
                    fprintf(stderr, "Failed to load '%s'\n", path.c_str());
                    return;
                }
                normCoords.push_back(glm::vec3(std::strtof(parts[0].c_str(), nullptr),
                                               std::strtof(parts[1].c_str(), nullptr),
                                               std::strtof(parts[2].c_str(), nullptr)));
            }
            else // vertex
            {
                std::vector<std::string> parts;
                splitString(parts, line.substr(2), ' ');
                if(parts.size() != 3)
                {
                    fprintf(stderr, "Failed to load '%s'\n", path.c_str());
                    return;
                }
                vertCoords.push_back(glm::vec3(std::strtof(parts[0].c_str(), nullptr),
                                               std::strtof(parts[1].c_str(), nullptr),
                                               std::strtof(parts[2].c_str(), nullptr)));
            }
        }
        else if(line[0] == 'f')
        {
            uint64_t v1, v2, v3,
                     n1, n2, n3,
                     t1=0, t2=0, t3=0;
            std::vector<std::string> parts;
            splitString(parts, line.substr(2), ' ');
            if(parts.size() < 3)
            {
                fprintf(stderr, "Failed to load '%s'\n", path.c_str());
                return;
            }

            std::vector<std::string> face;
            // triangle 0
            splitString(face, parts[0], '/');
            if(face.size() == 3)
            {
                v1 = atoll(face[0].c_str());
                t1 = atoll(face[1].c_str());
                n1 = atoll(face[2].c_str());
            }
            else
            {
                v1 = atoll(face[0].c_str());
                n1 = atoll(face[1].c_str());
            }
            // triangle 1
            splitString(face, parts[1], '/');
            if(face.size() == 3)
            {
                v2 = atoll(face[0].c_str());
                t2 = atoll(face[1].c_str());
                n2 = atoll(face[2].c_str());
            }
            else
            {
                v2 = atoll(face[0].c_str());
                n2 = atoll(face[1].c_str());
            }
            // triangle 2
            splitString(face, parts[2], '/');
            if(face.size() == 3)
            {
                v3 = atoll(face[0].c_str());
                t3 = atoll(face[1].c_str());
                n3 = atoll(face[2].c_str());
            }
            else
            {
                v3 = atoll(face[0].c_str());
                n3 = atoll(face[1].c_str());
            }
            //
            if(v1 * v2 * v3 <= 0)
                continue;

            out.push_back({vertCoords[v1-1],
                           n1 > 0 ? normCoords[n1-1] : glm::vec3(),
                           t1 > 0 ? texCoords[t1-1] : glm::vec2()});
            out.push_back({vertCoords[v2-1],
                           n2 > 0 ? normCoords[n2-1] : glm::vec3(),
                           t2 > 0 ? texCoords[t2-1] : glm::vec2()});
            out.push_back({vertCoords[v3-1],
                           n3 > 0 ? normCoords[n3-1] : glm::vec3(),
                           t3 > 0 ? texCoords[t3-1] : glm::vec2()});
        }
    }
    fin.close();
}

MdlManager::MdlManager()
{

}

MdlManager::~MdlManager()
{
    for(auto &p : m_models)
        delete p.second;
}

Model3D *MdlManager::get(const std::string &id) const
{
    if(m_models.find(id) != m_models.end())
        return m_models.at(id);
    return 0;
}

void MdlManager::loadModel(const std::string &path, const std::string &id)
{
    assert(m_models.find(id) == m_models.end() && "Model duplicate");

    m_models[id] = new Model3D(path);
}
