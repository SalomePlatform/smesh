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
// File   : SMESHGUI_SmoothingDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SmoothingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_FilterDlg.h"

#include <SMESH_Actor.h>
#include <SMESH_TypeFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SalomeApp_IntSpinBox.h>

#include <SVTK_ViewModel.h>
#include <SVTK_Selector.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_Selection.h>
#include <SALOME_ListIO.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QButtonGroup>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

/*!
  \class BusyLocker
  \brief Simple 'busy state' flag locker.
  \internal
*/

class BusyLocker
{
public:
  //! Constructor. Sets passed boolean flag to \c true.
  BusyLocker( bool& busy ) : myBusy( busy ) { myBusy = true; }
  //! Destructor. Clear external boolean flag passed as parameter to the constructor to \c false.
  ~BusyLocker() { myBusy = false; }
private:
  bool& myBusy; //! External 'busy state' boolean flag
};


//=================================================================================
// function : SMESHGUI_SmoothingDlg()
// purpose  : constructor
//=================================================================================
SMESHGUI_SmoothingDlg::SMESHGUI_SmoothingDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    mySelectedObject(SMESH::SMESH_IDSource::_nil()),
    myFilterDlg(0)
{
  QPixmap image0 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_DLG_SMOOTHING")));
  QPixmap image1 (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_SMOOTHING"));
  setSizeGripEnabled(true);

  /***************************************************************/
  QVBoxLayout* SMESHGUI_SmoothingDlgLayout = new QVBoxLayout(this);
  SMESHGUI_SmoothingDlgLayout->setSpacing(SPACING);
  SMESHGUI_SmoothingDlgLayout->setMargin(MARGIN);

  /***************************************************************/
  GroupConstructors = new QGroupBox(tr("SMESH_SMOOTHING"), this);
  QButtonGroup* ButtonGroup = new QButtonGroup(this);
  QHBoxLayout* GroupConstructorsLayout = new QHBoxLayout(GroupConstructors);
  GroupConstructorsLayout->setSpacing(SPACING);
  GroupConstructorsLayout->setMargin(MARGIN);

  Constructor1 = new QRadioButton(GroupConstructors);
  Constructor1->setIcon(image0);
  Constructor1->setChecked(true);
  GroupConstructorsLayout->addWidget(Constructor1);
  ButtonGroup->addButton(Constructor1, 0);

  /***************************************************************/
  GroupArguments = new QGroupBox(tr("SMESH_ARGUMENTS"), this);
  QGridLayout* GroupArgumentsLayout = new QGridLayout(GroupArguments);
  GroupArgumentsLayout->setSpacing(SPACING);
  GroupArgumentsLayout->setMargin(MARGIN);

  myIdValidator = new SMESHGUI_IdValidator(this);

  // Controls for elements selection
  TextLabelElements = new QLabel(tr("SMESH_ID_ELEMENTS"), GroupArguments);

  SelectElementsButton = new QPushButton(GroupArguments);
  SelectElementsButton->setIcon(image1);

  LineEditElements = new QLineEdit(GroupArguments);
  LineEditElements->setValidator(myIdValidator);
  LineEditElements->setMaxLength(-1);
  myElemFilterBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(myElemFilterBtn,   SIGNAL(clicked()), this, SLOT(setElemFilters()));

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox(tr("SMESH_SELECT_WHOLE_MESH"), GroupArguments);

  // Controls for nodes selection
  TextLabelNodes = new QLabel(tr("FIXED_NODES_IDS"), GroupArguments);

  SelectNodesButton  = new QPushButton(GroupArguments);
  SelectNodesButton->setIcon(image1);

  LineEditNodes  = new QLineEdit(GroupArguments);
  LineEditNodes->setValidator(myIdValidator);
  LineEditNodes->setMaxLength(-1);
  QPushButton* filterNodeBtn = new QPushButton( tr( "SMESH_BUT_FILTER" ), GroupArguments );
  connect(filterNodeBtn,   SIGNAL(clicked()), this, SLOT(setNodeFilters()));

  // Controls for method selection
  TextLabelMethod = new QLabel(tr("METHOD"), GroupArguments);

  ComboBoxMethod = new QComboBox(GroupArguments);

  // Controls for iteration limit defining
  TextLabelLimit = new QLabel(tr("ITERATION_LIMIT"), GroupArguments);

  SpinBox_IterationLimit = new SalomeApp_IntSpinBox(GroupArguments);

  // Controls for max. aspect ratio defining
  TextLabelAspectRatio = new QLabel(tr("MAX_ASPECT_RATIO"), GroupArguments);

  SpinBox_AspectRatio = new SMESHGUI_SpinBox(GroupArguments);

  // Check box "Is Parametric"
  CheckBoxParametric = new QCheckBox( tr("IS_PARAMETRIC"), GroupArguments );

  GroupArgumentsLayout->addWidget(TextLabelElements,      0, 0);
  GroupArgumentsLayout->addWidget(SelectElementsButton,   0, 1);
  GroupArgumentsLayout->addWidget(LineEditElements,       0, 2);
  GroupArgumentsLayout->addWidget(myElemFilterBtn,        0, 3);
  GroupArgumentsLayout->addWidget(CheckBoxMesh,           1, 0, 1, 4);
  GroupArgumentsLayout->addWidget(TextLabelNodes,         2, 0);
  GroupArgumentsLayout->addWidget(SelectNodesButton,      2, 1);
  GroupArgumentsLayout->addWidget(LineEditNodes,          2, 2);
  GroupArgumentsLayout->addWidget(filterNodeBtn,          2, 3);
  GroupArgumentsLayout->addWidget(TextLabelMethod,        3, 0);
  GroupArgumentsLayout->addWidget(ComboBoxMethod,         3, 2, 1, 2);
  GroupArgumentsLayout->addWidget(TextLabelLimit,         4, 0);
  GroupArgumentsLayout->addWidget(SpinBox_IterationLimit, 4, 2, 1, 2);
  GroupArgumentsLayout->addWidget(TextLabelAspectRatio,   5, 0);
  GroupArgumentsLayout->addWidget(SpinBox_AspectRatio,    5, 2, 1, 2);
  GroupArgumentsLayout->addWidget(CheckBoxParametric,     6, 0, 1, 4);

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
  SMESHGUI_SmoothingDlgLayout->addWidget(GroupConstructors);
  SMESHGUI_SmoothingDlgLayout->addWidget(GroupArguments);
  SMESHGUI_SmoothingDlgLayout->addWidget(GroupButtons);

  /***************************************************************/
  /* Initialisations */
  ComboBoxMethod->addItem(tr("LAPLACIAN"));
  ComboBoxMethod->addItem(tr("CENTROIDAL"));

  ComboBoxMethod->setCurrentIndex(0);

  CheckBoxParametric->setChecked( true );
  
  SpinBox_IterationLimit->setRange(1, 999999);
  SpinBox_IterationLimit->setValue(20);
  SpinBox_AspectRatio->RangeStepAndValidator(0.0, +999999.999, 0.1, "parametric_precision");
  SpinBox_AspectRatio->SetValue(1.1);

  GroupArguments->show();
  myConstructorId = 0;
  Constructor1->setChecked(true);
  
  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  mySMESHGUI->SetActiveDialogBox(this);

  // Costruction of the logical filter for the elements: mesh/sub-mesh/group
  QList<SUIT_SelectionFilter*> aListOfFilters;
  aListOfFilters << new SMESH_TypeFilter(SMESH::MESHorSUBMESH) << new SMESH_TypeFilter(SMESH::GROUP);

  myMeshOrSubMeshOrGroupFilter =
    new SMESH_LogicalFilter (aListOfFilters, SMESH_LogicalFilter::LO_OR);

  myHelpFileName = "smoothing_page.html";

  Init();

  /***************************************************************/
  // signals and slots connections
  connect(buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(buttonApply, SIGNAL(clicked()),  this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));

  connect(SelectElementsButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(SelectNodesButton, SIGNAL (clicked()), this, SLOT(SetEditCurrentArgument()));
  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
  /* to close dialog if study change */
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()),      this, SLOT(reject()));
  connect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL (SignalCloseView()),            this, SLOT(onCloseView()));
  connect(LineEditElements, SIGNAL(textChanged(const QString&)),
           SLOT(onTextChange(const QString&)));
  connect(LineEditNodes, SIGNAL(textChanged(const QString&)),
           SLOT(onTextChange(const QString&)));
  connect(CheckBoxMesh, SIGNAL(toggled(bool)),
           SLOT(onSelectMesh(bool)));
}

