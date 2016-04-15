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
// File   : SMESHGUI_GroupOpDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_GroupOpDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_TypeFilter.hxx"
#include <SMESH_ActorUtils.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <QtxColorButton.h>
#include <SALOMEDSClient_SObject.hxx>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SVTK_Selection.h>
#include <SVTK_ViewWindow.h>

// Qt includes
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#define SPACING 6
#define MARGIN  11

/*!
 *  Class       : SMESHGUI_GroupOpDlg
 *  Description : Perform boolean operations on groups
 */

/*!
  \brief Constructor
  \param theModule pointer on module instance
*/
SMESHGUI_GroupOpDlg::SMESHGUI_GroupOpDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule ),
    mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
    myIsApplyAndClose( false )
{
  setModal(false);

  mySelector = (SMESH::GetViewWindow( mySMESHGUI ))->GetSelector();

  QVBoxLayout* aDlgLay = new QVBoxLayout (this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  QWidget* aMainFrame = createMainFrame  (this);
  QWidget* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  Init();
}

/*!
  \brief Creates frame containing dialog's input fields
  \param theParent parent widget
  \return pointer on created widget
*/
QWidget* SMESHGUI_GroupOpDlg::createMainFrame( QWidget* theParent )
{
  QWidget* aMainGrp = new QWidget(theParent);
  QVBoxLayout* aLay = new QVBoxLayout(aMainGrp);
  aLay->setMargin(0);
  aLay->setSpacing(SPACING);
  
  // ------------------------------------------------------
  QGroupBox* aNameGrp = new QGroupBox(tr("RESULT"), aMainGrp);
  QHBoxLayout* aNameGrpLayout = new QHBoxLayout(aNameGrp);
  aNameGrpLayout->setMargin(MARGIN);
  aNameGrpLayout->setSpacing(SPACING);

  QLabel* aNameLab = new QLabel(tr("RESULT_NAME"), aNameGrp);
  myNameEdit = new QLineEdit(aNameGrp);

  aNameGrpLayout->addWidget(aNameLab);
  aNameGrpLayout->addWidget(myNameEdit);

  // ------------------------------------------------------
  myArgGrp = new QGroupBox(tr("ARGUMENTS"), aMainGrp);


  // ------------------------------------------------------
  
  QGroupBox* aColorBox = new QGroupBox(tr( "SMESH_SET_COLOR" ), this);
  QHBoxLayout* aColorBoxLayout = new QHBoxLayout(aColorBox);
  aColorBoxLayout->setMargin(MARGIN);
  aColorBoxLayout->setSpacing(SPACING);

  QLabel* aColorLab = new QLabel(tr( "SMESH_CHECK_COLOR" ), aColorBox );
  myColorBtn = new QtxColorButton(aColorBox);
  myColorBtn->setSizePolicy( QSizePolicy::MinimumExpanding, 
                             myColorBtn->sizePolicy().verticalPolicy() );

  aColorBoxLayout->addWidget(aColorLab);
  aColorBoxLayout->addWidget(myColorBtn);

  // ------------------------------------------------------
  aLay->addWidget( aNameGrp );
  aLay->addWidget( myArgGrp );
  aLay->addWidget( aColorBox );

  return aMainGrp;
}

/*!
  \brief Gets pointer on arguments group box
  \return pointer on arguments group box
*/
QGroupBox* SMESHGUI_GroupOpDlg::getArgGrp() const
{
  return myArgGrp;
}

/*!
  \brief Sets help file name
  \param theFName help file name
*/
void SMESHGUI_GroupOpDlg::setHelpFileName( const QString& theFName )
{
  myHelpFileName = theFName;
}

/*!
  \brief Gets pointer to the module instance
  \return pointer to the module instance
*/
SMESHGUI* SMESHGUI_GroupOpDlg::getSMESHGUI() const
{
  return mySMESHGUI;
}

/*!
  \brief Create frame containing buttons
  \param theParent parent widget
  \return pointer to the created frame
*/
QWidget* SMESHGUI_GroupOpDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aFrame = new QGroupBox(theParent);

  myOkBtn    = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aFrame);
  myApplyBtn = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn  = new QPushButton(tr("SMESH_BUT_HELP"),  aFrame);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addSpacing(10);
  aLay->addWidget(myApplyBtn);
  aLay->addSpacing(10);
  aLay->addStretch();
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  // connect signals and slots
  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(reject()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  return aFrame;
}

