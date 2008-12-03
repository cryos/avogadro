// wwlk
//
// Show me the normal

uniform vec4 my01H2; // please load with (0.0,1.0,0.5,2.0)
varying vec3 v_EyeNormal;
varying vec4 v_EyePosition;

uniform float time;
uniform vec3 lightDirection;
uniform vec3 blue;
uniform vec3 white;

uniform sampler3D tex1;
uniform sampler2D tex2;

void main (void)
{

	vec3 NNormal;
	vec4 MyColor;
	vec4 tex, tx2; 
	vec3 color;
	float turb;
	float illum;

	vec4 Intensity;
	float myHalf = 0.5;

//
//	Since the EyeNormal is getting interpolated, we
//	have to first restore it by normalizing it.
//
	NNormal = normalize( v_EyeNormal );
	illum = clamp( dot(NNormal, lightDirection), 0.0, 1.0) + 0.2;

//  Fetch two samples from a texture with 4 noise octaves
    tex = texture3D( tex1, gl_TexCoord[0].stp);
    tx2 = texture3D( tex1, gl_TexCoord[0].stp*16.0+0.5);

//  Remamp noise from [0,1] to [-1,1]
	tex = tex *2.0 -1.0;
    tx2 = tx2 *2.0 -1.0;

//accumulate turbulence
    turb = tex.r + myHalf*(tex.g + myHalf*(tex.b + myHalf*tex.a));
    turb += 0.0625*(tx2.r + myHalf*(tx2.g+ myHalf*tx2.b));

//remap the turbulence value and use it to index a color spline stored as a texture
    turb = clamp( turb*2.0 , 0.0, 1.0);
	vec2 coord = vec2(turb);
    vec4 c = texture2D( tex2, coord);
    color = vec3(c) * illum;


	gl_FragColor = vec4( color, 1.0);

}