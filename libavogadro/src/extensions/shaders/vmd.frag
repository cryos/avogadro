/***************************************************************************
 *cr
 *cr            (C) Copyright 1995-2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
/***************************************************************************
 * RCS INFORMATION:
 *
 *      $RCSfile: vmd.frag,v $
 *      $Author: johns $        $Locker:  $             $State: Exp $
 *      $Revision: 1.49 $       $Date: 2007/03/03 18:10:58 $
 *
 ***************************************************************************
 * DESCRIPTION:
 *  This file contains the VMD OpenGL fragment shader implementing 
 *  per-pixel lighting with phong highlights etc.
 ***************************************************************************/

// requires GLSL version 1.10
#version 110

//
// Fragment shader varying and uniform variable definitions for data 
// supplied by VMD and/or the vertex shader
//
varying vec3 oglnormal;       // interpolated normal from the vertex shader
varying vec3 oglcolor;        // interpolated color from the vertex shader
varying vec3 V;               // view direction vector
uniform vec3 vmdlight0;       // VMD directional lights
uniform vec3 vmdlight1;
uniform vec3 vmdlight2;
uniform vec3 vmdlight3;

uniform vec3 vmdlight0H;      // Blinn halfway vectors for all four lights
uniform vec3 vmdlight1H;
uniform vec3 vmdlight2H;
uniform vec3 vmdlight3H;

uniform vec4 vmdlightscale;   // VMD light on/off state for all 4 VMD lights,
                              // represented as a scaling constant.  Could be
                              // done with on/off flags but ATI doesn't deal
                              // well with branching constructs, so this value
                              // is simply multiplied by the light's 
                              // contribution.  Hacky, but it works for now.

uniform vec4 vmdmaterial;     // VMD material properties
                              // [0] is ambient (white ambient light only)
                              // [1] is diffuse
                              // [2] is specular
                              // [3] is shininess

uniform float vmdopacity;     // VMD global alpha value

uniform float vmdfogmode;       // VMD depth cueing / fog mode

uniform float vmdtexturemode;   // VMD texture mode 0=off 1=modulate 2=replace
//uniform sampler3D vmdtex0;    // active 3-D texture map

//
// VMD Fragment Shader
//
void main(void) {
  vec3 texcolor;                    // texture color if needed

  // perform texturing operations for volumetric data start texture
  // fetch as early as possible to hide memory latency
  //if (vmdtexturemode != 0) {
  //  texcolor = vec3(texture3D(vmdtex0, gl_TexCoord[0].xyz));
  //}
  
  // Flip the surface normal if it is facing away from the viewer,
  // determined by polygon winding order provided by OpenGL.
  vec3 N = normalize(oglnormal);
  if (!gl_FrontFacing) {
    N = -N;
  }

  // beginning of shading calculations
  float ambient = vmdmaterial[0];   // ambient
  float diffuse = 0.0;
  float specular = 0.0;
  float shininess = vmdmaterial[3]; // shininess 

  // calculate diffuse lighting contribution
  diffuse += max(0.0, dot(N, vmdlight0)) * vmdlightscale[0];
  diffuse += max(0.0, dot(N, vmdlight1)) * vmdlightscale[1];
  diffuse += max(0.0, dot(N, vmdlight2)) * vmdlightscale[2];
  diffuse += max(0.0, dot(N, vmdlight3)) * vmdlightscale[3];
  diffuse *= vmdmaterial[1]; // diffuse scaling factor

  // calculate specular lighting contribution with Phong highlights, based
  // on Blinn's halfway vector variation of Phong highlights
  specular += pow(max(0.0, dot(N, vmdlight0H)), shininess) * vmdlightscale[0];
  specular += pow(max(0.0, dot(N, vmdlight1H)), shininess) * vmdlightscale[1];
  specular += pow(max(0.0, dot(N, vmdlight2H)), shininess) * vmdlightscale[2];
  specular += pow(max(0.0, dot(N, vmdlight3H)), shininess) * vmdlightscale[3];
  specular *= vmdmaterial[2]; // specular scaling factor

  // Fog computations
  const float Log2E = 1.442695; // = log2(2.718281828)
  float fog = 1.0;
  if (vmdfogmode == 1.0) {
    // linear fog
    fog = (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale;
  } else if (vmdfogmode == 2.0) {
    // exponential fog
    fog = exp2(-gl_Fog.density * gl_FogFragCoord * Log2E);
  } else if (vmdfogmode == 3.0) { 
    // exponential-squared fog
    fog = exp2(-gl_Fog.density * gl_Fog.density * gl_FogFragCoord * gl_FogFragCoord * Log2E);
  }
  fog = clamp(fog, 0.0, 1.0);       // clamp the final fog parameter [0->1)

  vec3 objcolor = oglcolor * vec3(diffuse);         // texturing is disabled
  if (vmdtexturemode == 1.0) {
    objcolor = oglcolor * texcolor * vec3(diffuse); // emulate GL_MODULATE
  } else if (vmdtexturemode == 2.0) {
    objcolor = texcolor;                            // emulate GL_REPLACE
  } 

  vec3 color = objcolor + vec3(ambient + specular);
  gl_FragColor = vec4(mix(vec3(gl_Fog.color), color, fog), vmdopacity);
}


