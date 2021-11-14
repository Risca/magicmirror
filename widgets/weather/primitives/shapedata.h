#pragma once

#include <QtGui/qopengl.h>
#include <QVector>
#include "vertex.h"

namespace weather {

struct ShapeData
{
    QVector<Vertex> vertices;
    QVector<GLushort> indices;
    GLsizeiptr vertexBufferSize() const
    {
        return sizeof(Vertex) * vertices.count();
    }
    GLsizeiptr indexBufferSize() const
    {
        return sizeof(GLushort) * indices.count();
    }
    GLsizeiptr vertexStride() const
    {
        return sizeof(Vertex);
    }
};

} // namespace weather
