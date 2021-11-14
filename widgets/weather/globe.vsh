#version 100
attribute vec4 vertexPositionModel;
attribute vec3 normalModel;
attribute vec2 textureCoordinateAttribute;

uniform mat4 modelToWorldMatrix;
uniform mat4 worldToProjectionMatrix;

varying vec3 normalWorld;
varying vec3 vertexPositionWorld;
varying vec2 textureCoordinate;

void main(void)
{
    textureCoordinate = textureCoordinateAttribute;
    normalWorld = vec3(modelToWorldMatrix * vec4(normalModel, 0.0));
    vec4 worldPosition = modelToWorldMatrix * vertexPositionModel;
    vertexPositionWorld = vec3(worldPosition);
    gl_Position = worldToProjectionMatrix * worldPosition;
}
