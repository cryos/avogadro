#ifndef VRMLPAINTER_H
#define VRMLPAINTER_H

#include <avogadro/global.h>
#include <avogadro/painter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/glwidget.h>

#include "vrmldialog.h"

class QFile;
class QTextStream;

using namespace Eigen;

namespace Avogadro
{
	// Forward declaration
	class Color;

	class VRMLPainterPrivate;
	class VRMLPainter : public Painter
	{
	public:
		double scale;
		double thinnestCyl;
		double smallestSphere;
		/**
		* Constructor.
		* @param quality defaults to -1, valid range from 0 to 4.
		*/
		VRMLPainter();

		/**
		* Destructor
		*/
		~VRMLPainter();
		int quality() const { return 4; };


		void setName(const Primitive *) { }


		void setName(Primitive::Type, int) { }


		void setColor(const Color *color);


		void setColor(const QColor *color);


		void setColor(QString name);


		void setColor(float red, float green, float blue, float alpha = 1.0);


		void setPlaneNormal(Vector3d planeNormalVector);


		void drawSphere(const Vector3d &center, double radius);


		void drawCylinder(const Vector3d &end1, const Vector3d &end2,
			double radius);


		void drawMultiCylinder(const Vector3d &end1, const Vector3d &end2,
			double radius, int order, double shift);


		void drawCone(const Eigen::Vector3d &, const Eigen::Vector3d &,
			double, double) { }


		void drawLine(const Eigen::Vector3d &, const Eigen::Vector3d &,
			double) { }


		void drawMultiLine(const Eigen::Vector3d &, const Eigen::Vector3d &,
			double, int, short) { }


		void drawTriangle(const Eigen::Vector3d &, const Eigen::Vector3d &,
			const Eigen::Vector3d &) { }


		void drawTriangle(const Eigen::Vector3d &, const Eigen::Vector3d &,
			const Eigen::Vector3d &, const Eigen::Vector3d &) { }


		void drawSpline(const QVector<Eigen::Vector3d>&, double) { }


		void drawShadedSector(const Eigen::Vector3d &  origin, const Eigen::Vector3d & direction1,
			const Eigen::Vector3d & direction2, double radius,
			bool alternateAngle = false);


		void drawArc(const Eigen::Vector3d & origin, const Eigen::Vector3d & direction1,
			const Eigen::Vector3d & direction2,
			double radius, double lineWidth, bool alternateAngle = false);


		void drawShadedQuadrilateral(const Eigen::Vector3d & point1,
			const Eigen::Vector3d & point2,
			const Eigen::Vector3d & point3,
			const Eigen::Vector3d & point4);


		void drawQuadrilateral(const Eigen::Vector3d & point1,
			const Eigen::Vector3d & point2,
			const Eigen::Vector3d & point3,
			const Eigen::Vector3d & point4,
			double lineWidth);


		void drawMesh(const Mesh & mesh, int mode = 0);


		void drawColorMesh(const Mesh & mesh, int mode = 0);

		int drawText(int x, int y, const QString &string);
		int drawText(const QPoint& pos, const QString &string);
		int drawText(const Vector3d & pos, const QString &string);
		int drawText(const Vector3d & pos, const QString &string, const QFont &font);


		void drawBox(const Eigen::Vector3d &corner1,
			const Eigen::Vector3d &corner2);


		void drawTorus(const Eigen::Vector3d &pos,
			double majorRadius, double minorRadius);


		void drawEllipsoid(const Eigen::Vector3d &position,
			const Eigen::Matrix3d &matrix);




		void begin(QTextStream *output, Vector3d planeNormalVector);
		void end();

	private:
		VRMLPainterPrivate * const d;
	};

	class VRMLPainterDevice : public PainterDevice
	{
	public:
		VRMLPainterDevice(const QString& filename, const GLWidget* glwidget, const double scale, VRMLDialog* m_VRMLDialog);
		~VRMLPainterDevice();

		void initializeVRML();
		void render();

		Painter *painter() const { return m_painter; }
		Camera *camera() const { return m_glwidget->camera(); }
		bool isSelected(const Primitive *p) const { return m_glwidget->isSelected(p); }
		double radius(const Primitive *p) const { return m_glwidget->radius(p); }
		const Molecule *molecule() const { return m_glwidget->molecule(); }
		Color *colorMap() const { return m_glwidget->colorMap(); }

		int width() { return m_glwidget->width(); }
		int height() { return m_glwidget->height(); }

	private:
		const GLWidget *m_glwidget;
		QList<Engine *> m_engines;
		VRMLPainter *m_painter;
		QFile *m_file;
		QTextStream *m_output;
		double m_aspectRatio;
	};
}
#endif