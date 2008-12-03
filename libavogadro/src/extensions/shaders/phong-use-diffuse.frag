// wwlk
//
// Do one per fragment diffuse light
// + four specular lights
// + four more specular lights
// 8 specular lights per fragment!

const float Shininess = 40.0;


void main (void)
{


//
//	vec3 EyeNormal	= vec3(gl_TexCoord0);
//
//	vec3 ULLight	= vec3(gl_TexCoord1);
//	vec3 LRLight	= vec3(gl_TexCoord2);
//	vec3 RimLight	= vec3(gl_TexCoord3);
//
//	vec4 DifColor   = vec4(gl_TexCoord4);
//	vec4 SpecColor	= vec4(gl_TexCoord5);
//	vec4 ULColor    = vec4(gl_TexCoord6);
//  vec4 LRColor	= vec4(gl_TexCoord7);
//

	vec3 NNormal;
	vec3 LNormal;
	vec4 MyColor;
	vec4 SpecularColor;

	float Intensity;

//
//	Since the normal is getting interpolated, we
//	have to first restore it by normalizing it.
//
//	NNormal = normalize( EyeNormal );
	NNormal = normalize( vec3(gl_TexCoord[0]) );

//
//	Per fragment diffuse lighting
//	We are going to put the diffuse light about
//	the right spot....


//////	Intensity = dot ( ULLight, NNormal );
	Intensity = abs( dot (vec3(gl_TexCoord[1]), NNormal ) );
	Intensity = max ( Intensity, 0.0 );

//////	MyColor =  vec4(Intensity) * DifColor;
	MyColor =  vec4(Intensity) * gl_TexCoord[4];


//
//	We are going to point the normal to the
//	upper right front all of the time by taking
//	the abs.  No need to normalize again.
//
	NNormal = abs(NNormal);

//
//	Now, start the specular calculations.
//

//////	Intensity = dot( NNormal, LRLight );
	Intensity = abs( dot( NNormal, vec3( gl_TexCoord[1] ) ) );
	Intensity = max( Intensity, 0.0 );
	Intensity = pow( Intensity, Shininess );

//////	MyColor += vec4(Intensity) * ULColor;
	MyColor += vec4(Intensity) * gl_TexCoord[6];



//////	Intensity = dot( NNormal, LRLight );
	Intensity = dot( NNormal, vec3( gl_TexCoord[2] ) );
	Intensity = max( Intensity, 0.0 );
	Intensity = pow( Intensity, Shininess );


//////	MyColor += vec4(Intensity) * ULColor;
	MyColor += vec4(Intensity) * gl_TexCoord[7];


//
//	Squash the Y of the normal vector
//
	NNormal.y = 0.0;

	NNormal = normalize( NNormal );


//////	Intensity = dot( NNormal, RimLight );
	Intensity = dot( NNormal, vec3( gl_TexCoord[3] ) );
//////	Intensity = max( Intensity, 0.0 );
	Intensity = pow( Intensity, Shininess );


//////	MyColor += vec4(Intensity) * SpecColor;
	MyColor += vec4(Intensity) * gl_TexCoord[5];


//////	MyColor.a = Difcolor.a;
	MyColor.a = 1.0;//gl_TexCoord[4].a;
	
	gl_FragColor = MyColor;

}
