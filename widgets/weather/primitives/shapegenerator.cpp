#include "shapegenerator.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

namespace weather {

namespace {

ShapeData makePlaneVerts(uint dimensions)
{
    ShapeData ret;
    ret.vertices.resize(dimensions * dimensions);
    const uint half = dimensions / 2;
    for (uint i = 0; i < dimensions; i++)
    {
        for (uint j = 0; j < dimensions; j++)
        {
            Vertex& thisVert = ret.vertices[i * dimensions + j];
            thisVert.position.x = j - half;
            thisVert.position.z = i - half;
            thisVert.position.y = 0;
            thisVert.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            thisVert.textureCoordinate = vec2((float)i / dimensions, (float)j / dimensions);
        }
    }
    return ret;
}

ShapeData makePlaneIndices(uint dimensions)
{
    ShapeData ret;
    const size_t numIndices = (dimensions - 1) * (dimensions - 1) * 2 * 3; // 2 triangles per square, 3 indices per triangle
    ret.indices.resize(numIndices);
    size_t runner = 0;
    for (uint row = 0; row < dimensions - 1; row++)
    {
        for (uint col = 0; col < dimensions - 1; col++)
        {
            ret.indices[runner++] = dimensions * row + col;
            ret.indices[runner++] = dimensions * row + col + dimensions;
            ret.indices[runner++] = dimensions * row + col + dimensions + 1;

            ret.indices[runner++] = dimensions * row + col;
            ret.indices[runner++] = dimensions * row + col + dimensions + 1;
            ret.indices[runner++] = dimensions * row + col + 1;
        }
    }
    assert(runner == numIndices);
    return ret;
}

} // anonymous namespace

ShapeData weather::ShapeGenerator::makeSphere(uint tesselation)
{
    ShapeData ret = makePlaneVerts(tesselation);
    ShapeData ret2 = makePlaneIndices(tesselation);
    ret.indices = ret2.indices;

    uint dimensions = tesselation;
    const float RADIUS = 1.0f;
    const double CIRCLE = glm::pi<double>() * 2;
    const double SLICE_ANGLE = CIRCLE / (dimensions - 1);
    // rotate sphere pi/2 around X axis
    const mat4 rotationMatrix = glm::rotate(mat4(1.0f), glm::pi<float>()/2.0f, vec3(1.0f, 0.0f, 0.0f));
    for (size_t col = 0; col < dimensions; col++)
    {
        double phi = -SLICE_ANGLE * col;
        for (size_t row = 0; row < dimensions; row++)
        {
            double theta = -(SLICE_ANGLE / 2.0) * row;
            size_t vertIndex = col * dimensions + row;
            Vertex& v = ret.vertices[vertIndex];
            v.position.x = RADIUS * cos(phi) * sin(theta);
            v.position.y = RADIUS * sin(phi) * sin(theta);
            v.position.z = RADIUS * cos(theta);
            // fixup positions to show equator instead of south pole
            v.position = vec3(rotationMatrix * vec4(v.position, 0.0f));
            v.normal = glm::normalize(v.position);
        }
    }
    return ret;
}

} // namespace weather
