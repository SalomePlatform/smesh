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
// File   : SMESHGUI_DuplicateNodesDlg.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.

// SMESH includes
#include "SMESHGUI_DuplicateNodesDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SalomeApp_Tools.h>

#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

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
#include <QKeyEvent>

#include <utilities.h>
#include <SALOMEDSClient_SObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
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

/*!
  \brief Constructor
  \param theModule Mesh module instance
*/
SMESHGUI_DuplicateNodesDlg::SMESHGUI_DuplicateNodesDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  // Dialog attributes
  setModal(false);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("SMESH_DUPLICATE_TITLE"));
  setSizeGripEnabled(true);

  // Icons for the dialog operation modes and selection button
  SUIT_ResourceMgr* aResMgr = SMESH::GetResourceMgr( mySMESHGUI );
  QPixmap iconWithoutElem (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_DUPLICATE_NODES")));
  QPixmap iconWithElem (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_DUPLICATE_NODES_WITH_ELEM")));
  QPixmap iconElemOnly (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_DUPLICATE_ELEM_ONLY")));
  QPixmap iconGrpBoundary (aResMgr->loadPixmap("SMESH", tr("ICON_SMESH_DUPLICATE_GROUP_BOUNDARY")));
  QPixmap iconSelect (aResMgr->loadPixmap("SMESH", tr("ICON_SELECT")));

  // Main layout
  QVBoxLayout* aMainLayout = new QVBoxLayout(this);
  aMainLayout->setSpacing(SPACING);
  aMainLayout->setMargin(MARGIN);

  // Operation modes selector
  QGroupBox* aConstructorsBox = new QGroupBox(tr("DUPLICATION_MODE"), this);
  myGroupConstructors = new QButtonGroup(this);
  QHBoxLayout* aConstructorsBoxLayout = new QHBoxLayout(aConstructorsBox);
  aConstructorsBoxLayout->setSpacing(SPACING);
  aConstructorsBoxLayout->setMargin(MARGIN);

  QRadioButton* aRadioButton1 = new QRadioButton(aConstructorsBox);
  aRadioButton1->setIcon(iconWithoutElem);
  QRadioButton* aRadioButton2 = new QRadioButton(aConstructorsBox);
  aRadioButton2->setIcon(iconWithElem);
  QRadioButton* aRadioButton3 = new QRadioButton(aConstructorsBox);
  aRadioButton3->setIcon(iconElemOnly);
  QRadioButton* aRadioButton4 = new QRadioButton(aConstructorsBox);
  aRadioButton4->setIcon(iconGrpBoundary);
  
  aConstructorsBoxLayout->addWidget(aRadioButton1);
  aConstructorsBoxLayout->addWidget(aRadioButton2);
  aConstructorsBoxLayout->addWidget(aRadioButton3);
  aConstructorsBoxLayout->addWidget(aRadioButton4);
  myGroupConstructors->addButton(aRadioButton1, 0);
  myGroupConstructors->addButton(aRadioButton2, 1);
  myGroupConstructors->addButton(aRadioButton3, 2);
  myGroupConstructors->addButton(aRadioButton4, 3);

  // Arguments
  myGroupArguments = new QGroupBox(this);
  QGridLayout* aGroupArgumentsLayout = new QGridLayout(myGroupArguments);
  aGroupArgumentsLayout->setSpacing(SPACING);
  aGroupArgumentsLayout->setMargin(MARGIN);
    
  myTextLabel1 = new QLabel(myGroupArguments);
  mySelectButton1 = new QPushButton(myGroupArguments);
  mySelectButton1->setIcon(iconSelect);
  myLineEdit1 = new QLineEdit(myGroupArguments);
  myLineEdit1->setReadOnly(true);

  myTextLabel2 = new QLabel(myGroupArguments);
  mySelectButton2 = new QPushButton(myGroupArguments);
  mySelectButton2->setIcon(iconSelect);
  myLineEdit2 = new QLineEdit(myGroupArguments);
  myLineEdit2->setReadOnly(true);

  myTextLabel3 = new QLabel(myGroupArguments);
  mySelectButton3 = new QPushButton(myGroupArguments);
  mySelectButton3->setIcon(iconSelect);
  myLineEdit3 = new QLineEdit(myGroupArguments);
  myLineEdit3->setReadOnly(true);

  myCheckBox1 = new QCheckBox(tr("CONSTRUCT_NEW_GROUP_ELEMENTS"), myGroupArguments);
  myCheckBox2 = new QCheckBox(tr("CONSTRUCT_NEW_GROUP_NODES"), myGroupArguments);

  aGroupArgumentsLayout->addWidget(myTextLabel1,    0, 0);
  aGroupArgumentsLayout->addWidget(mySelectButton1, 0, 1);
  aGroupArgumentsLayout->addWidget(myLineEdit1,     0, 2);
  aGroupArgumentsLayout->addWidget(myTextLabel2,    1, 0);
  aGroupArgumentsLayout->addWidget(mySelectButton2, 1, 1);
  aGroupArgumentsLayout->addWidget(myLineEdit2,     1, 2);
  aGroupArgumentsLayout->addWidget(myTextLabel3,    2, 0);
  aGroupArgumentsLayout->addWidget(mySelectButton3, 2, 1);
  aGroupArgumentsLayout->addWidget(myLineEdit3,     2, 2);
  aGroupArgumentsLayout->addWidget(myCheckBox1, 3, 0);
  aGroupArgumentsLayout->addWidget(myCheckBox2, 4, 0);
  aGroupArgumentsLayout->setRowStretch(5, 1);
  
  // Buttons
  QGroupBox* aGroupButtons = new QGroupBox(this);
  QHBoxLayout* aGroupButtonsLayout = new QHBoxLayout(aGroupButtons);
  aGroupButtonsLayout->setSpacing(SPACING);
  aGroupButtonsLayout->setMargin(MARGIN);

  myButtonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aGroupButtons);
  myButtonOk->setAutoDefault(true);
  myButtonOk->setDefault(true);
  myButtonApply = new QPushButton(tr("SMESH_BUT_APPLY"), aGroupButtons);
  myButtonApply->setAutoDefault(true);
  myButtonClose = new QPushButton(tr("SMESH_BUT_CLOSE"), aGroupButtons);
  myButtonClose->setAutoDefault(true);
  myButtonHelp = new QPushButton(tr("SMESH_BUT_HELP"), aGroupButtons);
  myButtonHelp->setAutoDefault(true);

  aGroupButtonsLayout->addWidget(myButtonOk);
  aGroupButtonsLayout->addSpacing(10);
  aGroupButtonsLayout->addWidget(myButtonApply);
  aGroupButtonsLayout->addSpacing(10);
  aGroupButtonsLayout->addStretch();
  aGroupButtonsLayout->addWidget(myButtonClose);
  aGroupButtonsLayout->addWidget(myButtonHelp);

  // Add mode selector, arguments and buttons to the main layout
  aMainLayout->addWidget(aConstructorsBox);
  aMainLayout->addWidget(myGroupArguments);
  aMainLayout->addWidget(aGroupButtons);
  
  myCheckBox1->setChecked(true);
  myCheckBox2->setChecked(true);

  // Initialize the dialog
  Init();

  // Help file name
  myHelpFileName = "double_nodes_page.html";

  // Signals and slots connections
  connect(myGroupConstructors, SIGNAL(buttonClicked(int)), SLOT(onConstructorsClicked(int)));
     
  connect(mySelectButton1, SIGNAL (clicked()), this, SLOT(onEditCurrentArgument()));
  connect(mySelectButton2, SIGNAL (clicked()), this, SLOT(onEditCurrentArgument()));
  connect(mySelectButton3, SIGNAL (clicked()), this, SLOT(onEditCurrentArgument()));

  connect(myCheckBox2,    SIGNAL(stateChanged(int)), SLOT(updateButtons()));

  connect(myButtonOk,     SIGNAL(clicked()), this, SLOT(onOk()));
  connect(myButtonClose,  SIGNAL(clicked()), this, SLOT(reject()));
  connect(myButtonApply,  SIGNAL(clicked()), this, SLOT(onApply()));
  connect(myButtonHelp,   SIGNAL(clicked()), this, SLOT(onHelp()));
  
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionChanged()));

  connect(mySMESHGUI, SIGNAL (SignalDeactivateActiveDialog()), this, SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(reject()));
}

