// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
/**
*  SMESH SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_MeshDlg.cxx
*  Author : Sergey LITONIN
*  Module : SMESH
*/

#include "SMESHGUI_MeshDlg.h"

#include <SUIT_Session.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpushbutton.h>

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
: QFrame( theParent ),
  myPopup( 0 )
{
  SUIT_ResourceMgr* aResMgr = SUIT_Session::session()->resourceMgr();
  QIconSet aCreateIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_HYPO" ) ) );
  QIconSet aEditIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_HYPO_EDIT" ) ) );
  
  // Algorifm
  QLabel* anAlgoLbl = new QLabel( tr( "ALGORITHM" ), this );
  myHyp[ Algo ] = new QComboBox( this );
  
  // Hypothesis
  QLabel* aHypLbl = new QLabel( tr( "HYPOTHESIS" ), this );
  myHyp[ MainHyp ] = new QComboBox( this );
  myHyp[ MainHyp ]->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myCreateHyp[ MainHyp ] = new QToolButton( this );
  myCreateHyp[ MainHyp ]->setIconSet( aCreateIcon );
  myEditHyp[ MainHyp ] = new QToolButton( this );
  myEditHyp[ MainHyp ]->setIconSet( aEditIcon );
  
  // Line
  QFrame* aLine = new QFrame( this );
  aLine->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  
  // Add. hypothesis
  QLabel* anAddHypLbl = new QLabel( tr( "ADD_HYPOTHESIS" ), this );
  myHyp[ AddHyp ] = new QComboBox( this );
  myHyp[ AddHyp ]->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myCreateHyp[ AddHyp ] = new QToolButton( this );
  myCreateHyp[ AddHyp ]->setIconSet( aCreateIcon );
  myEditHyp[ AddHyp ] = new QToolButton( this );
  myEditHyp[ AddHyp ]->setIconSet( aEditIcon );
  
  // Fill layout
  QGridLayout* aLay = new QGridLayout( this, 5, 4, 5, 5 );
  aLay->addWidget( anAlgoLbl, 0, 0 );
  aLay->addWidget( myHyp[ Algo ], 0, 1 );
  aLay->addWidget( aHypLbl, 1, 0 );
  aLay->addWidget( myHyp[ MainHyp ], 1, 1 );
  aLay->addWidget( myCreateHyp[ MainHyp ], 1, 2 );
  aLay->addWidget( myEditHyp[ MainHyp ], 1, 3 );
  aLay->addMultiCellWidget( aLine, 2, 2, 0, 3 );
  aLay->addWidget( anAddHypLbl, 3, 0 );
  aLay->addWidget( myHyp[ AddHyp ], 3, 1 );
  aLay->addWidget( myCreateHyp[ AddHyp ], 3, 2 );
  aLay->addWidget( myEditHyp[ AddHyp ], 3, 3 );
  aLay->addItem( new QSpacerItem( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding ), 4, 0 );
    
  // Connect signals and slots
  for ( int i = MainHyp; i <= AddHyp; i++ )
  {
    connect( myCreateHyp[ i ], SIGNAL( clicked() ), SLOT( onCreateHyp() ) );
    connect( myEditHyp[ i ], SIGNAL( clicked() ), SLOT( onEditHyp() ) );
    connect( myHyp[ i ], SIGNAL( activated( int ) ), SLOT( onHyp( int ) ) );
  }
  
  // Initialize controls
  
  setAvailableHyps( Algo, QStringList() );
  setAvailableHyps( MainHyp, QStringList() );
  setAvailableHyps( AddHyp, QStringList() );
}

