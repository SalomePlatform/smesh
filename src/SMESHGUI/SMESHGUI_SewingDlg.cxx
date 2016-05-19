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
// File   : SMESHGUI_SewingDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SewingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_IdPreview.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MergeDlg.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_PreVisualObj.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>
#include <SMESH_TypeDefs.hxx>

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SALOMEDSClient_Study.hxx>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_IntSpinBox.h>
#include <SalomeApp_Tools.h>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>

#define SPACING 6
#define MARGIN  11

namespace
{
  enum ActionType { MODE_AUTO=0, MODE_MANUAL,
                    MOVE_LEFT_1=0, MOVE_RIGHT_1, MOVE_LEFT_2, MOVE_RIGHT_2,
                    GROUP_COLOR=Qt::UserRole, GROUP_INDEX };
}

//=================================================================================
/*!
 * \brief Dispalayer of free borders
 */
//=================================================================================

struct SMESHGUI_SewingDlg::BorderGroupDisplayer
{
  const SMESH::ListOfFreeBorders& myBorders;
  const SMESH::FreeBordersGroup&  myGroup;
  QColor                          myColor;
  SMESH::SMESH_Mesh_ptr           myMesh;

  std::vector< SMESH_Actor* >     myPartActors;
  SVTK_ViewWindow*                myViewWindow;
  SMESHGUI_IdPreview              myIdPreview;

  BorderGroupDisplayer( const SMESH::CoincidentFreeBorders& borders,
                        int                                 groupIndex,
                        QColor                              color,
                        SMESH::SMESH_Mesh_ptr               mesh);
  ~BorderGroupDisplayer();
  void Hide();
  void ShowGroup( bool wholeBorders );
  void ShowPart( int partIndex, bool toEdit );
  void Update();

private:
  void getPartEnds( int partIndex, std::vector<int> & ids, std::list<gp_XYZ>& coords);
};

