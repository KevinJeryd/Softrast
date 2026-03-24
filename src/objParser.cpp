#include "../include/objParser.h"
#include <fstream>
#include <string>
#include <sstream>

namespace ObjParser
{
    std::vector<GMath::Triangle> parseObj(std::string const &path)
    {
        std::vector<GMath::Vertex> vertices;
        std::vector<GMath::Triangle> triangles;
        std::ifstream objFile(path);

        std::string line;
        while (std::getline(objFile, line))
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                std::istringstream ss(line.substr(2));
                float x, y, z;
                ss >> x >> y >> z;
                vertices.push_back(GMath::Vertex{x, y, z});
            }
            else if (line[0] == 'f' && line[1] == ' ')
            {
                std::istringstream fss(line.substr(2));
                std::string t1, t2, t3;
                fss >> t1 >> t2 >> t3;
                int i = std::stoi(t1);
                int j = std::stoi(t2);
                int k = std::stoi(t3);
                GMath::Vertex p1 = vertices[i - 1];
                GMath::Vertex p2 = vertices[j - 1];
                GMath::Vertex p3 = vertices[k - 1];
                triangles.push_back(GMath::Triangle{p1, p2, p3, 0xFFFFFFFF});
            }
        }

        return triangles;
    }
}