/*!
  \brief Destructor
*/
SMESHGUI_GroupOpDlg::~SMESHGUI_GroupOpDlg()
{
}

/*!
  \brief Init dialog fields, connect signals and slots, show dialog
*/
void SMESHGUI_GroupOpDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  
  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()), SLOT(onCloseView()));

  // set selection mode
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  mySelectionMgr->installFilter(new SMESH_TypeFilter (SMESH::GROUP));

  setDefaultGroupColor();
}

/*!
  \brief Validate list of groups used for operation. Checks whether they corresponds 
  to the same face and have one type
  \param theListGrp input list of groups 
  \return TRUE if groups are valid, FALSE otherwise
*/
bool SMESHGUI_GroupOpDlg::isValid( const QList<SMESH::SMESH_GroupBase_var>& theListGrp )
{
  if ( theListGrp.isEmpty() )
  {
    SUIT_MessageBox::information( this, tr("SMESH_INSUFFICIENT_DATA"),
                                  tr("INCORRECT_ARGUMENTS") );
    return false;
  }

  int aMeshId = -1, aGrpType = -1;
  QList<SMESH::SMESH_GroupBase_var>::const_iterator anIter;
  for ( anIter = theListGrp.begin(); anIter != theListGrp.end(); ++anIter )
  {
    SMESH::SMESH_GroupBase_var aGrp = *anIter;
    if ( CORBA::is_nil( aGrp ) )
      continue; // nonsence

    SMESH::SMESH_Mesh_var aMesh = aGrp->GetMesh();
    if ( CORBA::is_nil( aMesh ) )
      continue;

    // mesh id
    int aCurrId = aMesh->GetId();
    if ( aMeshId == -1 )
      aMeshId = aCurrId;
    else 
    {
      if ( aMeshId != aCurrId )
      {
        aMeshId = -1; // different meshes
        break;
      }
    }

    // group type
    int aCurrType = aGrp->GetType();
    if ( aGrpType == -1 )
      aGrpType = aCurrType;
    else 
    {
      if ( aGrpType != aCurrType )
      {
        aGrpType = -1; // different types
        break;
      }
    }

  }

  if ( aMeshId == -1 )
  {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
                                 tr("DIFF_MESHES"));
    return false;
  }

  if ( aGrpType == -1 ) 
  {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
                                 tr("DIFF_TYPES"));
    return false;
  }

  return true;
}

/*!
  \brief SLOT called when "Ok" button pressed performs operation and closes dialog box
*/
void SMESHGUI_GroupOpDlg::onOk()
{
  SUIT_OverrideCursor oc;
  setIsApplyAndClose( true );
  if ( onApply() )
    reject();
  setIsApplyAndClose( false );
}

/*!
  \brief SLOT called when dialog is closed
*/
void SMESHGUI_GroupOpDlg::reject()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect( mySelectionMgr, 0, this, 0 );
  disconnect( mySMESHGUI, 0, this, 0 );
  mySMESHGUI->ResetState();
  mySelectionMgr->clearFilters();
  reset();
  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_GroupOpDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_GroupOpDlg::onCloseView()
{
  onDeactivate();
  mySelector = 0;
}

/*!
  \brief SLOT called when "Help" button pressed shows "Help" page
*/
void SMESHGUI_GroupOpDlg::onHelp()
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