//=================================================================================
// class    : SMESHGUI_SewingDlg()
// purpose  :
//=================================================================================
SMESHGUI_SewingDlg::SMESHGUI_SewingDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap image0 (mgr->loadPixmap("SMESH", tr("ICON_SMESH_SEWING_FREEBORDERS")));
  QPixmap image1 (mgr->loadPixmap("SMESH", tr("ICON_SMESH_SEWING_CONFORM_FREEBORDERS")));
  QPixmap image2 (mgr->loadPixmap("SMESH", tr("ICON_SMESH_SEWING_BORDERTOSIDE")));
  QPixmap image3 (mgr->loadPixmap("SMESH", tr("ICON_SMESH_SEWING_SIDEELEMENTS")));
  QPixmap image4 (mgr->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap IconRemove(mgr->loadPixmap("SMESH", tr("ICON_REMOVE")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_SEWING"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_SewingDlgLayout = new QVBoxLayout(this);
  SMESHGUI_SewingDlgLayout->setSpacing(SPACING);
  SMESHGUI_SewingDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  ConstructorsBox = new QGroupBox(tr("SMESH_SEWING"), this);
  GroupConstructors = new QButtonGroup(this);
  QHBoxLayout* ConstructorsBoxLayout = new QHBoxLayout(ConstructorsBox);
  ConstructorsBoxLayout->setSpacing(SPACING);
  ConstructorsBoxLayout->setMargin(MARGIN);

  RadioButton1 = new QRadioButton(ConstructorsBox);
  RadioButton1->setIcon(image0);
  RadioButton2 = new QRadioButton(ConstructorsBox);
  RadioButton2->setIcon(image1);
  RadioButton3 = new QRadioButton(ConstructorsBox);
  RadioButton3->setIcon(image2);
  RadioButton4 = new QRadioButton(ConstructorsBox);
  RadioButton4->setIcon(image3);

  ConstructorsBoxLayout->addWidget(RadioButton1);
  ConstructorsBoxLayout->addWidget(RadioButton2);
  ConstructorsBoxLayout->addWidget(RadioButton3);
  ConstructorsBoxLayout->addWidget(RadioButton4);
  GroupConstructors->addButton(RadioButton1, 0);
  GroupConstructors->addButton(RadioButton2, 1);
  GroupConstructors->addButton(RadioButton3, 2);
  GroupConstructors->addButton(RadioButton4, 3);

  /***************************************************************/
  GroupArguments = new QGroupBox(this);
  QVBoxLayout* GroupArgumentsLayout = new QVBoxLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  // First subgroup
  SubGroup1 = new QGroupBox(GroupArguments);
  QGridLayout* SubGroup1Layout = new QGridLayout(SubGroup1);
  SubGroup1Layout->setSpacing(SPACING);
  SubGroup1Layout->setMargin(MARGIN);

  // Controls of the first subgroup
  TextLabel1 = new QLabel(SubGroup1);
  SelectButton1  = new QPushButton(SubGroup1);
  SelectButton1->setIcon(image4);
  LineEdit1 = new QLineEdit(SubGroup1);

  TextLabel2 = new QLabel(SubGroup1);
  SelectButton2  = new QPushButton(SubGroup1);
  SelectButton2->setIcon(image4);
  LineEdit2 = new QLineEdit(SubGroup1);

  TextLabel3 = new QLabel(SubGroup1);
  SelectButton3  = new QPushButton(SubGroup1);
  SelectButton3->setIcon(image4);
  LineEdit3 = new QLineEdit(SubGroup1);

  SubGroup1Layout->addWidget(TextLabel1,    0, 0);
  SubGroup1Layout->addWidget(SelectButton1, 0, 1);
  SubGroup1Layout->addWidget(LineEdit1,     0, 2);
  SubGroup1Layout->addWidget(TextLabel2,    1, 0);
  SubGroup1Layout->addWidget(SelectButton2, 1, 1);
  SubGroup1Layout->addWidget(LineEdit2,     1, 2);
  SubGroup1Layout->addWidget(TextLabel3,    2, 0);
  SubGroup1Layout->addWidget(SelectButton3, 2, 1);
  SubGroup1Layout->addWidget(LineEdit3,     2, 2);

  // Second subgroup
  SubGroup2 = new QGroupBox(GroupArguments);
  QGridLayout* SubGroup2Layout = new QGridLayout(SubGroup2);
  SubGroup2Layout->setSpacing(SPACING);
  SubGroup2Layout->setMargin(MARGIN);

  // Controls of the first subgroup
  TextLabel4 = new QLabel(SubGroup2);
  SelectButton4  = new QPushButton(SubGroup2);
  SelectButton4->setIcon(image4);
  LineEdit4 = new QLineEdit(SubGroup2);

  TextLabel5 = new QLabel(SubGroup2);
  SelectButton5  = new QPushButton(SubGroup2);
  SelectButton5->setIcon(image4);
  LineEdit5 = new QLineEdit(SubGroup2);

  TextLabel6 = new QLabel(SubGroup2);
  SelectButton6  = new QPushButton(SubGroup2);
  SelectButton6->setIcon(image4);
  LineEdit6 = new QLineEdit(SubGroup2);

  SubGroup2Layout->addWidget(TextLabel4,    0, 0);
  SubGroup2Layout->addWidget(SelectButton4, 0, 1);
  SubGroup2Layout->addWidget(LineEdit4,     0, 2);
  SubGroup2Layout->addWidget(TextLabel5,    1, 0);
  SubGroup2Layout->addWidget(SelectButton5, 1, 1);
  SubGroup2Layout->addWidget(LineEdit5,     1, 2);
  SubGroup2Layout->addWidget(TextLabel6,    2, 0);
  SubGroup2Layout->addWidget(SelectButton6, 2, 1);
  SubGroup2Layout->addWidget(LineEdit6,     2, 2);

  // Control for the merging equal elements
  CheckBoxMerge = new QCheckBox(tr("MERGE_EQUAL_ELEMENTS"), GroupArguments);

  // Control for the polygons creation instead of splitting
  CheckBoxPolygons = new QCheckBox(tr("CREATE_POLYGONS_INSTEAD_SPLITTING"), GroupArguments);
  
  // Control for the polyedres creation to obtain conform mesh
  CheckBoxPolyedrs = new QCheckBox(tr("CREATE_POLYEDRS_NEAR_BOUNDARY"), GroupArguments);

  /***************************************************************/
  // Controls to switch free borders mode ( auto || manual )

  ModeGroup = new QGroupBox( tr( "SMESH_MODE" ), GroupArguments );
  ModeButGrp = new QButtonGroup( ModeGroup );
  QHBoxLayout* aModeGroupLayout = new QHBoxLayout( ModeGroup );
  aModeGroupLayout->setMargin( MARGIN );
  aModeGroupLayout->setSpacing( SPACING );

  QRadioButton* rb1 = new QRadioButton( tr( "SMESH_AUTOMATIC" ), ModeGroup );
  QRadioButton* rb2 = new QRadioButton( tr( "SMESH_MANUAL"    ), ModeGroup );
  ModeButGrp->addButton( rb1, MODE_AUTO );
  ModeButGrp->addButton( rb2, MODE_MANUAL );
  aModeGroupLayout->addWidget( rb1 );
  aModeGroupLayout->addWidget( rb2 );
  rb1->setChecked(true);

  /***************************************************************/
  // Controls for detecting coincident free borders

  SewFreeBordersWidget = new QWidget( GroupArguments );
  QVBoxLayout* aSewFreeBordersLayout = new QVBoxLayout( SewFreeBordersWidget );
  aSewFreeBordersLayout->setMargin( 0 );
  aSewFreeBordersLayout->setSpacing( SPACING );

  // Tolerance
  QWidget* TolAndAuto = new QWidget(SewFreeBordersWidget);
  QLabel* TextLabelTolerance = new QLabel(tr("SMESH_TOLERANCE"), TolAndAuto);
  SpinBoxTolerance = new SMESHGUI_SpinBox(TolAndAuto);
  SpinBoxTolerance->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  SpinBoxTolerance->RangeStepAndValidator(0.0, COORD_MAX, 0.00001, "len_tol_precision");
  SpinBoxTolerance->SetValue(0.); // auto-tolerance

  // Auto Sewing
  AutoSewCheck = new QCheckBox(tr("AUTO_SEWING"), TolAndAuto);
  AutoSewCheck->setChecked( true );

  // mesh
  QGroupBox* GroupMesh = new QGroupBox(tr("SMESH_MESH"), SewFreeBordersWidget);
  QHBoxLayout* GroupMeshLayout = new QHBoxLayout(GroupMesh);
  GroupMeshLayout->setSpacing(SPACING);
  GroupMeshLayout->setMargin(MARGIN);

  QLabel* TextLabelName = new QLabel(tr("SMESH_NAME"), GroupMesh);
  //SelectMeshButton = new QPushButton(GroupMesh);
  //SelectMeshButton->setIcon(IconSelect);
  LineEditMesh = new QLineEdit(GroupMesh);
  LineEditMesh->setReadOnly(true);

  GroupMeshLayout->addWidget(TextLabelName);
  //GroupMeshLayout->addWidget(SelectMeshButton);
  GroupMeshLayout->addWidget(LineEditMesh);

  QGridLayout* TolAndAutoLayout = new QGridLayout( TolAndAuto );
  TolAndAutoLayout->setSpacing(SPACING);
  TolAndAutoLayout->setMargin(0);
  TolAndAutoLayout->addWidget(GroupMesh,          0, 0, 1, 2 );
  TolAndAutoLayout->addWidget(TextLabelTolerance, 1, 0 );
  TolAndAutoLayout->addWidget(SpinBoxTolerance,   1, 1 );
  TolAndAutoLayout->addWidget(AutoSewCheck,       2, 0 );

  aSewFreeBordersLayout->addWidget( TolAndAuto );

  /******************/
  // Coincident group
  GroupCoincidentWidget = new QWidget(SewFreeBordersWidget);
  QGridLayout* GroupCoincidentLayout = new QGridLayout(GroupCoincidentWidget);
  GroupCoincidentLayout->setSpacing(SPACING);
  GroupCoincidentLayout->setMargin(0);

  QGroupBox* GroupCoincident = new QGroupBox(tr("COINCIDENT_FREE_BORDERS"), GroupCoincidentWidget);
  QGridLayout* aCoincidentLayout = new QGridLayout(GroupCoincident);
  aCoincidentLayout->setSpacing(SPACING);
  aCoincidentLayout->setMargin(MARGIN);

  /*******/
  // borders
  ListCoincident = new QListWidget(GroupCoincident);
  ListCoincident->setSelectionMode(QListWidget::ExtendedSelection);

  DetectButton      = new QPushButton(tr("DETECT"),           GroupCoincident);
  RemoveGroupButton = new QPushButton(tr("SMESH_BUT_REMOVE"), GroupCoincident);

  SelectAllCheck = new QCheckBox(tr("SELECT_ALL"), GroupCoincident);

  aCoincidentLayout->addWidget(ListCoincident,    0, 0, 5, 2);
  aCoincidentLayout->addWidget(DetectButton,      1, 2);
  aCoincidentLayout->addWidget(RemoveGroupButton, 3, 2);
  aCoincidentLayout->addWidget(SelectAllCheck,    5, 0);
  aCoincidentLayout->setRowMinimumHeight(1, 10);
  aCoincidentLayout->setRowStretch      (4, 5);
  aCoincidentLayout->setRowStretch      (5, 0);

  /*****************************************/
  // Controls for editing the selected group

  QGroupBox* GroupEdit = new QGroupBox(tr("EDIT_SELECTED_GROUP"), GroupCoincidentWidget);
  QGridLayout* GroupEditLayout = new QGridLayout(GroupEdit);
  GroupEditLayout->setSpacing(SPACING);
  GroupEditLayout->setMargin(MARGIN);

  ListEdit = new QListWidget(GroupEdit);
  ListEdit->setFlow( QListView::LeftToRight );
  ListEdit->setSelectionMode(QListWidget::ExtendedSelection);
  SetFirstButton = new QPushButton(GroupEdit);
  SetFirstButton->setIcon(QPixmap(SMESHGUI_MergeDlg::IconFirst()));
  RemoveElemButton = new QPushButton(GroupEdit);
  RemoveElemButton->setIcon(IconRemove);

  MoveBorderEndsButGrp = new QButtonGroup( GroupEdit );
  QToolButton* moveBut1 = new QToolButton( GroupEdit );
  QToolButton* moveBut2 = new QToolButton( GroupEdit );
  QToolButton* moveBut3 = new QToolButton( GroupEdit );
  QToolButton* moveBut4 = new QToolButton( GroupEdit );
  moveBut1->setArrowType( Qt::LeftArrow );
  moveBut2->setArrowType( Qt::RightArrow );
  moveBut3->setArrowType( Qt::LeftArrow );
  moveBut4->setArrowType( Qt::RightArrow );
  MoveBorderEndsButGrp->addButton( moveBut1, MOVE_LEFT_1 );
  MoveBorderEndsButGrp->addButton( moveBut2, MOVE_RIGHT_1 );
  MoveBorderEndsButGrp->addButton( moveBut3, MOVE_LEFT_2 );
  MoveBorderEndsButGrp->addButton( moveBut4, MOVE_RIGHT_2 );

  SwapBut  = new QPushButton( "<->", GroupEdit );
  BorderEndLine[0] = new QLineEdit( GroupEdit );
  BorderEndLine[1] = new QLineEdit( GroupEdit );
  BorderEndLine[0]->setReadOnly(true);
  BorderEndLine[1]->setReadOnly(true);
  QLabel* StepLabel = new QLabel(tr("STEP"), GroupEdit );
  StepSpin = new SalomeApp_IntSpinBox( 1, 100000, 1, GroupEdit,
                                       /*acceptNames=*/false, /*showTip=*/false );
  StepSpin->setValue( 1 );

  GroupEditLayout->addWidget(ListEdit,         0, 0, 1, 8);
  GroupEditLayout->addWidget(SetFirstButton,   0, 8);
  GroupEditLayout->addWidget(RemoveElemButton, 0, 9);
  GroupEditLayout->addWidget(moveBut1,         1, 0);
  GroupEditLayout->addWidget(BorderEndLine[0], 1, 1);
  GroupEditLayout->addWidget(moveBut2,         1, 2);
  GroupEditLayout->addWidget(moveBut3,         1, 3);
  GroupEditLayout->addWidget(BorderEndLine[1], 1, 4);
  GroupEditLayout->addWidget(moveBut4,         1, 5);
  GroupEditLayout->setColumnStretch(              6, 5 );
  GroupEditLayout->addWidget(SwapBut,          1, 7);
  GroupEditLayout->addWidget(StepLabel,        1, 8);
  GroupEditLayout->addWidget(StepSpin,         1, 9);
  GroupEditLayout->setRowStretch( 0, 1 );

  GroupCoincidentLayout->addWidget( GroupCoincident );
  GroupCoincidentLayout->addWidget( GroupEdit );
  GroupCoincidentLayout->setRowStretch( 0, 10 );
  GroupCoincidentLayout->setRowStretch( 1, 1 );

  aSewFreeBordersLayout->addWidget( GroupCoincidentWidget );

  // layout
  GroupArgumentsLayout->addWidget(ModeGroup);
  GroupArgumentsLayout->addWidget(SubGroup1);
  GroupArgumentsLayout->addWidget(SubGroup2);
  GroupArgumentsLayout->addWidget(SewFreeBordersWidget);
  GroupArgumentsLayout->addWidget(CheckBoxMerge);
  GroupArgumentsLayout->addWidget(CheckBoxPolygons);
  GroupArgumentsLayout->addWidget(CheckBoxPolyedrs);

  /***************************************************************/
  GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);

  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);

  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  /***************************************************************/
  SMESHGUI_SewingDlgLayout->addWidget(ConstructorsBox);
  SMESHGUI_SewingDlgLayout->addWidget(GroupArguments);
  SMESHGUI_SewingDlgLayout->addWidget(GroupButtons);
  //SMESHGUI_SewingDlgLayout->setStretch( 2, 10 );

  /* Initialisations */
  RadioButton1->setChecked(true);

  LineEdit2->setValidator(new SMESHGUI_IdValidator(this, 1));
  LineEdit3->setValidator(new SMESHGUI_IdValidator(this, 1));
  LineEdit5->setValidator(new SMESHGUI_IdValidator(this, 1));
  LineEdit6->setValidator(new SMESHGUI_IdValidator(this, 1));

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myHelpFileName = "sewing_meshes_page.html";

  myActor = 0;
  myStoredEntityMode = 0;

  setDisplayMode();
  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply,  SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp,   SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(GroupConstructors, SIGNAL(buttonClicked(int)), SLOT(ConstructorsClicked(int)));

  connect(SelectButton1, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton2, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton3, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton4, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton5, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));
  connect(SelectButton6, SIGNAL (clicked()),   this, SLOT(SetEditCurrentArgument()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),      this, SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()),            this, SLOT(onCloseView()));

  connect(LineEdit1, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit2, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit3, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit4, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit5, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit6, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));

  connect(ModeButGrp,           SIGNAL(buttonClicked(int)),     SLOT(onModeChange(int)));
  connect(AutoSewCheck,         SIGNAL(stateChanged(int)),      SLOT(onAutoSew(int)));
  connect(DetectButton,         SIGNAL(clicked()),              SLOT(onDetectClicked()));
  connect(RemoveGroupButton,    SIGNAL(clicked()),              SLOT(onRemoveGroupClicked()));
  connect(ListCoincident,       SIGNAL(itemSelectionChanged()), SLOT(onSelectGroup()));
  connect(SelectAllCheck,       SIGNAL(stateChanged(int)),      SLOT(onSelectAll(int)));
  connect(ListEdit,             SIGNAL(itemSelectionChanged()), SLOT(onSelectBorderPartFromGroup()));
  connect(SetFirstButton,       SIGNAL(clicked()),              SLOT(onSetFirstClicked()));
  connect(RemoveElemButton,     SIGNAL(clicked()),              SLOT(onRemoveElemClicked()));
  connect(MoveBorderEndsButGrp, SIGNAL(buttonClicked(int)),     SLOT(onMoveBorderEnd(int)));
  connect(SwapBut,              SIGNAL(clicked()),              SLOT(onSwapClicked()));

  ConstructorsClicked(0);
}

