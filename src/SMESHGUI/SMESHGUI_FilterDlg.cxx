//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_FilterDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_Filter.h"
#include "SMESH_Actor.h"
#include "VTKViewer_InteractorStyleSALOME.h"
#include "VTKViewer_ViewFrame.h"
#include "QAD_RightFrame.h"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOMEGUI_QtCatchCorbaException.hxx"


#include <TColStd_MapOfInteger.hxx>

#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qtable.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qfontmetrics.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qlistbox.h>
#include <qcheckbox.h>

#define SPACING 5                  
#define MARGIN  10

static int maxLength( const QStringList& theList, const QFontMetrics& theMetrics )
{
  int aRes = 0;
  QStringList::const_iterator anIter;
  for ( anIter = theList.begin(); anIter != theList.end(); ++anIter )
    aRes = Max( aRes, theMetrics.width( *anIter ) );
  return aRes; 
}

/*
  Class       : SMESHGUI_FilterDlg::Table            
  Description : Table used by this dialog
*/

class SMESHGUI_FilterDlg::Table : public QTable
{
public:
                          Table( QWidget* parent = 0 );
                          Table( int numRows, int numCols, QWidget* parent = 0 );
  virtual                 ~Table();

  void                    SetEditable( const bool state, const int row, const int col );
  bool                    IsEditable( const int row, const int col ) const;  

  virtual bool            eventFilter( QObject *o, QEvent *e );
  virtual void            insertRows( int row, int count = 1 );
  virtual QString         text( int row, int col ) const;
};

SMESHGUI_FilterDlg::Table::Table( QWidget* parent )
: QTable( parent, "SMESHGUI_FilterDlg::Table" )
{
}

SMESHGUI_FilterDlg::Table::Table( int numRows, int numCols, QWidget* parent )
: QTable( numRows, numCols, parent, "SMESHGUI_FilterDlg::Table" )
{
}

SMESHGUI_FilterDlg::Table::~Table()
{
}

void SMESHGUI_FilterDlg::Table::SetEditable( const bool isEditable, const int row, const int col )
{
  QTableItem* anItem = item( row, col );
  if( anItem )
    takeItem( anItem );

  if ( !isEditable )
  {
    setItem( row, col, new QTableItem( this, QTableItem::Never, "" ) );
  }
  else
  {
    setItem( row, col, new QTableItem( this, QTableItem::OnTyping, "" ) );
  }
}

bool SMESHGUI_FilterDlg::Table::IsEditable( const int row, const int col ) const
{
  QTableItem* anItem = item( row, col );
  return anItem == 0 || anItem->editType() != QTableItem::Never;
}

void SMESHGUI_FilterDlg::Table::insertRows( int row, int count )
{
  int anEditRow = currEditRow();
  int anEditCol = currEditCol();

//  printf( "sln: anEditRow = %d, anEditCol = %d\n", anEditRow, anEditCol );

  if ( anEditRow >= 0 && anEditCol >= 0 )
    endEdit( anEditRow, anEditCol, true, false );

  QTable::insertRows( row, count );
}

QString SMESHGUI_FilterDlg::Table::text( int row, int col ) const
{
  int anEditRow = currEditRow();
  int anEditCol = currEditCol();

  if ( anEditRow >= 0 && anEditCol >= 0 && anEditRow == row && anEditCol == col )
    ((Table*)this)->endEdit( row, col, true, false );

  return QTable::text( row, col );
}

bool SMESHGUI_FilterDlg::Table::eventFilter( QObject *o, QEvent *e )
{
  return QTable::eventFilter( o, e );
}

/*
  Class       : SMESHGUI_FilterDlg
  Description : Dialog to specify filters for VTK viewer
*/


//=======================================================================
// name    : SMESHGUI_FilterDlg::SMESHGUI_FilterDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterDlg::SMESHGUI_FilterDlg( QWidget*          theParent, 
                                        const int         theType,
                                        const bool        theModal,
                                        const char*       theName )
