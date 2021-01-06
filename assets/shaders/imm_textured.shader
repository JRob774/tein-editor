#version 130

uniform sampler2D uTexture0;
varying vec2 vTexCoord;

[VertProgram]

void main ()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
    vTexCoord = gl_MultiTexCoord0.st;
}

[FragProgram]

void main ()
{
    gl_FragColor = texture(uTexture0, vTexCoord) * gl_Color;
}
