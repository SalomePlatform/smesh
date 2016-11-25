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
// File   : SMESHGUI_MeshDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshDlg.h"

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <QtxMenu.h>

// Qt includes
#include <QComboBox>
#include <QCursor>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <Standard_Integer.hxx>

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
  QIcon aPlusIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_PLUS" ) ) );
  QIcon aMinusIcon( aResMgr->loadPixmap( "SMESH", tr( "ICON_MINUS" ) ) );
  
  // Algorifm
  QLabel* anAlgoLbl = new QLabel( tr( "ALGORITHM" ), this );
  myHypCombo[ Algo ] = new QComboBox( this );
  
  // Hypothesis
  QLabel* aHypLbl = new QLabel( tr( "HYPOTHESIS" ), this );
  myHypCombo[ MainHyp ] = new QComboBox( this );
  myHypCombo[ MainHyp ]->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myCreateHypBtn[ MainHyp ] = new QToolButton( this );
  myCreateHypBtn[ MainHyp ]->setIcon( aCreateIcon );
  myEditHypBtn[ MainHyp ] = new QToolButton( this );
  myEditHypBtn[ MainHyp ]->setIcon( aEditIcon );

  // Line
  QFrame* aLine = new QFrame( this );
  aLine->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  // Add. hypothesis
  QLabel* anAddHypLbl = new QLabel( tr( "ADD_HYPOTHESIS" ), this );
  myHypCombo[ AddHyp ] = new QComboBox( this );
  myHypCombo[ AddHyp ]->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myCreateHypBtn[ AddHyp ] = new QToolButton( this );
  myCreateHypBtn[ AddHyp ]->setIcon( aCreateIcon );
  myEditHypBtn[ AddHyp ] = new QToolButton( this );
  myEditHypBtn[ AddHyp ]->setIcon( aEditIcon );
  myEditHypBtn[ MoreAddHyp ] = new QToolButton( this );
  myEditHypBtn[ MoreAddHyp ]->setIcon( aEditIcon );

  myAddHypList = new QListWidget( this );
  myMoreAddHypBtn = new QToolButton( this );
  myMoreAddHypBtn->setIcon( aPlusIcon );
  myLessAddHypBtn = new QToolButton( this );
  myLessAddHypBtn->setIcon( aMinusIcon );

  // Fill layout
  QGridLayout* aLay = new QGridLayout( this );
  aLay->setMargin( MARGIN );
  aLay->setSpacing( SPACING );

  aLay->addWidget( anAlgoLbl, 0, 0 );
  aLay->addWidget( myHypCombo[ Algo ], 0, 1 );
  aLay->addWidget( aHypLbl, 1, 0 );
  aLay->addWidget( myHypCombo[ MainHyp ], 1, 1 );
  aLay->addWidget( myCreateHypBtn[ MainHyp ], 1, 2 );
  aLay->addWidget( myEditHypBtn[ MainHyp ], 1, 3 );
  aLay->addWidget( aLine, 2, 0, 1, 4 );
  aLay->addWidget( anAddHypLbl, 3, 0 );
  aLay->addWidget( myHypCombo[ AddHyp ], 3, 1 );
  aLay->addWidget( myCreateHypBtn[ AddHyp ], 3, 2 );
  aLay->addWidget( myEditHypBtn[ AddHyp ], 3, 3 );
  aLay->addWidget( myAddHypList, 4, 1, 2, 1 );
  aLay->addWidget( myMoreAddHypBtn, 4, 2 );
  aLay->addWidget( myEditHypBtn[ MoreAddHyp ], 4, 3 );
  aLay->addWidget( myLessAddHypBtn, 5, 2 );
  aLay->addItem( new QSpacerItem( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding ), 6, 0 );

  // Connect signals and slots
  for ( int i = MainHyp; i <= AddHyp; i++ )
  {
    connect( myCreateHypBtn[ i ], SIGNAL( clicked() )       ,  SLOT( onCreateHyp() ) );
    connect( myEditHypBtn[ i ]  , SIGNAL( clicked() )       ,  SLOT( onEditHyp() ) );
    connect( myHypCombo[ i ]    , SIGNAL( activated( int ) ),  SLOT( onHyp( int ) ) );
  }
  connect( myHypCombo[ Algo ], SIGNAL( activated( int ) ), SLOT( onHyp( int ) ) );

  connect( myAddHypList, SIGNAL( currentRowChanged( int ) ), SLOT( onHyp( int ) ) );
  connect( myEditHypBtn[ MoreAddHyp ], SIGNAL( clicked() ), SLOT( onEditHyp() ) );
  connect( myMoreAddHypBtn, SIGNAL( clicked() ), SLOT( onMoreAddHyp() ));
  connect( myLessAddHypBtn, SIGNAL( clicked() ), SLOT( onLessAddHyp() ));

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
 * \brief Adds an item in a control corresponding to \a type
 *  \param [in] txt - item text
 *  \param [in] type - HypType
 *  \param [in] index - index of item in a list of items
 *  \param [in] isGroup - is the item a group title
 */
