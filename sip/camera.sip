namespace Avogadro {
  
class Camera
{
%TypeHeaderCode
#include <avogadro/camera.h>
%End
    
    protected:
      void setParent(const Avogadro::GLWidget *glwidget);

    public:
      explicit Camera(const Avogadro::GLWidget *parent = 0, double angleOfViewY = 40.0);
      virtual ~Camera();
      Camera(const Avogadro::Camera *camera);
      const Avogadro::GLWidget *parent() const;
      void setAngleOfViewY(double angleOfViewY);
      double angleOfViewY() const;
      void setModelview(const Eigen::Transform3d &matrix);
      // const Eigen::Transform3d & modelview() const;
      Eigen::Transform3d & modelview();
      void applyPerspective() const;
      void applyModelview() const;
      void initializeViewPoint();
      double distance(const Eigen::Vector3d & point) const;
      void translate(const Eigen::Vector3d &vector);
      void pretranslate(const Eigen::Vector3d &vector);
      void rotate(const double &angle, const Eigen::Vector3d &axis);
      void prerotate(const double &angle, const Eigen::Vector3d &axis);
      Eigen::Vector3d unProject(const Eigen::Vector3d& v) const;
      Eigen::Vector3d unProject(const QPoint& p, const Eigen::Vector3d& ref) const;
      Eigen::Vector3d unProject(const QPoint& p) const;
      Eigen::Vector3d project(const Eigen::Vector3d& v) const;
      Eigen::Vector3d backTransformedXAxis() const;
      Eigen::Vector3d backTransformedYAxis() const;
      Eigen::Vector3d backTransformedZAxis() const;
      Eigen::Vector3d transformedXAxis() const;
      Eigen::Vector3d transformedYAxis() const;
      Eigen::Vector3d transformedZAxis() const;
      void normalize();

};

}; // end namespace Avogadro
