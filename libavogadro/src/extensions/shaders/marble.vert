//
// marble.vert
//
//  Transform the object position and normal to provide to the fragment
// shader for lighting. Also, scle the object space coords, to use as
// texture coordinates into the volume.

varying vec3 v_EyeNormal;
varying vec4 v_EyePos;



uniform vec4 my01H2;			// please load with ( 0.0, 1.0, 0.5, 2.0)
uniform vec4 myObjScale;		// Object Scale Factor

uniform float time;
void main(void)
{ 
	vec4 ObjNormal;
	vec4 ObjPos;
	vec3 EyeNormal;
	vec4 EyePos; 

	ObjPos = gl_Vertex;

	//	Must write gl_Position for rasterization to be defined....
	//
	gl_Position = gl_ModelViewProjectionMatrix * ObjPos; 

	//	Transform to shading space (we are going to shade in eyespace)
	//
	//
	EyeNormal = gl_Normal* gl_NormalMatrix;
	v_EyeNormal = EyeNormal; 

	EyePos = gl_ModelViewMatrix * ObjPos;
	v_EyePos = EyePos;

	//	Pass through object normal and object position
	//
	ObjNormal = vec4(gl_Normal, my01H2.x); 


	gl_TexCoord[0] = ObjPos* 0.0078125 + 0.5;
}