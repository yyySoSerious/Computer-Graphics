#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 anOffset;

out vec3 fColor;

uniform vec2 offsets[100];

void main()
{
    //vec2 offset = offsets[gl_InstanceID];
    vec2 pos = aPos * (gl_InstanceID / 100.0); //scale downward from top-right to bottom-left
    gl_Position = vec4(pos + anOffset, 0.0, 1.0);
    fColor = aColor;
}  