/*!
  \brief Destructor
*/
SMESHGUI_DuplicateNodesDlg::~SMESHGUI_DuplicateNodesDlg()
{
}

/*!
  \brief Destructor
*/
void SMESHGUI_DuplicateNodesDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // Set initial parameters
  myBusy = false;
  myCurrentLineEdit = myLineEdit1;

  myGroups1.clear();
  myGroups2.clear();
  myGroups3.clear();
  
  // Set selection mode
  mySelectionMgr->installFilter(new SMESH_TypeFilter(SMESH::GROUP));
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  
  // Set construction mode
  int operationMode = myGroupConstructors->checkedId();
  if (operationMode < 0) {
    // The dialog has been just displayed
    operationMode = 0;
    myGroupConstructors->button(0)->setChecked(true);
  }
  onConstructorsClicked(operationMode);
}

/*!
  \brief SLOT called to change the dialog operation mode.
  \param constructorId id of the radio button in mode selector button group
*/
void SMESHGUI_DuplicateNodesDlg::onConstructorsClicked (int constructorId)
{
  // Clear all fields
  myLineEdit1->clear();
  myLineEdit2->clear();
  myLineEdit3->clear();

  myGroups1.clear();
  myGroups2.clear();
  myGroups3.clear();

  // Set the first field as current
  myCurrentLineEdit = myLineEdit1;
  myCurrentLineEdit->setFocus();

  switch (constructorId) {
  case 0:
    {
      // Set text to the group of arguments and to the first two labels
      myGroupArguments->setTitle(tr("DUPLICATION_WITHOUT_ELEMS"));
      myTextLabel1->setText(tr("GROUP_NODES_TO_DUPLICATE"));
      myTextLabel2->setText(tr("GROUP_NODES_TO_REPLACE"));

      myCheckBox1->hide();
      myCheckBox2->show();
      myCheckBox2->setText( tr("CONSTRUCT_NEW_GROUP_NODES"));
      
      // Hide the third field
      myTextLabel2->show();
      mySelectButton2->show();
      myLineEdit2->show();
      myTextLabel3->hide();
      mySelectButton3->hide();
      myLineEdit3->hide();
      
      break;
    }
  case 1:
    {
      // Set text to the group of arguments and to all the labels
      myGroupArguments->setTitle(tr("DUPLICATION_WITH_ELEMS"));
      myTextLabel1->setText(tr("GROUP_ELEMS_TO_DUPLICATE"));
      myTextLabel2->setText(tr("GROUP_NODES_NOT_DUPLICATE"));
      myTextLabel3->setText(tr("GROUP_ELEMS_TO_REPLACE"));
      
      myCheckBox1->show();
      myCheckBox2->show();
      myCheckBox1->setText( tr("CONSTRUCT_NEW_GROUP_ELEMENTS"));
      myCheckBox2->setText( tr("CONSTRUCT_NEW_GROUP_NODES"));

      // Show the third field
      myTextLabel2->show();
      mySelectButton2->show();
      myLineEdit2->show();
      myTextLabel3->show();
      mySelectButton3->show();
      myLineEdit3->show();

      break;
    }
  case 2:
    {
      // Set text to the group of arguments and to all the labels
      myGroupArguments->setTitle(tr("DUPLICATION_ONLY_ELEMS"));
      myTextLabel1->setText(tr("GROUP_ELEMS_TO_DUPLICATE"));
      
      myCheckBox1->show();
      myCheckBox1->setText( tr("CONSTRUCT_NEW_GROUP_ELEMENTS"));
      myCheckBox2->hide();

      // Hide the second and the third field
      myTextLabel2->hide();
      mySelectButton2->hide();
      myLineEdit2->hide();
      myTextLabel3->hide();
      mySelectButton3->hide();
      myLineEdit3->hide();

      break;
    }
  case 3:
    {
      // Set text to the group of arguments and to all the labels
      myGroupArguments->setTitle(tr("DUPLICATION_GROUP_BOUNDARY"));
      myTextLabel1->setText(tr("GROUP_VOLUME_GROUPS"));
      
      myCheckBox1->show();
      myCheckBox2->show();
      myCheckBox1->setText( tr("CREATE_JOINT_ELEMENTS"));
      myCheckBox2->setText( tr("ON_ALL_BOUNDARIES"));

      // Hide the second and the third field
      myTextLabel2->hide();
      mySelectButton2->hide();
      myLineEdit2->hide();
      myTextLabel3->hide();
      mySelectButton3->hide();
      myLineEdit3->hide();

      break;
    }
  }
  myGroupArguments->hide();
  myGroupArguments->show();
  this->resize(this->sizeHint().width(), this->minimumSizeHint().height());
  // Process selection
  onSelectionChanged();
}