//=================================================================================
// function : ~SMESHGUI_SewingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_SewingDlg::~SMESHGUI_SewingDlg()
{
  for ( size_t i = 0; i < myBorderDisplayers.size(); ++i )
  {
    delete myBorderDisplayers[ i ];
    myBorderDisplayers[ i ] = 0;
  }
  myBorderDisplayers.clear();
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::Init()
{
  myBusy = false;

  if ( LineEdit1->isVisible() )
    myEditCurrentArgument = LineEdit1;
  else
    myEditCurrentArgument = LineEditMesh;
  myEditCurrentArgument->setFocus();
  //myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();
  // CheckBoxMerge->setChecked(false);
  // CheckBoxPolygons->setChecked(false);
  // CheckBoxPolyedrs->setChecked(false);
  SelectionIntoArgument();
}

//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_SewingDlg::ConstructorsClicked (int constructorId)
{
  disconnect(mySelectionMgr, 0, this, 0);
  SALOME_ListIO io;
  mySelectionMgr->selectedObjects( io );
  mySelectionMgr->clearSelected();
  LineEdit1->setText("");
  LineEdit2->setText("");
  LineEdit3->setText("");
  LineEdit4->setText("");
  LineEdit5->setText("");
  LineEdit6->setText("");
  myOk1 = myOk2 = myOk3 = myOk4 = myOk5 = myOk6 = false;
  myEditCurrentArgument = LineEdit1;
  myEditCurrentArgument->setFocus();

  if (!TextLabel5->isEnabled()) {
    TextLabel5->setEnabled(true);
    SelectButton5->setEnabled(true);
    LineEdit5->setEnabled(true);
  } else if (!TextLabel6->isEnabled()) {
    TextLabel6->setEnabled(true);
    SelectButton6->setEnabled(true);
    LineEdit6->setEnabled(true);
  }

  if (constructorId == 1 || constructorId == 3) {
    if (CheckBoxPolygons->isVisible())
      CheckBoxPolygons->hide();
    if (CheckBoxPolyedrs->isVisible())
      CheckBoxPolyedrs->hide();
  }

  CheckBoxMerge->setVisible ( constructorId == 3 );

  if (( !SubGroup1->isVisible() ) &&
      ( constructorId != 0 || ModeButGrp->checkedId() == MODE_MANUAL ))
  {
    SubGroup1->show();
    SubGroup2->show();
  }

  if ( constructorId != 0 )
  {
    ModeGroup->hide();
    SewFreeBordersWidget->hide();
    restoreDisplayMode();
  }

  bool isNodeSelection = true;

  switch (constructorId) {
  case 0:
  {
    GroupArguments->setTitle(tr("SEW_FREE_BORDERS"));
    SubGroup1->setTitle(tr("BORDER_1"));
    SubGroup2->setTitle(tr("BORDER_2"));

    if (!CheckBoxPolygons->isVisible())
      CheckBoxPolygons->show();
    if (!CheckBoxPolyedrs->isVisible())
      CheckBoxPolyedrs->show();

    if ( !ModeGroup->isVisible() )
    {
      ModeGroup->show();
    }
    onModeChange( ModeButGrp->checkedId() );

    isNodeSelection = ( ModeButGrp->checkedId() == MODE_MANUAL );

    break;
  }
  case 1:
  {
    GroupArguments->setTitle(tr("SEW_CONFORM_FREE_BORDERS"));
    SubGroup1->setTitle(tr("BORDER_1"));
    SubGroup2->setTitle(tr("BORDER_2"));

    TextLabel6->setEnabled(false);
    SelectButton6->setEnabled(false);
    LineEdit6->setEnabled(false);

    myOk6 = true;

    break;
  }
  case 2:
  {
    GroupArguments->setTitle(tr("SEW_BORDER_TO_SIDE"));
    SubGroup1->setTitle(tr("BORDER"));
    SubGroup2->setTitle(tr("SIDE"));

    TextLabel5->setEnabled(false);
    SelectButton5->setEnabled(false);
    LineEdit5->setEnabled(false);

    if (!CheckBoxPolygons->isVisible())
      CheckBoxPolygons->show();
    if (!CheckBoxPolyedrs->isVisible())
      CheckBoxPolyedrs->show();

    myOk5 = true;

    break;
  }
  case 3:
  {
    GroupArguments->setTitle(tr("SEW_SIDE_ELEMENTS"));
    SubGroup1->setTitle(tr("SIDE_1"));
    SubGroup2->setTitle(tr("SIDE_2"));

    TextLabel1->setText(tr("SMESH_ID_ELEMENTS"));
    TextLabel2->setText(tr("NODE1_TO_MERGE"));
    TextLabel3->setText(tr("NODE2_TO_MERGE"));
    TextLabel4->setText(tr("SMESH_ID_ELEMENTS"));
    TextLabel5->setText(tr("NODE1_TO_MERGE"));
    TextLabel6->setText(tr("NODE2_TO_MERGE"));

    LineEdit1->setValidator(new SMESHGUI_IdValidator(this));
    LineEdit4->setValidator(new SMESHGUI_IdValidator(this));

    isNodeSelection = false;
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(CellSelection);
    break;
  }
  }

  if (constructorId != 3) {
    TextLabel1->setText(tr("FIRST_NODE_ID"));
    TextLabel2->setText(tr("SECOND_NODE_ID"));
    TextLabel3->setText(tr("LAST_NODE_ID"));
    TextLabel4->setText(tr("FIRST_NODE_ID"));
    TextLabel5->setText(tr("SECOND_NODE_ID"));
    TextLabel6->setText(tr("LAST_NODE_ID"));

    LineEdit1->setValidator(new SMESHGUI_IdValidator(this, 1));
    LineEdit4->setValidator(new SMESHGUI_IdValidator(this, 1));
  }

  if ( myActor )
    myActor->SetPointRepresentation( isNodeSelection );

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode( isNodeSelection ? NodeSelection : ActorSelection );

  UpdateButtons();

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  mySelectionMgr->setSelectedObjects( io );

  QApplication::instance()->processEvents();
  updateGeometry();

  resize(100,100);
}

//=======================================================================
//function : storeDisplayMode
//purpose  : save representation of a mesh and switch it to wireframe mode
//=======================================================================

void SMESHGUI_SewingDlg::setDisplayMode()
{
  if ( myStoredEntityMode )
    return;
  myStoredEntityMode = 0;
  myStoredRepresentation = -1;

  if ( myActor && AutoSewCheck->isVisible() && !AutoSewCheck->isChecked() )
  {
    myStoredEntityMode     = myActor->GetEntityMode();
    myStoredRepresentation = myActor->GetRepresentation();

    myActor->SetEntityMode( myStoredEntityMode & ~SMESH_Actor::eVolumes );
    myActor->SetRepresentation( SMESH_Actor::eEdge );
  }
}

//=======================================================================
//function : restoreDisplayMode
//purpose  : restore representation of a mesh
//=======================================================================

void SMESHGUI_SewingDlg::restoreDisplayMode()
{
  if ( myActor && myStoredEntityMode )
  {
    if ( myActor->GetEntityMode() == ( myStoredEntityMode & ~SMESH_Actor::eVolumes ))
      myActor->SetEntityMode( myStoredEntityMode );

    if ( myActor->GetRepresentation() == SMESH_Actor::eEdge )
      myActor->SetRepresentation( myStoredRepresentation );

    myStoredEntityMode = 0;
    myStoredRepresentation = -1;
  }
  for ( size_t i = 0; i < myBorderDisplayers.size(); ++i )
    if ( myBorderDisplayers[ i ])
      myBorderDisplayers[ i ]->Hide();
}

//=======================================================================
//function : onModeChange
//purpose  : SLOT called when mode (auto or manual) of Sew free borders change
//=======================================================================

void SMESHGUI_SewingDlg::onModeChange( int mode )
{
  if ( mode == MODE_MANUAL )
  {
    myEditCurrentArgument = LineEdit1;
    if ( !SubGroup1->isVisible() )
      SubGroup1->show(), SubGroup2->show();
    SewFreeBordersWidget->hide();
  }
  else
  {
    myEditCurrentArgument = LineEditMesh;
    SubGroup1->hide(), SubGroup2->hide();
    if ( !SewFreeBordersWidget->isVisible() )
      SewFreeBordersWidget->show();
  }
  if ( myActor )
    myActor->SetPointRepresentation( mode == MODE_MANUAL );

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView() )
    aViewWindow->SetSelectionMode( mode == MODE_MANUAL ? NodeSelection : ActorSelection );

  onAutoSew( AutoSewCheck->isChecked() );

  QApplication::instance()->processEvents();
  updateGeometry();

  resize(100,100);
}

