#include "config.h"

#include "vrmlpainter.h"
#include "vrmldialog.h"

#include <avogadro/color.h>
#include <avogadro/engine.h>
#include <avogadro/camera.h>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/mesh.h>
#include <avogadro/color3f.h>

#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QTemporaryFile>
#include <Eigen/Geometry>

#include <iomanip>
#include <iostream>

namespace Avogadro
{
	class VRMLPainterPrivate
	{
	public:
		VRMLPainterPrivate() : pd(0), initialized(false), sharing(0),
			color(0), output(0), planeNormalVector(0., 0., 0.)
		{
			color.setFromRgba(0., 0., 0., 0.);
		}
		~VRMLPainterPrivate()
		{
		}

		VRMLPainterDevice *pd;

		bool initialized;

		inline bool isValid();

		int sharing;

		Color color;
		QTextStream *output;
		Vector3d planeNormalVector;
	};

	VRMLPainter::VRMLPainter() : d(new VRMLPainterPrivate)
	{

	}

	VRMLPainter::~VRMLPainter()
	{
		delete d;
	}


	void VRMLPainter::setColor(const Color *color)
	{
		d->color.setFromRgba(color->red(), color->green(), color->blue(),
			color->alpha());
	}

	void VRMLPainter::setColor(const QColor *color)
	{
		d->color.setFromRgba(color->redF(), color->greenF(), color->blueF(),
			color->alphaF());
	}

	void VRMLPainter::setColor(float red, float green, float blue, float alpha)
	{
		d->color.setFromRgba(red, green, blue, alpha);
	}

	void VRMLPainter::setColor(QString name)
	{
		d->color.setFromQColor(QColor(name));
	}

	void VRMLPainter::setPlaneNormal(Vector3d planeNormalVector)
	{
		// Set the plane normal, multiCylinders are drawn relative to this
		d->planeNormalVector = planeNormalVector;
	}

	void VRMLPainter::drawSphere(const Vector3d &center, double radius)
	{
		double x = center.x() * this->scale;
		double y = center.y() * this->scale;
		double z = center.z() * this->scale;
		double r = radius * this->scale; //from py file
		if (r < this->smallestSphere) {
			this->smallestSphere = r*2; //keep track of the diameter
		}
		// Write out a VRMLRay sphere for rendering
		*(d->output) << "Transform {\n"
			<< "\ttranslation\t" << x << "\t" << y << "\t" << z
			<< "\n\tchildren Shape {\n"
			<< "\t\tgeometry Sphere {\n\t\t\tradius\t" << r << "\n\t\t}\n"
			<< "\t\tappearance Appearance {\n"
			<< "\t\t\tmaterial Material {\n"
			<< "\t\t\t\tdiffuseColor\t" << d->color.red() << "\t" << d->color.green()
			<< "\t" << d->color.blue() << "\n\t\t\t}\n\t\t}\n\t}\n}\n";
	}

	void VRMLPainter::drawCylinder(const Vector3d &end1, const Vector3d &end2,
		double radius)
	{
		double x1, x2, y1, y2, z1, z2;
		x1 = end1.x();
		x2 = end2.x();
		y1 = end1.y();
		y2 = end2.y();
		z1 = end1.z();
		z2 = end2.z();

		double dx = x2 - x1;
		double dy = y2 - y1;
		double dz = z2 - z1;

		double length = sqrt(dx*dx + dy*dy + dz*dz);
		double tx = dx / 2 + x1;
		double ty = dy / 2 + y1;
		double tz = dz / 2 + z1;

		dx = dx / length;
		dy = dy / length;
		dz = dz / length;

		double ax, ay, az, angle;

		if (dy > 0.999) {
			ax = 1.0;
			ay = 0.0;
			az = 0.0;
			angle = 0.0;
		}
		else if (dy < -0.999) {
			ax = 1.0;
			ay = 0.0;
			az = 0.0;
			angle = 3.14159265359;
		}
		else {
			ax = dz;
			ay = 0.0;
			az = dx * -1.0;
			angle = acos(dy);
		}
		length = length / 2.0;

		if ((radius*this->scale) < this->thinnestCyl) //keep track of the thinnest cylinder
			this->thinnestCyl = radius*this->scale*2; //diameter

		*(d->output) << "Transform {\n"
			<< "\ttranslation\t" << tx*this->scale << "\t" << ty*this->scale << "\t" << tz*this->scale
			<< "\n\tscale " << " 1 " << length*this->scale << " 1"
			<< "\n\trotation " << ax << " " << ay << " " << az << " " << angle
			<< "\n\tchildren Shape {\n"
			<< "\t\tgeometry Cylinder {\n\t\t\tradius\t" << radius*this->scale << "\n\t\t}\n"
			<< "\t\tappearance Appearance {\n"
			<< "\t\t\tmaterial Material {\n"
			<< "\t\t\t\tdiffuseColor\t" << d->color.red() << "\t" << d->color.green()
			<< "\t" << d->color.blue() << "\n\t\t\t}\n\t\t}\n\t}\n}\n";
	}