/*!
  \brief Gets list of currently selected groups from selection manager
  \param theOutList out list of groups
  \param theOutNames out list of group of group names
  \return TRUE if operation theOutList is not empty, FALSE otherwise
*/
bool SMESHGUI_GroupOpDlg::getSelectedGroups( QList<SMESH::SMESH_GroupBase_var>& theOutList, 
                                             QStringList& theOutNames )
{
  theOutList.clear();

  theOutList.clear();
  theOutNames.clear();

  SALOME_ListIO aListIO;
  mySelectionMgr->selectedObjects( aListIO );
  SALOME_ListIteratorOfListIO anIter ( aListIO );
  for ( ; anIter.More(); anIter.Next()) 
  {
    SMESH::SMESH_GroupBase_var aGroup =
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIter.Value());
    if ( !aGroup->_is_nil()) 
    {
      theOutList.append( aGroup );
      theOutNames.append( aGroup->GetName() );
    }
  }

  return theOutList.count() > 0;
}

/*!
  \brief Converts QT-list of group to the list acceptable by IDL interface
  \param theIn input list
  \return list acceptable by IDL interface
*/
SMESH::ListOfGroups* SMESHGUI_GroupOpDlg::convert( 
  const QList<SMESH::SMESH_GroupBase_var>& theIn )
{
  SMESH::ListOfGroups_var aList = new SMESH::ListOfGroups();
  aList->length( theIn.count() );

  QList<SMESH::SMESH_GroupBase_var>::const_iterator anIter = theIn.begin();
  for ( int i = 0; anIter != theIn.end(); ++anIter, ++i )
    aList[ i ] = *anIter;

  return aList._retn();
}

/*!
  \brief Get color to be assigned to group
  \return color to be assigned to group
*/
SALOMEDS::Color SMESHGUI_GroupOpDlg::getColor() const
{
  QColor aQColor = myColorBtn->color();

  SALOMEDS::Color aColor;
  aColor.R = (float)aQColor.red() / 255.0;
  aColor.G = (float)aQColor.green() / 255.0;
  aColor.B = (float)aQColor.blue() / 255.0;

  return aColor;
}

/*!
  \brief Set default color for group
*/
void SMESHGUI_GroupOpDlg::setDefaultGroupColor()
{
  myColorBtn->setColor( SMESH::GetColor( "SMESH", "default_grp_color", QColor( 255, 170, 0 ) ) );
}

/*!
  \brief SLOT, called when selection is changed. Current implementation does 
   nothing. The method should be redefined in derived classes to update 
   corresponding GUI controls
*/
void SMESHGUI_GroupOpDlg::onSelectionDone()
{
}

/*!
  \brief Calls onSelectionDone() and setVisible() method of base class
  \param visible the visible state of the dialog 
*/
void SMESHGUI_GroupOpDlg::setVisible( bool visible )
{
  if ( visible )
  {
    onSelectionDone();
    resize( minimumSizeHint().width(), sizeHint().height() );
  }
  QDialog::setVisible( visible );
}

/*!
  \brief SLOT called when dialog must be deativated
*/
void SMESHGUI_GroupOpDlg::onDeactivate()
{
  setEnabled(false);
  mySelectionMgr->clearFilters();
}

/*!
  \brief Event filter updates selection mode and selection filter. This virtual method 
  is redefined from the base class it is called when dialog obtains input focus
*/
void SMESHGUI_GroupOpDlg::enterEvent(QEvent*)
{
  mySMESHGUI->EmitSignalDeactivateDialog();
  setEnabled(true);
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
  if ( aViewWindow ) {
    aViewWindow->SetSelectionMode(ActorSelection);
    if (!mySelector)
      mySelector = aViewWindow->GetSelector();
  }
  mySelectionMgr->installFilter(new SMESH_TypeFilter (SMESH::GROUP));
}

/*!
  \brief Resets state of the dialog, initializes its fields with default value, etc. 
  Usually called by onApply() slot to reinitialize dialog  fields. This virtual method 
  should be redefined in derived class to update its own fileds
*/
void SMESHGUI_GroupOpDlg::reset()
{
  myNameEdit->setText("");
  myNameEdit->setFocus();
  setDefaultGroupColor();
}

