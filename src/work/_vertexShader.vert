#version 410 core

layout (location = 0) in vec3 position;

//Matrices to transform the vertices 
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vec4 mvPosition = viewMatrix * modelMatrix * vec4(position, 1.0);

    gl_Position = projectionMatrix * mvPosition;
}
