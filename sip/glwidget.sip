namespace Avogadro {

class GLWidget : QGLWidget
{
%TypeHeaderCode
#include <avogadro/glwidget.h>
%End

    public:
      GLWidget(QWidget *parent = 0);
      explicit GLWidget(const QGLFormat &format, QWidget *parent = 0, const Avogadro::GLWidget * shareWidget = 0);
      GLWidget(Avogadro::Molecule *molecule, const QGLFormat &format, QWidget *parent = 0, 
          const Avogadro::GLWidget * shareWidget = 0);
      ~GLWidget();
      //bool isStable() const;
      //void setStable(bool stable);
      int deviceWidth();
      int deviceHeight();
      QSize sizeHint() const;
      QSize minimumSizeHint() const;
      double radius(const Avogadro::Primitive *p) const;
      Avogadro::Tool* tool() const;
      //Avogadro::ToolGroup toolManger() const;
      QColor background() const;
      void setColorMap(Avogadro::Color *);
      Avogadro::Color *colorMap() const;
      void setUndoStack(QUndoStack *undoStack);
      QUndoStack* undoStack() const;
      //  const Avogadro::Molecule* molecule() const;
      Avogadro::Molecule* molecule();
      void updateGeometry();
      Avogadro::Camera * camera() const;
      QList<Avogadro::Engine *> engines() const;
//      QList<Avogadro::EngineFactory *> engineFactories() const;
//      QList<GLHit> hits(int x, int y, int w, int h);
      Avogadro::Primitive* computeClickedPrimitive(const QPoint& p);
      Avogadro::Atom* computeClickedAtom(const QPoint& p);
      Avogadro::Bond* computeClickedBond(const QPoint& p);
      const Eigen::Vector3d & center() const;
      const Eigen::Vector3d & normalVector() const;
//      const double & radius() const;
      const Avogadro::Atom *farthestAtom() const;
      void setQuality(int quality);
      int quality() const;
      void setRenderAxes(bool renderAxes);
      bool renderAxes();
      void setRenderDebug(bool renderDebug);
      bool renderDebug();
      void setToolGroup(Avogadro::ToolGroup *toolGroup);
      Avogadro::ToolGroup * toolGroup() const;
      Avogadro::Painter *painter() const;
      Avogadro::PrimitiveList primitives() const;
      Avogadro::PrimitiveList selectedPrimitives() const;
      void toggleSelected(Avogadro::PrimitiveList primitives);
      void setSelected(Avogadro::PrimitiveList primitives, bool select = true);
      void clearSelected();
      bool isSelected(const Avogadro::Primitive *p) const;
      bool addNamedSelection(const QString &name, Avogadro::PrimitiveList &primitives);
      void removeNamedSelection(const QString &name);
      void removeNamedSelection(int index);
      void renameNamedSelection(int index, const QString &name);
      QStringList namedSelections();
      Avogadro::PrimitiveList namedSelectionPrimitives(const QString &name);
      Avogadro::PrimitiveList namedSelectionPrimitives(int index);
      void setUnitCells(int a, int b, int c);
      void clearUnitCell();
      int aCells();
      int bCells();
      int cCells();
      static Avogadro::GLWidget *m_current;
      static Avogadro::GLWidget *current();
      static void setCurrent(Avogadro::GLWidget *current);
      virtual void writeSettings(QSettings &settings) const;
      virtual void readSettings(QSettings &settings);

    protected:
      virtual void initializeGL();
      virtual void paintGL();
      virtual void paintEvent(QPaintEvent *event);
      virtual void resizeEvent(QResizeEvent *event);
      virtual void resizeGL(int, int);
      virtual bool event(QEvent *event);
      virtual void mousePressEvent(QMouseEvent * event);
      virtual void mouseReleaseEvent(QMouseEvent * event);
      virtual void mouseMoveEvent(QMouseEvent * event);
      virtual void wheelEvent(QWheelEvent * event);
      virtual void render();
      virtual void renderCrystal(GLuint displayList);
      virtual void renderCrystalAxes();
      virtual void renderAxesOverlay();
      virtual void renderDebugOverlay();
//      void loadEngineFactories();
//       QWaitCondition *paintCondition() const;


    public slots:
      void setTool(Avogadro::Tool *tool);
      void addPrimitive(Avogadro::Primitive *primitive);
      void updatePrimitive(Avogadro::Primitive *primitive);
      void removePrimitive(Avogadro::Primitive *primitive);
      void setBackground(const QColor &background);
      void setMolecule(Avogadro::Molecule *molecule);
      void addEngine(Avogadro::Engine *engine);
      void removeEngine(Avogadro::Engine *engine);
      void loadDefaultEngines();
      void invalidateDLs();

    signals:
      void mousePress(QMouseEvent * event);
      void mouseRelease( QMouseEvent * event );
      void mouseMove( QMouseEvent * event );
      void wheel( QWheelEvent * event);
      void moleculeChanged(Avogadro::Molecule *previous, Avogadro::Molecule *next);
      void engineAdded(Avogadro::Engine *engine);
      void engineRemoved(Avogadro::Engine *engine);
      void resized();


};

}; // namespace Avogadro
