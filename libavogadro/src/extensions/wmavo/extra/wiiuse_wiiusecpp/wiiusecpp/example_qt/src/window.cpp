/*
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This file is part of example_qt of the wiiusecpp library.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "window.h"

Window::Window() : m_extension(NULL), m_rowCurrent(0), file( "./out2.txt" )
{
    m_glWidget = new GLWidget(this);
    m_extension = new Extension(this) ;

    if( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
        QMessageBox( QMessageBox::Critical, QString("ERROR"), QString("POWNED1, File not created") ) ;

    m_sliderXRot = createSliderRotation() ; m_sliderYRot = createSliderRotation() ;
    m_sliderZRot = createSliderRotation() ;
    m_sliderXTransl = createSliderTranslation() ; m_sliderYTransl = createSliderTranslation() ;
    m_sliderZTransl = createSliderTranslation() ;

    m_buttonStartWm = new QPushButton("Connect Wiimote") ;

    m_textBoxPosX = createTextBox() ; m_textBoxPosY = createTextBox() ;
    m_textBoxPosZ = createTextBox() ;
    m_textBoxPosXMoy = createTextBox() ; m_textBoxPosYMoy = createTextBox() ;
    m_textBoxPosZMoy = createTextBox() ;
    m_textBoxPosXGaus = createTextBox() ; m_textBoxPosYGaus = createTextBox() ;
    m_textBoxPosZGaus = createTextBox() ;

    m_labelPos = new QLabel("Position:") ;
    m_labelPosX = new QLabel("X:") ; m_labelPosY = new QLabel("Y:") ; m_labelPosZ = new QLabel("Z:") ;
    m_labelPosMoy = new QLabel("Average Position:") ;
    m_labelPosXMoy = new QLabel("X:") ; m_labelPosYMoy = new QLabel("Y:") ; m_labelPosZMoy = new QLabel("Z:") ;
    m_labelPosGaus = new QLabel("Gaussian Position:") ;
    m_labelPosXGaus = new QLabel("X:") ; m_labelPosYGaus= new QLabel("Y:") ; m_labelPosZGaus = new QLabel("Z:") ;

    QSplitter *splitter=new QSplitter ;
    m_modelTable = createTable(*splitter) ;

    m_checkBoxActivated = new QCheckBox( "Activated log" ) ; m_checkBoxActivated->setChecked(false);
    m_checkBoxFile = new QCheckBox( "To file" ) ; m_checkBoxFile->setChecked(false);
    m_checkBoxTable = new QCheckBox( "To table" ) ; m_checkBoxTable->setChecked(false);


    connect(m_sliderXRot, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setXRotation(int))) ;
    connect(m_glWidget, SIGNAL(xRotationChanged(int)), m_sliderXRot, SLOT(setValue(int))) ;
    connect(m_sliderYRot, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setYRotation(int))) ;
    connect(m_glWidget, SIGNAL(yRotationChanged(int)), m_sliderYRot, SLOT(setValue(int))) ;
    connect(m_sliderZRot, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setZRotation(int))) ;
    connect(m_glWidget, SIGNAL(zRotationChanged(int)), m_sliderZRot, SLOT(setValue(int))) ;

    connect(m_sliderXTransl, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setXTranslation(int))) ;
    connect(m_sliderYTransl, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setYTranslation(int))) ;
    connect(m_sliderZTransl, SIGNAL(valueChanged(int)), m_glWidget, SLOT(setZTranslation(int))) ;

    connect( m_buttonStartWm, SIGNAL(pressed()), m_extension, SLOT(connectAndStart()) ) ;
    connect( this, SIGNAL(xyzTranslationChanged(float,float,float)), m_glWidget, SLOT(setTranslation(float,float,float)) ) ;

    connect( m_extension, SIGNAL(xyzPositionsChanged(double,double,double,double,double,double,double,double,double)),
             this, SLOT(setWmTable(double,double,double,double,double,double,double,double,double)) ) ;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    QHBoxLayout *hLayout3 = new QHBoxLayout;
    QHBoxLayout *hLayout4 = new QHBoxLayout;
    QHBoxLayout *hLayout5 = new QHBoxLayout;
    QHBoxLayout *hLayout6 = new QHBoxLayout;
    QVBoxLayout *vLayout1 = new QVBoxLayout;

    hLayout1->addWidget(m_sliderXRot);
    hLayout1->addWidget(m_sliderYRot);
    hLayout1->addWidget(m_sliderZRot);
    vLayout1->addLayout(hLayout1);

    hLayout2->addWidget(m_sliderXTransl);
    hLayout2->addWidget(m_sliderYTransl);
    hLayout2->addWidget(m_sliderZTransl);
    vLayout1->addLayout(hLayout2);

    vLayout1->addWidget(m_buttonStartWm) ;
    hLayout3->addWidget(m_labelPos) ;
    hLayout3->addWidget(m_labelPosX) ;
    hLayout3->addWidget(m_textBoxPosX) ;
    hLayout3->addWidget(m_labelPosY) ;
    hLayout3->addWidget(m_textBoxPosY) ;
    hLayout3->addWidget(m_labelPosZ) ;
    hLayout3->addWidget(m_textBoxPosZ) ;
    vLayout1->addLayout(hLayout3) ;

    hLayout4->addWidget(m_labelPosMoy) ;
    hLayout4->addWidget(m_labelPosXMoy) ;
    hLayout4->addWidget(m_textBoxPosXMoy) ;
    hLayout4->addWidget(m_labelPosYMoy) ;
    hLayout4->addWidget(m_textBoxPosYMoy) ;
    hLayout4->addWidget(m_labelPosZMoy) ;
    hLayout4->addWidget(m_textBoxPosZMoy) ;
    vLayout1->addLayout(hLayout4) ;

    hLayout5->addWidget(m_labelPosGaus) ;
    hLayout5->addWidget(m_labelPosXGaus) ;
    hLayout5->addWidget(m_textBoxPosXGaus) ;
    hLayout5->addWidget(m_labelPosYGaus) ;
    hLayout5->addWidget(m_textBoxPosYGaus) ;
    hLayout5->addWidget(m_labelPosZGaus) ;
    hLayout5->addWidget(m_textBoxPosZGaus) ;
    vLayout1->addLayout(hLayout5) ;

    hLayout6->addWidget(m_checkBoxActivated) ;
    hLayout6->addWidget(m_checkBoxFile) ;
    hLayout6->addWidget(m_checkBoxTable) ;
    vLayout1->addLayout(hLayout6) ;

    vLayout1->addWidget( splitter ) ;

    mainLayout->addWidget(m_glWidget);
    mainLayout->addLayout(vLayout1);
    setLayout(mainLayout);

    m_sliderXRot->setValue(15 * SLIDER_ROT_CONST);
    m_sliderYRot->setValue(345 * SLIDER_ROT_CONST);
    m_sliderZRot->setValue(0 * SLIDER_ROT_CONST);
    setWindowTitle(tr("Wiimote GL"));
}

Window::~Window()
{
    if( m_extension != NULL )
    {
        delete m_extension ;
        m_extension = NULL ;
    }
}


bool Window::isCheckFile()
{
    return m_checkBoxActivated->isChecked() && m_checkBoxFile->isChecked() ;
}


void Window::setWmPosition( double x, double y, double z )
{
    m_textBoxPosX->setText( QString::number(x*100.0f) ) ;
    m_textBoxPosY->setText( QString::number(y*100.0f) ) ;
    m_textBoxPosZ->setText( QString::number(z*100.0f) ) ;
    //m_glWidget->setTranslation( x, y, z ) ;
    emit xyzTranslationChanged( x, y, z );
}

void Window::setWmPositionMoy( double x, double y, double z )
{
    m_textBoxPosXMoy->setText( QString::number(x) ) ;
    m_textBoxPosYMoy->setText( QString::number(y) ) ;
    m_textBoxPosZMoy->setText( QString::number(z) ) ;
}

void Window::setWmPositionGaus( double x, double y, double z )
{
    m_textBoxPosXGaus->setText( QString::number(x) ) ;
    m_textBoxPosYGaus->setText( QString::number(y) ) ;
    m_textBoxPosZGaus->setText( QString::number(z) ) ;
}

void Window::setWmTable( double x, double y, double z, double x1, double y1, double z1, double x2, double y2, double z2 )
{

    if( m_checkBoxActivated->isChecked() )
    {
        if( m_checkBoxFile->isChecked() )
        { // See void Extension::updateWindow()
            // => Disable table.

            QTextStream out( &file ) ;
            out << x << ", " << y << ", " << z << " | "
                << x1 << ", " << y1 << ", " << z1 << " | "
                << x2 << ", " << y2 << ", " << z2 << " | "
                << "\n" ;
        }

        if( m_checkBoxTable->isChecked() )
        {
            m_textBoxPosX->setText( QString::number(x) ) ;
            m_textBoxPosY->setText( QString::number(y) ) ;
            m_textBoxPosZ->setText( QString::number(z) ) ;

            m_textBoxPosXMoy->setText( QString::number(x1) ) ;
            m_textBoxPosYMoy->setText( QString::number(y1) ) ;
            m_textBoxPosZMoy->setText( QString::number(z1) ) ;

            m_textBoxPosXGaus->setText( QString::number(x2) ) ;
            m_textBoxPosYGaus->setText( QString::number(y2) ) ;
            m_textBoxPosZGaus->setText( QString::number(z2) ) ;

            m_modelTable->insertRows(m_rowCurrent, 1, QModelIndex()) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 0, QModelIndex()),
                                   QString::number(x) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 1, QModelIndex()),
                                   QString::number(y) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 2, QModelIndex()),
                                   QString::number(z) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 3, QModelIndex()),
                                   QString::number(x1) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 4, QModelIndex()),
                                   QString::number(y1) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 5, QModelIndex()),
                                   QString::number(z1) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 6, QModelIndex()),
                                   QString::number(x2) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 7, QModelIndex()),
                                   QString::number(y2) ) ;
            m_modelTable->setData( m_modelTable->index(m_rowCurrent, 8, QModelIndex()),
                                   QString::number(z2) ) ;
            m_rowCurrent++ ;
        }
    }
}


QSlider *Window::createSliderRotation()
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 360 * SLIDER_ROT_CONST);
    slider->setSingleStep(SLIDER_ROT_CONST);
    slider->setPageStep(15 * SLIDER_ROT_CONST);
    slider->setTickInterval(15 * SLIDER_ROT_CONST);
    slider->setTickPosition(QSlider::TicksRight);
    slider->setAutoFillBackground(false) ;
    return slider;
}

QSlider *Window::createSliderTranslation()
{
    QSlider *slider = new QSlider(Qt::Horizontal) ;
    slider->setRange( 0, SLIDER_TRANS_MAX ) ; //m_glWidget->maximumSize().height() ) ;
    //slider->setSingleStep( 1 );
    //slider->setPageStep(15 );
    //slider->setTickInterval(15 );
    slider->setTickPosition(QSlider::TicksRight);
    slider->setAutoFillBackground(false);
    return slider;
}

QLineEdit* Window::createTextBox()
{
    QLineEdit *textBox = new QLineEdit("0") ;
    textBox->setReadOnly(true) ;
    return textBox ;
}

QAbstractItemModel* Window::createTable( QSplitter &splitter_out )
{
    QAbstractItemModel* model = new QStandardItemModel(2, 9, this);
    QTableView *table = new QTableView ;
    //splitter_out = new QSplitter ;

    /*
    model->setHeaderData(0, Qt::Horizontal, tr("PosX"));
    model->setHeaderData(1, Qt::Horizontal, tr("PosY"));
    model->setHeaderData(2, Qt::Horizontal, tr("PosZ"));
    model->setHeaderData(3, Qt::Horizontal, tr("X1"));
    model->setHeaderData(4, Qt::Horizontal, tr("Y1"));
    model->setHeaderData(5, Qt::Horizontal, tr("Z1"));
    model->setHeaderData(6, Qt::Horizontal, tr("X2"));
    model->setHeaderData(7, Qt::Horizontal, tr("Y2"));
    model->setHeaderData(8, Qt::Horizontal, tr("Z2"));
    */

    splitter_out.addWidget(table) ;
    splitter_out.setStretchFactor(0, 0) ;

    table->setModel(model) ;

    QItemSelectionModel *selectionModel=new QItemSelectionModel(model) ;
    table->setSelectionModel(selectionModel) ;

    QHeaderView *headerView = table->horizontalHeader() ;
    headerView->setStretchLastSection(true) ;
    return model ;
}

void Window::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}