/*!
  \brief Gets name of group to be created
  \return name of group to be created
  \sa setName()
*/
QString SMESHGUI_GroupOpDlg::getName() const
{
  return myNameEdit->text();
}

/*!
  \brief Sets name of group to be created
  \param theName name of group to be created
  \sa getName()
*/
void SMESHGUI_GroupOpDlg::setName( const QString& theName )
{
  myNameEdit->setText( theName );
}

/*!
  \brief Provides reaction on �F1� button pressing
  \param e  key press event
*/
void SMESHGUI_GroupOpDlg::keyPressEvent( QKeyEvent* e )
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
  \brief This virtual slot does nothing and should be redefined in derived classes
  \return return false;
*/
bool SMESHGUI_GroupOpDlg::onApply()
{
  return false;
}

/*!
  \brief Set value of the flag indicating that the dialog is
  accepted by Apply & Close button
  \param theFlag value of the flag
  \sa isApplyAndClose()
*/
void SMESHGUI_GroupOpDlg::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

/*!
  \brief Get value of the flag indicating that the dialog is
  accepted by Apply & Close button
  \return value of the flag
  \sa setApplyAndClose()
*/
bool SMESHGUI_GroupOpDlg::isApplyAndClose() const
{
  return myIsApplyAndClose;
}

// === === === === === === === === === === === === === === === === === === === === === 

/*!
  \brief Constructor
  \param theModule module
*/
SMESHGUI_UnionGroupsDlg::SMESHGUI_UnionGroupsDlg( SMESHGUI* theModule )
: SMESHGUI_GroupOpDlg( theModule )
{
  setWindowTitle(tr("UNION_OF_GROUPS"));
  setHelpFileName( "using_operations_on_groups_page.html#union_anchor" );

  QGroupBox* anArgGrp = getArgGrp();
  myListWg = new QListWidget( anArgGrp );

  QHBoxLayout* aLay = new QHBoxLayout( anArgGrp );
  aLay->addWidget( myListWg );
}

/*!
  \brief Destructor
*/
SMESHGUI_UnionGroupsDlg::~SMESHGUI_UnionGroupsDlg()
{
}

/*!
  \brief This virtual method redefined from the base class resets state 
  of the dialog, initializes its fields with default value, etc. 
*/
void SMESHGUI_UnionGroupsDlg::reset()
{
  SMESHGUI_GroupOpDlg::reset();
  myListWg->clear();
  myGroups.clear();
}

/*!
  \brief SLOT called when apply button is pressed performs operation
  \return TRUE if operation has been completed successfully, FALSE otherwise
*/
bool SMESHGUI_UnionGroupsDlg::onApply()
{
  if ( getSMESHGUI()->isActiveStudyLocked())
    return false;

  // Verify validity of group name
  if ( getName() == "" ) 
  {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
                                 SMESHGUI_GroupOpDlg::tr("EMPTY_NAME"));
    return false;
  }

  if ( !isValid( myGroups ) )
    return false;

  SMESH::SMESH_Mesh_var aMesh = myGroups.first()->GetMesh();
  QString aName = getName();
  
  bool aRes = false;
  QStringList anEntryList;
  try
  {
    SMESH::ListOfGroups_var aList = convert( myGroups );
    SMESH::SMESH_Group_var aNewGrp = 
      aMesh->UnionListOfGroups( aList, aName.toLatin1().constData() );
    if ( !CORBA::is_nil( aNewGrp ) )
    {
      aNewGrp->SetColor(  getColor() );
      if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aNewGrp ) )
        anEntryList.append( aSObject->GetID().c_str() );
      aRes = true;
    }
  }
  catch( ... )
  {
    aRes = false;
  }

  if ( aRes ) 
  {
    SMESHGUI::Modified();
    getSMESHGUI()->updateObjBrowser(true);
    reset();
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );
    return true;
  } 
  else 
  {
    SUIT_MessageBox::critical(this, tr("SMESH_ERROR"),
                              tr("SMESH_OPERATION_FAILED"));
    return false;
  }
}

