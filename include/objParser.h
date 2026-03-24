#ifndef INCLUDE_OBJPARSER_H
#define INCLUDE_OBJPARSER_H

#include <vector>
#include <iostream>

#include "gmath.h"

namespace ObjParser
{
    std::vector<GMath::Triangle> parseObj(std::string const &path);
}

#endif