//=======================================================================
//function : onAutoSew
//purpose  : SLOT called when Auto Sewing check box is checked
//=======================================================================

void SMESHGUI_SewingDlg::onAutoSew( int isAuto )
{
  GroupCoincidentWidget->setVisible( !isAuto );

  QApplication::instance()->processEvents();

  SewFreeBordersWidget->hide();
  if ( ModeButGrp->checkedId() == MODE_AUTO )
    SewFreeBordersWidget->show();

  if ( isAuto )
    restoreDisplayMode();
  else
    setDisplayMode();
  SMESH::RepaintCurrentView();

  UpdateButtons();

  updateGeometry();
  resize(minimumSizeHint());
}

//=======================================================================
//function : haveBorders
//purpose  : Returns true if myBorders have been initialized
//=======================================================================

bool SMESHGUI_SewingDlg::haveBorders()
{
  return ( & myBorders.in() &&
           myBorders->borders.length() &&
           myBorders->coincidentGroups.length() );
}

//=======================================================================
//function : getGroupText
//purpose  : Returns a text of a given group of coincident free borders
//=======================================================================

QString SMESHGUI_SewingDlg::getPartText(const SMESH::FreeBorderPart& aPART)
{
  typedef CORBA::Long TInt;
  QString text;
  if ( 0 <= aPART.border && aPART.border < (TInt)myBorders->borders.length() )
  {
    const SMESH::FreeBorder& aBRD = myBorders->borders[ aPART.border ];
    if ( 0 <= aPART.node1    && aPART.node1    < (TInt)aBRD.nodeIDs.length() &&
         0 <= aPART.nodeLast && aPART.nodeLast < (TInt)aBRD.nodeIDs.length() )
    {
      text += QString("( %1 %2 %3 ) ")
        .arg( aBRD.nodeIDs[ aPART.node1 ] )
        .arg( aBRD.nodeIDs[ aPART.node2 ] )
        .arg( aBRD.nodeIDs[ aPART.nodeLast ] );
    }
  }
  return text;
}

//=======================================================================
//function : getGroupText
//purpose  : Returns a text of a given group of coincident free borders
//=======================================================================

QString SMESHGUI_SewingDlg::getGroupText(int groupIndex)
{
  QString text;

  if ( haveBorders()   &&
       groupIndex >= 0 &&
       groupIndex < (int)myBorders->coincidentGroups.length() )
  {
    const SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ groupIndex ];

    for ( CORBA::ULong iP = 0; iP < aGRP.length(); ++iP )
    {
      QString partText = getPartText( aGRP[ iP ]);
      if ( partText.isEmpty() )
        return "";
      text += partText;
    }
  }
  return text;
}

//=======================================================================
//function : onDetectClicked
//purpose  : SLOT called when [Detect] is clicked
//=======================================================================

void SMESHGUI_SewingDlg::onDetectClicked()
{
  myBusy = true;
  ListCoincident->clear();

  if ( myMesh->_is_nil() )
    return;

  SUIT_OverrideCursor wc;

  SMESH::SMESH_MeshEditor_var editor = myMesh->GetMeshEditor();
  myBorders = editor->FindCoincidentFreeBorders( SpinBoxTolerance->GetValue() );
  if ( haveBorders() )
  {
    for ( size_t i = 0; i < myBorderDisplayers.size(); ++i )
    {
      delete myBorderDisplayers[ i ];
      myBorderDisplayers[ i ] = 0;
    }
    myBorderDisplayers.resize( myBorders->coincidentGroups.length(), 0 );

    for ( uint i = 0; i < myBorders->coincidentGroups.length(); ++i )
    {
      QString groupText = getGroupText( i );
      if ( groupText.isEmpty() )
        continue;

      QColor groupColor;
      groupColor.setHsvF( float(i) / myBorders->coincidentGroups.length(), 1., 1. );
      QPixmap icon( QSize( 20, 20 ));
      icon.fill( groupColor );

      QListWidgetItem * item = new QListWidgetItem( icon, groupText, ListCoincident );
      item->setData( GROUP_COLOR, groupColor );
      item->setData( GROUP_INDEX, i );
    }
  }
  myBusy = false;

  onSelectGroup();

  UpdateButtons();
}