/*!
  \brief SLOT, called when selection is changed, updates corresponding GUI controls
*/
void SMESHGUI_UnionGroupsDlg::onSelectionDone()
{
  QStringList aNames;
  getSelectedGroups( myGroups, aNames );
  myListWg->clear();
  myListWg->addItems( aNames );
}

// === === === === === === === === === === === === === === === === === === === === === 

/*!
  \brief Constructor
  \param theModule module
*/
SMESHGUI_IntersectGroupsDlg::SMESHGUI_IntersectGroupsDlg( SMESHGUI* theModule )
: SMESHGUI_GroupOpDlg( theModule )
{
  setWindowTitle(tr("INTERSECTION_OF_GROUPS"));
  setHelpFileName( "using_operations_on_groups_page.html#intersection_anchor" );

  QGroupBox* anArgGrp = getArgGrp();
  myListWg = new QListWidget( anArgGrp );

  QHBoxLayout* aLay = new QHBoxLayout( anArgGrp );
  aLay->addWidget( myListWg );
}

/*!
  \brief Destructor
*/
SMESHGUI_IntersectGroupsDlg::~SMESHGUI_IntersectGroupsDlg()
{
}

/*!
  \brief This virtual method redefined from the base class resets state 
  of the dialog, initializes its fields with default value, etc. 
*/
void SMESHGUI_IntersectGroupsDlg::reset()
{
  SMESHGUI_GroupOpDlg::reset();
  myListWg->clear();
  myGroups.clear();
}

/*!
  \brief SLOT called when apply button is pressed performs operation
  \return TRUE if operation has been completed successfully, FALSE otherwise
*/
bool SMESHGUI_IntersectGroupsDlg::onApply()
{
  if ( getSMESHGUI()->isActiveStudyLocked())
    return false;

  // Verify validity of group name
  if ( getName() == "" ) 
  {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
                                 SMESHGUI_GroupOpDlg::tr("EMPTY_NAME"));
    return false;
  }

  if ( !isValid( myGroups ) )
    return false;

  SMESH::SMESH_Mesh_var aMesh = myGroups.first()->GetMesh();
  QString aName = getName();
  
  bool aRes = false;
  QStringList anEntryList;
  try
  {
    SMESH::ListOfGroups_var aList = convert( myGroups );
    SMESH::SMESH_Group_var aNewGrp = 
      aMesh->IntersectListOfGroups( aList, aName.toLatin1().constData() );
    if ( !CORBA::is_nil( aNewGrp ) )
    {
      aNewGrp->SetColor(  getColor() );
      if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aNewGrp ) )
        anEntryList.append( aSObject->GetID().c_str() );
      aRes = true;
    }
  }
  catch( ... )
  {
    aRes = false;
  }

  if ( aRes ) 
  {
    SMESHGUI::Modified();
    getSMESHGUI()->updateObjBrowser(true);
    reset();
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );
    return true;
  } 
  else 
  {
    SUIT_MessageBox::critical(this, tr("SMESH_ERROR"),
                              tr("SMESH_OPERATION_FAILED"));
    return false;
  }
}

/*!
  \brief SLOT, called when selection is changed, updates corresponding GUI controls
*/
void SMESHGUI_IntersectGroupsDlg::onSelectionDone()
{
  QStringList aNames;
  getSelectedGroups( myGroups, aNames );
  myListWg->clear();
  myListWg->addItems( aNames );
}

// === === === === === === === === === === === === === === === === === === === === === 