SMESHGUI_MeshTab::~SMESHGUI_MeshTab()
{
  if ( myPopup )
    delete myPopup;
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
  if ( theId == Algo )
  {
    myHyp[ Algo ]->clear();
    myHyp[ Algo ]->insertItem( tr( "NONE" ) );
    myHyp[ Algo ]->insertStringList( theHyps );
    myHyp[ Algo ]->setCurrentItem( 0 );
  }
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
    myHyp[ theId ]->insertItem( tr( "NONE" ) );
    myHyp[ theId ]->insertStringList( theHyps );
    myHyp[ theId ]->setCurrentItem( 0 );
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
  myHyp[ theId ]->insertItem( theHyp );
  myHyp[ theId ]->setCurrentItem( myHyp[ theId ]->count() - 1 );
  myEditHyp[ theId ]->setEnabled( true );
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
    myHyp[ theId ]->changeItem( theNewName, theIndex );
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
    myHyp[ theId ]->setCurrentItem( theIndex );
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
  return myHyp[ theId ]->currentItem();
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
  const QObject* aSender = sender();
    
  if ( !myPopup )
  {
    myPopup = new QPopupMenu( 0 );
    connect( myPopup, SIGNAL( activated( int ) ), SLOT( onPopupItem( int ) ) );
  }
  
  QStringList aHypNames;
  if ( aSender == myCreateHyp[ MainHyp ] )
  {
    aHypNames = myAvailableHyps[ MainHyp ];
    myPopup->setName( "MainHypPopup" );
  }
  else
  {
    aHypNames = myAvailableHyps[ AddHyp ];
    myPopup->setName( "AddHypPopup" );
  }
 
  myPopup->clear();
  for ( int i = 0, n = aHypNames.count(); i < n; i++ )
    myPopup->insertItem( aHypNames[ i ], i );

  myPopup->exec( QCursor::pos() );
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
  emit editHyp( aHypType, myHyp[ aHypType ]->currentItem() );
}

//================================================================================
/*!
 * \brief Updates "Edit hypothesis" button state
 * 
 * SLOT called when current hypothesis changed disables "Edit hypothesis" button
 * if current hypothesis is <None>, enables otherwise
 */
//================================================================================
void SMESHGUI_MeshTab::onHyp( int theIndex )
{
  const QObject* aSender = sender();
  int anIndex = aSender == myHyp[ MainHyp ] ? MainHyp : AddHyp;
  myEditHyp[ anIndex ]->setEnabled( theIndex > 0 );
}

//================================================================================
/*!
 * \brief Emits createHyp signal
 * 
 * SLOT called when item of popup for hypothesis creation is activated. Emits 
 * createHyp signal to notify operation obout this event
 */
