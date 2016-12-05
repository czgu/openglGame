#version 330

uniform mat4 MVP;
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec4 particle_position;
layout (location = 2) in vec4 color;

out vec4 Color;

void main() {
    gl_Position = MVP * vec4(particle_position.xyz, 1.0) + vec4(vertex_position * particle_position.w, 0.0);
    Color = color;
}
