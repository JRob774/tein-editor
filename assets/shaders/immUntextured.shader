#version 130

[VertProgram]

void main ()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
}

[FragProgram]

void main ()
{
    gl_FragColor = gl_Color;
}
