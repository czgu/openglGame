#version 330

layout(location = 0) out float fragmentdepth;

void main() {
    // No need to use homogenous coord, because projection
    // is an orthonormal matrix
    fragmentdepth = gl_FragCoord.z;
}