//================================================================================
void SMESHGUI_MeshTab::onPopupItem( int theId )
{
  const QObject* aSender = sender();
  if ( aSender )
    emit createHyp( strcmp( aSender->name(),  "MainHypPopup" ) == 0 ? MainHyp : AddHyp, theId );
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
    myHyp[ i ]->setCurrentItem( 0 );
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
  
  QGroupBox* aGrp = new QGroupBox( 3, Qt::Horizontal, mainFrame() );
  aGrp->setFrameStyle( QFrame::NoFrame );
  aGrp->setInsideMargin( 0 );
  // name 
  createObject( tr( "NAME" ), aGrp, Obj );
  setNameIndication( Obj, OneName );
  setReadOnly( Obj, false );
  // mesh
  createObject( tr( "MESH" ), aGrp, Mesh );
  // geometry
  createObject( tr( "GEOMETRY" ), aGrp, Geom );
  myGeomPopup = 0;
  
  // Create tab widget
  
  myTabWg = new QTabWidget( mainFrame() );
  myTabs[ Dim1D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabs[ Dim2D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabs[ Dim3D ] = new SMESHGUI_MeshTab( myTabWg );
  myTabWg->addTab( myTabs[ Dim1D ], tr( "DIM_1D" ) );
  myTabWg->addTab( myTabs[ Dim2D ], tr( "DIM_2D" ) );
  myTabWg->addTab( myTabs[ Dim3D ], tr( "DIM_3D" ) );

  // Hypotheses Sets
  myHypoSetPopup = new QPopupMenu();
  QButton* aHypoSetButton = new QPushButton( mainFrame(), "aHypoSetButton");
  aHypoSetButton->setText( tr( "HYPOTHESES_SETS" ) );
  
  // Fill layout
  QVBoxLayout* aLay = new QVBoxLayout( mainFrame(), 0, 5 );
  aLay->addWidget( aGrp );
  aLay->addItem( new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
  aLay->addWidget( myTabWg );
  aLay->addWidget( aHypoSetButton );

  // Disable controls if necessary
  setObjectShown( Mesh, false );
  if ( theToCreate )
  {
    setCaption( tr( "CREATE_MESH" ) );
    objectWg( Obj, Btn )->hide();
    if ( theIsMesh )
      setCaption( tr( "CREATE_MESH" ) );
    else
    {
      setCaption( tr( "CREATE_SUBMESH" ) );
      setObjectShown( Mesh, true );
    }
  }
  else
  {
    setCaption( tr( "EDIT_MESH_SUBMESH" ) );
    objectWg( Mesh, Btn )->hide();
    objectWg( Geom, Btn )->hide();
  }

  // Connect signals and slots
  connect( aHypoSetButton, SIGNAL( clicked() ), SLOT( onHypoSetButton() ) );
  connect( myHypoSetPopup, SIGNAL( activated( int ) ), SLOT( onHypoSetPopup( int ) ) );
}

SMESHGUI_MeshDlg::~SMESHGUI_MeshDlg()
{
  if ( myHypoSetPopup )
    delete myHypoSetPopup;
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
  return ( theId >= Dim1D && theId <= Dim3D ? myTabs[ theId ] : 0 );
}

//================================================================================
/*!
 * \brief Resets all dialog fields
 */
//================================================================================  
void SMESHGUI_MeshDlg::reset()
{
  clearSelection();
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
  myTabWg->setCurrentPage( theId );
}

//================================================================================
/*!
 * \brief Enable/disable tabs
  * \param int - maximum possible dimention
 */
//================================================================================

void SMESHGUI_MeshDlg::setMaxHypoDim( const int maxDim )
{
  for ( int i = Dim1D; i <= Dim3D; ++i ) {
    int dim = i + 1;
    bool enable = ( dim <= maxDim );
    if ( !enable )
      myTabs[ i ]->reset();
    myTabWg->setTabEnabled( myTabs[ i ], enable );
  }
}

//================================================================================
/*!
 * \brief Sets list of available Sets of Hypotheses
 */
//================================================================================

void SMESHGUI_MeshDlg::setHypoSets( const QStringList& theSets )
{
  myHypoSetPopup->clear();
  for ( int i = 0, n = theSets.count(); i < n; i++ ) {
    myHypoSetPopup->insertItem( theSets[ i ], i );
  }
}

//================================================================================
/*!
 * \brief Emits hypoSet signal
 * 
 * SLOT is called when a hypotheses set is selected. Emits hypoSet
 * signal to notify operation about this event
 */
//================================================================================

void SMESHGUI_MeshDlg::onHypoSetPopup( int theIndex )
{
  emit hypoSet( myHypoSetPopup->text( theIndex ));
}
  
//================================================================================
/*!
 * \brief Shows myHypoSetPopup
 */
//================================================================================

void SMESHGUI_MeshDlg::onHypoSetButton()
{
  myHypoSetPopup->exec( QCursor::pos() );
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
  if ( QButton* selBtn = dynamic_cast<QButton*>( objectWg( Geom, Btn )))
  {
    disconnect( selBtn, SIGNAL( toggled(bool) ), this, SLOT( onGeomSelectionButton(bool) ));
    if ( enable ) {
      if ( ! myGeomPopup ) {
        myGeomPopup = new QPopupMenu();
        myGeomPopup->insertItem( tr("DIRECT_GEOM_SELECTION"), DIRECT_GEOM_INDEX );
        myGeomPopup->insertItem( tr("GEOM_BY_MESH_ELEM_SELECTION"), GEOM_BY_MESH_INDEX );
        connect( myGeomPopup, SIGNAL( activated( int ) ), SLOT( onGeomPopup( int ) ) );
      }
      connect( selBtn, SIGNAL( toggled(bool) ), this, SLOT( onGeomSelectionButton(bool) ));
    }
  }
}

void SMESHGUI_MeshDlg::onGeomSelectionButton(bool isBtnOn)
{
  if ( myGeomPopup && isBtnOn )
    myGeomPopup->exec( QCursor::pos() );
}

void SMESHGUI_MeshDlg::onGeomPopup( int index )
{
  emit geomSelectionByMesh( index == GEOM_BY_MESH_INDEX );
}
