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
// File   : SMESHGUI_MeshDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshDlg.h"

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>

// Qt includes
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTabWidget>
#include <QGroupBox>
#include <QToolButton>
#include <QComboBox>
#include <QMenu>
#include <QCursor>
#include <QPushButton>

#define SPACING 6
#define MARGIN  11

/*!
 * \brief Tab for tab widget containing controls for definition of 
 * algorithms and hypotheses
*/ 

//================================================================================
/*!
 * \brief Constructor
  * \param theParent - Parent widget for this tab
 * 
 * Makes tab's look and feel
 */
//================================================================================ 
SMESHGUI_MeshTab::SMESHGUI_MeshTab( QWidget* theParent )
  : QFrame( theParent )
{
  SUIT_ResourceMgr* aResMgr = SUIT_Session::session()->resourceMgr();
  QIcon aCreateIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_HYPO" ) ) );
  QIcon aEditIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_HYPO_EDIT" ) ) );
  
  // Algorifm
  QLabel* anAlgoLbl = new QLabel( tr( "ALGORITHM" ), this );
  myHyp[ Algo ] = new QComboBox( this );
  
  // Hypothesis
  QLabel* aHypLbl = new QLabel( tr( "HYPOTHESIS" ), this );
  myHyp[ MainHyp ] = new QComboBox( this );
  myHyp[ MainHyp ]->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myCreateHyp[ MainHyp ] = new QToolButton( this );
  myCreateHyp[ MainHyp ]->setIcon( aCreateIcon );
  myEditHyp[ MainHyp ] = new QToolButton( this );
  myEditHyp[ MainHyp ]->setIcon( aEditIcon );
  
  // Line
  QFrame* aLine = new QFrame( this );
  aLine->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  
  // Add. hypothesis
  QLabel* anAddHypLbl = new QLabel( tr( "ADD_HYPOTHESIS" ), this );
  myHyp[ AddHyp ] = new QComboBox( this );
  myHyp[ AddHyp ]->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myCreateHyp[ AddHyp ] = new QToolButton( this );
  myCreateHyp[ AddHyp ]->setIcon( aCreateIcon );
  myEditHyp[ AddHyp ] = new QToolButton( this );
  myEditHyp[ AddHyp ]->setIcon( aEditIcon );
  
  // Fill layout
  QGridLayout* aLay = new QGridLayout( this );
  aLay->setMargin( MARGIN );
  aLay->setSpacing( SPACING );

  aLay->addWidget( anAlgoLbl, 0, 0 );
  aLay->addWidget( myHyp[ Algo ], 0, 1 );
  aLay->addWidget( aHypLbl, 1, 0 );
  aLay->addWidget( myHyp[ MainHyp ], 1, 1 );
  aLay->addWidget( myCreateHyp[ MainHyp ], 1, 2 );
  aLay->addWidget( myEditHyp[ MainHyp ], 1, 3 );
  aLay->addWidget( aLine, 2, 0, 1, 4 );
  aLay->addWidget( anAddHypLbl, 3, 0 );
  aLay->addWidget( myHyp[ AddHyp ], 3, 1 );
  aLay->addWidget( myCreateHyp[ AddHyp ], 3, 2 );
  aLay->addWidget( myEditHyp[ AddHyp ], 3, 3 );
  aLay->addItem( new QSpacerItem( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding ), 4, 0 );
    
  // Connect signals and slots
  for ( int i = MainHyp; i <= AddHyp; i++ )
  {
    connect( myCreateHyp[ i ], SIGNAL( clicked() )       ,  SLOT( onCreateHyp() ) );
    connect( myEditHyp[ i ]  , SIGNAL( clicked() )       ,  SLOT( onEditHyp() ) );
    connect( myHyp[ i ]      , SIGNAL( activated( int ) ),  SLOT( onHyp( int ) ) );
  }
  connect( myHyp[ Algo ], SIGNAL( activated( int ) ), SLOT( onHyp( int ) ) );
  
  // Initialize controls
  
  setAvailableHyps( Algo, QStringList() );
  setAvailableHyps( MainHyp, QStringList() );
  setAvailableHyps( AddHyp, QStringList() );
}

//================================================================================
/*!
 * \brief Destructor
 */
