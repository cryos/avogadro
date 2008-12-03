// Taken from Molekel trunk 3 December 2008
uniform float xmin;
uniform float xmax;
uniform float ymin;
uniform float ymax;
uniform float zmin;
uniform float zmax;

varying vec4 color;
varying vec3 normal;
varying vec4 pos;


vec3 lightDir = vec3( 0., 0., 1. );

void main()
{
  if( pos.x < xmin || pos.x > xmax ||
          pos.y < ymin || pos.y > ymax ||
          pos.z < zmin || pos.z > zmax ) discard;

  const float kd = abs( dot( normalize( lightDir ), normal ) );
  gl_FragColor = vec4( color.rgb * kd, color.a );

}

