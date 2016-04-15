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

// File   : SMESHGUI_MultiEditDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes

#include "SMESHGUI_MultiEditDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Filter.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESH_Actor.h"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_TypeFilter.hxx"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>

#include <LightApp_SelectionMgr.h>
#include <LightApp_Application.h>
#include <SALOME_ListIO.hxx>
#include <SalomeApp_Tools.h>

#include <SVTK_Selector.h>
#include <SVTK_ViewWindow.h>
#include <VTKViewer_CellLocationsArray.h>

// OCCT includes
#include <Bnd_B3d.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <gp_Ax1.hxx>

// VTK includes
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

// Qt includes
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

#define SPACING 6
#define MARGIN  11

/*!
 *  Class       : SMESHGUI_MultiEditDlg
 *  Description : Description : Inversion of the diagonal of a pseudo-quadrangle formed by
 *                2 neighboring triangles with 1 common edge
 */

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::SMESHGUI_MultiEditDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_MultiEditDlg
::SMESHGUI_MultiEditDlg(SMESHGUI*  theModule,
                        const int  theMode,
                        const bool the3d2d,
                        bool       theDoInit):
  SMESHGUI_PreviewDlg(theModule),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
  mySMESHGUI(theModule)
{
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);

  myFilterDlg = 0;
  myEntityType = 0;

  myFilterType = theMode;
  QVBoxLayout* aDlgLay = new QVBoxLayout(this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  QWidget* aMainFrame = createMainFrame  (this, the3d2d);
  QWidget* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  if ( theDoInit )
    Init();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_MultiEditDlg::createMainFrame (QWidget* theParent, const bool the3d2d)
{
  QWidget* aMainGrp = new QWidget(theParent);
  QVBoxLayout* aMainGrpLayout = new QVBoxLayout(aMainGrp);
  aMainGrpLayout->setMargin(0);
  aMainGrpLayout->setSpacing(SPACING);

  QPixmap aPix (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  // "Selected cells" group
  mySelGrp = new QGroupBox(aMainGrp);
  
  QGridLayout* mySelGrpLayout = new QGridLayout(mySelGrp);
  mySelGrpLayout->setMargin(MARGIN);
  mySelGrpLayout->setSpacing(SPACING);

  myEntityTypeGrp = 0;
  if (the3d2d) {
    QGroupBox* aEntityTypeGrp = new QGroupBox(tr("SMESH_ELEMENTS_TYPE"), mySelGrp);
    myEntityTypeGrp = new QButtonGroup(mySelGrp);
    QHBoxLayout* aEntityLayout = new QHBoxLayout(aEntityTypeGrp);
    aEntityLayout->setMargin(MARGIN);
    aEntityLayout->setSpacing(SPACING);

    QRadioButton* aFaceRb = new QRadioButton(tr("SMESH_FACE"), aEntityTypeGrp);
    QRadioButton* aVolumeRb = new QRadioButton(tr("SMESH_VOLUME"), aEntityTypeGrp);


    aEntityLayout->addWidget(aFaceRb);
    aEntityLayout->addWidget(aVolumeRb);

    myEntityTypeGrp->addButton(aFaceRb, 0);
    myEntityTypeGrp->addButton(aVolumeRb, 1);
    aFaceRb->setChecked(true);
    myEntityType = myEntityTypeGrp->checkedId();

    mySelGrpLayout->addWidget(aEntityTypeGrp, 0, 0, 1, 2);
  }

  myListBox = new QListWidget(mySelGrp);
  myListBox->setSelectionMode(QListWidget::ExtendedSelection);
  myListBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  myListBox->installEventFilter(this);

  myFilterBtn = new QPushButton(tr("FILTER"),    mySelGrp);
  myAddBtn    = new QPushButton(tr("ADD"),       mySelGrp);
  myRemoveBtn = new QPushButton(tr("REMOVE"),    mySelGrp);
  mySortBtn   = new QPushButton(tr("SORT_LIST"), mySelGrp);

  int row = mySelGrpLayout->rowCount();
  mySelGrpLayout->addWidget(myListBox,   row,   0, 6, 1);
  mySelGrpLayout->addWidget(myFilterBtn, row,   1);
  mySelGrpLayout->addWidget(myAddBtn,    row+2, 1);
  mySelGrpLayout->addWidget(myRemoveBtn, row+3, 1);
  mySelGrpLayout->addWidget(mySortBtn,   row+5, 1);
  mySelGrpLayout->setRowMinimumHeight(row+1, 10);
  mySelGrpLayout->setRowMinimumHeight(row+4, 10);
  mySelGrpLayout->setRowStretch(row+1, 5);
  mySelGrpLayout->setRowStretch(row+4, 5);

  myToAllChk = new QCheckBox(tr("TO_ALL"), mySelGrp);
  mySelGrpLayout->addWidget(myToAllChk, mySelGrpLayout->rowCount(), 0, 
                            1, mySelGrpLayout->columnCount());

  // Split/Join criterion group
  myCriterionGrp = new QGroupBox(tr("SPLIT_JOIN_CRITERION"), aMainGrp);
  QVBoxLayout* aCriterionLayout = new QVBoxLayout(myCriterionGrp);
  aCriterionLayout->setMargin(MARGIN);
  aCriterionLayout->setSpacing(SPACING);
  
  myChoiceWidget = new QWidget(myCriterionGrp);
  myGroupChoice = new QButtonGroup(myChoiceWidget);
  QVBoxLayout* aGroupChoiceLayout = new QVBoxLayout(myChoiceWidget);
  aGroupChoiceLayout->setMargin(0);
  aGroupChoiceLayout->setSpacing(SPACING);

  QRadioButton* aDiag13RB  = new QRadioButton(tr("USE_DIAGONAL_1_3"), myChoiceWidget);
  QRadioButton* aDiag24RB  = new QRadioButton(tr("USE_DIAGONAL_2_4"), myChoiceWidget);
  QRadioButton* aNumFuncRB = new QRadioButton(tr("USE_NUMERIC_FUNC"), myChoiceWidget);

  aGroupChoiceLayout->addWidget(aDiag13RB);
  aGroupChoiceLayout->addWidget(aDiag24RB);
  aGroupChoiceLayout->addWidget(aNumFuncRB);
  myGroupChoice->addButton(aDiag13RB,  0);
  myGroupChoice->addButton(aDiag24RB,  1);
  myGroupChoice->addButton(aNumFuncRB, 2);
  aDiag13RB->setChecked(true);

  myComboBoxFunctor = new QComboBox(myCriterionGrp);
  myComboBoxFunctor->addItem(tr("ASPECTRATIO_ELEMENTS"));
  myComboBoxFunctor->addItem(tr("MINIMUMANGLE_ELEMENTS"));
  myComboBoxFunctor->addItem(tr("SKEW_ELEMENTS"));
  myComboBoxFunctor->setCurrentIndex(0);

  aCriterionLayout->addWidget(myChoiceWidget);
  aCriterionLayout->addWidget(myComboBoxFunctor);

  myCriterionGrp->hide();
  myChoiceWidget->hide();
  myComboBoxFunctor->setEnabled(false);

  // "Select from" group
  QGroupBox* aGrp = new QGroupBox(tr("SELECT_FROM"), aMainGrp);
  QGridLayout* aGrpLayout = new QGridLayout(aGrp);
  aGrpLayout->setMargin(MARGIN);
  aGrpLayout->setSpacing(SPACING);

  mySubmeshChk = new QCheckBox(tr("SMESH_SUBMESH"), aGrp);
  mySubmeshBtn = new QPushButton(aGrp);
  mySubmesh = new QLineEdit(aGrp);
  mySubmesh->setReadOnly(true);
  mySubmeshBtn->setIcon(aPix);

  myGroupChk = new QCheckBox(tr("SMESH_GROUP"), aGrp);
  myGroupBtn = new QPushButton(aGrp);
  myGroup = new QLineEdit(aGrp);
  myGroup->setReadOnly(true);
  myGroupBtn->setIcon(aPix);

  aGrpLayout->addWidget(mySubmeshChk,0,0);
  aGrpLayout->addWidget(mySubmeshBtn,0,1);
  aGrpLayout->addWidget(mySubmesh,0,2);
  aGrpLayout->addWidget(myGroupChk,1,0);
  aGrpLayout->addWidget(myGroupBtn,1,1);
  aGrpLayout->addWidget(myGroup,1,2);
  
  aMainGrpLayout->addWidget(mySelGrp);
  aMainGrpLayout->addWidget(myCriterionGrp);
  aMainGrpLayout->addWidget(aGrp);

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QWidget* SMESHGUI_MultiEditDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aFrame = new QGroupBox(theParent);

  myOkBtn     = new QPushButton (tr("SMESH_BUT_APPLY_AND_CLOSE"), aFrame);
  myApplyBtn  = new QPushButton (tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn  = new QPushButton (tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn   = new QPushButton (tr("SMESH_BUT_HELP"),  aFrame);

  QHBoxLayout* aLay = new QHBoxLayout (aFrame);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addSpacing(10);
  aLay->addWidget(myApplyBtn);
  aLay->addSpacing(10);
  aLay->addStretch();
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_MultiEditDlg::isValid (const bool /*theMess*/)
{
  return (!myMesh->_is_nil() &&
          (myListBox->count() > 0 || (myToAllChk->isChecked() && nbElemsInMesh() > 0)));
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::~SMESHGUI_MultiEditDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_MultiEditDlg::~SMESHGUI_MultiEditDlg()
{
  if (myFilterDlg != 0)
  {
    myFilterDlg->setParent(0);
    delete myFilterDlg;
  }
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::eventFilter
// Purpose : event filter
//=======================================================================
bool SMESHGUI_MultiEditDlg::eventFilter (QObject* object, QEvent* event)
{
  if (object == myListBox && event->type() == QEvent::KeyPress) {
    QKeyEvent* ke = (QKeyEvent*)event;
    if (ke->key() == Qt::Key_Delete)
      onRemoveBtn();
  }
  return QDialog::eventFilter(object, event);
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::getNumericalFunctor
// Purpose :
//=======================================================================
SMESH::NumericalFunctor_ptr SMESHGUI_MultiEditDlg::getNumericalFunctor()
{
  SMESH::NumericalFunctor_var aNF = SMESH::NumericalFunctor::_nil();

  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
  if (aFilterMgr->_is_nil())
    return aNF._retn();

  if (myComboBoxFunctor->currentText() == tr("ASPECTRATIO_ELEMENTS"))
    aNF = aFilterMgr->CreateAspectRatio();
  else if (myComboBoxFunctor->currentText() == tr("WARP_ELEMENTS"))
    aNF = aFilterMgr->CreateWarping();
  else if (myComboBoxFunctor->currentText() == tr("MINIMUMANGLE_ELEMENTS"))
    aNF = aFilterMgr->CreateMinimumAngle();
  else if (myComboBoxFunctor->currentText() == tr("TAPER_ELEMENTS"))
    aNF = aFilterMgr->CreateTaper();
  else if (myComboBoxFunctor->currentText() == tr("SKEW_ELEMENTS"))
    aNF = aFilterMgr->CreateSkew();
  else if (myComboBoxFunctor->currentText() == tr("AREA_ELEMENTS"))
    aNF = aFilterMgr->CreateArea();
  else if (myComboBoxFunctor->currentText() == tr("LENGTH2D_EDGES"))
    aNF = aFilterMgr->CreateLength2D();
  else if (myComboBoxFunctor->currentText() == tr("MULTI2D_BORDERS"))
    aNF = aFilterMgr->CreateMultiConnection2D();
  else if (myComboBoxFunctor->currentText() == tr("MIN_DIAG_ELEMENTS"))
    aNF = aFilterMgr->CreateMaxElementLength2D();
  else;

  return aNF._retn();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myListBox->clear();
  myIds.Clear();
  myBusy = false;
  myActor = 0;
  emit ListContensChanged();

  // main buttons
  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(reject()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), SLOT( onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()), SLOT( onCloseView()));

  // dialog controls
  connect(myFilterBtn, SIGNAL(clicked()), SLOT(onFilterBtn()  ));
  connect(myAddBtn,    SIGNAL(clicked()), SLOT(onAddBtn()     ));
  connect(myRemoveBtn, SIGNAL(clicked()), SLOT(onRemoveBtn()  ));
  connect(mySortBtn,   SIGNAL(clicked()), SLOT(onSortListBtn()));

  connect(mySubmeshChk, SIGNAL(stateChanged(int)), SLOT(onSubmeshChk()));
  connect(myGroupChk,   SIGNAL(stateChanged(int)), SLOT(onGroupChk()  ));
  connect(myToAllChk,   SIGNAL(stateChanged(int)), SLOT(onToAllChk()  ));

  if (myEntityTypeGrp)
    connect(myEntityTypeGrp, SIGNAL(buttonClicked(int)), SLOT(on3d2dChanged(int)));

  connect(myListBox, SIGNAL(itemSelectionChanged()), SLOT(onListSelectionChanged()));

  onSelectionDone();

  // set selection mode
  setSelectionMode();
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::onOk()
{
  if (onApply())
    reject();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::getIds
// Purpose : Retrive identifiers from list box or the whole object
//=======================================================================

SMESH::long_array_var SMESHGUI_MultiEditDlg::getIds(SMESH::SMESH_IDSource_var& obj)
{
  SMESH::long_array_var anIds = new SMESH::long_array;

  if (myToAllChk->isChecked())
  {
    myIds.Clear();
    obj = SMESH::SMESH_IDSource::_narrow( myMesh );
  }

  anIds->length(myIds.Extent());
  TColStd_MapIteratorOfMapOfInteger anIter(myIds);
  for (int i = 0; anIter.More(); anIter.Next() )
  {
    anIds[ i++ ] = anIter.Key();
  }
  return anIds._retn();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::reject
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::reject()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();

  SMESH::RemoveFilters();
  SMESH::SetPickable();

  //mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_MultiEditDlg::onOpenView()
{
  if(!mySelector) {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_MultiEditDlg::onCloseView()
{
  onDeactivate();
  mySelector = 0;
}


//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MultiEditDlg::onHelp()
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
// name    : SMESHGUI_MultiEditDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_MultiEditDlg::onSelectionDone()
{
  if (myBusy || !isEnabled()) return;
  myBusy = true;

  const SALOME_ListIO& aList = mySelector->StoredIObjects();

  int nbSel = aList.Extent();
  myListBox->clearSelection();

  if (mySubmeshChk->isChecked() || myGroupChk->isChecked()) {
    QLineEdit* aNameEdit = mySubmeshChk->isChecked() ? mySubmesh : myGroup;
    if (nbSel == 1) {
      Handle(SALOME_InteractiveObject) anIO = aList.First();
      QString aName = "";
      SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aName);
      anIO.IsNull() ? aNameEdit->clear() : aNameEdit->setText(aName);

      if (mySubmeshChk->isChecked()) {
        SMESH::SMESH_subMesh_var aSubMesh =
          SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIO);
        if (!aSubMesh->_is_nil())
          myMesh = aSubMesh->GetFather();
      } else {
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIO);
        if (!aGroup->_is_nil())
          myMesh = aGroup->GetMesh();
      }
    } else if (nbSel > 1) {
      QString aStr = mySubmeshChk->isChecked() ?
        tr("SMESH_SUBMESH_SELECTED") : tr("SMESH_GROUP_SELECTED");
      aNameEdit->setText(aStr.arg(nbSel));
    } else {
      aNameEdit->clear();
    }
  } else if (nbSel > 0) {
    QString aListStr = "";
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    int aNbItems = SMESH::GetNameOfSelectedElements(mySelector,anIO,aListStr);
    if (aNbItems > 0) {
      QStringList anElements = aListStr.split(" ", QString::SkipEmptyParts);
      for (QStringList::iterator it = anElements.begin(); it != anElements.end(); ++it) {
        QList<QListWidgetItem*> items = myListBox->findItems(*it, Qt::MatchExactly);
        QListWidgetItem* anItem;
        foreach(anItem, items)
          anItem->setSelected(true);
      }
    }
    SMESH::SMESH_Mesh_var aSelMesh = SMESH::GetMeshByIO(anIO);
    if (!aSelMesh->_is_nil())
      myMesh = aSelMesh;
  }

  if (nbSel > 0) {
    myActor = SMESH::FindActorByEntry(aList.First()->getEntry());
    if (!myActor)
      myActor = SMESH::FindActorByObject(myMesh);
    SVTK_Selector* aSelector = SMESH::GetSelector();
    Handle(VTKViewer_Filter) aFilter = aSelector->GetFilter(myFilterType);
    if (!aFilter.IsNull())
      aFilter->SetActor(myActor);
  }
  myBusy = false;

  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_MultiEditDlg::onDeactivate()
{
  setEnabled(false);
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_MultiEditDlg::enterEvent (QEvent*)
{
  if (!isEnabled()) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
    setSelectionMode();
  }
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onFilterBtn
// Purpose : SLOT. Called when "Filter" button pressed.
//           Start "Selection filters" dialog
//=======================================================================
void SMESHGUI_MultiEditDlg::onFilterBtn()
{
  if (myFilterDlg == 0) {
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, entityType() ? SMESH::VOLUME : SMESH::FACE);
    connect(myFilterDlg, SIGNAL(Accepted()), SLOT(onFilterAccepted()));
  } else {
    myFilterDlg->Init(entityType() ? SMESH::VOLUME : SMESH::FACE);
  }

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh(myMesh);
  myFilterDlg->SetSourceWg(myListBox, false);

  myFilterDlg->show();
}

//=======================================================================
// name    : onFilterAccepted()
// Purpose : SLOT. Called when Filter dlg closed with OK button.
//            Uncheck "Select submesh" and "Select group" checkboxes
//=======================================================================
void SMESHGUI_MultiEditDlg::onFilterAccepted()
{
  myIds.Clear();
  for (int i = 0, n = myListBox->count(); i < n; i++)
    myIds.Add(myListBox->item(i)->text().toInt());

  emit ListContensChanged();

  if (mySubmeshChk->isChecked() || myGroupChk->isChecked()) {
    mySubmeshChk->blockSignals(true);
    myGroupChk->blockSignals(true);
    mySubmeshChk->setChecked(false);
    myGroupChk->setChecked(false);
    mySubmeshChk->blockSignals(false);
    myGroupChk->blockSignals(false);
  }
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::isIdValid
// Purpose : Verify whether Id of element satisfies to filters from viewer
//=======================================================================
bool SMESHGUI_MultiEditDlg::isIdValid (const int theId) const
{
  if ( !myActor )
    return true; // filter can't work w/o actor

  SVTK_Selector* aSelector = SMESH::GetSelector();
  Handle(SMESHGUI_Filter) aFilter =
    Handle(SMESHGUI_Filter)::DownCast(aSelector->GetFilter(myFilterType));

  return (!aFilter.IsNull() && aFilter->IsObjValid(theId));
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onAddBtn
// Purpose : SLOT. Called when "Add" button pressed.
//           Add selected in viewer entities in list box
//=======================================================================
void SMESHGUI_MultiEditDlg::onAddBtn()
{
  const SALOME_ListIO& aList = mySelector->StoredIObjects();

  int nbSelected = aList.Extent();
  if (nbSelected == 0)
    return;

  TColStd_IndexedMapOfInteger toBeAdded;

  if (!mySubmeshChk->isChecked() && !myGroupChk->isChecked()) {
    if (nbSelected > 0)
      mySelector->GetIndex(aList.First(),toBeAdded);
  } else if (mySubmeshChk->isChecked()) {
    SALOME_ListIteratorOfListIO anIter(aList);
    for ( ; anIter.More(); anIter.Next()) {
      SMESH::SMESH_subMesh_var aSubMesh =
        SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIter.Value());
      if (!aSubMesh->_is_nil()) {
        if (aSubMesh->GetFather()->GetId() == myMesh->GetId()) {
          SMESH::long_array_var anIds = aSubMesh->GetElementsId();
          for (int i = 0, n = anIds->length(); i < n; i++) {
            if (isIdValid(anIds[ i ]))
              toBeAdded.Add(anIds[ i ]);
          }
        }
      }
    }
  } else if (myGroupChk->isChecked()) {
    SALOME_ListIteratorOfListIO anIter(aList);
    for ( ; anIter.More(); anIter.Next()) {
      SMESH::SMESH_GroupBase_var aGroup =
        SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIter.Value());
      if (!aGroup->_is_nil() && ((aGroup->GetType() == SMESH::FACE && entityType() == 0) || 
                                 (aGroup->GetType() == SMESH::VOLUME && entityType() == 1))) {
        if (aGroup->GetMesh()->GetId() == myMesh->GetId()) {
          SMESH::long_array_var anIds = aGroup->GetListOfID();
          for (int i = 0, n = anIds->length(); i < n; i++) {
            if (isIdValid(anIds[ i ]))
              toBeAdded.Add(anIds[ i ]);
          }
        }
      }
    }
  } else {
  }

  myBusy = true;
  bool isGroupOrSubmesh = (mySubmeshChk->isChecked() || myGroupChk->isChecked());
  mySubmeshChk->setChecked(false);
  myGroupChk->setChecked(false);
  QStringList items;
  for(int i = 1; i <= toBeAdded.Extent(); i++)
    if (myIds.Add(toBeAdded(i))) {
      items.append(QString("%1").arg(toBeAdded(i)));
    }
  myListBox->addItems(items);
  myListBox->selectAll();
  myBusy = false;

  emit ListContensChanged();

  if (isGroupOrSubmesh)
    onListSelectionChanged();

  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::updateButtons
// Purpose : Enable/disable buttons of dialog in accordance with current state
//=======================================================================
void SMESHGUI_MultiEditDlg::updateButtons()
{
  bool isOk = isValid(false);
  myOkBtn->setEnabled(isOk);
  myApplyBtn->setEnabled(isOk);

  bool isListBoxNonEmpty = myListBox->count() > 0;
  bool isToAll = myToAllChk->isChecked();
  myFilterBtn->setEnabled(!isToAll);
  myRemoveBtn->setEnabled(myListBox->selectedItems().count() && !isToAll);
  mySortBtn->setEnabled(isListBoxNonEmpty &&!isToAll);

  const SALOME_ListIO& aList = mySelector->StoredIObjects();

  if (isToAll ||
      myMesh->_is_nil() ||
      aList.Extent() < 1 ||
      (SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(aList.First())->_is_nil() &&
       SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(aList.First())->_is_nil() &&
       SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(aList.First())->_is_nil()))
    myAddBtn->setEnabled(false);
  else
    myAddBtn->setEnabled(true);

  mySubmeshChk->setEnabled(!isToAll);
  mySubmeshBtn->setEnabled(mySubmeshChk->isChecked());
  mySubmesh->setEnabled(mySubmeshChk->isChecked());

  myGroupChk->setEnabled(!isToAll);
  myGroupBtn->setEnabled(myGroupChk->isChecked());
  myGroup->setEnabled(myGroupChk->isChecked());

  if (!mySubmeshChk->isChecked())
    mySubmesh->clear();
  if (!myGroupChk->isChecked())
    myGroup->clear();

}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onRemoveBtn
// Purpose : SLOT. Called when "Remove" button pressed.
//           Remove selected in list box entities
//=======================================================================
void SMESHGUI_MultiEditDlg::onRemoveBtn()
{
  myBusy = true;

  QList<QListWidgetItem*> selItems = myListBox->selectedItems();
  QListWidgetItem* item;
  foreach(item, selItems)
  {
    myIds.Remove(item->text().toInt());
    delete item;
  }

  myBusy = false;

  emit ListContensChanged();
  updateButtons();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onSortListBtn
// Purpose : SLOT. Called when "Sort list" button pressed.
//           Sort entities of list box
//=======================================================================
void SMESHGUI_MultiEditDlg::onSortListBtn()
{
  myBusy = true;

  int i, k = myListBox->count();
  if (k > 0)
  {
    QList<int> aSelected;
    std::vector<int> anArray(k);
    for (i = 0; i < k; i++)
    {
      int id = myListBox->item(i)->text().toInt();
      anArray[ i ] = id;
      if (myListBox->item(i)->isSelected())
        aSelected.append(id);
    }

    std::sort(anArray.begin(), anArray.end());

    myListBox->clear();
    for (i = 0; i < k; i++) {
      QListWidgetItem* item = new QListWidgetItem(QString::number(anArray[i]));
      myListBox->addItem(item);
      item->setSelected(aSelected.contains(anArray[i]));
    }
  }
  myBusy = false;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onListSelectionChanged
// Purpose : SLOT. Called when selection in list box changed.
//           Highlight in selected entities
//=======================================================================
void SMESHGUI_MultiEditDlg::onListSelectionChanged()
{
  if (myActor == 0 || myBusy)
    return;

  if (mySubmeshChk->isChecked() || myGroupChk->isChecked())
    return;

  SMESH_Actor * anActor = SMESH::FindActorByObject(myMesh);
  if (!anActor)
    anActor = myActor;
  TVisualObjPtr anObj = anActor->GetObject();

  TColStd_MapOfInteger anIndexes;
  int total = myListBox->count();
  for (int i = 0; i < total; i++)
  {
    if (myListBox->item(i)->isSelected())
    {
      int anId = myListBox->item(i)->text().toInt();
      if (anObj->GetElemVTKId(anId) >= 0) // avoid exception in hilight
        anIndexes.Add(anId);
    }
  }

  mySelector->AddOrRemoveIndex(anActor->getIO(),anIndexes,false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->highlight(anActor->getIO(),true,true);

  myRemoveBtn->setEnabled( anIndexes.Extent() );
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onSubmeshChk
// Purpose : SLOT. Called when state of "SubMesh" check box changed.
//           Activate/deactivate selection of submeshes
//=======================================================================
void SMESHGUI_MultiEditDlg::onSubmeshChk()
{
  bool isChecked = mySubmeshChk->isChecked();
  mySubmeshBtn->setEnabled(isChecked);
  mySubmesh->setEnabled(isChecked);
  if (!isChecked)
    mySubmesh->clear();
  if (isChecked && myGroupChk->isChecked())
      myGroupChk->setChecked(false);

  setSelectionMode();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onGroupChk
// Purpose : SLOT. Called when state of "Group" check box changed.
//           Activate/deactivate selection of groupes
//=======================================================================
void SMESHGUI_MultiEditDlg::onGroupChk()
{
  bool isChecked = myGroupChk->isChecked();
  myGroupBtn->setEnabled(isChecked);
  myGroup->setEnabled(isChecked);
  if (!isChecked)
    myGroup->clear();
  if (isChecked && mySubmeshChk->isChecked())
      mySubmeshChk->setChecked(false);

  setSelectionMode();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onToAllChk
// Purpose : SLOT. Called when state of "Apply to all" check box changed.
//           Activate/deactivate selection
//=======================================================================
void SMESHGUI_MultiEditDlg::onToAllChk()
{
  bool isChecked = myToAllChk->isChecked();

  if (isChecked)
    myListBox->clear();

  myIds.Clear();

  emit ListContensChanged();

  updateButtons();
  setSelectionMode();
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::setSelectionMode
// Purpose : Set selection mode
//=======================================================================
void SMESHGUI_MultiEditDlg::setSelectionMode()
{
  SMESH::RemoveFilters();

  //  mySelectionMgr->clearSelected();
  mySelectionMgr->clearFilters();

  if (mySubmeshChk->isChecked()) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(new SMESH_TypeFilter(SMESH::SUBMESH));
  }
  else if (myGroupChk->isChecked()) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(new SMESH_TypeFilter(SMESH::GROUP));
  }

  if (entityType()) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(VolumeSelection);
    SMESH::SetFilter(new SMESHGUI_VolumesFilter());
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(FaceSelection);
    if (myFilterType == SMESH::TriaFilter)
      SMESH::SetFilter(new SMESHGUI_TriangleFilter());
    else if (myFilterType == SMESH::QuadFilter)
      SMESH::SetFilter(new SMESHGUI_QuadrangleFilter());
    else
      SMESH::SetFilter(new SMESHGUI_FacesFilter());
  }
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::onApply
// Purpose : SLOT. Called when "Apply" button clicked.
//=======================================================================
bool SMESHGUI_MultiEditDlg::onApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;
  if (!isValid(true))
    return false;

  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
  if (aMeshEditor->_is_nil())
    return false;

  myBusy = true;

  SUIT_OverrideCursor aWaitCursor;

  SMESH::SMESH_IDSource_var obj;
  SMESH::long_array_var anIds = getIds(obj);

  bool aResult = process(aMeshEditor, anIds.inout(), obj);
  if (aResult) {
    if (myActor) {
      SALOME_ListIO sel;
      mySelectionMgr->selectedObjects( sel );
      mySelector->ClearIndex();
      mySelectionMgr->setSelectedObjects( sel );
      SMESH::UpdateView();
      SMESHGUI::Modified();
    }

    myListBox->clear();
    myIds.Clear();
    emit ListContensChanged();

    updateButtons();
  }
  obj._retn(); // else myMesh is deleted by ~obj

  myBusy = false;
  return aResult;
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::on3d2dChanged
// Purpose :
//=======================================================================
void SMESHGUI_MultiEditDlg::on3d2dChanged (int type)
{
  if (myEntityType != type) {
    myEntityType = type;

    myListBox->clear();
    myIds.Clear();

    emit ListContensChanged();

    if (type)
      myFilterType = SMESH::VolumeFilter;
    else
      myFilterType = SMESH::FaceFilter;

    updateButtons();
    setSelectionMode();
  }
}

//=======================================================================
// name    : SMESHGUI_MultiEditDlg::entityType
// Purpose :
//=======================================================================
int SMESHGUI_MultiEditDlg::entityType()
{
  return myEntityType;
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MultiEditDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}

/*!
 *  Class       : SMESHGUI_ChangeOrientationDlg
 *  Description : Modification of orientation of faces
 */

SMESHGUI_ChangeOrientationDlg
::SMESHGUI_ChangeOrientationDlg(SMESHGUI* theModule):
  SMESHGUI_MultiEditDlg(theModule, SMESH::FaceFilter, true)
{
  setWindowTitle(tr("CAPTION"));
  myHelpFileName = "changing_orientation_of_elements_page.html";
}

SMESHGUI_ChangeOrientationDlg::~SMESHGUI_ChangeOrientationDlg()
{
}

bool SMESHGUI_ChangeOrientationDlg::process (SMESH::SMESH_MeshEditor_ptr theEditor,
                                             const SMESH::long_array&    theIds,
                                             SMESH::SMESH_IDSource_ptr   obj)
{
  if ( CORBA::is_nil( obj ))
    return theEditor->Reorient(theIds);
  else
    return theEditor->ReorientObject( obj );
}

int SMESHGUI_ChangeOrientationDlg::nbElemsInMesh()
{
  return ( myFilterType == SMESH::FaceFilter ) ? myMesh->NbFaces() : myMesh->NbVolumes();
}

/*!
 *  Class       : SMESHGUI_UnionOfTrianglesDlg
 *  Description : Construction of quadrangles by automatic association of triangles
 */

SMESHGUI_UnionOfTrianglesDlg
::SMESHGUI_UnionOfTrianglesDlg(SMESHGUI* theModule):
  SMESHGUI_MultiEditDlg(theModule, SMESH::TriaFilter, false)
{
  setWindowTitle(tr("CAPTION"));

  //Preview check box
  myPreviewCheckBox = new QCheckBox(tr("PREVIEW"), mySelGrp);
  QGridLayout* aLay = (QGridLayout*)(mySelGrp->layout());
  aLay->addWidget(myPreviewCheckBox, aLay->rowCount(), 0, 1, aLay->columnCount());

  myComboBoxFunctor->setEnabled(true);  
  myComboBoxFunctor->addItem(tr("AREA_ELEMENTS"));
  myComboBoxFunctor->addItem(tr("WARP_ELEMENTS")); // for quadrangles only
  myComboBoxFunctor->addItem(tr("TAPER_ELEMENTS")); // for quadrangles only

  // Maximum angle
  QWidget* aMaxAngleGrp = new QWidget(myCriterionGrp);
  QHBoxLayout* aMaxAngleGrpLayout = new QHBoxLayout(aMaxAngleGrp);
  aMaxAngleGrpLayout->setMargin(0);
  aMaxAngleGrpLayout->setSpacing(SPACING);

  QLabel* aLab = new QLabel (tr("MAXIMUM_ANGLE"), aMaxAngleGrp);
  myMaxAngleSpin = new SMESHGUI_SpinBox (aMaxAngleGrp);
  myMaxAngleSpin->RangeStepAndValidator(0, 180.0, 1.0, "angle_precision");
  myMaxAngleSpin->SetValue(30.0);

  aMaxAngleGrpLayout->addWidget(aLab);
  aMaxAngleGrpLayout->addWidget(myMaxAngleSpin);

  ((QVBoxLayout*)(myCriterionGrp->layout()))->addWidget(aMaxAngleGrp);
  myCriterionGrp->show();

  connect(myComboBoxFunctor, SIGNAL(activated(int)),       this, SLOT(toDisplaySimulation()));
  connect(myMaxAngleSpin,    SIGNAL(valueChanged(int)),    this, SLOT(toDisplaySimulation()));
  connect(this,              SIGNAL(ListContensChanged()), this, SLOT(toDisplaySimulation()));
  connectPreviewControl(); //To Connect preview check box

  myPreviewCheckBox->setChecked(false);
  onDisplaySimulation(false);

  myHelpFileName = "uniting_set_of_triangles_page.html";
}

SMESHGUI_UnionOfTrianglesDlg::~SMESHGUI_UnionOfTrianglesDlg()
{
}

bool SMESHGUI_UnionOfTrianglesDlg::isValid (const bool theMess)
{
  bool ok = SMESHGUI_MultiEditDlg::isValid( theMess );
  if( !ok )
    return false;

  QString msg;
  ok = myMaxAngleSpin->isValid( msg, theMess );
  if( !ok ) {
    if( theMess ) {
      QString str( tr( "SMESH_INCORRECT_INPUT" ) );
      if ( !msg.isEmpty() )
        str += "\n" + msg;
      SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    }
    return false;
  }

  return ok;
}

bool SMESHGUI_UnionOfTrianglesDlg::process (SMESH::SMESH_MeshEditor_ptr theEditor,
                                            const SMESH::long_array&    theIds,
                                            SMESH::SMESH_IDSource_ptr   obj)
{
  {
    QStringList aParameters;
    aParameters << myMaxAngleSpin->text();
    myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );
  }
  SMESH::NumericalFunctor_var aCriterion = getNumericalFunctor();
  double aMaxAngle = myMaxAngleSpin->GetValue() * M_PI / 180.0;
  bool ok;
  if ( CORBA::is_nil( obj ))
    ok = theEditor->TriToQuad(theIds, aCriterion, aMaxAngle);
  else
    ok = theEditor->TriToQuadObject(obj, aCriterion, aMaxAngle);
  return ok;
}

int SMESHGUI_UnionOfTrianglesDlg::nbElemsInMesh()
{
  return myMesh->NbTriangles();
}
  
void SMESHGUI_UnionOfTrianglesDlg::onDisplaySimulation( bool toDisplayPreview )
{
  if ( myPreviewCheckBox->isChecked() && toDisplayPreview ) {
    if ( isValid( true ) ) {
      try{
        SUIT_OverrideCursor aWaitCursor;
        // get Ids of elements
        SMESH::SMESH_IDSource_var obj;
        SMESH::long_array_var anElemIds = getIds( obj );

        SMESH::NumericalFunctor_var aCriterion  = getNumericalFunctor();
        SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditPreviewer();

        double aMaxAngle = myMaxAngleSpin->GetValue() * M_PI / 180.0;
      
        if ( CORBA::is_nil( obj ) )
          aMeshEditor->TriToQuad( anElemIds.inout(), aCriterion, aMaxAngle );
        else
          aMeshEditor->TriToQuadObject( obj, aCriterion, aMaxAngle );
      
        SMESH::MeshPreviewStruct_var aMeshPreviewStruct = aMeshEditor->GetPreviewData();

        vtkProperty* aProp = vtkProperty::New();
        aProp->SetRepresentationToWireframe();
        aProp->SetColor( 250, 0, 250 );
        aProp->SetLineWidth( SMESH::GetFloat( "SMESH:element_width", 1 ) + 3 );
        mySimulation->GetActor()->SetProperty( aProp );
        aProp->Delete();

        mySimulation->SetData( aMeshPreviewStruct._retn() );
      } catch ( ... ) {
        hidePreview();
      }
    } else {
      hidePreview();
    }
  } else {
    hidePreview();
  }
}

/*!
 *  Class       : SMESHGUI_CuttingOfQuadsDlg
 *  Description : Automatic splitting of quadrangles into triangles
 */

SMESHGUI_CuttingOfQuadsDlg
::SMESHGUI_CuttingOfQuadsDlg(SMESHGUI* theModule):
  SMESHGUI_MultiEditDlg(theModule, SMESH::QuadFilter, false)
{
  setWindowTitle(tr("CAPTION"));
  myPreviewActor = 0;

  myPreviewChk = new QCheckBox (tr("PREVIEW"), mySelGrp);
  QGridLayout* aLay = (QGridLayout*)(mySelGrp->layout());
  aLay->addWidget(myPreviewChk, aLay->rowCount(), 0, 1, aLay->columnCount());

  // "split to 4 tria" option

  QRadioButton* to4TriaRB = new QRadioButton(tr("TO_4_TRIA"), myChoiceWidget);
  ((QVBoxLayout*)(myCriterionGrp->layout()))->insertWidget(0, to4TriaRB);
  myGroupChoice->addButton(to4TriaRB, 3);
  to4TriaRB->setChecked(true);
  onCriterionRB();

  myCriterionGrp->show();
  myChoiceWidget->show();
  myComboBoxFunctor->insertItem(0, tr("MIN_DIAG_ELEMENTS"));
  myComboBoxFunctor->setCurrentIndex(0);
  myComboBoxFunctor->setEnabled(false);

  connect(myPreviewChk,      SIGNAL(stateChanged(int)),    this, SLOT(onPreviewChk()));
  connect(myGroupChoice,     SIGNAL(buttonClicked(int)),   this, SLOT(onCriterionRB()));
  connect(myComboBoxFunctor, SIGNAL(activated(int)),       this, SLOT(onPreviewChk()));
  connect(this,              SIGNAL(ListContensChanged()), this, SLOT(onPreviewChk()));

  myHelpFileName = "cutting_quadrangles_page.html";
}

SMESHGUI_CuttingOfQuadsDlg::~SMESHGUI_CuttingOfQuadsDlg()
{
}

void SMESHGUI_CuttingOfQuadsDlg::reject()
{
  erasePreview();
  SMESHGUI_MultiEditDlg::reject();
}

bool SMESHGUI_CuttingOfQuadsDlg::process (SMESH::SMESH_MeshEditor_ptr theEditor,
                                          const SMESH::long_array&    theIds,
                                          SMESH::SMESH_IDSource_ptr   obj)
{
  bool hasObj = (! CORBA::is_nil( obj ));
  switch (myGroupChoice->checkedId()) {
  case 0: // use diagonal 1-3
    return hasObj ? theEditor->SplitQuadObject(obj, true) : theEditor->SplitQuad(theIds, true);
  case 1: // use diagonal 2-4
    return hasObj ? theEditor->SplitQuadObject(obj, false) : theEditor->SplitQuad(theIds, false);
  case 3: // split to 4 tria
    {
      if ( hasObj )
        return theEditor->QuadTo4Tri( obj ), true;
      SMESH::IDSource_wrap elems = theEditor->MakeIDSource( theIds, SMESH::FACE );
      theEditor->QuadTo4Tri( elems );
      return true;
    }
  default: // use numeric functor
    break;
  }

  SMESH::NumericalFunctor_var aCrit = getNumericalFunctor();
  return hasObj ? theEditor->QuadToTriObject(obj, aCrit) : theEditor->QuadToTri(theIds, aCrit);
}

int SMESHGUI_CuttingOfQuadsDlg::nbElemsInMesh()
{
  return myMesh->NbQuadrangles();
}


void SMESHGUI_CuttingOfQuadsDlg::onCriterionRB()
{
  if (myGroupChoice->checkedId() == 2) // Use numeric functor
    myComboBoxFunctor->setEnabled(true);
  else
    myComboBoxFunctor->setEnabled(false);

  if (myGroupChoice->checkedId() == 3) // To 4 tria
  {
    if ( myPreviewChk->isChecked() )
      myPreviewChk->setChecked( false );
    myPreviewChk->setEnabled( false );
  }
  else
  {
    myPreviewChk->setEnabled( true );
  }
  onPreviewChk();
}

void SMESHGUI_CuttingOfQuadsDlg::onPreviewChk()
{
  myPreviewChk->isChecked() ? displayPreview() : erasePreview();
}

void SMESHGUI_CuttingOfQuadsDlg::erasePreview()
{
  if (myPreviewActor == 0)
    return;

  if (SVTK_ViewWindow* vf = SMESH::GetCurrentVtkView()) {
    vf->RemoveActor(myPreviewActor);
    vf->Repaint();
  }
  myPreviewActor->Delete();
  myPreviewActor = 0;
}
  
void SMESHGUI_CuttingOfQuadsDlg::displayPreview()
{
  if (myActor == 0)
    return;

  if (myPreviewActor != 0)
    erasePreview();

  SUIT_OverrideCursor aWaitCursor;
  // get Ids of elements
  SMESH::SMESH_IDSource_var obj;
  SMESH::long_array_var anElemIds = getIds(obj);
  if (anElemIds->length() == 0 && obj->_is_nil() )
    return;

  SMDS_Mesh* aMesh = myActor->GetObject()->GetMesh();
  if (aMesh == 0)
    return;

  // 0 - use diagonal 1-3, 1 - use diagonal 2-4, 2 - use numerical functor
  int aChoice = myGroupChoice->checkedId();
  SMESH::NumericalFunctor_var aCriterion  = SMESH::NumericalFunctor::_nil();
  SMESH::SMESH_MeshEditor_var aMeshEditor = SMESH::SMESH_MeshEditor::_nil();
  if (aChoice == 2) {
    aCriterion  = getNumericalFunctor();
    aMeshEditor = myMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil())
      return;
  }

  if ( anElemIds->length() == 0 ) {
    anElemIds = obj->GetIDs();
  }

  //Create grid
  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::New();

  vtkIdType aNbCells = anElemIds->length() * 2;
  vtkIdType aCellsSize = 4 * aNbCells;
  vtkCellArray* aConnectivity = vtkCellArray::New();
  aConnectivity->Allocate(aCellsSize, 0);

  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints(anElemIds->length() * 4);

  vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
  aCellTypesArray->SetNumberOfComponents(1);
  aCellTypesArray->Allocate(aNbCells * aCellTypesArray->GetNumberOfComponents());

  vtkIdList *anIdList = vtkIdList::New();
  anIdList->SetNumberOfIds(3);

  TColStd_DataMapOfIntegerInteger anIdToVtk;

  int aNodes[ 4 ];
  int nbPoints = -1;
  for (int i = 0, n = anElemIds->length(); i < n; i++)
  {
    const SMDS_MeshElement* anElem = aMesh->FindElement(anElemIds[ i ]);
    if (anElem == 0 || anElem->NbNodes() != 4)
      continue;

    SMDS_ElemIteratorPtr anIter = anElem->nodesIterator();
    int k = 0;
    while (anIter->more()) {
      const SMDS_MeshNode* aNode = static_cast<const SMDS_MeshNode*>(anIter->next());
      if (aNode)
      {
        if (!anIdToVtk.IsBound(aNode->GetID()))
        {
          aPoints->SetPoint(++nbPoints, aNode->X(), aNode->Y(), aNode->Z());
          anIdToVtk.Bind(aNode->GetID(), nbPoints);
        }

        aNodes[ k++ ] = aNode->GetID();
      }
    }

    if (k != 4)
      continue;

    bool isDiag13 = true;
    if (aChoice == 0) // use diagonal 1-3
    {
      isDiag13 = true;
    }
    else if (aChoice == 1) // use diagonal 2-4
    {
      isDiag13 = false;
    }
    else // use numerical functor
    {
      // compare two sets of possible triangles
      int diag = aMeshEditor->BestSplit(anElemIds[i], aCriterion);
      if (diag == 1) // 1-3
        isDiag13 = true;
      else if (diag == 2) // 2-4
        isDiag13 = false;
      else // error
        continue;
    }

    if (isDiag13)
    {
      anIdList->SetId(0, anIdToVtk(aNodes[ 0 ]));
      anIdList->SetId(1, anIdToVtk(aNodes[ 1 ]));
      anIdList->SetId(2, anIdToVtk(aNodes[ 2 ]));
      aConnectivity->InsertNextCell(anIdList);
      aCellTypesArray->InsertNextValue(VTK_TRIANGLE);

      anIdList->SetId(0, anIdToVtk(aNodes[ 2 ]));
      anIdList->SetId(1, anIdToVtk(aNodes[ 3 ]));
      anIdList->SetId(2, anIdToVtk(aNodes[ 0 ]));
      aConnectivity->InsertNextCell(anIdList);
      aCellTypesArray->InsertNextValue(VTK_TRIANGLE);
    }
    else
    {
      anIdList->SetId(0, anIdToVtk(aNodes[ 1 ]));
      anIdList->SetId(1, anIdToVtk(aNodes[ 2 ]));
      anIdList->SetId(2, anIdToVtk(aNodes[ 3 ]));
      aConnectivity->InsertNextCell(anIdList);
      aCellTypesArray->InsertNextValue(VTK_TRIANGLE);

      anIdList->SetId(0, anIdToVtk(aNodes[ 3 ]));
      anIdList->SetId(1, anIdToVtk(aNodes[ 0 ]));
      anIdList->SetId(2, anIdToVtk(aNodes[ 1 ]));
      aConnectivity->InsertNextCell(anIdList);
      aCellTypesArray->InsertNextValue(VTK_TRIANGLE);
    }
  }

  VTKViewer_CellLocationsArray* aCellLocationsArray = VTKViewer_CellLocationsArray::New();
  aCellLocationsArray->SetNumberOfComponents(1);
  aCellLocationsArray->SetNumberOfTuples(aNbCells);

  aConnectivity->InitTraversal();
  for(vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell(npts, pts); idType++)
    aCellLocationsArray->SetValue(idType, aConnectivity->GetTraversalLocation(npts));

  aGrid->SetPoints(aPoints);
  aGrid->SetCells(aCellTypesArray, aCellLocationsArray,aConnectivity);

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInputData(aGrid);

  myPreviewActor = SALOME_Actor::New();
  myPreviewActor->PickableOff();
  myPreviewActor->SetMapper(aMapper);

  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor(250, 0, 250);
  aProp->SetLineWidth(myActor->GetLineWidth() + 1);
  myPreviewActor->SetProperty(aProp);

  SMESH::GetCurrentVtkView()->AddActor(myPreviewActor);
  SMESH::GetCurrentVtkView()->Repaint();

  aProp->Delete();
  aPoints->Delete();
  aConnectivity->Delete();
  aGrid->Delete();
  aMapper->Delete();
  anIdList->Delete();
  aCellTypesArray->Delete();
  aCellLocationsArray->Delete();
}

/*!
 *  Class       : SMESHGUI_SplitVolumesDlg
 *  Description : Spliter of volumes into tetrahedra or prisms
 */

SMESHGUI_SplitVolumesDlg::SMESHGUI_SplitVolumesDlg(SMESHGUI* theModule)
  : SMESHGUI_MultiEditDlg(theModule, SMESH::VolumeFilter, /*the3d2d=*/true, /*doInit=*/false)
{
  setWindowTitle(tr("CAPTION"));
  myHelpFileName = "split_to_tetra_page.html";
  myEntityType = 1;
  myCellSize = -1.;

  // Facet selection group

  myFacetSelGrp = new QGroupBox(tr("FACET_TO_SPLIT"), myCriterionGrp->parentWidget());
  QGridLayout* facetSelLayout = new QGridLayout( myFacetSelGrp );
  facetSelLayout->setMargin(MARGIN);
  facetSelLayout->setSpacing(SPACING);

  QLabel* pointLbl = new QLabel( tr("START_POINT"), myFacetSelGrp);
  QLabel* normalLbl = new QLabel( tr("FACET_NORMAL"), myFacetSelGrp);
  myFacetSelBtn = new QPushButton( mySubmeshBtn->icon(), "", myFacetSelGrp );
  myFacetSelBtn->setCheckable( true );
  QLabel* XLbl = new QLabel( tr("SMESH_X"), myFacetSelGrp);
  QLabel* YLbl = new QLabel( tr("SMESH_Y"), myFacetSelGrp);
  QLabel* ZLbl = new QLabel( tr("SMESH_Z"), myFacetSelGrp);
  QLabel* dXLbl = new QLabel( tr("SMESH_DX"), myFacetSelGrp);
  QLabel* dYLbl = new QLabel( tr("SMESH_DY"), myFacetSelGrp);
  QLabel* dZLbl = new QLabel( tr("SMESH_DZ"), myFacetSelGrp);
  for ( int i = 0; i < 3; ++i )
  {
    myPointSpin[i] = new SMESHGUI_SpinBox( myFacetSelGrp );
    myDirSpin  [i] = new SMESHGUI_SpinBox( myFacetSelGrp );
    myPointSpin[i]->RangeStepAndValidator( -1e10, 1e10, 10 );
    myDirSpin  [i]->RangeStepAndValidator( -1., 1., 0.1 );
    myPointSpin[i]->SetValue(0.);
    myDirSpin  [i]->SetValue(0.);
    myAxisBtn  [i] = new QPushButton( QString("|| O") + char('X'+i ), myFacetSelGrp);
  }
  myDirSpin[2]->SetValue(1.);

  myAllDomainsChk = new QCheckBox( tr("ALL_DOMAINS"), mySelGrp );

  facetSelLayout->addWidget( pointLbl,      0, 0 );
  facetSelLayout->addWidget( myFacetSelBtn, 0, 1 );
  facetSelLayout->addWidget( XLbl,          0, 2 );
  facetSelLayout->addWidget( myPointSpin[0],0, 3 );
  facetSelLayout->addWidget( YLbl,          0, 4 );
  facetSelLayout->addWidget( myPointSpin[1],0, 5 );
  facetSelLayout->addWidget( ZLbl,          0, 6 );
  facetSelLayout->addWidget( myPointSpin[2],0, 7 );

  facetSelLayout->addWidget( normalLbl,     1, 0 );
  facetSelLayout->addWidget( dXLbl,         1, 2 );
  facetSelLayout->addWidget( myDirSpin[0],  1, 3 );
  facetSelLayout->addWidget( dYLbl,         1, 4 );
  facetSelLayout->addWidget( myDirSpin[1],  1, 5 );
  facetSelLayout->addWidget( dZLbl,         1, 6 );
  facetSelLayout->addWidget( myDirSpin[2],  1, 7 );

  facetSelLayout->addWidget( myAxisBtn[0],  2, 2, 1, 2 );
  facetSelLayout->addWidget( myAxisBtn[1],  2, 4, 1, 2 );
  facetSelLayout->addWidget( myAxisBtn[2],  2, 6, 1, 2 );

  myCriterionGrp->layout()->addWidget( myFacetSelGrp );
  myCriterionGrp->layout()->addWidget( myAllDomainsChk );
  //myChoiceWidget->layout()->addWidget( myAllDomainsChk );

  connect( myFacetSelBtn,    SIGNAL(clicked(bool)), SLOT(onFacetSelection(bool)) );
  for ( int i = 0; i < 3; ++i )
  {
    connect( myAxisBtn  [i], SIGNAL(clicked()),     SLOT(onSetDir()) );
    connect( myPointSpin[i], SIGNAL(valueChanged       (const QString&)),
             this,           SLOT  (updateNormalPreview(const QString&)) );
    connect( myDirSpin  [i], SIGNAL(valueChanged       (const QString&)),
             this,           SLOT  (updateNormalPreview(const QString&)) );
  }
  if ( myEntityTypeGrp )
  {
    myEntityTypeGrp->button(0)->setText( tr("SMESH_TETRAS"));
    myEntityTypeGrp->button(1)->setText( tr("SMESH_PRISM"));
    if ( QGroupBox* gb = qobject_cast< QGroupBox* >( myEntityTypeGrp->button(0)->parent() ))
      gb->setTitle( tr("TARGET_ELEM_TYPE"));
  }

  myToAllChk->setChecked( true ); //aplly to the whole mesh by default

  bool hasHexa = true;//myMesh->_is_nil() ? false : myMesh->NbHexas();
  if ( hasHexa )
  {
    myCriterionGrp->setTitle( tr("SPLIT_METHOD"));
    myCriterionGrp->show();
    myComboBoxFunctor->hide();
    myChoiceWidget->show();
  }

  on3d2dChanged( 0 );
  Init();
}

SMESHGUI_SplitVolumesDlg::~SMESHGUI_SplitVolumesDlg()
{
}

bool SMESHGUI_SplitVolumesDlg::process (SMESH::SMESH_MeshEditor_ptr theEditor,
                                        const SMESH::long_array&    theIds,
                                        SMESH::SMESH_IDSource_ptr   theObj)
{
  SMESH::IDSource_wrap obj = theObj;
  if ( CORBA::is_nil( obj ))
    obj = theEditor->MakeIDSource( theIds, SMESH::VOLUME );
  else
    obj->Register();
  try {
    if ( isIntoPrisms() )
    {
      QStringList aParameters;
      aParameters << myPointSpin[0]->text();
      aParameters << myPointSpin[1]->text();
      aParameters << myPointSpin[2]->text();
      aParameters << myDirSpin[0]->text();
      aParameters << myDirSpin[1]->text();
      aParameters << myDirSpin[2]->text();
      myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      SMESH::PointStruct_var point = new SMESH::PointStruct;
      point->x  = myPointSpin[0]->GetValue();
      point->y  = myPointSpin[1]->GetValue();
      point->z  = myPointSpin[2]->GetValue();
      SMESH::DirStruct_var norm = new SMESH::DirStruct;
      norm->PS.x = myDirSpin[0]->GetValue();
      norm->PS.y = myDirSpin[1]->GetValue();
      norm->PS.z = myDirSpin[2]->GetValue();

      theEditor->SplitHexahedraIntoPrisms( obj, point, norm,
                                           myGroupChoice->checkedId()+1,
                                           myAllDomainsChk->isChecked() );
    }
    else
    {
      theEditor->SplitVolumesIntoTetra( obj, myGroupChoice->checkedId()+1 );
    }
  }
  catch ( const SALOME::SALOME_Exception& S_ex ) {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
    return false;
  }
  catch(...) {
    return false;
  }
  return true;
}

int SMESHGUI_SplitVolumesDlg::nbElemsInMesh()
{
  return isIntoPrisms() ? myMesh->NbHexas() : myMesh->NbVolumes() - myMesh->NbTetras();
}

bool SMESHGUI_SplitVolumesDlg::isIntoPrisms()
{
  return ( myEntityTypeGrp->checkedId() == 1 );
}

//================================================================================
/*!
 * \brief Slot called when a target element type changes
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::on3d2dChanged(int isPrism)
{
  if ( isPrism )
  {
    myFacetSelGrp->show();
    myAllDomainsChk->show();
    myGroupChoice->button(2)->hide();
    myGroupChoice->button(0)->setText( tr("SPLIT_HEX_TO_2_PRISMS"));
    myGroupChoice->button(1)->setText( tr("SPLIT_HEX_TO_4_PRISMS"));
  }
  else
  {
    myFacetSelGrp->hide();
    myAllDomainsChk->hide();
    myGroupChoice->button(2)->show();
    myGroupChoice->button(0)->setText( tr("SPLIT_HEX_TO_5_TETRA"));
    myGroupChoice->button(1)->setText( tr("SPLIT_HEX_TO_6_TETRA"));
    myGroupChoice->button(2)->setText( tr("SPLIT_HEX_TO_24_TETRA"));
  }
  SMESHGUI_MultiEditDlg::on3d2dChanged( !myEntityType );
  myEntityType = 1; // == VOLUME
  myChoiceWidget->hide();
  myChoiceWidget->show();
  resize(minimumSizeHint());
  onSelectionDone();
}

//================================================================================
/*!
 * \brief Set selection mode
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::setSelectionMode()
{
  if ( myBusy || !isEnabled() ) return;

  SMESH::RemoveFilters();

  mySelectionMgr->clearFilters();

  if (mySubmeshChk->isChecked()) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(new SMESH_TypeFilter(SMESH::SUBMESH));
    myFacetSelBtn->setChecked( false );
  }
  else if (myGroupChk->isChecked()) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(new SMESH_TypeFilter(SMESH::GROUP));
    myFacetSelBtn->setChecked( false );
  }

  if ( myFacetSelBtn->isChecked() )
  {
    // facet selection - select any element
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( CellSelection );
    myFilterType = SMESH::AllElementsFilter;
  }
  else
  {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( VolumeSelection );
    if ( isIntoPrisms() )
    {
      SMESH::SetFilter(new SMESHGUI_VolumeShapeFilter( SMDSGeom_HEXA ));
      myFilterType = SMESHGUI_VolumeShapeFilter::GetId( SMDSGeom_HEXA );
    }
    else // to tetrahedra
    {
      SMESH::SetFilter(new SMESHGUI_VolumesFilter());
      myFilterType = SMESH::VolumeFilter;
    }
  }
}

//================================================================================
/*!
 * \brief SLOT called when selection changed
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::onSelectionDone()
{
  if (myBusy || !isEnabled()) return;

  if ( !myFacetSelBtn->isChecked() )
  {
    SMESHGUI_MultiEditDlg::onSelectionDone();
  }
  else // set point and normal by a selected element
  {
    const SALOME_ListIO& aList = mySelector->StoredIObjects();
    int nbSel = aList.Extent();
    if (nbSel > 0)
    {
      Handle(SALOME_InteractiveObject) anIO = aList.First();

      myActor = SMESH::FindActorByEntry( anIO->getEntry() );

      SMESH::SMESH_Mesh_var aSelMesh = SMESH::GetMeshByIO(anIO);
      if (!aSelMesh->_is_nil())
        myMesh = aSelMesh;

      TColStd_IndexedMapOfInteger aMapIndex;
      mySelector->GetIndex( anIO, aMapIndex );
      if ( !aMapIndex.IsEmpty() )
        showFacetByElement( aMapIndex(1) );
      else if ( myCellSize < 0 )
        showFacetByElement( 1 );
    }
    updateButtons();
  }

  myCriterionGrp->setEnabled( !myMesh->_is_nil() && nbElemsInMesh() > 0 );
}

//================================================================================
/*!
 * \brief Show facet normal by a selected element
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::showFacetByElement( int elemID )
{
  if ( !isIntoPrisms() || !myActor )
  {
    mySimulation->SetVisibility( false );
    return;
  }
  SMDS_Mesh*              mesh = myActor->GetObject()->GetMesh();
  const SMDS_MeshElement* elem = mesh->FindElement( elemID );
  if ( !elem ) return;

  // set point XYZ by the element barycenter
  gp_XYZ bc( 0,0,0 );
  Bnd_B3d bbox;
  SMDS_NodeIteratorPtr nIt = elem->nodeIterator();
  std::vector< const SMDS_MeshNode* > nodes;
  nodes.reserve( elem->NbNodes() );
  while ( nIt->more() )
  {
    nodes.push_back( nIt->next() );
    gp_XYZ p = SMESH_TNodeXYZ( nodes.back() );
    bc += p;
    bbox.Add( p );
  }
  bc /= nodes.size();

  myPointSpin[0]->SetValue( bc.X() );
  myPointSpin[1]->SetValue( bc.Y() );
  myPointSpin[2]->SetValue( bc.Z() );

  // set size
  myCellSize = sqrt( bbox.SquareExtent() );

  // set normal and size
  gp_XYZ norm;
  switch ( elem->GetType())
  {
  case SMDSAbs_Edge:
  {
    norm = SMESH_TNodeXYZ( nodes[1] ) - SMESH_TNodeXYZ( nodes[0] );
    break;
  }
  case SMDSAbs_Face:
  {
    if ( !SMESH_MeshAlgos::FaceNormal( elem, norm, /*normalized=*/false ))
      return;
    break;
  }
  case SMDSAbs_Volume:
  {
    SMDS_VolumeTool vTool( elem );
    vTool.SetExternalNormal();
    bool freeFacetFound = false;
    double n[3];
    for ( int i = 0; i < vTool.NbFaces() && !freeFacetFound; ++i )
      if (( freeFacetFound = vTool.IsFreeFace( i )))
        vTool.GetFaceNormal( i, n[0], n[1], n[2] );
    if ( !freeFacetFound )
      vTool.GetFaceNormal( 0, n[0], n[1], n[2] );
    norm.SetCoord( n[0], n[1], n[2] );
    break;
  }
  default: return;
  }

  double size = norm.Modulus();
  if ( size < 1e-20 )
    return;
  norm /= size;

  myDirSpin[0]->SetValue( norm.X() );
  myDirSpin[1]->SetValue( norm.Y() );
  myDirSpin[2]->SetValue( norm.Z() );

  if ( myCellSize > 0. )
    updateNormalPreview();
}

//================================================================================
/*!
 * \brief SLOT called when a point or a normal changes
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::updateNormalPreview(const QString&)
{
  if ( myCellSize < 0. )
  {
    showFacetByElement( 1 );
    return;
  }

  gp_Pnt point ( myPointSpin[0]->GetValue(),
                 myPointSpin[1]->GetValue(),
                 myPointSpin[2]->GetValue() );
  gp_XYZ norm  ( myDirSpin[0]->GetValue(),
                 myDirSpin[1]->GetValue(),
                 myDirSpin[2]->GetValue() );
  if ( norm.Modulus() < 1e-20 )
    return;

  vtkUnstructuredGrid* grid = mySimulation->GetGrid();

  // Initialize the preview mesh of an arrow
  if ( grid->GetNumberOfPoints() == 0 )
  {
    mySimulation->SetArrowShapeAndNb( /*nb=*/1, /*hLen=*/0.3, /*R=*/0.1, /*start=*/0 );
  }

  // Compute new coordinates of the grid according to the dialog controls

  gp_Ax1 axis( point, norm );
  mySimulation->SetArrows( &axis, 4 * myCellSize );
  mySimulation->SetVisibility(true);
}

//================================================================================
/*!
 * \brief Slot called when facet selection button is clicked
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::onFacetSelection(bool isFacetSelection)
{
  setSelectionMode();
  onSelectionDone();
  mySelGrp->setEnabled( !isFacetSelection );
}

//================================================================================
/*!
 * \brief Slot called when an || axis button is clicked
 */
//================================================================================

void SMESHGUI_SplitVolumesDlg::onSetDir()
{
  myDirSpin[0]->SetValue(0.);
  myDirSpin[1]->SetValue(0.);
  myDirSpin[2]->SetValue(0.);
  int i = 0;
  for ( ; i < 3; ++i )
    if ( sender() == myAxisBtn[i] )
      break;
  if ( i == 3 )
    i = 0;
  myDirSpin[i]->SetValue(1.);

  if ( myActor && !myMesh->_is_nil() && myMesh->NbNodes() > 0 )
  {
    double b[6];
    myActor->GetUnstructuredGrid()->GetBounds(b);
    gp_XYZ center( 0.5 * ( b[0] + b[1] ),
                   0.5 * ( b[2] + b[3] ),
                   0.5 * ( b[4] + b[5] ));
    gp_XYZ point ( myPointSpin[0]->GetValue(),
                   myPointSpin[1]->GetValue(),
                   myPointSpin[2]->GetValue() );
    gp_XYZ norm  ( myDirSpin[0]->GetValue(),
                   myDirSpin[1]->GetValue(),
                   myDirSpin[2]->GetValue() );

    gp_Vec cp( center, point );
    if ( cp.Dot( norm ) < 0. )
      myDirSpin[i]->SetValue(-1.);
  }

  updateNormalPreview();
}
