#include "dist.hpp"

std::string asString(const glm::ivec3 &v)
{
    return std::to_string(v.x)+";"+std::to_string(v.y)+";"+std::to_string(v.z);
}

std::string &trimString(std::string &str)
{
    size_t fnf = str.find_first_not_of(' ');
    str.erase(0, fnf);

    size_t lnf = str.find_last_not_of(' ')+1;
    /*if(lnf <= str.length()-1)
        lnf++;
    */

    str.erase(lnf, str.length());
    return str;
}

void splitString(std::vector<std::string> &out, const std::string &str, char del)
{
    out.clear();
    size_t oldit=0;
    for(size_t it=0; it < str.length();)
    {
        oldit = it;
        it = str.find_first_of(del, it+1);
        std::string v;
        if(it == str.length() || it == oldit)
        {
            v = str.substr(oldit, str.length()-oldit-1);
            out.push_back(trimString(v));
            break;
        }
        v = str.substr(oldit + ( (oldit == 0) ? 0: 1), it-oldit - ((oldit == 0) ? 0 : 1));
        out.push_back(trimString(v));
    }
    if(out.at(out.size()-1) == "")
        out.pop_back();
}
