// Copyright (C) 2014-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI_DisplayEntitiesDlg.cxx
//  Author : Alexander KOVALEV, Open CASCADE S.A.S. (alexander.kovalev@opencascade.com)

#include "SMESHGUI_DisplayEntitiesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

const int MARGIN  = 9;
const int SPACING = 6;

/*!
  \class SMESHGUI_DisplayEntitiesDlg
  \brief Dialog box to select entities to be displayed in viewer
*/

/*
  \brief Constructor
  \param parent parent widget
*/
SMESHGUI_DisplayEntitiesDlg::SMESHGUI_DisplayEntitiesDlg( QWidget* parent )
  : SMESHGUI_Dialog( parent, true, false, Standard )
{
  SUIT_ResourceMgr* aResMgr = SUIT_Session::session()->resourceMgr();

  LightApp_SelectionMgr* mgr = SMESHGUI::selectionMgr();
  SALOME_ListIO selected;
  mgr->selectedObjects( selected );
  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_nil();
  myActor = 0;
  myNbCheckedButtons = 0;

  SALOME_ListIteratorOfListIO it( selected );
  myIObject = selected.First();
  if ( myIObject->hasEntry() ) {
    myActor = SMESH::FindActorByEntry( myIObject->getEntry() );
  }
  myEntityMode = myActor ? myActor->GetEntityMode() : 0;

  aMesh = SMESH::GetMeshByIO( myIObject );

  // set title
  setWindowTitle( tr( "MEN_DISP_ENT" ) );

  // create widgets
  QGroupBox* anEntitiesGrp = new QGroupBox( tr( "SMESH_MESHINFO_ENTITIES" ), mainFrame() );
  QGridLayout* hl = new QGridLayout( anEntitiesGrp );
  hl->setMargin( MARGIN );
  hl->setSpacing( SPACING );
  int nbElements;

  // 0DElements
  nbElements = myActor ? myActor->GetObject()->GetNbEntities( SMDSAbs_0DElement ) : aMesh->Nb0DElements();
  my0DElemsTB = new QCheckBox( tr("SMESH_ELEMS0D"), anEntitiesGrp );
  my0DElemsTB->setIcon( QIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_DLG_ELEM0D" ) ) ) );
  bool has0DElems = myEntityMode & SMESH_Actor::e0DElements;
  my0DElemsTB->setChecked( has0DElems );
  if ( has0DElems )
    myNbCheckedButtons++;
  connect( my0DElemsTB, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool)) );
  QLabel* nb0DElemsLab = new QLabel( QString("%1").arg(nbElements).toLatin1().data(), anEntitiesGrp );
  hl->addWidget( my0DElemsTB, 0, 0 );
  hl->addWidget( nb0DElemsLab, 0, 1 );
  my0DElemsTB->setEnabled( nbElements );
  nb0DElemsLab->setEnabled( nbElements );

  // Edges
  nbElements = myActor ? myActor->GetObject()->GetNbEntities( SMDSAbs_Edge ) : aMesh->NbEdges();
  myEdgesTB = new QCheckBox( tr("SMESH_EDGES"), anEntitiesGrp );
  myEdgesTB->setIcon( QIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_DLG_EDGE" ) ) ) );
  bool hasEdges = myEntityMode & SMESH_Actor::eEdges;
  myEdgesTB->setChecked( hasEdges );
  if ( hasEdges )
    myNbCheckedButtons++;
  connect( myEdgesTB, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool)) );
  QLabel* nbEdgesLab = new QLabel( QString("%1").arg(nbElements).toLatin1().data(), anEntitiesGrp );
  hl->addWidget( myEdgesTB, 1, 0 );
  hl->addWidget( nbEdgesLab, 1, 1 );
  myEdgesTB->setEnabled( nbElements );
  nbEdgesLab->setEnabled( nbElements );

  // Faces
  nbElements = myActor ? myActor->GetObject()->GetNbEntities( SMDSAbs_Face ) : aMesh->NbFaces();
  myFacesTB = new QCheckBox( tr("SMESH_FACES"), anEntitiesGrp );
  myFacesTB->setIcon( QIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_DLG_TRIANGLE" ) ) ) );
  bool hasFaces = myEntityMode & SMESH_Actor::eFaces;
  myFacesTB->setChecked( hasFaces );
  if ( hasFaces )
    myNbCheckedButtons++;
  connect( myFacesTB, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool)) );
  QLabel* nbFacesLab = new QLabel( QString("%1").arg(nbElements).toLatin1().data(), anEntitiesGrp );
  hl->addWidget( myFacesTB, 2, 0 );
  hl->addWidget( nbFacesLab, 2, 1 );
  myFacesTB->setEnabled( nbElements );
  nbFacesLab->setEnabled( nbElements );

  // Volumes
  nbElements = myActor ? myActor->GetObject()->GetNbEntities( SMDSAbs_Volume ) : aMesh->NbVolumes();
  myVolumesTB = new QCheckBox( tr("SMESH_VOLUMES"), anEntitiesGrp );
  myVolumesTB->setIcon( QIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_DLG_TETRAS" ) ) ) );
  bool hasVolumes = myEntityMode & SMESH_Actor::eVolumes;
  myVolumesTB->setChecked( hasVolumes );
  if ( hasVolumes )
    myNbCheckedButtons++;
  connect( myVolumesTB, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool) ) );
  QLabel* nbVolumesLab = new QLabel( QString("%1").arg(nbElements).toLatin1().data(), anEntitiesGrp );
  hl->addWidget( myVolumesTB, 3, 0 );
  hl->addWidget( nbVolumesLab, 3, 1 );
  myVolumesTB->setEnabled( nbElements );
  nbVolumesLab->setEnabled( nbElements );

  // Balls
  nbElements = myActor ? myActor->GetObject()->GetNbEntities( SMDSAbs_Ball ) : aMesh->NbBalls();
  myBallsTB = new QCheckBox( tr("SMESH_BALLS"), anEntitiesGrp );
  myBallsTB->setIcon( QIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_DLG_BALL" ) ) ) );
  bool hasBalls = myEntityMode & SMESH_Actor::eBallElem;
  myBallsTB->setChecked( hasBalls );
  if ( hasBalls )
    myNbCheckedButtons++;
  connect( myBallsTB, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool)) );
  QLabel* nbBallsLab = new QLabel( QString("%1").arg(nbElements).toLatin1().data(), anEntitiesGrp );
  hl->addWidget( myBallsTB, 4, 0 );
  hl->addWidget( nbBallsLab, 4, 1 );
  myBallsTB->setEnabled( nbElements );
  nbBallsLab->setEnabled( nbElements );

  QVBoxLayout* aDlgLay = new QVBoxLayout( mainFrame() );
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );
  aDlgLay->addWidget( anEntitiesGrp );
  
  button( OK )->setText( tr( "SMESH_BUT_OK" ) );

  connect( this, SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );
  connect( this, SIGNAL( dlgOk() ),   this, SLOT( onOk() ) );
}

