varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;

void main(void)
{
Normal = normalize(gl_NormalMatrix * gl_Normal);

Light = normalize(gl_LightSource[0].position.xyz);

HalfVector = normalize(gl_LightSource[0].halfVector.xyz);

gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}