//================================================================================
void SMESHGUI_MeshTab::addItem( const QString& txt,
                                const int      type,
                                const int      index,
                                const bool     isGroup )
{
  const char* prefix = "  ";
  if ( type <= AddHyp )
  {
    if ( isGroup )
    {
      int idx = myHypCombo[ type ]->count();
      myHypCombo[ type ]->addItem( txt.mid( 6 ), QVariant( index ));
      myHypCombo[ type ]->setItemData( idx, "separator", Qt::AccessibleDescriptionRole );
    }
    else
    {
      myHypCombo[ type ]->addItem( prefix + txt, QVariant( index ));
    }
    //myHypCombo[ type ]->setMaxVisibleItems( qMax( 10, myHypCombo[ type ]->count() ) );
  }
  else
  {
    QListWidgetItem* item = new QListWidgetItem( prefix + txt, myAddHypList );
    item->setData( Qt::UserRole, QVariant( index ));
  }
}

//================================================================================
/*!
 * \brief Returns index of hyp of a given type
 */
//================================================================================
int SMESHGUI_MeshTab::getCurrentIndex( const int type, const bool curByType ) const
{
  if ( type <= AddHyp )
  {
    return myHypCombo[ type ]->itemData( myHypCombo[ type ]->currentIndex() ).toInt();
  }
  else
  {
    int row = curByType ? ( type - AddHyp - 1 ) : myAddHypList->currentRow();
    if ( QListWidgetItem* item = myAddHypList->item( row ))
      return item->data( Qt::UserRole ).toInt();
  }
  return -1;
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
  myAvailableHypTypes[ theId ] = theHyps;

  bool enable = ! theHyps.isEmpty();
  if ( theId == Algo ) // fill list of algos
  {
    myHypCombo[ Algo ]->clear();
    if ( enable )
    {
      addItem( tr( "NONE"), Algo, 0 );
      for ( int i = 0, nbHyp = theHyps.count(); i < nbHyp; ++i )
        addItem( theHyps[i], Algo, i+1, theHyps[i].startsWith( "GROUP:" ));
      myHypCombo[ Algo ]->setCurrentIndex( 0 );
    }
  }
  else // enable buttons
  {
    myCreateHypBtn[ theId ]->setEnabled( enable );
    myEditHypBtn  [ theId ]->setEnabled( false );
  }
  myHypCombo[ theId ]->setEnabled( enable );
}

//================================================================================
/*!
 * \brief Sets existing hypothesis
 * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
 * \param theHyps - list of available hypothesis names
 * \param theDefaultAvlbl - \c true means that the algorithm can work w/o hypothesis
 *                          with some default parameters
 *
 * Sets existing main or additional hypothesis for this tab
 */
