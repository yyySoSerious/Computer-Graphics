#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 position;
    vec2 texCoord;
    vec3 normal;
} gs_in[];

out vec3 position;
out vec2 texCoord;
out vec3 normal;

uniform mat4 projection;
uniform float time;

vec3 GetNormal() { 
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal) { 
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0 )/ 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

void main() {    
    vec3 normal = GetNormal();

    gl_Position = projection * gl_in[0].gl_Position; //explode(gl_in[0].gl_Position, normal);
    texCoord = gs_in[0].texCoord;
    position = gs_in[0].position;
    normal = gs_in[0].normal;
    EmitVertex();

    gl_Position = projection * gl_in[1].gl_Position; //explode(gl_in[1].gl_Position, normal);
    texCoord = gs_in[1].texCoord;
    position = gs_in[1].position;
    normal = gs_in[1].normal;
    EmitVertex();

    gl_Position = projection * gl_in[2].gl_Position; //explode(gl_in[2].gl_Position, normal);
    texCoord = gs_in[2].texCoord;
    position = gs_in[2].position;
    normal = gs_in[2].normal;
    EmitVertex();

    EndPrimitive();
}  