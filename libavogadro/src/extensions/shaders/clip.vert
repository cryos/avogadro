// Taken from Molekel trunk 3 December 2008
varying vec4 pos;
varying vec3 normal;
varying vec4 color;
uniform float factor;
void main()
{
        pos = gl_Vertex;
        pos.xyz /= gl_Vertex.w;
        pos.xyz *= factor;
        normal = normalize( gl_NormalMatrix * gl_Normal );
        color = gl_Color;
        gl_Position = ftransform();

}

