// X-ray shader
// Opacity is proportional to angle between view vector and surface normal.
// Fragment whose dNormal is less than a specified threshold are discarded; i.e.
// flat areas facing the viewer are completely transparent

varying vec3 ViewDirection;
varying vec3 Normal;

uniform float minOpacity;
uniform float maxOpacity;
uniform vec3 color;
uniform float da;

void main(void)
{
   float opacity = 1.0 - abs( dot( ViewDirection, Normal ) );
   opacity = clamp( opacity, minOpacity, maxOpacity );
   if( opacity < da ) discard; 
   gl_FragColor = vec4( color, opacity );
}
