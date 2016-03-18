// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : SMESHGUI_PropertiesDlg.cxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#include "SMESHGUI_PropertiesDlg.h"
#include "SMESH_Actor.h"

#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

#include <QtxBiColorTool.h>
#include <QtxColorButton.h>
#include <QtxIntSpinBox.h>
#include <QtxDoubleSpinBox.h>
#include <VTKViewer_MarkerWidget.h>
#include <SUIT_Session.h>
#include <LightApp_Application.h>

const int MARGIN  = 9;
const int SPACING = 6;

/*!
  \class SMESHGUI_PropertiesDlg
  \brief Dialog box to set-up mesh presentation properties: colors,
  sizes of elements, width of lines etc
*/

/*
  \brief Constructor
  \param customMarkers custom node markers
  \param parent parent widget
*/
SMESHGUI_PropertiesDlg::SMESHGUI_PropertiesDlg( const VTK::MarkerMap& customMarkers, QWidget* parent )
  : SMESHGUI_Dialog( parent, true, true, Standard )
{
  // set title
  setWindowTitle( tr( "TITLE" ) );
  
  // create widgets

  QHBoxLayout* hl;
  int widthLab1 = 0, widthLab2 = 0;

  // -- node controls
  myNodeGrp = new QGroupBox( tr( "NODES" ), mainFrame() );
  QLabel* nodeColorLab = new QLabel( tr( "COLOR" ), myNodeGrp );
  myNodeColor = new QtxColorButton( myNodeGrp );
  myNodeMarker = new VTKViewer_MarkerWidget( myNodeGrp );
  hl = new QHBoxLayout( myNodeGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( nodeColorLab );
  hl->addWidget( myNodeColor );
  hl->addWidget( myNodeMarker );
  widthLab1 = qMax( widthLab1, nodeColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, myNodeMarker->typeLabel()->minimumSizeHint().width() );

  // -- edge controls
  myEdgeGrp = new QGroupBox( tr( "EDGES" ), mainFrame() );
  QLabel* edgeColorLab = new QLabel( tr( "COLOR" ), myEdgeGrp );
  myEdgeColor = new QtxColorButton( myEdgeGrp );
  QLabel* edgeWidthLab = new QLabel( tr( "WIDTH" ), myEdgeGrp );
  myEdgeWidth = new QtxIntSpinBox( myEdgeGrp );
  hl = new QHBoxLayout( myEdgeGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( edgeColorLab );
  hl->addWidget( myEdgeColor );
  hl->addWidget( edgeWidthLab );
  hl->addWidget( myEdgeWidth );
  widthLab1 = qMax( widthLab1, edgeColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, edgeWidthLab->minimumSizeHint().width() );
  
  // -- face controls
  myFaceGrp = new QGroupBox( tr( "FACES" ), mainFrame() );
  QLabel* faceColorLab = new QLabel( tr( "FACE_FRONT" ), myFaceGrp );
  myFaceColor = new QtxBiColorTool( myFaceGrp );
  myFaceColor->label()->setText( tr( "FACE_BACK" ) );
  hl = new QHBoxLayout( myFaceGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( faceColorLab );
  hl->addWidget( myFaceColor );
  widthLab1 = qMax( widthLab1, faceColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, myFaceColor->label()->minimumSizeHint().width() );
  
  // -- volumes
  myVolumeGrp = new QGroupBox( tr( "VOLUMES" ), mainFrame() );
  QLabel* volumeColorLab = new QLabel( tr( "VOLUME_NORMAL" ), myVolumeGrp );
  myVolumeColor = new QtxBiColorTool( myVolumeGrp );
  myVolumeColor->label()->setText( tr( "VOLUME_REVERSED" ) );
  hl = new QHBoxLayout( myVolumeGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( volumeColorLab );
  hl->addWidget( myVolumeColor );
  widthLab1 = qMax( widthLab1, volumeColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, myVolumeColor->label()->minimumSizeHint().width() );

  // -- outline controls
  myOutlineGrp = new QGroupBox( tr( "OUTLINES" ), mainFrame() );
  QLabel* outlineColorLab = new QLabel( tr( "COLOR" ), myOutlineGrp );
  myOutlineColor = new QtxColorButton( myOutlineGrp );
  QLabel* outlineWidthLab = new QLabel( tr( "WIDTH" ), myOutlineGrp );
  myOutlineWidth = new QtxIntSpinBox( myOutlineGrp );
  hl = new QHBoxLayout( myOutlineGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( outlineColorLab );
  hl->addWidget( myOutlineColor );
  hl->addWidget( outlineWidthLab );
  hl->addWidget( myOutlineWidth );
  widthLab1 = qMax( widthLab1, outlineColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, outlineWidthLab->minimumSizeHint().width() );

  // -- 0d element controls
  myElem0dGrp = new QGroupBox( tr( "0D_ELEMENTS" ), mainFrame() );
  QLabel* elem0dColorLab = new QLabel( tr( "COLOR" ), myElem0dGrp );
  myElem0dColor = new QtxColorButton( myElem0dGrp );
  QLabel* elem0dSizeLab = new QLabel( tr( "SIZE" ), myElem0dGrp );
  myElem0dSize = new QtxIntSpinBox( myElem0dGrp );
  hl = new QHBoxLayout( myElem0dGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( elem0dColorLab );
  hl->addWidget( myElem0dColor );
  hl->addWidget( elem0dSizeLab );
  hl->addWidget( myElem0dSize );
  widthLab1 = qMax( widthLab1, elem0dColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, elem0dSizeLab->minimumSizeHint().width() );
  
  // -- ball controls
  myBallGrp = new QGroupBox( tr( "BALLS" ), mainFrame() );
  QLabel* ballColorLab = new QLabel( tr( "COLOR" ), myBallGrp );
  myBallColor = new QtxColorButton( myBallGrp );
  // QLabel* ballSizeLab = new QLabel( tr( "SIZE" ), myBallGrp );
  // myBallSize = new QtxIntSpinBox( myBallGrp );
  QLabel* ballScaleLab = new QLabel( tr( "SCALE_FACTOR" ), myBallGrp );
  myBallScale = new QtxDoubleSpinBox( 1e-2, 1e7, 0.5, myBallGrp );
  hl = new QHBoxLayout( myBallGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( ballColorLab );
  hl->addWidget( myBallColor );
  // hl->addWidget( ballSizeLab );
  // hl->addWidget( myBallSize );
  hl->addWidget( ballScaleLab );
  hl->addWidget( myBallScale );
  widthLab1 = qMax( widthLab1, ballColorLab->minimumSizeHint().width() );
  // widthLab2 = qMax( widthLab2, ballSizeLab->minimumSizeHint().width() );
  
  // -- orientation vector controls
  myOrientationGrp = new QGroupBox( tr( "ORIENTATIONS" ), mainFrame() );
  QLabel* orientationColorLab = new QLabel( tr( "COLOR" ), myOrientationGrp );
  myOrientationColor = new QtxColorButton( myOrientationGrp );
  QLabel* orientationScaleLab = new QLabel( tr( "SCALE_FACTOR" ), myOrientationGrp );
  myOrientationSize = new QtxIntSpinBox( myOrientationGrp );
  myOrientationSize->setSuffix( "% ");
  myOrientation3d = new QCheckBox( tr("ORIENTATION_3D"), myOrientationGrp );
  hl = new QHBoxLayout( myOrientationGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( orientationColorLab );
  hl->addWidget( myOrientationColor );
  hl->addWidget( orientationScaleLab );
  hl->addWidget( myOrientationSize );
  hl->addWidget( myOrientation3d );
  widthLab1 = qMax( widthLab1, orientationColorLab->minimumSizeHint().width() );
  widthLab2 = qMax( widthLab2, orientationScaleLab->minimumSizeHint().width() );

  // -- other controls
  myExtraGrp = new QFrame( mainFrame() );
  QLabel* shrinkSizeLab = new QLabel( tr( "SHRINK" ), myExtraGrp );
  myShrinkSize = new QtxIntSpinBox( myExtraGrp );
  myShrinkSize->setSuffix( "% ");
  hl = new QHBoxLayout( myExtraGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  hl->addWidget( shrinkSizeLab );
  hl->addWidget( myShrinkSize );
  hl->addStretch();
  widthLab1 = qMax( widthLab1, shrinkSizeLab->minimumSizeHint().width() );

  // layout widgets
  QVBoxLayout* vl = new QVBoxLayout( mainFrame() );
  vl->setMargin( 0 );
  vl->setSpacing( SPACING );
  vl->addWidget( myNodeGrp );
  vl->addWidget( myEdgeGrp );
  vl->addWidget( myFaceGrp );
  vl->addWidget( myVolumeGrp );
  vl->addWidget( myOutlineGrp );
  vl->addWidget( myElem0dGrp );
  vl->addWidget( myBallGrp );
  vl->addWidget( myOrientationGrp );
  vl->addWidget( myExtraGrp );
  
  nodeColorLab->setMinimumWidth( widthLab1 );
  edgeColorLab->setMinimumWidth( widthLab1 );
  faceColorLab->setMinimumWidth( widthLab1 );
  volumeColorLab->setMinimumWidth( widthLab1 );
  outlineColorLab->setMinimumWidth( widthLab1 );
  elem0dColorLab->setMinimumWidth( widthLab1 );
  ballColorLab->setMinimumWidth( widthLab1 );
  orientationColorLab->setMinimumWidth( widthLab1 );
  shrinkSizeLab->setMinimumWidth( widthLab1 );

  myNodeMarker->typeLabel()->setMinimumWidth( widthLab2 );
  edgeWidthLab->setMinimumWidth( widthLab2 );
  myFaceColor->label()->setMinimumWidth( widthLab2 );
  myVolumeColor->label()->setMinimumWidth( widthLab2 );
  outlineWidthLab->setMinimumWidth( widthLab2 );
  elem0dSizeLab->setMinimumWidth( widthLab2 );
  // ballSizeLab->setMinimumWidth( widthLab2 );
  orientationScaleLab->setMinimumWidth( widthLab2 );

  myEdgeWidth->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myOutlineWidth->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myElem0dSize->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  // myBallSize->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myBallScale->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myOrientationSize->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myShrinkSize->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

  // initialize widgets
  myNodeMarker->setCustomMarkers( customMarkers );
  myElem0dSize->setRange( 1, 10 );
  // myBallSize->setRange( 1, 10 );
  myEdgeWidth->setRange( 1, 5 );
  myOutlineWidth->setRange( 1, 5 );
  myShrinkSize->setRange( 20, 100 );
  myOrientationSize->setRange( 5, 100 );

  button( OK )->setText( tr( "SMESH_BUT_OK" ) );

  connect( this, SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );
}

/*
  \brief Destructor: clean-up resources if necessary
*/
SMESHGUI_PropertiesDlg::~SMESHGUI_PropertiesDlg()
{
}

/*!
  \brief Set nodes color
  \param color nodes color
*/
void SMESHGUI_PropertiesDlg::setNodeColor( const QColor& color )
{
  myNodeColor->setColor( color );
}

/*!
  \brief Get nodes color
  \return current nodes color
*/
QColor SMESHGUI_PropertiesDlg::nodeColor() const
{
  return myNodeColor->color();
}

/*!
  \brief Set standard nodes marker
  \param type standard nodes marker type
  \param scale standard nodes marker scale
*/
void SMESHGUI_PropertiesDlg::setNodeMarker( VTK::MarkerType type, VTK::MarkerScale scale )
{
  myNodeMarker->setMarker( type, scale );
}

/*!
  \brief Set custom nodes marker
  \param id custom nodes marker id
*/
void SMESHGUI_PropertiesDlg::setNodeCustomMarker( int id )
{
  myNodeMarker->setCustomMarker( id );
}

/*!
  \brief Get nodes marker type.
  For custom marker, VTK::MT_USER is returned and markerId() function
  then returns its identifier.
  \return currently selected nodes marker type
*/
VTK::MarkerType SMESHGUI_PropertiesDlg::nodeMarkerType() const
{
  return myNodeMarker->markerType();
}

/*!
  \brief Get nodes marker scale.
  For custom marker return value is undefined.
  \return currently selected nodes marker scale
*/
VTK::MarkerScale SMESHGUI_PropertiesDlg::nodeMarkerScale() const
{
  return myNodeMarker->markerScale();
}

/*!
  \brief Get custom nodes marker.
  For standard markers return value is VTK::MT_NONE.
  \return custom nodes marker id
*/
int SMESHGUI_PropertiesDlg::nodeMarkerId() const
{
  return myNodeMarker->markerId();
}

/*!
  \brief Set edges (wireframe) color
  \param color edges color
*/
void SMESHGUI_PropertiesDlg::setEdgeColor( const QColor& color )
{
  myEdgeColor->setColor( color );
}

/*!
  \brief Get edges (wireframe) color
  \return current edges color
*/
QColor SMESHGUI_PropertiesDlg::edgeColor() const
{
  return myEdgeColor->color();
}

/*!
  \brief Set edges width
  \param width edges width
*/
void SMESHGUI_PropertiesDlg::setEdgeWidth( int width )
{
  myEdgeWidth->setValue( width );
}

/*!
  \brief Get edges width
  \return current edges width
*/
int SMESHGUI_PropertiesDlg::edgeWidth() const
{
  return myEdgeWidth->value();
}

/*!
  \brief Set faces colors
  \param color front faces color
  \param delta back faces coloring delta
*/
void SMESHGUI_PropertiesDlg::setFaceColor( const QColor& color, int delta )
{
  myFaceColor->setMainColor( color );
  myFaceColor->setDelta( delta );
}

/*!
  \brief Get front faces color
  \return current front faces color
*/
QColor SMESHGUI_PropertiesDlg::faceColor() const
{
  return myFaceColor->mainColor();
}

/*!
  \brief Get back faces coloring delta
  \return current back faces coloring delta
*/
int SMESHGUI_PropertiesDlg::faceColorDelta() const
{
  return myFaceColor->delta();
}

/*!
  \brief Set volumes colors
  \param color normal volumes color
  \param delta reversed volumes coloring delta
*/
void SMESHGUI_PropertiesDlg::setVolumeColor( const QColor& color, int delta )
{
  myVolumeColor->setMainColor( color );
  myVolumeColor->setDelta( delta );
}

/*!
  \brief Get normal volumes color
  \return current normal volumes color
*/
QColor SMESHGUI_PropertiesDlg::volumeColor() const
{
  return myVolumeColor->mainColor();
}

/*!
  \brief Get reversed volumes coloring delta
  \return current reversed volumes coloring delta
*/
int SMESHGUI_PropertiesDlg::volumeColorDelta() const
{
  return myVolumeColor->delta();
}

/*!
  \brief Set outlines color
  \param color outlines color
*/
void SMESHGUI_PropertiesDlg::setOutlineColor( const QColor& color )
{
  myOutlineColor->setColor( color );
}

/*!
  \brief Get outlines color
  \return current outlines color
*/
QColor SMESHGUI_PropertiesDlg::outlineColor() const
{
  return myOutlineColor->color();
}

/*!
  \brief Set outlines width
  \param width outlines width
*/
void SMESHGUI_PropertiesDlg::setOutlineWidth( int width )
{
  myOutlineWidth->setValue( width );
}

/*!
  \brief Get outlines width
  \return current outlines width
*/
int SMESHGUI_PropertiesDlg::outlineWidth() const
{
  return myOutlineWidth->value();
}

/*!
  \brief Set 0D elements color
  \param color 0D elements color
*/
void SMESHGUI_PropertiesDlg::setElem0dColor( const QColor& color )
{
  myElem0dColor->setColor( color );
}

/*!
  \brief Get 0D elements color
  \return current 0D elements color
*/
QColor SMESHGUI_PropertiesDlg::elem0dColor() const
{
  return myElem0dColor->color();
}

/*!
  \brief Set 0D elements size
  \param size 0D elements size
*/
void SMESHGUI_PropertiesDlg::setElem0dSize( int size )
{
  myElem0dSize->setValue( size );
}

/*!
  \brief Get 0D elements size
  \return current 0D elements size
*/
int SMESHGUI_PropertiesDlg::elem0dSize() const
{
  return myElem0dSize->value();
}

/*!
  \brief Set discrete elements (balls) color
  \param color discrete elements (balls) color
*/
void SMESHGUI_PropertiesDlg::setBallColor( const QColor& color )
{
  myBallColor->setColor( color );
}

/*!
  \brief Get discrete elements (balls) color
  \return current discrete elements (balls) color
*/
QColor SMESHGUI_PropertiesDlg::ballColor() const
{
  return myBallColor->color();
}

/*!
  \brief Set discrete elements (balls) size
  \param size discrete elements (balls) size
*/
/*void SMESHGUI_PropertiesDlg::setBallSize( int size )
{
  myBallSize->setValue( size );
}*/

/*!
  \brief Get discrete elements (balls) size
  \return current discrete elements (balls) size
*/
/*int SMESHGUI_PropertiesDlg::ballSize() const
{
  return myBallSize->value();
}*/

/*!
  \brief Set discrete elements (balls) scale factor
  \param size discrete elements (balls) scale factor
*/
void SMESHGUI_PropertiesDlg::setBallScale( double size )
{
  myBallScale->setValue( size );
}

/*!
  \brief Get discrete elements (balls) scale factor
  \return current discrete elements (balls) scale factor
*/
double SMESHGUI_PropertiesDlg::ballScale() const
{
  return myBallScale->value();
}

/*!
  \brief Set orientation vectors color
  \param color orientation vectors color
*/
void SMESHGUI_PropertiesDlg::setOrientationColor( const QColor& color )
{
  myOrientationColor->setColor( color );
}

/*!
  \brief Get orientation vectors color
  \return current orientation vectors color
*/
QColor SMESHGUI_PropertiesDlg::orientationColor() const
{
  return myOrientationColor->color();
}

/*!
  \brief Set orientation vectors scale (percent)
  \param scale orientation vectors scale
*/
void SMESHGUI_PropertiesDlg::setOrientationSize( int scale )
{
  myOrientationSize->setValue( scale );
}

/*!
  \brief Get orientation vectors scale (percent)
  \return current orientation vectors scale
*/
int SMESHGUI_PropertiesDlg::orientationSize() const
{
  return myOrientationSize->value();
}

/*!
  \brief Set orientation vectors 3d flag
  \param on orientation vectors 3d flag value
*/
void SMESHGUI_PropertiesDlg::setOrientation3d( bool on )
{
  myOrientation3d->setChecked( on );
}

/*!
  \brief Get orientation vectors 3d flag
  \return orientation vectors 3d flag value
*/
bool SMESHGUI_PropertiesDlg::orientation3d() const
{
  return myOrientation3d->isChecked();
}

/*!
  \brief Set shrink coefficient (percent)
  \param coef shrink coefficient
*/
void SMESHGUI_PropertiesDlg::setShrinkCoef( int coef )
{
  myShrinkSize->setValue( coef );
}

/*!
  \brief Get shrink coefficient (percent)
  \return current shrink coefficient
*/
int SMESHGUI_PropertiesDlg::shrinkCoef() const
{
  return myShrinkSize->value();
}

/*
  \brief Get custom markers
  \return custom markers map
*/
VTK::MarkerMap SMESHGUI_PropertiesDlg::customMarkers() const
{
  return myNodeMarker->customMarkers();
}

/*!
  \brief Show / hide controls for specified entity type
  \param elements mesh element types (an or-ed combination of flags)
  \param nodes mesh nodes presence flag
*/
void SMESHGUI_PropertiesDlg::showControls( int elements, bool nodes )
{
  // node controls are supposed to be shown if at least any element type is present
  // or if there are only nodes
  myNodeGrp->setVisible( nodes || elements & SMESH_Actor::eAllEntity );
  // edge/wireframe controls are needed for edges, faces and volumes
  myEdgeGrp->setVisible( elements & ( SMESH_Actor::eEdges | SMESH_Actor::eFaces | SMESH_Actor::eVolumes ) );
  // face controls are shown only if there are faces
  myFaceGrp->setVisible( elements & SMESH_Actor::eFaces );
  // volume controls are shown only if there are volumes
  myVolumeGrp->setVisible( elements & SMESH_Actor::eVolumes );
  // 0d elements controls are shown only if there are 0d elements
  myElem0dGrp->setVisible( elements & SMESH_Actor::e0DElements );
  // ball controls are shown only if there are balls
  myBallGrp->setVisible( elements & SMESH_Actor::eBallElem );
  // outline controls are needed for faces and volumes
  myOutlineGrp->setVisible( elements & ( SMESH_Actor::eFaces | SMESH_Actor::eVolumes ) );
  // orientation controls are needed for faces and volumes
  myOrientationGrp->setVisible( elements & ( SMESH_Actor::eFaces | SMESH_Actor::eVolumes ) );
  // shrink factor is shown if there are edges and/or faces and/or volumes
  myExtraGrp->setVisible( elements & ( SMESH_Actor::eEdges | SMESH_Actor::eFaces | SMESH_Actor::eVolumes ) );
}

/*!
  \brief Show online help on dialog box
*/
void SMESHGUI_PropertiesDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  app->onHelpContextModule( "SMESH", "colors_size_page.html" );
}
