#version 410 core

// Values recieved from vertex shader (in)
in vec2 interp_texCoords;

// Values given from the application (uniform)
uniform float repeat;
uniform sampler2D tex;

// values passed to render (out)
out vec4 color;

void main()
{
    vec2 repeatedUv = mod(interp_texCoords*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeatedUv);

    vec3 finalColor = surfaceColor.rgb;

    color = vec4(finalColor, 1.0);
}