//=================================================================================
// function : ~SMESHGUI_SmoothingDlg()
// purpose  : destructor
//=================================================================================
SMESHGUI_SmoothingDlg::~SMESHGUI_SmoothingDlg()
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
void SMESHGUI_SmoothingDlg::Init()
{
  myBusy = false;

//   ComboBoxMethod->setCurrentItem(0);

//   SpinBox_IterationLimit->setValue(20);
//   SpinBox_AspectRatio->SetValue(1.1);

  myEditCurrentArgument = LineEditElements;
  LineEditElements->setFocus();
  LineEditElements->clear();
  LineEditNodes->clear();
  myNbOkElements = 0;
  myNbOkNodes = 0;
  myActor     = 0;
  myIO.Nullify();
  myMesh = SMESH::SMESH_Mesh::_nil();

  CheckBoxMesh->setChecked(false);
  onSelectMesh(false);
}

//=================================================================================
// function : ClickOnApply()
// purpose  : Called when user presses <Apply> button
//=================================================================================
bool SMESHGUI_SmoothingDlg::ClickOnApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;

  if (!isValid())
    return false;

  if (myNbOkElements && (myNbOkNodes || LineEditNodes->text().trimmed().isEmpty())) {
    QStringList aListElementsId = LineEditElements->text().split(" ", QString::SkipEmptyParts);
    QStringList aListNodesId    = LineEditNodes->text().split(" ", QString::SkipEmptyParts);

    SMESH::long_array_var anElementsId = new SMESH::long_array;
    SMESH::long_array_var aNodesId = new SMESH::long_array;

    anElementsId->length(aListElementsId.count());
    for (int i = 0; i < aListElementsId.count(); i++)
      anElementsId[i] = aListElementsId[i].toInt();

    if ( myNbOkNodes && aListNodesId.count() > 0 ) {
      aNodesId->length(aListNodesId.count());
      for (int i = 0; i < aListNodesId.count(); i++)
        aNodesId[i] = aListNodesId[i].toInt();
    } else {
      aNodesId->length(0);
    }

    long anIterationLimit = (long)SpinBox_IterationLimit->value();
    double aMaxAspectRatio = SpinBox_AspectRatio->GetValue();

    QStringList aParameters;
    aParameters << SpinBox_IterationLimit->text();
    aParameters << SpinBox_AspectRatio->text();

    SMESH::SMESH_MeshEditor::Smooth_Method aMethod = SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH;
    if (ComboBoxMethod->currentIndex() > 0)
      aMethod =  SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH;

    bool aResult = false;
    try {
      SUIT_OverrideCursor aWaitCursor;
      SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();

      myMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      if ( CheckBoxParametric->isChecked() ) {
        if(CheckBoxMesh->isChecked())
          aResult = aMeshEditor->SmoothParametricObject(mySelectedObject, aNodesId.inout(),
                                                        anIterationLimit, aMaxAspectRatio, aMethod);
        else
          aResult = aMeshEditor->SmoothParametric(anElementsId.inout(), aNodesId.inout(),
                                                  anIterationLimit, aMaxAspectRatio, aMethod);
      }
      else {
        if(CheckBoxMesh->isChecked())
          aResult = aMeshEditor->SmoothObject(mySelectedObject, aNodesId.inout(),
                                              anIterationLimit, aMaxAspectRatio, aMethod);
        else
          aResult = aMeshEditor->Smooth(anElementsId.inout(), aNodesId.inout(),
                                        anIterationLimit, aMaxAspectRatio, aMethod);
      }

    } catch (...) {
    }

    if (aResult) {
      SMESH::Update(myIO, SMESH::eDisplay);
      SMESH::RepaintCurrentView();
      SMESHGUI::Modified();
      //Init();

      //mySelectedObject = SMESH::SMESH_IDSource::_nil();
    }
  }

  return true;
}

