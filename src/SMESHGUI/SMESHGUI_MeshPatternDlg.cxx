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
//  File   : SMESHGUI_MeshPatternDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_MeshPatternDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_CreatePatternDlg.h"
#include "SMESHGUI_PatternWidget.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_NumberFilter.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshElement.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"
#include "SUIT_FileDlg.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "LightApp_SelectionMgr.h"
#include "SalomeApp_Tools.h"
#include "SalomeApp_Study.h"
#include "LightApp_Application.h"

#include "SALOMEDS_SObject.hxx"

#include "SALOME_ListIO.hxx"
#include "SVTK_Selection.h"

#include "SVTK_ViewModel.h"
#include "SVTK_Selector.h"
#include "SVTK_ViewWindow.h"
#include "VTKViewer_CellLocationsArray.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

// QT Includes
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qmessagebox.h>
#include <qcstring.h>
#include <qspinbox.h>
#include <qvaluelist.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfiledialog.h>

// VTK Includes
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

#define SPACING 5
#define MARGIN  10

/*!
 *  Class       : SMESHGUI_MeshPatternDlg
 *  Description : Dialog to specify filters for VTK viewer
 */

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::SMESHGUI_MeshPatternDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_MeshPatternDlg::SMESHGUI_MeshPatternDlg( SMESHGUI*   theModule,
                                                  const char* theName )
     : QDialog( SMESH::GetDesktop( theModule ), theName, false, WStyle_Customize |
                WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu),
       myBusy(false),
       mySMESHGUI( theModule ),
       mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  setCaption(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout(this, MARGIN, SPACING);

  QFrame* aMainFrame = createMainFrame  (this);
  QFrame* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);

  myCreationDlg = 0;

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  myHelpFileName = "pattern_mapping.htm";

  Init();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_MeshPatternDlg::createMainFrame (QWidget* theParent)
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap iconSlct ( mgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap icon2d   ( mgr->loadPixmap("SMESH", tr("ICON_PATTERN_2d")));
  QPixmap icon3d   ( mgr->loadPixmap("SMESH", tr("ICON_PATTERN_3d")));
  QPixmap iconOpen ( mgr->loadPixmap("STD", tr("ICON_FILE_OPEN")));

  QPixmap iconSample2d ( mgr->loadPixmap("SMESH", tr("ICON_PATTERN_SAMPLE_2D")));
  QPixmap iconSample3d ( mgr->loadPixmap("SMESH", tr("ICON_PATTERN_SAMPLE_3D")));

  QGroupBox* aMainGrp = new QGroupBox (1, Qt::Horizontal, theParent);
  aMainGrp->setFrameStyle(QFrame::NoFrame);
  aMainGrp->setInsideMargin(0);

  // Pattern type group

  myTypeGrp = new QButtonGroup (1, Qt::Vertical, tr("PATTERN_TYPE"), aMainGrp);
  mySwitch2d = new QRadioButton (myTypeGrp);
  mySwitch3d = new QRadioButton (myTypeGrp);
  mySwitch2d->setPixmap(icon2d);
  mySwitch3d->setPixmap(icon3d);
  myTypeGrp->insert(mySwitch2d, Type_2d);
  myTypeGrp->insert(mySwitch3d, Type_3d);

  // Mesh group

  QGroupBox* aMeshGrp = new QGroupBox(1, Qt::Vertical, tr("SMESH_MESH"), aMainGrp);
  new QLabel(tr("SMESH_MESH"), aMeshGrp);
  mySelBtn[ Mesh ] = new QPushButton(aMeshGrp);
  mySelBtn[ Mesh ]->setPixmap(iconSlct);
  mySelEdit[ Mesh ] = new QLineEdit(aMeshGrp);
  mySelEdit[ Mesh ]->setReadOnly(true);

  // Pattern group

  QGroupBox* aPatGrp = new QGroupBox(1, Qt::Horizontal, tr("PATTERN"), aMainGrp);

  // pattern name
  QGroupBox* aNameGrp = new QGroupBox(1, Qt::Vertical, aPatGrp);
  aNameGrp->setFrameStyle(QFrame::NoFrame);
  aNameGrp->setInsideMargin(0);
  new QLabel(tr("PATTERN"), aNameGrp);
  myName = new QLineEdit(aNameGrp);
  myName->setReadOnly(true);
  myOpenBtn = new QPushButton(aNameGrp);
  myOpenBtn->setPixmap(iconOpen);
  myNewBtn = new QPushButton(tr("NEW"), aNameGrp);

  // Mode selection check box
  myRefine = new QCheckBox(tr("REFINE"), aPatGrp);

  // selection widgets for Apply to geom mode
  myGeomGrp = new QGroupBox(3, Qt::Horizontal, aPatGrp);
  myGeomGrp->setFrameStyle(QFrame::NoFrame);
  myGeomGrp->setInsideMargin(0);

  for (int i = Object; i <= Vertex2; i++)
  {
    mySelLbl[ i ] = new QLabel(myGeomGrp);
    mySelBtn[ i ] = new QPushButton(myGeomGrp);
    mySelBtn[ i ]->setPixmap(iconSlct);
    mySelEdit[ i ] = new QLineEdit(myGeomGrp);
    mySelEdit[ i ]->setReadOnly(true);
  }

  // Widgets for refinement of existing mesh elements
  myRefineGrp = new QFrame(aPatGrp);
  myRefineGrp->setFrameStyle(QFrame::NoFrame);
  QGridLayout* aRefGrid = new QGridLayout(myRefineGrp, 3, 3, 0, 5);

  mySelLbl[ Ids ] = new QLabel(myRefineGrp);
  mySelBtn[ Ids ] = new QPushButton(myRefineGrp);
  mySelBtn[ Ids ]->setPixmap(iconSlct);
  mySelEdit[ Ids ] = new QLineEdit(myRefineGrp);

  QLabel* aNodeLbl = new QLabel(tr("NODE_1"), myRefineGrp);
  myNode1          = new QSpinBox(myRefineGrp);
  myNode2Lbl       = new QLabel(tr("NODE_2"), myRefineGrp);
  myNode2          = new QSpinBox(myRefineGrp);

  aRefGrid->addWidget(mySelLbl [ Ids ], 0, 0);
  aRefGrid->addWidget(mySelBtn [ Ids ], 0, 1);
  aRefGrid->addWidget(mySelEdit[ Ids ], 0, 2);
  aRefGrid->addWidget(aNodeLbl, 1, 0);
  aRefGrid->addMultiCellWidget(myNode1, 1, 1, 1, 2);
  aRefGrid->addWidget(myNode2Lbl, 2, 0);
  aRefGrid->addMultiCellWidget(myNode2, 2, 2, 1, 2);

  // reverse check box
  myReverseChk = new QCheckBox(tr("REVERSE"), aPatGrp);

  // CreatePoly check box
  myCreatePolygonsChk = new QCheckBox( tr( "CREATE_POLYGONS_NEAR_BOUNDARY" ), aPatGrp );
  myCreatePolyedrsChk = new QCheckBox( tr( "CREATE_POLYEDRS_NEAR_BOUNDARY" ), aPatGrp );

  // Pictures 2d and 3d
  for (int i = 0; i < 2; i++) {
    if (i == 0) {
      myPicture2d = new SMESHGUI_PatternWidget(aPatGrp),
      myPicture2d->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    } else {
      myPicture3d = new QFrame(aPatGrp),
      myPreview3d = new QLabel(myPicture3d);
      myPreview3d->setPixmap(iconSample3d);
      QGridLayout* aLay = new QGridLayout(myPicture3d, 3, 3, 0, 0);
      QSpacerItem* aSpacerH1 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
      QSpacerItem* aSpacerH2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
      QSpacerItem* aSpacerV1 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      QSpacerItem* aSpacerV2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      aLay->addItem(aSpacerH1, 1, 0);
      aLay->addItem(aSpacerH2, 1, 2);
      aLay->addItem(aSpacerV1, 0, 1);
      aLay->addItem(aSpacerV2, 2, 1);
      aLay->addWidget(myPreview3d, 1, 1);
    }
  }

  myPreviewChk = new QCheckBox(tr("PREVIEW"), aPatGrp);

  // Connect signals and slots

  connect(myTypeGrp, SIGNAL(clicked(int)), SLOT(onTypeChanged(int)));
  connect(myOpenBtn, SIGNAL(clicked()),    SLOT(onOpen()));
  connect(myNewBtn,  SIGNAL(clicked()),    SLOT(onNew()));

  connect(myReverseChk, SIGNAL(toggled(bool)), SLOT(onReverse(bool)));
  connect(myPreviewChk, SIGNAL(toggled(bool)), SLOT(onPreview(bool)));
  connect(myRefine,     SIGNAL(toggled(bool)), SLOT(onModeToggled(bool)));

  connect(myNode1, SIGNAL(valueChanged(int)), SLOT(onNodeChanged(int)));
  connect(myNode2, SIGNAL(valueChanged(int)), SLOT(onNodeChanged(int)));

  connect(mySelEdit[Ids], SIGNAL(textChanged(const QString&)), SLOT(onTextChanged(const QString&)));

  QMap< int, QPushButton* >::iterator anIter;
  for (anIter = mySelBtn.begin(); anIter != mySelBtn.end(); ++anIter)
    connect(*anIter, SIGNAL(clicked()), SLOT(onSelInputChanged()));

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_MeshPatternDlg::createButtonFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);
  aFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);

  myOkBtn     = new QPushButton(tr("SMESH_BUT_OK"   ), aFrame);
  myApplyBtn  = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn  = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn   = new QPushButton(tr("SMESH_BUT_HELP"), aFrame);

  QSpacerItem* aSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame, MARGIN, SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addWidget(myApplyBtn);
  aLay->addItem(aSpacer);
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::~SMESHGUI_MeshPatternDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_MeshPatternDlg::~SMESHGUI_MeshPatternDlg()
{
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_MeshPatternDlg::Init()
{
  myPattern = SMESH::GetPattern();
  myPreviewActor = 0;
  myIsCreateDlgOpen = false;
  mySelInput = Mesh;
  myType = -1;
  myNbPoints = -1;
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myMesh = SMESH::SMESH_Mesh::_nil();

  myMeshShape = GEOM::GEOM_Object::_nil();
  myGeomObj[ Object  ] = GEOM::GEOM_Object::_nil();
  myGeomObj[ Vertex1 ] = GEOM::GEOM_Object::_nil();
  myGeomObj[ Vertex2 ] = GEOM::GEOM_Object::_nil();

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(onClose()));

  myTypeGrp->setButton(Type_2d);
  onTypeChanged(Type_2d);
  onModeToggled(isRefine());

  updateGeometry();

  resize(minimumSize());

  activateSelection();
  onSelectionDone();

  this->show();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::isValid
// Purpose : Verify validity of entry data
//=======================================================================
bool SMESHGUI_MeshPatternDlg::isValid (const bool theMess)
{
  QValueList<int> ids;
  if ((isRefine() &&
       (myMesh->_is_nil() || !getIds(ids) || getNode(false) < 0 ||
        myType == Type_3d && (getNode(true) < 0 || getNode(false) == getNode(true))))
      ||
      (!isRefine() &&
       (myMesh->_is_nil() || myMeshShape->_is_nil() || myGeomObj[ Object ]->_is_nil() ||
        myGeomObj[ Vertex1 ]->_is_nil() || myType == Type_3d && myGeomObj[ Vertex2 ]->_is_nil())))
  {
    if (theMess)
      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                               tr("SMESHGUI_INVALID_PARAMETERS"), QMessageBox::Ok);
    return false;
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onApply
// Purpose : SLOT called when "Apply" button pressed.
//=======================================================================
bool SMESHGUI_MeshPatternDlg::onApply()
{
  try {
    if (!isValid())
      return false;

    erasePreview();

    if (isRefine()) { // Refining existing mesh elements
      QValueList<int> ids;
      getIds(ids);
      SMESH::long_array_var varIds = new SMESH::long_array();
      varIds->length(ids.count());
      int i = 0;
      for (QValueList<int>::iterator it = ids.begin(); it != ids.end(); ++it)
	varIds[i++] = *it;
      myType == Type_2d
	? myPattern->ApplyToMeshFaces  (myMesh, varIds, getNode(false), myReverseChk->isChecked())
	: myPattern->ApplyToHexahedrons(myMesh, varIds, getNode(false), getNode(true));

    } else { // Applying a pattern to geometrical object
      if (myType == Type_2d)
        myPattern->ApplyToFace(myGeomObj[Object], myGeomObj[Vertex1], myReverseChk->isChecked());
      else
        myPattern->ApplyTo3DBlock(myGeomObj[Object], myGeomObj[Vertex1], myGeomObj[Vertex2]);
    }

    bool toCreatePolygons = myCreatePolygonsChk->isChecked();
    bool toCreatePolyedrs = myCreatePolyedrsChk->isChecked();
    if ( myPattern->MakeMesh( myMesh, toCreatePolygons, toCreatePolyedrs ) ) {
      //mySelectionMgr->clearSelected();
      bool autoUpdate = SMESHGUI::automaticUpdate();
      if (!isRefine() && autoUpdate) {
	_PTR(SObject) aSO = SMESH::FindSObject(myMesh.in());
	SMESH_Actor* anActor = SMESH::FindActorByEntry(aSO->GetID().c_str());
	if (!anActor) {
	  anActor = SMESH::CreateActor(aSO->GetStudy(), aSO->GetID().c_str());
	  if (anActor) {
	    SMESH::DisplayActor(SMESH::GetActiveWindow(), anActor);
	    SMESH::FitAll();
	  }
	}
      }
      mySelectionMgr->clearSelected();
      SMESH::UpdateView();

      mySMESHGUI->updateObjBrowser(true);

      mySelEdit[ Ids ]->setText("");

      return true;
    } else {
      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                               tr("SMESH_OPERATION_FAILED"), QMessageBox::Ok);
      return false;
    }
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  } catch (...) {
  }

  return false;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_MeshPatternDlg::onOk()
{
  if (onApply())
    onClose();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_MeshPatternDlg::onClose()
{
  mySelectionMgr->clearFilters();
  SMESH::SetPickable();
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  erasePreview();
  reject();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MeshPatternDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", "application")).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_MeshPatternDlg::onSelectionDone()
{
  if (myBusy)
    return;

  try {
    if (mySelInput == Mesh) {
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());
      if (aList.Extent() != 1)
	return;

      // Retrieve mesh from selection
      Handle(SALOME_InteractiveObject) anIO = aList.First();
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIO);
      if (aMesh->_is_nil())
        return;

      // Get geom object corresponding to the mesh
      _PTR(SObject) aSO = SMESH::FindSObject(aMesh.in());
      myMeshShape = SMESH::GetGeom(aSO);

      // Clear fields of geom objects if mesh was changed
      if (myMesh != aMesh) {
        for (int i = Object; i <= Ids; i++) {
          myGeomObj[ i ] = GEOM::GEOM_Object::_nil();
          mySelEdit[ i ]->setText("");
        }
      }

      myMesh = aMesh;

      // Set name of mesh in line edit
      QString aName;
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aName);
      mySelEdit[ Mesh ]->setText(aName);

    } else if (mySelInput == Ids) {
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());
      if (aList.Extent() != 1)
	return;

      QString anIds;
      if (!SMESH::GetNameOfSelectedElements(mySelector, aList.First(), anIds))
	anIds = "";

      myBusy = true;
      mySelEdit[ Ids ]->setText(anIds);
      myBusy = false;

    } else {
      SALOME_ListIO aList;
      mySelectionMgr->selectedObjects(aList, SVTK_Viewer::Type());
      if (aList.Extent() != 1)
	return;

      // Get geom object from selection
      Handle(SALOME_InteractiveObject) anIO = aList.First();
      GEOM::GEOM_Object_var anObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
      if (anObj->_is_nil())
        return;

      // Clear fields of vertexes if face or 3d block was changed
      if (anObj != myGeomObj[ mySelInput ] && mySelInput == Object) {
        for (int i = Vertex1; i <= Vertex2; i++) {
          myGeomObj[ i ] = GEOM::GEOM_Object::_nil();
          mySelEdit[ i ]->setText("");
        }
      }

      myGeomObj[ mySelInput ] = anObj;

      // Set name of geom object in line edit
      QString aName;
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aName);
      mySelEdit[ mySelInput ]->setText(aName);
    }
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
    resetSelInput();
  } catch (...) {
    resetSelInput();
  }

  updateWgState();
  displayPreview();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::resetSelInput