//=======================================================================
//function : onRemoveGroupClicked
//purpose  :
//=======================================================================

void SMESHGUI_SewingDlg::onRemoveGroupClicked()
{
  myBusy = true;
  QList<QListWidgetItem*> selItems = ListCoincident->selectedItems();
  for ( int i = 0; i < selItems.count(); ++i )
  {
    QListWidgetItem* item = selItems[ i ];
    item->setSelected( false );
    int groupIndex = item->data( GROUP_INDEX ).toInt();
    delete item;
    if ( myBorderDisplayers[ groupIndex ])
      myBorderDisplayers[ groupIndex ]->Hide();
    SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ groupIndex ];
    aGRP.length( 0 );
  }
  myBusy = false;

  onSelectGroup();
  UpdateButtons();
}

//=======================================================================
//function : showGroup
//purpose  : display a group of coincident free borders in the Viewer
//=======================================================================

void SMESHGUI_SewingDlg::showGroup( QListWidgetItem* item )
{
  if ( !item ||
       item->listWidget() != ListCoincident ||
       !haveBorders())
    return;

  int    groupIndex = item->data( GROUP_INDEX ).toInt();
  QColor groupColor = item->data( GROUP_COLOR ).value<QColor>();
  if ( groupIndex >= 0       &&
       groupIndex < (int)myBorders->coincidentGroups.length() )
  {
    if ( !myBorderDisplayers[ groupIndex ] && SMESH::GetCurrentVtkView())
      myBorderDisplayers[ groupIndex ] = new BorderGroupDisplayer( myBorders, groupIndex, groupColor, myMesh );
    bool wholeBorders = setCurrentGroup();
    if ( myBorderDisplayers[ groupIndex ])
      myBorderDisplayers[ groupIndex ]->ShowGroup( wholeBorders );
  }
}

//=======================================================================
//function : setCurrentGroup
//purpose  : set index of a current free border group to myCurGroupIndex
//=======================================================================

bool SMESHGUI_SewingDlg::setCurrentGroup()
{
  if ( !haveBorders() )
    return false;

  QList<QListWidgetItem*> selItems = ListCoincident->selectedItems();
  if ( selItems.count() != 1 )
    return false;
  
  myCurGroupIndex = selItems[0]->data( GROUP_INDEX ).toInt();

  return ( myCurGroupIndex >= 0 && myCurGroupIndex < (int)myBorders->coincidentGroups.length() );
}

//=======================================================================
//function : setCurrentPart
//purpose  : set index of a current free border of a current group to myCurPartIndex
//=======================================================================

bool SMESHGUI_SewingDlg::setCurrentPart()
{
  if ( !setCurrentGroup() )
    return false;

  if ( ListEdit->selectedItems().count() != 1 )
    return false;

  myCurPartIndex = ListEdit->currentRow();
  const SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];

  return ( myCurPartIndex >= 0 && myCurPartIndex < (int)aGRP.length() );
}

//=======================================================================
//function : onSelectGroup
//purpose  : SLOT called when selection of coincident free borders change
//=======================================================================

void SMESHGUI_SewingDlg::onSelectGroup()
{
  if ( myBusy )
    return;
  ListEdit->clear();
  BorderEndLine[0]->clear();
  BorderEndLine[1]->clear();
  for ( size_t i = 0; i < myBorderDisplayers.size(); ++i )
    if ( myBorderDisplayers[ i ])
      myBorderDisplayers[ i ]->Hide();

  QList<QListWidgetItem*> selItems = ListCoincident->selectedItems();

  RemoveGroupButton->setEnabled( selItems.count() > 0 );

  onSelectBorderPartFromGroup(); // enable buttons

  if ( !haveBorders() )
    return;

  SelectAllCheck->blockSignals( true );
  if ( ListCoincident->count() != selItems.count() )
    SelectAllCheck->setChecked( false );
  SelectAllCheck->blockSignals( false );

  if ( selItems.empty() ) // nothing selected - show all
    for ( int i = 0; i < ListCoincident->count(); ++i )
      showGroup( ListCoincident->item( i ));
  else
    for ( int i = 0; i < selItems.count(); ++i )
      showGroup( selItems[ i ]);

  if ( setCurrentGroup() ) // edit a selected group
  {
    const SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];
    for ( CORBA::ULong iP = 0; iP < aGRP.length(); ++iP )
      new QListWidgetItem( getPartText( aGRP[ iP ]), ListEdit );
  }
  SMESH::RepaintCurrentView();
}

//=======================================================================
//function : onSelectAll
//purpose  : SLOT called when Select All is checked
//=======================================================================

void SMESHGUI_SewingDlg::onSelectAll(int isOn)
{
  if ( isOn )
    ListCoincident->selectAll();
  else
    ListCoincident->clearSelection();
}

//=======================================================================
//function : onSelectBorderPartFromGroup
//purpose  : SLOT called when selection of borders in an edited group changes
//=======================================================================

void SMESHGUI_SewingDlg::onSelectBorderPartFromGroup()
{
  if ( myBusy ) return;
  BorderEndLine[0]->setText("");
  BorderEndLine[1]->setText("");
  MoveBorderEndsButGrp->button( MOVE_LEFT_1  )->setEnabled( false );
  MoveBorderEndsButGrp->button( MOVE_RIGHT_1 )->setEnabled( false );
  MoveBorderEndsButGrp->button( MOVE_LEFT_2  )->setEnabled( false );
  MoveBorderEndsButGrp->button( MOVE_RIGHT_2 )->setEnabled( false );
  SwapBut->setEnabled( false );
  SetFirstButton->setEnabled( false );
  RemoveElemButton->setEnabled ( ListEdit->count() > 2 );

  if ( !setCurrentGroup() )
    return;

  if ( !myBorderDisplayers[ myCurGroupIndex ]) return;
  myBorderDisplayers[ myCurGroupIndex ]->Hide();

  QList<QListWidgetItem*> selItems = ListEdit->selectedItems();
  bool editPart = ( setCurrentPart() );
  for ( int i = 0; i < selItems.count(); ++i )
    myBorderDisplayers[ myCurGroupIndex ]->ShowPart( ListEdit->row( selItems[i] ), editPart );

  if ( selItems.isEmpty() )
    myBorderDisplayers[ myCurGroupIndex ]->ShowGroup( /*wholeBorders=*/ true );

  if ( editPart )
  {
    SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];
    SMESH::FreeBorderPart&   aPRT = aGRP[ myCurPartIndex ];
    SMESH::FreeBorder&       aBRD = myBorders->borders[ aPRT.border ];

    BorderEndLine[0]->setText( QString::number( aBRD.nodeIDs[ aPRT.node1 ]));
    BorderEndLine[1]->setText( QString::number( aBRD.nodeIDs[ aPRT.nodeLast ]));
    SwapBut->setEnabled( true );
    SetFirstButton->setEnabled( myCurPartIndex > 0 );

    int      size = (int) aBRD.nodeIDs.length();
    bool isClosed = ( aBRD.nodeIDs[0] == aBRD.nodeIDs[ size-1 ]);
    if ( !isClosed )
    {
      bool isFwd = ( Abs( aPRT.node2 - aPRT.node1 ) == 1 ) ? aPRT.node2 > aPRT.node1 : aPRT.node2 < aPRT.node1;
      int dn     = ( isFwd ? +1 : -1 ) * StepSpin->value();
      MoveBorderEndsButGrp->button( MOVE_LEFT_1  )->
        setEnabled( 0 <= aPRT.node1-dn && aPRT.node1-dn < size );
      MoveBorderEndsButGrp->button( MOVE_RIGHT_1 )->
        setEnabled( 0 <= aPRT.node1+dn && aPRT.node1+dn < size );
      MoveBorderEndsButGrp->button( MOVE_LEFT_2  )->
        setEnabled( 0 <= aPRT.nodeLast-dn && aPRT.nodeLast-dn < size );
      MoveBorderEndsButGrp->button( MOVE_RIGHT_2  )->
        setEnabled( 0 <= aPRT.nodeLast+dn && aPRT.nodeLast+dn < size );
    }
    else
    {
      MoveBorderEndsButGrp->button( MOVE_LEFT_1  )->setEnabled( true );
      MoveBorderEndsButGrp->button( MOVE_RIGHT_1 )->setEnabled( true );
      MoveBorderEndsButGrp->button( MOVE_LEFT_2  )->setEnabled( true );
      MoveBorderEndsButGrp->button( MOVE_RIGHT_2 )->setEnabled( true );
    }
  }
  SMESH::RepaintCurrentView();
}