//=================================================================================
// function : ClickOnOk()
// purpose  : Called when user presses <OK> button
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnOk()
{
  if( ClickOnApply() )
    reject();
}

//=================================================================================
// function : reject()
// purpose  : Called when dialog box is closed
//=================================================================================
void SMESHGUI_SmoothingDlg::reject()
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

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::onOpenView()
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
void SMESHGUI_SmoothingDlg::onCloseView()
{
  DeactivateActiveDialog();
  mySelector = 0;
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnHelp()
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
// function : onTextChange()
// purpose  :
//=======================================================================
void SMESHGUI_SmoothingDlg::onTextChange (const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  // return if busy
  if (myBusy || myIO.IsNull()) return;

  // set busy flag
  BusyLocker lock( myBusy );

  if (send == LineEditElements)
    myNbOkElements = 0;
  else if (send == LineEditNodes)
    myNbOkNodes = 0;

  buttonOk->setEnabled(false);
  buttonApply->setEnabled(false);

  // hilight entered elements/nodes
  SMDS_Mesh* aMesh = myActor ? myActor->GetObject()->GetMesh() : 0;
  QStringList aListId = theNewText.split(" ", QString::SkipEmptyParts);

  if (send == LineEditElements) {
    TColStd_MapOfInteger newIndices;
    for (int i = 0; i < aListId.count(); i++) {
      int id = aListId[ i ].toInt();
      if ( id > 0 ) {
        bool validId = aMesh ? ( aMesh->FindElement( id ) != 0 ) : ( myMesh->GetElementType( id, true ) != SMESH::EDGE );
        if ( validId ) 
          newIndices.Add( id );
      }
      myNbOkElements = newIndices.Extent();
      mySelector->AddOrRemoveIndex(myIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight( myIO, true, true );
    }
  }
  else if (send == LineEditNodes) {
    TColStd_MapOfInteger newIndices;
    for (int i = 0; i < aListId.count(); i++) {
      int id = aListId[ i ].toInt();
      if ( id > 0 ) {
        bool validId = aMesh ? ( aMesh->FindNode( id ) != 0 ) : ( myMesh->GetElementType( id, false ) != SMESH::EDGE );
        if ( validId ) 
          newIndices.Add( id );
      }
      myNbOkNodes = newIndices.Extent();
      mySelector->AddOrRemoveIndex(myIO, newIndices, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight( myIO, true, true );
    }
  }

  if (myNbOkElements && (myNbOkNodes || LineEditNodes->text().trimmed().isEmpty())) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed or other cases
//=================================================================================
void SMESHGUI_SmoothingDlg::SelectionIntoArgument()
{
  if (myBusy) return;
  if (myFilterDlg && myFilterDlg->isVisible()) return; // filter dlg active

  // clear
  QString aString = "";

  // set busy flag
  BusyLocker lock( myBusy );

  if (myEditCurrentArgument == LineEditElements ||
      myEditCurrentArgument == LineEditNodes)
  {
    myEditCurrentArgument->setText(aString);
    if (myEditCurrentArgument == LineEditElements) {
      myNbOkElements = 0;
      myActor = 0;
      myIO.Nullify();
    }
    else {
      myNbOkNodes = 0;
    }
    buttonOk->setEnabled(false);
    buttonApply->setEnabled(false);
  }

  if (!GroupButtons->isEnabled()) // inactive
    return;

  // get selected mesh
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);
  int nbSel = aList.Extent();
  if (nbSel == 1)
  {
    Handle(SALOME_InteractiveObject) IO = aList.First();

    if (myEditCurrentArgument == LineEditElements) {
      myMesh = SMESH::GetMeshByIO(IO);
      if (myMesh->_is_nil())
        return;
      myIO = IO;
      myActor = SMESH::FindActorByObject(myMesh);

      if (CheckBoxMesh->isChecked()) {
        SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aString);

        SMESH::SMESH_IDSource_var obj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( myIO );
        if ( !CORBA::is_nil( obj ) )
          mySelectedObject = obj;
        else
          return;
        myNbOkElements = true;
      } else {
        // get indices of selected elements
        TColStd_IndexedMapOfInteger aMapIndex;
        mySelector->GetIndex(IO,aMapIndex);
        myNbOkElements = aMapIndex.Extent();

        if (myNbOkElements < 1)
          return;

        QStringList elements;
        for ( int i = 0; i < myNbOkElements; ++i )
          elements << QString::number( aMapIndex( i+1 ) );
        aString = elements.join(" ");
      }
    } else if (myEditCurrentArgument == LineEditNodes && !myMesh->_is_nil() && myIO->isSame(IO) )
    {
      myNbOkNodes = SMESH::GetNameOfSelectedNodes(mySelector, IO, aString);
    }
  }

  myEditCurrentArgument->setText(aString);
  myEditCurrentArgument->repaint();
  myEditCurrentArgument->setEnabled(false); // to update lineedit IPAL 19809
  myEditCurrentArgument->setEnabled(true);

  if (myNbOkElements && (myNbOkNodes || LineEditNodes->text().trimmed().isEmpty())) {
    buttonOk->setEnabled(true);
    buttonApply->setEnabled(true);
  }
}

//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();

  switch (myConstructorId) {
  case 0: /* default constructor */
    {
      disconnect(mySelectionMgr, 0, this, 0);
      mySelectionMgr->clearSelected();
      mySelectionMgr->clearFilters();

      if (send == SelectElementsButton) {
        myEditCurrentArgument = LineEditElements;
        SMESH::SetPointRepresentation(false);
        if (CheckBoxMesh->isChecked()) {
          //          mySelectionMgr->setSelectionModes(ActorSelection);
          if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->SetSelectionMode(ActorSelection);
          mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
        } else {
          if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
            aViewWindow->SetSelectionMode(FaceSelection);
        }
      } else if (send == SelectNodesButton) {
        LineEditNodes->clear();
        myEditCurrentArgument = LineEditNodes;
        SMESH::SetPointRepresentation(true);
        if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI )) {
          aViewWindow->SetSelectionMode(NodeSelection);
        }
      }

      myEditCurrentArgument->setFocus();
      connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
      SelectionIntoArgument();
      break;
    }
  }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : Deactivates this dialog