/*!
  \brief Constructor
  \param theModule module
*/
SMESHGUI_CutGroupsDlg::SMESHGUI_CutGroupsDlg( SMESHGUI* theModule )
: SMESHGUI_GroupOpDlg( theModule )
{
  setWindowTitle(tr("CUT_OF_GROUPS"));
  setHelpFileName( "using_operations_on_groups_page.html#cut_anchor" );

  QGroupBox* anArgGrp = getArgGrp();

  QPixmap aPix (SMESH::GetResourceMgr( getSMESHGUI() )->loadPixmap("SMESH", tr("ICON_SELECT")));
  
  // frame 1
  QFrame* aFrame1 = new QFrame( anArgGrp );
  QLabel* aLbl1 = new QLabel( tr("MAIN_OBJECT"), aFrame1 );
  aLbl1->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myBtn1 = new QPushButton( aFrame1 );
  myBtn1->setIcon(aPix);
  myListWg1 = new QListWidget( aFrame1 );

  QGridLayout* aLay1 = new QGridLayout( aFrame1 );
  aLay1->setSpacing( SPACING );
  aLay1->addWidget( aLbl1, 0, 0 );
  aLay1->addWidget( myBtn1, 0, 1 );
  aLay1->addWidget( myListWg1, 1, 0, 1, 2 );
  //QSpacerItem* aHSpacer1 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  //aLay1->addItem( aHSpacer1, 0, 2 );


  // frame 2
  QFrame* aFrame2 = new QFrame( anArgGrp );
  QLabel* aLbl2 = new QLabel( tr("TOOL_OBJECT"), aFrame2 );
  aLbl2->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myBtn2 = new QPushButton( aFrame2 );
  myBtn2->setIcon(aPix);
  myListWg2 = new QListWidget( aFrame2 );

  QGridLayout* aLay2 = new QGridLayout( aFrame2 );
  aLay2->setSpacing( SPACING );
  aLay2->addWidget( aLbl2, 0, 0 );
  aLay2->addWidget( myBtn2, 0, 1 );
  aLay2->addWidget( myListWg2, 1, 0, 1, 2 );
  //QSpacerItem* aHSpacer2 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  //aLay2->addItem( aHSpacer2, 0, 2 );

  // create button group 

  QButtonGroup* aGrp = new QButtonGroup( anArgGrp );
  aGrp->addButton( myBtn1, 0 );
  aGrp->addButton( myBtn2, 1 );
  myBtn1->setCheckable( true );
  myBtn2->setCheckable( true );
  aGrp->setExclusive( true );
  myBtn1->setChecked( true );
  
  // fill layout
  QHBoxLayout* aLay = new QHBoxLayout( anArgGrp );
  aLay->setSpacing( SPACING );
  aLay->addWidget( aFrame1 );
  aLay->addWidget( aFrame2 );
}

/*!
  \brief Destructor
*/
SMESHGUI_CutGroupsDlg::~SMESHGUI_CutGroupsDlg()
{
}

/*!
  \brief This virtual method redefined from the base class resets state 
  of the dialog, initializes its fields with default value, etc. 
*/
void SMESHGUI_CutGroupsDlg::reset()
{
  SMESHGUI_GroupOpDlg::reset();

  myListWg1->clear();
  myGroups1.clear();

  myListWg2->clear();
  myGroups2.clear();
}

