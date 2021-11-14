#include "globe.h"
#include "primitives/shapedata.h"
#include "primitives/shapegenerator.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QImage>
#include <QSizePolicy>
#include <QTime>
#include <QVector>

namespace weather {

using glm::vec3;
using glm::vec4;
using glm::mat4;

namespace {

const double EARTH_TILT_ANGLE = 23.43645 * glm::pi<double>() / 180.0; // radians
Q_GLOBAL_STATIC_WITH_ARGS(const vec3, EARTH_TILT, (sin(EARTH_TILT_ANGLE), cos(EARTH_TILT_ANGLE), 0.0f));

template <typename T>
T deg2rad(T degrees)
{
    return degrees * glm::pi<T>() / T(180);
}

double ERA(const QDateTime& dt)
{
    double ut1;
    QDateTime UTC = dt.toUTC();
    if (UTC.time() >= QTime(12, 00)) {
        ut1 = UTC.date().toJulianDay();
    }
    else {
        ut1 = UTC.date().addDays(-1).toJulianDay();
    }
    // add fraction of a day
    ut1 += (double(UTC.time().msecsSinceStartOfDay()) / 1000.0) / 86400.0;
    // https://en.wikipedia.org/wiki/Sidereal_time#ERA
    return 2 * glm::pi<double>() * fmod(0.7790572732640 + 1.00273781191135448 * (ut1 - 2451545.0), 1.0);
}

void sendDataToOpenGL(GLuint& bufferID, GLuint textureID[], GLint& numIndices, GLsizeiptr& indexOffset)
{
    GLenum status;
    ShapeData shape = ShapeGenerator::makeSphere(50);

    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, shape.vertexBufferSize() + shape.indexBufferSize(), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, shape.vertexBufferSize(), &shape.vertices[0]);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, shape.vertexBufferSize(), shape.indexBufferSize(), &shape.indices[0]);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, shape.vertexStride(), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, shape.vertexStride(), (void*)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, shape.vertexStride(), (void*)(sizeof(GLfloat) * 6));

    numIndices = shape.indices.count();
    indexOffset = shape.vertexBufferSize();
    status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Some warnings were generated creating buffers:" << (int)status;

    glGenTextures(2, textureID);
    status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Failed to generate textures!";

    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    const QString filename[] = {
        ":/img/blue_marble.jpg",
        ":/img/black_marble.jpg",
    };
    for (int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, textureID[i]);
        QImage img = QImage(filename[i])
                .convertToFormat(QImage::Format_RGBA8888)
                .mirrored(false, true);
        if (img.isNull()) {
            qWarning() << "Failed to load texture" << filename[i];
            return;
        }
        if (img.height() > maxTextureSize || img.width() > maxTextureSize) {
            img = img.scaled(maxTextureSize, maxTextureSize, Qt::KeepAspectRatio);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
        status = glGetError();
        if (status != GL_NO_ERROR)
            qWarning() << "Some warnings were generated loading texture" << i << ":" << (int)status;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        status = glGetError();
        if (status != GL_NO_ERROR)
            qWarning() << "Some warnings were generated setting params for texture" << i << ":" << (int)status;
    }
    qDebug() << "Textures successfully loaded!";
}

std::string readShaderCode(const QString& filename)
{
    QFile file(filename);
    if (file.open(QFile::ReadOnly))
        return file.readAll().toStdString();
    return "";
}

template<typename F1, typename F2>
bool CheckStatus(GLuint objectID, F1 objectPropertyGetterFunc, F2 getInfoLogFunc, GLenum statusType)
{
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE) {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (!infoLogLength) {
            qDebug() << "Status not OK, but no log line";
            return false;
        }

        QVector<GLchar> buffer;
        buffer.resize(infoLogLength);
        GLsizei bufferSize;
        getInfoLogFunc(objectID, infoLogLength, &bufferSize, &buffer[0]);
        qDebug().noquote() << QString(&buffer[0]);
        return false;
    }
    return true;
}

bool CheckShaderStatus(GLuint shaderID)
{
    return CheckStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool CheckProgramStatus(GLuint programID)
{
    return CheckStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

struct ScopedShader
{
    explicit ScopedShader(GLenum type) :
        _shader(glCreateShader(type)) { /* empty */ }
    ~ScopedShader() {
        glDeleteShader(_shader);
    }
    operator GLuint() { return _shader; }
private:
    GLuint _shader;
};

void installShaders(GLuint& programID)
{
    std::string shaderCode;
    ScopedShader vertexShaderID(GL_VERTEX_SHADER);
    ScopedShader fragmentShaderID(GL_FRAGMENT_SHADER);

    const GLchar* adapter[1];
    shaderCode = readShaderCode(":/shaders/globe.vsh");
    adapter[0] = shaderCode.c_str();
    glShaderSource(vertexShaderID, 1, adapter, 0);

    shaderCode = readShaderCode(":/shaders/globe.fsh");
    adapter[0] = shaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, adapter, 0);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);
    if (!CheckShaderStatus(vertexShaderID) || !CheckShaderStatus(fragmentShaderID))
        return;

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);

    // Need to bind attributes before linking
    glBindAttribLocation(programID, 0, "vertexPositionModel");
    glBindAttribLocation(programID, 1, "normalModel");
    glBindAttribLocation(programID, 2, "textureCoordinateAttribute");

    glLinkProgram(programID);

    if (!CheckProgramStatus(programID))
        return;

    glUseProgram(programID);

    GLenum status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Some warnings were generated loading shaders:" << (int)status;
    qDebug() << "Shaders successfully installed!";
}

} // anonymous namespace

