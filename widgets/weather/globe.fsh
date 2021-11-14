#version 100
varying lowp vec3 normalWorld;
varying lowp vec3 vertexPositionWorld;
varying mediump vec2 textureCoordinate;

uniform lowp vec3 sunPositionWorld;
uniform sampler2D dayTextureHandle;
uniform sampler2D nightTextureHandle;

void main(void)
{
    lowp vec4 night = texture2D(nightTextureHandle, textureCoordinate);
    lowp vec4 day = texture2D(dayTextureHandle, textureCoordinate);
    lowp vec3 lightVectorWorld = normalize(sunPositionWorld - vertexPositionWorld);
    lowp float brightness = dot(lightVectorWorld, normalize(normalWorld));
    gl_FragColor = mix(night, day, brightness);
}