/*!
  \brief SLOT called to apply changes.
*/
bool SMESHGUI_DuplicateNodesDlg::onApply()
{
  if ( mySMESHGUI->isActiveStudyLocked() || !isValid() )
    return false;

  BusyLocker lock( myBusy );
 
  bool toCreateElemGroup = myCheckBox1->isChecked();
  bool toCreateNodeGroup = myCheckBox2->isChecked();
  int operationMode      = myGroupConstructors->checkedId();
  
  // Apply changes
  bool result = false;
  SUIT_OverrideCursor aWaitCursor;
  QStringList anEntryList;

  try {
    SMESH::SMESH_Mesh_var aMesh = myGroups1[0]->GetMesh();
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();

    switch ( operationMode ) {
    case 0:
    {
      SMESH::ListOfGroups_var g1 = new SMESH::ListOfGroups();
      g1->length( myGroups1.count() );
      for ( int i = 0; i < myGroups1.count(); i++ )
        g1[i] = myGroups1[i];
      SMESH::ListOfGroups_var g2 = new SMESH::ListOfGroups();
      g2->length( myGroups2.count() );
      for ( int i = 0; i < myGroups2.count(); i++ )
        g2[i] = myGroups2[i];

      if ( toCreateNodeGroup ) {
        SMESH::SMESH_GroupBase_var aNewGroup = 
          aMeshEditor->DoubleNodeGroupsNew( g1.in(), g2.in() );
        result = !CORBA::is_nil( aNewGroup );
      }
      else {
        result = aMeshEditor->DoubleNodeGroups( g1.in(), g2.in() );
      }
      break;
    }
    case 1:
    {
      SMESH::ListOfGroups_var g1 = new SMESH::ListOfGroups();
      g1->length( myGroups1.count() );
      for ( int i = 0; i < myGroups1.count(); i++ )
        g1[i] = myGroups1[i];
      SMESH::ListOfGroups_var g2 = new SMESH::ListOfGroups();
      g2->length( myGroups2.count() );
      for ( int i = 0; i < myGroups2.count(); i++ )
        g2[i] = myGroups2[i];
      SMESH::ListOfGroups_var g3 = new SMESH::ListOfGroups();
      g3->length( myGroups3.count() );

      for ( int i = 0; i < myGroups3.count(); i++ )
        g3[i] = myGroups3[i];
      if ( toCreateElemGroup || toCreateNodeGroup ) {
        SMESH::ListOfGroups_var aNewGroups = 
          aMeshEditor->DoubleNodeElemGroups2New( g1.in(), g2.in(), g3.in(),
                                                 toCreateElemGroup, toCreateNodeGroup );
        result = ( aNewGroups[ !toCreateElemGroup ].in() );
      }
      else {
        result = aMeshEditor->DoubleNodeElemGroups( g1.in(), g2.in(), g3.in() );
      }
      break;
    }
    case 2:
    {
      result = true;
      QString groupName;
      if ( toCreateElemGroup )
        groupName = SMESH::UniqueName("DoubleElements");
      for ( int i = 0; i < myGroups1.count(); i++ )
      {
        SMESH::SMESH_Group_var group =
          aMeshEditor->DoubleElements( myGroups1[i], groupName.toLatin1().data() );
        if ( group->_is_nil() )
        {
          if ( toCreateElemGroup )
            result = false;
        }
        else
        {
          if ( _PTR(SObject) so = SMESH::FindSObject( group ))
            anEntryList.append( so->GetID().c_str() );
        }
      }
      break;
    }
    case 3:
    {
      bool createJointElems = myCheckBox1->isChecked();
      bool onAllBoundaries  = myCheckBox2->isChecked();

      SMESH::ListOfGroups_var g1 = new SMESH::ListOfGroups();
      g1->length( myGroups1.count() );
      for ( int i = 0; i < myGroups1.count(); i++ )
        g1[i] = myGroups1[i];

      result = aMeshEditor->DoubleNodesOnGroupBoundaries( g1.in(), createJointElems, onAllBoundaries );

      break;
    }
    } // switch( operationMode )
  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch ( const std::exception& exc ) {
    INFOS( "Follow exception was cought:\n\t" << exc.what() );
  }
  catch (...) {
    INFOS( "Unknown exception was cought !!!" );
  }

  if (!result) {
    SUIT_MessageBox::warning(this,
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_OPERATION_FAILED"));
    return false;
  }

  // Update GUI
  mySelectionMgr->clearSelected();
  SMESH::UpdateView();
  SMESHGUI::Modified();
  mySMESHGUI->updateObjBrowser(true);

  if ( !anEntryList.isEmpty())
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ))
      anApp->browseObjects( anEntryList, true, false );

  // Reinitialize the dialog
  Init();

  return true;
}

