/****************************************************************************
**
** Copyright (C) 1991-2007 Heinz van Saanen
**
** This file is part of the the function viewer Zhu3D.
**
** This file may be used under the terms of the GNU General Public
** License version 2 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


/****************************************************************************
**
** Most of the code for the Marching Cubes and Tetrahedras and the tables are
** from an exemplary implementation of Cory Gene Bloyd. In a first step I
** made just minor adoptions like reformatting, interfacing or optimizations
** in the code. Later on I made it reentrant and thread-safe with some more
** adaptions.
**
** Thanks to Cory Gene Bloyd for publishing this excellent and understandable
** piece of software. For a description of the original algorithm look at:
**
** http://astronomy.swin.edu.au/pbourke/modelling/polygonise/
**
****************************************************************************/


#ifndef ISO_H
#define ISO_H

#include <QList>
#include <QThread>
#include <cmath>
#include <GL/gl.h>
#include "property.h"
#include "pinterface.h"
#include "debug.h"


// Vector structure
struct GLvector {
	float fX;
	float fY;
	float fZ;
} __attribute__((aligned(16)));


// Triangle structure
struct triangle {
	GLvector p0;
	GLvector p1;
	GLvector p2;
};


// Does isosurface tessellation. Called from gldraw only
class ISOGEN : public QThread {

public:

	// General stuff
	ISOGEN(QObject *parent = 0) : QThread(parent) {};
	~ISOGEN() {};

	// Vertex/normal-lists
	QList<triangle> normList;
	QList<triangle> vertList;

	// Central functions
	void vMarching(const long pfrom, const long pto, const long n);

protected:
	void run();

private:
	float fStepSize;		// Grid density == 2.0f/sta.tgrids;
	long totTri;			// Triangles calculated in total; currently not used
	long from, to;			// Grid-distribution for multithreading

	// Constants/tables
	static const float fTargetValue;
	static const float a2fVertexOffset[8][3];
	static const long  a2iEdgeConnection[12][2];
	static const float a2fEdgeDirection[12][3];
	static const long  a2iTetrahedronEdgeConnection[6][2];
	static const long  a2iTetrahedronsInACube[6][4];
	static const long  aiTetrahedronEdgeFlags[16];
	static const long  a2iTetrahedronTriangles[16][7];
	static const long  aiCubeEdgeFlags[256];
	static const long  a2iTriangleConnectionTable[256][16];

	// Functions
	void vNormalizeVector(GLvector &rfVectorResult, const GLvector &rfVectorSource);
	void vGetNormal(GLvector &rfNormal, const float fX, const float fY, const float fZ);
	void vMarchCube1(const float fX, const float fY, const float fZ);
	void vMarchCube2(const float fX, const float fY, const float fZ);
	void vMarchTetrahedron(GLvector *pasTetrahedronPosition, const float *pafTetrahedronValue);
	void (ISOGEN::*tessellation)(const float fX, const float fY, const float fZ);

	// Multithreading stuff:
	// Using own class-instance of standard iso-parsers and own calc-function
	// Copies of Warp-instances are not reentrant/threadsafe by itself
	FunctionParser isoInstance;
	float isoPar(const float xpar, const float ypar, const float zpar);
};

// ISO_H
#endif
