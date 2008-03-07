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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_ExtrusionAlongPathDlg.cxx
//  Author : Vadim SANDLER
//  Module : SMESH
//  $Header:

#include "SMESHGUI_ExtrusionAlongPathDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"

#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_NumberFilter.hxx"
#include "SMESH_LogicalFilter.hxx"

#include "SMDS_Mesh.hxx"

#include "GEOMBase.h"

#include "SUIT_ResourceMgr.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_Desktop.h"
#include "SUIT_MessageBox.h"
#include "SUIT_Session.h"

#include "LightApp_Application.h"

#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SVTK_Selection.h"
#include "SALOME_ListIO.hxx"

#include "utilities.h"

// OCCT Includes
#include <BRep_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qvalidator.h>
#include <qptrlist.h>
#include <qpixmap.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

using namespace std;

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionAlongPathDlg::SMESHGUI_ExtrusionAlongPathDlg( SMESHGUI* theModule,
                                                                bool modal )
     : QDialog( SMESH::GetDesktop( theModule ), "SMESHGUI_ExtrusionAlongPathDlg", modal,
                WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | Qt::WDestructiveClose),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap edgeImage   ( mgr->loadPixmap("SMESH", tr("ICON_DLG_EDGE")));
  QPixmap faceImage   ( mgr->loadPixmap("SMESH", tr("ICON_DLG_TRIANGLE")));
  QPixmap selectImage ( mgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap addImage    ( mgr->loadPixmap("SMESH", tr("ICON_APPEND")));
  QPixmap removeImage ( mgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  myType = -1;

  setCaption(tr("EXTRUSION_ALONG_PATH"));
  setSizeGripEnabled(TRUE);

  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setSpacing(6);
  topLayout->setMargin(11);

  /***************************************************************/
  // Elements type group box (1d / 2d elements)
  GroupConstructors = new QButtonGroup(tr("SMESH_EXTRUSION"), this);
  GroupConstructors->setColumnLayout(0, Qt::Vertical);
  GroupConstructors->layout()->setSpacing(0);
  GroupConstructors->layout()->setMargin(0);
  QGridLayout* GroupConstructorsLayout = new QGridLayout(GroupConstructors->layout());
  GroupConstructorsLayout->setAlignment(Qt::AlignTop);
  GroupConstructorsLayout->setSpacing(6); GroupConstructorsLayout->setMargin(11);

  Elements1dRB = new QRadioButton(GroupConstructors);
  Elements1dRB->setPixmap(edgeImage);
  Elements2dRB = new QRadioButton(GroupConstructors);
  Elements2dRB->setPixmap(faceImage);
  Elements1dRB->setChecked(true);

  // layouting
  GroupConstructorsLayout->addWidget(Elements1dRB, 0, 0);
  GroupConstructorsLayout->addWidget(Elements2dRB, 0, 2);

  /***************************************************************/
  // Arguments group box
  GroupArguments = new QGroupBox(tr("EXTRUSION_1D"), this);
  GroupArguments->setColumnLayout(0, Qt::Vertical);
  GroupArguments->layout()->setSpacing(0); GroupArguments->layout()->setMargin(0);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments->layout());
  GroupArgumentsLayout->setAlignment(Qt::AlignTop);
  GroupArgumentsLayout->setSpacing(6); GroupArgumentsLayout->setMargin(11);

  // Controls for elements selection
  ElementsLab = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QToolButton(GroupArguments);
  SelectElementsButton->setPixmap(selectImage);

  ElementsLineEdit = new QLineEdit(GroupArguments);
  ElementsLineEdit->setValidator(new SMESHGUI_IdValidator(this));

  // Controls for the whole mesh selection
  MeshCheck = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for path selection
  PathGrp = new QGroupBox(tr("SMESH_PATH"), GroupArguments);
  PathGrp->setColumnLayout(0, Qt::Vertical);
  PathGrp->layout()->setSpacing(0); PathGrp->layout()->setMargin(0);
  QGridLayout* PathGrpLayout = new QGridLayout(PathGrp->layout());
  PathGrpLayout->setAlignment(Qt::AlignTop);
  PathGrpLayout->setSpacing(6); PathGrpLayout->setMargin(11);

  // Controls for path mesh selection
  PathMeshLab = new QLabel(tr("SMESH_PATH_MESH"), PathGrp);

  SelectPathMeshButton = new QToolButton(PathGrp);
  SelectPathMeshButton->setPixmap(selectImage);

  PathMeshLineEdit = new QLineEdit(PathGrp);
  PathMeshLineEdit->setReadOnly(true);

  // Controls for path shape selection
  PathShapeLab = new QLabel(tr("SMESH_PATH_SHAPE"), PathGrp);

  SelectPathShapeButton = new QToolButton(PathGrp);
  SelectPathShapeButton->setPixmap(selectImage);

  PathShapeLineEdit = new QLineEdit(PathGrp);
  PathShapeLineEdit->setReadOnly(true);

  // Controls for path starting point selection
  StartPointLab = new QLabel(tr("SMESH_PATH_START"), PathGrp);

  SelectStartPointButton = new QToolButton(PathGrp);
  SelectStartPointButton->setPixmap(selectImage);

  StartPointLineEdit = new QLineEdit(PathGrp);
  StartPointLineEdit->setValidator(new QIntValidator(this));

  // layouting
  PathGrpLayout->addWidget(PathMeshLab,            0, 0);
  PathGrpLayout->addWidget(SelectPathMeshButton,   0, 1);
  PathGrpLayout->addWidget(PathMeshLineEdit,       0, 2);
  PathGrpLayout->addWidget(PathShapeLab,           1, 0);
  PathGrpLayout->addWidget(SelectPathShapeButton,  1, 1);
  PathGrpLayout->addWidget(PathShapeLineEdit,      1, 2);
  PathGrpLayout->addWidget(StartPointLab,          2, 0);
  PathGrpLayout->addWidget(SelectStartPointButton, 2, 1);
  PathGrpLayout->addWidget(StartPointLineEdit,     2, 2);

  // Controls for base point defining
  BasePointCheck = new QCheckBox(tr("SMESH_USE_BASE_POINT"), GroupArguments);

  BasePointGrp = new QGroupBox(tr("SMESH_BASE_POINT"), GroupArguments);
  BasePointGrp->setColumnLayout(0, Qt::Vertical);
  BasePointGrp->layout()->setSpacing(0); BasePointGrp->layout()->setMargin(0);
  QGridLayout* BasePointGrpLayout = new QGridLayout(BasePointGrp->layout());
  BasePointGrpLayout->setAlignment(Qt::AlignTop);
  BasePointGrpLayout->setSpacing(6); BasePointGrpLayout->setMargin(11);

  SelectBasePointButton = new QToolButton(BasePointGrp);
  SelectBasePointButton->setPixmap(selectImage);

  XLab  = new QLabel(tr("SMESH_X"), BasePointGrp);
  XLab->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  XSpin = new SMESHGUI_SpinBox(BasePointGrp);
  YLab  = new QLabel(tr("SMESH_Y"), BasePointGrp);
  YLab->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  YSpin = new SMESHGUI_SpinBox(BasePointGrp);
  ZLab  = new QLabel(tr("SMESH_Z"), BasePointGrp);
  ZLab->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  ZSpin = new SMESHGUI_SpinBox(BasePointGrp);

  // layouting
  BasePointGrpLayout->addWidget(SelectBasePointButton, 0, 0);
  BasePointGrpLayout->addWidget(XLab,                  0, 1);
  BasePointGrpLayout->addWidget(XSpin,                 0, 2);
  BasePointGrpLayout->addWidget(YLab,                  0, 3);
  BasePointGrpLayout->addWidget(YSpin,                 0, 4);
  BasePointGrpLayout->addWidget(ZLab,                  0, 5);
  BasePointGrpLayout->addWidget(ZSpin,                 0, 6);

  // Controls for angles defining
  AnglesCheck = new QCheckBox(tr("SMESH_USE_ANGLES"), GroupArguments);

  AnglesGrp = new QGroupBox(tr("SMESH_ANGLES"), GroupArguments);
  AnglesGrp->setColumnLayout(0, Qt::Vertical);
  AnglesGrp->layout()->setSpacing(0); AnglesGrp->layout()->setMargin(0);
  QGridLayout* AnglesGrpLayout = new QGridLayout(AnglesGrp->layout());
  AnglesGrpLayout->setAlignment(Qt::AlignTop);
  AnglesGrpLayout->setSpacing(6); AnglesGrpLayout->setMargin(11);

  AnglesList = new QListBox(AnglesGrp);
  AnglesList->setSelectionMode(QListBox::Extended);

  AddAngleButton = new QToolButton(AnglesGrp);
  AddAngleButton->setPixmap(addImage);

  RemoveAngleButton = new QToolButton(AnglesGrp);
  RemoveAngleButton->setPixmap(removeImage);

  AngleSpin = new SMESHGUI_SpinBox(AnglesGrp);

  LinearAnglesCheck = new QCheckBox(tr("LINEAR_ANGLES"), AnglesGrp);

  // layouting
  QVBoxLayout* bLayout = new QVBoxLayout();
  bLayout->addWidget(AddAngleButton);
  bLayout->addSpacing(6);
  bLayout->addWidget(RemoveAngleButton);
  bLayout->addStretch();
  AnglesGrpLayout->addMultiCellWidget(AnglesList, 0, 1, 0, 0);
  AnglesGrpLayout->addMultiCellLayout(bLayout,    0, 1, 1, 1);
  AnglesGrpLayout->addWidget(         AngleSpin,  0,    2   );
  AnglesGrpLayout->addWidget( LinearAnglesCheck,  2,    0   );
  AnglesGrpLayout->setRowStretch(1, 10);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  MakeGroupsCheck->setChecked(true);

  // layouting
  GroupArgumentsLayout->addWidget(         ElementsLab,            0,    0   );
  GroupArgumentsLayout->addWidget(         SelectElementsButton,   0,    1   );
  GroupArgumentsLayout->addWidget(         ElementsLineEdit,       0,    2   );
  GroupArgumentsLayout->addMultiCellWidget(MeshCheck,              1, 1, 0, 2);
  GroupArgumentsLayout->addMultiCellWidget(PathGrp,                2, 2, 0, 2);
  GroupArgumentsLayout->addWidget(         BasePointCheck,         3,    0   );
  GroupArgumentsLayout->addMultiCellWidget(BasePointGrp,           3, 4, 1, 2);
  GroupArgumentsLayout->addWidget(         AnglesCheck,            5,    0   );
  GroupArgumentsLayout->addMultiCellWidget(AnglesGrp,              5, 6, 1, 2);
  GroupArgumentsLayout->addMultiCellWidget(MakeGroupsCheck,        7, 7, 0, 2);
  GroupArgumentsLayout->setRowStretch(6, 10);

  /***************************************************************/
  // common buttons group box
  GroupButtons = new QGroupBox(this);
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6); GroupButtonsLayout->setMargin(11);

  OkButton = new QPushButton(tr("SMESH_BUT_OK"), GroupButtons);
  OkButton->setAutoDefault(true);
  OkButton->setDefault(true);

  ApplyButton = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  ApplyButton->setAutoDefault(true);

  CloseButton = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  CloseButton->setAutoDefault(true);

  HelpButton = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  HelpButton->setAutoDefault(true);

  // layouting
  GroupButtonsLayout->addWidget(OkButton,    0, 0);
  GroupButtonsLayout->addWidget(ApplyButton, 0, 1);
  GroupButtonsLayout->addWidget(CloseButton, 0, 3);
  GroupButtonsLayout->addWidget(HelpButton, 0, 4);
  GroupButtonsLayout->addColSpacing(2, 10);
  GroupButtonsLayout->setColStretch(2, 10);

  /***************************************************************/
  // layouting
  topLayout->addWidget(GroupConstructors, 0, 0);
  topLayout->addWidget(GroupArguments,    1, 0);
  topLayout->addWidget(GroupButtons,      2, 0);

  /***************************************************************/
  // Initialisations
  XSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  YSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  ZSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, 3);
  AngleSpin->RangeStepAndValidator(-180.0, 180.0, 5.0, 3);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox(this);

  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QPtrList<SUIT_SelectionFilter> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myElementsFilter = new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);
  myPathMeshFilter = new SMESH_TypeFilter (MESH);

  myHelpFileName = "extrusion_along_path_page.html";

  Init();

  /***************************************************************/
  // signals-slots connections
  connect(OkButton,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(CloseButton,  SIGNAL(clicked()), this, SLOT(reject()));
  connect(ApplyButton,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(HelpButton,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(AddAngleButton,    SIGNAL(clicked()), this, SLOT(OnAngleAdded()));
  connect(RemoveAngleButton, SIGNAL(clicked()), this, SLOT(OnAngleRemoved()));

  connect(GroupConstructors, SIGNAL(clicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectPathMeshButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectPathShapeButton,  SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectStartPointButton, SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectBasePointButton,  SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));

  connect(mySMESHGUI,  SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()),      this, SLOT(SelectionIntoArgument()));
  connect(mySMESHGUI,  SIGNAL(SignalCloseAllDialogs()),        this, SLOT(reject()));

  connect(ElementsLineEdit, SIGNAL(textChanged(const QString&)),
	   SLOT(onTextChange(const QString&)));
  connect(StartPointLineEdit, SIGNAL(textChanged(const QString&)),
	   SLOT(onTextChange(const QString&)));

  connect(MeshCheck,      SIGNAL(toggled(bool)), SLOT(onSelectMesh()));
  connect(AnglesCheck,    SIGNAL(toggled(bool)), SLOT(onAnglesCheck()));
  connect(BasePointCheck, SIGNAL(toggled(bool)), SLOT(onBasePointCheck()));

  AnglesList->installEventFilter(this);
  ElementsLineEdit->installEventFilter(this);
  StartPointLineEdit->installEventFilter(this);
  XSpin->editor()->installEventFilter(this);
  YSpin->editor()->installEventFilter(this);
  ZSpin->editor()->installEventFilter(this);

  /***************************************************************/
  
  this->show(); // displays Dialog
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionAlongPathDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionAlongPathDlg::~SMESHGUI_ExtrusionAlongPathDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=================================================================================
// function : Init()
// purpose  : initialization
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::Init (bool ResetControls)
{
  myBusy = false;
  myEditCurrentArgument = 0;

  myMesh      = SMESH::SMESH_Mesh::_nil();
  myIDSource  = SMESH::SMESH_IDSource::_nil();
  myMeshActor = 0;
  myPathMesh  = SMESH::SMESH_Mesh::_nil();
  myPathShape = GEOM::GEOM_Object::_nil();

  ElementsLineEdit->clear();
  PathMeshLineEdit->clear();
  PathShapeLineEdit->clear();
  StartPointLineEdit->clear();

  if (ResetControls) {
    XSpin->SetValue(0.0);
    YSpin->SetValue(0.0);
    ZSpin->SetValue(0.0);

    AngleSpin->SetValue(45);
    MeshCheck->setChecked(false);
    ConstructorsClicked(0);
    onSelectMesh();
    onAnglesCheck();
    onBasePointCheck();
  }
  SetEditCurrentArgument(0);
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Called when user changes type of elements (1d / 2d)
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::ConstructorsClicked (int type)
{
  if (myType == type) return;

  disconnect(mySelectionMgr, 0, this, 0);

  if (type == 0)
    GroupArguments->setTitle(tr("EXTRUSION_1D"));
  else if (type == 1)
    GroupArguments->setTitle(tr("EXTRUSION_2D"));

  // clear elements ID list
  if (!MeshCheck->isChecked()) {
    ElementsLineEdit->clear();
  }
  // set selection mode if necessary
  if (myEditCurrentArgument == ElementsLineEdit) {
    mySelectionMgr->clearSelected();
    mySelectionMgr->clearFilters();
    SMESH::SetPickable();

    SMESH::SetPointRepresentation(false);
    if (MeshCheck->isChecked()) {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(ActorSelection);
      mySelectionMgr->installFilter(myElementsFilter);
    } else {
      if (type == 0)
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(EdgeSelection);
	}
      if (type == 1)
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(FaceSelection);
	}
    }
  }
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));

  myType = type;
}