//=================================================================================
void SMESHGUI_SmoothingDlg::DeactivateActiveDialog()
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
void SMESHGUI_SmoothingDlg::ActivateThisDialog()
{
  // Emit a signal to deactivate the active dialog
  mySMESHGUI->EmitSignalDeactivateDialog();
  GroupConstructors->setEnabled(true);
  GroupArguments->setEnabled(true);
  GroupButtons->setEnabled(true);

  mySMESHGUI->SetActiveDialogBox(this);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(FaceSelection);
  SelectionIntoArgument();
}

//=================================================================================
// function : enterEvent()
// purpose  : Mouse enter event
//=================================================================================
void SMESHGUI_SmoothingDlg::enterEvent (QEvent*)
{
  if (!GroupConstructors->isEnabled()) {
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow && !mySelector) {
      mySelector = aViewWindow->GetSelector();
    }
    ActivateThisDialog();
  }
}

//=======================================================================
// function : onSelectMesh()
// purpose  :
//=======================================================================
void SMESHGUI_SmoothingDlg::onSelectMesh (bool toSelectMesh)
{
  if (toSelectMesh)
    TextLabelElements->setText(tr("SMESH_NAME"));
  else
    TextLabelElements->setText(tr("SMESH_ID_ELEMENTS"));
  myElemFilterBtn->setEnabled(!toSelectMesh);

  if (myEditCurrentArgument != LineEditElements &&
      myEditCurrentArgument != LineEditNodes) {
    LineEditElements->clear();
    LineEditNodes->clear();
    return;
  }

  mySelectionMgr->clearFilters();
  SMESH::SetPointRepresentation(false);

  if (toSelectMesh) {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    //    mySelectionMgr->setSelectionModes(ActorSelection);
    mySelectionMgr->installFilter(myMeshOrSubMeshOrGroupFilter);
    myEditCurrentArgument->setReadOnly(true);
    myEditCurrentArgument->setValidator(0);
  } else {
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(myEditCurrentArgument == LineEditElements ? FaceSelection 
                                                                              : NodeSelection );
    myEditCurrentArgument->setReadOnly(false);
    LineEditElements->setValidator(myIdValidator);
    onTextChange(myEditCurrentArgument->text());
  }

  SelectionIntoArgument();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::keyPressEvent( QKeyEvent* e )
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
// purpose  : activate filter dialog
//=================================================================================
void SMESHGUI_SmoothingDlg::setFilters( const bool theIsElem )
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
    types.append( SMESH::NODE );
    types.append( SMESH::ALL );
    myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, types );
  }
  myFilterDlg->Init( theIsElem ? SMESH::ALL : SMESH::NODE );

  myFilterDlg->SetSelection();
  myFilterDlg->SetMesh( myMesh );
  myFilterDlg->SetSourceWg( theIsElem ? LineEditElements : LineEditNodes );

  myFilterDlg->show();
}

//=================================================================================
// function : setElemFilters()
// purpose  : SLOT. Called when element "Filter" button pressed.
//=================================================================================
void SMESHGUI_SmoothingDlg::setElemFilters()
{
  setFilters( true );
}

//=================================================================================
// function : setNodeFilters()
// purpose  : SLOT. Called when node "Filter" button pressed.
//=================================================================================
void SMESHGUI_SmoothingDlg::setNodeFilters()
{
  setFilters( false );
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================
bool SMESHGUI_SmoothingDlg::isValid()
{
  QString msg;
  bool ok = true;
  ok = SpinBox_IterationLimit->isValid( msg, true ) && ok;
  ok = SpinBox_AspectRatio->isValid( msg, true ) && ok;

  if( !ok ) {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return false;
  }
  return true;
}