Globe::Globe(QWidget *parent) : QOpenGLWidget(parent),
    _programID(0),
    _glBufferId(0),
    _glTextureID{0},
    _numIndices(0),
    _indexOffset(0)
{
    // empty
}

Globe::~Globe()
{
    glUseProgram(0);
    glDeleteProgram(_programID);
}

QSize Globe::sizeHint() const
{
    return QSize(200, 200);
}

void Globe::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // removes back side of shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    sendDataToOpenGL(_glBufferId, _glTextureID, _numIndices, _indexOffset);
    installShaders(_programID);
}

void Globe::paintGL()
{
    GLenum status;

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Something is still slightly off with these calculations.
    // Winter solstice 2021 should be 15:59 UTC, but with these calculations
    // it looks more like 17:00 UTC.
    // TODO: center around solstice or equinox
    const QDateTime dt = QDateTime::currentDateTime();
    const QDate date = dt.date();
    float phi = 2 * glm::pi<double>() * (double)date.dayOfYear() / (double)date.daysInYear();
    float theta = ERA(dt);

    const float r = 10.0f;
    const float sunHeight = 2.0 - sin(phi / 2.0f); // Move sun higher on winter to get approximately correct polar lights
    const vec3 earthPositionWorld = vec3(glm::rotate(mat4(1.0f), phi, vec3(0.0f, 1.0f, 0.0f)) * vec4(r, 0.0f, 0.0f, 1.0f));
    mat4 earthTranslationMatrix = glm::translate(mat4(1.0f), earthPositionWorld);
    mat4 modelToWorldMatrix =
        glm::rotate(
            glm::rotate(
                earthTranslationMatrix,
            theta, *EARTH_TILT),
        static_cast<float>(EARTH_TILT_ANGLE), vec3(0.0f, 0.0f, -1.0f));;

    GLint modelToWorldMatrixUniformLocation = glGetUniformLocation(_programID, "modelToWorldMatrix");
    glUniformMatrix4fv(modelToWorldMatrixUniformLocation, 1, GL_FALSE, &modelToWorldMatrix[0][0]);

    GLint ambientLightUniformLocation = glGetUniformLocation(_programID, "ambientLight");
    glUniform4f(ambientLightUniformLocation, 0.05f, 0.05f, 0.1f, 1.0f);
    GLint sunColorUniformLocation = glGetUniformLocation(_programID, "sunColor");
    glUniform4f(sunColorUniformLocation, 1.0f, 0.6f, 0.2f, 1.0f);

    const vec3 eyePositionWorld = vec3(
            glm::rotate(
                glm::rotate(
                    earthTranslationMatrix,
                deg2rad(100.0f) + theta, *EARTH_TILT),    // follow earth rotation
            deg2rad(-60.0f), glm::cross(*EARTH_TILT, vec3(0.0f, 0.0f, 1.0f))) * // look more north
        vec4(0.0f, 0.0f, 3.0f, 1.0f));
    GLint eyePositionWorldUniformLocation = glGetUniformLocation(_programID, "eyePositionWorld");
    glUniform3fv(eyePositionWorldUniformLocation, 1, &eyePositionWorld[0]);

    mat4 projectionMatrix = glm::perspective(deg2rad(60.0f), ((float)width()/height()), 0.1f, 10.0f);
    mat4 worldToProjectionMatrix = projectionMatrix * glm::lookAt(eyePositionWorld, earthPositionWorld, *EARTH_TILT);
    GLint worldToProjectionMatrixUniformLocation = glGetUniformLocation(_programID, "worldToProjectionMatrix");
    glUniformMatrix4fv(worldToProjectionMatrixUniformLocation, 1, GL_FALSE, &worldToProjectionMatrix[0][0]);

    const vec3 sunPositionWorld = vec3(0.0f, sunHeight, 0.0f);
    GLint sunPositionUniformLocation = glGetUniformLocation(_programID, "sunPositionWorld");
    glUniform3fv(sunPositionUniformLocation, 1, &sunPositionWorld[0]);

    status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Some warnings were generated loading matrices:" << (int)status;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _glTextureID[0]);
    GLint dayTextureHandleUniformLocation = glGetUniformLocation(_programID, "dayTextureHandle");
    glUniform1i(dayTextureHandleUniformLocation, 0);

    status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Some warnings were generated loading texture 0:" << (int)status;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _glTextureID[1]);
    GLint nightTextureHandleUniformLocation = glGetUniformLocation(_programID, "nightTextureHandle");
    glUniform1i(nightTextureHandleUniformLocation, 1);

    status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Some warnings were generated loading texture 1:" << (int)status;

    glDrawElements(GL_TRIANGLES, _numIndices, GL_UNSIGNED_SHORT, (void*)_indexOffset);
    status = glGetError();
    if (status != GL_NO_ERROR)
        qWarning() << "Some warnings were generated while drawing triangles:" << (int)status;
}

} // namespace weather
