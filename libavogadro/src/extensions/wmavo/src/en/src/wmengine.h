
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

  Some portions :
  Copyright (C) 2007 Donald Ephraim Curtis

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef WMENGINE_H
#define WMENGINE_H

//#include "ui_wmenginesettingswidget.h"


#ifdef _WIN32
#pragma warning( disable : 4365 ) // conversion from 'x' to 'y', signed/unsigned mismatch
#pragma warning( disable : 4820 ) // 'x' bytes padding added after data member '...'
#pragma warning( disable : 4668 ) // '...' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning( disable : 4514 ) // '...' : unreferenced inline function has been removed
#pragma warning( disable : 4738 ) // storing 32-bit float result in memory, possible loss of performance
#pragma warning( disable : 4710 ) // 'T qvariant_cast<QSet<QAccessible::Method>>(const QVariant &)' : function not inlined

#pragma warning( push, 0 )
#endif

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/camera.h>
#include <avogadro/painter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/color.h>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>

#include <QGLWidget> // for OpenGL bits
#include <QDebug>

//#include <GL/glut.h>
#include <openbabel/mol.h>

#ifdef _WIN32
#pragma warning( pop )
#endif

using namespace std;
using namespace Eigen;

namespace Avogadro {

  class Atom;
  //class WMSettingsWidget;



  /**
    * @class WmEngine
    * @brief Realize an anaglyph (red/cyan) render in Avogadro.
    *
    * In stereo mode, the object is drawn in red for the left eye
    * and cyan for the right eye.  Viewing the scene with red/cyan
    * filter stereo glasses should give a sense of stereo 3D.
    * glColorMask is used to control update of the red and cyan
    * channel. glFrustum is used to setup two different view frustums
    * for each eye based on eye separation.
    */
  class WmEngine : public Engine
  {

    /**
      * @name Object Qt initialization.
      * @{ */
    Q_OBJECT
    AVOGADRO_ENGINE( "Ball and Stick in 3D-stereo",
                     tr("Ball and Stick STEREO"),
                     tr("Renders primitives using Balls (atoms) and Sticks (bonds) in 3D-stereo"))
    // @}

    private :
      /**
        * @name Try to solve doxygen bug with the previous macro
        * Try to solve by add a no used method.
        * This message does not appear in the doc.
        * @{ */
      void solveDoxygenBug(){} ;
        //@}

    //
    // Public methods.
    public:

      /**
        * @name Avogadro default methods
        * @{ */
      WmEngine(QObject *parent=0) ;
      ~WmEngine() ;

      Engine *clone() const;
      Engine::Layers layers() const;

      bool hasSettings() { return false; } // true

      //QWidget *settingsWidget();
          ///< @return a pointer to the WmEngine settings widget

      //void writeSettings(QSettings &settings) const;
          ///< Write the engine settings so that they can be saved between sessions.

      //void readSettings(QSettings &settings);
          ///< Read in the settings that have been saved for the engine instance.

      // @}


      /**
        * @name Rendering methods & Co
        * @{
        */
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      bool renderPick(PainterDevice *pd);

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

      //double transparencyDepth() const;
      // @}


    //
    // Private methods.
    private:


      /**
        * @name Rendering methods & Co
        * @{
        */
      void begin() ; // begin, Init Stereo.
      void end() ; // Init Avo.
      void displayStereoAnag() ;
      void drawAtomBond() ;

      double radius(const Atom *atom) const ;
      // @}


    //
    // Private attributs.
    private :


      const GLWidget *m_widget ;
      Camera *m_cam ;
      //WMSettingsWidget *m_settingsWidget;

      /**
        * @name Objects & dimensions of the rendered object
        * @{ */
      GLUquadric *quadricAtom ;
      static const double radiusAtom ;
      static const int slicesAtom, stacksAtom ;

      double m_atomRadiusPercentage ;
      double m_bondRadius ;
      int m_showMulti ;

      //double m_alpha; // transparency of the balls & sticks
      // @}


      /**
        * @name 3D-stereo data
        * @{ */
      static const GLfloat m_mat_ambient[], m_mat_diffuse[] ;
      static const GLfloat m_mat_specular[], m_mat_shininess[] ;

      static const GLfloat m_light_position[] ;

      static const GLfloat m_light_ambient0[], m_light_diffuse0[], m_light_specular0[] ;
      static const GLfloat m_light_ambient1[], m_light_diffuse1[], m_light_specular1[] ;


      struct frustumValue
      {
        // Size screen & widget.
        static const double wScreenPx, hScreenPx ;
        static const double wScreenCm, hScreenCm ;
        double wWidgetPx, hWidgetPx ;
        double wWidgetCm, hWidgetCm ;
        double aspectRatioWidget ;

        void convertWWidgetPxToCm( double a )
        {
          wWidgetPx = a ;
          wWidgetCm = (wWidgetPx*wScreenCm)/wScreenPx ;
        } ;

        void convertHWidgetPxToCm( double a )
        {
          hWidgetPx = a ;
          hWidgetCm = (hWidgetPx*hScreenCm)/hScreenPx ;
        } ;


        // The conversion cm to openGL unit is a strange thing which must
        // disapear. Wait the next refactoring ...

        // openGL unit = camera-space coordinates

        // Distance between eye and camera in openGL (openGL unit),
        // between 0.6 and 0.8 (constant in openGL world gets by ... defaut).
		// Values deprecated !!! This management must be change ! Test and fix default values.
		// And let the user to change this values.
		// And stop the cm/px conversion ... It is stupid ...
        static const double gapEyeCamGL ;

        // Distance between eye in real world (cm unit),
        // between 6 cm and 8 cm.
        static const double gapEyeCm ;

        double convertPxToGL( double px )
        {
          // 6 cm to px.
          // wScreenCm -> wScreenPx
          //  gapEyeCm -> gapEyePx
          double gapEyePx=(gapEyeCm*wScreenPx) / wScreenCm ;

          // px to gl.
          // gapEyePx -> (gapEyeCamGL*2)
          //       px -> returnGL
          return (px*(gapEyeCamGL*2)) / gapEyePx ;
        } ;

        // Some clip plane.
        double zScreen ; ///< openGL unit (clip plane)
        double zNear, zFar ; ///< openGL unit (clip plane)
        double fov ; ///< field of view

        static const double xFactor, yFactor ;
            ///< To compensate a bad ratio (pixels non-square, or the resolution of the scene which not respect the ratio of the screen.
      } ;
      struct frustumValue m_frust ;

      //@}

   //private Q_SLOTS:
      //void settingsWidgetDestroyed();

      /**
       * @param percent percentage of the VdwRad
       */
      //void setAtomRadiusPercentage(int percent);

      /**
       * @param value radius of the bonds * 10
       */
      //void setBondRadius(int value);

      /**
       * @param value determines where multiple bonds are shown
       */
      //void setShowMulti(int value);

      /**
       * @param value opacity of the balls & sticks / 20
       */
      //void setOpacity(int value);

      
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html

  };

  /*
  class WMSettingsWidget : public QWidget, public Ui::WMSettingsWidget
  {
    public:
      WMSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };
  */

  /**
   * @class WmEngineFactory wmengine.h
   * @brief Factory class to create instances of the WmEngine class.
   */
  class WmEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(WmEngine);
  };

} // end namespace Avogadro



#endif
