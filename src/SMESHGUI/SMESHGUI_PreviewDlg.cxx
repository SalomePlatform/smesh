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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_PreviewDlg.cxx
// Author : Roman NIKOLAEV, Open CASCADE S.A.S.
// SMESH includes
//

//SMESH includes
#include "SMESHGUI.h"
#include "SMESHGUI_PreviewDlg.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Utils.h"

//GUI includes
#include <SUIT_Desktop.h>

//QT includes
#include <QCheckBox>


//=================================================================================
// class    : SMESHGUI_SMESHGUI_PreviewDlg()
// purpose  :
//=================================================================================
SMESHGUI_PreviewDlg::SMESHGUI_PreviewDlg(SMESHGUI* theModule) :
  QDialog(SMESH::GetDesktop( theModule )),
  mySMESHGUI(theModule),
  myIsApplyAndClose( false )
{
  mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()),            this, SLOT(onCloseView()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), this, SLOT(onOpenView()));
}

//=================================================================================
// function : ~SMESHGUI_PreviewDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_PreviewDlg::~SMESHGUI_PreviewDlg()
{
  delete mySimulation;
}

//=================================================================================
// function : showPreview
// purpose  : Show preview in the viewer
//=================================================================================
void SMESHGUI_PreviewDlg::showPreview(){
  if(mySimulation && mySimulation->GetActor())
    mySimulation->SetVisibility(true);
}

//=================================================================================
// function : hidePreview
// purpose  : Hide preview in the viewer
//=================================================================================
void SMESHGUI_PreviewDlg::hidePreview(){
  if(mySimulation && mySimulation->GetActor())
    mySimulation->SetVisibility(false);
}

//=================================================================================
// function : connectPreviewControl
// purpose  : Connect the preview check box
//=================================================================================
void SMESHGUI_PreviewDlg::connectPreviewControl(){
  connect(myPreviewCheckBox, SIGNAL(toggled(bool)), this, SLOT(onDisplaySimulation(bool)));
}

//=================================================================================
// function : toDisplaySimulation
// purpose  : 
//=================================================================================
void SMESHGUI_PreviewDlg::toDisplaySimulation() {
  onDisplaySimulation(true);
}

//=================================================================================
// function : onDisplaySimulation
// purpose  : 
//=================================================================================
void SMESHGUI_PreviewDlg::onDisplaySimulation(bool toDisplayPreview) {
  //Empty implementation here
}

//================================================================
// Function : setIsApplyAndClose
// Purpose  : Set value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
void SMESHGUI_PreviewDlg::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

//================================================================
// Function : isApplyAndClose
// Purpose  : Get value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
bool SMESHGUI_PreviewDlg::isApplyAndClose() const
{
  return myIsApplyAndClose;
}

//=================================================================================
// function : onCloseView()
// purpose  : SLOT called when close view
//=================================================================================
void SMESHGUI_PreviewDlg::onCloseView()
{
  if ( mySimulation && mySimulation->GetActor())
    mySimulation->SetVisibility(false);
  delete mySimulation;
  mySimulation=0;
}

//=================================================================================
// function : onOpenView()
// purpose  : SLOT called when open view
//=================================================================================
void SMESHGUI_PreviewDlg::onOpenView()
{
  if ( !mySimulation)
    mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));
}
//=================================================================================
// class    : SMESHGUI_SMESHGUI_MultiPreviewDlg()
// purpose  :
//=================================================================================
SMESHGUI_MultiPreviewDlg::SMESHGUI_MultiPreviewDlg( SMESHGUI* theModule ) :
  QDialog( SMESH::GetDesktop( theModule ) ),
  mySMESHGUI( theModule ),
  myIsApplyAndClose( false )
{
  mySimulationList.clear();
  connect(mySMESHGUI, SIGNAL(SignalCloseView()), this, SLOT(onCloseView()));
}

//=================================================================================
// function : ~SMESHGUI_MultiPreviewDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_MultiPreviewDlg::~SMESHGUI_MultiPreviewDlg()
{
  qDeleteAll( mySimulationList );
}

//=================================================================================
// function : showPreview
// purpose  : Show preview in the viewer
//=================================================================================
void SMESHGUI_MultiPreviewDlg::showPreview()
{
  for ( int i = 0; i < mySimulationList.count(); i++ )
    if(mySimulationList[i] && mySimulationList[i]->GetActor())
      mySimulationList[i]->SetVisibility( true );
}

//=================================================================================
// function : hidePreview
// purpose  : Hide preview in the viewer
//=================================================================================
void SMESHGUI_MultiPreviewDlg::hidePreview()
{
  for ( int i = 0; i < mySimulationList.count(); i++ )
    if(mySimulationList[i] && mySimulationList[i]->GetActor())
      mySimulationList[i]->SetVisibility( false );
}

//=================================================================================
// function : connectPreviewControl
// purpose  : Connect the preview check box
//=================================================================================
void SMESHGUI_MultiPreviewDlg::connectPreviewControl()
{
  connect( myPreviewCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onDisplaySimulation( bool ) ) );
}

//=================================================================================
// function : toDisplaySimulation
// purpose  : 
//=================================================================================
void SMESHGUI_MultiPreviewDlg::toDisplaySimulation()
{
  onDisplaySimulation( true );
}

//=================================================================================
// function : onDisplaySimulation
// purpose  : 
//=================================================================================
void SMESHGUI_MultiPreviewDlg::onDisplaySimulation( bool toDisplayPreview )
{
  //Empty implementation here
}

//================================================================
// Function : setIsApplyAndClose
// Purpose  : Set value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
void SMESHGUI_MultiPreviewDlg::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

//================================================================
// Function : isApplyAndClose
// Purpose  : Get value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
bool SMESHGUI_MultiPreviewDlg::isApplyAndClose() const
{
  return myIsApplyAndClose;
}

//================================================================
// Function : setSimulationPreview
// Purpose  : 
//================================================================
void SMESHGUI_MultiPreviewDlg::setSimulationPreview( QList<SMESH::MeshPreviewStruct_var>& theMeshPreviewStruct )
{
  hidePreview();
  qDeleteAll( mySimulationList );
  mySimulationList.clear();
  for ( int i = 0; i < theMeshPreviewStruct.count(); i++ ) {
    mySimulationList << new SMESHGUI_MeshEditPreview( SMESH::GetViewWindow( mySMESHGUI ) );
    mySimulationList[i]->SetData( theMeshPreviewStruct[i].operator->() );
  }
}

//=================================================================================
// function : onCloseView()
// purpose  : SLOT called when close view
//=================================================================================
void SMESHGUI_MultiPreviewDlg::onCloseView()
{
  qDeleteAll( mySimulationList );
  mySimulationList.clear();
}
