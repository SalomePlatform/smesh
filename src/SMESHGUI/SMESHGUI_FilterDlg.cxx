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

#include "SMESHGUI_FilterDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_FilterUtils.h"

#include "SMESHGUI_Filter.h"
#include "SMESH_Actor.h"
#include "VTKViewer_ViewFrame.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "QAD_RightFrame.h"
#include "QAD_Config.h"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_DataMapIteratorOfDataMapOfIOMapOfInteger.hxx"
#include "SALOMEGUI_QtCatchCorbaException.hxx"
#include "SMESHGUI_FilterLibraryDlg.h"
#include "SALOME_TypeFilter.hxx"

#include "GEOMBase.h"
#include "GEOM_FaceFilter.hxx"

#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <Precision.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>

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
#include <qobjectlist.h>
#include <qvalidator.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)

#define SPACING 5                  
#define MARGIN  10

using namespace SMESH;

static int maxLength( const QMap<int, QString> theMap, const QFontMetrics& theMetrics )
{
  int aRes = 0;
  QMap<int, QString>::const_iterator anIter;
  for ( anIter = theMap.begin(); anIter != theMap.end(); ++anIter )
    aRes = Max( aRes, theMetrics.width( anIter.data() ) );
  return aRes; 
}

static int getFilterId( SMESH::ElementType theType )
{
  switch ( theType )
  {
    case SMESH::NODE   : return SMESHGUI_NodeFilter;
    case SMESH::EDGE   : return SMESHGUI_EdgeFilter;
    case SMESH::FACE   : return SMESHGUI_FaceFilter;
    case SMESH::VOLUME : return SMESHGUI_VolumeFilter;
    case SMESH::ALL    : return SMESHGUI_AllElementsFilter;
    default            : return SMESHGUI_UnknownFilter;
  }
}

/*
  Class       : SMESHGUI_FilterTable::AdditionalWidget
  Description : Class for storing additional parameters of criterion
*/

class SMESHGUI_FilterTable::AdditionalWidget : public QFrame
{
public:
  enum { Tolerance };
  
public:

                          AdditionalWidget( QWidget* theParent );
  virtual                 ~AdditionalWidget();

  virtual void            GetParameters( QValueList<int>& ) const;
  virtual bool            IsValid( const bool theMsg = true ) const;
  virtual double          GetDouble( const int theId ) const;
  virtual int             GetInteger( const int theId ) const;
  virtual QString         GetString( const int theId ) const;
  virtual void            SetDouble( const int theId, const double theVal );
  virtual void            SetInteger( const int theId, const int theVal );
  virtual void            SetString( const int theId, const QString& theVal );
  void                    SetEditable( const int theId, const bool isEditable );
  void                    SetEditable( const bool isEditable );

private:
  QMap< int, QLineEdit* > myLineEdits;
};

SMESHGUI_FilterTable::AdditionalWidget::AdditionalWidget( QWidget* theParent )
: QFrame( theParent )
{
  QLabel* aLabel = new QLabel( tr( "SMESH_TOLERANCE" ), this );
  myLineEdits[ Tolerance ] = new QLineEdit( this );
  QDoubleValidator* aValidator = new QDoubleValidator( myLineEdits[ Tolerance ] );
  aValidator->setBottom( 0 );
  myLineEdits[ Tolerance ]->setValidator( aValidator );

  QHBoxLayout* aLay = new QHBoxLayout( this, 0, SPACING );
  aLay->addWidget( aLabel );
  aLay->addWidget( myLineEdits[ Tolerance ] );
  
  QSpacerItem* aSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  aLay->addItem( aSpacer );

  QString aText = QString( "%1" ).arg( Precision::Confusion() );
  myLineEdits[ Tolerance ]->setText( aText );
}
SMESHGUI_FilterTable::AdditionalWidget::~AdditionalWidget()
{
}

void SMESHGUI_FilterTable::AdditionalWidget::GetParameters( QValueList<int>& theList ) const
{
  theList.clear();
  theList.append( Tolerance );
}

bool SMESHGUI_FilterTable::AdditionalWidget::IsValid( const bool theMsg ) const
{
  if ( !isEnabled() )
    return true;

  QValueList<int> aParams;
  GetParameters( aParams );
  QValueList<int>::const_iterator anIter;
  for ( anIter = aParams.begin(); anIter != aParams.end(); ++anIter )
  {
    const QLineEdit* aWg = myLineEdits[ *anIter ];
    int p = 0;
    QString aText = aWg->text();
    if ( aWg->isEnabled() && aWg->validator()->validate( aText, p ) != QValidator::Acceptable )
    {
      if ( theMsg )
          QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
            tr( "SMESH_INSUFFICIENT_DATA" ), tr( "SMESHGUI_INVALID_PARAMETERS" ), QMessageBox::Ok );
        return false;
    }
  }

  return true;
}

double SMESHGUI_FilterTable::AdditionalWidget::GetDouble( const int theId ) const
{
  return myLineEdits.contains( theId ) ? myLineEdits[ theId ]->text().toDouble() : 0;
}

int SMESHGUI_FilterTable::AdditionalWidget::GetInteger( const int theId ) const
{
  return myLineEdits.contains( theId ) ? myLineEdits[ theId ]->text().toInt() : 0;
}

QString SMESHGUI_FilterTable::AdditionalWidget::GetString( const int theId ) const
{
  return myLineEdits.contains( theId ) ? myLineEdits[ theId ]->text() : QString("");
}

void SMESHGUI_FilterTable::AdditionalWidget::SetDouble( const int theId, const double theVal )
{
  if ( myLineEdits.contains( theId ) )
    myLineEdits[ theId ]->setText( QString( "%1" ).arg( theVal ) );
}

void SMESHGUI_FilterTable::AdditionalWidget::SetInteger( const int theId, const int theVal )
{
  if ( myLineEdits.contains( theId ) )
    myLineEdits[ theId ]->setText( QString( "%1" ).arg( theVal ) );
}

void SMESHGUI_FilterTable::AdditionalWidget::SetString( const int theId, const QString& theVal )
{
  if ( myLineEdits.contains( theId ) )
    myLineEdits[ theId ]->setText( theVal );
}

void SMESHGUI_FilterTable::AdditionalWidget::SetEditable( const int theId, const bool isEditable )
{
  if ( myLineEdits.contains( theId ) )
    myLineEdits[ theId ]->setEdited( isEditable );
}

void SMESHGUI_FilterTable::AdditionalWidget::SetEditable( const bool isEditable )
{
  QValueList<int> aParams;
  GetParameters( aParams );
  QValueList<int>::const_iterator anIter;
  for ( anIter = aParams.begin(); anIter != aParams.end(); ++anIter )
    myLineEdits[ *anIter ]->setEdited( isEditable );
}

/*
  Class       : SMESHGUI_FilterTable::ComboItem
  Description : Combo table item. Identificator corresponding to string may be assigned
*/

class SMESHGUI_FilterTable::ComboItem : public QComboTableItem

{
public:
                          ComboItem( QTable*, const QMap<int, QString>& );
  virtual                 ~ComboItem();

  virtual void            setStringList ( const QStringList & l );
  void                    setStringList( const QMap<int, QString>& theIds );

  int                     GetValue() const;
  void                    SetValue( const int );
  
private:
  
  QMap<int, int>          myNumToId;
  QMap<int, int>          myIdToNum;
};

SMESHGUI_FilterTable::ComboItem::ComboItem( QTable*                   theParent, 
                                            const QMap<int, QString>& theIds )
: QComboTableItem( theParent, QStringList() )
{
  setStringList( theIds );
}

void SMESHGUI_FilterTable::ComboItem::setStringList( const QStringList & l )
{
  QComboTableItem::setStringList( l );
}

void SMESHGUI_FilterTable::ComboItem::setStringList( const QMap<int, QString>& theIds )
{
  int i = 0;
  QStringList aList;
  QMap<int, QString>::const_iterator anIter;
  for ( anIter = theIds.begin(); anIter != theIds.end(); ++anIter )
  {
    myNumToId[ i ] = anIter.key();
    myIdToNum[ anIter.key() ] = i;
    aList.append( anIter.data() );
    i++;
  }

  setStringList( aList );
}

SMESHGUI_FilterTable::ComboItem::~ComboItem()
{
}
  
int SMESHGUI_FilterTable::ComboItem::GetValue() const
{
  return myNumToId[ currentItem() ];
}

void SMESHGUI_FilterTable::ComboItem::SetValue( const int theVal )
{
  setCurrentItem( myIdToNum[ theVal ] );
}


/*
  Class       : SMESHGUI_FilterTable::Table            
  Description : Table used by this widget
*/

class SMESHGUI_FilterTable::Table : public QTable
{
public:
                          Table( QWidget* parent );
                          Table( int numRows, int numCols, QWidget* parent = 0 );
  virtual                 ~Table();

  void                    SetEditable( const bool state, const int row, const int col );
  bool                    IsEditable( const int row, const int col ) const;  

  virtual void            insertRows( int row, int count = 1 );
  virtual QString         text( int row, int col ) const;
};

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::Table
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterTable::Table::Table( QWidget* parent )
: QTable( parent, "SMESHGUI_FilterTable::Table" )
{
}

