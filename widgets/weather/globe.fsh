#version 100
varying highp vec3 normalWorld;
varying highp vec3 vertexPositionWorld;
varying highp vec2 textureCoordinate;

uniform highp vec3 sunPositionWorld;
uniform sampler2D dayTextureHandle;
uniform sampler2D nightTextureHandle;

void main(void)
{
    highp vec4 night = texture2D(nightTextureHandle, textureCoordinate);
    highp vec4 day = texture2D(dayTextureHandle, textureCoordinate);
    highp vec3 lightVectorWorld = normalize(sunPositionWorld - vertexPositionWorld);
    highp float brightness = dot(lightVectorWorld, normalize(normalWorld));
    gl_FragColor = mix(night, day, brightness);
}