//================================================================================ 
SMESHGUI_MeshTab::~SMESHGUI_MeshTab()
{
}

//================================================================================
/*!
 * \brief Sets available hypothesis or algorithms
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \param theHyps - list of available hypothesis names
 * 
 * Sets available main or additional hypothesis for this tab
 */
//================================================================================
void SMESHGUI_MeshTab::setAvailableHyps( const int theId, const QStringList& theHyps )
{
  myAvailableHyps[ theId ] = theHyps;

  bool enable = ! theHyps.isEmpty();
  if ( theId == Algo )
  {
    myHyp[ Algo ]->clear();
    myHyp[ Algo ]->addItem( tr( "NONE" ) );
    myHyp[ Algo ]->addItems( theHyps );
    myHyp[ Algo ]->setCurrentIndex( 0 );
  }
  else {
    myCreateHyp[ theId ]->setEnabled( enable );
    myEditHyp[ theId ]->setEnabled( false );
  }
  myHyp[ theId ]->setEnabled( enable );
}

//================================================================================
/*!
 * \brief Sets existing hypothesis
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \param theHyps - list of available hypothesis names
 * 
 * Sets existing main or additional hypothesis for this tab
 */
//================================================================================
void SMESHGUI_MeshTab::setExistingHyps( const int theId, const QStringList& theHyps )
{
  if ( theId != Algo )
  {
    myHyp[ theId ]->clear();
    myHyp[ theId ]->addItem( tr( "NONE" ) );
    myHyp[ theId ]->addItems( theHyps );
    myHyp[ theId ]->setCurrentIndex( 0 );
    myHyp[ theId ]->setEnabled( !theHyps.isEmpty() );
    myEditHyp[ theId ]->setEnabled( false );
  }
}

//================================================================================
/*!
 * \brief Adds hypothesis in combo box of available ones
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \param theHyp - name of hypothesis to be added
 * 
 * Adds hypothesis in combo box of available ones. This method is called by operation
 * after creation of new hypothesis.
 */
//================================================================================
void SMESHGUI_MeshTab::addHyp( const int theId, const QString& theHyp )
{
  myHyp[ theId ]->addItem( theHyp );
  myHyp[ theId ]->setCurrentIndex( myHyp[ theId ]->count() - 1 );
  myEditHyp[ theId ]->setEnabled( true );
  myHyp[ theId ]->setEnabled( true );
}

//================================================================================
/*!
 * \brief Renames hypothesis
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \param theIndex - index of hypothesis to be renamed
  * \param theNewName - new name of hypothesis to be renamed
 * 
 * Renames hypothesis
 */
//================================================================================
void SMESHGUI_MeshTab::renameHyp( const int theId, 
                                  const int theIndex, 
                                  const QString& theNewName )
{
  if ( theIndex > 0 && theIndex < myHyp[ theId ]->count() )
    myHyp[ theId ]->setItemText( theIndex, theNewName );
}                                  

//================================================================================
/*!
 * \brief Sets current hypothesis 
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \param theIndex - index of hypothesis to be set as current
 * 
 * Sets current hypothesis 
 */
//================================================================================
void SMESHGUI_MeshTab::setCurrentHyp( const int theId, const int theIndex )
{
  if ( theIndex >= 0 && theIndex < myHyp[ theId ]->count() )
  {
    myHyp[ theId ]->setCurrentIndex( theIndex );
    if ( myEditHyp[ theId ] )
      myEditHyp[ theId ]->setEnabled( theIndex > 0 );
  }
}

//================================================================================
/*!
 * \brief Gets current hypothesis
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \retval int - index of current hypothesis
 * 
 * Gets current hypothesis
 */
//================================================================================
int SMESHGUI_MeshTab::currentHyp( const int theId ) const
{
  return myHyp[ theId ]->currentIndex();
}

//================================================================================
/*!
 * \brief Emits createHyp( const int ) signal
 * 
 * SLOT called when "Create hypothesis" button clicked specifies sender and emits
 * createHyp( const int ) signal
 */
