#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main() {
    // get distance between light and fragment position
    float ligthDist = length(FragPos.xyz - lightPos);

    // map to [0, 1]
    ligthDist /= farPlane;

    // write to depth map
    gl_FragDepth = ligthDist;
}