	void VRMLPainter::drawMultiCylinder(const Vector3d &end1, const Vector3d &end2,
		double radius, int order, double)
	{
		if (order == 1)
			drawCylinder(end1, end2, radius);

		else {
			//taken from povpainter.cpp
			// Find the bond axis
			Vector3d axis = end2 - end1;
			double axisNorm = axis.norm();
			if (axisNorm < 1.0e-5) return;
			Vector3d axisNormalized = axis / axisNorm;

			// Use the plane normal vector for the molecule to draw multicylinders along
			Vector3d ortho1 = axisNormalized.cross(d->planeNormalVector);
			double ortho1Norm = ortho1.norm();
			if (ortho1Norm > 0.001) ortho1 /= ortho1Norm;
			else ortho1 = axisNormalized.unitOrthogonal();
			// This number seems to work well for drawing the multiCylinder inside
			ortho1 *= radius*1.5;
			Vector3d ortho2 = axisNormalized.cross(ortho1);
			// Use an angle offset of zero for double bonds, 90 for triple and 22.5 for higher order
			double angleOffset = 0.0;
			if (order >= 3) {
				if (order == 3)
					angleOffset = 90.0;
				else
					angleOffset = 22.5;
			}
			// Actually draw the cylinders
			for (int i = 0; i < order; ++i) {
				double alpha = angleOffset / 180.0 * M_PI + 2.0 * M_PI * i / order;
				Vector3d displacement = cos(alpha) * ortho1 + sin(alpha) * ortho2;
				Vector3d displacedEnd1 = end1 + displacement;
				Vector3d displacedEnd2 = end2 + displacement;

				drawCylinder(displacedEnd1, displacedEnd2, radius);

			}
		}
	}

	void VRMLPainter::drawShadedSector(const Eigen::Vector3d &,
		const Eigen::Vector3d &,
		const Eigen::Vector3d &, double, bool)
	{
	}

	void VRMLPainter::drawArc(const Eigen::Vector3d &, const Eigen::Vector3d &,
		const Eigen::Vector3d &, double, double, bool)
	{
	}

	void VRMLPainter::drawShadedQuadrilateral(const Eigen::Vector3d &,
		const Eigen::Vector3d &,
		const Eigen::Vector3d &,
		const Eigen::Vector3d &)
	{
	}

	void VRMLPainter::drawQuadrilateral(const Eigen::Vector3d &,
		const Eigen::Vector3d &,
		const Eigen::Vector3d &,
		const Eigen::Vector3d &, double)
	{
	}

