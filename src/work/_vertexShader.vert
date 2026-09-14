#version 410 core

// Vertex attributes (layout)
layout (location = 0) in vec3 position;

layout (location = 2) in vec2 UV;

//Matrices to transform the vertices 
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

//Parameters passed to the fragment shader
out vec2 interp_texCoords;

void main()
{
    vec4 mvPosition = viewMatrix * modelMatrix * vec4(position, 1.0);

    gl_Position = projectionMatrix * mvPosition;

    interp_texCoords = UV;
}
