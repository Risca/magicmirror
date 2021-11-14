#pragma once

#include <QOpenGLWidget>

namespace weather {

// Just connect to QWidget::repaint() when time changes
class Globe : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit Globe(QWidget *parent = 0);
    ~Globe();

    virtual QSize sizeHint() const;

protected:
    void initializeGL();
    void paintGL();

private:
    GLuint _programID;
    GLuint _glBufferId;
    GLuint _glTextureID[2];
    GLint _numIndices;
    GLsizeiptr _indexOffset;
};

} // namespace weather