/*!
  \brief SLOT called when apply button is pressed performs operation
  \return TRUE if operation has been completed successfully, FALSE otherwise
*/
bool SMESHGUI_CutGroupsDlg::onApply()
{
  if ( getSMESHGUI()->isActiveStudyLocked())
    return false;

  // Verify validity of group name
  if ( getName() == "" ) 
  {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
                                 SMESHGUI_GroupOpDlg::tr("EMPTY_NAME"));
    return false;
  }

  if ( myGroups1.isEmpty() || myGroups2.isEmpty() )
  {
    SUIT_MessageBox::information( this, tr("SMESH_INSUFFICIENT_DATA"),
                                  SMESHGUI_GroupOpDlg::tr("INCORRECT_ARGUMENTS") );
    return false;
  }

  QList<SMESH::SMESH_GroupBase_var> aGroups = myGroups1;
  QList<SMESH::SMESH_GroupBase_var>::iterator anIter;
  for ( anIter = myGroups2.begin(); anIter != myGroups2.end(); ++anIter )
    aGroups.append( *anIter );

  if ( !isValid( aGroups ) )
    return false;

  SMESH::SMESH_Mesh_var aMesh = myGroups1.first()->GetMesh();
  QString aName = getName();
  
  bool aRes = false;
  QStringList anEntryList;
  try
  {
    SMESH::ListOfGroups_var aList1 = convert( myGroups1 );
    SMESH::ListOfGroups_var aList2 = convert( myGroups2 );
    SMESH::SMESH_Group_var aNewGrp = 
      aMesh->CutListOfGroups( aList1, aList2, aName.toLatin1().constData() );
    if ( !CORBA::is_nil( aNewGrp ) )
    {
      aNewGrp->SetColor(  getColor() );
      if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aNewGrp ) )
        anEntryList.append( aSObject->GetID().c_str() );
      aRes = true;
    }
  }
  catch( ... )
  {
    aRes = false;
  }

  if ( aRes ) 
  {
    SMESHGUI::Modified();
    getSMESHGUI()->updateObjBrowser(true);
    reset();
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );
    return true;
  } 
  else 
  {
    SUIT_MessageBox::critical(this, tr("SMESH_ERROR"),
                              tr("SMESH_OPERATION_FAILED"));
    return false;
  }
}

/*!
  \brief SLOT, called when selection is changed, updates corresponding GUI controls
*/
void SMESHGUI_CutGroupsDlg::onSelectionDone()
{
  QStringList aNames;
  if ( myBtn2->isChecked() )
  {
    getSelectedGroups( myGroups2, aNames );
    myListWg2->clear();
    myListWg2->addItems( aNames );
  }
  else 
  {
    getSelectedGroups( myGroups1, aNames );
    myListWg1->clear();
    myListWg1->addItems( aNames );
  }
}

// === === === === === === === === === === === === === === === === === === === === === 

/*!
  \brief Constructor
  \param theModule module
*/
SMESHGUI_DimGroupDlg::SMESHGUI_DimGroupDlg( SMESHGUI* theModule )
  : SMESHGUI_GroupOpDlg( theModule )
{
  setWindowTitle( tr( "CREATE_GROUP_OF_UNDERLYING_ELEMS" ) );
  setHelpFileName( "group_of_underlying_elements_page.html" );

  QGroupBox* anArgGrp = getArgGrp();

  QLabel* aTypeLbl = new QLabel( tr( "ELEMENTS_TYPE" ), anArgGrp );

  myTypeCombo = new QComboBox( anArgGrp );
  QStringList anItems;
  {
    anItems.append( tr( "MESH_NODE" ) );
    anItems.append( tr( "SMESH_EDGE" ) );
    anItems.append( tr( "SMESH_FACE" ) );
    anItems.append( tr( "SMESH_VOLUME" ) );
    anItems.append( tr( "SMESH_ELEM0D" ) );
    anItems.append( tr( "SMESH_BALL" ) );
  }
  myTypeCombo->addItems( anItems );
  myTypeCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QLabel* aNbNoLbl = new QLabel( tr( "NUMBER_OF_COMMON_NODES" ), anArgGrp );

  myNbNoCombo = new QComboBox( anArgGrp );
  anItems.clear();
  {
    anItems.append( tr( "ALL" ) );
    anItems.append( tr( "MAIN" ) );
    anItems.append( tr( "AT_LEAST_ONE" ) );
    anItems.append( tr( "MAJORITY" ) );
  }
  myNbNoCombo->addItems( anItems );
  myNbNoCombo->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myListWg = new QListWidget( anArgGrp );

  myUnderlOnlyChk = new QCheckBox( tr("UNDERLYING_ENTITIES_ONLY"), anArgGrp );
  myUnderlOnlyChk->setChecked( false );

  // layout
  QGridLayout* aLay = new QGridLayout( anArgGrp );
  aLay->setSpacing( SPACING );
  aLay->addWidget( aTypeLbl,        0, 0 );
  aLay->addWidget( myTypeCombo,     0, 1 );
  aLay->addWidget( aNbNoLbl,        1, 0 );
  aLay->addWidget( myNbNoCombo,     1, 1 );
  aLay->addWidget( myListWg,        2, 0, 1, 2 );
  aLay->addWidget( myUnderlOnlyChk, 3, 0 );
}

