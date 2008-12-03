// wwlk
//
// Trivial Vertex shader to setup per fragment lighting
//

vec4 ULLight = vec4 ( 0.435028 , 0.602347 , 0.669274 , 0.0 );
vec4 LRLight = vec4 ( 0.894427 , 0.0      , 0.447214 , 0.0 );
vec4  RimLight = vec4( 0.94868, 0.0,  0.316228, 1.0 );

vec4  DifColor = vec4( 0.35, 0.35, 0.5, 1.0 );
vec4  SpecColor = vec4( 0.4, 0.4, 0.2, 1.0 );
vec4  ULColor = vec4( 0.5, 0.3875, 0.75, 1.0 );
vec4  LRColor = vec4( 0.75, 0.5, 0.5, 1.0 );


void main(void)
{ 
	vec3 eyeNormal;
	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	eyeNormal = gl_NormalMatrix * gl_Normal;

    gl_TexCoord[0] = vec4( eyeNormal, 0.0 );

	gl_TexCoord[1] = ULLight;
	gl_TexCoord[2] = LRLight;
	gl_TexCoord[3] = RimLight;

	gl_TexCoord[4] = DifColor;
	gl_TexCoord[5] = SpecColor;
	gl_TexCoord[6] = ULColor;
	gl_TexCoord[7] = LRColor;

}