//================================================================================
void SMESHGUI_MeshTab::onCreateHyp()
{
  bool isMainHyp = sender() == myCreateHyp[ MainHyp ];

  QMenu aPopup( this );
  
  QStringList aHypNames = isMainHyp ? 
    myAvailableHyps[ MainHyp ] : myAvailableHyps[ AddHyp ];

  QList<QAction*> actions;
  for ( int i = 0, n = aHypNames.count(); i < n; i++ )
    actions.append( aPopup.addAction( aHypNames[ i ] ) );

  QAction* a = aPopup.exec( QCursor::pos() );
  if ( a )
    emit createHyp( isMainHyp ? MainHyp : AddHyp, actions.indexOf( a ) );
}

//================================================================================
/*!
 * \brief Emits editHyp( const int ) signal
 * 
 * SLOT called when "Edit hypothesis" button clicked specifies sender and emits
 * editHyp( const int ) signal
 */
//================================================================================
void SMESHGUI_MeshTab::onEditHyp()
{
  const QObject* aSender = sender();
  int aHypType = aSender == myEditHyp[ MainHyp ] ? MainHyp : AddHyp;
  emit editHyp( aHypType, myHyp[ aHypType ]->currentIndex() - 1 );  // - 1 because there is NONE on the top
}

//================================================================================
/*!
 * \brief Updates "Edit hypothesis" button state
 * 
 * SLOT called when current hypothesis changed. Disables "Edit hypothesis" button
 * if current hypothesis is <None>, enables otherwise.
 * If an algorithm changed, emits selectAlgo( theIndex ) signal
 */
//================================================================================
void SMESHGUI_MeshTab::onHyp( int theIndex )
{
  const QObject* aSender = sender();
  if ( aSender == myHyp[ Algo ] )
    emit selectAlgo( theIndex - 1 ); // - 1 because there is NONE on the top
  else {
    int anIndex = aSender == myHyp[ MainHyp ] ? MainHyp : AddHyp;
    myEditHyp[ anIndex ]->setEnabled( theIndex > 0 );
  }
}

//================================================================================
/*!
 * \brief Resets all tab fields
 *
 * Resets all tab fields
 */
//================================================================================  
void SMESHGUI_MeshTab::reset()
{
  for ( int i = Algo; i <= AddHyp; i++ )
  {
    myHyp[ i ]->setCurrentIndex( 0 );
    if ( myEditHyp[ i ] )
      myEditHyp[ i ]->setEnabled( false );
  }
}

/*!
 * \brief Dialog for mech creation or editing
 *
 *  This dialog is used for mech creation or editing. 
*/

//================================================================================
/*!
 * \brief Constructor
  * \param theToCreate - if this parameter is true then dialog is used for creation,
  * for editing otherwise
  * \param theIsMesh - if this parameter is true then dialog is used for mesh,
  * for sub-mesh otherwise
 * 
 * Makes dialog's look and feel
 */