	void VRMLPainter::drawMesh(const Mesh & mesh, int mode)
	{
		std::vector<Eigen::Vector3f> t = mesh.vertices();
		std::vector<Eigen::Vector3f> n = mesh.normals();
		std::vector<Color3f> c;

		//take color from d->color
		for (unsigned int j = 0; j < t.size(); j++) {
			Color3f new_col;
			new_col.set(d->color.red(), d->color.green(), d->color.blue());
			c.push_back(new_col);
		}

		// If there are no triangles then don't bother doing anything
		if (t.size() == 0 || t.size() != c.size())
			return;

		QString vertsStr, ivertsStr, colorStr;
		QTextStream verts(&vertsStr);
		QTextStream iverts(&ivertsStr);
		QTextStream colors(&colorStr);

		for (unsigned int i = 0; i < t.size(); ++i) {
			if (i == t.size() - 1) {
				verts << t[i].x()*this->scale << " " << t[i].y()*this->scale << " " << t[i].z()*this->scale;
				colors << c[i].red() << " " << c[i].green() << " " << c[i].blue();
				break;
			}
			verts << t[i].x()*this->scale << " " << t[i].y()*this->scale << " " << t[i].z()*this->scale << ",\n";
			colors << c[i].red() << " " << c[i].green() << " " << c[i].blue() << ", ";
		}
		// Now to write out the indices
		for (unsigned int i = 0; i < t.size(); i += 3) {
			iverts << i << ", " << i + 1 << ", " << i + 2 << ", -1,\n";

		}

		// Now to write out the full mesh - could be pretty big...
		*(d->output) << "Shape {\n"
			<< "\tgeometry IndexedFaceSet {\n"
			<< "\t\tcoord Coordinate {\n"
			<< "\t\t\tpoint ["
			<< vertsStr << "\t\t\t]\n\t\t}\n"
			<< "\t\tcoordIndex["
			<< ivertsStr << "\t\t\t]\n"
			<< "color Color {\n color ["
			<< colorStr << "]\n}\n}\n}";
	}

	void VRMLPainter::drawColorMesh(const Mesh & mesh, int mode)
	{
		std::vector<Eigen::Vector3f> t = mesh.vertices();
		std::vector<Eigen::Vector3f> n = mesh.normals();
		std::vector<Color3f> c = mesh.colors();

		// If there are no triangles then don't bother doing anything
		if (t.size() == 0 || t.size() != c.size())
			return;

		QString vertsStr, ivertsStr, colorStr;
		QTextStream verts(&vertsStr);
		QTextStream iverts(&ivertsStr);
		QTextStream colors(&colorStr);

		for (unsigned int i = 0; i < t.size(); ++i) {
			if (i == t.size() - 1) {
				verts << t[i].x()*this->scale << " " << t[i].y()*this->scale << " " << t[i].z()*this->scale;
				colors << c[i].red() << " " << c[i].green() << " " << c[i].blue();
				break;
			}
			verts << t[i].x()*this->scale << " " << t[i].y()*this->scale << " " << t[i].z()*this->scale << ",\n";
			colors << c[i].red() << " " << c[i].green() << " " << c[i].blue() << ", ";
		}
		// Now to write out the indices
		for (unsigned int i = 0; i < t.size(); i += 3) {
			iverts << i << ", " << i + 1 << ", " << i + 2 << ", -1,\n";

		}

		// Now to write out the full mesh - could be pretty big...
		*(d->output) << "Shape {\n"
			<< "\tgeometry IndexedFaceSet {\n"
			<< "\t\tcoord Coordinate {\n"
			<< "\t\t\tpoint ["
			<< vertsStr << "\t\t\t]\n\t\t}\n"
			<< "\t\tcoordIndex["
			<< ivertsStr << "\t\t\t]\n"
			<< "color Color {\n color ["
			<< colorStr << "]\n}\n}\n}";

	}

	int VRMLPainter::drawText(int, int, const QString &)
	{
		return 0;
	}

	int VRMLPainter::drawText(const QPoint&, const QString &)
	{
		return 0;
	}

	int VRMLPainter::drawText(const Vector3d &, const QString &)
	{
		return 0;
	}

	int VRMLPainter::drawText(const Vector3d &, const QString &, const QFont &)
	{
		return 0;
	}

	void VRMLPainter::drawBox(const Eigen::Vector3d &,
		const Eigen::Vector3d &)
	{
	}

	void VRMLPainter::drawTorus(const Eigen::Vector3d &,
		double, double)
	{
	}

