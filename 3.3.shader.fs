#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float opacity;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    // FragColor = gl_Position;
    FragColor = vec4(lightColor * objectColor, 1.0);
}