//=======================================================================
//function : onGroupChange
//purpose  : Update after modification of a current group by the user
//=======================================================================

void SMESHGUI_SewingDlg::onGroupChange( bool partChange )
{
  ListCoincident->currentItem()->setText( getGroupText( myCurGroupIndex ));

  const SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];
  for ( int i = 0; i < ListEdit->count(); ++i )
    ListEdit->item( i )->setText( getPartText( aGRP[ i ]));

  if ( myBorderDisplayers[ myCurGroupIndex ])
    myBorderDisplayers[ myCurGroupIndex ]->Update();

  if ( partChange )
    onSelectBorderPartFromGroup();
}

//=======================================================================
//function : onSetFirstClicked
//purpose  : STOL called when |<< is clicked
//=======================================================================

void SMESHGUI_SewingDlg::onSetFirstClicked()
{
  if ( !setCurrentPart() || myCurPartIndex == 0 || ListEdit->count() == 0 )
    return;

  SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];

  SMESH::FreeBorderPart new1st = aGRP[ myCurPartIndex ];
  for ( ; myCurPartIndex > 0; --myCurPartIndex )
    aGRP[ myCurPartIndex ] = aGRP[ myCurPartIndex - 1 ];

  aGRP[ 0 ] = new1st;

  onGroupChange();

  myBusy = true;
  ListEdit->clearSelection();
  myBusy = false;
  ListEdit->setCurrentItem( ListEdit->item(0) );//ListEdit->item(0)->setSelected(true);
}

//=======================================================================
//function : onRemoveElemClicked
//purpose  : 
//=======================================================================

void SMESHGUI_SewingDlg::onRemoveElemClicked()
{
  if ( !setCurrentGroup() )
    return;

  SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];

  myBusy = true;
  QList<QListWidgetItem*> selItems = ListEdit->selectedItems();
  for ( int i = 0; i < selItems.count(); ++i )
  {
    int part = ListEdit->row( selItems[i] );
    for ( ; part + 1 < (int)aGRP.length(); ++part )
      aGRP[ part ] = aGRP[ part + 1 ];
    if ( aGRP.length() > 0 )
      aGRP.length( aGRP.length() - 1 );
    delete selItems[i];
  }
  myBusy = false;

  if ( aGRP.length() == 0 )
    onRemoveGroupClicked();
  else
    onGroupChange( /*partChange=*/true );
}

//=======================================================================
//function : onMoveBorderEnd
//purpose  : 
//=======================================================================

void SMESHGUI_SewingDlg::onMoveBorderEnd(int button)
{
  if ( !setCurrentPart() )
    return;

  SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];
  SMESH::FreeBorderPart&   aPRT = aGRP[ myCurPartIndex ];
  SMESH::FreeBorder&       aBRD = myBorders->borders[ aPRT.border ];
  int size = (int) aBRD.nodeIDs.length();

  bool isClosed = ( aBRD.nodeIDs[0] == aBRD.nodeIDs[ size-1 ]);
  if ( isClosed ) --size;

  bool isFwd = ( Abs( aPRT.node2 - aPRT.node1 ) == 1 ) ? aPRT.node2 > aPRT.node1 : aPRT.node2 < aPRT.node1;
  int dn     = ( isFwd ? +1 : -1 ) * StepSpin->value();
  if ( button == MOVE_LEFT_1 || button == MOVE_LEFT_2 )
    dn *= -1;

  switch ( button ) {
  case MOVE_LEFT_1:
  case MOVE_RIGHT_1:
    if (( isClosed ) ||
        ( 0 <= aPRT.node1+dn && aPRT.node1+dn < size ))
    {
      aPRT.node1 = ( aPRT.node1 + size + dn ) % size;
      aPRT.node2 = ( aPRT.node2 + size + dn ) % size;
      break;
    }
  case MOVE_LEFT_2:
  case MOVE_RIGHT_2:
    if (( isClosed ) ||
        ( 0 <= aPRT.nodeLast+dn && aPRT.nodeLast+dn < size ))
    {
      aPRT.nodeLast = ( aPRT.nodeLast + size + dn ) % size;
      break;
    }
  default:
    return; // impossible to move
  }
  
  onGroupChange( /*partChange=*/true );
}

//=======================================================================
//function : onSwapClicked
//purpose  : SLOT called when <-> is clicked
//=======================================================================

void SMESHGUI_SewingDlg::onSwapClicked()
{
  if ( !setCurrentPart() )
    return;

  SMESH::FreeBordersGroup& aGRP = myBorders->coincidentGroups[ myCurGroupIndex ];
  SMESH::FreeBorderPart&   aPRT = aGRP[ myCurPartIndex ];
  SMESH::FreeBorder&       aBRD = myBorders->borders[ aPRT.border ];
  int size = (int) aBRD.nodeIDs.length();

  bool isClosed = ( aBRD.nodeIDs[0] == aBRD.nodeIDs[ size-1 ]);
  if ( isClosed ) --size;

  bool isFwd = ( Abs( aPRT.node2 - aPRT.node1 ) == 1 ) ? aPRT.node2 > aPRT.node1 : aPRT.node2 < aPRT.node1;

  std::swap( aPRT.nodeLast, aPRT.node1 );

  aPRT.node2 = ( aPRT.node1 + ( isFwd ? -1 : +1 ) + size ) % size;

  onGroupChange( /*partChange=*/true );
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool SMESHGUI_SewingDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  bool aResult = false;

  if (IsValid())
  {
    bool toMerge          = CheckBoxMerge->isChecked();
    bool toCreatePolygons = CheckBoxPolygons->isChecked();
    bool toCreatePolyedrs = CheckBoxPolyedrs->isChecked();
    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      int aConstructorId = GetConstructorId();
      SMESH::SMESH_MeshEditor::Sew_Error anError = SMESH::SMESH_MeshEditor::SEW_OK;

      if (aConstructorId == 0)
      {
        if ( ModeButGrp->checkedId() == MODE_MANUAL )
        {
          anError = aMeshEditor->SewFreeBorders(LineEdit1->text().toLong(),
                                                LineEdit2->text().toLong(),
                                                LineEdit3->text().toLong(),
                                                LineEdit4->text().toLong(),
                                                LineEdit5->text().toLong(),
                                                LineEdit6->text().toLong(),
                                                toCreatePolygons,
                                                toCreatePolyedrs);
        }
        else
        {
          int nbCoincGroups = ListCoincident->count();
          if ( AutoSewCheck->isChecked() )
          {
            myBorders     = aMeshEditor->FindCoincidentFreeBorders( SpinBoxTolerance->GetValue() );
            nbCoincGroups = myBorders->coincidentGroups.length();
          }
          CORBA::Short nbSewed = aMeshEditor->SewCoincidentFreeBorders( myBorders.inout(),
                                                                        toCreatePolygons,
                                                                        toCreatePolyedrs);
          QString msg;
          if ( nbCoincGroups == 0 )
            msg = tr("NO_BORDERS_TO_SEW");
          else if ( nbSewed < nbCoincGroups )
            msg = tr("NOT_ALL_BORDERS_SEWED").arg( nbSewed ).arg( nbCoincGroups );
          else
            msg = tr("ALL_BORDERS_SEWED").arg( nbSewed );
          SUIT_MessageBox::information( this, tr("SMESH_INFORMATION"), msg );

          anError = SMESH::SMESH_MeshEditor::SEW_OK;
        }
      }
      else if (aConstructorId == 1)
      {
        anError = aMeshEditor->SewConformFreeBorders(LineEdit1->text().toLong(),
                                                     LineEdit2->text().toLong(),
                                                     LineEdit3->text().toLong(),
                                                     LineEdit4->text().toLong(),
                                                     LineEdit5->text().toLong());
      }
      else if (aConstructorId == 2)
      {
        anError = aMeshEditor->SewBorderToSide(LineEdit1->text().toLong(),
                                               LineEdit2->text().toLong(),
                                               LineEdit3->text().toLong(),
                                               LineEdit4->text().toLong(),
                                               LineEdit6->text().toLong(),
                                               toCreatePolygons,
                                               toCreatePolyedrs);
      }
      else if (aConstructorId == 3) {
        QStringList aListElementsId1 = LineEdit1->text().split(" ", QString::SkipEmptyParts);
        QStringList aListElementsId2 = LineEdit4->text().split(" ", QString::SkipEmptyParts);

        SMESH::long_array_var anElementsId1 = new SMESH::long_array;
        SMESH::long_array_var anElementsId2 = new SMESH::long_array;

        anElementsId1->length(aListElementsId1.count());
        anElementsId2->length(aListElementsId2.count());

        for (int i = 0; i < aListElementsId1.count(); i++)
          anElementsId1[i] = aListElementsId1[i].toInt();
        for (int i = 0; i < aListElementsId2.count(); i++)
          anElementsId2[i] = aListElementsId2[i].toInt();

        anError = aMeshEditor->SewSideElements(anElementsId1.inout(),
                                               anElementsId2.inout(),
                                               LineEdit2->text().toLong(),
                                               LineEdit5->text().toLong(),
                                               LineEdit3->text().toLong(),
                                               LineEdit6->text().toLong());
      }
      aResult = (anError == SMESH::SMESH_MeshEditor::SEW_OK);

      if (toMerge && aResult)
        aMeshEditor->MergeEqualElements();

      if (!aResult) {
        QString msg = tr(QString("ERROR_%1").arg(anError).toLatin1().data());
        SUIT_MessageBox::warning(this, tr("SMESH_WRN_WARNING"), msg);
      }
    }
    catch ( const SALOME::SALOME_Exception& S_ex )
    {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
      return false;
    }

    if (aResult) {

      if ( myActor )
      {
        Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
        SALOME_ListIO aList;
        aList.Append(anIO);
        mySelectionMgr->setSelectedObjects(aList, false);
        SMESH::UpdateView();
      }
      Init();
      ConstructorsClicked(GetConstructorId());

      SMESHGUI::Modified();
    }
  }

  return aResult;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::ClickOnOk()
{
  if (ClickOnApply())
    reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    ActivateThisDialog();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
  myActor = 0;

  for ( size_t i = 0; i < myBorderDisplayers.size(); ++i )
  {
    delete myBorderDisplayers[ i ];
    myBorderDisplayers[ i ] = 0;
  }
  myBorderDisplayers.clear();
}

