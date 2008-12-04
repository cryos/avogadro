// X-ray shader
// Set eye position , normal and transformed vertex position

varying vec3 ViewDirection;
varying vec3 Normal;

uniform vec3 eyePosition;

void main( void )
{
   // World coordinates.
   vec4 vertexPosition = gl_ModelViewMatrix * gl_Vertex;
   // View Direction.
   ViewDirection  = normalize( eyePosition - vertexPosition.xyz );
   // Vertex Normal.
   Normal         = gl_NormalMatrix * gl_Normal;
   // Assign transformed vertex coordinates to GLSL variable.
   gl_Position = ftransform();
}