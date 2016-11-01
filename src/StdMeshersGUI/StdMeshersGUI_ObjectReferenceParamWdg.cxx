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

// File   : StdMeshersGUI_ObjectReferenceParamWdg.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "StdMeshersGUI_ObjectReferenceParamWdg.h"

#include <SMESHGUI.h>
#include <SMESHGUI_VTKUtils.h>
#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <LightApp_SelectionMgr.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// SALOME KERNEL incldues
#include <SALOMEDSClient_SObject.hxx>
#include <SALOMEDSClient_Study.hxx>

// Qt includes
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>

#define SPACING 6

//================================================================================
/*!
 * \brief Constructor initialized by filter
  * \param f - object filter
 */
//================================================================================

StdMeshersGUI_ObjectReferenceParamWdg::StdMeshersGUI_ObjectReferenceParamWdg
( SUIT_SelectionFilter* f, QWidget* parent, bool multiSelection )
  : QWidget( parent ), myMultiSelection( multiSelection )
{
  myFilter = f;
  init();
}

//================================================================================
/*!
 * \brief Constructor initialized by object type
  * \param MeshObjectType - type of object to select
 */
//================================================================================

StdMeshersGUI_ObjectReferenceParamWdg::StdMeshersGUI_ObjectReferenceParamWdg
( SMESH::MeshObjectType objType, QWidget* parent, bool multiSelection )
  : QWidget( parent ), myMultiSelection( multiSelection )
{
  myFilter = new SMESH_TypeFilter( objType );
  init();
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================

StdMeshersGUI_ObjectReferenceParamWdg::~StdMeshersGUI_ObjectReferenceParamWdg()
{
  if ( myFilter )
  {
    mySelectionMgr->removeFilter( myFilter );
    delete myFilter;
  }
}

//================================================================================
/*!
 * \brief Create a leayout, initialize fields
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::init()
{
  QHBoxLayout* aHBox = new QHBoxLayout(this);

  aHBox->setMargin(0);
  aHBox->setSpacing(SPACING);

  mySMESHGUI     = SMESHGUI::GetSMESHGUI();
  mySelectionMgr = SMESH::GetSelectionMgr( mySMESHGUI );
  mySelectionActivated = false;
  myParamValue = "";
  myEmptyText = "";
  myEmptyStyleSheet ="";

  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap iconSlct ( mgr->loadPixmap("SMESH", tr("ICON_SELECT")));

  mySelButton = new QPushButton(this);
  mySelButton->setIcon(iconSlct);
  mySelButton->setCheckable( true );

  myObjNameLineEdit = new QLineEdit(this);
  myObjNameLineEdit->setReadOnly(true);
  myObjNameLineEdit->setStyleSheet(myEmptyStyleSheet);
  myObjNameLineEdit->setMinimumWidth(150);

  aHBox->addWidget( mySelButton );
  aHBox->addWidget( myObjNameLineEdit );

  connect( mySelButton, SIGNAL(clicked()), SLOT(activateSelection()));
}

//================================================================================
/*!
 * \brief SLOT: Installs selection filter that is not done automatically
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::activateSelection()
{
  if ( !mySelectionActivated && mySelectionMgr )
  {
    mySelectionActivated = true;
    mySelectionMgr->clearFilters();
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    if ( myFilter )
      mySelectionMgr->installFilter( myFilter );
    connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  }
  emit selectionActivated();
  onSelectionDone();

  mySelButton->setChecked( mySelectionActivated );
}

//================================================================================
/*!
 * \brief SLOT: stop treating selection changes
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::deactivateSelection()
{
  mySelectionActivated = false;
  disconnect(mySelectionMgr, 0, this, 0 );
  mySelectionMgr->removeFilter( myFilter );

  mySelButton->setChecked( mySelectionActivated );
}

//================================================================================
/*!
 * \brief Connect selection slots
  * \param other - another StdMeshersGUI_ObjectReferenceParamWdg
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::AvoidSimultaneousSelection
                                   ( StdMeshersGUI_ObjectReferenceParamWdg* other)
{
  connect(other, SIGNAL(selectionActivated()), this, SLOT(deactivateSelection()));
  connect(this, SIGNAL(selectionActivated()), other, SLOT(deactivateSelection()));
}

//================================================================================
/*!
 * \brief Initialize selected object name
 * \param obj - the current object
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::SetObject(CORBA::Object_ptr obj)
{
  myObjects.clear();
  myObjNameLineEdit->setText( myEmptyText );
  myObjNameLineEdit->setStyleSheet(myEmptyStyleSheet);
  myParamValue = "";

  _PTR(SObject) sobj;
  if ( !CORBA::is_nil( obj ))
    sobj = SMESH::FindSObject (obj);
  if ( sobj ) {
    std::string name = sobj->GetName();
    myObjNameLineEdit->setText( QString( name.c_str() ).trimmed() );
    myObjNameLineEdit->setStyleSheet("");
    myObjects.push_back( CORBA::Object::_duplicate( obj ));
    myParamValue = sobj->GetID().c_str();
  }
  emit contentModified();
}

//================================================================================
/*!
 * \brief Initialize selected objects
 * \param objects - entries of objects
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::SetObjects(SMESH::string_array_var& objects)
{
  myObjects.clear();
  myObjNameLineEdit->setText( myEmptyText );
  myObjNameLineEdit->setStyleSheet(myEmptyStyleSheet);
  myParamValue = "";
  bool selChanged = false;
  
  for ( unsigned i = 0; i < objects->length(); ++i )
  {
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    _PTR(SObject) aSObj = aStudy->FindObjectID(objects[i].in());
    CORBA::Object_var anObj = SMESH::SObjectToObject(aSObj,aStudy);
    if ( !CORBA::is_nil( anObj )) {
      std::string name = aSObj->GetName();
      QString text = myObjNameLineEdit->text();
      if ( text != myEmptyText )
        text += " ";
      else
        text = "";
      text += QString( name.c_str() ).trimmed();
      myObjNameLineEdit->setText( text );
      myObjNameLineEdit->setStyleSheet("");
      myObjects.push_back( anObj );
      myParamValue += " ";
      myParamValue += objects[i];
      selChanged = true;
    }
  }
  if (selChanged)
    emit contentModified();
}

//================================================================================
/*!
 * \brief Takes selected object
 */
//================================================================================

void StdMeshersGUI_ObjectReferenceParamWdg::onSelectionDone()
{
  if ( mySelectionActivated ) {
    CORBA::Object_var obj;
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList);
    if (aList.Extent() == 1)
    {
      obj = SMESH::IObjectToObject( aList.First() );
      SetObject( obj.in() );
    }
    else if (myMultiSelection)
    {
      SMESH::string_array_var objIds = new SMESH::string_array;
      objIds->length( aList.Extent());
      SALOME_ListIteratorOfListIO io( aList );
      int i = 0;
      for ( ; io.More(); io.Next(), ++i )
      {
        Handle(SALOME_InteractiveObject) anIO = io.Value();
        if ( anIO->hasEntry() )
          objIds[i] = anIO->getEntry();
        else
          i--;
      }
      objIds->length(i);
      SetObjects( objIds );
    }
  }
}

void StdMeshersGUI_ObjectReferenceParamWdg::SetDefaultText(QString defaultText, QString styleSheet)
{
  myEmptyText = defaultText;
  myEmptyStyleSheet = styleSheet;
  myObjNameLineEdit->setText( myEmptyText );
  myObjNameLineEdit->setStyleSheet( myEmptyStyleSheet);
}