//=================================================================================
// function : reject()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::reject()
{
  restoreDisplayMode();
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::ClickOnHelp()
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

//=======================================================================
//function : onTextChange
//purpose  :
//=======================================================================
void SMESHGUI_SewingDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if (myBusy) return;
  myBusy = true;

  if (send)
    myEditCurrentArgument = send;

  if      (send == LineEdit1)
    myOk1 = false;
  else if (send == LineEdit2)
    myOk2 = false;
  else if (send == LineEdit3)
    myOk3 = false;
  else if (send == LineEdit4)
    myOk4 = false;
  else if (send == LineEdit5)
    myOk5 = false;
  else if (send == LineEdit6)
    myOk6 = false;

  // hilight entered elements/nodes
  SMDS_Mesh* aMesh = 0;

  if (myActor)
    aMesh = myActor->GetObject()->GetMesh();
  else
    send->clear();

  if (aMesh) {
    TColStd_MapOfInteger newIndices;
    
    if (GetConstructorId() != 3 || (send != LineEdit1 && send != LineEdit4)) {
      SMESH::SetPointRepresentation(true);

      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(NodeSelection);

      const SMDS_MeshNode * n = aMesh->FindNode(theNewText.toInt());
      if (n) {
        newIndices.Add(n->GetID());
        mySelector->AddOrRemoveIndex(myActor->getIO(), newIndices, false);
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
          aViewWindow->highlight( myActor->getIO(), true, true );
        
        if      (send == LineEdit1)
          myOk1 = true;
        else if (send == LineEdit2)
          myOk2 = true;
        else if (send == LineEdit3)
          myOk3 = true;
        else if (send == LineEdit4)
          myOk4 = true;
        else if (send == LineEdit5)
          myOk5 = true;
        else if (send == LineEdit6)
          myOk6 = true;
      }
    } else {
      SMESH::SetPointRepresentation(false);

      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->SetSelectionMode(CellSelection);

      QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

      bool isEvenOneExists = false;

      for (int i = 0; i < aListId.count(); i++) {
        const SMDS_MeshElement * e = aMesh->FindElement(aListId[ i ].toInt());
        if (e) 
          newIndices.Add(e->GetID());
        
          if (!isEvenOneExists)
            isEvenOneExists = true;
      }
      
      mySelector->AddOrRemoveIndex(myActor->getIO(), newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight( myActor->getIO(), true, true );
      
      if (isEvenOneExists) {
        if (send == LineEdit1)
          myOk1 = true;
        else if(send == LineEdit4)
          myOk4 = true;
      } else {
        send->clear();
      }
    }
  }

  UpdateButtons();

  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_SewingDlg::SelectionIntoArgument (bool isSelectionChanged)
{
  if (myBusy) return;

  // clear
  restoreDisplayMode();
  if (isSelectionChanged)
    myActor = 0;

  QString aString = "";

  myBusy = true;
  myEditCurrentArgument->setText(aString);
  ListCoincident->clear();
  ListEdit->clear();
  myBusy = false;

  onSelectGroup(); // erase preview

  if (!GroupButtons->isEnabled()) // inactive
    return;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);
  DetectButton->setEnabled(false);

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh  = SMESH::GetMeshByIO(IO);
  myActor = SMESH::FindActorByEntry(aList.First()->getEntry());
  if (myMesh->_is_nil())
    return;

  CheckBoxPolyedrs->setEnabled( myMesh->NbVolumes() > 0 );

  if ( myEditCurrentArgument == LineEditMesh )
  {
    if ( _PTR(SObject) meshSO = SMESH::FindSObject( myMesh ))
      LineEditMesh->setText( meshSO->GetName().c_str() );
    ListCoincident->clear();
    if ( AutoSewCheck->isChecked() )
    {
      buttonOk->setEnabled(true);
      buttonApply->setEnabled(true);
    }
    DetectButton->setEnabled( myMesh->NbFaces() > 0 );
    setDisplayMode();
    return;
  }

  if (!myActor)
    return;

  // get selected elements/nodes
  int aNbUnits = 0;
  if (( GetConstructorId() != 3 ) ||
      ( myEditCurrentArgument != LineEdit1 && myEditCurrentArgument != LineEdit4))
  {
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    if (aNbUnits != 1)
      return;
  }
  else {
    aNbUnits = SMESH::GetNameOfSelectedElements(mySelector, IO, aString);
    if (aNbUnits < 1)
      return;
  }

  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;

  // OK
  if (myEditCurrentArgument == LineEdit1)
    myOk1 = true;
  else if (myEditCurrentArgument == LineEdit2)
    myOk2 = true;
  else if (myEditCurrentArgument == LineEdit3)
    myOk3 = true;
  else if (myEditCurrentArgument == LineEdit4)
    myOk4 = true;
  else if (myEditCurrentArgument == LineEdit5)
    myOk5 = true;
  else if (myEditCurrentArgument == LineEdit6)
    myOk6 = true;

  UpdateButtons();
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  disconnect(mySelectionMgr, 0, this, 0);
  mySelectionMgr->clearSelected();

  if (send == SelectButton1) {
    myEditCurrentArgument = LineEdit1;
    myOk1 = false;
  }
  else if (send == SelectButton2) {
    myEditCurrentArgument = LineEdit2;
    myOk2 = false;
  }
  else if (send == SelectButton3) {
    myEditCurrentArgument = LineEdit3;
    myOk3 = false;
  }
  else if (send == SelectButton4) {
    myEditCurrentArgument = LineEdit4;
    myOk4 = false;
  }
  else if (send == SelectButton5) {
    myEditCurrentArgument = LineEdit5;
    myOk5 = false;
  }
  else if (send == SelectButton6) {
    myEditCurrentArgument = LineEdit6;
    myOk6 = false;
  }

  if (GetConstructorId() != 3 || (send != SelectButton1 && send != SelectButton4)) {
    SMESH::SetPointRepresentation(true);

    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);

  } else {
    SMESH::SetPointRepresentation(false);
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(CellSelection);
  }

  myEditCurrentArgument->setFocus();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  SelectionIntoArgument(false);
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::DeactivateActiveDialog()
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
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  ConstructorsBox->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  ConstructorsClicked(GetConstructorId());
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::enterEvent (QEvent* e)
{
  if (!ConstructorsBox->isEnabled()) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_SewingDlg::GetConstructorId()
{
  return GroupConstructors->checkedId();
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
bool SMESHGUI_SewingDlg::IsValid()
{
  if ( myMesh->_is_nil() )
    return false;

  if ( GetConstructorId() == 0 && ModeButGrp->checkedId() == MODE_AUTO )
  {
    if ( AutoSewCheck->isChecked() )
      return true;

    int nbGroups = 0;
    if ( haveBorders() )
      for ( int i = 0; i < ListCoincident->count(); ++i )
      {
        int groupIndex = ListCoincident->item(i)->data( GROUP_INDEX ).toInt();
        nbGroups += ( !getGroupText( groupIndex ).isEmpty() );
      }
    return nbGroups > 0;
  }
  return (myOk1 && myOk2 && myOk3 && myOk4 && myOk5 && myOk6);
}

//=======================================================================
//function : UpdateButtons
//purpose  : activate [Apply] buttons
//=======================================================================

void SMESHGUI_SewingDlg::UpdateButtons()
{
  bool ok = IsValid();
  buttonOk->setEnabled( ok );
  buttonApply->setEnabled( ok );
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}

SMESHGUI_SewingDlg::
BorderGroupDisplayer::BorderGroupDisplayer( const SMESH::CoincidentFreeBorders& borders,
                                            int                                 groupIndex,
                                            QColor                              color,
                                            SMESH::SMESH_Mesh_ptr               mesh):
  myBorders   ( borders.borders ),
  myGroup     ( borders.coincidentGroups[ groupIndex ]),
  myColor     ( color ),
  myMesh      ( mesh ),
  myViewWindow( SMESH::GetCurrentVtkView() ),
  myIdPreview ( myViewWindow )
{
  Update();
}

SMESHGUI_SewingDlg::BorderGroupDisplayer::~BorderGroupDisplayer()
{
  for ( size_t i = 0; i < myPartActors.size(); ++i )
  {
    if ( myPartActors[ i ]) {
      myViewWindow->RemoveActor( myPartActors[i] );
      myPartActors[i]->Delete();
    }
  }
  myIdPreview.SetPointsLabeled(false);
}

void SMESHGUI_SewingDlg::BorderGroupDisplayer::Hide()
{
  for ( size_t i = 0; i < myPartActors.size(); ++i )
    if ( myPartActors[ i ])
      myPartActors[ i ]->SetVisibility(false);

  myIdPreview.SetPointsLabeled(false);
}

void SMESHGUI_SewingDlg::BorderGroupDisplayer::ShowGroup( bool wholeBorders )
{
  std::vector<int> ids;
  std::list<gp_XYZ> coords;
  for ( size_t i = 0; i < myPartActors.size(); ++i )
    if ( myPartActors[ i ])
    {
      myPartActors[ i ]->SetPointRepresentation( wholeBorders );
      myPartActors[ i ]->SetVisibility( true );
      if ( wholeBorders )
        getPartEnds( i, ids, coords );
    }
  if ( wholeBorders )
    myIdPreview.SetElemsData( ids, coords );
  myIdPreview.SetPointsLabeled( wholeBorders, true );
}

void SMESHGUI_SewingDlg::BorderGroupDisplayer::ShowPart( int partIndex, bool toEdit )
{
  if ( partIndex < (int) myPartActors.size() )
  {
    myPartActors[partIndex]->SetVisibility(true);
    myPartActors[partIndex]->SetPointRepresentation(toEdit);

    if ( toEdit )
    {
      std::vector<int> ids;
      std::list<gp_XYZ> coords;
      getPartEnds( partIndex, ids, coords );

      myIdPreview.SetElemsData( ids, coords );
      myIdPreview.SetPointsLabeled( true, /*show=*/true );
    }
  }
}

void SMESHGUI_SewingDlg::BorderGroupDisplayer::getPartEnds( int                partIndex,
                                                            std::vector<int> & ids,
                                                            std::list<gp_XYZ>& coords)
{
  if ( partIndex >= (int)myGroup.length() ) return;
  const SMESH::FreeBorderPart& aPART = myGroup  [ partIndex ];
  const SMESH::FreeBorder&      aBRD = myBorders[ aPART.border ];

  ids.push_back( aBRD.nodeIDs[ aPART.node1 ]);
  ids.push_back( aBRD.nodeIDs[ aPART.nodeLast ]);
  if ( aPART.node1 == aPART.nodeLast )
    ids.push_back( aBRD.nodeIDs[ aPART.node2 ]);

  SMDS_Mesh* mesh = myPartActors[ partIndex ]->GetObject()->GetMesh();

  coords.push_back( SMESH_TNodeXYZ( mesh->FindNode( aPART.node1+1 )));
  coords.push_back( SMESH_TNodeXYZ( mesh->FindNode( aPART.nodeLast+1 )));
  if ( aPART.node1 == aPART.nodeLast )
    coords.push_back( SMESH_TNodeXYZ( mesh->FindNode( aPART.node2+1 )));
}

void SMESHGUI_SewingDlg::BorderGroupDisplayer::Update()
{
  Hide();
  myPartActors.resize( myGroup.length(), 0 );

  for ( size_t i = 0; i < myPartActors.size(); ++i )
  {
    TVisualObjPtr obj;
    if ( myPartActors[ i ])
      obj = myPartActors[ i ]->GetObject();
    else
      obj = TVisualObjPtr( new SMESHGUI_PreVisualObj() );
    SMDS_Mesh* mesh = obj->GetMesh();
    mesh->Clear();

    // add nodes
    const SMESH::FreeBorderPart& aPRT = myGroup[ i ];
    const SMESH::FreeBorder&     aBRD = myBorders[ aPRT.border ];
    for ( CORBA::ULong iN = 0; iN < aBRD.nodeIDs.length(); ++iN )
    {
      SMESH::double_array_var xyz = myMesh->GetNodeXYZ( aBRD.nodeIDs[ iN ]);
      if ( xyz->length() == 3 )
        mesh->AddNode( xyz[0], xyz[1], xyz[2] );
    }

    // add edges
    bool isFwd = ( Abs( aPRT.node2 - aPRT.node1 ) == 1 ) ? aPRT.node2 > aPRT.node1 : aPRT.node2 < aPRT.node1;
    int dn     = isFwd ? +1 : -1;
    int size   = (int) aBRD.nodeIDs.length();
    int n2, n1 = aPRT.node1;
    for ( n2 = n1 + dn; ( n2 >= 0 && n2 < size ); n2 += dn )
    {
      mesh->AddEdgeWithID( n1+1, n2+1, mesh->NbEdges() + 1 );
      n1 = n2;
      if ( n2 == aPRT.nodeLast )
        break;
    }
    if ( n2 % size != aPRT.nodeLast )
    {
      if ( n2 < 0 ) n1 = size;
      else          n1 = 0;
      for ( n2 = n1 + dn; ( n2 >= 0 && n2 < size ); n2 += dn )
      {
        mesh->AddEdgeWithID( n1+1, n2+1, mesh->NbEdges() + 1 );
        n1 = n2;
        if ( n2 == aPRT.nodeLast )
          break;
      }
    }

    if ( !myPartActors[ i ]) // TVisualObj must be filled before actor creation
    {
      myPartActors[ i ] = SMESH_Actor::New( obj, "", "", 1 );
      myPartActors[ i ]->SetEdgeColor( myColor.redF(), myColor.greenF(), myColor.blueF() );
      myPartActors[ i ]->SetLineWidth( 3 * SMESH::GetFloat("SMESH:element_width",1));
      myPartActors[ i ]->SetNodeColor( myColor.redF(), myColor.greenF(), myColor.blueF() );
      myPartActors[ i ]->SetMarkerStd( VTK::MT_POINT, 13 );
      myPartActors[ i ]->SetPickable ( false );
      myViewWindow->AddActor( myPartActors[ i ]);
    }
  }
}