	void VRMLPainter::drawEllipsoid(const Eigen::Vector3d &,
		const Eigen::Matrix3d &)
	{
	}
	void VRMLPainter::begin(QTextStream *output, Vector3d planeNormalVector)
	{
		d->output = output;
		d->planeNormalVector = planeNormalVector;
	}

	void VRMLPainter::end()
	{
		d->output = 0;
	}



	VRMLPainterDevice::VRMLPainterDevice(const QString& filename,
		const GLWidget* glwidget,const double scale, VRMLDialog* m_VRMLDialog)
	{
		m_output = 0;
		m_glwidget = glwidget;

		m_painter = new VRMLPainter;
		m_painter->scale = scale;
		m_painter->thinnestCyl = std::numeric_limits<double>::max(); //keep track of the thinnest cylinder
		m_painter->smallestSphere = std::numeric_limits<double>::max(); //keep track of the smallest sphere
		if (!filename.isEmpty()) {
			m_file = new QFile(filename);
			if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
				return;
			m_output = new QTextStream(m_file);
		}
		else {
			m_file = new QTemporaryFile();
			m_output = new QTextStream();
		}
		m_output->setRealNumberPrecision(5);
		m_painter->begin(m_output, m_glwidget->normalVector());

		m_engines = m_glwidget->engines();

		initializeVRML();
		render();
		m_painter->end();

		m_VRMLDialog->calcVRML(m_painter->thinnestCyl, m_painter->smallestSphere); //send the thinnest cylinder back to the dialog box

		m_file->close();
	}

	VRMLPainterDevice::~VRMLPainterDevice()
	{
		delete m_output;
		m_output = 0;
		delete m_file;
		delete m_painter;
	}

	void VRMLPainterDevice::initializeVRML()
	{
		// Initialise our VRML-Ray scene
		// The VRML-Ray camera basically has the same matrix elements - we just need to translate
		// FIXME Still working on getting the translation to VRML-Ray right...
		Vector3d cameraT = -(m_glwidget->camera()->modelview().linear().adjoint()
			* m_glwidget->camera()->modelview().translation()
			);
		Vector3d cameraX = m_glwidget->camera()->backTransformedXAxis();
		Vector3d cameraY = m_glwidget->camera()->backTransformedYAxis();
		Vector3d cameraZ = -m_glwidget->camera()->backTransformedZAxis();

		double huge;
		if (m_glwidget->farthestAtom()) {
			huge = 10 * m_glwidget->farthestAtom()->pos()->norm();
		}
		else {
			huge = 10;
		}

		Vector3d light0pos = huge * (m_glwidget->camera()->modelview().linear().adjoint()
			* Vector3d(LIGHT0_POSITION[0],
				LIGHT0_POSITION[1],
				LIGHT0_POSITION[2]));
		Vector3d light1pos = huge * (m_glwidget->camera()->modelview().linear().adjoint()
			* Vector3d(LIGHT1_POSITION[0],
				LIGHT1_POSITION[1],
				LIGHT1_POSITION[2]));

		// Output the VRML-Ray initialisation code
		*(m_output) << "#VRML V2.0 utf8\n"
			<< "WorldInfo {\n"
			<< "title "
			<< "\"test\"\n" //NEED TITLE
			<< "info [ \"By Avogadro\" ]\n}\n"

			<< "NavigationInfo {\n"
			<< "type [\"EXAMINE\",\"ANY\"]\n"
			<< "headlight TRUE\n"
			<< "visibilityLimit 0.0\n"
			<< "speed 1.0\n}\n"

			//TODO
			<< "DEF DefaultView Viewpoint {\n"
			<< "position 0 0 57.558\n"
			<< "description \"Default view\"\n"
			<< "fieldOfView 0.785398\n}\n";

	}

	void VRMLPainterDevice::render()
	{

		// Now render the scene using the active engines
		foreach(Engine *engine, m_engines) {
			if (engine->isEnabled()) {
				// Use unions for opaque objects - they are faster

				engine->renderOpaque(this);

			}
			if (engine->isEnabled() && engine->layers() & Engine::Transparent) {
				// Use merge for transparent objects, slower but more correct

				engine->renderTransparent(this);

			}
		}
	}
}