//================================================================================
SMESHGUI_MeshDlg::SMESHGUI_MeshDlg( const bool theToCreate, const bool theIsMesh )
: SMESHGUI_Dialog( 0, false, true )
{
  // Create top controls

  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  // name 
  createObject( tr( "NAME" ), mainFrame(), Obj );
  setNameIndication( Obj, OneName );
  setReadOnly( Obj, false );
  // mesh
  createObject( tr( "MESH" ), mainFrame(), Mesh );
  // geometry
  createObject( tr( "GEOMETRY" ), mainFrame(), Geom );
  myGeomPopup = 0;
  
  // Create tab widget
  
  myTabWg = new QTabWidget( mainFrame() );
  myTabs[ Dim0D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabs[ Dim1D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabs[ Dim2D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabs[ Dim3D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabWg->addTab( myTabs[ Dim3D ], tr( "DIM_3D" ) );
  myTabWg->addTab( myTabs[ Dim2D ], tr( "DIM_2D" ) );
  myTabWg->addTab( myTabs[ Dim1D ], tr( "DIM_1D" ) );
  myTabWg->addTab( myTabs[ Dim0D ], tr( "DIM_0D" ) );

  // Hypotheses Sets
  myHypoSetButton = new QToolButton( mainFrame() );
  myHypoSetButton->setText( tr( "HYPOTHESES_SETS" ) );
  myHypoSetButton->setEnabled( false );
  myHypoSetButton->setSizePolicy( QSizePolicy::MinimumExpanding, 
                                  myHypoSetButton->sizePolicy().verticalPolicy() );
  
  // Fill layout
  QGridLayout* aLay = new QGridLayout( mainFrame() );
  aLay->setMargin( 0 );
  aLay->setSpacing( SPACING );

  aLay->addWidget( objectWg( Obj,  Label ),   0, 0 );
  aLay->addWidget( objectWg( Obj,  Btn ),     0, 1 );
  aLay->addWidget( objectWg( Obj,  Control ), 0, 2 );
  aLay->addWidget( objectWg( Mesh, Label ),   1, 0 );
  aLay->addWidget( objectWg( Mesh, Btn ),     1, 1 );
  aLay->addWidget( objectWg( Mesh, Control ), 1, 2 );
  aLay->addWidget( objectWg( Geom, Label ),   2, 0 );
  aLay->addWidget( objectWg( Geom, Btn ),     2, 1 );
  aLay->addWidget( objectWg( Geom, Control ), 2, 2 );
  aLay->addWidget( myTabWg,                   4, 0, 1, 3 );
  aLay->addWidget( myHypoSetButton,           5, 0, 1, 3 );
  aLay->setRowMinimumHeight( 3, 20 );

  // Disable controls if necessary
  setObjectShown( Mesh, false );
  if ( theToCreate )
  {
    setWindowTitle( tr( "CREATE_MESH" ) );
    objectWg( Obj, Btn )->hide();
    if ( theIsMesh )
      setWindowTitle( tr( "CREATE_MESH" ) );
    else
    {
      setWindowTitle( tr( "CREATE_SUBMESH" ) );
      setObjectShown( Mesh, true );
    }
  }
  else
  {
    setWindowTitle( tr( "EDIT_MESH_SUBMESH" ) );
    objectWg( Mesh, Btn )->hide();
    objectWg( Geom, Btn )->hide();
  }
}

SMESHGUI_MeshDlg::~SMESHGUI_MeshDlg()
{
}

//================================================================================
/*!
 * \brief Gets tab with given id
  * \param theId - Tab identifier. Possible values are in "Dimensions" enumeration
  * \retval SMESHGUI_MeshTab* - pointer to the tab or null if given parameter is 
  * invalid
 * 
 * Gets tab containing controls for definition of algorithms and AddHypotheses
 */
//================================================================================
SMESHGUI_MeshTab* SMESHGUI_MeshDlg::tab( const int theId ) const
{
  return ( theId >= Dim0D && theId <= Dim3D ? myTabs[ theId ] : 0 );
}

//================================================================================
/*!
 * \brief Resets all dialog fields
 */
//================================================================================  
void SMESHGUI_MeshDlg::reset()
{
  clearSelection();
  myTabs[ Dim0D ]->reset();
  myTabs[ Dim1D ]->reset();
  myTabs[ Dim2D ]->reset();
  myTabs[ Dim3D ]->reset();
}

//================================================================================
/*!
 * \brief Sets curent tab
 */
//================================================================================    
void SMESHGUI_MeshDlg::setCurrentTab( const int theId  )
{
  myTabWg->setCurrentIndex( Dim3D - theId );
}

//================================================================================
/*!
 * \brief Enable/disable tabs
  * \param int - maximum possible dimention
 */
//================================================================================

void SMESHGUI_MeshDlg::setMaxHypoDim( const int maxDim )
{
  const int DIM = maxDim;
  for ( int dim = Dim0D; dim <= Dim3D; ++dim ) {
    bool enable = ( dim <= DIM );
    if ( !enable ) {
      myTabs[ dim ]->reset();
      disableTab( dim );
    }
    else {
      enableTab( dim );
    }
  }
  // deselect desabled tab
  if ( !myTabWg->isTabEnabled( myTabWg->currentIndex() ) )
    setCurrentTab( DIM );
}

//================================================================================
/*!
 * \brief Sets list of available Sets of Hypotheses
 */
//================================================================================

void SMESHGUI_MeshDlg::setHypoSets( const QStringList& theSets )
{
  QMenu* aHypoSetPopup = myHypoSetButton->menu();
  if ( !aHypoSetPopup ) {
    aHypoSetPopup = new QMenu( myHypoSetButton );
    connect( aHypoSetPopup, SIGNAL( triggered( QAction* ) ), SLOT( onHypoSetPopup( QAction* ) ) );
    myHypoSetButton->setMenu( aHypoSetPopup );
    myHypoSetButton->setPopupMode( QToolButton::InstantPopup );
  }
  aHypoSetPopup->clear();
  for ( int i = 0, n = theSets.count(); i < n; i++ ) {
    aHypoSetPopup->addAction( theSets[ i ] );
  }
  myHypoSetButton->setEnabled( !aHypoSetPopup->isEmpty() );
}

//================================================================================
/*!
 * \brief Emits hypoSet signal
 * 
 * SLOT is called when a hypotheses set is selected. Emits hypoSet
 * signal to notify operation about this event
 */
//================================================================================

void SMESHGUI_MeshDlg::onHypoSetPopup( QAction* a )
{
  emit hypoSet( a->text() );
}
  
//================================================================================
/*!
 * \brief Enable showing of the popup when Geometry selection btn is clicked
  * \param enable - true to enable
 */
//================================================================================

enum { DIRECT_GEOM_INDEX = 0, GEOM_BY_MESH_INDEX };

void SMESHGUI_MeshDlg::setGeomPopupEnabled( const bool enable )
{
  if ( QToolButton* selBtn = qobject_cast<QToolButton*>( objectWg( Geom, Btn )))
  {
    if ( enable ) {
      if ( ! myGeomPopup ) {
        myGeomPopup = new QMenu();
        myGeomPopup->addAction( tr("DIRECT_GEOM_SELECTION") )->setData( DIRECT_GEOM_INDEX );
        myGeomPopup->addAction( tr("GEOM_BY_MESH_ELEM_SELECTION") )->setData( GEOM_BY_MESH_INDEX );
        connect( myGeomPopup, SIGNAL( triggered( QAction* ) ), SLOT( onGeomPopup( QAction* ) ) );
        connect( selBtn, SIGNAL( toggled(bool) ), this, SLOT( onGeomSelectionButton(bool) ));
      }
    }
    else {
      disconnect( selBtn, SIGNAL( toggled(bool) ), this, SLOT( onGeomSelectionButton(bool) ));
      if ( myGeomPopup ) {
        delete myGeomPopup;
        myGeomPopup = 0;
      }
    }
  }
}


//================================================================================
/*!
 * \brief Disable tab
 * \param int - tab ID
 */
//================================================================================
void SMESHGUI_MeshDlg::disableTab(const int theTabId) {
  myTabWg->setTabEnabled( myTabWg->indexOf( myTabs[ theTabId ] ), false );
  if ( theTabId == Dim3D ) myHypoSetButton->setEnabled( false );
}

//================================================================================
/*!
 * \brief Enable tabs
 * \param int - tab ID
 */
//================================================================================
void SMESHGUI_MeshDlg::enableTab(const int theTabId) {
  myTabWg->setTabEnabled( myTabWg->indexOf( myTabs[ theTabId ] ), true );
  if ( theTabId == Dim3D ) {
    QMenu* aHypoSetPopup = myHypoSetButton->menu();
    myHypoSetButton->setEnabled( aHypoSetPopup && !aHypoSetPopup->actions().isEmpty() );
  }
}

//================================================================================
/*!
 * \brief Check if tab enabled
 * \param int - tab ID
 */
//================================================================================
bool SMESHGUI_MeshDlg::isTabEnabled(const int theTabId) const {
  return myTabWg->isTabEnabled( myTabWg->indexOf( myTabs[ theTabId ] ) );
}

void SMESHGUI_MeshDlg::onGeomSelectionButton(bool isBtnOn)
{
  if ( myGeomPopup && isBtnOn )
    myGeomPopup->exec( QCursor::pos() );
}

void SMESHGUI_MeshDlg::onGeomPopup( QAction* a )
{
  emit geomSelectionByMesh( a->data().toInt() == GEOM_BY_MESH_INDEX );
}

int SMESHGUI_MeshDlg::getActiveObject()
{
  for (int i = 0; i < 3; ++i )
    if ( isObjectShown( i ) &&
         (( QToolButton* )objectWg( i, Btn ))->isChecked())
      return i;
  return -1;
}