// Purpose : Reset fields corresponding to the current selection input
//=======================================================================
void SMESHGUI_MeshPatternDlg::resetSelInput()
{
  if (mySelInput == Mesh)
  {
    myMesh = SMESH::SMESH_Mesh::_nil();
    myMeshShape = GEOM::GEOM_Object::_nil();
  }

  else
    myGeomObj[ mySelInput ] = GEOM::GEOM_Object::_nil();

  mySelEdit[ mySelInput ]->setText("");
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_MeshPatternDlg::onDeactivate()
{
  mySelectionMgr->clearFilters();
  //if (myReverseChk->isChecked())
  //  erasePreview();
  disconnect(mySelectionMgr, 0, this, 0);
  setEnabled(false);
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_MeshPatternDlg::enterEvent (QEvent*)
{
  if (myIsCreateDlgOpen)
    return;

  if (myReverseChk->isChecked())
    displayPreview();
  mySMESHGUI->EmitSignalDeactivateDialog();
  setEnabled(true);
  activateSelection();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  onTextChanged(mySelEdit[Ids]->text());
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::closeEvent
// Purpose :
//=======================================================================
void SMESHGUI_MeshPatternDlg::closeEvent (QCloseEvent*)
{
  onClose();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onSelInputChanged
// Purpose : SLOT. Called when -> button clicked.
//           Change current selection input field
//=======================================================================
void SMESHGUI_MeshPatternDlg::onSelInputChanged()
{
  const QObject* aSender = sender();
  for (int i = Mesh; i <= Ids; i++)
    if (aSender == mySelBtn[ i ])
      mySelInput = i;

  activateSelection();
  onSelectionDone();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::prepareFilters
// Purpose : Prepare filters for dialog
//=======================================================================
QStringList SMESHGUI_MeshPatternDlg::prepareFilters() const
{
  static QStringList aList;
  if (aList.isEmpty())
  {
    aList.append(tr("PATTERN_FILT"));
    //aList.append(tr("ALL_FILES_FILTER"));
  }

  return aList;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::autoExtension
// Purpose : Append extension to the file name
//=======================================================================
QString SMESHGUI_MeshPatternDlg::autoExtension (const QString& theFileName) const
{
  QString anExt = theFileName.section('.', -1);
  return anExt != "smp" && anExt != "SMP" ? theFileName + ".smp" : theFileName;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onOpen
// Purpose : SLOT. Called when "Open" button clicked.
//           Displays file open dialog
//=======================================================================
void SMESHGUI_MeshPatternDlg::onOpen()
{
  SUIT_FileDlg* aDlg = new SUIT_FileDlg (this, true);
  aDlg->setCaption(tr("LOAD_PATTERN"));
  aDlg->setMode(QFileDialog::ExistingFile);
  aDlg->setFilters(prepareFilters());
  if (myName->text() != "")
    aDlg->setSelection(myName->text() + ".smp");
  QPushButton* anOkBtn = (QPushButton*)aDlg->child("OK", "QPushButton");
  if (anOkBtn != 0)
    anOkBtn->setText(tr("SMESH_BUT_OK"));

  if (aDlg->exec() != Accepted)
    return;

  QString fName = aDlg->selectedFile();
  if (fName.isEmpty())
    return;

  if (QFileInfo(fName).extension().isEmpty())
    fName = autoExtension(fName);

  fName = QDir::convertSeparators(fName);

  QString prev = QDir::convertSeparators(myName->text());
  if (prev == fName)
    return;

  // Read string from file
  QFile aFile(fName);
  if (!aFile.open(IO_ReadOnly)) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("ERROR_OF_OPENING"), QMessageBox::Ok);
    return;
  }

  QByteArray aDataArray = aFile.readAll();
  const char* aData = aDataArray.data();
  if (aData == 0) {
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("ERROR_OF_READING"), QMessageBox::Ok);
    return;
  }

  if (loadFromFile(aData))
    myName->setText(QFileInfo(fName).baseName());

  updateWgState();
  displayPreview();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onCloseCreationDlg
// Purpose : SLOT. Called when "Pattern creation" dialog closed with "Close"
//=======================================================================
void SMESHGUI_MeshPatternDlg::onCloseCreationDlg()
{
  setEnabled(true);
  myIsCreateDlgOpen = false;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onOkCreationDlg
// Purpose : SLOT. Called when "Pattern creation" dialog closed with OK
//           or SAVE buttons. Initialize myPattern field. Redisplay preview
//=======================================================================
void SMESHGUI_MeshPatternDlg::onOkCreationDlg()
{
  myPattern = SMESH::SMESH_Pattern::_duplicate(myCreationDlg->GetPattern());
  myName->setText(myCreationDlg->GetPatternName());

  updateWgState();
  displayPreview();

  setEnabled(true);
  myIsCreateDlgOpen = false;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onNew
// Purpose : SLOT. Called when "New..." button clicked. Create new pattern
//=======================================================================
void SMESHGUI_MeshPatternDlg::onNew()
{
  setEnabled(false);
  myIsCreateDlgOpen = true;
  if (myCreationDlg == 0)
  {
    myCreationDlg = new SMESHGUI_CreatePatternDlg( mySMESHGUI, myType);
    connect(myCreationDlg, SIGNAL(NewPattern()), SLOT(onOkCreationDlg()));
    connect(myCreationDlg, SIGNAL(Close()), SLOT(onCloseCreationDlg()));
  }
  else
    myCreationDlg->Init(myType);

  myCreationDlg->SetMesh(myMesh);
  myCreationDlg->show();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onReverse
// Purpose : SLOT. Called when state of "Reverse order..." checkbox chaged
//           Calculate new points of the mesh to be created. Redisplay preview
//=======================================================================
void SMESHGUI_MeshPatternDlg::onReverse (bool)
{
  displayPreview();
}

//=======================================================================

// name    : SMESHGUI_MeshPatternDlg::onPreview
// Purpose : SLOT. Called when state of "Preview" checkbox changed
//           Display/Erase preview
//=======================================================================
void SMESHGUI_MeshPatternDlg::onPreview (bool)
{
  displayPreview();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::displayPreview
// Purpose : Display preview
//=======================================================================
void SMESHGUI_MeshPatternDlg::displayPreview()
{
  try {
    // Redisplay preview in dialog
    SMESH::point_array_var pnts = myPattern->GetPoints();
    SMESH::long_array_var keyPoints = myPattern->GetKeyPoints();
    SMESH::array_of_long_array_var elemPoints = myPattern->GetElementPoints(false);

    if (pnts->length()       == 0 ||
        keyPoints->length()  == 0 ||
        elemPoints->length() == 0) {
      erasePreview();
      return;
    } else {
      PointVector aPoints(pnts->length());
      QValueVector<int> aKeyPoints(keyPoints->length());
      ConnectivityVector anElemPoints(elemPoints->length());

      for (int i = 0, n = pnts->length(); i < n; i++)
        aPoints[ i ] = pnts[ i ];

      for (int i2 = 0, n2 = keyPoints->length(); i2 < n2; i2++)
        aKeyPoints[ i2 ] = keyPoints[ i2 ];

      for (int i3 = 0, n3 = elemPoints->length(); i3 < n3; i3++) {
        QValueVector<int> aVec(elemPoints[ i3 ].length());
        for (int i4 = 0, n4 = elemPoints[ i3 ].length(); i4 < n4; i4++)
          aVec[ i4 ] = elemPoints[ i3 ][ i4 ];

        anElemPoints[ i3 ] = aVec;
      }

      myPicture2d->SetPoints(aPoints, aKeyPoints, anElemPoints);
    }

    // Redisplay preview in 3D viewer
    if (myPreviewActor != 0) {
      if (SVTK_ViewWindow* vf = SMESH::GetCurrentVtkView()) {
        vf->RemoveActor(myPreviewActor);
        vf->Repaint();
      }
      myPreviewActor->Delete();
      myPreviewActor = 0;
    }

    if (!myPreviewChk->isChecked() || !isValid(false))
      return;

    vtkUnstructuredGrid* aGrid = getGrid();
    if (aGrid == 0)
      return;

    // Create and display actor
    vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
    aMapper->SetInput(aGrid);

    myPreviewActor = SALOME_Actor::New();
    myPreviewActor->PickableOff();
    myPreviewActor->SetMapper(aMapper);

    vtkProperty* aProp = vtkProperty::New();
    aProp->SetRepresentationToWireframe();
    aProp->SetColor(250, 0, 250);
    if (SMESH::FindActorByObject(myMesh))
      aProp->SetLineWidth( SMESH::GetFloat( "SMESH:element_width", 1 ) + 1 );
    else
      aProp->SetLineWidth(1);
    myPreviewActor->SetProperty(aProp);

    myPreviewActor->SetRepresentation(3);

    SMESH::GetCurrentVtkView()->AddActor(myPreviewActor);
    SMESH::GetCurrentVtkView()->Repaint();

    aProp->Delete();
    aGrid->Delete();
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
    erasePreview();
  } catch (...) {
    erasePreview();
  }
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::erasePreview
// Purpose : Erase preview
//=======================================================================
void SMESHGUI_MeshPatternDlg::erasePreview()
{
  // Erase preview in 2D viewer
  myPicture2d->SetPoints(PointVector(), QValueVector<int>(), ConnectivityVector());

  // Erase preview in 3D viewer
  if (myPreviewActor == 0)
    return;


  if (SVTK_ViewWindow* vf = SMESH::GetCurrentVtkView())
  {
    vf->RemoveActor(myPreviewActor);
    vf->Repaint();
  }
  myPreviewActor->Delete();
  myPreviewActor = 0;
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::updateWgState
// Purpose : Enable/disable selection widgets
//=======================================================================
void SMESHGUI_MeshPatternDlg::updateWgState()
{
  if (myMesh->_is_nil()) {
    for (int i = Object; i <= Ids; i++) {
      mySelBtn [ i ]->setEnabled(false);
      mySelEdit[ i ]->setEnabled(false);
      mySelEdit[ i ]->setText("");
    }
    myNode1->setEnabled(false);
    myNode2->setEnabled(false);
    myNode1->setRange(0, 0);
    myNode2->setRange(0, 0);
  } else {
    mySelBtn [ Object ]->setEnabled(true);
    mySelEdit[ Object ]->setEnabled(true);
    mySelBtn [ Ids ]   ->setEnabled(true);
    mySelEdit[ Ids ]   ->setEnabled(true);

    if (myGeomObj[ Object ]->_is_nil()) {
      for (int i = Vertex1; i <= Vertex2; i++) {
        mySelBtn [ i ]->setEnabled(false);
        mySelEdit[ i ]->setEnabled(false);
        mySelEdit[ i ]->setText("");
      }
    } else {
      for (int i = Object; i <= Vertex2; i++) {
        mySelBtn [ i ]->setEnabled(true);
        mySelEdit[ i ]->setEnabled(true);
      }
    }

    QValueList<int> ids;
    if (!CORBA::is_nil(myPattern)/* && getIds(ids)*/) {
      SMESH::long_array_var keyPoints = myPattern->GetKeyPoints();
      if (keyPoints->length()) {
	myNode1->setEnabled(true);
	myNode2->setEnabled(true);
	myNode1->setRange(1, keyPoints->length());
	myNode2->setRange(1, keyPoints->length());
	return;
      }
    }

    myNode1->setEnabled(false);
    myNode2->setEnabled(false);
    myNode1->setRange(0, 0);
    myNode2->setRange(0, 0);
  }
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::activateSelection
// Purpose : Activate selection in accordance with current selection input
//=======================================================================
void SMESHGUI_MeshPatternDlg::activateSelection()
{
  mySelectionMgr->clearFilters();
  if (mySelInput == Ids) {
    SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
    if (anActor)
      SMESH::SetPickable(anActor);

    if (myType == Type_2d)
      {
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(FaceSelection);
      }
    else
      {
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(CellSelection);
      }
  }
  else {
    SMESH::SetPickable();
    //mySelectionMgr->setSelectionModes(ActorSelection);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
  }

  if (mySelInput == Object && !myMeshShape->_is_nil()) {
    if (myType == Type_2d) {
      if (myNbPoints > 0)
        mySelectionMgr->installFilter
          (new SMESH_NumberFilter ("GEOM", TopAbs_VERTEX, myNbPoints, TopAbs_FACE, myMeshShape));
      else
        mySelectionMgr->installFilter
          (new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE, myNbPoints, TopAbs_FACE, myMeshShape));
    } else {
      TColStd_MapOfInteger aTypes;
      aTypes.Add(TopAbs_SHELL);
      aTypes.Add(TopAbs_SOLID);
      mySelectionMgr->installFilter
        (new SMESH_NumberFilter ("GEOM", TopAbs_FACE, 6, aTypes, myMeshShape, true));
    }
  } else if ((mySelInput == Vertex1 || mySelInput == Vertex2) && !myGeomObj[ Object ]->_is_nil()) {
    mySelectionMgr->installFilter
      (new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE, 1, TopAbs_VERTEX, myGeomObj[ Object ]));
  } else {
  }
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::loadFromFile
// Purpose : Load pattern from file
//=======================================================================
bool SMESHGUI_MeshPatternDlg::loadFromFile (const QString& theName)
{
  try {
    SMESH::SMESH_Pattern_var aPattern = SMESH::GetPattern();

    if (!aPattern->LoadFromFile(theName.latin1()) ||
        myType == Type_2d && !aPattern->Is2D()) {
      SMESH::SMESH_Pattern::ErrorCode aCode = aPattern->GetErrorCode();
      QString aMess;
      if      (aCode == SMESH::SMESH_Pattern::ERR_READ_NB_POINTS     ) aMess = tr("ERR_READ_NB_POINTS");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_POINT_COORDS  ) aMess = tr("ERR_READ_POINT_COORDS");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_TOO_FEW_POINTS) aMess = tr("ERR_READ_TOO_FEW_POINTS");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_3D_COORD      ) aMess = tr("ERR_READ_3D_COORD");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_NO_KEYPOINT   ) aMess = tr("ERR_READ_NO_KEYPOINT");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_BAD_INDEX     ) aMess = tr("ERR_READ_BAD_INDEX");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_ELEM_POINTS   ) aMess = tr("ERR_READ_ELEM_POINTS");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_NO_ELEMS      ) aMess = tr("ERR_READ_NO_ELEMS");
      else if (aCode == SMESH::SMESH_Pattern::ERR_READ_BAD_KEY_POINT ) aMess = tr("ERR_READ_BAD_KEY_POINT");
      else                                                             aMess = tr("ERROR_OF_LOADING");

      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"), aMess, QMessageBox::Ok);
      return false;
    } else {
      myPattern = aPattern;
      return true;
    }
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
    QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                             tr("ERROR_OF_LOADING"), QMessageBox::Ok);
      return false;
  }
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::onTypeChanged
// Purpose : SLOT. Called when pattern type changed.
//           Change dialog's look and feel
//=======================================================================
void SMESHGUI_MeshPatternDlg::onTypeChanged (int theType)
{
  if (myType == theType)
    return;

  myType = theType;

  myNbPoints = -1;
  myGeomObj[ Object  ] = GEOM::GEOM_Object::_nil();
  myGeomObj[ Vertex1 ] = GEOM::GEOM_Object::_nil();
  myGeomObj[ Vertex2 ] = GEOM::GEOM_Object::_nil();
  myPattern = SMESH::GetPattern();

  myName->setText("");
  mySelEdit[ Object  ]->setText("");
  mySelEdit[ Vertex1 ]->setText("");
  mySelEdit[ Vertex2 ]->setText("");
  mySelEdit[ Ids ]    ->setText("");
  myCreatePolygonsChk->show();
  myCreatePolyedrsChk->show();

  if (theType == Type_2d) {
    // Geom widgets
    mySelLbl [ Vertex2 ]->hide();
    mySelBtn [ Vertex2 ]->hide();
    mySelEdit[ Vertex2 ]->hide();
    myReverseChk->show();
    myPicture2d->show();
    myPicture3d->hide();
    mySelLbl[ Object  ]->setText(tr("FACE"));
    mySelLbl[ Vertex1 ]->setText(tr("VERTEX"));
    // Refine widgets
    mySelLbl[ Ids ]->setText(tr("MESH_FACES"));
    myNode2Lbl->hide();
    myNode2   ->hide();
  } else {
    // Geom widgets
    mySelLbl [ Vertex2 ]->show();
    mySelBtn [ Vertex2 ]->show();
    mySelEdit[ Vertex2 ]->show();
    myReverseChk->hide();
    myPicture2d->hide();
    myPicture3d->show();
    mySelLbl[ Object  ]->setText(tr("3D_BLOCK"));
    mySelLbl[ Vertex1 ]->setText(tr("VERTEX1"));
    mySelLbl[ Vertex2 ]->setText(tr("VERTEX2"));
    // Refine widgets
    mySelLbl[ Ids ]->setText(tr("MESH_VOLUMES"));
    myNode2Lbl->show();
    myNode2   ->show();
  }

  mySelInput = Mesh;
  activateSelection();
  updateWgState();
  displayPreview();
}

//=======================================================================
// name    : SMESHGUI_MeshPatternDlg::getGrid
// Purpose : Get unstructured grid for pattern
//=======================================================================
vtkUnstructuredGrid* SMESHGUI_MeshPatternDlg::getGrid()
{
  try {
    // Get points from pattern
    SMESH::point_array_var pnts;
    QValueList<int> ids;
    if (isRefine() && getIds(ids)) {
      SMESH::long_array_var varIds = new SMESH::long_array();
      varIds->length(ids.count());
      int i = 0;
      for (QValueList<int>::iterator it = ids.begin(); it != ids.end(); ++it)
	varIds[i++] = *it;
      pnts = myType == Type_2d
	? myPattern->ApplyToMeshFaces  (myMesh, varIds, getNode(false), myReverseChk->isChecked())
	: myPattern->ApplyToHexahedrons(myMesh, varIds, getNode(false), getNode(true));
    } else {
      pnts = myType == Type_2d
	? myPattern->ApplyToFace   (myGeomObj[ Object ], myGeomObj[ Vertex1 ], myReverseChk->isChecked())
      : myPattern->ApplyTo3DBlock(myGeomObj[ Object ], myGeomObj[ Vertex1 ], myGeomObj[ Vertex2 ]);
    }

    SMESH::array_of_long_array_var elemPoints = myPattern->GetElementPoints(true);

    if (pnts->length() == 0 || elemPoints->length() == 0)
      return 0;

    // to do : to be removed /////////////////////////////////////////////

#ifdef DEB_SLN
    for (int i1 = 0, n1 = pnts->length(); i1 < n1; i1++)
      printf("%d: %g %g %g\n", i1, pnts[ i1 ].x, pnts[ i1 ].y, pnts[ i1 ].z);

    printf("\nELEMENTS : \n");
    for (int i2 = 0, n2 = elemPoints->length(); i2 < n2; i2++)
    {

      printf("%d: ", i2);
      for (int i3 = 0, n3 = elemPoints[ i2 ].length(); i3 < n3; i3++)
        printf("%d ", elemPoints[ i2 ][ i3 ]);

      printf("\n");

    }
#endif
    //////////////////////////////////////////////////////////////////////

    // Calculate number of points used for cell
    vtkIdType aNbCells = elemPoints->length();
    vtkIdType aCellsSize = 0;
    for (int i = 0, n = elemPoints->length(); i < n; i++)
      aCellsSize += elemPoints[ i ].length();

    // Create unstructured grid and other  usefull arrays
    vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

    vtkCellArray* aConnectivity = vtkCellArray::New();
    aConnectivity->Allocate(aCellsSize, 0);

    vtkPoints* aPoints = vtkPoints::New();
    aPoints->SetNumberOfPoints(pnts->length());

    vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
    aCellTypesArray->SetNumberOfComponents(1);
    aCellTypesArray->Allocate(aNbCells * aCellTypesArray->GetNumberOfComponents());

    vtkIdList *anIdList = vtkIdList::New();

    // Fill array of points
    for (int p = 0, nbPnt = pnts->length(); p < nbPnt; p++)
      aPoints->SetPoint(p, pnts[ p ].x, pnts[ p ].y, pnts[ p ].z);

    for (int e = 0, nbElem = elemPoints->length(); e < nbElem; e++) {
      int nbPoints = elemPoints[ e ].length();
      anIdList->SetNumberOfIds(nbPoints);
      for (int i = 0; i < nbPoints; i++)
        anIdList->SetId(i, elemPoints[ e ][ i ]);

      aConnectivity->InsertNextCell(anIdList);

      if      (nbPoints == 3) aCellTypesArray->InsertNextValue(VTK_TRIANGLE);
      else if (nbPoints == 5) aCellTypesArray->InsertNextValue(VTK_PYRAMID);
      else if (nbPoints == 6) aCellTypesArray->InsertNextValue(VTK_WEDGE);
      else if (nbPoints == 8) aCellTypesArray->InsertNextValue(VTK_HEXAHEDRON);
      else if (nbPoints == 4 && myType == Type_2d) aCellTypesArray->InsertNextValue(VTK_QUAD);
      else if (nbPoints == 4 && myType == Type_3d) aCellTypesArray->InsertNextValue(VTK_TETRA);
      else aCellTypesArray->InsertNextValue(VTK_EMPTY_CELL);
    }

    VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
    aCellLocationsArray->SetNumberOfComponents(1);
    aCellLocationsArray->SetNumberOfTuples(aNbCells);

    aConnectivity->InitTraversal();
    for (vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell(npts, pts); idType++)
      aCellLocationsArray->SetValue(idType, aConnectivity->GetTraversalLocation(npts));

    aGrid->SetPoints(aPoints);
    aGrid->SetCells(aCellTypesArray, aCellLocationsArray,aConnectivity);

    aConnectivity->Delete();
    aPoints->Delete();
    aCellTypesArray->Delete();
    anIdList->Delete();
    aCellLocationsArray->Delete();

    return aGrid;
  } catch (...) {
    return 0;
  }
}

//=======================================================================
// name    : onModeToggled
// Purpose :
//=======================================================================
void SMESHGUI_MeshPatternDlg::onModeToggled (bool on)
{
  on ? myRefineGrp->show() : myRefineGrp->hide();
  on ? myGeomGrp->hide()   : myGeomGrp->show();

  displayPreview();
}

//=======================================================================
// name    : isRefine
// Purpose :
//=======================================================================
bool SMESHGUI_MeshPatternDlg::isRefine() const
{
  return myRefine->isChecked();
}

//=======================================================================
// name    : onTextChanged
// Purpose :
//=======================================================================
void SMESHGUI_MeshPatternDlg::onTextChanged (const QString& theNewText)
{
  if (myBusy || !isRefine())
    return;

  myBusy = true;

  if (mySelInput != Ids) {
    mySelInput = Ids;
    activateSelection();
  }

  // hilight entered elements/nodes
  SMDS_Mesh* aMesh = 0;
  SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
  if (anActor)
    aMesh = anActor->GetObject()->GetMesh();

  if (aMesh) {
    QStringList aListId = QStringList::split(" ", theNewText, false);

    TColStd_MapOfInteger newIndices;

    for (int i = 0; i < aListId.count(); i++) {
      const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt());
      if (e && e->GetType() == (myType == Type_2d ? SMDSAbs_Face : SMDSAbs_Volume))
	newIndices.Add(e->GetID());
    }
    mySelector->AddOrRemoveIndex( anActor->getIO(), newIndices, false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->highlight( anActor->getIO(), true, true );
  }

  myBusy = false;

  displayPreview();
}

//=======================================================================
// name    : onNodeChanged
// Purpose :
//=======================================================================
void SMESHGUI_MeshPatternDlg::onNodeChanged (int value)
{
  if (myType == Type_3d) {
    QSpinBox* first = (QSpinBox*)sender();
    QSpinBox* second = first == myNode1 ? myNode2 : myNode1;
    int secondVal = second->value();
    if (secondVal == value) {
      secondVal = value == second->maxValue() ? second->minValue() : value + 1;
      bool blocked = second->signalsBlocked();
      second->blockSignals(true);
      second->setValue(secondVal);
      second->blockSignals(blocked);
    }
  }

  displayPreview();
}

//=======================================================================
// name    : getIds
// Purpose :
//=======================================================================
bool SMESHGUI_MeshPatternDlg::getIds (QValueList<int>& ids) const
{
  ids.clear();
  QStringList strIds = QStringList::split(" ", mySelEdit[Ids]->text());
  bool isOk;
  int val;
  for (QStringList::iterator it = strIds.begin(); it != strIds.end(); ++it) {
    val = (*it).toInt(&isOk);
    if (isOk)
      ids.append(val);
  }

  return ids.count();
}

//=======================================================================
// name    : getNode1
// Purpose :
//=======================================================================
int SMESHGUI_MeshPatternDlg::getNode (bool second) const
{
  return second ? myNode2->value() - 1 : myNode1->value() - 1;
}
