//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_StandardMeshInfosDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header$

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtextbrowser.h>
#include <qmap.h>
#include <qpushbutton.h>

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_WaitCursor.h"

#include "SMESHGUI_StandardMeshInfosDlg.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI.h"

#include "SMESH.hxx"

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(GEOM_Gen)

#include "utilities.h"

using namespace std;


//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::SMESHGUI_StandardMeshInfosDlg
 * 
 *  Constructor
 */
//=================================================================================
SMESHGUI_StandardMeshInfosDlg::SMESHGUI_StandardMeshInfosDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
     : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose )
{
  if ( !name )
      setName( "SMESHGUI_StandardMeshInfosDlg" );
  setCaption( tr( "SMESH_STANDARD_MESHINFO_TITLE"  ) );
  setSizeGripEnabled( TRUE );

  myStartSelection = true;
  myIsActiveWindow = true;

  // dialog layout
  QGridLayout* aDlgLayout = new QGridLayout( this ); 
  aDlgLayout->setSpacing( 6 );
  aDlgLayout->setMargin( 11 );
  
  // mesh group box
  myMeshGroup = new QGroupBox( this, "myMeshGroup" );
  myMeshGroup->setTitle( tr( "SMESH_MESH" ) );
  myMeshGroup->setColumnLayout(0, Qt::Vertical );
  myMeshGroup->layout()->setSpacing( 0 );
  myMeshGroup->layout()->setMargin( 0 );
  QGridLayout* myMeshGroupLayout = new QGridLayout( myMeshGroup->layout() );
  myMeshGroupLayout->setAlignment( Qt::AlignTop );
  myMeshGroupLayout->setSpacing( 6 );
  myMeshGroupLayout->setMargin( 11 );
  
  // select button, label and line edit with mesh name
  myNameLab = new QLabel( myMeshGroup, "myNameLab" );
  myNameLab->setText( tr( "SMESH_NAME"  ) );
  myMeshGroupLayout->addWidget( myNameLab, 0, 0 );

  QPixmap image0( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr( "ICON_SELECT" ) ) );
  mySelectBtn = new QPushButton( myMeshGroup, "mySelectBtn" );
  mySelectBtn->setPixmap( image0 );
  mySelectBtn->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  myMeshGroupLayout->addWidget( mySelectBtn, 0, 1 );
  
  myMeshLine = new QLineEdit( myMeshGroup, "myMeshLine" );
  myMeshGroupLayout->addWidget( myMeshLine, 0, 2 );
  
  aDlgLayout->addWidget( myMeshGroup, 0, 0 );

  // information group box
  myInfoGroup  = new QGroupBox( this, "myInfoGroup" );
  myInfoGroup->setTitle( tr( "SMESH_INFORMATION" ) );
  myInfoGroup->setColumnLayout(0, Qt::Vertical );
  myInfoGroup->layout()->setSpacing( 0 );
  myInfoGroup->layout()->setMargin( 0 );
  QGridLayout* myInfoGroupLayout = new QGridLayout( myInfoGroup->layout() );
  myInfoGroupLayout->setAlignment( Qt::AlignTop );
  myInfoGroupLayout->setSpacing( 6 );
  myInfoGroupLayout->setMargin( 11 );
  
  // information text browser
  myInfo = new QTextBrowser(myInfoGroup, "myInfo");
  myInfoGroupLayout->addWidget( myInfo, 0, 0 );

  aDlgLayout->addWidget( myInfoGroup, 1, 0 );

  // buttons group
  myButtonsGroup = new QGroupBox( this, "myButtonsGroup" );
  myButtonsGroup->setColumnLayout(0, Qt::Vertical );
  myButtonsGroup->layout()->setSpacing( 0 );  myButtonsGroup->layout()->setMargin( 0 );
  QHBoxLayout* myButtonsGroupLayout = new QHBoxLayout( myButtonsGroup->layout() );
  myButtonsGroupLayout->setAlignment( Qt::AlignTop );
  myButtonsGroupLayout->setSpacing( 6 ); myButtonsGroupLayout->setMargin( 11 );
  
  // buttons --> OK button
  myOkBtn = new QPushButton( tr( "SMESH_BUT_OK"  ), myButtonsGroup, "myOkBtn" );
  myOkBtn->setAutoDefault( TRUE ); myOkBtn->setDefault( TRUE );
  myButtonsGroupLayout->addStretch();
  myButtonsGroupLayout->addWidget( myOkBtn );
  myButtonsGroupLayout->addStretch();
  
  aDlgLayout->addWidget( myButtonsGroup, 2, 0 );
  
  mySelection = SALOME_Selection::Selection( SMESHGUI::GetSMESHGUI()->GetActiveStudy()->getSelection() );
  SMESHGUI::GetSMESHGUI()->SetActiveDialogBox( this ) ;

  // connect signals
  connect( myOkBtn,                  SIGNAL( clicked() ),                      this, SLOT( close() ) );
  connect( mySelectBtn,              SIGNAL( clicked() ),                      this, SLOT( onStartSelection() ) );
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalCloseAllDialogs() ),        this, SLOT( close() ) ) ;
  connect( SMESHGUI::GetSMESHGUI(),  SIGNAL( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection,              SIGNAL( currentSelectionChanged() ),      this, SLOT( onSelectionChanged() ) );

  // resize and move dialog, then show
  this->setMinimumSize(270, 428);
  int x, y;
  SMESHGUI::GetSMESHGUI()->DefineDlgPosition( this, x, y );
  this->move( x, y );
  this->show();
  
  // init dialog with current selection
  myMeshFilter = new SMESH_TypeFilter( MESH );
  mySelection->AddFilter( myMeshFilter );
  onSelectionChanged();
}

