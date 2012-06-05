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
// File   : SMESHGUI_SewingDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SewingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"

#include <SMESH_Actor.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

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

  // layout
  GroupArgumentsLayout->addWidget(SubGroup1);
  GroupArgumentsLayout->addWidget(SubGroup2);
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

  /* Initialisations */
  RadioButton1->setChecked(true);

  LineEdit2->setValidator(new SMESHGUI_IdValidator(this, 1));
  LineEdit3->setValidator(new SMESHGUI_IdValidator(this, 1));
  LineEdit5->setValidator(new SMESHGUI_IdValidator(this, 1));
  LineEdit6->setValidator(new SMESHGUI_IdValidator(this, 1));

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  myHelpFileName = "sewing_meshes_page.html";

  Init();

  /* signals and slots connections */
  connect(buttonOk,     SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
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
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));

  connect(LineEdit1, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit2, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit3, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit4, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit5, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));
  connect(LineEdit6, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));

  ConstructorsClicked(0);
}

//=================================================================================
// function : ~SMESHGUI_SewingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_SewingDlg::~SMESHGUI_SewingDlg()
{
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::Init()
{
  myBusy = false;

  myEditCurrentArgument = LineEdit1;
  LineEdit1->setFocus();
  myActor = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();
  CheckBoxMerge->setChecked(false);
  CheckBoxPolygons->setChecked(false);
  CheckBoxPolyedrs->setChecked(false);
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

      SMESH::SetPointRepresentation(false);

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

    SMESH::SetPointRepresentation(true);

    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(NodeSelection);
  }

  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  mySelectionMgr->setSelectedObjects( io );

  QApplication::instance()->processEvents();
  updateGeometry();
  resize(100,100);
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

  if (IsValid()) {
    bool toMerge = CheckBoxMerge->isChecked();
    bool toCreatePolygons = CheckBoxPolygons->isChecked();
    bool toCreatePolyedrs = CheckBoxPolyedrs->isChecked();

    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      int aConstructorId = GetConstructorId();
      SMESH::SMESH_MeshEditor::Sew_Error anError;

      if (aConstructorId == 0)
        anError = aMeshEditor->SewFreeBorders(LineEdit1->text().toLong(),
                                              LineEdit2->text().toLong(),
                                              LineEdit3->text().toLong(),
                                              LineEdit4->text().toLong(),
                                              LineEdit5->text().toLong(),
                                              LineEdit6->text().toLong(),
                                              toCreatePolygons,
                                              toCreatePolyedrs);
      else if (aConstructorId == 1)
        anError = aMeshEditor->SewConformFreeBorders(LineEdit1->text().toLong(),
                                                     LineEdit2->text().toLong(),
                                                     LineEdit3->text().toLong(),
                                                     LineEdit4->text().toLong(),
                                                     LineEdit5->text().toLong());
      else if (aConstructorId == 2)
        anError = aMeshEditor->SewBorderToSide(LineEdit1->text().toLong(),
                                               LineEdit2->text().toLong(),
                                               LineEdit3->text().toLong(),
                                               LineEdit4->text().toLong(),
                                               LineEdit6->text().toLong(),
                                               toCreatePolygons,
                                               toCreatePolyedrs);
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
    } catch (...) {
    }

    if (aResult) {
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();

      SALOME_ListIO aList;
      aList.Append(anIO);
      mySelectionMgr->setSelectedObjects(aList, false);
      SMESH::UpdateView();

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
    ClickOnCancel();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::ClickOnCancel()
{
  //mySelectionMgr->clearSelected();
  SMESH::SetPointRepresentation(false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
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

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

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

  if (IsValid()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }

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
  if (isSelectionChanged)
    myActor = 0;

  QString aString = "";

  myBusy = true;
  myEditCurrentArgument->setText(aString);
  myBusy = false;

  if (!GroupButtons->isEnabled()) // inactive
    return;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList,SVTK_Viewer::Type());

  int nbSel = aList.Extent();
  if (nbSel != 1)
    return;

  Handle(SALOME_InteractiveObject) IO = aList.First();
  myMesh = SMESH::GetMeshByIO(IO); //@ SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO);
  myActor = SMESH::FindActorByEntry(aList.First()->getEntry());

  if (myMesh->_is_nil() || !myActor)
    return;

  // get selected elements/nodes
  int aNbUnits = 0;

  if (GetConstructorId() != 3 ||
      (myEditCurrentArgument != LineEdit1 && myEditCurrentArgument != LineEdit4)) {
    aNbUnits = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    if (aNbUnits != 1)
      return;
  } else {
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

  if (IsValid()) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
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
  if (!ConstructorsBox->isEnabled())
    ActivateThisDialog();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SewingDlg::closeEvent (QCloseEvent*)
{
  /* same than click on cancel button */
  ClickOnCancel();
}

//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================
void SMESHGUI_SewingDlg::hideEvent (QHideEvent*)
{
  if (!isMinimized())
    ClickOnCancel();
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
  return (myOk1 && myOk2 && myOk3 && myOk4 && myOk5 && myOk6);
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