//================================================================================
void SMESHGUI_MeshTab::setExistingHyps( const int          theId,
                                        const QStringList& theHyps,
                                        bool               theDefaultAvlbl)
{
  if ( theId != Algo )
  {
    bool enable = ! myAvailableHypTypes[ theId ].isEmpty();
    myHypCombo[ theId ]->clear();
    if ( enable )
    {
      QString none = tr( theDefaultAvlbl ? "DEFAULT" : ( theId == AddHyp ) ? "NONE" : "NONE" );
      addItem( none, theId, 0 );
      for ( int i = 0, nbHyp = theHyps.count(); i < nbHyp; ++i )
        addItem( theHyps[i], theId, i+1 );
      myHypCombo[ theId ]->setCurrentIndex( 0 );
    }
    myHypCombo  [ theId ]->setEnabled( enable );
    myEditHypBtn[ theId ]->setEnabled( false );
    if ( theId == AddHyp )
    {
      myAddHypList->clear();
      myEditHypBtn[ MoreAddHyp ]->setEnabled( false );
      myMoreAddHypBtn->setEnabled( false );
      myLessAddHypBtn->setEnabled( false );
    }
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
  int index = myHypCombo[ theId ]->count();
  if ( theId == AddHyp )
    index += myAddHypList->count();
  addItem( theHyp, theId, index );
  myHypCombo[ theId ]->setCurrentIndex( myHypCombo[ theId ]->count() - 1 );
  myEditHypBtn[ theId ]->setEnabled( true );
  myHypCombo[ theId ]->setEnabled( true );
  if ( theId == AddHyp )
    myMoreAddHypBtn->setEnabled( true );
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
  if ( theId <= AddHyp )
  {
    if ( theIndex >= 0 && theIndex < myHypCombo[ theId ]->count() )
    {
      myHypCombo[ theId ]->setCurrentIndex( theIndex );
      if ( myEditHypBtn[ theId ] )
        myEditHypBtn[ theId ]->setEnabled( theIndex > 0 );
      if ( theId == AddHyp )
        myMoreAddHypBtn      ->setEnabled( theIndex > 0 );
    }
  }
  else // more than one additional hyp assigned
  {
    if ( theIndex > 0 )
    {
      // move a hyp from myHypCombo[ AddHyp ] to myAddHypList
      for ( int i = 1, nb = myHypCombo[ AddHyp ]->count(); i < nb; ++i )
      {
        int curIndex = myHypCombo[ AddHyp ]->itemData( i ).toInt();
        if ( theIndex == curIndex )
        {
          addItem( myHypCombo[ AddHyp ]->itemText( i ), theId, theIndex );
          myHypCombo[ AddHyp ]->removeItem( i );
          break;
        }
      }
    }
    else
    {
      myAddHypList->clear();
    }
  }
}

//================================================================================
/*!
 * \brief Gets current hypothesis
  * \param theId - identifier of hypothesis (main or additional, see HypType enumeration)
  * \retval int - index of current hypothesis
 * 
 * Gets current hypothesis
 * Use theId > AddHyp to get more than selected addetional hyps (see nbAddHypTypes()).
 */
//================================================================================
int SMESHGUI_MeshTab::currentHyp( const int theId ) const
{
  return getCurrentIndex( theId, /*curByType=*/true );
}

//================================================================================
/*!
 * \brief Returns nb of selected supplementary additional hypotheses
 * 
 * Access to their indices is via currentHyp( AddHyp + i ) where i is within the
 * range 0 <= i < this->nbAddHypTypes()
 */
//================================================================================
int SMESHGUI_MeshTab::nbAddHypTypes() const
{
  return myAddHypList->count();
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
  bool isMainHyp = ( sender() == myCreateHypBtn[ MainHyp ]);

  QtxMenu aPopup( this );
  
  QStringList aHypNames = isMainHyp ? 
    myAvailableHypTypes[ MainHyp ] : myAvailableHypTypes[ AddHyp ];

  QList<QAction*> actions;
  for ( int i = 0, n = aHypNames.count(); i < n; i++ )
  {
    QAction* a = 0;
    if ( aHypNames[ i ].startsWith( "GROUP:" ))
    {
      aPopup.appendGroupTitle( aHypNames[ i ].mid( 6 ));
    }
    else
    {
      a = aPopup.addAction( aHypNames[ i ] );
    }
    actions.append( a );
  }
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
  int aHypType = MainHyp;
  for ( ; aHypType <= MoreAddHyp; ++aHypType )
    if ( aSender == myEditHypBtn[ aHypType ])
      break;
  emit editHyp( Min( aHypType, AddHyp ),
                getCurrentIndex( aHypType ) - 1 );  // - 1 because there is NONE on the top
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
  QObject* aSender = sender();

  if ( QComboBox* cb = qobject_cast< QComboBox* >( aSender ))
  {
    // don't allow selecting a group title
    if ( cb->itemData( theIndex, Qt::AccessibleDescriptionRole ) == "separator" )
    {
      cb->setCurrentIndex( theIndex+1 );
      return;
    }
  }

  if ( aSender == myHypCombo[ Algo ] )
  {
    emit selectAlgo( theIndex - 1 ); // - 1 because there is NONE on the top
  }
  else if ( aSender == myAddHypList )
  {
    myEditHypBtn[ MoreAddHyp ]->setEnabled( theIndex >= 0 );
    myLessAddHypBtn           ->setEnabled( theIndex >= 0 );
  }
  else
  {
    int type = ( aSender == myHypCombo[ MainHyp ] ? MainHyp : AddHyp );
    myEditHypBtn[ type ]->setEnabled( theIndex > 0 );

    if ( type == AddHyp )
      myMoreAddHypBtn   ->setEnabled( theIndex > 0 );
  }
}

//================================================================================
/*!
 * \brief Adds a current additional hyp to myAddHypList
 * 
 * SLOT called when myMoreAddHypBtn ("plus") clicked
 */
//================================================================================
void SMESHGUI_MeshTab::onMoreAddHyp()
{
  int hypIndex = currentHyp( AddHyp );
  if ( hypIndex > 0 )
  {
    // move a hyp from myHypCombo[ AddHyp ] to myAddHypList
    int comboIndex = myHypCombo[ AddHyp ]->currentIndex();
    addItem( myHypCombo[ AddHyp ]->itemText( comboIndex ), MoreAddHyp, hypIndex );

    myHypCombo[ AddHyp ]->removeItem( comboIndex );
    myHypCombo[ AddHyp ]->setCurrentIndex( 0 );

    myMoreAddHypBtn->setEnabled( false );
    myEditHypBtn[ AddHyp ]->setEnabled( false );
  }
}

//================================================================================
/*!
 * \brief Removes a current additional hyp from myAddHypList
 * 
 * SLOT called when myLessAddHypBtn ("minus") clicked
 */
//================================================================================
void SMESHGUI_MeshTab::onLessAddHyp()
{
  if ( QListWidgetItem * item = myAddHypList->currentItem() )
  {
    // move a hyp from myAddHypList to myHypCombo[ AddHyp ]
    int hypIndex = item->data( Qt::UserRole ).toInt();
    addItem( item->text(), AddHyp, hypIndex );
    delete item;//myAddHypList->takeItem( myAddHypList->currentRow() );
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
    myHypCombo[ i ]->setCurrentIndex( 0 );
    if ( myEditHypBtn[ i ] )
      myEditHypBtn[ i ]->setEnabled( false );
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
  // mesh type
  QLabel* anMeshTypeLbl = new QLabel( tr( "MESH_TYPE" ), this );
  myMeshType = new QComboBox( this );
  
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
  aLay->addWidget( anMeshTypeLbl,             3, 0 );
  aLay->addWidget( myMeshType,                3, 2 );
  aLay->addWidget( myTabWg,                   5, 0, 1, 3 );
  aLay->addWidget( myHypoSetButton,           6, 0, 1, 3 );
  aLay->setRowMinimumHeight( 3, 20 );

  myMeshType->clear();

  // Connect signals and slots
  connect( myMeshType, SIGNAL( activated( int ) ), SLOT( onChangedMeshType( int ) ) );
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
  setTitile( theToCreate, theIsMesh );
}

SMESHGUI_MeshDlg::~SMESHGUI_MeshDlg()
{
}

//================================================================================
/*!
 * \brief Set dialog title
 */
//================================================================================
void SMESHGUI_MeshDlg::setTitile( const bool theToCreate, const bool theIsMesh )
{
  if ( theToCreate )
  {
    setWindowTitle( tr( theIsMesh ? "CREATE_MESH" : "CREATE_SUBMESH" ));
  }
  else
  {
    setWindowTitle( tr( theIsMesh ? "EDIT_MESH" : "EDIT_SUBMESH") );
  }
  
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
void SMESHGUI_MeshDlg::disableTab(const int theTabId)
{
  myTabWg->setTabEnabled( myTabWg->indexOf( myTabs[ theTabId ] ), false );
}

//================================================================================
/*!
 * \brief Enable tabs
 * \param int - tab ID
 */
//================================================================================
void SMESHGUI_MeshDlg::enableTab(const int theTabId)
{
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
bool SMESHGUI_MeshDlg::isTabEnabled(const int theTabId) const
{
  return myTabWg->isTabEnabled( myTabWg->indexOf( myTabs[ theTabId ] ) );
}

//================================================================================
/*!
 * \brief SLOT called when a Geom selection button is clicked
 */
//================================================================================
void SMESHGUI_MeshDlg::onGeomSelectionButton(bool isBtnOn)
{
  if ( myGeomPopup && isBtnOn )
    myGeomPopup->exec( QCursor::pos() );
}

//================================================================================
/*!
 * \brief SLOT called when a item of Geom selection popup is choosen
 */
//================================================================================
void SMESHGUI_MeshDlg::onGeomPopup( QAction* a )
{
  emit geomSelectionByMesh( a->data().toInt() == GEOM_BY_MESH_INDEX );
}

//================================================================================
/*!
 * \brief Return ID of an active selection button
 */
//================================================================================
int SMESHGUI_MeshDlg::getActiveObject()
{
  for (int i = 0; i < 3; ++i )
    if ( isObjectShown( i ) &&
         (( QToolButton* )objectWg( i, Btn ))->isChecked())
      return i;
  return -1;
}
//================================================================================
/*!
 * \brief Sets available types of mesh
 * \param theTypeMesh - list of available types of mesh
 */
//================================================================================
void SMESHGUI_MeshDlg::setAvailableMeshType( const QStringList& theTypeMesh )
{
  myMeshType->clear();
  myMeshType->addItems(theTypeMesh);
}
//================================================================================
/*!
 * \brief Emits selectMeshType( const int, const int ) signal
 *
 * SLOT is called when a combo box "mesh type" is selected.
 */
//================================================================================
void SMESHGUI_MeshDlg::onChangedMeshType( const int isIndex )
{
  emit selectMeshType( Dim3D - myTabWg->currentIndex(), isIndex );
}
//================================================================================
/*!
 * \brief Get current index types of mesh
 */
//================================================================================
int SMESHGUI_MeshDlg::currentMeshType( )
{
  return myMeshType->currentIndex( );
}
//================================================================================
/*!
 * \brief Set current index types of mesh
 */
//================================================================================
void SMESHGUI_MeshDlg::setCurrentMeshType( const int theIndex )
{
  myMeshType->setCurrentIndex( theIndex );
}