//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::~SMESHGUI_StandardMeshInfosDlg
 * 
 *  Destructor
 */
//=================================================================================
SMESHGUI_StandardMeshInfosDlg::~SMESHGUI_StandardMeshInfosDlg()
{
}

//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::DumpMeshInfos
 */
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::DumpMeshInfos()
{
  QAD_WaitCursor wc;
  int nbSel = mySelection->IObjectCount();
  myInfo->clear();
  if ( nbSel == 1 ) {
    myStartSelection = false;
    myMeshLine->setText( "" );
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( mySelection->firstIObject() );
    
    if ( !aMesh->_is_nil() ) {
      QString aName, anInfo;
      SMESH::GetNameOfSelectedIObjects(mySelection, aName);
      myMeshLine->setText( aName );
      int aNbNodes =   (int)aMesh->NbNodes();
      int aNbEdges =   (int)aMesh->NbEdges();
      int aNbFaces =   (int)aMesh->NbFaces();
      int aNbVolumes = (int)aMesh->NbVolumes();
      
      int aDimension = 0;
      double aNbDimElements = 0;
      if (aNbVolumes > 0) {
	aNbDimElements = aNbVolumes;
	aDimension = 3;
      }
      else if(aNbFaces > 0 ) {
	aNbDimElements = aNbFaces;
	aDimension = 2;
      }
      else if(aNbEdges > 0 ) {
	aNbDimElements = aNbEdges;
	aDimension = 1;
      }
      else if(aNbNodes > 0 ) {
	aNbDimElements = aNbNodes;
	aDimension = 0;
      }
      
      // information about the mesh
      anInfo.append(QString("Nb of element of dimension %1:<b> %2</b><br>").arg(aDimension).arg(aNbDimElements));
      anInfo.append(QString("Nb of nodes: <b>%1</b><br><br>").arg(aNbNodes));

      // information about the groups of the mesh
      SALOMEDS::Study_var aStudy = SMESH::GetActiveStudyDocument();
      SALOMEDS::SObject_var aMeshSO = aStudy->FindObjectIOR( aStudy->ConvertObjectToIOR(aMesh) ); 
      SALOMEDS::SObject_var anObj;
      
      bool hasGroup = false;

      // info about groups on nodes
      aMeshSO->FindSubObject(Tag_NodeGroups , anObj);
      if ( !anObj->_is_nil() )
	{
	  SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(anObj);
	  if (it->More())
	    {
	      anInfo.append(QString("Groups:<br><br>"));
	      hasGroup = true;
	    }
	  for(; it->More(); it->Next()){
	    SALOMEDS::SObject_var subObj = it->Value();
	    SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( subObj->GetObject() );
	    if ( !aGroup->_is_nil() )
	      {
		anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
		anInfo.append(QString("%1<br>").arg("on nodes"));
		anInfo.append(QString("%1<br>").arg(aGroup->Size()));
		// check if the group based on geometry
		SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
		if ( !aGroupOnGeom->_is_nil() )
		  {
		    GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
		    QString aShapeName = "<unknown>";
		    SALOMEDS::SObject_var aGeomObj, aRef;
		    if ( subObj->FindSubObject( 1, aGeomObj ) &&  aGeomObj->ReferencedObject( aRef ))
		      aShapeName = aRef->GetName();
		    anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
		  }
		else
		  anInfo.append(QString("<br>"));
	      }
	  }
	}  
      
      // info about groups on edges
      anObj = SALOMEDS::SObject::_nil();
      aMeshSO->FindSubObject(Tag_EdgeGroups , anObj);
      if ( !anObj->_is_nil() )
	{
	  SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(anObj);
	  if (!hasGroup && it->More())
	    {
	      anInfo.append(QString("Groups:<br><br>"));
	      hasGroup = true;
	    }
	  for(; it->More(); it->Next()){
	    SALOMEDS::SObject_var subObj = it->Value();
	    SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( subObj->GetObject() );
	    if ( !aGroup->_is_nil() )
	      {
		anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
		anInfo.append(QString("%1<br>").arg("on edges"));
		anInfo.append(QString("%1<br>").arg(aGroup->Size()));
		// check if the group based on geometry
		SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
		if ( !aGroupOnGeom->_is_nil() )
		  {
		    GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
		    QString aShapeName = "<unknown>";
		    SALOMEDS::SObject_var aGeomObj, aRef;
		    if ( subObj->FindSubObject( 1, aGeomObj ) &&  aGeomObj->ReferencedObject( aRef ))
		      aShapeName = aRef->GetName();
		    anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
		  }
		else
		  anInfo.append(QString("<br>"));
	      }
	  }
	}
      
      // info about groups on faces
      anObj = SALOMEDS::SObject::_nil();
      aMeshSO->FindSubObject(Tag_FaceGroups , anObj);
      if ( !anObj->_is_nil() )
	{
	  SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(anObj);
	  if (!hasGroup && it->More())
	    {
	      anInfo.append(QString("Groups:<br><br>"));
	      hasGroup = true;
	    }
	  for(; it->More(); it->Next()){
	    SALOMEDS::SObject_var subObj = it->Value();
	    SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( subObj->GetObject() );
	    if ( !aGroup->_is_nil() )
	      {
		anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
		anInfo.append(QString("%1<br>").arg("on faces"));
		anInfo.append(QString("%1<br>").arg(aGroup->Size()));
		// check if the group based on geometry
		SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
		if ( !aGroupOnGeom->_is_nil() )
		  {
		    GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
		    QString aShapeName = "<unknown>";
		    SALOMEDS::SObject_var aGeomObj, aRef;
		    if ( subObj->FindSubObject( 1, aGeomObj ) &&  aGeomObj->ReferencedObject( aRef ))
		      aShapeName = aRef->GetName();
		    anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
		  }
		else
		  anInfo.append(QString("<br>"));
	      }
	  }
	}
      
      // info about groups on volumes
      anObj = SALOMEDS::SObject::_nil();
      aMeshSO->FindSubObject(Tag_VolumeGroups , anObj);
      if ( !anObj->_is_nil() )
	{
	  SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator(anObj);
	  if (!hasGroup && it->More())
	    anInfo.append(QString("Groups:<br>"));
	  for(; it->More(); it->Next()){
	    SALOMEDS::SObject_var subObj = it->Value();
	    SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( subObj->GetObject() );
	    if ( !aGroup->_is_nil() )
	      {
		anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
		anInfo.append(QString("%1<br>").arg("on volumes"));
		anInfo.append(QString("%1<br>").arg(aGroup->Size()));
		// check if the group based on geometry
		SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
		if ( !aGroupOnGeom->_is_nil() )
		  {
		    GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
		    QString aShapeName = "<unknown>";
		    SALOMEDS::SObject_var aGeomObj, aRef;
		    if ( subObj->FindSubObject( 1, aGeomObj ) &&  aGeomObj->ReferencedObject( aRef ))
		      aShapeName = aRef->GetName();
		    anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
		  }
		else
		  anInfo.append(QString("<br>"));
	      }
	  }
	}
      
      myInfo->setText(anInfo);
      return;
    }
  }
  
  return;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::onSelectionChanged()
{
  if ( myStartSelection )
    DumpMeshInfos();
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::closeEvent( QCloseEvent* e )
{
  mySelection->ClearFilters();
  SMESHGUI::GetSMESHGUI()->ResetState();
  QDialog::closeEvent( e );
}


//=================================================================================
// function : windowActivationChange()
// purpose  : called when window is activated/deactivated
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::windowActivationChange( bool oldActive )
{
  QDialog::windowActivationChange( oldActive );
  if ( isActiveWindow() && myIsActiveWindow != isActiveWindow() )
    ActivateThisDialog() ;
  myIsActiveWindow = isActiveWindow();
}


//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::DeactivateActiveDialog()
{
  disconnect( mySelection, 0, this, 0 );
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog() ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
}

//=================================================================================
// function : onStartSelection()
// purpose  : starts selection
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::onStartSelection()
{
  myStartSelection = true;
  mySelection->AddFilter( myMeshFilter ) ;
  myMeshLine->setText( tr( "Select a mesh" ) );
  onSelectionChanged();
  myStartSelection = true;
}