SMESHGUI_FilterTable::Table::Table( int numRows, int numCols, QWidget* parent )
: QTable( numRows, numCols, parent, "SMESHGUI_FilterTable::Table" )
{
}

SMESHGUI_FilterTable::Table::~Table()
{
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::SetEditable
// Purpose : Set editable of specified cell
//=======================================================================
void SMESHGUI_FilterTable::Table::SetEditable( const bool isEditable, const int row, const int col )
{
  QTableItem* anItem = item( row, col );
  if( anItem )
    takeItem( anItem );

  if ( !isEditable )
    setItem( row, col, new QTableItem( this, QTableItem::Never, "" ) );
  else
    setItem( row, col, new QTableItem( this, QTableItem::OnTyping, "" ) );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::IsEditable
// Purpose : Verify wheter cell is editable
//=======================================================================
bool SMESHGUI_FilterTable::Table::IsEditable( const int row, const int col ) const
{
  QTableItem* anItem = item( row, col );
  return anItem == 0 || anItem->editType() != QTableItem::Never;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::insertRows
// Purpose : Insert rows ( virtual redefined )
//=======================================================================
void SMESHGUI_FilterTable::Table::insertRows( int row, int count )
{
  int anEditRow = currEditRow();
  int anEditCol = currEditCol();

  if ( anEditRow >= 0 && anEditCol >= 0 )
    endEdit( anEditRow, anEditCol, true, false );

  QTable::insertRows( row, count );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::text
// Purpose : Get text from cell ( virtual redefined ) 
//=======================================================================
QString SMESHGUI_FilterTable::Table::text( int row, int col ) const
{
  int anEditRow = currEditRow();
  int anEditCol = currEditCol();

  if ( anEditRow >= 0 && anEditCol >= 0 && anEditRow == row && anEditCol == col )
    ((Table*)this)->endEdit( row, col, true, false );

  return QTable::text( row, col );
}



/*
  Class       : SMESHGUI_FilterTable
  Description : Frame containig 
                  - Button group for switching entity type
                  - Table for displaying filter criterions
                  - Buttons for editing table and filter libraries
*/

//=======================================================================
// name    : SMESHGUI_FilterTable::SMESHGUI_FilterTable
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterTable::SMESHGUI_FilterTable( QWidget* parent, 
                                            const int type )
: QFrame( parent )                                            
{
  myEntityType = -1;
  Init( type );
}                 

//=======================================================================
// name    : SMESHGUI_FilterTable::SMESHGUI_FilterTable
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterTable::SMESHGUI_FilterTable( QWidget* parent, 
                                            const QValueList<int>& types )
: QFrame( parent )                                            
{
  myEntityType = -1;
  Init( types );
}                                            

SMESHGUI_FilterTable::~SMESHGUI_FilterTable()
{
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Init
// Purpose : Create table corresponding to the specified type
//=======================================================================
void SMESHGUI_FilterTable::Init( const int type )
{
  QValueList<int> aTypes;
  aTypes.append( type );
  Init( aTypes );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Init
// Purpose : Create table corresponding to the specified type
//=======================================================================
void SMESHGUI_FilterTable::Init( const QValueList<int>& theTypes )
{
  if ( theTypes.isEmpty() )
    return;
  
  // Create buttons if necessary
  
  if ( myTables.isEmpty() ) 
  {
    int aType = theTypes.first();

    // create main layout
    QVBoxLayout* aMainLay = new QVBoxLayout( this );
    QGroupBox* aMainGrp = new QGroupBox( 1, Qt::Horizontal, this );
    aMainGrp->setFrameStyle( QFrame::NoFrame );
    aMainGrp->setInsideMargin( 0 );
    aMainLay->addWidget( aMainGrp );
    
    // create switch of entity types
    myEntityTypeGrp = new QButtonGroup( 1, Qt::Vertical, tr( "ENTITY_TYPE" ), aMainGrp );
    const QMap<int, QString>& aSupportedTypes = getSupportedTypes();
    QMap<int, QString>::const_iterator anIter;
    for ( anIter = aSupportedTypes.begin(); anIter != aSupportedTypes.end(); ++anIter )
    {
      QRadioButton* aBtn = new QRadioButton( anIter.data(), myEntityTypeGrp );
      myEntityTypeGrp->insert( aBtn, anIter.key() );
    }

    myTableGrp = new QGroupBox( 1, Qt::Horizontal, tr( "FILTER" ), aMainGrp  );
    QFrame* aTableFrame = new QFrame( myTableGrp );
  
    // create table
    mySwitchTableGrp = new QGroupBox( 1, Qt::Horizontal, aTableFrame );
    mySwitchTableGrp->setFrameStyle( QFrame::NoFrame );
    mySwitchTableGrp->setInsideMargin( 0 );
    
    myTables[ aType ] = createTable( mySwitchTableGrp, aType );
  
    // create buttons
    myAddBtn      = new QPushButton( tr( "ADD" ), aTableFrame );
    myRemoveBtn   = new QPushButton( tr( "REMOVE" ), aTableFrame );
    myClearBtn    = new QPushButton( tr( "CLEAR" ), aTableFrame );
    myInsertBtn   = new QPushButton( tr( "INSERT" ), aTableFrame );
    myCopyFromBtn = new QPushButton( tr( "COPY_FROM" ), aTableFrame );
    myAddToBtn    = new QPushButton( tr( "ADD_TO" ), aTableFrame );

    myAddBtn->setAutoDefault( false );
    myRemoveBtn->setAutoDefault( false );
    myClearBtn->setAutoDefault( false );
    myInsertBtn->setAutoDefault( false );
    myCopyFromBtn->setAutoDefault( false );
    myAddToBtn->setAutoDefault( false );

    myCopyFromBtn->hide();
    myAddToBtn->hide();
    
    // layout widgets
    QGridLayout* aLay = new QGridLayout( aTableFrame, 8, 2, 0, SPACING );

    aLay->addMultiCellWidget( mySwitchTableGrp, 0, 6, 0, 0 );
    aLay->addWidget( myAddBtn, 0, 1 );
    aLay->addWidget( myInsertBtn, 1, 1 );
    aLay->addWidget( myRemoveBtn, 2, 1 );
    aLay->addWidget( myClearBtn, 3, 1 );
    aLay->addWidget( myCopyFromBtn, 5, 1 );
    aLay->addWidget( myAddToBtn, 6, 1 );
    aLay->addMultiCellWidget( createAdditionalFrame( aTableFrame ), 7, 7, 0, 1  );
    
    aLay->setColStretch( 0, 1 );
    aLay->setColStretch( 1, 0 );
    
    QSpacerItem* aVSpacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    aLay->addItem( aVSpacer, 4, 1 );
    
    // signals and slots
    connect( myAddBtn,    SIGNAL( clicked() ), this, SLOT( onAddBtn() ) );
    connect( myInsertBtn, SIGNAL( clicked() ), this, SLOT( onInsertBtn() ) );
    connect( myRemoveBtn, SIGNAL( clicked() ), this, SLOT( onRemoveBtn() ) );
    connect( myClearBtn,  SIGNAL( clicked() ), this, SLOT( onClearBtn() ) );
  
    connect( myCopyFromBtn, SIGNAL( clicked() ), this, SLOT( onCopyFromBtn() ) );
    connect( myAddToBtn,    SIGNAL( clicked() ), this, SLOT( onAddToBtn() ) );
    
    connect( myEntityTypeGrp, SIGNAL( clicked( int ) ), this, SLOT( onEntityType( int ) ) );
    
    myLibDlg = 0;
  }
  
  // Hide buttons of entity types if necessary
  const QMap<int, QString>& aSupportedTypes = getSupportedTypes();
  QMap<int, QString>::const_iterator anIt;
  for ( anIt = aSupportedTypes.begin(); anIt != aSupportedTypes.end(); ++anIt )
  {
    QButton* aBtn = myEntityTypeGrp->find( anIt.key() );
    theTypes.contains( anIt.key() ) ? aBtn->show() : aBtn->hide();
  }

  // select first button if there is no selected buttons or it is hidden
  QButton* aBtn = myEntityTypeGrp->selected();
  if (  aBtn == 0 || theTypes.find( myEntityTypeGrp->id( aBtn ) ) == theTypes.end() )
    myEntityTypeGrp->setButton( theTypes.first() );

  if ( theTypes.count() == 1 )
    myEntityTypeGrp->hide();
  else
    myEntityTypeGrp->show();

  myTableGrp->updateGeometry();
  int aType = myEntityTypeGrp->id( myEntityTypeGrp->selected() );
  onEntityType( aType );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetTableGrp
// Purpose : Get group box containing table. May be used for adding new widgets in it
////=======================================================================
QWidget* SMESHGUI_FilterTable::createAdditionalFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );
  
  QFrame* aLine1 = new QFrame( aFrame );
  QFrame* aLine2 = new QFrame( aFrame );
  aLine1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  aLine2->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  aLine1->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed) );
  aLine2->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed) );
  
  QLabel* aLabel = new QLabel( tr( "ADDITIONAL_PARAMETERS" ), aFrame );

  myWgStack = new QWidgetStack( aFrame );

  QGridLayout* aLay = new QGridLayout( aFrame, 2, 3, 0, SPACING );
  aLay->addWidget( aLine1, 0, 0 );
  aLay->addWidget( aLabel, 0, 1 );
  aLay->addWidget( aLine2, 0, 2 );
  aLay->addMultiCellWidget( myWgStack, 1, 1, 0, 2 );

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetTableGrp
// Purpose : Get group box containing table. May be used for adding new widgets in it
////=======================================================================
QGroupBox* SMESHGUI_FilterTable::GetTableGrp()
{
  return myTableGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onEntityType
// Purpose : SLOT. Called when entity type changed.
//           Display corresponding table 
//=======================================================================
void SMESHGUI_FilterTable::onEntityType( int theType )
{
  if ( myEntityType == theType )
    return;

  myIsValid = true;
  emit NeedValidation();
  if ( !myIsValid )
  {
    myEntityTypeGrp->setButton( myEntityType );
    return;
  }

  myEntityType = theType;
  
  if ( !myTables.contains( theType ) )
    myTables[ theType ] = createTable( mySwitchTableGrp, theType );    
  
  TableMap::iterator anIter;
  for ( anIter = myTables.begin(); anIter != myTables.end(); ++anIter )
    myEntityType == anIter.key() ? anIter.data()->show() : anIter.data()->hide();

  updateBtnState();
  qApp->processEvents();
  myTables[ myEntityType ]->updateGeometry();
  adjustSize();

  emit EntityTypeChanged( theType );

}

//=======================================================================
// name    : SMESHGUI_FilterTable::IsValid
// Purpose : Verify validity of entered data
//=======================================================================
bool SMESHGUI_FilterTable::IsValid( const bool theMess, const int theEntityType ) const
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;

  Table* aTable = myTables[ aType ];
  for ( int i = 0, n = aTable->numRows(); i < n; i++ )
  {
    int aCriterion = GetCriterionType( i, aType );

    if ( aCriterion == FT_RangeOfIds ||
         aCriterion == FT_BelongToGeom ||
         aCriterion == FT_BelongToPlane ||
         aCriterion == FT_BelongToCylinder ||
	 aCriterion == FT_LyingOnGeom)
    {
      if ( aTable->text( i, 2 ).isEmpty() )
      {
        if ( theMess )
          QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
            tr( "SMESH_INSUFFICIENT_DATA" ), tr( "ERROR" ), QMessageBox::Ok );
        return false;
      }
    }
    else
    {
      bool aRes = false;
      aTable->blockSignals( true );
      double  aThreshold = ( int )aTable->text( i, 2 ).toDouble( &aRes );
      aTable->blockSignals( false );

      if ( !aRes && aTable->IsEditable( i, 2 ) )
      {
        if ( theMess )
          QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
            tr( "SMESH_INSUFFICIENT_DATA" ), tr( "ERROR" ), QMessageBox::Ok );
        return false;
      }
      else if ( aType == SMESH::EDGE &&
                GetCriterionType( i, aType ) == SMESH::FT_MultiConnection &&
                aThreshold == 1 )
      {
        if ( theMess )
          QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
            tr( "SMESH_INSUFFICIENT_DATA" ), tr( "MULTIEDGES_ERROR" ), QMessageBox::Ok );
        return false;
      }
    }

    QTableItem* anItem = aTable->item( i, 0 );
    if ( myAddWidgets.contains( anItem ) && !myAddWidgets[ anItem ]->IsValid() )
      return false;
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetValidity
// Purpose : Set validity of the table
//=======================================================================
void SMESHGUI_FilterTable::SetValidity( const bool isValid )
{
  myIsValid = isValid;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetType
// Purpose : Get current entity type
//=======================================================================
int SMESHGUI_FilterTable::GetType() const
{
  return myEntityType;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetType
// Purpose : Set current entity type
//=======================================================================
void SMESHGUI_FilterTable::SetType( const int type )
{
  myEntityTypeGrp->setButton( type );
  onEntityType( type );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::RestorePreviousEntityType
// Purpose : Restore previous entity type
//=======================================================================
void SMESHGUI_FilterTable::RestorePreviousEntityType()
{
  SetType( myEntityType );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetCriterionType
// Purpose : Get type of criterion from specified row ( corresponding enums in h-file )
//=======================================================================
int SMESHGUI_FilterTable::GetCriterionType( const int theRow, const int theType ) const
{
  int aType = theType == -1 ? GetType() : theType;
  Table* aTable = myTables[ aType ];
  ComboItem* anItem = (ComboItem*)aTable->item( theRow, 0 );
  return anItem != 0 ? anItem->GetValue() : FT_Undefined;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetCriterion
// Purpose : Get parameters of criterion from specified row
//=======================================================================
void SMESHGUI_FilterTable::GetCriterion( const int                 theRow,
                                         SMESH::Filter::Criterion& theCriterion,
                                         const int                 theEntityType ) const
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;
  Table* aTable = myTables[ aType ];
  
  theCriterion.Type = ( (ComboItem*)aTable->item( theRow, 0 ) )->GetValue();
  theCriterion.UnaryOp = ( (QCheckTableItem*)aTable->item( theRow, 3 ) )->isChecked() ? FT_LogicalNOT : FT_Undefined;
  theCriterion.BinaryOp = theRow != aTable->numRows() - 1 ?
    ( (ComboItem*)aTable->item( theRow, 4 ) )->GetValue() : FT_Undefined;
  theCriterion.TypeOfElement = (ElementType)aType;

  int aCriterionType = GetCriterionType( theRow, aType );

  if ( aCriterionType != FT_RangeOfIds &&
       aCriterionType != FT_BelongToGeom &&
       aCriterionType != FT_BelongToPlane &&
       aCriterionType != FT_BelongToCylinder &&
       aCriterionType != FT_LyingOnGeom)
  {
    theCriterion.Compare = ( (ComboItem*)aTable->item( theRow, 1 ) )->GetValue();
    theCriterion.Threshold = aTable->item( theRow, 2 )->text().toDouble();
  }
  else
    theCriterion.ThresholdStr = aTable->text( theRow, 2 ).latin1();

  QTableItem* anItem = aTable->item( theRow, 0 );
  if ( myAddWidgets.contains( anItem ) )
    theCriterion.Tolerance = myAddWidgets[ anItem ]->GetDouble( AdditionalWidget::Tolerance );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetCriterion
// Purpose : Set parameters of criterion of specified row
//=======================================================================
void SMESHGUI_FilterTable::SetCriterion( const int                       theRow,
                                         const SMESH::Filter::Criterion& theCriterion,
                                         const int                       theEntityType )
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;

  Table* aTable = myTables[ aType ];

  if ( theRow > aTable->numRows() - 1 )
    return;

  ( (ComboItem*)aTable->item( theRow, 0 ) )->SetValue( theCriterion.Type );
  onCriterionChanged( theRow, 0, aType );
  ( (ComboItem*)aTable->item( theRow, 1 ) )->SetValue( theCriterion.Compare );
  ( (QCheckTableItem*)aTable->item( theRow, 3 ) )->setChecked( theCriterion.UnaryOp == FT_LogicalNOT );

  if ( theCriterion.BinaryOp != FT_Undefined )
  {
    if ( !aTable->IsEditable( theRow, 4 ) )
      aTable->setItem( theRow, 4, getBinaryItem( aTable ) );
    ( (ComboItem*)aTable->item( theRow, 4 ) )->SetValue( theCriterion.BinaryOp );
  }
  else
    aTable->SetEditable( false, theRow, 4 );

  if ( theCriterion.Type != FT_RangeOfIds &&
       theCriterion.Type != FT_BelongToGeom &&
       theCriterion.Type != FT_BelongToPlane &&
       theCriterion.Type != FT_BelongToCylinder &&
       theCriterion.Type != FT_LyingOnGeom)
    aTable->setText( theRow, 2, QString( "%1" ).arg( theCriterion.Threshold, 0, 'g', 15 ) );
  else
    aTable->setText( theRow, 2, QString( theCriterion.ThresholdStr ) );

  if ( theCriterion.Compare == FT_EqualTo ||
       theCriterion.Type    == FT_BelongToPlane ||
       theCriterion.Type    == FT_BelongToCylinder )
  {
    QTableItem* anItem = aTable->item( theRow, 0 );
    if ( !myAddWidgets.contains( anItem ) )
    {
      myAddWidgets[ anItem ] = new AdditionalWidget( myWgStack );
      myWgStack->addWidget( myAddWidgets[ anItem ] );
    }
    myAddWidgets[ anItem ]->SetDouble( AdditionalWidget::Tolerance, theCriterion.Tolerance );
  }

  emit CretarionChanged( theRow, aType );

}

//=======================================================================
// name    : SMESHGUI_FilterTable::Update
// Purpose : Update table
//=======================================================================
void SMESHGUI_FilterTable::Update()
{
  Table* aTable = myTables[ GetType() ];
  int aCurrRow = aTable->currentRow();
  int numRows = aTable->numRows();
  if ( ( aCurrRow < 0 || aCurrRow >= numRows ) && numRows > 0 )
    aTable->setCurrentCell( 0, 0 );
  updateAdditionalWidget();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::AddCriterion
// Purpose : Add criterion with parameters
//=======================================================================
void SMESHGUI_FilterTable::AddCriterion( const SMESH::Filter::Criterion& theCriterion,
                                         const int                       theEntityType )
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;
  Table* aTable = myTables[ aType ];
  addRow( aTable, aType );
  SetCriterion( aTable->numRows() - 1, theCriterion );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::NumRows
// Purpose : Get number of criterions of current type
//=======================================================================
int SMESHGUI_FilterTable::NumRows( const int theEntityType ) const
{
  return myTables[ theEntityType == -1 ? GetType() : theEntityType ]->numRows();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Clear
// Purpose : Clear current table
//=======================================================================
void SMESHGUI_FilterTable::Clear( const int theType )
{
  int aType = theType == -1 ? GetType() : theType;
  QTable* aTable = myTables[ aType ];

  if ( aTable->numRows() == 0 )
    return;

  while ( aTable->numRows() > 0 )
  {
    removeAdditionalWidget( aTable, 0 );
    aTable->removeRow( 0 );
  }

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onAddBtn
// Purpose : SLOT. Called then "Add" button pressed.
//           Adds new string to table
//=======================================================================
void SMESHGUI_FilterTable::onAddBtn()
{
  int aType = GetType();
  addRow( myTables[ aType ], aType );

  Update();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onInsertBtn
// Purpose : SLOT. Called then "Insert" button pressed.
//           Inserts new string before current one
//=======================================================================
void SMESHGUI_FilterTable::onInsertBtn()
{
  addRow( myTables[ GetType() ], GetType(), false );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onRemoveBtn
// Purpose : SLOT. Called then "Remove" button pressed.
//           Removes current string from table
//=======================================================================
void SMESHGUI_FilterTable::onRemoveBtn()
{
  Table* aTable = myTables[ GetType() ];

  if ( aTable->numRows() == 0 )
    return;

  QMemArray<int> aRows;
  for ( int i = 0, n = aTable->numRows(); i < n; i++ )
  {
    if ( aTable->isRowSelected( i ) )
    {
      aRows.resize( aRows.size() + 1 );
      aRows[ aRows.size() - 1 ] = i;
      removeAdditionalWidget( aTable, i );
    }
  }

  aTable->removeRows( aRows );

  // remove control of binary logical operation from last row
  if ( aTable->numRows() > 0 )
    aTable->SetEditable( false, aTable->numRows() - 1, 4 );

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::updateAdditionalWidget
// Purpose : Enable/Disable widget with additonal parameters
//=======================================================================
void SMESHGUI_FilterTable::updateAdditionalWidget()
{
  Table* aTable = myTables[ GetType() ];
  int aRow = aTable->currentRow();
  if ( aRow < 0 || aRow >= aTable->numRows() )
  {
    myWgStack->setEnabled( false );
    return;
  }

  ComboItem* anItem = ( (ComboItem*)aTable->item( aRow, 0 ) );
  bool toEnable = ( (ComboItem*)aTable->item( aRow, 1 ) )->GetValue() == FT_EqualTo &&
                  GetCriterionType( aRow ) != FT_BelongToGeom &&
                  GetCriterionType( aRow ) != FT_LyingOnGeom &&
                  GetCriterionType( aRow ) != FT_RangeOfIds &&
                  GetCriterionType( aRow ) != FT_FreeEdges;
  if ( !myAddWidgets.contains( anItem ) )
  {
    myAddWidgets[ anItem ] = new AdditionalWidget( myWgStack );
    myWgStack->addWidget( myAddWidgets[ anItem ] );
  }

  myWgStack->raiseWidget( myWgStack->id( myAddWidgets[ anItem ] ) );
  myWgStack->setEnabled( toEnable );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::removeAdditionalWidget
// Purpose : Remove widgets containing additional parameters from widget
//           stack and internal map
//=======================================================================
void SMESHGUI_FilterTable::removeAdditionalWidget( QTable* theTable, const int theRow )
{
  QTableItem* anItem = theTable->item( theRow, 0 );
  if ( myAddWidgets.contains( anItem ) )
  {
    myWgStack->removeWidget( myAddWidgets[ anItem ] );
    myAddWidgets[ anItem ]->reparent( 0, QPoint() );
    delete myAddWidgets[ anItem ];
    myAddWidgets.remove( anItem );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onClearBtn
// Purpose : SLOT. Called then "Clear" button pressed.
//           Removes all strings from table
//=======================================================================
void SMESHGUI_FilterTable::onClearBtn()
{
  QTable* aTable = myTables[ GetType() ];

  if ( aTable->numRows() == 0 )
    return;

  while ( aTable->numRows() > 0 )
  {
    removeAdditionalWidget( aTable, 0 );
    aTable->removeRow( 0 );
  }

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCurrentChanged()
// Purpose : SLOT. Called when current cell changed
//=======================================================================
void SMESHGUI_FilterTable::onCurrentChanged( int theRow, int theCol )
{
  updateAdditionalWidget();
  emit CurrentChanged( theRow, theCol );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCriterionChanged()
// Purpose : Provides reaction on change of criterion
//=======================================================================
void SMESHGUI_FilterTable::onCriterionChanged( const int row, const int col, const int entityType )
{
  int aType = entityType == -1 ? GetType() : entityType;
  Table* aTable = myTables[ aType ];
  ComboItem* aCompareItem = (ComboItem*)aTable->item( row, 1 );

  int aCriterionType = GetCriterionType( row );

  if ( aType == SMESH::EDGE && aCriterionType == SMESH::FT_FreeBorders ||
       aType == SMESH::FACE && aCriterionType == SMESH::FT_FreeEdges )
  {
    if ( aCompareItem->count() > 0 )
      aCompareItem->setStringList( QStringList() );
    aTable->SetEditable( false, row, 2 );
  }
  else if ( aCriterionType == SMESH::FT_RangeOfIds ||
            aCriterionType == SMESH::FT_BelongToGeom ||
            aCriterionType == SMESH::FT_BelongToPlane ||
            aCriterionType == SMESH::FT_BelongToCylinder ||
	    aCriterionType == SMESH::FT_LyingOnGeom)
  {
    QMap<int, QString> aMap;
    aMap[ FT_EqualTo ] = tr( "EQUAL_TO" );
    aCompareItem->setStringList( aMap );
    if ( !aTable->IsEditable( row, 2 ) )
      aTable->SetEditable( true, row, 2 );
  }
  else
  {
    if ( aCompareItem->count() != 3 )
    {
      aCompareItem->setStringList( QStringList() );
      aCompareItem->setStringList( getCompare() );
    }

    QString aText = aTable->text( row, 2 );
    bool isOk = false;
    aText.toDouble( &isOk );
    aTable->setText( row, 2, isOk ? aText : QString("") );
    if ( !aTable->IsEditable( row, 2 ) )
      aTable->SetEditable( true, row, 2 );
  }

  updateAdditionalWidget();
  
  emit CretarionChanged( row, entityType );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCriterionChanged()
// Purpose : SLOT. Called then contents of table changed
//           Provides reaction on change of criterion
//=======================================================================
void SMESHGUI_FilterTable::onCriterionChanged( int row, int col )
{
  onCriterionChanged( row, col, -1 );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getFirstSelectedRow
// Purpose : Get first selected row
//=======================================================================
int SMESHGUI_FilterTable::getFirstSelectedRow() const
{
  QTable* aTable = myTables[ GetType() ];
  for ( int i = 0, n = aTable->numRows(); i < n; i++ )
    if ( aTable->isRowSelected( i ) )
      return i;

  int aRow = aTable->currentRow();
  return aRow >= 0 && aRow < aTable->numRows() ? aRow : -1;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::addRow
// Purpose : Add row at the end of table
//=======================================================================
void SMESHGUI_FilterTable::addRow( Table* theTable, const int theType, const bool toTheEnd )
{
  int aCurrRow = 0;
  int aSelectedRow = getFirstSelectedRow();
  int aCurrCol = theTable->currentColumn();

  if ( toTheEnd || aSelectedRow == -1 )
  {
    theTable->insertRows( theTable->numRows() );
    aCurrRow = theTable->numRows() - 1;
  }
  else
  {
    theTable->insertRows( aSelectedRow );
    aCurrRow = aSelectedRow;
  }

  // Criteria
  theTable->setItem( aCurrRow, 0, getCriterionItem( theTable, theType ) );

  // Compare
  theTable->setItem( aCurrRow, 1, getCompareItem( theTable ) );

  // Threshold
  //theTable->setItem( aCurrRow, 2, new QTableItem( theTable ) );  

  //Logical operation NOT
  theTable->setItem( aCurrRow, 3, getUnaryItem( theTable ) );
  
  // Logical binary operation for previous value
  int anAddBinOpStr = -1;
  if ( aCurrRow == theTable->numRows() - 1 )
    anAddBinOpStr = aCurrRow - 1;
  else if ( aCurrRow >= 0  )
    anAddBinOpStr = aCurrRow;
  
  if ( theTable->item( aCurrRow, 4 ) == 0 ||
       theTable->item( aCurrRow, 4 )->rtti() != 1 )
  {
    

    if ( anAddBinOpStr >= 0 &&
         ( theTable->item( anAddBinOpStr, 4 ) == 0 ||
           theTable->item( anAddBinOpStr, 4 )->rtti() != 1 ) )
      theTable->setItem( anAddBinOpStr, 4, getBinaryItem( theTable ) );
  }
  
  theTable->SetEditable( false, theTable->numRows() - 1, 4 );

  if ( aCurrRow >=0 && aCurrRow < theTable->numRows() &&
       aCurrCol >=0 && aCurrCol < theTable->numRows() )
  theTable->setCurrentCell( aCurrRow, aCurrCol );

  onCriterionChanged( aCurrRow, 0 );

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getCriterionItem
// Purpose : Get combo table item for criteria of specified type
//=======================================================================
QTableItem* SMESHGUI_FilterTable::getCriterionItem( QTable* theParent , const int theType )
{
  return new ComboItem( theParent, getCriteria( theType ) );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getCompareItem
// Purpose : Get combo table item for operation of comparision
//=======================================================================
QTableItem* SMESHGUI_FilterTable::getCompareItem( QTable* theParent )
{
  return new ComboItem( theParent, getCompare() );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getBinaryItem
// Purpose :
//=======================================================================
QTableItem* SMESHGUI_FilterTable::getBinaryItem( QTable* theParent )
{
  static QMap<int, QString> aMap;
  if ( aMap.isEmpty() )
  {
    aMap[ SMESH::FT_LogicalAND ] = tr( "AND" );
    aMap[ SMESH::FT_LogicalOR  ] = tr( "OR" );
  }

  return new ComboItem( theParent, aMap );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getUnaryItem
// Purpose : Get check table item
//=======================================================================
QTableItem* SMESHGUI_FilterTable::getUnaryItem( QTable* theParent )
{
  return new QCheckTableItem( theParent, tr( "NOT" ) );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getSupportedTypes
// Purpose : Get all supported type
//=======================================================================
const QMap<int, QString>& SMESHGUI_FilterTable::getSupportedTypes() const
{
  static QMap<int, QString> aTypes;
  if ( aTypes.isEmpty() )
  {
    aTypes[ SMESH::NODE   ] = tr( "NODES" );
    aTypes[ SMESH::EDGE   ] = tr( "EDGES" );
    aTypes[ SMESH::FACE   ] = tr( "FACES" );
    aTypes[ SMESH::VOLUME ] = tr( "VOLUMES" );
  }

  return aTypes;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getCriteria
// Purpose : Get criteria for specified type
//=======================================================================
const QMap<int, QString>& SMESHGUI_FilterTable::getCriteria( const int theType ) const
{
  if ( theType == SMESH::NODE )
  {
    static QMap<int, QString> aCriteria;
    if ( aCriteria.isEmpty() )
    {
      aCriteria[ SMESH::FT_RangeOfIds       ] = tr( "RANGE_OF_IDS" );
      aCriteria[ SMESH::FT_BelongToGeom     ] = tr( "BELONG_TO_GEOM" );
      aCriteria[ SMESH::FT_BelongToPlane    ] = tr( "BELONG_TO_PLANE" );
      aCriteria[ SMESH::FT_BelongToCylinder ] = tr( "BELONG_TO_CYLINDER" );
      aCriteria[ SMESH::FT_LyingOnGeom      ] = tr( "LYING_ON_GEOM" );
    }
    return aCriteria;
  }
  else if ( theType == SMESH::EDGE )
  {
    static QMap<int, QString> aCriteria;
    if ( aCriteria.isEmpty() )
    {
      aCriteria[ SMESH::FT_FreeBorders      ] = tr( "FREE_BORDERS" );
      aCriteria[ SMESH::FT_MultiConnection  ] = tr( "MULTI_BORDERS" );
      aCriteria[ SMESH::FT_Length           ] = tr( "LENGTH" );
      aCriteria[ SMESH::FT_RangeOfIds       ] = tr( "RANGE_OF_IDS" );
      aCriteria[ SMESH::FT_BelongToGeom     ] = tr( "BELONG_TO_GEOM" );
      aCriteria[ SMESH::FT_BelongToPlane    ] = tr( "BELONG_TO_PLANE" );
      aCriteria[ SMESH::FT_BelongToCylinder ] = tr( "BELONG_TO_CYLINDER" );
      aCriteria[ SMESH::FT_LyingOnGeom      ] = tr( "LYING_ON_GEOM" );
    }
    return aCriteria;
  }
  else if ( theType == SMESH::FACE )
  {
    static QMap<int, QString> aCriteria;
    if ( aCriteria.isEmpty() )
    {
      aCriteria[ SMESH::FT_AspectRatio      ] = tr( "ASPECT_RATIO" );
      aCriteria[ SMESH::FT_Warping          ] = tr( "WARPING" );
      aCriteria[ SMESH::FT_MinimumAngle     ] = tr( "MINIMUM_ANGLE" );
      aCriteria[ SMESH::FT_Taper            ] = tr( "TAPER" );
      aCriteria[ SMESH::FT_Skew             ] = tr( "SKEW" );
      aCriteria[ SMESH::FT_Area             ] = tr( "AREA" );
      aCriteria[ SMESH::FT_FreeEdges        ] = tr( "FREE_EDGES" );
      aCriteria[ SMESH::FT_RangeOfIds       ] = tr( "RANGE_OF_IDS" );
      aCriteria[ SMESH::FT_BelongToGeom     ] = tr( "BELONG_TO_GEOM" );
      aCriteria[ SMESH::FT_BelongToPlane    ] = tr( "BELONG_TO_PLANE" );
      aCriteria[ SMESH::FT_BelongToCylinder ] = tr( "BELONG_TO_CYLINDER" );
      aCriteria[ SMESH::FT_LyingOnGeom      ] = tr( "LYING_ON_GEOM" );
      aCriteria[ SMESH::FT_Length2D         ] = tr( "LENGTH2D" );
      aCriteria[ SMESH::FT_MultiConnection2D] = tr( "MULTI2D_BORDERS" );
    }
    return aCriteria;
  }
  else if ( theType == SMESH::VOLUME )
  {
    static QMap<int, QString> aCriteria;
    if ( aCriteria.isEmpty() )
    {
      aCriteria[ SMESH::FT_AspectRatio3D] = tr( "ASPECT_RATIO_3D" );
      aCriteria[ SMESH::FT_RangeOfIds   ] = tr( "RANGE_OF_IDS" );
      aCriteria[ SMESH::FT_BelongToGeom ] = tr( "BELONG_TO_GEOM" );
      aCriteria[ SMESH::FT_LyingOnGeom ] = tr( "LYING_ON_GEOM" );
    }
    return aCriteria;
  }
  else
  {
    static QMap<int, QString> aCriteria;
    return aCriteria;
  }
}


//=======================================================================
// name    : SMESHGUI_FilterTable::getCompare
// Purpose : Get operation of comparison
//=======================================================================
const QMap<int, QString>& SMESHGUI_FilterTable::getCompare() const
{
  static QMap<int, QString> aMap;

  if ( aMap.isEmpty() )
  {
    aMap[ SMESH::FT_LessThan ] = tr( "LESS_THAN" );
    aMap[ SMESH::FT_MoreThan ] = tr( "MORE_THAN" );
    aMap[ SMESH::FT_EqualTo  ] = tr( "EQUAL_TO"  );
  }

  return aMap;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::createTable
// Purpose : Create table
//=======================================================================
SMESHGUI_FilterTable::Table* SMESHGUI_FilterTable::createTable( QWidget*  theParent,
                                                                const int theType )
{
  // create table
  Table* aTable= new Table( 0, 5, theParent );

  QHeader* aHeaders = aTable->horizontalHeader();

  QFontMetrics aMetrics( aHeaders->font() );

  // append spaces to the header of criteria in order to
  // provide visibility of criterion inside comboboxes
  static int aMaxLenCr = 0;

  if ( aMaxLenCr == 0 )
  {
    const QMap<int, QString>& aSupportedTypes = getSupportedTypes();
    QMap<int, QString>::const_iterator anIter;
    for ( anIter = aSupportedTypes.begin(); anIter != aSupportedTypes.end(); ++anIter )
      aMaxLenCr = Max( maxLength( getCriteria( anIter.key() ), aMetrics ), aMaxLenCr );
  }

  static int aLenCr = abs(  aMaxLenCr -
                            aMetrics.width( tr( "CRITERION" ) ) ) / aMetrics.width( ' ' ) + 5;

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

  connect( aTable, SIGNAL( valueChanged( int, int ) ),
           this,   SLOT( onCriterionChanged( int, int ) ) );

  connect( aTable, SIGNAL( currentChanged( int, int ) ),
           this,   SLOT( onCurrentChanged( int, int ) ) );

  return aTable;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::updateBtnState
// Purpose : Update button state
//=======================================================================
void SMESHGUI_FilterTable::updateBtnState()
{
  myRemoveBtn->setEnabled( myTables[ GetType() ]->numRows() > 0 );
  myClearBtn->setEnabled( myTables[ GetType() ]->numRows() > 0 );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetEditable
// Purpose : Set read only flag for tables. Show/hide buttons for work with rows
//=======================================================================
void SMESHGUI_FilterTable::SetEditable( const bool isEditable )
{
  TableMap::iterator anIter;
  for ( anIter = myTables.begin(); anIter != myTables.end(); ++anIter )
  {
    anIter.data()->setReadOnly( !isEditable );

    if ( isEditable )
    {
      myAddBtn->show();
      myInsertBtn->show();
      myRemoveBtn->show();
      myClearBtn->show();
    }
    else
    {
      myAddBtn->hide();
      myInsertBtn->hide();
      myRemoveBtn->hide();
      myClearBtn->hide();
    }
  }

  QMap<QTableItem*, AdditionalWidget*>::iterator anIter2;
  for ( anIter2 = myAddWidgets.begin(); anIter2 != myAddWidgets.end(); ++anIter2 )
    anIter2.data()->SetEditable( isEditable );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetEnabled
// Purpose : Enable/Disable table. Switching type of elements already enabled
//=======================================================================
void SMESHGUI_FilterTable::SetEnabled( const bool isEnabled )
{
  myAddBtn->setEnabled( isEnabled );
  myInsertBtn->setEnabled( isEnabled );
  myRemoveBtn->setEnabled( isEnabled );
  myClearBtn->setEnabled( isEnabled );

  if ( isEnabled )
    updateBtnState();
  
  QMap<QTableItem*, AdditionalWidget*>::iterator anIter2;
  for ( anIter2 = myAddWidgets.begin(); anIter2 != myAddWidgets.end(); ++anIter2 )
    anIter2.data()->setEnabled( isEnabled );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::IsEditable
// Purpose : Verify whether table is editable
//=======================================================================
bool SMESHGUI_FilterTable::IsEditable() const
{
  return !myTables[ GetType() ]->isReadOnly();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetLibsEnabled
// Purpose : Show/hide buttons for work with libraries
//=======================================================================
void SMESHGUI_FilterTable::SetLibsEnabled( const bool isEnabled )
{
  if ( isEnabled )
  {
    myCopyFromBtn->show();
    myAddToBtn->show();
  }
  else
  {
    myCopyFromBtn->hide();
    myAddToBtn->hide();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCopyFromBtn
// Purpose : SLOT. Called the "Copy from ..." button clicked
//           Display filter library dialog
//=======================================================================
void SMESHGUI_FilterTable::onCopyFromBtn()
{
  if ( myLibDlg == 0 )
    myLibDlg = new SMESHGUI_FilterLibraryDlg(
      this, GetType(), SMESHGUI_FilterLibraryDlg::COPY_FROM );
  else
    myLibDlg->Init( GetType(), SMESHGUI_FilterLibraryDlg::COPY_FROM );

  if ( myLibDlg->exec() == QDialog::Accepted )
  {
    Copy( myLibDlg->GetTable() );
    Update();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onAddToBtn
// Purpose : SLOT. Called the "Add to ..." button clicked
//           Display filter library dialog
//=======================================================================
void SMESHGUI_FilterTable::onAddToBtn()
{
  if ( !IsValid( true ) )
    return;
  if ( myLibDlg == 0 )
    myLibDlg = new SMESHGUI_FilterLibraryDlg(
      this, GetType(), SMESHGUI_FilterLibraryDlg::ADD_TO );
  else
    myLibDlg->Init( GetType(), SMESHGUI_FilterLibraryDlg::ADD_TO );

  myLibDlg->SetTable( this );
    
  myLibDlg->exec();
}                                          

//=======================================================================
// name    : SMESHGUI_FilterTable::Copy
// Purpose : Initialise table with values of other table
//=======================================================================
void SMESHGUI_FilterTable::Copy( const SMESHGUI_FilterTable* theTable )
{
  Clear();

  for ( int i = 0, n = theTable->NumRows(); i < n; i++ )
  {
    SMESH::Filter::Criterion aCriterion = SMESHGUI_FilterDlg::createCriterion();
    theTable->GetCriterion( i, aCriterion );
    AddCriterion( aCriterion );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::CurrentCell
// Purpose : Returns current cell
//=======================================================================
bool SMESHGUI_FilterTable::CurrentCell( int& theRow, int& theCol ) const
{
  theRow = myTables[ GetType() ]->currentRow();
  theCol = myTables[ GetType() ]->currentColumn();
  return theRow >= 0 && theCol >= 0;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetText
// Purpose : Set text and internal value in cell of threshold value 
//=======================================================================
void SMESHGUI_FilterTable::SetThreshold( const int      theRow,
                                         const QString& theText,
                                         const int      theEntityType )
{
  Table* aTable = myTables[ theEntityType == -1 ? GetType() : theEntityType ];
  aTable->setText( theRow, 2, theText );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetText
// Purpose : Get text and internal value from cell of threshold value
//=======================================================================
bool SMESHGUI_FilterTable::GetThreshold( const int      theRow,
                                         QString&       theText,
                                         const int      theEntityType )
{
  Table* aTable = myTables[ theEntityType == -1 ? GetType() : theEntityType ];
  QTableItem* anItem = aTable->item( theRow, 2 );
  if ( anItem != 0 )
  {
    theText = anItem->text();
    return true;    
  }
  else
   return false;
}

/*                                                                                                                                                 
  Class       : SMESHGUI_FilterDlg
  Description : Dialog to specify filters for VTK viewer
*/


//=======================================================================
// name    : SMESHGUI_FilterDlg::SMESHGUI_FilterDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterDlg::SMESHGUI_FilterDlg( QWidget*               theParent,
                                        const QValueList<int>& theTypes,
                                        const char*            theName )
: QDialog( theParent, theName, false,
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  construct( theTypes );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SMESHGUI_FilterDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterDlg::SMESHGUI_FilterDlg( QWidget*    theParent,
                                        const int   theType,
                                        const char* theName )
: QDialog( theParent, theName, false,
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  QValueList<int> aTypes;
  aTypes.append( theType );
  construct( aTypes );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::construct
// Purpose : Construct dialog ( called by constructor )
//=======================================================================
void SMESHGUI_FilterDlg::construct( const QValueList<int>& theTypes )
{
  myTypes = theTypes;

  setCaption( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  myMainFrame        = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( myMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( myMainFrame, 1 );

  Init( myTypes );
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

  myTable = new SMESHGUI_FilterTable( aMainFrame, myTypes );
  myTable->SetLibsEnabled( true );

  QFrame* aLine = new QFrame( myTable->GetTableGrp() );
  aLine->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  mySetInViewer = new QCheckBox( tr( "SET_IN_VIEWER" ), myTable->GetTableGrp() );
  mySetInViewer->setChecked( true );

  // other controls
  mySourceGrp = createSourceGroup( aMainFrame );

  connect( myTable, SIGNAL( CretarionChanged( const int, const int ) ),
                    SLOT( onCriterionChanged( const int, const int ) ) );

  connect( myTable, SIGNAL( CurrentChanged( int, int ) ),
                    SLOT( onCurrentChanged( int, int ) ) );                    

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

  aGrp->insert( aMeshBtn, Mesh );
  aGrp->insert( aSelBtn, Selection );
  aGrp->insert( aGrpBtn, Dialog );

  aGrp->setButton( Selection );

  return aGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::updateMainButtons
// Purpose : Update visibility of main buttons ( OK, Cancel, Close ... )
//=======================================================================
void SMESHGUI_FilterDlg::updateMainButtons()
{
  if ( myTypes.count() == 1 )
  {
    myButtons[ BTN_Cancel ]->show();
    myButtons[ BTN_Apply  ]->hide();
    myButtons[ BTN_Close  ]->hide();
  }
  else
  {
    myButtons[ BTN_Cancel ]->hide();
    myButtons[ BTN_Apply  ]->show();
    myButtons[ BTN_Close  ]->show();
  }

//  updateGeometry();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_FilterDlg::createButtonFrame( QWidget* theParent )
{
  QGroupBox* aGrp = new QGroupBox( 1, Qt::Vertical, theParent );

  myButtons[ BTN_OK    ] = new QPushButton( tr( "SMESH_BUT_OK"    ), aGrp );
  myButtons[ BTN_Apply ] = new QPushButton( tr( "SMESH_BUT_APPLY" ), aGrp );

  QLabel* aLbl = new QLabel( aGrp );
  aLbl->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myButtons[ BTN_Cancel ] = new QPushButton( tr( "SMESH_BUT_CANCEL" ), aGrp );
  myButtons[ BTN_Close  ] = new QPushButton( tr( "SMESH_BUT_CLOSE" ), aGrp );

  connect( myButtons[ BTN_OK     ], SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myButtons[ BTN_Cancel ], SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myButtons[ BTN_Close  ], SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myButtons[ BTN_Apply  ], SIGNAL( clicked() ), SLOT( onApply() ) );

  updateMainButtons();

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
void SMESHGUI_FilterDlg::Init( const int type )
{
  QValueList<int> aTypes;
  aTypes.append( type );
  Init( aTypes );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FilterDlg::Init( const QValueList<int>& theTypes )
{
  mySourceWg  = 0;
  mySelection = 0;
  myTypes     = theTypes;
  myMesh      = SMESH::SMESH_Mesh::_nil();
  myIObjects.Clear();
  myIsSelectionChanged = false;

  myTable->Init( theTypes );

  // set caption
  if ( theTypes.count() == 1 )
  {
    int aType = theTypes.first();
    if      ( aType == SMESH::NODE   ) setCaption( tr( "NODES_TLT" ) );
    else if ( aType == SMESH::EDGE   ) setCaption( tr( "EDGES_TLT" ) );
    else if ( aType == SMESH::FACE   ) setCaption( tr( "FACES_TLT" ) );
    else if ( aType == SMESH::VOLUME ) setCaption( tr( "VOLUMES_TLT" ) );
  }
  else
    setCaption( tr( "TLT" ) );

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

  updateMainButtons();
  updateSelection();

  // Initialise filter table with values of previous filter
  QValueList<int>::const_iterator anIter;
  for ( anIter = theTypes.begin(); anIter != theTypes.end(); ++anIter )
  {
    myTable->Clear( *anIter );
    if ( !myFilter[ *anIter ]->_is_nil() )
    {
      SMESH::Filter::Criteria_var aCriteria = new SMESH::Filter::Criteria;
      if ( myFilter[ *anIter ]->GetCriteria( aCriteria ) )
      {
        for ( int i = 0, n = aCriteria->length(); i < n; i++ )
          myTable->AddCriterion( aCriteria[ i ], *anIter );
      }
    }
  }

  if ( myInsertState.contains( theTypes.first() ) )
    mySetInViewer->setChecked( myInsertState[ theTypes.first() ] );
  else
    mySetInViewer->setChecked( true );
  if ( myApplyToState.contains( theTypes.first() ) )
    mySourceGrp->setButton( myApplyToState[ theTypes.first() ] );
  else
    mySourceGrp->setButton( Selection ); 
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
    mySelection->ClearFilters();
    disconnect( mySMESHGUI, 0, this, 0 );
    disconnect( mySelection, 0, this, 0 );
    mySMESHGUI->ResetState() ;
    accept();
    emit Accepted();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_FilterDlg::onClose()
{
  // Restore previously selected object
  if ( mySelection )
  {
    mySelection->ClearFilters();
    mySelection->Clear();
    SALOME_DataMapIteratorOfDataMapOfIOMapOfInteger anIter( myIObjects );
    for ( ; anIter.More(); anIter.Next() )
    {
      mySelection->AddIObject( anIter.Key() );

      TColStd_MapOfInteger aResMap;
      const TColStd_IndexedMapOfInteger& anIndMap = anIter.Value();
      for ( int i = 1, n = anIndMap.Extent(); i <= n; i++ )
        aResMap.Add( anIndMap( i ) );
      
      mySelection->AddOrRemoveIndex( anIter.Key(), aResMap, false );
    }
  }

  disconnect( mySMESHGUI, 0, this, 0 );
  disconnect( mySelection, 0, this, 0 );
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
// name    : SMESHGUI_FilterDlg::getIdsFromWg
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
// Purpose : Get selection mode of specified type
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
// name    : SMESHGUI_FilterDlg::setIdsToWg
// Purpose : Insert identifiers in specified widgets
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
// name    : SMESHGUI_FilterDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_FilterDlg::isValid() const
{
  if ( !myTable->IsValid() )
    return false;
    
  for ( int i = 0, n = myTable->NumRows(); i < n; i++ )
  {
    int aType = myTable->GetCriterionType( i );
    if ( aType == FT_BelongToGeom ||
         aType == FT_BelongToPlane ||
         aType == FT_BelongToCylinder ||
	 aType == FT_LyingOnGeom)
    {
      QString aName;
      myTable->GetThreshold( i, aName );
      
      SALOMEDS::Study::ListOfSObject_var aList = SMESHGUI::GetSMESHGUI()->GetActiveStudy()->
        getStudyDocument()->FindObjectByName( aName.latin1(), "GEOM" );
      if ( aList->length() == 0 )
      {
        QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
          tr( "SMESH_INSUFFICIENT_DATA" ), tr( "BAD_SHAPE_NAME" ).arg( aName ), QMessageBox::Ok );
        return false;
      }

      if ( aType == FT_BelongToCylinder || aType == FT_BelongToPlane )
      {
        GEOM::GEOM_Object_var aGeomObj =
          GEOM::GEOM_Object::_narrow( aList[ 0 ]->GetObject() );
        if ( !aGeomObj->_is_nil() )
        {
          TopoDS_Shape aFace;
          if ( !GEOMBase::GetShape( aGeomObj, aFace ) ||
               aFace.IsNull() ||
               aFace.ShapeType() != TopAbs_FACE )
          {
            QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
              tr( "SMESH_INSUFFICIENT_DATA" ), tr( "SHAPE_IS_NOT_A_FACE" ).arg( aName ), QMessageBox::Ok );
            return false;
          }

          Handle(Geom_Surface) aSurf = BRep_Tool::Surface( TopoDS::Face( aFace ) );
          if ( aSurf.IsNull() )
          {
            QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
              tr( "SMESH_INSUFFICIENT_DATA" ), tr( "SHAPE_IS_NOT_A_FACE" ).arg( aName ), QMessageBox::Ok );
            return false;
          }

          if ( aType == FT_BelongToPlane && !aSurf->IsKind( STANDARD_TYPE( Geom_Plane ) ) )
          {
            QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
              tr( "SMESH_INSUFFICIENT_DATA" ), tr( "SHAPE_IS_NOT_A_PLANE" ).arg( aName ), QMessageBox::Ok );
            return false;
          }

          if ( aType == FT_BelongToCylinder && !aSurf->IsKind( STANDARD_TYPE( Geom_CylindricalSurface ) ) )
          {
            QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
              tr( "SMESH_INSUFFICIENT_DATA" ), tr( "SHAPE_IS_NOT_A_CYLINDER" ).arg( aName ), QMessageBox::Ok );
            return false;
          }
        }
      }
    }
  }

  return true;
}

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
  if ( mySelection )
    disconnect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( onSelectionDone() ) );
    
  mySelection = theSel;

  if ( mySelection )
  {
    myIObjects.Clear();
    const SALOME_ListIO& anObjs = mySelection->StoredIObjects();
    SALOME_ListIteratorOfListIO anIter( anObjs );
    for ( ;anIter.More(); anIter.Next() )
    {
      TColStd_IndexedMapOfInteger aMap;
      mySelection->GetIndex( anIter.Value(), aMap );
      myIObjects.Bind( anIter.Value(), aMap );
    }
    
    connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );

    updateSelection();
  }
  else
    myIObjects.Clear();
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
    int aCurrType = myTable->GetType();

    if ( !createFilter( aCurrType ) )
      return false;

    insertFilterInViewer();

    if ( !myFilter[ aCurrType ]->GetPredicate()->_is_nil() )
    {
      QValueList<int> aResultIds;
      filterSource( aCurrType, aResultIds );
      selectInViewer( aCurrType, aResultIds );
    }

    myInsertState[ aCurrType ] = mySetInViewer->isChecked();
    myApplyToState[ aCurrType ] = mySourceGrp->id( mySourceGrp->selected() );
    
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
// name    : SMESHGUI_FilterDlg::createFilter
// Purpose : Create predicate for given type
//=======================================================================
bool SMESHGUI_FilterDlg::createFilter( const int theType )
{
  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
  if ( aFilterMgr->_is_nil() )
    return false;

  int n = myTable->NumRows();

  SMESH::Filter::Criteria_var aCriteria = new SMESH::Filter::Criteria;
  aCriteria->length( n );

  long aPrecision = -1;
  if ( QAD_CONFIG->hasSetting( "SMESH:ControlsPrecision" ) )
  {
    QString aStr = QAD_CONFIG->getSetting( "SMESH:ControlsPrecision" );
    bool isOk = false;
    int aVal = aStr.toInt( &isOk );
    if ( isOk )
      aPrecision = aVal;
  }
  
  for ( CORBA::ULong i = 0; i < n; i++ )
  {
    SMESH::Filter::Criterion aCriterion = createCriterion();
    myTable->GetCriterion( i, aCriterion );
    aCriterion.Precision = aPrecision;
    aCriteria[ i ] = aCriterion;
  }

  myFilter[ theType ] = aFilterMgr->CreateFilter();
  myFilter[ theType ]->SetCriteria( aCriteria.inout() );

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::insertFilterInViewer
// Purpose : Insert filter in viewer
//=======================================================================
void SMESHGUI_FilterDlg::insertFilterInViewer()
{
  if ( VTKViewer_InteractorStyleSALOME* aStyle = SMESH::GetInteractorStyle() )
  {
    SMESH::ElementType anEntType = (SMESH::ElementType)myTable->GetType();

    if ( myFilter[ myTable->GetType() ]->_is_nil() ||
         myFilter[ myTable->GetType() ]->GetPredicate()->_is_nil() ||
         !mySetInViewer->isChecked() )
      SMESH::RemoveFilter( getFilterId( anEntType ), aStyle );
    else
    {
      Handle(SMESHGUI_PredicateFilter) aFilter = new SMESHGUI_PredicateFilter();
      aFilter->SetPredicate( myFilter[ myTable->GetType() ]->GetPredicate() );
      SMESH::SetFilter( aFilter, aStyle );
    }
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::filterSource
// Purpose : Filter source ids
//=======================================================================
void SMESHGUI_FilterDlg::filterSource( const int theType,
                                       QValueList<int>& theResIds )
{
  theResIds.clear();
  if ( myFilter[ theType ]->_is_nil() )
    return;

  int aSourceId = mySourceGrp->id( mySourceGrp->selected() );

  if ( aSourceId == Mesh )
  {
    if ( myMesh->_is_nil() )
      return;
    SMESH::long_array_var anIds = myFilter[ theType ]->GetElementsId( myMesh );
    for ( int i = 0, n = anIds->length(); i < n; i++ )
      theResIds.append( anIds[ i ] );
  }
  else if ( aSourceId == Selection )
  {
    filterSelectionSource( theType, theResIds );
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
    SMESH::Predicate_ptr aPred = myFilter[ theType ]->GetPredicate();
    aPred->SetMesh( myMesh );
    QValueList<int>::const_iterator anIter;
    for ( anIter = aDialogIds.begin(); anIter != aDialogIds.end(); ++ anIter )
      if ( aPred->IsSatisfy( *anIter ) )
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
                                                QValueList<int>& theResIds )
{
  theResIds.clear();
  if ( myMesh->_is_nil() || mySelection == 0 )
    return;

  // Create map of entities to be filtered
  TColStd_MapOfInteger aToBeFiltered;
  SALOME_DataMapIteratorOfDataMapOfIOMapOfInteger anIter( myIObjects );

  for ( ; anIter.More(); anIter.Next() )
  {
    // process sub mesh
    SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>( anIter.Key() );
    if ( !aSubMesh->_is_nil() )
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
    SMESH::SMESH_GroupBase_var aGroup =
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>( anIter.Key() );
    if ( !aGroup->_is_nil() )
    {
      if ( aGroup->GetType() == theType && aGroup->GetMesh()->GetId() == myMesh->GetId() )
      {
        SMESH::long_array_var anIds = aGroup->GetListOfID();
        for ( int i = 0, n = anIds->length(); i < n; i++ )
          aToBeFiltered.Add( anIds[ i ] );
      }
    }

    // process mesh
    SMESH::SMESH_Mesh_var aMeshPtr = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>( anIter.Key() );
    if ( !aMeshPtr->_is_nil() && aMeshPtr->GetId() == myMesh->GetId() )
    {
      const TColStd_IndexedMapOfInteger& aSelMap = anIter.Value();

      if ( aSelMap.Extent() > 0 )
      {
        if( SMESH::FindActorByEntry( anIter.Key()->getEntry() ) )
        {
          for ( int i = 1; i <= aSelMap.Extent(); i++ )
            aToBeFiltered.Add( aSelMap(i) );
        }
      }
    }
  }

  // Filter entities
  SMESH::Predicate_ptr aPred = myFilter[ theType ]->GetPredicate();
  aPred->SetMesh( myMesh );
  TColStd_MapIteratorOfMapOfInteger aResIter( aToBeFiltered );
  for ( ; aResIter.More(); aResIter.Next() )
    if ( aPred->IsSatisfy( aResIter.Key() ) )
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

  mySelection->ClearFilters();

  // Set new selection mode if necessary
  Selection_Mode aSelMode = getSelMode( theType );
  if ( aSelMode != mySelection->SelectionMode() )
  {
    mySelection->ClearIObjects();
    mySelection->ClearFilters();
    if ( aSelMode == NodeSelection )
      SMESH::SetPointRepresentation(true);
    QAD_Application::getDesktop()->SetSelectionMode( aSelMode );
  }

  // Clear selection
  SMESH_Actor* anActor = SMESH::FindActorByObject(myMesh);
  if ( !anActor || !anActor->hasIO() )
    return;

  Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
  mySelection->ClearIObjects();
  mySelection->AddIObject( anIO, false );

  // Remove filter corresponding to the current type from viewer
  int aType = myTable->GetType();
  int aFilterId = SMESHGUI_UnknownFilter;
  if      ( aType == SMESH::EDGE   ) aFilterId = SMESHGUI_EdgeFilter;
  else if ( aType == SMESH::FACE   ) aFilterId = SMESHGUI_FaceFilter;
  else if ( aType == SMESH::VOLUME ) aFilterId = SMESHGUI_VolumeFilter;
  Handle(VTKViewer_Filter) aFilter = SMESH::GetFilter( aFilterId );
  SMESH::RemoveFilter( aFilterId );

  // get vtk ids
  TColStd_MapOfInteger aMap;
  QValueList<int>::const_iterator anIter;
  for ( anIter = theIds.begin(); anIter != theIds.end(); ++anIter )
  {
    aMap.Add( *anIter );
  }

  // Set new selection
  mySelection->AddOrRemoveIndex( anIO, aMap, false, true );

  // insert previously stored filter in viewer if necessary
  if ( !aFilter.IsNull() )
    SMESH::SetFilter( aFilter );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createCriterion
// Purpose : Create criterion structure with default values
//=======================================================================
SMESH::Filter::Criterion SMESHGUI_FilterDlg::createCriterion()
{
   SMESH::Filter::Criterion aCriterion;

  aCriterion.Type          = FT_Undefined;
  aCriterion.Compare       = FT_Undefined;
  aCriterion.Threshold     = 0;
  aCriterion.UnaryOp       = FT_Undefined;
  aCriterion.BinaryOp      = FT_Undefined;
  aCriterion.ThresholdStr  = "";
  aCriterion.TypeOfElement = SMESH::ALL;

  return aCriterion;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onSelectionDone
// Purpose : SLOT called when selection changed.
//           If current cell corresponds to the threshold value of
//           BelongToGeom criterion name of selected object is set in this cell
//=======================================================================
void SMESHGUI_FilterDlg::onSelectionDone()
{
  int aRow, aCol;
  if (  mySelection->IObjectCount() != 1 ||
        !myTable->CurrentCell( aRow, aCol ) ||
        myTable->GetCriterionType( aRow ) != FT_BelongToGeom &&
        myTable->GetCriterionType( aRow ) != FT_BelongToPlane &&
        myTable->GetCriterionType( aRow ) != FT_BelongToCylinder &&
	myTable->GetCriterionType( aRow ) != FT_LyingOnGeom )
    return;

  Handle(SALOME_InteractiveObject) anIO = mySelection->firstIObject() ;
  GEOM::GEOM_Object_var anObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>( anIO ) ;
  if ( !anObj->_is_nil() )
    myTable->SetThreshold( aRow, anIO->getName() );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onCriterionChanged
// Purpose : SLOT called when cretarion of current row changed. Update selection
//=======================================================================
void SMESHGUI_FilterDlg::onCriterionChanged( const int , const int )
{
  updateSelection();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onCurrentChanged
// Purpose : SLOT called when current row changed. Update selection
//=======================================================================
void SMESHGUI_FilterDlg::onCurrentChanged( int, int )
{
  updateSelection();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::updateSelection
// Purpose : UpdateSelection in accordance with current row
//=======================================================================
void SMESHGUI_FilterDlg::updateSelection()
{
  if ( mySelection == 0 )
    return;
  
  mySelection->ClearFilters();

  int aRow, aCol;
  
  if ( myTable->CurrentCell( aRow, aCol ) &&
       ( myTable->GetCriterionType( aRow ) == FT_BelongToGeom ||
         myTable->GetCriterionType( aRow ) == FT_BelongToPlane ||
         myTable->GetCriterionType( aRow ) == FT_BelongToCylinder ||
	 myTable->GetCriterionType( aRow ) == FT_LyingOnGeom) )
  {
    if ( myTable->GetCriterionType( aRow ) == FT_BelongToGeom ||  myTable->GetCriterionType( aRow ) == FT_LyingOnGeom )
      mySelection->AddFilter( new SALOME_TypeFilter( "GEOM" ) );
    else if ( myTable->GetCriterionType( aRow ) == FT_BelongToPlane )
      mySelection->AddFilter( new GEOM_FaceFilter( StdSelect_Plane ) );
    else if ( myTable->GetCriterionType( aRow ) == FT_BelongToCylinder )
      mySelection->AddFilter( new GEOM_FaceFilter( StdSelect_Cylinder ) );

    myIsSelectionChanged = true;
  }
  else
  {
    if ( myIsSelectionChanged )
      mySelection->AddFilter( new SALOME_TypeFilter( "This filter deactivate selection" ) );
  }
}
























