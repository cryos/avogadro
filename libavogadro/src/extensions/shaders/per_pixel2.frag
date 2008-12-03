varying vec3 Normal;
varying vec3 Light;
varying vec3 HalfVector;


// max(abs( dot(n, Light) ),0.0) --> abs( dot(n, Light) ) to fix an issue with OpenMOIV 

void main(void)
{
  vec3 n = normalize(Normal);

  vec4 Diffuse = ( gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse ) * abs( dot(n, Light) );

  vec4 Ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  Ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;

  vec4 Specular = ( gl_FrontMaterial.specular * gl_LightSource[0].specular ) * pow(abs(dot(n,HalfVector)), gl_FrontMaterial.shininess );

  gl_FragColor = Ambient + Diffuse + Specular;
}