: QDialog( theParent, theName, theModal, 
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  myType = theType;

  setCaption( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  myMainFrame        = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this, theModal );

  aDlgLay->addWidget( myMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( myMainFrame, 1 );

  Init( myType ); 
}


//=======================================================================
// name    : SMESHGUI_FilterDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_FilterDlg::createMainFrame( QWidget* theParent )
{
  QGroupBox* aMainFrame = new QGroupBox( 1, Qt::Horizontal, theParent );
  aMainFrame->setFrameStyle( QFrame::NoFrame );
  aMainFrame->setInsideMargin( 0 );

  // filter frame

  QGroupBox* aFilterGrp = new QGroupBox( 1, Qt::Horizontal, tr ( "Filter" ), aMainFrame );
  QFrame* aFilterFrame = new QFrame( aFilterGrp );

  myTableGrp = new QGroupBox( 1, Qt::Horizontal, aFilterFrame );
  myTableGrp->setFrameStyle( QFrame::NoFrame );
  myTableGrp->setInsideMargin( 0 );
      
  myTables[ myType ] = createTable( myTableGrp, myType );
  myAddBtn = new QPushButton( tr( "ADD" ), aFilterFrame );
  myRemoveBtn = new QPushButton( tr( "REMOVE" ), aFilterFrame );
  myClearBtn = new QPushButton( tr( "CLEAR" ), aFilterFrame );

  QGridLayout* aLay = new QGridLayout( aFilterFrame, 4, 2, 0, SPACING );

  aLay->addMultiCellWidget( myTableGrp, 0, 3, 0, 0 );
  aLay->addWidget( myAddBtn, 0, 1 );
  aLay->addWidget( myRemoveBtn, 1, 1 );
  aLay->addWidget( myClearBtn, 2, 1 );
  aLay->setColStretch( 0, 1 );
  aLay->setColStretch( 1, 0 );

  mySetInViewer = new QCheckBox( tr( "SET_IN_VIEWER" ), aFilterGrp );
  mySetInViewer->setChecked( true );

  QSpacerItem* aVSpacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
  aLay->addItem( aVSpacer, 3, 1 );

  // other controls
  mySourceGrp = createSourceGroup( aMainFrame );
  
  // signals and slots
  connect( myAddBtn,    SIGNAL( clicked() ), this, SLOT( onAddBtn() ) );
  connect( myRemoveBtn, SIGNAL( clicked() ), this, SLOT( onRemoveBtn() ) );
  connect( myClearBtn,  SIGNAL( clicked() ), this, SLOT( onClearBtn() ) );

  return aMainFrame;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createSourceFrame
// Purpose : Create frame containing source radio button
//=======================================================================
QButtonGroup* SMESHGUI_FilterDlg::createSourceGroup( QWidget* theParent )
{
  QButtonGroup* aGrp = new QButtonGroup( 1, Qt::Vertical, tr( "SOURCE" ), theParent );

  QRadioButton* aMeshBtn = new QRadioButton( tr( "MESH" ), aGrp );
  QRadioButton* aSelBtn  = new QRadioButton( tr( "SELECTION" ), aGrp );
  QRadioButton* aGrpBtn  = new QRadioButton( tr( "CURRENT_GROUP" ), aGrp );
//  QRadioButton* aNoneBtn = new QRadioButton( tr( "NONE" ), aGrp );

  aGrp->insert( aMeshBtn, Mesh );
  aGrp->insert( aSelBtn, Selection );
  aGrp->insert( aGrpBtn, Dialog );
//  aGrp->insert( aNoneBtn, None );

  aGrp->setButton( Selection );

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createTable
// Purpose : Create table
//=======================================================================
SMESHGUI_FilterDlg::Table* SMESHGUI_FilterDlg::createTable( QWidget*  theParent, 
                                                            const int theType )
{
  // create table
  Table* aTable= new Table( 0, 5, theParent );

  QHeader* aHeaders = aTable->horizontalHeader();

  QFontMetrics aMetrics( aHeaders->font() );

  int aLenCr = abs(  maxLength( getCriteria( theType ), aMetrics ) - 
                     aMetrics.width( tr( "CRITERION" ) ) ) / aMetrics.width( ' ' ) + 5;

  int aLenCo = abs(  maxLength( getCriteria( theType ), aMetrics ) - 
                     aMetrics.width( tr( "COMPARE" ) ) ) / aMetrics.width( ' ' ) + 5;

  QString aCrStr;
  aCrStr.fill( ' ', aLenCr );
  QString aCoStr;
  aCoStr.fill( ' ', 10 );

  aHeaders->setLabel( 0, tr( "CRITERION" ) + aCrStr );
  aHeaders->setLabel( 1, tr( "COMPARE" ) + aCoStr );
  aHeaders->setLabel( 2, tr( "THRESHOLD_VALUE" ) );
  aHeaders->setLabel( 3, tr( "UNARY" ) );
  aHeaders->setLabel( 4, tr( "BINARY" ) + "  " );

  // set geometry of the table 

  for ( int i = 0; i <= 4; i++ )
    aTable->adjustColumn( i );

  aTable->updateGeometry();
  QSize aSize = aTable->sizeHint();
  int aWidth = aSize.width();
  aTable->setMinimumSize( QSize( aWidth, aWidth / 2 ) );
  aTable->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding) );

  if ( theType == SMESH::EDGE )
    connect( aTable, SIGNAL( valueChanged( int, int ) ),
             this, SLOT( onCriterionChanged( int, int ) ) );

  return aTable;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_FilterDlg::createButtonFrame( QWidget* theParent, const bool theModal )
{
  QGroupBox* aGrp = new QGroupBox( 1, Qt::Vertical, theParent );
 
  myOkBtn      = new QPushButton( tr( "SMESH_BUT_OK"    ), aGrp );
  myApplyBtn   = new QPushButton( tr( "SMESH_BUT_APPLY" ), aGrp );
  QLabel* aLbl = new QLabel( aGrp );
  myCloseBtn   = new QPushButton( tr( "SMESH_BUT_CANCEL" ), aGrp );

  aLbl->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  connect( myOkBtn,    SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myApplyBtn, SIGNAL( clicked() ), SLOT( onApply() ) );

  if ( theModal )
    myApplyBtn->hide();

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::~SMESHGUI_FilterDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_FilterDlg::~SMESHGUI_FilterDlg()
{
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FilterDlg::Init( const int theType )
{
  mySourceWg  = 0;
  mySelection = 0;
  myType      = theType;
  myMesh      = SMESH::SMESH_Mesh::_nil();

  // activate corresponding tab
  if ( !myTables.contains( myType ) )
    myTables[ myType ] = createTable( myTableGrp, myType );

  TableMap::iterator anIter;
  for ( anIter = myTables.begin(); anIter != myTables.end(); ++anIter )
    if ( anIter.key() == theType )
      anIter.data()->show();
    else
      anIter.data()->hide();

  // set caption
  setCaption(  myType == SMESH::EDGE ? tr( "EDGES_TLT" ) : tr( "FACES_TLT" ) );

  qApp->processEvents();
  updateGeometry();
  adjustSize();
  setEnabled( true );

  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( ( QDialog* )this ) ;

  connect( mySMESHGUI, SIGNAL( SignalDeactivateActiveDialog() ), SLOT( onDeactivate() ) );
  connect( mySMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( onClose() ) );

  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y );
  this->move( x, y );

  this->show();

  return;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onOk
// Purpose : SLOT called when "Ok" button pressed. 
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_FilterDlg::onOk()
{
  if ( onApply() )
  {
    disconnect( mySMESHGUI, 0, this, 0 );
    mySMESHGUI->ResetState() ;
    accept();
  }
    
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_FilterDlg::onClose()
{
  disconnect( mySMESHGUI, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
  return ;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_FilterDlg::onDeactivate()
{
  setEnabled( false );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_FilterDlg::enterEvent( QEvent* )
{
//  mySMESHGUI->EmitSignalDeactivateDialog();   
  setEnabled( true );
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_FilterDlg::closeEvent( QCloseEvent* e )
{
  onClose() ;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCriteria
// Purpose : Retrieve list of ids from given widget
//=======================================================================
void SMESHGUI_FilterDlg::getIdsFromWg( const QWidget* theWg, QValueList<int>& theRes ) const
{
  theRes.clear();
  if ( theWg == 0 )
    return;

  if ( theWg->inherits( "QListBox" ) )
  {
    QListBox* aListBox = ( QListBox* )theWg;
    bool b;
    for ( int i = 0, n = aListBox->count(); i < n; i++ )
    {
      int anId = aListBox->text( i ).toInt( &b );
      if ( b )
        theRes.append( anId );
    }
  }
  else if ( theWg->inherits( "QLineEdit" ) )
  {
    QLineEdit* aLineEdit = ( QLineEdit* )theWg;
    QString aStr = aLineEdit->text();
    QRegExp aRegExp( "(\\d+)" );
    bool b;
    int aPos = 0;
    while ( aPos >= 0 )
    {
      aPos = aRegExp.search( aStr, aPos );
      if ( aPos > -1 )
      {
        int anId = aRegExp.cap( 1 ).toInt( &b );
        if ( b )
          theRes.append( anId );
        aPos += aRegExp.matchedLength();
      }
    }
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getSelMode
// Purpose : Get criteria for specified type
//=======================================================================
Selection_Mode SMESHGUI_FilterDlg::getSelMode( const int theType ) const
{
  switch ( theType )
  {
    case SMESH::NODE   : return NodeSelection;
    case SMESH::EDGE   : return EdgeSelection;
    case SMESH::FACE   : return FaceSelection;
    case SMESH::VOLUME : return VolumeSelection;
    default            : return ActorSelection;
  }
 
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCriteria
// Purpose : Get criteria for specified type
//=======================================================================
void SMESHGUI_FilterDlg::setIdsToWg( QWidget* theWg, const QValueList<int>& theIds )
{
  if ( theWg == 0 )
    return;
    
  if ( theWg->inherits( "QListBox" ) )
  {
    QListBox* aListBox = ( QListBox* )theWg;
    aListBox->clear();
    
    QStringList aStrList;
    QValueList<int>::const_iterator anIter;
    for ( anIter = theIds.begin(); anIter != theIds.end(); ++anIter )
      aStrList.append( QString( "%1" ).arg( *anIter ) );

    aListBox->insertStringList( aStrList );
  }
  else if ( theWg->inherits( "QLineEdit" ) )
  {
    QLineEdit* aLineEdit = ( QLineEdit* )theWg;
    QString aStr;
    QValueList<int>::const_iterator anIter;

    for ( anIter = theIds.begin(); anIter != theIds.end(); ++ anIter )
      aStr += QString( "%1 " ).arg( *anIter );

    if ( !aStr.isEmpty() )
      aStr.remove( aStr.length() - 1, 1 );

    aLineEdit->setText( aStr );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCriteria
// Purpose : Get criteria for specified type
//=======================================================================
const QStringList& SMESHGUI_FilterDlg::getCriteria( const int theType ) const
{
  if ( theType == SMESH::EDGE )
  {
    static QStringList aCriteria;
    if ( aCriteria.isEmpty() )
    {
      aCriteria.append( tr( "FREE_BORDERS" ) );
      aCriteria.append( tr( "MULTI_BORDERS" ) );
      aCriteria.append( tr( "LENGTH" ) );
    }
    return aCriteria;
  }
  else if ( theType == SMESH::FACE )
  {
    static QStringList aCriteria;
    if ( aCriteria.isEmpty() )
    {
      aCriteria.append( tr( "ASPECT_RATIO" ) );
      aCriteria.append( tr( "WARPING" ) );
      aCriteria.append( tr( "MINIMUM_ANGLE" ) );
      aCriteria.append( tr( "TAPER" ) );
      aCriteria.append( tr( "SKEW" ) );
      aCriteria.append( tr( "AREA" ) );
    }
    return aCriteria;
  }
  else
  {
    static QStringList aCriteria;
    return aCriteria;
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCompare
// Purpose : Get operation of comparison
//=======================================================================
const QStringList& SMESHGUI_FilterDlg::getCompare () const
{
  static QStringList aList;

  if ( aList.isEmpty() )
  {
    aList.append( tr( "LESS_THAN" ) );
    aList.append( tr( "MORE_THAN" ) );
    aList.append( tr( "EQUAL_TO" ) );
  }

  return aList;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCriterionItem
// Purpose : Get combo table item for criteria of specified type
//=======================================================================
QTableItem* SMESHGUI_FilterDlg::getCriterionItem( QTable* theParent , const int theType )
{
  return new QComboTableItem( theParent, getCriteria( theType ) );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCompareItem
// Purpose : Get combo table item for operation of comparision
//=======================================================================
QTableItem* SMESHGUI_FilterDlg::getCompareItem( QTable* theParent )
{
  return new QComboTableItem( theParent, getCompare() );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getLogOpItem
// Purpose :
//=======================================================================
QTableItem* SMESHGUI_FilterDlg::getLogOpItem( QTable* theParent )
{
  static QStringList aList;
  if ( aList.isEmpty() )
  {
    aList.append( tr( "AND" ) );
    aList.append( tr( "OR" ) );
  }

  return new QComboTableItem( theParent, aList );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getNotItem
// Purpose : Get check table item
//=======================================================================
QTableItem* SMESHGUI_FilterDlg::getNotItem( QTable* theParent )
{
  return new QCheckTableItem( theParent, tr( "NOT" ) );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCurrType
// Purpose : Get current entity type
//=======================================================================
int SMESHGUI_FilterDlg::getCurrType() const
{
  return myType;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getCriterion
// Purpose :
//=======================================================================
int SMESHGUI_FilterDlg::getCriterion( const int theType, const int theRow ) const
{
  QComboTableItem* anItem = ( QComboTableItem* )myTables[ getCurrType() ]->item( theRow, 0 );
  return anItem->currentItem();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::addRow
// Purpose :
//=======================================================================
void SMESHGUI_FilterDlg::addRow( Table* theTable, const int theType )
{
  theTable->insertRows( theTable->numRows() );
  int aCurrRow = theTable->numRows() - 1;

  // Criteria
  theTable->setItem( aCurrRow, 0, getCriterionItem( theTable, theType ) );

  // Compare
  theTable->setItem( aCurrRow, 1, getCompareItem( theTable ) );

  //Logical operation NOT
  theTable->setItem( aCurrRow, 3, getNotItem( theTable ) );

  // Logical binary operation for previous value
  if ( aCurrRow > 0 )
    theTable->setItem( aCurrRow - 1, 4, getLogOpItem( theTable ) );
  theTable->SetEditable( false, aCurrRow, 4 );

  onCriterionChanged( aCurrRow, 0 );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onAddBtn
// Purpose : SLOT. Called then "Add" button pressed.
//           Adds new string to table
//=======================================================================
void SMESHGUI_FilterDlg::onAddBtn()
{
  // QTable
  int aType = getCurrType();
  Table* aTable = myTables[ aType ];

  addRow( aTable, aType );

  updateBtnState();

}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onCriterionChanged()
// Purpose : SLOT. Called then contents of table changed
//           Provides reaction on change of criterion
//=======================================================================
void SMESHGUI_FilterDlg::onCriterionChanged( int row, int col )
{
  int aType = getCurrType();
  if ( aType != SMESH::EDGE || col != 0 )
    return;

  Table* aTable = myTables[ aType ];
  QComboTableItem* aCompareItem = (QComboTableItem*)aTable->item( row, 1 );

  if ( getCriterion( aType, row ) != FreeBorders )
  {
    if ( aCompareItem->count() == 0 )
      aCompareItem->setStringList( getCompare() );

    QString aText = aTable->text( row, 2 );
    aTable->SetEditable( true, row, 2 );
    aTable->setText( row, 2, aText );
  }
  else
  {
    if ( aCompareItem->count() > 0 )
      aCompareItem->setStringList( QStringList() );
    aTable->SetEditable( false, row, 2 );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onRemoveBtn
// Purpose : SLOT. Called then "Remove" button pressed. 
//           Removes current string from table
//=======================================================================
void SMESHGUI_FilterDlg::onRemoveBtn()
{
  Table* aTable = myTables[ getCurrType() ];

  if ( aTable->numRows() == 0 )
    return;

  QMemArray<int> aRows;
  for ( int i = 0, n = aTable->numRows(); i < n; i++ )
  {
    if ( aTable->isRowSelected( i ) )
    {
      aRows.resize( aRows.size() + 1 );
      aRows[ aRows.size() - 1 ] = i;
    }
  }

  aTable->removeRows( aRows );

  // remove control of binary logical operation from last row
  if ( aTable->numRows() > 0 )
    aTable->SetEditable( false, aTable->numRows() - 1, 4 );

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onClearBtn
// Purpose : SLOT. Called then "Clear" button pressed.
//           Removes all strings from table
//=======================================================================
void SMESHGUI_FilterDlg::onClearBtn()
{
  QTable* aTable = myTables[ getCurrType() ];

  if ( aTable->numRows() == 0 )
    return;

  while ( aTable->numRows() > 0 )
    aTable->removeRow( 0 );

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::updateBtnState
// Purpose : Update button state
//=======================================================================
void SMESHGUI_FilterDlg::updateBtnState()
{
  myRemoveBtn->setEnabled( myTables[ getCurrType() ]->numRows() > 0 );
  myClearBtn->setEnabled( myTables[ getCurrType() ]->numRows() > 0 );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_FilterDlg::isValid() const
{
  Table* aTable = myTables[ getCurrType() ];
  for ( int i = 0, n = aTable->numRows(); i < n; i++ )
  {
    bool isEditable = aTable->IsEditable( i ,2 );
    bool aRes       = false;
    int  aThreshold = ( int )aTable->text( i, 2 ).toDouble( &aRes );
    if ( isEditable && !aRes )
    {
      QMessageBox::information( mySMESHGUI->GetDesktop(),
        tr( "SMESH_INSUFFICIENT_DATA" ), tr( "ERROR" ),
        QMessageBox::Ok );
      return false;
    }
    else if ( getCurrType() == SMESH::EDGE &&
              getCriterion( SMESH::EDGE, i ) == MultiBorders &&
              aThreshold == 1 )
    {
      QMessageBox::information( mySMESHGUI->GetDesktop(),
        tr( "SMESH_INSUFFICIENT_DATA" ), tr( "MULTIEDGES_ERROR" ),
        QMessageBox::Ok );
      return false;
    }
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::GetResultIds
// Purpose : Get filtered ids
//=======================================================================
/*void SMESHGUI_FilterDlg::GetResultIds( SMESH::SMESH_Mesh_ptr theMesh,
                                       QValueList<int>&      theIds ) const
{
  if ( !myPredicate->_is_nil() )
    theIds = myInputIds;
  else
    myPredicate->SetMesh( theMesh );
  
  theIds.clear();
  QValueList<int>::const_iterator anIter;
  for ( anIter = myInputIds.begin(); anIter != myInputIds.end(); ++anIter )
    if ( myPredicate->IsSatisfy( *anIter ) )
     theIds.append( *anIter );
}
*/

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetSourceWg
// Purpose : Set widget of parent dialog containing idsto be filtered if
//           user select corresponding source radio button
//=======================================================================
void SMESHGUI_FilterDlg::SetSourceWg( QWidget* theWg )
{
  mySourceWg = theWg;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetGroupIds
// Purpose : Set mesh
//=======================================================================
void SMESHGUI_FilterDlg::SetMesh( SMESH::SMESH_Mesh_ptr theMesh )
{
  myMesh = theMesh;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetSelection
// Purpose : Get filtered ids
//=======================================================================
void SMESHGUI_FilterDlg::SetSelection( SALOME_Selection* theSel )
{
  mySelection = theSel;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onApply
// Purpose : SLOT called when "Apply" button pressed.
//           Assign filters to VTK viewer
//=======================================================================
bool SMESHGUI_FilterDlg::onApply()
{
  if ( !isValid() )
    return false;

  try
  {
    int aCurrType = getCurrType();

    SMESH::Predicate_ptr aPredicate = createPredicate( aCurrType );

    if ( mySetInViewer->isChecked() )
      insertFilterInViewer( aPredicate );

    if ( !aPredicate->_is_nil() )
    {
      QValueList<int> aResultIds;
      filterSource( aCurrType, aPredicate, aResultIds );
      selectInViewer( aCurrType, aResultIds );
    }
  }
  catch( const SALOME::SALOME_Exception& S_ex )
  {
    QtCatchCorbaException( S_ex );
  }
  catch( ... )
  {
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createPredicate
// Purpose : Create predicate for given type
//=======================================================================
SMESH::Predicate_ptr SMESHGUI_FilterDlg::createPredicate( const int theType )
{
  SMESH::FilterManager_ptr aFilterMgr = mySMESHGUI->GetFilterMgr();
  if ( aFilterMgr->_is_nil() )
    return SMESH::Predicate::_nil();

  QTable* aTable = myTables[ theType ];
  if ( aTable == 0 )
    return SMESH::Predicate::_nil();

  // CREATE two lists ( PREDICATES  and LOG OP )

  // Criterion
  QValueList<SMESH::Predicate_ptr> aPredicates;
  QValueList<int>                  aLogOps;
  for ( int i = 0, n = aTable->numRows(); i < n; i++ )
  {
    int aCriterion = getCriterion( theType, i );

    SMESH::Predicate_ptr aPredicate = SMESH::Predicate::_nil();
    SMESH::NumericalFunctor_ptr aFunctor = SMESH::NumericalFunctor::_nil();

    if ( theType == SMESH::EDGE )
    {
      switch ( aCriterion )
      {
      case FreeBorders:
        aPredicate = aFilterMgr->CreateFreeBorders();
        break;
      case MultiBorders:
        aFunctor = aFilterMgr->CreateMultiConnection();
        break;
      case Length:
        aFunctor = aFilterMgr->CreateLength();
        break;
      default:
        continue;
      }
    }
    else
    {
      switch ( aCriterion )
      {
      case AspectRatio:
        aFunctor = aFilterMgr->CreateAspectRatio();
        break;
      case Warping:
        aFunctor = aFilterMgr->CreateWarping();
        break;
      case MinimumAngle:
        aFunctor = aFilterMgr->CreateMinimumAngle();
        break;
      case Taper:
        aFunctor = aFilterMgr->CreateTaper();
        break;
      case Skew:
        aFunctor = aFilterMgr->CreateSkew();
        break;
      case Area:
        aFunctor = aFilterMgr->CreateArea();
        break;
      default:
        continue;
      }
    }

    // Comparator
    if ( !aFunctor->_is_nil() && aPredicate->_is_nil() )
    {
      QComboTableItem* aCombo = (QComboTableItem*)aTable->item( i, 1 );
      int aCompareOp = aCombo->currentItem();
      double aThreshold = aTable->text( i, 2 ).toDouble();

      SMESH::Comparator_ptr aComparator = SMESH::Comparator::_nil();

      if ( aCompareOp == LessThan )
        aComparator = aFilterMgr->CreateLessThan();
      else if ( aCompareOp == MoreThan )
        aComparator = aFilterMgr->CreateMoreThan();
      else if ( aCompareOp == EqualTo )
        aComparator = aFilterMgr->CreateEqualTo();
      else
        continue;

      aComparator->SetNumFunctor( aFunctor );
      aComparator->SetMargin( aThreshold );

      aPredicate = aComparator;
    }

    // Logical not
    QCheckTableItem* anItem = (QCheckTableItem*)aTable->item( i, 3 );
    if ( anItem->isChecked() )
    {
      SMESH::LogicalNOT_ptr aNotPred = aFilterMgr->CreateLogicalNOT();
      aNotPred->SetPredicate( aPredicate );
      aPredicate = aNotPred;
    }

    // logical op
    int aLogOp = ( i == n - 1 ) ? LO_Undefined
                                : ( (QComboTableItem*)aTable->item( i, 4 ) )->currentItem();
    aPredicates.append( aPredicate );
    aLogOps.append( aLogOp );

  } // for

  // CREATE ONE PREDICATE FROM PREVIOUSLY CREATED MAP

  // combine all "AND" operations

  QValueList<SMESH::Predicate_ptr> aResList;

  QValueList<SMESH::Predicate_ptr>::iterator aPredIter;
  QValueList<int>::iterator                  aLogOpIter;

  SMESH::Predicate_ptr aPrevPredicate = SMESH::Predicate::_nil();
  int aPrevLogOp = LO_Undefined;

  for ( aPredIter = aPredicates.begin(), aLogOpIter = aLogOps.begin();
        aPredIter != aPredicates.end() && aLogOpIter != aLogOps.end();
        ++aPredIter, ++aLogOpIter )
  {
    int aCurrLogOp = *aLogOpIter;

    SMESH::Predicate_ptr aCurrPred = SMESH::Predicate::_nil();

    if ( aPrevLogOp == LO_And )
    {

      SMESH::LogicalBinary_ptr aBinaryPred = aFilterMgr->CreateLogicalAND();
      aBinaryPred->SetPredicate1( aPrevPredicate );
      aBinaryPred->SetPredicate2( *aPredIter );
      aCurrPred = aBinaryPred;
    }
    else
      aCurrPred = *aPredIter;

    if ( aCurrLogOp != LO_And )
      aResList.append( aCurrPred );

    aPrevPredicate = aCurrPred;
    aPrevLogOp = aCurrLogOp;
  }

  // combine all "OR" operations

  SMESH::Predicate_ptr aResPredicate = SMESH::Predicate::_nil();

  if ( aResList.count() == 1 )
    aResPredicate = aResList.first();
  else if ( aResList.count() > 1 )
  {
    QValueList<SMESH::Predicate_ptr>::iterator anIter = aResList.begin();
    aResPredicate = *anIter;
    anIter++;
    for ( ; anIter != aResList.end(); ++anIter )
    {
      SMESH::LogicalBinary_ptr aBinaryPred = aFilterMgr->CreateLogicalOR();
      aBinaryPred->SetPredicate1( aResPredicate );
      aBinaryPred->SetPredicate2( *anIter );
      aResPredicate = aBinaryPred;
    }
  }

  return aResPredicate;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::insertFilterInViewer
// Purpose : Insert filter in viewer
//=======================================================================
void SMESHGUI_FilterDlg::insertFilterInViewer( SMESH::Predicate_ptr thePred )
{
  VTKViewer_InteractorStyleSALOME* aStyle = ((VTKViewer_ViewFrame*)mySMESHGUI->GetActiveStudy()->
    getActiveStudyFrame()->getRightFrame()->getViewFrame())->
      getRWInteractor()->GetInteractorStyleSALOME();

  if ( thePred->_is_nil() )
  {
    if ( myType == SMESH::EDGE )
      aStyle->RemoveEdgeFilter();
    else if ( myType == SMESH::FACE )
      aStyle->RemoveFaceFilter();
  }
  else
  {
    Handle(SMESHGUI_Filter) aFilter = new SMESHGUI_Filter();
    aFilter->SetPredicate( thePred );
    if ( myType == SMESH::EDGE )
      aStyle->SetEdgeFilter( aFilter );
    else if ( myType == SMESH::FACE )
      aStyle->SetFaceFilter( aFilter );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::filterSource
// Purpose : Filter source ids
//=======================================================================
void SMESHGUI_FilterDlg::filterSource( const int theType,
                                       SMESH::Predicate_ptr thePred,
                                       QValueList<int>& theResIds )
{
  theResIds.clear();
  
  int aSourceId = mySourceGrp->id( mySourceGrp->selected() );

  if ( aSourceId == Mesh )
  {
    if ( myMesh->_is_nil() )
      return;
    SMESH::FilterManager_ptr aFilterMgr = mySMESHGUI->GetFilterMgr();
    SMESH::Filter_var aFilter = aFilterMgr->CreateFilter();
    aFilter->SetPredicate( thePred );
    SMESH::long_array_var anIds = aFilter->GetElementsId( myMesh );
    for ( int i = 0, n = anIds->length(); i < n; i++ )
      theResIds.append( anIds[ i ] );
  }
  else if ( aSourceId == Selection )
  {
    filterSelectionSource( theType, thePred, theResIds );
  }
  else if ( aSourceId == Dialog )
  {
    // retrieve ids from dialog
    QValueList<int> aDialogIds;
    getIdsFromWg( mySourceWg, aDialogIds );
    
    if ( myMesh->_is_nil() )
    {
      theResIds = aDialogIds;
      return;
    }

    // filter ids
    thePred->SetMesh( myMesh );
    QValueList<int>::const_iterator anIter;
    for ( anIter = aDialogIds.begin(); anIter != aDialogIds.end(); ++ anIter )
      if ( thePred->IsSatisfy( *anIter ) )
        theResIds.append( *anIter );

    // set ids to the dialog
    setIdsToWg( mySourceWg, theResIds );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::filterSelectionSource
// Purpose : Filter source selection
//=======================================================================
void SMESHGUI_FilterDlg::filterSelectionSource( const int theType,
                                                SMESH::Predicate_ptr thePred,
                                                QValueList<int>& theResIds )
{
  theResIds.clear();
  if ( myMesh->_is_nil() || mySelection == 0 )
    return;

  // Create map of entities to be filtered
  TColStd_MapOfInteger aToBeFiltered;
  Standard_Boolean aRes = false;
  SALOME_ListIteratorOfListIO anIter( mySelection->StoredIObjects() );
  
  for ( ; anIter.More(); anIter.Next() )
  {
    // process sub mesh
    SMESH::SMESH_subMesh_ptr aSubMesh = mySMESHGUI->ConvertIOinSubMesh( anIter.Value(), aRes );
    if ( aRes && !aSubMesh->_is_nil() )
    {
      if ( aSubMesh->GetFather()->GetId() == myMesh->GetId() )
      {
        SMESH::long_array_var anIds =
          theType == SMESH::NODE ? aSubMesh->GetNodesId() : aSubMesh->GetElementsId();
        for ( int i = 0, n = anIds->length(); i < n; i++ )
          aToBeFiltered.Add( anIds[ i ] );
      }
    }

    // process group
    SMESH::SMESH_Group_ptr aGroup = mySMESHGUI->ConvertIOinSMESHGroup( anIter.Value(), aRes );
    if ( aRes && !aGroup->_is_nil() )
    {
      if ( aGroup->GetType() == theType && aGroup->GetMesh()->GetId() == myMesh->GetId() )
      {
        SMESH::long_array_var anIds = aGroup->GetListOfID();
        for ( int i = 0, n = anIds->length(); i < n; i++ )
          aToBeFiltered.Add( anIds[ i ] );
      }
    }

    // process mesh
    SMESH::SMESH_Mesh_ptr aMeshPtr = mySMESHGUI->ConvertIOinMesh( anIter.Value(), aRes );
    if ( aRes && !aMeshPtr->_is_nil() && aMeshPtr->GetId() == myMesh->GetId() )
    {
      TColStd_MapOfInteger aVtkMap;
      mySelection->GetIndex( anIter.Value(), aVtkMap );

      if ( aVtkMap.Extent() > 0 )
      {
        SMESH_Actor *anActor = mySMESHGUI->FindActorByEntry(
          anIter.Value()->getEntry(), aRes, true );
        if ( aRes && anActor != 0 )
        {
          TColStd_MapIteratorOfMapOfInteger aVtkMapIter( aVtkMap );
          for ( ; aVtkMapIter.More(); aVtkMapIter.Next() )
            aToBeFiltered.Add( theType == SMESH::NODE
              ? anActor->GetNodeObjId( aVtkMapIter.Key() )
              : anActor->GetElemObjId( aVtkMapIter.Key() ) );
        }
      }
    }
  }

  // Filter entities
  thePred->SetMesh( myMesh );
  TColStd_MapIteratorOfMapOfInteger aResIter( aToBeFiltered );
  for ( ; aResIter.More(); aResIter.Next() )
    if ( thePred->IsSatisfy( aResIter.Key() ) )
      theResIds.append( aResIter.Key() );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::selectInViewer
// Purpose : Select given entities in viewer
//=======================================================================
void SMESHGUI_FilterDlg::selectInViewer( const int theType, const QValueList<int>& theIds )
{
  if ( mySelection == 0 || myMesh->_is_nil() )
    return;

  // Set new selection mode if necessary
  Selection_Mode aSelMode = getSelMode( theType );
  if ( aSelMode != mySelection->SelectionMode() )
  {
    mySelection->ClearIObjects();
    mySelection->ClearFilters();
    if ( aSelMode == NodeSelection )
      mySMESHGUI->ViewNodes();
    QAD_Application::getDesktop()->SetSelectionMode( aSelMode );
  }

  Standard_Boolean aRes = false;
  SMESH_Actor* anActor = mySMESHGUI->FindActor( myMesh, aRes, true );
  if ( !aRes || anActor == 0 || !anActor->hasIO() )
    return;

  Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
  mySelection->ClearIObjects();
  mySelection->AddIObject( anIO, false );

  TColStd_MapOfInteger aMap;
  QValueList<int>::const_iterator anIter;
  for ( anIter = theIds.begin(); anIter != theIds.end(); ++anIter )
  {
    std::vector<int> aVtkList = anActor->GetElemVtkId( *anIter );
    std::vector<int>::iterator it;
    for ( it = aVtkList.begin(); it != aVtkList.end(); ++it )
      aMap.Add( *it );
  }

  mySelection->AddOrRemoveIndex( anIO, aMap, false, true );
  
}




