/*!
  \brief Destructor
*/
SMESHGUI_DimGroupDlg::~SMESHGUI_DimGroupDlg()
{
}

/*!
  \brief This virtual method redefined from the base class resets state 
  of the dialog, initializes its fields with default value, etc. 
*/
void SMESHGUI_DimGroupDlg::reset()
{
  SMESHGUI_GroupOpDlg::reset();
  myListWg->clear();
  myGroups.clear();
}

/*!
  \brief Gets elements type
  \return elements type
  \sa setElementType()
*/
SMESH::ElementType SMESHGUI_DimGroupDlg::getElementType() const
{
  return (SMESH::ElementType)( myTypeCombo->currentIndex() + 1 );
}

/*!
  \brief Sets elements type
  \param theElemType elements type
  \sa getElementType()
*/
void SMESHGUI_DimGroupDlg::setElementType( const SMESH::ElementType& theElemType )
{
  myTypeCombo->setCurrentIndex( theElemType - 1 );
}

/*!
  \brief SLOT called when apply button is pressed performs operation
  \return TRUE if operation has been completed successfully, FALSE otherwise
*/
bool SMESHGUI_DimGroupDlg::onApply()
{
  if ( getSMESHGUI()->isActiveStudyLocked())
    return false;

  // Verify validity of group name
  if ( getName() == "" ) 
  {
    SUIT_MessageBox::information(this, tr("SMESH_INSUFFICIENT_DATA"),
                                 SMESHGUI_GroupOpDlg::tr("EMPTY_NAME"));
    return false;
  }

  if ( !isValid( myGroups ) )
    return false;

  SMESH::SMESH_Mesh_var aMesh = myGroups.first()->GetMesh();
  QString aName = getName();
  
  bool aRes = false;
  QStringList anEntryList;
  try
  {
    SMESH::ListOfIDSources_var aList = new SMESH::ListOfIDSources();
    aList->length( myGroups.count() );
    QList<SMESH::SMESH_GroupBase_var>::const_iterator anIter = myGroups.begin();
    for ( int i = 0; anIter != myGroups.end(); ++anIter, ++i )
      aList[ i ] = SMESH::SMESH_IDSource::_narrow( *anIter );

    SMESH::ElementType anElemType = getElementType();
    SMESH::NB_COMMON_NODES_ENUM aNbCoNodes =
      (SMESH::NB_COMMON_NODES_ENUM) myNbNoCombo->currentIndex();

    SMESH::SMESH_Group_var aNewGrp =
      aMesh->CreateDimGroup( aList, anElemType, aName.toLatin1().constData(),
                             aNbCoNodes, myUnderlOnlyChk->isChecked() );
    if ( !CORBA::is_nil( aNewGrp ) )
    {
      aNewGrp->SetColor(  getColor() );
      if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aNewGrp ) )
        anEntryList.append( aSObject->GetID().c_str() );
      aRes = true;
    }
  }
  catch( ... )
  {
    aRes = false;
  }

  if ( aRes ) 
  {
    SMESHGUI::Modified();
    getSMESHGUI()->updateObjBrowser(true);
    reset();
    if( LightApp_Application* anApp =
        dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
      anApp->browseObjects( anEntryList, isApplyAndClose() );
    return true;
  } 
  else 
  {
    SUIT_MessageBox::critical(this, tr("SMESH_ERROR"),
                              tr("SMESH_OPERATION_FAILED"));
    return false;
  }
}

/*!
  \brief SLOT, called when selection is changed, updates corresponding GUI controls
*/
void SMESHGUI_DimGroupDlg::onSelectionDone()
{
  QStringList aNames;
  getSelectedGroups( myGroups, aNames );
  myListWg->clear();
  myListWg->addItems( aNames );
}
