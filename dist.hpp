#ifndef DIST_HPP
#define DIST_HPP

#define UNIMPLEMENTED fprintf(stderr, "Unimplemented function %s\n", __FUNCTION__); return

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include <SDL2/SDL.h>

#include <vector>
#include <string>

//
std::string asString(const glm::ivec3 &v);

std::string &trimString(std::string &str);
void splitString(std::vector<std::string> &out, const std::string &str, char del);
//

#endif // DIST_HPP