/*
  \brief Destructor: clean-up resources if necessary
*/
SMESHGUI_DisplayEntitiesDlg::~SMESHGUI_DisplayEntitiesDlg()
{
}

void SMESHGUI_DisplayEntitiesDlg::InverseEntityMode(unsigned int& theOutputMode,
                                                    unsigned int theMode)
{
  bool anIsNotPresent = ~theOutputMode & theMode;
  if(anIsNotPresent)
    theOutputMode |= theMode;
  else
    theOutputMode &= ~theMode;
}

/*!
  \brief Slot for changing entities state
*/
void SMESHGUI_DisplayEntitiesDlg::onChangeEntityMode( bool isChecked )
{
  QCheckBox* aSender = (QCheckBox*)sender();
  if ( myNbCheckedButtons == 1 && !isChecked ) {
    SUIT_MessageBox::warning(this, tr("SMESH_WRN_WARNING"),
                             tr("WRN_AT_LEAST_ONE"));
    disconnect( aSender, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool)) );
    aSender->setChecked( true );
    connect( aSender, SIGNAL(toggled(bool)), this, SLOT(onChangeEntityMode(bool)) );
    return;
  }
  if ( my0DElemsTB == aSender )
    InverseEntityMode( myEntityMode, SMESH_Actor::e0DElements );
  else if ( myEdgesTB == aSender )
    InverseEntityMode( myEntityMode, SMESH_Actor::eEdges );
  else if ( myFacesTB == aSender )
    InverseEntityMode( myEntityMode, SMESH_Actor::eFaces );
  else if ( myVolumesTB == aSender )
    InverseEntityMode( myEntityMode, SMESH_Actor::eVolumes );
  else if ( myBallsTB == aSender )
    InverseEntityMode( myEntityMode, SMESH_Actor::eBallElem );
  
  isChecked ? myNbCheckedButtons++ : myNbCheckedButtons--;
  
}

/*!
  \brief Show online help on dialog box
*/
void SMESHGUI_DisplayEntitiesDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  app->onHelpContextModule( "SMESH", "display_entity_page.html" );
}

/*!
  \brief Display or update the mesh in the 3D view with selected entity mode
*/
void SMESHGUI_DisplayEntitiesDlg::onOk()
{
  SUIT_OverrideCursor wc;

  const char* entry = myIObject->getEntry();
  
  if ( !myActor ) {
    myActor = SMESH::CreateActor(SMESH::GetActiveStudyDocument(), 
                                 entry, true);
  }

  if( myEntityMode != myActor->GetEntityMode() ) {
    myActor->SetEntityMode(myEntityMode);
    SUIT_ViewWindow* wnd = SMESH::GetActiveWindow();
    SMESH::DisplayActor( wnd, myActor );
    SUIT_DataOwnerPtrList aList;
    aList.append( new LightApp_DataOwner( entry ) );
    SMESHGUI::selectionMgr()->setSelected( aList, false );
    SMESH::UpdateView( wnd, SMESH::eDisplay, entry );
  }
}
