//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_StandardMeshInfosDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_StandardMeshInfosDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"

#include <SMESH_TypeFilter.hxx>

// SALOME KERNEL includes 
#include <SALOMEDSClient_Study.hxx>
#include <SALOMEDSClient_SObject.hxx>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SALOME_ListIO.hxx>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

#define SPACING 6
#define MARGIN  11

//=================================================================================
/*!
 *  SMESHGUI_StandardMeshInfosDlg::SMESHGUI_StandardMeshInfosDlg
 *
 *  Constructor
 */
//=================================================================================
SMESHGUI_StandardMeshInfosDlg::SMESHGUI_StandardMeshInfosDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_STANDARD_MESHINFO_TITLE"));
  setSizeGripEnabled(true);

  myStartSelection = true;
  myIsActiveWindow = true;

  // dialog layout
  QVBoxLayout* aDlgLayout = new QVBoxLayout(this);
  aDlgLayout->setSpacing(SPACING);
  aDlgLayout->setMargin(MARGIN);

  // mesh group box
  myMeshGroup = new QGroupBox(tr("SMESH_MESH"), this);
  QHBoxLayout* myMeshGroupLayout = new QHBoxLayout(myMeshGroup);
  myMeshGroupLayout->setSpacing(SPACING);
  myMeshGroupLayout->setMargin(MARGIN);

  // select button, label and line edit with mesh name
  myNameLab = new QLabel(tr("SMESH_NAME"), myMeshGroup);
  myMeshGroupLayout->addWidget(myNameLab);

  QPixmap image0(SUIT_Session::session()->resourceMgr()->loadPixmap("SMESH",tr("ICON_SELECT")));
  mySelectBtn = new QPushButton(myMeshGroup);
  mySelectBtn->setIcon(image0);
  myMeshGroupLayout->addWidget(mySelectBtn);

  myMeshLine = new QLineEdit(myMeshGroup);
  myMeshGroupLayout->addWidget(myMeshLine);

  aDlgLayout->addWidget(myMeshGroup);

  // information group box
  myInfoGroup  = new QGroupBox(tr("SMESH_INFORMATION"), this);
  QVBoxLayout* myInfoGroupLayout = new QVBoxLayout(myInfoGroup);
  myInfoGroupLayout->setSpacing(SPACING);
  myInfoGroupLayout->setMargin(MARGIN);

  // information text browser
  myInfo = new QTextBrowser(myInfoGroup);
  myInfo->setMinimumSize(200, 150);
  myInfoGroupLayout->addWidget(myInfo);

  aDlgLayout->addWidget(myInfoGroup);

  // buttons group
  myButtonsGroup = new QGroupBox(this);
  QHBoxLayout* myButtonsGroupLayout = new QHBoxLayout(myButtonsGroup);
  myButtonsGroupLayout->setSpacing(SPACING);
  myButtonsGroupLayout->setMargin(MARGIN);

  // buttons --> OK and Help buttons
  myOkBtn = new QPushButton(tr("SMESH_BUT_OK"), myButtonsGroup);
  myOkBtn->setAutoDefault(true); myOkBtn->setDefault(true);
  myHelpBtn = new QPushButton(tr("SMESH_BUT_HELP"), myButtonsGroup);
  myHelpBtn->setAutoDefault(true);

  myButtonsGroupLayout->addWidget(myOkBtn);
  myButtonsGroupLayout->addSpacing(10);
  myButtonsGroupLayout->addStretch();
  myButtonsGroupLayout->addWidget(myHelpBtn);

  aDlgLayout->addWidget(myButtonsGroup);

  mySMESHGUI->SetActiveDialogBox(this);

  // connect signals
  connect( myOkBtn,         SIGNAL(clicked()),                      this, SLOT(close()));
  connect( myHelpBtn,       SIGNAL(clicked()),                      this, SLOT(onHelp()));
  connect( mySelectBtn,     SIGNAL(clicked()),                      this, SLOT(onStartSelection()));
  connect( mySMESHGUI,      SIGNAL(SignalCloseAllDialogs()),        this, SLOT(close()));
  connect( mySMESHGUI,      SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect( mySelectionMgr,  SIGNAL(currentSelectionChanged()),      this, SLOT(onSelectionChanged()));

  // init dialog with current selection
  myMeshFilter = new SMESH_TypeFilter (MESH);
  mySelectionMgr->installFilter(myMeshFilter);
  onSelectionChanged();

  myHelpFileName = "mesh_infos_page.html#standard_mesh_infos_anchor";
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
  SUIT_OverrideCursor wc;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  int nbSel = aList.Extent();
  myInfo->clear();
  if (nbSel == 1) {
    myStartSelection = false;
    myMeshLine->setText("");
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(aList.First());

    if (!aMesh->_is_nil()) {
      QString aName, anInfo;
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aName);
      myMeshLine->setText(aName);
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
      else if(aNbFaces > 0) {
        aNbDimElements = aNbFaces;
        aDimension = 2;
      }
      else if(aNbEdges > 0) {
        aNbDimElements = aNbEdges;
        aDimension = 1;
      }
      else if(aNbNodes > 0) {
        aNbDimElements = aNbNodes;
        aDimension = 0;
      }

      // information about the mesh
      anInfo.append(QString("Nb of element of dimension %1:<b> %2</b><br>").arg(aDimension).arg(aNbDimElements));
      anInfo.append(QString("Nb of nodes: <b>%1</b><br><br>").arg(aNbNodes));

      // information about the groups of the mesh
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
      _PTR(SObject) aMeshSO = SMESH::FindSObject(aMesh);
      _PTR(SObject) anObj;

      bool hasGroup = false;

      // info about groups on nodes
      aMeshSO->FindSubObject(SMESH::Tag_NodeGroups, anObj);
      if (anObj) {
        _PTR(ChildIterator) it = aStudy->NewChildIterator(anObj);
        if (it->More()) {
          anInfo.append(QString("Groups:<br><br>"));
          hasGroup = true;
        }
        for ( ; it->More(); it->Next()) {
          _PTR(SObject) subObj = it->Value();
          CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
          SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
          if (!aGroup->_is_nil()) {
            anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
            anInfo.append(QString("%1<br>").arg("on nodes"));
            anInfo.append(QString("%1<br>").arg(aGroup->Size()));
            // check if the group based on geometry
            SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
            if (!aGroupOnGeom->_is_nil()) {
              GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
              QString aShapeName = "<unknown>";
              _PTR(SObject) aGeomObj, aRef;
              if (subObj->FindSubObject(1, aGeomObj) &&  aGeomObj->ReferencedObject(aRef))
                aShapeName = aRef->GetName().c_str();
              anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
            } else {
              anInfo.append(QString("<br>"));
            }
          }
        }
      }

      // info about groups on edges
      anObj.reset();
      aMeshSO->FindSubObject(SMESH::Tag_EdgeGroups, anObj);
      if (anObj) {
        _PTR(ChildIterator) it = aStudy->NewChildIterator(anObj);
        if (!hasGroup && it->More()) {
          anInfo.append(QString("Groups:<br><br>"));
          hasGroup = true;
        }
        for ( ; it->More(); it->Next()) {
          _PTR(SObject) subObj = it->Value();
          CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
          SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
          if (!aGroup->_is_nil()) {
            anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
            anInfo.append(QString("%1<br>").arg("on edges"));
            anInfo.append(QString("%1<br>").arg(aGroup->Size()));
            // check if the group based on geometry
            SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
            if (!aGroupOnGeom->_is_nil()) {
              GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
              QString aShapeName = "<unknown>";
              _PTR(SObject) aGeomObj, aRef;
              if (subObj->FindSubObject(1, aGeomObj) && aGeomObj->ReferencedObject(aRef))
                aShapeName = aRef->GetName().c_str();
              anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
            } else {
              anInfo.append(QString("<br>"));
            }
          }
        }
      }

      // info about groups on faces
      anObj.reset();
      aMeshSO->FindSubObject(SMESH::Tag_FaceGroups, anObj);
      if (anObj) {
        _PTR(ChildIterator) it = aStudy->NewChildIterator(anObj);
        if (!hasGroup && it->More()) {
          anInfo.append(QString("Groups:<br><br>"));
          hasGroup = true;
        }
        for ( ; it->More(); it->Next()) {
          _PTR(SObject) subObj = it->Value();
          CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
          SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
          if (!aGroup->_is_nil()) {
            anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
            anInfo.append(QString("%1<br>").arg("on faces"));
            anInfo.append(QString("%1<br>").arg(aGroup->Size()));
            // check if the group based on geometry
            SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
            if (!aGroupOnGeom->_is_nil()) {
              GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
              QString aShapeName = "<unknown>";
              _PTR(SObject) aGeomObj, aRef;
              if (subObj->FindSubObject(1, aGeomObj) && aGeomObj->ReferencedObject(aRef))
                aShapeName = aRef->GetName().c_str();
              anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
            } else {
              anInfo.append(QString("<br>"));
            }
          }
        }
      }

      // info about groups on volumes
      anObj.reset();
      aMeshSO->FindSubObject(SMESH::Tag_VolumeGroups, anObj);
      if (anObj) {
        _PTR(ChildIterator) it = aStudy->NewChildIterator(anObj);
        if (!hasGroup && it->More())
          anInfo.append(QString("Groups:<br>"));
        for ( ; it->More(); it->Next()) {
          _PTR(SObject) subObj = it->Value();
          CORBA::Object_var anObject = SMESH::SObjectToObject(subObj);
          SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(anObject);
          if (!aGroup->_is_nil()) {
            anInfo.append(QString("-   <b>%1</b><br>").arg(aGroup->GetName()));
            anInfo.append(QString("%1<br>").arg("on volumes"));
            anInfo.append(QString("%1<br>").arg(aGroup->Size()));
            // check if the group based on geometry
            SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow(aGroup);
            if (!aGroupOnGeom->_is_nil()) {
              GEOM::GEOM_Object_var aGroupMainShape = aGroupOnGeom->GetShape();
              QString aShapeName = "<unknown>";
              _PTR(SObject) aGeomObj, aRef;
              if (subObj->FindSubObject(1, aGeomObj) &&  aGeomObj->ReferencedObject(aRef))
                aShapeName = aRef->GetName().c_str();
              anInfo.append(QString("based on <i>%1</i> geometry object<br><br>").arg(aShapeName));
            } else {
              anInfo.append(QString("<br>"));
            }
          }
        }
      }

      myInfo->setText(anInfo);
      return;
    }
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::onSelectionChanged()
{
  if (myStartSelection)
    DumpMeshInfos();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::closeEvent (QCloseEvent* e)
{
  mySelectionMgr->clearFilters();
  mySMESHGUI->ResetState();
  QDialog::closeEvent(e);
}

//=================================================================================
// function : windowActivationChange()
// purpose  : called when window is activated/deactivated
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::windowActivationChange (bool oldActive)
{
  QDialog::windowActivationChange(oldActive);
  if (isActiveWindow() && myIsActiveWindow != isActiveWindow())
    ActivateThisDialog();
  myIsActiveWindow = isActiveWindow();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::DeactivateActiveDialog()
{
  disconnect(mySelectionMgr, 0, this, 0);
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(onSelectionChanged()));
}

//=================================================================================
// function : onStartSelection()
// purpose  : starts selection
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::onStartSelection()
{
  myStartSelection = true;
  mySelectionMgr->installFilter(myMeshFilter);
  myMeshLine->setText(tr("Select a mesh"));
  onSelectionChanged();
  myStartSelection = true;
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                                                 platform)).
                             arg(myHelpFileName));
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_StandardMeshInfosDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}
