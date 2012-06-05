// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
// File   : SMESHGUI_ExtrusionAlongPathDlg.cxx
// Author : Vadim SANDLER, Open CASCADE S.A.S.
// SMESH includes

#include "SMESHGUI_ExtrusionAlongPathDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_NumberFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

#include <SMDS_Mesh.hxx>

// SALOME GEOM includes
#include <GEOMBase.h>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Session.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewWindow.h>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

class SMESHGUI_ExtrusionAlongPathDlg::SetBusy
{
public:
  SetBusy( SMESHGUI_ExtrusionAlongPathDlg* _dlg )
  {
    myDlg = _dlg; 
    myDlg->myBusy = true;
  }
  
  ~SetBusy()
  { 
    myDlg->myBusy = false;
  }
  
private:
  SMESHGUI_ExtrusionAlongPathDlg* myDlg;
};

//=================================================================================
// function : SMESHGUI_ExtrusionAlongPathDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_ExtrusionAlongPathDlg::SMESHGUI_ExtrusionAlongPathDlg( SMESHGUI* theModule )
  : SMESHGUI_PreviewDlg( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myFilterDlg( 0 )
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap edgeImage   ( mgr->loadPixmap("SMESH", tr("ICON_DLG_EDGE")));
  QPixmap faceImage   ( mgr->loadPixmap("SMESH", tr("ICON_DLG_TRIANGLE")));
  QPixmap selectImage ( mgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap addImage    ( mgr->loadPixmap("SMESH", tr("ICON_APPEND")));
  QPixmap removeImage ( mgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  myType = -1;

  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("EXTRUSION_ALONG_PATH"));
  setSizeGripEnabled(true);

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(SPACING);
  topLayout->setMargin(MARGIN);

  /***************************************************************/
  // Elements type group box (1d / 2d elements)
  ConstructorsBox = new QGroupBox(tr("SMESH_EXTRUSION"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING); ConstructorsBoxLayout->setMargin(MARGIN);

  Elements1dRB = new QRadioButton(ConstructorsBox);
  Elements1dRB->setIcon(edgeImage);
  Elements2dRB = new QRadioButton(ConstructorsBox);
  Elements2dRB->setIcon(faceImage);
  Elements1dRB->setChecked(true);

  // layouting
  ConstructorsBoxLayout->addWidget(Elements1dRB);
  ConstructorsBoxLayout->addWidget(Elements2dRB);
  GroupConstructors->addButton(Elements1dRB,  0);
  GroupConstructors->addButton(Elements2dRB, 1);

  /***************************************************************/
  // Arguments group box
  GroupArguments = new QGroupBox(tr("EXTRUSION_1D"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING); GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  ElementsLab = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QToolButton(GroupArguments);
  SelectElementsButton->setIcon(selectImage);

  ElementsLineEdit = new QLineEdit(GroupArguments);
  ElementsLineEdit->setValidator(myIdValidator);
  ElementsLineEdit->setMaxLength(-1);
  myFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myFilterBtn,   SIGNAL(clicked()), this, SLOT(setFilters()));

  // Controls for the whole mesh selection
  MeshCheck = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for path selection
  PathGrp = new QGroupBox(tr("SMESH_PATH"), GroupArguments);
  QGridLayout* PathGrpLayout = new QGridLayout(PathGrp);
  PathGrpLayout->setSpacing(SPACING); PathGrpLayout->setMargin(MARGIN);

  // Controls for path mesh selection
  QLabel* PathMeshLab = new QLabel(tr("SMESH_PATH_MESH"), PathGrp);

  SelectPathMeshButton = new QToolButton(PathGrp);
  SelectPathMeshButton->setIcon(selectImage);

  PathMeshLineEdit = new QLineEdit(PathGrp);
  PathMeshLineEdit->setReadOnly(true);

  // Controls for path starting point selection
  QLabel* StartPointLab = new QLabel(tr("SMESH_PATH_START"), PathGrp);

  SelectStartPointButton = new QToolButton(PathGrp);
  SelectStartPointButton->setIcon(selectImage);

  StartPointLineEdit = new QLineEdit(PathGrp);
  StartPointLineEdit->setValidator(new QIntValidator(this));

  // layouting
  PathGrpLayout->addWidget(PathMeshLab,            0, 0);
  PathGrpLayout->addWidget(SelectPathMeshButton,   0, 1);
  PathGrpLayout->addWidget(PathMeshLineEdit,       0, 2);
  PathGrpLayout->addWidget(StartPointLab,          1, 0);
  PathGrpLayout->addWidget(SelectStartPointButton, 1, 1);
  PathGrpLayout->addWidget(StartPointLineEdit,     1, 2);

  BasePointGrp = new QGroupBox(tr("SMESH_BASE_POINT"), GroupArguments);
  BasePointGrp->setCheckable(true);
  BasePointGrp->setChecked(false);
  QHBoxLayout* BasePointGrpLayout = new QHBoxLayout(BasePointGrp);
  BasePointGrpLayout->setSpacing(SPACING); BasePointGrpLayout->setMargin(MARGIN);

  SelectBasePointButton = new QToolButton(BasePointGrp);
  SelectBasePointButton->setIcon(selectImage);

  QLabel* XLab  = new QLabel(tr("SMESH_X"), BasePointGrp);
  XSpin = new SMESHGUI_SpinBox(BasePointGrp);
  QLabel* YLab  = new QLabel(tr("SMESH_Y"), BasePointGrp);
  YSpin = new SMESHGUI_SpinBox(BasePointGrp);
  QLabel* ZLab  = new QLabel(tr("SMESH_Z"), BasePointGrp);
  ZSpin = new SMESHGUI_SpinBox(BasePointGrp);

  // layouting
  BasePointGrpLayout->addWidget(SelectBasePointButton);
  BasePointGrpLayout->addWidget(XLab);
  BasePointGrpLayout->addWidget(XSpin);
  BasePointGrpLayout->addWidget(YLab);
  BasePointGrpLayout->addWidget(YSpin);
  BasePointGrpLayout->addWidget(ZLab);
  BasePointGrpLayout->addWidget(ZSpin);

  AnglesGrp = new QGroupBox(tr("SMESH_ANGLES"), GroupArguments);
  AnglesGrp->setCheckable(true);
  AnglesGrp->setChecked(false);
  QGridLayout* AnglesGrpLayout = new QGridLayout(AnglesGrp);
  AnglesGrpLayout->setSpacing(SPACING); AnglesGrpLayout->setMargin(MARGIN);

  AnglesList = new QListWidget(AnglesGrp);
  AnglesList->setSelectionMode(QListWidget::ExtendedSelection);

  AddAngleButton = new QToolButton(AnglesGrp);
  AddAngleButton->setIcon(addImage);

  RemoveAngleButton = new QToolButton(AnglesGrp);
  RemoveAngleButton->setIcon(removeImage);

  AngleSpin = new SMESHGUI_SpinBox(AnglesGrp);

  LinearAnglesCheck = new QCheckBox(tr("LINEAR_ANGLES"), AnglesGrp);

  // layouting
  AnglesGrpLayout->addWidget(AnglesList,        0, 0, 4, 1);
  AnglesGrpLayout->addWidget(AddAngleButton,    0, 1);
  AnglesGrpLayout->addWidget(RemoveAngleButton, 2, 1);
  AnglesGrpLayout->addWidget(AngleSpin,         0, 2);
  AnglesGrpLayout->addWidget(LinearAnglesCheck, 4, 0);
  AnglesGrpLayout->setRowMinimumHeight(1, 10);
  AnglesGrpLayout->setRowStretch(3, 10);

  // CheckBox for groups generation
  MakeGroupsCheck = new QCheckBox(tr("SMESH_MAKE_GROUPS"), GroupArguments);
  MakeGroupsCheck->setChecked(true);

  //Preview check box
  myPreviewCheckBox = new QCheckBox(tr("PREVIEW"), GroupArguments);

  // layouting
  GroupArgumentsLayout->addWidget(ElementsLab,          0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton, 0, 1);
  GroupArgumentsLayout->addWidget(ElementsLineEdit,     0, 2);
  GroupArgumentsLayout->addWidget(myFilterBtn,          0, 3);
  GroupArgumentsLayout->addWidget(MeshCheck,            1, 0, 1, 4);
  GroupArgumentsLayout->addWidget(PathGrp,              2, 0, 1, 4);
  GroupArgumentsLayout->addWidget(BasePointGrp,         3, 0, 1, 4);
  GroupArgumentsLayout->addWidget(AnglesGrp,            4, 0, 1, 4);
  GroupArgumentsLayout->addWidget(myPreviewCheckBox,    5, 0, 1, 4);
  GroupArgumentsLayout->addWidget(MakeGroupsCheck,      6, 0, 1, 4);

  /***************************************************************/
  // common buttons group box
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING); GroupButtonsLayout->setMargin(MARGIN);

  OkButton = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  OkButton->setAutoDefault(true);
  OkButton->setDefault(true);

  ApplyButton = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  ApplyButton->setAutoDefault(true);

  CloseButton = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  CloseButton->setAutoDefault(true);

  HelpButton = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  HelpButton->setAutoDefault(true);

  // layouting
  GroupButtonsLayout->addWidget(OkButton);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(ApplyButton);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(CloseButton);
  GroupButtonsLayout->addWidget(HelpButton);

  /***************************************************************/
  // layouting
  topLayout->addWidget(ConstructorsBox);
  topLayout->addWidget(GroupArguments);
  topLayout->addWidget(GroupButtons);

  /***************************************************************/
  // Initialisations
  XSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  YSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  ZSpin->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  AngleSpin->RangeStepAndValidator(-180.0, 180.0, 5.0, "angle_precision");

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox(this);

  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  SMESH_TypeFilter* aMeshOrSubMeshFilter = new SMESH_TypeFilter (MESHorSUBMESH);
  SMESH_TypeFilter* aSmeshGroupFilter    = new SMESH_TypeFilter (GROUP);

  QList<SUIT_SelectionFilter*> aListOfFilters;
  if (aMeshOrSubMeshFilter) aListOfFilters.append(aMeshOrSubMeshFilter);
  if (aSmeshGroupFilter)    aListOfFilters.append(aSmeshGroupFilter);

  myElementsFilter = new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);
  //myPathMeshFilter = new SMESH_TypeFilter (MESH);
  myPathMeshFilter = new SMESH_TypeFilter(MESHorSUBMESH);

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

  connect(GroupConstructors, SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectElementsButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectPathMeshButton,   SIGNAL(clicked()), this, SLOT(SetEditCurrentArgument()));
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

  connect(XSpin,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(YSpin,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(ZSpin,  SIGNAL(valueChanged(double)), this, SLOT(toDisplaySimulation()));
  connect(AddAngleButton,  SIGNAL(clicked()), this, SLOT(toDisplaySimulation()));
  connect(RemoveAngleButton, SIGNAL(clicked()), this, SLOT(toDisplaySimulation()));
  connect(LinearAnglesCheck, SIGNAL(toggled(bool)), SLOT(onSelectMesh()));


  //To Connect preview check box
  connectPreviewControl();

  AnglesList->installEventFilter(this);
  ElementsLineEdit->installEventFilter(this);
  StartPointLineEdit->installEventFilter(this);
  XSpin->editor()->installEventFilter(this);
  YSpin->editor()->installEventFilter(this);
  ZSpin->editor()->installEventFilter(this);
}

//=================================================================================
// function : ~SMESHGUI_ExtrusionAlongPathDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_ExtrusionAlongPathDlg::~SMESHGUI_ExtrusionAlongPathDlg()
{
  // no need to delete child widgets, Qt does it all for us
  if ( myFilterDlg != 0 ) {
    myFilterDlg->setParent( 0 );
    delete myFilterDlg;
  }
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
  myPath  = SMESH::SMESH_IDSource::_nil();

  ElementsLineEdit->clear();
  PathMeshLineEdit->clear();
  StartPointLineEdit->clear();

  if (ResetControls) {
    XSpin->SetValue(0.0);
    YSpin->SetValue(0.0);
    ZSpin->SetValue(0.0);

    AngleSpin->SetValue(45);
    MeshCheck->setChecked(false);
    ConstructorsClicked(0);
    onSelectMesh();
    myPreviewCheckBox->setChecked(false);
    onDisplaySimulation(false);
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

  hidePreview();

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

  //if (myMesh->_is_nil() || MeshCheck->isChecked() && myIDSource->_is_nil() ||
  //    !myMeshActor || myPathMesh->_is_nil() || myPathShape->_is_nil())
  if ( myMesh->_is_nil() || (MeshCheck->isChecked() && myIDSource->_is_nil()) ||
       /*!myMeshActor ||*/ myPath->_is_nil() )
    return false;

  if (!isValid())
    return false;

  SMESH::long_array_var anElementsId = getSelectedElements();

  if (StartPointLineEdit->text().trimmed().isEmpty()) {
    return false;
  }
  
  bool bOk;
  long aNodeStart = StartPointLineEdit->text().toLong(&bOk);
  if (!bOk) {
    return false;
  }

  QStringList aParameters;
  
  //get angles
  SMESH::double_array_var anAngles = getAngles();
  
  for (int i = 0; i < myAnglesList.count(); i++) 
    aParameters << AnglesList->item(i)->text();


  // get base point
  SMESH::PointStruct aBasePoint;
  if (BasePointGrp->isChecked()) {
    aBasePoint.x = XSpin->GetValue();
    aBasePoint.y = YSpin->GetValue();
    aBasePoint.z = ZSpin->GetValue();
  }

  aParameters << XSpin->text();
  aParameters << YSpin->text();
  aParameters << ZSpin->text();

  try {
    SUIT_OverrideCursor wc;

    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    SMESH::SMESH_MeshEditor::Extrusion_Error retVal;

    myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

    bool NeedGroups = ( MakeGroupsCheck->isEnabled() && MakeGroupsCheck->isChecked() );
    SMESH::ElementType ElemType = SMESH::FACE;
    if( GetConstructorId() == 0 )
      ElemType = SMESH::EDGE;
    if( !MeshCheck->isChecked() ) {
      SMESH::ListOfGroups_var groups = 
        aMeshEditor->ExtrusionAlongPathX(anElementsId, myPath, aNodeStart, AnglesGrp->isChecked(),
                                         anAngles, LinearAnglesCheck->isChecked(),
                                         BasePointGrp->isChecked(), aBasePoint,
                                         NeedGroups, ElemType, retVal);
    }
    else {
      SMESH::ListOfGroups_var groups = 
        aMeshEditor->ExtrusionAlongPathObjX(myIDSource, myPath, aNodeStart, AnglesGrp->isChecked(),
                                          anAngles, LinearAnglesCheck->isChecked(),
                                          BasePointGrp->isChecked(), aBasePoint,
                                          NeedGroups, ElemType, retVal);
    }


    wc.suspend();
    switch (retVal) {
    case SMESH::SMESH_MeshEditor::EXTR_NO_ELEMENTS:
      SUIT_MessageBox::warning(this,
                               tr("SMESH_ERROR"),
                               tr("NO_ELEMENTS_SELECTED"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_PATH_NOT_EDGE:
      SUIT_MessageBox::warning(this,
                               tr("SMESH_ERROR"),
                               tr("SELECTED_PATH_IS_NOT_EDGE"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE:
      SUIT_MessageBox::warning(this,
                               tr("SMESH_ERROR"),
                               tr("BAD_SHAPE_TYPE"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE:
      SUIT_MessageBox::warning(this,
                               tr("SMESH_ERROR"),
                               tr("EXTR_BAD_STARTING_NODE"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_BAD_ANGLES_NUMBER:
      SUIT_MessageBox::warning(this,
                               tr("SMESH_ERROR"),
                               tr("WRONG_ANGLES_NUMBER"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_CANT_GET_TANGENT:
      SUIT_MessageBox::warning(this,
                               tr("SMESH_ERROR"),
                               tr("CANT_GET_TANGENT"));
      return false; break;
    case SMESH::SMESH_MeshEditor::EXTR_OK:
      break;
    }
  } catch (...) {
    return false;
  }

  //mySelectionMgr->clearSelected();
  if ( myMeshActor )
    SMESH::Update( myMeshActor->getIO(), myMeshActor->GetVisibility() );
    
  SMESHGUI::Modified();

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
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser", 
                                                                 platform)).
                             arg(myHelpFileName));
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
  if (SMESH::GetCurrentVtkView()) {
    SMESH::RemoveFilters(); // PAL6938 -- clean all mesh entity filters
    SMESH::SetPointRepresentation(false);
    SMESH::SetPickable();
  }
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

      QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);
      bool bOk;
      const Handle(SALOME_InteractiveObject)& anIO = myMeshActor->getIO();
      TColStd_MapOfInteger newIndices;
      for (int i = 0; i < aListId.count(); i++) {
        long ind = aListId[ i ].toLong(&bOk);
        if (bOk) {
          const SMDS_MeshElement* e = aMesh->FindElement(ind);
          if (e) {
            // check also type of element
            bool typeMatch = (Elements1dRB->isChecked() && e->GetType() == SMDSAbs_Edge) ||
                             (Elements2dRB->isChecked() && e->GetType() == SMDSAbs_Face);
            if (typeMatch)
              newIndices.Add(e->GetID());
          }
        }
      }
      mySelector->AddOrRemoveIndex(anIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight( anIO, true, true );
    }
  }
  else if (send == StartPointLineEdit &&
             myEditCurrentArgument == StartPointLineEdit) {
    if (!myPath->_is_nil()) {
      SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPath);
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
    if (!myMeshActor && !MeshCheck->isChecked())
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
  }
  else if (myEditCurrentArgument == PathMeshLineEdit) {
    // we are now selecting path mesh
    // reset
    PathMeshLineEdit->clear();
    myPath = SMESH::SMESH_IDSource::_nil();
    StartPointLineEdit->clear();
    
    // try to get mesh from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();
    myPath = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(IO);
    if( myPath->_is_nil() )
      return;
    
    QString aString;
    SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);
    PathMeshLineEdit->setText(aString);
  }
  else if (myEditCurrentArgument == StartPointLineEdit) {
    // we are now selecting start point of path
    // reset
    StartPointLineEdit->clear();

    // return if path mesh or path shape is not yet selected
    if( myPath->_is_nil() )
      return;

    // try to get shape from selection
    Handle(SALOME_InteractiveObject) IO = aList.First();

    SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPath);
    if ( !aPathActor )
      return;
    
    QString aString;
    int aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, aPathActor->getIO(), aString);
    if (aNbUnits == 1)
      StartPointLineEdit->setText(aString.trimmed());

  } else if (myEditCurrentArgument == XSpin) {
    // we are now selecting base point
    // reset is not performed here!

    // return if is not enabled
    if (!BasePointGrp->isChecked())
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
  onDisplaySimulation(true);
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
  //  mySelectionMgr->clearSelected();
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
  else if (button == SelectStartPointButton) {
    myEditCurrentArgument = StartPointLineEdit;
    //if (!myPathMesh->_is_nil()) {
    if (!myPath->_is_nil()) {
      SMESH_Actor* aPathActor = SMESH::FindActorByObject(myPath);
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
    QList<SUIT_SelectionFilter*> aListOfFilters;
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
  if (ConstructorsBox->isEnabled()) {
    ConstructorsBox->setEnabled(false);
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
  ConstructorsBox->setEnabled(true);
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
  if (!ConstructorsBox->isEnabled())
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
  ElementsLineEdit->setValidator(toSelectMesh ? 0 : myIdValidator);
  ElementsLab->setText(toSelectMesh ? tr("SMESH_NAME") : tr("SMESH_ID_ELEMENTS"));
  ElementsLineEdit->clear();
  myFilterBtn->setEnabled(!toSelectMesh);

  SetEditCurrentArgument(SelectElementsButton);
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_ExtrusionAlongPathDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=======================================================================
// function : OnAngleAdded()
// purpose  : Called when user adds angle to the list
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::OnAngleAdded()
{
  QString msg;
  if( !AngleSpin->isValid( msg, true ) ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return;
  }
  AnglesList->addItem(AngleSpin->text());
  myAnglesList.append(AngleSpin->GetValue());

  updateLinearAngles();
}

//=======================================================================
// function : OnAngleRemoved()
// purpose  : Called when user removes angle(s) from the list
//=======================================================================
void SMESHGUI_ExtrusionAlongPathDlg::OnAngleRemoved()
{
  QList<QListWidgetItem*> aList = AnglesList->selectedItems();
  QListWidgetItem* anItem;
  foreach(anItem, aList) {
    myAnglesList.removeAt(AnglesList->row(anItem));
    delete anItem;
  }

  updateLinearAngles();
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
      if (ke->key() == Qt::Key_Delete)
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

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::setFilters()
{
  if(myMesh->_is_nil()) {
    SUIT_MessageBox::critical(this,
                              tr("SMESH_ERROR"),
                              tr("NO_MESH_SELECTED"));
   return;
  }
  if ( !myFilterDlg )
  {
    QList<int> types;  
    types.append( SMESH::EDGE );
    types.append( SMESH::FACE );
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, types );
  }
  myFilterDlg->Init( Elements1dRB->isChecked() ? SMESH::EDGE : SMESH::FACE );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( ElementsLineEdit );

  myFilterDlg->show();
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_ExtrusionAlongPathDlg::isValid()
{
  QString msg;
  bool ok = true;
  ok = XSpin->isValid( msg, true ) && ok;
  ok = YSpin->isValid( msg, true ) && ok;
  ok = ZSpin->isValid( msg, true ) && ok;

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}

//=================================================================================
// function : updateLinearAngles
// purpose  :
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::updateLinearAngles()
{
  bool enableLinear = true;
  for( int row = 0, nbRows = AnglesList->count(); row < nbRows; row++ ) {
    if( QListWidgetItem* anItem = AnglesList->item( row ) ) {
      enableLinear = false;
      anItem->text().toDouble(&enableLinear);
      if( !enableLinear )
        break;
    }
  }
  if( !enableLinear )
    LinearAnglesCheck->setChecked( false );
  LinearAnglesCheck->setEnabled( enableLinear );
}

//=================================================================================
// function : isValuesValid()
// purpose  : Return true in case if values entered into dialog are valid
//=================================================================================
bool SMESHGUI_ExtrusionAlongPathDlg::isValuesValid() {
  
  if ( (MeshCheck->isChecked() && myIDSource->_is_nil()) ||
       myMesh->_is_nil() ||
       myPath->_is_nil() )
    return false;
  
  if(!MeshCheck->isChecked()) {
    QStringList aListElementsId = ElementsLineEdit->text().split(" ", QString::SkipEmptyParts);
    if(aListElementsId.count() <= 0)
      return false;
  }

  bool bOk;
  StartPointLineEdit->text().toLong(&bOk);
  if (!bOk) {
    return false;
  }

  return true;
}


//=================================================================================
// function : onDisplaySimulation
// purpose  : Show/Hide preview
//=================================================================================
void SMESHGUI_ExtrusionAlongPathDlg::onDisplaySimulation( bool toDisplayPreview ) {
  if (myPreviewCheckBox->isChecked() && toDisplayPreview) {
    if(isValid() && isValuesValid()) {
      
      //Get selected elements:
      SMESH::long_array_var anElementsId = getSelectedElements();

      // get angles
      SMESH::double_array_var anAngles = getAngles();
      
      // get base point
      SMESH::PointStruct aBasePoint;
      if (BasePointGrp->isChecked()) {
        aBasePoint.x = XSpin->GetValue();
        aBasePoint.y = YSpin->GetValue();
        aBasePoint.z = ZSpin->GetValue();
      }
      bool bOk;
      long aNodeStart = StartPointLineEdit->text().toLong(&bOk);
      if (bOk) {
        
        try {
          SUIT_OverrideCursor wc;
          
          SMESH::SMESH_MeshEditor::Extrusion_Error retVal;
          SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();
          bool NeedGroups = false;
          SMESH::ElementType ElemType = SMESH::FACE;
          if( GetConstructorId() == 0 )
            ElemType = SMESH::EDGE;
          if( !MeshCheck->isChecked() ) {
            aMeshEditor->ExtrusionAlongPathX(anElementsId, myPath, aNodeStart, AnglesGrp->isChecked(),
                                             anAngles, LinearAnglesCheck->isChecked(),
                                             BasePointGrp->isChecked(), aBasePoint,
                                             NeedGroups, ElemType, retVal);
          }
          else {
            SMESH::ListOfGroups_var groups = 
              aMeshEditor->ExtrusionAlongPathObjX(myIDSource, myPath, aNodeStart, AnglesGrp->isChecked(),
                                                  anAngles, LinearAnglesCheck->isChecked(),
                                                  BasePointGrp->isChecked(), aBasePoint,
                                                  NeedGroups, ElemType, retVal);
          }
          
          wc.suspend();
          if( retVal == SMESH::SMESH_MeshEditor::EXTR_OK ) {
            SMESH::MeshPreviewStruct_var aMeshPreviewStruct = aMeshEditor->GetPreviewData();
            mySimulation->SetData(aMeshPreviewStruct._retn());
          } else {
            hidePreview();
          }
          
        } catch (...) {
          hidePreview();
        }
      } else {
        hidePreview();
      }
      
    } else {
      hidePreview();
    }
  } else {
    hidePreview();
  }
}


//=================================================================================
// function : getSelectedElements
// purpose  : return list of the selected elements
//=================================================================================
SMESH::long_array_var SMESHGUI_ExtrusionAlongPathDlg::getSelectedElements() {

  // If "Select whole mesh, submesh or group" check box is off ->
  // use only elements of given type selected by user
  SMESH::long_array_var anElementsId = new SMESH::long_array;
  if (!MeshCheck->isChecked()) {
    
    SMDS_Mesh* aMesh;
    if ( myMeshActor )
      aMesh = myMeshActor->GetObject()->GetMesh();
    
    if (aMesh) {
      QStringList aListElementsId = ElementsLineEdit->text().split(" ", QString::SkipEmptyParts);
      anElementsId = new SMESH::long_array;
      anElementsId->length(aListElementsId.count());
      bool bOk;
      int j = 0;
      for (int i = 0; i < aListElementsId.count(); i++) {
        long ind = aListElementsId[ i ].toLong(&bOk);
        if  (bOk) {
          const SMDS_MeshElement* e = aMesh->FindElement(ind);
          if (e) {
            bool typeMatch = (Elements1dRB->isChecked() && e->GetType() == SMDSAbs_Edge) ||
                             (Elements2dRB->isChecked() && e->GetType() == SMDSAbs_Face);
            if (typeMatch)
                  anElementsId[ j++ ] = ind;
          }
        }
      }
      anElementsId->length(j);
    }
  }
  return anElementsId;
}

SMESH::double_array_var SMESHGUI_ExtrusionAlongPathDlg::getAngles() {
  SMESH::double_array_var anAngles = new SMESH::double_array;
  if (AnglesGrp->isChecked()) {
    anAngles->length(myAnglesList.count());
    int j = 0;
    for (int i = 0; i < myAnglesList.count(); i++) {
      double angle = myAnglesList[i];
      anAngles[ j++ ] = angle*M_PI/180.;
    }
    anAngles->length(j);
  }
  return anAngles;
}