/*!
  \brief SLOT called to apply changes and close the dialog.
*/
void SMESHGUI_DuplicateNodesDlg::onOk()
{
  if (onApply())
    reject();
}

/*!
  \brief SLOT called to close the dialog.
*/
void SMESHGUI_DuplicateNodesDlg::reject()
{
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  mySelectionMgr->clearFilters();
  QDialog::reject();
}

/*!
  \brief  SLOT called when selection changed.
*/
void SMESHGUI_DuplicateNodesDlg::onSelectionChanged()
{
  if ( myBusy || !isEnabled() ) return;
  
  int operationMode = myGroupConstructors->checkedId();

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects( aList );
  //int aNbSel = aList.Extent();

  QList<SMESH::SMESH_GroupBase_var> aGroups;

  SALOME_ListIteratorOfListIO anIter ( aList );
  bool ok = true;
  for ( ; anIter.More() && ok; anIter.Next()) {
    SMESH::SMESH_GroupBase_var aGroup = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>( anIter.Value() );
    // check group is selected
    ok = !CORBA::is_nil( aGroup );
    // check groups of the same mesh are selected
    if ( ok ) {
      SMESH::SMESH_Mesh_var aMesh1;
      if ( !aGroups.isEmpty() ) aMesh1 = aGroups[0]->GetMesh();
      SMESH::SMESH_Mesh_var aMesh2 = aGroup->GetMesh();
      ok = CORBA::is_nil( aMesh1 ) || aMesh1->_is_equivalent( aMesh2 );
    }
    // check group of proper type is selected
    if ( ok ) {
      SMESH::ElementType aGroupType = aGroup->GetType();
      switch ( operationMode ) {
      case 0:
        ok = ( myCurrentLineEdit == myLineEdit1 && aGroupType == SMESH::NODE ) ||
             ( myCurrentLineEdit == myLineEdit2 && aGroupType != SMESH::NODE );
        break;
      case 1:
        ok = ( myCurrentLineEdit == myLineEdit1 && ( aGroupType == SMESH::EDGE ||
                                                     aGroupType == SMESH::FACE ) ) ||
             ( myCurrentLineEdit == myLineEdit2 && aGroupType == SMESH::NODE )     ||
             ( myCurrentLineEdit == myLineEdit3 && aGroupType != SMESH::NODE );
        break;
      case 2:
        ok = ( aGroupType != SMESH::NODE );
        break;
      case 3:
        ok = ( aGroupType == SMESH::VOLUME || 
               aGroupType == SMESH::FACE );
        break;
      }
    }
    if ( ok ) aGroups << aGroup;
  }

  // Clear current field
  myCurrentLineEdit->clear();

  if ( ok && !aGroups.isEmpty() ) {
    if      ( myCurrentLineEdit == myLineEdit1 ) myGroups1 = aGroups;
    else if ( myCurrentLineEdit == myLineEdit2 ) myGroups2 = aGroups;
    else if ( myCurrentLineEdit == myLineEdit3 ) myGroups3 = aGroups;
    CORBA::String_var name = aGroups[0]->GetName();
    myCurrentLineEdit->setText( aGroups.count() == 1 ? QString(name).trimmed() : 
                                QObject::tr( "SMESH_OBJECTS_SELECTED" ).arg( aGroups.count() ) );
  }
  else {
    if      ( myCurrentLineEdit == myLineEdit1 ) myGroups1.clear();
    else if ( myCurrentLineEdit == myLineEdit2 ) myGroups2.clear();
    else if ( myCurrentLineEdit == myLineEdit3 ) myGroups3.clear();
    myCurrentLineEdit->clear();
  }
  // Enable/disable "Apply and Close" and "Apply" buttons
  updateButtons();
}

