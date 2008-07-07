namespace Avogadro {

class Painter : QObject
{
%TypeHeaderCode
#include <avogadro/painter.h>
%End

  public:
    Painter();
    ~Painter();
    virtual int quality() const = 0;
    virtual void setName (const Avogadro::Primitive *primitive) = 0;
    virtual void setName (Avogadro::Primitive::Type type, int id) = 0;
    virtual void setColor (const Avogadro::Color *color) = 0;
    virtual void setColor (float red, float green, float blue, float alpha = 1.0) = 0;
    virtual void drawSphere (const Eigen::Vector3d & center, double radius) = 0;
    virtual void drawCylinder (const Eigen::Vector3d &end1,
                               const Eigen::Vector3d &end2,
                               double radius) = 0;
    virtual void drawMultiCylinder (const Eigen::Vector3d &end1,
                                    const Eigen::Vector3d &end2,
                                    double radius, int order, double shift) = 0;
    virtual void drawCone(const Eigen::Vector3d &base,
                          const Eigen::Vector3d &tip,
                          double radius) = 0;
    virtual void drawLine(const Eigen::Vector3d &start,
                          const Eigen::Vector3d &end,
                          double lineWidth) = 0;
    virtual void drawMultiLine(const Eigen::Vector3d &start,
                               const Eigen::Vector3d &end, double lineWidth,
                               int order, short stipple) = 0;
    virtual void drawTriangle(const Eigen::Vector3d &p1,
                              const Eigen::Vector3d &p2,
                              const Eigen::Vector3d &p3) = 0;
    virtual void drawTriangle(const Eigen::Vector3d &p1,
                              const Eigen::Vector3d &p2,
                              const Eigen::Vector3d &p3,
                              const Eigen::Vector3d &n) = 0;
    virtual void drawSpline(const QVector<Eigen::Vector3d>& pts,
                            double radius) = 0;
    virtual void drawShadedSector(const Eigen::Vector3d & origin,
                                  const Eigen::Vector3d & direction1,
                                  const Eigen::Vector3d & direction2, double radius,
                                  bool alternateAngle = false) = 0;
    virtual void drawArc(const Eigen::Vector3d & origin, const Eigen::Vector3d & direction1,
                         const Eigen::Vector3d & direction2, double radius,
                         double lineWidth, bool alternateAngle = false) = 0;
    virtual void drawShadedQuadrilateral(const Eigen::Vector3d & point1,
                                         const Eigen::Vector3d & point2,
                                         const Eigen::Vector3d & point3,
                                         const Eigen::Vector3d & point4) = 0;
    virtual void drawQuadrilateral(const Eigen::Vector3d & point1,
                                   const Eigen::Vector3d & point2,
                                   const Eigen::Vector3d & point3,
                                   const Eigen::Vector3d & point4,
                                   double lineWidth) = 0;
    virtual int drawText (int x, int y, const QString &string) const = 0;
    virtual int drawText (const QPoint& pos, const QString &string) const = 0;
    virtual int drawText (const Eigen::Vector3d & pos, const QString &string) const = 0;
};

}; // end namespace Avogadro
