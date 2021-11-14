#pragma once

#include "shapedata.h"

namespace weather {

class ShapeGenerator
{
public:
    static ShapeData makeSphere(uint tesselation = 10);
};

} // namespace weather
