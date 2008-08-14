/**********************************************************************
  LinMorph - compute a lin morph and display given a second conformation 
             of the current molecule

  Copyright (C) 2008 by Naomi Fox

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/


#include "linmorphextension.h"
#include "trajvideomaker.h"

#include <openbabel/obconversion.h>
#include <avogadro/povpainter.h>

#include <QAction>
#include <QMessageBox>
#include <QInputDialog>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  LinMorphExtension::LinMorphExtension( QObject *parent ) :Extension( parent ), m_molecule(0), m_secondMolecule(0), m_widget(0), m_linMorphDialog(0), m_timeLine(0), m_frameCount(0)
  {  
    
    setFrameCount(10);

    QAction *action = new QAction(this);
    action->setText(tr("Lin Morph..."));
    m_actions.append(action);
    
    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append(action);

  }

  LinMorphExtension::~LinMorphExtension()
  {
    if (m_secondMolecule)
      delete m_secondMolecule;
  }

  QList<QAction *> LinMorphExtension::actions() const
  {
    return m_actions;
  }


  void LinMorphExtension::loadFile(QString file)
  {
    qDebug("LinMorphExtension::loadFile()");

    if (file.isEmpty())
      return;

    if (!m_secondMolecule) 
      m_secondMolecule = new Molecule;
    
    OBConversion conv;
    OBFormat *inFormat = OBConversion::FormatFromExt(( file.toAscii() ).data() );        
    if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "Cannot read file format of file %1." )
          .arg( file ) );
      return;
    }

    if (!conv.ReadFile(m_secondMolecule, file.toStdString())) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
	  tr( "Read mol file %1 failed." )
          .arg( file ) );
      return;
    }

    qDebug("LinMorphExtension::loadFile complete");
    computeConformers(m_secondMolecule);

    m_linMorphDialog->setFrame(1);
    m_timeLine->setFrameRange(1, m_frameCount);
    setDuration(m_linMorphDialog->fps());
  }

  // allows us to set the intended menu path for each action
  QString LinMorphExtension::menuPath(QAction *) const
  {
    return tr("&Extensions");
  }


  //! compute the conformers and set in molecule
  void LinMorphExtension::computeConformers(Molecule* conformer2Mol){
    
    int k,l;
    vector<double*> conf;
    double* xyz = NULL;

    if (conformer2Mol->NumAtoms() != m_molecule->NumAtoms()) {
      
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "Two molecules have different number atoms %1 %2" )
			    .arg(m_molecule->NumAtoms()).arg( conformer2Mol->NumAtoms()));
      return;
    }
    
    
    double* initCoords = m_molecule->GetCoordinates();
    double* finalCoords = conformer2Mol->GetCoordinates();


    for (k=0 ; k < m_frameCount; ++k) {
      xyz = new double [3*m_molecule->NumAtoms()];
      for (l=0 ; l<(int) (3*m_molecule->NumAtoms()) ; ++l) {
	double dCoord = (finalCoords[l] - initCoords[l])/m_frameCount;
	xyz[l]=initCoords[l] + dCoord*k;
      }
      conf.push_back(xyz);
    }
    
    m_molecule->SetConformers(conf);
        
    qDebug("Num conformers set: %d", m_molecule->NumConformers());
  }


  QDockWidget * LinMorphExtension::dockWidget()
  {
    // if we need a dock widget we can set one here
    return 0;
  }

  void LinMorphExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* LinMorphExtension::performAction(QAction *, GLWidget* widget)
  {
    m_widget = widget;
    
    if (!m_linMorphDialog)
      {
	m_timeLine = new QTimeLine;
	m_linMorphDialog = new LinMorphDialog;
	
	connect(m_linMorphDialog, SIGNAL(fileName(QString)), 
		this, SLOT(loadFile(QString)));
	//connect(m_linMorphDialog, SIGNAL(snapshotsPrefix(QString)), 
	//	this, SLOT(savePovSnapshots(QString)));
	connect(m_linMorphDialog, SIGNAL(trajFileName(QString)), 
		this, SLOT(saveTrajectoryFile(QString)));
	connect(m_linMorphDialog, SIGNAL(movieFileInfo(QString)), 
		this, SLOT(saveMovie(QString)));
	connect(m_linMorphDialog, SIGNAL(sliderChanged(int)), 
		this, SLOT(setFrame(int)));
	connect(m_linMorphDialog, SIGNAL(fpsChanged(int)), 
		this, SLOT(setDuration(int)));
	connect(m_linMorphDialog, SIGNAL(loopChanged(int)), 
		this, SLOT(setLoop(int)));
	connect(m_timeLine, SIGNAL(frameChanged(int)), 
		this, SLOT(setFrame(int)));
	connect(m_linMorphDialog, SIGNAL(play()), 
		m_timeLine, SLOT(start()));
	connect(m_linMorphDialog, SIGNAL(pause()), 
		m_timeLine, SLOT(stop()));
	connect(m_linMorphDialog, SIGNAL(stop()), 
		this, SLOT(stop()));
	connect(m_linMorphDialog, SIGNAL(frameCountChanged(int)), 
		this, SLOT(setFrameCount(int)));
	} 
    setFrameCount(m_frameCount);
    m_linMorphDialog->show();
    return 0;
  } 


  void LinMorphExtension::setDuration(int i)
  {
    int interval = 1000 / i;
    m_timeLine->setUpdateInterval(interval);
    int duration = interval * m_frameCount;
    m_timeLine->setDuration(duration);
  }


  void LinMorphExtension::setFrameCount(int i)
  {
    m_frameCount = i;
    if (m_linMorphDialog)
      m_linMorphDialog->setFrameCount(i);
    if (m_secondMolecule)
      computeConformers(m_secondMolecule);
  }

  void LinMorphExtension::setLoop(int state)
  {
    if (state == Qt::Checked) {
      m_timeLine->setLoopCount(0);
    } else {
      m_timeLine->setLoopCount(1);
    }
  }

  void LinMorphExtension::setFrame(int i)
  {
    m_linMorphDialog->setFrame(i);
    m_molecule->SetConformer(i - 1);
    m_molecule->update();
  }

  void LinMorphExtension::stop()
  {
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    setFrame(1);
  }


  void LinMorphExtension::saveGlSnapshots(QString prefix)
  {
    // This function does not work.  NKF - 6/30/2008 
    if (!m_widget) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "GL widget was not correctly initialized in order to save snapshots" ));
      return;
    }
      
    //computeConformers(m_secondMolecule);
    for (int i=1; i<=m_frameCount; i++) {
      setFrame(i);
      QImage exportImage = m_widget->grabFrameBuffer( true );
      QString ssfileName = prefix + QString::number(i) + ".png";
      if ( !exportImage.save( ssfileName ) ) {
	QMessageBox::warning( NULL, tr( "Avogadro" ),
			      tr( "Cannot save file %1." ).arg( ssfileName ) );
	return;
      }
    }
  }


  //movieFileName is the full filename (with full path)
  void LinMorphExtension::saveMovie(QString movieFileName) {
        
    if (movieFileName.isEmpty()) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "Must specify a valid .avi file name" ));
      return;
    }
    
    if (!movieFileName.endsWith(".avi")){
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "Must specify a valid .avi file name" ));
      return;
    }
    
    // use the current glWidge for things like camera
    if (!m_widget) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "GL widget was not correctly initialized in order to save movie" ));
      return;
    }
    
    //first, split out the directory and filenames
    QString dir, fileName, prefix;

    int slashPos = movieFileName.lastIndexOf("/");
    
    if (slashPos < 0) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "Invalid movie filename.  Must include full directory path" ));
      return;
    }

    dir = movieFileName.left(slashPos) + "/"; 
    fileName = movieFileName.right(movieFileName.length() - (slashPos+1));
    if (fileName.isEmpty()) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
			    tr( "Invalid movie filename.  Must include full directory path and name, ending with .avi" ));
      return;
    }
    
    //if (fileName.endsWith(".avi")) {
    prefix = fileName.left(fileName.length() - 4);
    

    //Make the directory where the snapshots will be saved
    QString snapshotsDir = dir + prefix + "/";
    QString mkdirCommand = "mkdir " + snapshotsDir;
    system(mkdirCommand.toStdString().c_str());

    TrajVideoMaker::makeVideo(m_widget, snapshotsDir, movieFileName);
  }
    
  void LinMorphExtension::saveTrajectoryFile(QString filename) {
    
    OBConversion conv;

    QMessageBox::warning( NULL, tr( "Avogadro" ),
			tr( "Would write file to %1." )
			.arg( filename ) );

    conv.SetInAndOutFormats("XYZ","XYZ");
    
    
    writeXYZTraj(filename);

QMessageBox::warning( NULL, tr( "Avogadro" ),
		      tr( "Wrote file %1." )
		      .arg( filename ) );
    

  }

  bool LinMorphExtension::writeXYZTraj(QString filename) {
    OBConversion conv;
    conv.SetInAndOutFormats("XYZ","XYZ");
    
    ofstream file;
    file.open(filename.toStdString().c_str());
	
    for (int i=1; i <= m_molecule->NumConformers(); i++) {
      setFrame(i);
      conv.Write(m_molecule,&file);
      file << endl;
    }
    
    file.close();				//close it
    
    return true;
  }
}
#include "linmorphextension.moc"

Q_EXPORT_PLUGIN2(linmorphextension, Avogadro::LinMorphExtensionFactory)