/*!
 * \brief Enable/disable "Apply and Close" and "Apply" buttons
 */
void SMESHGUI_DuplicateNodesDlg::updateButtons()
{
  bool isDataValid = isValid();
  myButtonOk->setEnabled( isDataValid );
  myButtonApply->setEnabled( isDataValid );
}

/*!
  \brief  SLOT called when selection button clicked.
*/
void SMESHGUI_DuplicateNodesDlg::onEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  
  // Set current field for edition
  if (send == mySelectButton1) {
    myCurrentLineEdit = myLineEdit1;
  } 
  else if (send == mySelectButton2) {
    myCurrentLineEdit = myLineEdit2;
  }
  else if (send == mySelectButton3) {
    myCurrentLineEdit = myLineEdit3;
  }
  
  myCurrentLineEdit->setFocus();
  onSelectionChanged();
}

/*!
  \brief Check if the input data is valid.
  \return \c true if the data is valid
*/
bool SMESHGUI_DuplicateNodesDlg::isValid()
{
  switch( myGroupConstructors->checkedId() )
  {
  case 1:  return ( !myGroups1.isEmpty() && !myGroups3.isEmpty()  );
  case 3:  return ( myGroups1.count() > ( myCheckBox2->isChecked() ? 0 : 1 ));
  default: return !myGroups1.isEmpty();
  }
  return false;
}

/*!
  \brief SLOT called when dialog shoud be deativated.
*/
void SMESHGUI_DuplicateNodesDlg::onDeactivate()
{
  if (isEnabled()) {
    mySelectionMgr->clearFilters();
    setEnabled(false);
    mySMESHGUI->ResetState();
    mySMESHGUI->SetActiveDialogBox(0);
  }
}

/*!
  \brief Receive dialog enter events.
  Activates the dialog when the mouse cursor enters.
  Reimplemented from QWidget class.
*/
void SMESHGUI_DuplicateNodesDlg::enterEvent (QEvent*)
{
  if ( !isEnabled() ) {
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
    mySMESHGUI->SetActiveDialogBox((QDialog*)this);
    
    // Set selection mode
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode(ActorSelection);
    mySelectionMgr->installFilter(new SMESH_TypeFilter (SMESH::GROUP));
  }
}

/*!
  \brief Receive key press events.
  Reimplemented from QWidget class.
*/
void SMESHGUI_DuplicateNodesDlg::keyPressEvent( QKeyEvent* e )
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
  \brief Show the dialog help page.
*/
void SMESHGUI_DuplicateNodesDlg::onHelp()
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