//=================================================================================
// function : ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================
bool SMESHGUI_ExtrusionAlongPathDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (myMesh->_is_nil() || MeshCheck->isChecked() && myIDSource->_is_nil() ||
      !myMeshActor || myPathMesh->_is_nil() || myPathShape->_is_nil())
    return false;

  SMESH::long_array_var anElementsId = new SMESH::long_array;

  if (MeshCheck->isChecked()) {
    // If "Select whole mesh, submesh or group" check box is on ->
    // get all elements of the required type from the object selected

    // if MESH object is selected
    if (!CORBA::is_nil(SMESH::SMESH_Mesh::_narrow(myIDSource))) {
      // get mesh
      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow(myIDSource);
      // get IDs from mesh...
      if (Elements1dRB->isChecked())
	// 1d elements
	anElementsId = aMesh->GetElementsByType(SMESH::EDGE);
      else if (Elements2dRB->isChecked()) {
	anElementsId = aMesh->GetElementsByType(SMESH::FACE);
      }
    }
    // SUBMESH is selected
    if (!CORBA::is_nil(SMESH::SMESH_subMesh::_narrow(myIDSource))) {
      // get submesh
      SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(myIDSource);
      // get IDs from submesh
      if (Elements1dRB->isChecked())
	// 1d elements
	anElementsId = aSubMesh->GetElementsByType(SMESH::EDGE);
      else if (Elements2dRB->isChecked())
	// 2d elements
	anElementsId = aSubMesh->GetElementsByType(SMESH::FACE);
    }
    // GROUP is selected
    if (!CORBA::is_nil(SMESH::SMESH_GroupBase::_narrow(myIDSource))) {
      // get smesh group
      SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow(myIDSource);
      // get IDs from group
      // 1d elements or 2d elements
      if (Elements1dRB->isChecked() && aGroup->GetType() == SMESH::EDGE ||
	   Elements2dRB->isChecked() && aGroup->GetType() == SMESH::FACE)
	anElementsId = aGroup->GetListOfID();
    }
  } else {
    // If "Select whole mesh, submesh or group" check box is off ->
    // use only elements of given type selected by user

    SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh();
    if (aMesh) {
      QStringList aListElementsId = QStringList::split(" ", ElementsLineEdit->text(), false);
      anElementsId = new SMESH::long_array;
      anElementsId->length(aListElementsId.count());
      bool bOk;
      int j = 0;
      for (int i = 0; i < aListElementsId.count(); i++) {
	long ind = aListElementsId[ i ].toLong(&bOk);
	if  (bOk) {
	  const SMDS_MeshElement* e = aMesh->FindElement(ind);
	  if (e) {
	    bool typeMatch = Elements1dRB->isChecked() && e->GetType() == SMDSAbs_Edge ||
	                     Elements2dRB->isChecked() && e->GetType() == SMDSAbs_Face;
	    if (typeMatch)
	      anElementsId[ j++ ] = ind;
	  }
	}
      }
      anElementsId->length(j);
    }
  }

  if (anElementsId->length() <= 0) {
    return false;
  }

  if (StartPointLineEdit->text().stripWhiteSpace().isEmpty()) {
    return false;
  }

  bool bOk;
  long aNodeStart = StartPointLineEdit->text().toLong(&bOk);
  if (!bOk) {
    return false;
  }

  // get angles
  SMESH::double_array_var anAngles = new SMESH::double_array;
  if (AnglesCheck->isChecked()) {
    anAngles->length(AnglesList->count());
    int j = 0;
    bool bOk;
    for (int i = 0; i < AnglesList->count(); i++) {
      double angle = AnglesList->text(i).toDouble(&bOk);
      if  (bOk)
	anAngles[ j++ ] = angle*PI/180;
    }
    anAngles->length(j);
  }

  // get base point
  SMESH::PointStruct aBasePoint;
  if (BasePointCheck->isChecked()) {
    aBasePoint.x = XSpin->GetValue();
    aBasePoint.y = YSpin->GetValue();
    aBasePoint.z = ZSpin->GetValue();
  }

  try {
    SUIT_OverrideCursor wc;
    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    if ( LinearAnglesCheck->isChecked() )
      anAngles = aMeshEditor->LinearAnglesVariation( myPathMesh, myPathShape, anAngles );

    SMESH::SMESH_MeshEditor::Extrusion_Error retVal;
    if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
      SMESH::ListOfGroups_var groups = 
        aMeshEditor->ExtrusionAlongPathMakeGroups(anElementsId, myPathMesh,
                                                  myPathShape, aNodeStart,
                                                  AnglesCheck->isChecked(), anAngles,
                                                  BasePointCheck->isChecked(), aBasePoint, retVal);
    else
      retVal = aMeshEditor->ExtrusionAlongPath(anElementsId, myPathMesh,
                                               myPathShape, aNodeStart,
                                               AnglesCheck->isChecked(), anAngles,
                                               BasePointCheck->isChecked(), aBasePoint);

    //wc.stop();
    wc.suspend();
    switch (retVal) {
    case SMESH::SMESH_MeshEditor::EXTR_NO_ELEMENTS:
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     tr("SMESH_ERROR"),
			     tr("NO_ELEMENTS_SELECTED"),
			     tr("SMESH_BUT_OK"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_PATH_NOT_EDGE:
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     tr("SMESH_ERROR"),
			     tr("SELECTED_PATH_IS_NOT_EDGE"),
			     tr("SMESH_BUT_OK"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE:
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     tr("SMESH_ERROR"),
			     tr("BAD_SHAPE_TYPE"),
			     tr("SMESH_BUT_OK"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE:
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     tr("SMESH_ERROR"),
			     tr("EXTR_BAD_STARTING_NODE"),
			     tr("SMESH_BUT_OK"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_BAD_ANGLES_NUMBER:
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     tr("SMESH_ERROR"),
			     tr("WRONG_ANGLES_NUMBER"),
			     tr("SMESH_BUT_OK"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_CANT_GET_TANGENT:
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     tr("SMESH_ERROR"),
			     tr("CANT_GET_TANGENT"),
			     tr("SMESH_BUT_OK"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_OK:
      break;
    }
  } catch (...) {
    return false;
  }

  //mySelectionMgr->clearSelected();
  SMESH::Update( myMeshActor->getIO(), myMeshActor->GetVisibility() );
  if ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() )
    mySMESHGUI->updateObjBrowser(true); // new groups may appear
  //SMESH::UpdateView();
  Init(false);
  ConstructorsClicked(GetConstructorId());
  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  : Called when user presses <OK> button
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::ClickOnOk()
{
  if (ClickOnApply())
    reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::ClickOnHelp()
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
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=================================================================================
// function : reject()
// purpose  : Called when dialog box is closed
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::reject()
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearFilters();
  //mySelectionMgr->clearSelected();
  SMESH::SetPickable(); // ???
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=======================================================================
// function : onTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  // return if busy
  if (myBusy) return;

  // set busy flag
  SetBusy sb (this);

  if (send != StartPointLineEdit && send != ElementsLineEdit)
    send = ElementsLineEdit;

  if (send == ElementsLineEdit && myEditCurrentArgument == ElementsLineEdit) {
    // hilight entered elements
    SMDS_Mesh* aMesh = 0;
    if (myMeshActor)
      aMesh = myMeshActor->GetObject()->GetMesh();

    if (aMesh) {
      //mySelectionMgr->clearSelected();
      //mySelectionMgr->AddIObject(myMeshActor->getIO());
      SALOME_ListIO aList;
      aList.Append(myMeshActor->getIO());
      mySelectionMgr->setSelectedObjects(aList, false);

      QStringList aListId = QStringList::split(" ", theNewText, false);
      bool bOk;
      const Handle(SALOME_InteractiveObject)& anIO = myMeshActor->getIO();
      TColStd_MapOfInteger newIndices;
      for (int i = 0; i < aListId.count(); i++) {
	long ind = aListId[ i ].toLong(&bOk);
	if (bOk) {
	  const SMDS_MeshElement* e = aMesh->FindElement(ind);
	  if (e) {
	    // check also type of element
	    bool typeMatch = Elements1dRB->isChecked() && e->GetType() == SMDSAbs_Edge ||
	                     Elements2dRB->isChecked() && e->GetType() == SMDSAbs_Face;
	    if (typeMatch)
	      newIndices.Add(e->GetID());
	  }
	}
      }
      mySelector->AddOrRemoveIndex(anIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->highlight( anIO, true, true );
    }
  } else if (send == StartPointLineEdit &&
             myEditCurrentArgument == StartPointLineEdit) {
    if (!myPathMesh->_is_nil()) {
      SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPathMesh);
      SMDS_Mesh* aMesh = 0;
      if (aPathActor)
	aMesh = aPathActor->GetObject()->GetMesh();
      if (aMesh) {
	//mySelectionMgr->clearSelected();
	//mySelectionMgr->AddIObject(aPathActor->getIO());
        SALOME_ListIO aList;
        aList.Append(aPathActor->getIO());
        mySelectionMgr->setSelectedObjects(aList, false);

	bool bOk;
	long ind = theNewText.toLong(&bOk);
	if (bOk) {
	  const SMDS_MeshNode* n = aMesh->FindNode(ind);
	  if (n) {
	    //if (!mySelectionMgr->IsIndexSelected(aPathActor->getIO(), n->GetID())) {
            TColStd_MapOfInteger newIndices;
	    newIndices.Add(n->GetID());
	    mySelector->AddOrRemoveIndex( aPathActor->getIO(), newIndices, false );
	    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	      aViewWindow->highlight( aPathActor->getIO(), true, true );
	  }
	}
      }
    }
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::SelectionIntoArgument()
{
  if (myBusy) return;

  // return if dialog box is inactive
  if (!GroupButtons->isEnabled())
    return;

  // selected objects count
  const SALOME_ListIO& aList = mySelector->StoredIObjects();
  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  // set busy flag
  SetBusy sb (this);

  if (myEditCurrentArgument == ElementsLineEdit) {
    // we are now selecting mesh elements (or whole mesh/submesh/group)
    // reset
    ElementsLineEdit->clear();
    myMesh      = SMESH::SMESH_Mesh::_nil();
    myIDSource  = SMESH::SMESH_IDSource::_nil();
    myMeshActor = 0;

    // try to get mesh from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();
    myMesh = SMESH::GetMeshByIO(IO);
    if (myMesh->_is_nil())
      return;

    // MakeGroups is available if there are groups
    if ( myMesh->NbGroups() == 0 ) {
      MakeGroupsCheck->setChecked(false);
      MakeGroupsCheck->setEnabled(false);
    } else {
      MakeGroupsCheck->setEnabled(true);
    }
    // find actor
    myMeshActor = SMESH::FindActorByObject(myMesh);
    if (!myMeshActor)
      return;

    if (MeshCheck->isChecked()) {
      // If "Select whole mesh, submesh or group" check box is on ->
      // get ID source and put it's name to the edit box
      QString aString;
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

      myIDSource = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
      ElementsLineEdit->setText(aString);
    } else {
      // If "Select whole mesh, submesh or group" check box is off ->
      // try to get selected elements IDs
      QString aString;
      //int aNbUnits = SMESH::GetNameOfSelectedElements(mySelectionMgr, aString);
      SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
      ElementsLineEdit->setText(aString);
    }
  } else if (myEditCurrentArgument == PathMeshLineEdit) {
    // we are now selecting path mesh
    // reset
    PathMeshLineEdit->clear();
    myPathMesh = SMESH::SMESH_Mesh::_nil();
    PathShapeLineEdit->clear();
    myPathShape = GEOM::GEOM_Object::_nil();
    StartPointLineEdit->clear();

    // try to get mesh from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();
    myPathMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
    if(myPathMesh->_is_nil())
      return;

    QString aString;
    SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
    PathMeshLineEdit->setText(aString);
  } else if (myEditCurrentArgument == PathShapeLineEdit) {
    // we are now selecting path mesh
    // reset
    PathShapeLineEdit->clear();
    myPathShape = GEOM::GEOM_Object::_nil();
    StartPointLineEdit->clear();

    // return if path mesh is not yet selected
    if (myPathMesh->_is_nil())
      return;

    // try to get shape from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();
    myPathShape = SMESH::IObjectToInterface<GEOM::GEOM_Object>(IO);
    if (myPathShape->_is_nil())
      return;

    QString aString;
    SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
    PathShapeLineEdit->setText(aString);
  } else if (myEditCurrentArgument == StartPointLineEdit) {
    // we are now selecting start point of path
    // reset
    StartPointLineEdit->clear();

    // return if path mesh or path shape is not yet selected
    if (myPathMesh->_is_nil() || myPathShape->_is_nil())
      return;

    // try to get shape from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();

    SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPathMesh);
    if ( !aPathActor )
      return;
    
    QString aString;
    int aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, aPathActor->getIO(), aString);
    if (aNbUnits == 1)
      StartPointLineEdit->setText(aString.stripWhiteSpace());

  } else if (myEditCurrentArgument == XSpin) {
    // we are now selecting base point
    // reset is not performed here!

    // return if is not enabled
    if (!BasePointGrp->isEnabled())
      return;

    // try to get shape from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();

    // check if geom vertex is selected
    GEOM::GEOM_Object_var aGeomObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(IO);
    TopoDS_Vertex aVertex;
    if (!aGeomObj->_is_nil()) {
      if (aGeomObj->IsShape() && GEOMBase::GetShape(aGeomObj, aVertex) && !aVertex.IsNull()) {
	gp_Pnt aPnt = BRep_Tool::Pnt(aVertex);
	XSpin->SetValue(aPnt.X());
	YSpin->SetValue(aPnt.Y());
	ZSpin->SetValue(aPnt.Z());
      }
      return;
    }

    // check if smesh node is selected
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(IO);
    if (aMesh->_is_nil())
      return;

    QString aString;
    int aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, myMeshActor->getIO(), aString);
    // return if more than one node is selected
    if (aNbUnits != 1)
      return;

    SMESH_Actor* aMeshActor = SMESH::FindActorByObject(aMesh);
    if (!aMeshActor)
      return;

    SMDS_Mesh* mesh = aMeshActor->GetObject()->GetMesh();
    if (!mesh)
      return;

    const SMDS_MeshNode* n = mesh->FindNode(aString.toLong());
    if (!n)
      return;

    XSpin->SetValue(n->X());
    YSpin->SetValue(n->Y());
    ZSpin->SetValue(n->Z());
  }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument()
{
  QToolButton* send = (QToolButton*)sender();
  if (send != SelectElementsButton   &&
      send != SelectPathMeshButton   &&
      send != SelectPathShapeButton  &&
      send != SelectStartPointButton &&
      send != SelectBasePointButton)
    return;
  SetEditCurrentArgument(send);
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::SetEditCurrentArgument (QToolButton* button)
{
  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();
  SMESH::SetPickable();

  if (button == SelectElementsButton) {
    myEditCurrentArgument = ElementsLineEdit;
    SMESH::SetPointRepresentation(false);
    if (MeshCheck->isChecked()) {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(ActorSelection);
      mySelectionMgr->installFilter(myElementsFilter);
    } else {
      if (Elements1dRB->isChecked())
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(EdgeSelection);
	}
      else if (Elements2dRB->isChecked())
	{
	  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	    aViewWindow->SetSelectionMode(FaceSelection);
	}
    }
  } else if (button == SelectPathMeshButton) {
    myEditCurrentArgument = PathMeshLineEdit;
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(myPathMeshFilter);
  }
  else if (button == SelectPathShapeButton) {
    myEditCurrentArgument = PathShapeLineEdit;
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);

    if (!myPathMesh->_is_nil()) {
      GEOM::GEOM_Object_var aMainShape = myPathMesh->GetShapeToMesh();
      SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPathMesh);

      if (!aMainShape->_is_nil() && aPathActor)
	mySelectionMgr->installFilter(new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE, -1,
                                                              TopAbs_EDGE, aMainShape));
	//SMESH::SetPickable(aPathActor);
    }
  }
  else if (button == SelectStartPointButton) {
    myEditCurrentArgument = StartPointLineEdit;
    if (!myPathMesh->_is_nil()) {
      SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPathMesh);
      if (aPathActor) {
	SMESH::SetPointRepresentation(true);
	if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	  aViewWindow->SetSelectionMode(NodeSelection);
	SMESH::SetPickable(aPathActor);
      }
    }
  }
  else if (button == SelectBasePointButton) {
    myEditCurrentArgument = XSpin;
    SMESH::SetPointRepresentation(true);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);

    SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter(MESHorSUBMESH);
    SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter(GROUP);
    SMESH_NumberFilter* aVertexFilter      = new SMESH_NumberFilter ("GEOM", TopAbs_SHAPE,
                                                                     -1, TopAbs_VERTEX);
    QPtrList<SUIT_SelectionFilter> aListOfFilters;
    if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
    if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);
    if (aVertexFilter)        aListOfFilters.append(aVertexFilter);

    mySelectionMgr->installFilter(new SMESH_LogicalFilter
                                  (aListOfFilters, SMESH_LogicalFilter::LO_OR));
  }

  if (myEditCurrentArgument && !myEditCurrentArgument->hasFocus())
    myEditCurrentArgument->setFocus();

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : Deactivates this dialog
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::DeactivateActiveDialog()
{
  if (GroupConstructors->isEnabled()) {
    GroupConstructors->setEnabled(false);
    GroupArguments->setEnabled(false);
    GroupButtons->setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  : Activates this dialog
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::ActivateThisDialog()
{
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox(this);

  ConstructorsClicked(GetConstructorId());
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  : Mouse enter event
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled())
    ActivateThisDialog();
}

//=======================================================================
// function : onSelectMesh()
// purpose  :
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onSelectMesh()
{
  bool toSelectMesh = MeshCheck->isChecked();

  ElementsLineEdit->setReadOnly(toSelectMesh);
  ElementsLab->setText(toSelectMesh ? tr("SMESH_NAME") : tr("SMESH_ID_ELEMENTS"));
  ElementsLineEdit->clear();

  SetEditCurrentArgument(SelectElementsButton);
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_ExtrusionAlongPathDlg::GetConstructorId()
{
  if (GroupConstructors != NULL && GroupConstructors->selected() != NULL)
    return GroupConstructors->id(GroupConstructors->selected());
  return -1;
}

//=======================================================================
// function : onAnglesCheck()
// purpose  : called when "Use Angles" check box is switched
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onAnglesCheck()
{
  AnglesGrp->setEnabled(AnglesCheck->isChecked());
}

//=======================================================================
// function : onBasePointCheck()
// purpose  : called when "Use Base Point" check box is switched
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onBasePointCheck()
{
  BasePointGrp->setEnabled(BasePointCheck->isChecked());
}

//=======================================================================
// function : OnAngleAdded()
// purpose  : Called when user adds angle to the list
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::OnAngleAdded()
{
  AnglesList->insertItem(QString::number(AngleSpin->GetValue()));
}

//=======================================================================
// function : OnAngleRemoved()
// purpose  : Called when user removes angle(s) from the list
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::OnAngleRemoved()
{
  QList<QListBoxItem> aList;
  aList.setAutoDelete(false);
  for (int i = 0; i < AnglesList->count(); i++)
    if (AnglesList->isSelected(i))
      aList.append(AnglesList->item(i));

  for (int i = 0; i < aList.count(); i++)
    delete aList.at(i);
}

//=================================================================================
// function : eventFilter()
// purpose  : event filter ???
//=================================================================================
bool SMESHGUI_ExtrusionAlongPathDlg::eventFilter (QObject* object, QEvent* event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent* ke = (QKeyEvent*)event;
    if (object == AnglesList) {
      if (ke->key() == Key_Delete)
	OnAngleRemoved();
    }
  }
  else if (event->type() == QEvent::FocusIn) {
    if (object == ElementsLineEdit) {
      if (myEditCurrentArgument != ElementsLineEdit)
	SetEditCurrentArgument(SelectElementsButton);
    }
    else if (object == StartPointLineEdit) {
      if (myEditCurrentArgument != StartPointLineEdit)
	SetEditCurrentArgument(SelectStartPointButton);
    }
    else if (object == XSpin->editor() || object == YSpin->editor() || object == ZSpin->editor()) {
      if (myEditCurrentArgument != XSpin)
	SetEditCurrentArgument(SelectBasePointButton);
    }
  }
  return QDialog::eventFilter(object, event);
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Key_F1 )
    {
      e->accept();
      ClickOnHelp();
    }
}
