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
// File   : SMESHGUI_FilterDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_FilterDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_Filter.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_FilterLibraryDlg.h"
#include "SMESHGUI_SpinBox.h"

#include "SMESH_Actor.h"
#include "SMESH_NumberFilter.hxx"
#include "SMESH_TypeFilter.hxx"
#include "SMESH_LogicalFilter.hxx"

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GEOM_FaceFilter.h>
#include <GEOM_TypeFilter.h>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <QtxColorButton.h>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_DoubleSpinBox.h>
#include <SalomeApp_IntSpinBox.h>
#include <SalomeApp_Study.h>
#include <SalomeApp_Tools.h>

#include <SALOME_ListIO.hxx>

#include <SVTK_ViewWindow.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>
#include <Basics_Utils.hxx>

// OCCT includes
#include <StdSelect_TypeOfFace.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Precision.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

// Qt includes
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QTableWidget>
#include <QStringList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QApplication>
#include <QComboBox>
#include <QFontMetrics>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QRegExp>
#include <QListWidget>
#include <QCheckBox>
#include <QItemDelegate>
#include <QDoubleValidator>
#include <QKeyEvent>
#include <QHeaderView>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

#define SPACING 6
#define MARGIN  11

//---------------------------------------
// maxLength
//---------------------------------------
static int maxLength (const QMap<int, QString> theMap, const QFontMetrics& theMetrics)
{
  int aRes = 0;
  QMap<int, QString>::const_iterator anIter;
  for (anIter = theMap.begin(); anIter != theMap.end(); ++anIter)
    aRes = qMax(aRes, theMetrics.width(anIter.value()));
  return aRes;
}

//---------------------------------------
// getFilterId
//---------------------------------------
static int getFilterId (SMESH::ElementType theType)
{
  switch (theType)
  {
    case SMESH::NODE   : return SMESH::NodeFilter;
    case SMESH::EDGE   : return SMESH::EdgeFilter;
    case SMESH::FACE   : return SMESH::FaceFilter;
    case SMESH::VOLUME : return SMESH::VolumeFilter;
    case SMESH::ALL    : return SMESH::AllElementsFilter;
    default            : return SMESH::UnknownFilter;
  }
}

/*
  Class       : SMESHGUI_FilterTable::AdditionalWidget
  Description : Class for storing additional parameters of criterion
*/

class SMESHGUI_FilterTable::AdditionalWidget : public QWidget
{
public:
  enum { Tolerance };

public:
  AdditionalWidget(QWidget* theParent);
  virtual ~AdditionalWidget();

  virtual QList<int>      GetParameters() const;
  virtual bool            IsValid(const bool = true) const;
  virtual double          GetDouble(const int) const;
  virtual int             GetInteger(const int) const;
  virtual QString         GetString(const int) const;
  virtual void            SetDouble(const int, const double);
  virtual void            SetInteger(const int, const int);
  virtual void            SetString(const int, const QString&);
  void                    SetEditable(const int, const bool);
  void                    SetEditable(const bool);
  void                    SetPrecision(const int, const char* = 0);

private:
  QMap< int, QWidget* > myWidgets;
};

SMESHGUI_FilterTable::AdditionalWidget::AdditionalWidget (QWidget* theParent)
  : QWidget(theParent)
{
  QLabel* aLabel = new QLabel(tr("SMESH_TOLERANCE"), this);

  SMESHGUI_SpinBox* sb = new SMESHGUI_SpinBox(this);
  sb->setAcceptNames( false ); // No Notebook variables allowed
  sb->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  sb->RangeStepAndValidator( 0., 1.e20, 0.1, "len_tol_precision" );
  myWidgets[ Tolerance ] = sb;

  QHBoxLayout* aLay = new QHBoxLayout(this);
  aLay->setSpacing(SPACING);
  aLay->setMargin(0);

  aLay->addWidget(aLabel);
  aLay->addWidget(myWidgets[ Tolerance ]);
  aLay->addStretch();

  SetDouble( Tolerance, Precision::Confusion() );
}

SMESHGUI_FilterTable::AdditionalWidget::~AdditionalWidget()
{
}

QList<int> SMESHGUI_FilterTable::AdditionalWidget::GetParameters() const
{
  QList<int> theList;
  theList.append(Tolerance);
  return theList;
}

bool SMESHGUI_FilterTable::AdditionalWidget::IsValid (const bool theMsg) const
{
  if (!isEnabled())
    return true;

  QList<int> aParams = GetParameters();
  QList<int>::const_iterator anIter;
  for (anIter = aParams.begin(); anIter != aParams.end(); ++anIter) {
    if ( !myWidgets.contains( *anIter ) ) continue;
    bool valid = true;
    if ( qobject_cast<QLineEdit*>( myWidgets[ *anIter ] ) ) {
      valid = qobject_cast<QLineEdit*>( myWidgets[ *anIter ] )->hasAcceptableInput();
    }
    else if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ *anIter ] ) ) {
      QString foo;
      valid = qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ *anIter ] )->isValid( foo, false );
    }
    if (!valid && theMsg) {
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                   tr("SMESHGUI_INVALID_PARAMETERS"));
      return false;
    }
  }

  return true;
}

double SMESHGUI_FilterTable::AdditionalWidget::GetDouble (const int theId) const
{
  double retval = 0;
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      retval = qobject_cast<QLineEdit*>( myWidgets[ theId ] )->text().toDouble();
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      retval = qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->GetValue();
  }
  return retval;
}

int SMESHGUI_FilterTable::AdditionalWidget::GetInteger (const int theId) const
{
  int retval = 0;
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      retval = qobject_cast<QLineEdit*>( myWidgets[ theId ] )->text().toInt();
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      retval = (int)( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->GetValue() );
  }
  return retval;
}

QString SMESHGUI_FilterTable::AdditionalWidget::GetString (const int theId) const
{
  QString retval = "";
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      retval = qobject_cast<QLineEdit*>( myWidgets[ theId ] )->text();
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      retval = QString::number( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->GetValue() );
  }
  return retval;
}

void SMESHGUI_FilterTable::AdditionalWidget::SetDouble (const int theId, const double theVal)
{
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      qobject_cast<QLineEdit*>( myWidgets[ theId ] )->setText( QString::number( theVal ) );
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->SetValue( theVal );
  }
}

void SMESHGUI_FilterTable::AdditionalWidget::SetInteger (const int theId, const int theVal)
{
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      qobject_cast<QLineEdit*>( myWidgets[ theId ] )->setText( QString::number( theVal ) );
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->SetValue( (double)theVal );
  }
}

void SMESHGUI_FilterTable::AdditionalWidget::SetString (const int theId, const QString& theVal)
{
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      qobject_cast<QLineEdit*>( myWidgets[ theId ] )->setText( theVal );
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->SetValue( theVal.toDouble() );
  }
}

void SMESHGUI_FilterTable::AdditionalWidget::SetEditable (const int theId, const bool isEditable)
{
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<QLineEdit*>( myWidgets[ theId ] ) )
      qobject_cast<QLineEdit*>( myWidgets[ theId ] )->setReadOnly( !isEditable );
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) )
      qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] )->setReadOnly( !isEditable );
  }
}

void SMESHGUI_FilterTable::AdditionalWidget::SetEditable (const bool isEditable)
{
  QList<int> aParams = GetParameters();
  QList<int>::const_iterator anIter;
  for (anIter = aParams.begin(); anIter != aParams.end(); ++anIter)
    SetEditable( *anIter,  isEditable );
}

void SMESHGUI_FilterTable::AdditionalWidget::SetPrecision(const int theId, const char* precision)
{
  if ( myWidgets.contains( theId ) ) {
    if ( qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] ) ) {
      SMESHGUI_SpinBox* sb = qobject_cast<SMESHGUI_SpinBox*>( myWidgets[ theId ] );
      double val = sb->GetValue();
      double min = pow(10.0, -(sb->decimals()));
      sb->RangeStepAndValidator( 0., 1.e20, 0.1, precision ? precision : "len_tol_precision" );
      sb->SetValue( qMax( val, min ) );
    }
  }
}

/*
  Class       : SMESHGUI_FilterTable::ComboItem
  Description : Combo table item. Identificator corresponding to string may be assigned
*/

class SMESHGUI_FilterTable::ComboItem : public QTableWidgetItem
{
public:
  static int     Type();

  ComboItem( const QMap<int, QString>& );

  void           setItems( const QMap<int, QString>& );
  void           clear();
  int            count() const;

  int            value() const;
  void           setValue( const int );

private:
  int            id( int ) const;
  int            index( int ) const;

private:
  QMap<int, int> myIdToIdx;
};

int SMESHGUI_FilterTable::ComboItem::Type()
{
  return QTableWidgetItem::UserType + 1;
}

SMESHGUI_FilterTable::ComboItem::ComboItem( const QMap<int, QString>& theIds )
 : QTableWidgetItem( Type() )
{
  setItems( theIds );
}

void SMESHGUI_FilterTable::ComboItem::setItems( const QMap<int, QString>& theIds )
{
  myIdToIdx.clear();
  QMap<int, QString>::const_iterator it;
  QStringList items;
  for ( it = theIds.begin(); it != theIds.end(); ++it ) {
    myIdToIdx[it.key()] = items.count();
    items.append( it.value() );
  }
  setData( Qt::UserRole, items );
  setValue( id( 0 ) ); 
}

void SMESHGUI_FilterTable::ComboItem::clear()
{
  QMap<int, QString> empty;
  setItems( empty );
}

int SMESHGUI_FilterTable::ComboItem::count() const
{
  return myIdToIdx.count();
}

int SMESHGUI_FilterTable::ComboItem::value() const
{
  return( id( data( Qt::UserRole ).toStringList().indexOf( text() ) ) ); 
}

void SMESHGUI_FilterTable::ComboItem::setValue( const int theId )
{
  int idx = index( theId );
  QStringList items = data( Qt::UserRole ).toStringList();
  setText( idx >= 0 && idx < items.count() ? items[idx] : "" );
}

int SMESHGUI_FilterTable::ComboItem::id( int idx ) const
{
  QMap<int,int>::const_iterator it;
  for ( it = myIdToIdx.begin(); it != myIdToIdx.end(); ++it )
    if ( it.value() == idx ) return it.key();
  return -1;
}

int SMESHGUI_FilterTable::ComboItem::index( int i ) const
{
  return myIdToIdx.contains( i ) ? myIdToIdx[i] : -1;
}
/*
  Class       : SMESHGUI_FilterTable::CheckItem
  Description : Check table item.
*/

class SMESHGUI_FilterTable::CheckItem : public QTableWidgetItem
{
public:
  static int     Type();

  CheckItem( bool = false );
  CheckItem( const QString&, bool = false );
  ~CheckItem();

  void  setChecked( bool );
  bool  checked() const;
};

int SMESHGUI_FilterTable::CheckItem::Type()
{
  return QTableWidgetItem::UserType + 2;
}

SMESHGUI_FilterTable::CheckItem::CheckItem( bool value )
 : QTableWidgetItem( Type() )
{
  Qt::ItemFlags f = flags();
  f = f | Qt::ItemIsUserCheckable;
  f = f & ~Qt::ItemIsTristate;
  f = f & ~Qt::ItemIsEditable;
  setFlags( f );
  setChecked(value);
}

SMESHGUI_FilterTable::CheckItem::CheckItem( const QString& text, bool value )
 : QTableWidgetItem( Type() )
{
  Qt::ItemFlags f = flags();
  f = f | Qt::ItemIsUserCheckable;
  f = f & ~Qt::ItemIsTristate;
  f = f & ~Qt::ItemIsEditable;
  setFlags( f );
  setChecked( value );
  setText( text );
}

SMESHGUI_FilterTable::CheckItem::~CheckItem()
{
}

void SMESHGUI_FilterTable::CheckItem::setChecked( bool value )
{
  setCheckState( value ? Qt::Checked : Qt::Unchecked );
}

bool SMESHGUI_FilterTable::CheckItem::checked() const
{
  return checkState() == Qt::Checked;
}

/*
  Class       : SMESHGUI_FilterTable::IntSpinItem
  Description : Integer spin table item.
*/

class SMESHGUI_FilterTable::IntSpinItem : public QTableWidgetItem
{
public:
  static int     Type();

  IntSpinItem( const int theValue );

  int            value() const;
  void           setValue( const int theValue );

  void           clear();
};

int SMESHGUI_FilterTable::IntSpinItem::Type()
{
  return QTableWidgetItem::UserType + 3;
}

SMESHGUI_FilterTable::IntSpinItem::IntSpinItem( const int theValue )
 : QTableWidgetItem( Type() )
{
  setValue( theValue );
}

int SMESHGUI_FilterTable::IntSpinItem::value() const
{
  bool ok = false;
  int value = data( Qt::UserRole ).toInt( &ok );
  return ok ? value : 0; 
}

void SMESHGUI_FilterTable::IntSpinItem::setValue( const int theValue )
{
  setData( Qt::UserRole, theValue );
  setText( QString::number( theValue ) ); 
}

void SMESHGUI_FilterTable::IntSpinItem::clear()
{
  setText( "" );
}

/*
  Class       : SMESHGUI_FilterTable::DoubleSpinItem
  Description : Double spin table item.
*/

class SMESHGUI_FilterTable::DoubleSpinItem : public QTableWidgetItem
{
public:
  static int     Type();

  DoubleSpinItem( const double theValue );

  double         value() const;
  void           setValue( const double theValue );

  int            precision() const;
  void           setPrecision( const int thePrecision );

  void           clear();
};

int SMESHGUI_FilterTable::DoubleSpinItem::Type()
{
  return QTableWidgetItem::UserType + 4;
}

SMESHGUI_FilterTable::DoubleSpinItem::DoubleSpinItem( const double theValue )
 : QTableWidgetItem( Type() )
{
  setValue( theValue );
}

double SMESHGUI_FilterTable::DoubleSpinItem::value() const
{
  bool ok = false;
  double value = data( Qt::UserRole ).toDouble( &ok );
  return ok ? value : 0; 
}

void SMESHGUI_FilterTable::DoubleSpinItem::setValue( const double theValue )
{
  setData( Qt::UserRole, theValue );
  setText( QString::number( theValue ) ); 
}

int SMESHGUI_FilterTable::DoubleSpinItem::precision() const
{
  bool ok = false;
  int precision = data( Qt::UserRole + 1 ).toInt( &ok );
  return ok ? precision : 0; 
}

void SMESHGUI_FilterTable::DoubleSpinItem::setPrecision( const int thePrecision )
{
  setData( Qt::UserRole + 1, thePrecision );
}

void SMESHGUI_FilterTable::DoubleSpinItem::clear()
{
  setText( "" );
}

/*
  Class       : SMESHGUI_FilterTable::ComboDelegate
  Description : Table used by this widget
*/

class SMESHGUI_FilterTable::ComboDelegate : public QItemDelegate
{
public:
  ComboDelegate( QObject* = 0 );
  ~ComboDelegate();
  
  QWidget*      createEditor( QWidget*, const QStyleOptionViewItem&,
                              const QModelIndex& ) const;
  
  void          setEditorData( QWidget*, const QModelIndex& ) const;
  void          setModelData( QWidget*, QAbstractItemModel*, const QModelIndex& ) const;
  
  void          updateEditorGeometry( QWidget*, const QStyleOptionViewItem&, 
                                      const QModelIndex& ) const;
private:
  QTableWidget* myTable;
};

SMESHGUI_FilterTable::ComboDelegate::ComboDelegate( QObject* parent )
  : QItemDelegate( parent ), 
    myTable( qobject_cast<QTableWidget*>( parent ) )
{
}
  
SMESHGUI_FilterTable::ComboDelegate::~ComboDelegate()
{
}

QWidget* SMESHGUI_FilterTable::ComboDelegate::createEditor( QWidget* parent,
                                                            const QStyleOptionViewItem& option,
                                                            const QModelIndex& index ) const
{
  QVariant aData = index.data( Qt::UserRole );
  QVariant::Type aDataType = aData.type();
  if( aDataType == QVariant::StringList ) {
    QStringList l = aData.toStringList();
    if ( !l.isEmpty() ) {
      QComboBox* cb = new QComboBox( parent );
      cb->setFrame( false );
      cb->addItems( l );
      return cb;
    }
  }
  else if( aDataType == QVariant::Int ) {
    bool ok = false;
    int aValue = aData.toInt( &ok );
    if ( ok ) {
      SalomeApp_IntSpinBox* intSpin = new SalomeApp_IntSpinBox( 0, 1000, 1, parent, false, true );
      intSpin->setFrame( false );
      intSpin->setValue( aValue );
      return intSpin;
    }
  }
  else if( aDataType == QVariant::Double ) {
    bool ok = false;
    double aValue = aData.toDouble( &ok );
    if ( ok ) {
      int aPrecision = index.data( Qt::UserRole + 1 ).toInt( &ok );
      if ( !ok )
        aPrecision = 0;

      SalomeApp_DoubleSpinBox* dblSpin = new SalomeApp_DoubleSpinBox( -1.e20, 1.e20, 1, aPrecision, 20, parent, false, true );
      dblSpin->setFrame( false );
      dblSpin->setValue( aValue );
      return dblSpin;
    }
  }
  return QItemDelegate::createEditor( parent, option, index );
}

void SMESHGUI_FilterTable::ComboDelegate::setEditorData( QWidget* editor, 
                                                         const QModelIndex& index ) const
{
  QVariant data = index.model()->data( index, Qt::DisplayRole );
  QString value = data.toString();
  bool bOk = false;
  if ( QComboBox* cb = dynamic_cast<QComboBox*>( editor ) ) {
    int i = cb->findText( value );
    if ( i >= 0 ) {
      cb->setCurrentIndex( i );
      bOk = true;
    }
  }
  else if ( SalomeApp_DoubleSpinBox* dblSpin = dynamic_cast<SalomeApp_DoubleSpinBox*>( editor ) ) {
    if( data.type() == QVariant::Double ) {
      double valueDouble = data.toDouble( &bOk );
      if( bOk )
        dblSpin->setValue( valueDouble );
    }
  }
  if ( !bOk ) QItemDelegate::setEditorData( editor, index );
}

void SMESHGUI_FilterTable::ComboDelegate::setModelData( QWidget* editor,
                                                        QAbstractItemModel* model,
                                                        const QModelIndex& index) const
{
  if( QComboBox* cb = dynamic_cast<QComboBox*>( editor ) )
    model->setData( index, cb->currentText(), Qt::DisplayRole );
  else if( SalomeApp_IntSpinBox* intSpin = dynamic_cast<SalomeApp_IntSpinBox*>( editor ) )
    model->setData( index, intSpin->value(), Qt::DisplayRole );
  else if( SalomeApp_DoubleSpinBox* dblSpin = dynamic_cast<SalomeApp_DoubleSpinBox*>( editor ) )
    model->setData( index, dblSpin->value(), Qt::DisplayRole );
  else QItemDelegate::setModelData( editor, model, index );
}

void SMESHGUI_FilterTable::ComboDelegate::updateEditorGeometry( QWidget* editor,
                                                                const QStyleOptionViewItem& option, 
                                                                const QModelIndex& index ) const
{
  editor->setGeometry( option.rect );
}

/*
  Class       : SMESHGUI_FilterTable::Table
  Description : Table used by this widget
*/

class SMESHGUI_FilterTable::Table : public QTableWidget
{
public:
  Table( QWidget* = 0 );
  Table( int, int, QWidget* = 0 );
  virtual ~Table();

  QSize                   minimumSizeHint() const;

  void                    setEditable( bool, int, int );
  bool                    isEditable( int, int ) const;

  void                    setReadOnly( bool );
  bool                    isReadOnly() const;

  void                    insertRows( int, int = 1 );
  QString                 text( int, int );

  QList<int>              selectedRows();
};

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::Table
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterTable::Table::Table (QWidget* parent)
: QTableWidget(parent)
{
  // set custom item delegate
  setItemDelegate( new ComboDelegate(this) );
  // set edit triggers by default
  setReadOnly( false );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::Table
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterTable::Table::Table (int numRows, int numCols, QWidget* parent)
: QTableWidget(numRows, numCols, parent)
{
  // set custom item delegate
  setItemDelegate( new ComboDelegate(this) );
  // set edit triggers by default
  setReadOnly( false );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::~Table
// Purpose : Destructor
//=======================================================================
SMESHGUI_FilterTable::Table::~Table()
{
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::minimumSizeHint
// Purpose : Get minimum size for the table
//=======================================================================
QSize SMESHGUI_FilterTable::Table::minimumSizeHint() const
{
  QSize s = QTableWidget::minimumSizeHint();
  QHeaderView* hv = horizontalHeader();
  if ( hv )
    s.setWidth( qMax( s.width(), hv->length() ) );
  return s;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::setEditable
// Purpose : Set editable of specified cell
//=======================================================================
void SMESHGUI_FilterTable::Table::setEditable (bool isEditable,
                                               int row, int col)
{
  QTableWidgetItem* anItem = item( row, col );
  if ( anItem ) {
    bool isSignalsBlocked = signalsBlocked();
    blockSignals( true );

    Qt::ItemFlags f = anItem->flags();
    if ( !isEditable ) f = f & ~Qt::ItemIsEditable;
    else f = f | Qt::ItemIsEditable;
    anItem->setFlags( f );
    
    blockSignals( isSignalsBlocked );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::isEditable
// Purpose : Verify wheter cell is editable
//=======================================================================
bool SMESHGUI_FilterTable::Table::isEditable (int row, int col) const
{
  QTableWidgetItem* anItem = item( row, col );
  return anItem == 0 || anItem->flags() & Qt::ItemIsEditable;
}

void SMESHGUI_FilterTable::Table::setReadOnly( bool on )
{
  setEditTriggers( on ? 
                   QAbstractItemView::NoEditTriggers  :
                   QAbstractItemView::AllEditTriggers );
}

bool SMESHGUI_FilterTable::Table::isReadOnly() const
{
  return editTriggers() == QAbstractItemView::NoEditTriggers;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::insertRows
// Purpose : Insert rows (virtual redefined)
//=======================================================================
void SMESHGUI_FilterTable::Table::insertRows (int row, int count)
{
  closePersistentEditor( currentItem() );
  while ( count-- ) insertRow( row );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Table::text
// Purpose : Get text from cell (virtual redefined)
//=======================================================================
QString SMESHGUI_FilterTable::Table::text (int row, int col)
{
  closePersistentEditor( currentItem() );
  QTableWidgetItem* anItem = item( row, col );
  return anItem ? anItem->text() : QString();
}

QList<int> SMESHGUI_FilterTable::Table::selectedRows()
{
  QList<QTableWidgetItem*> selItems = selectedItems();
  QTableWidgetItem* anItem;
  QList<int> rows;

  foreach( anItem, selItems ) {
    int r = row( anItem );
    if ( !rows.contains( r ) ) rows.append( r );
  }

  qSort( rows );
  return rows;
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
SMESHGUI_FilterTable::SMESHGUI_FilterTable( SMESHGUI* theModule,
                                            QWidget* parent,
                                            const int type )
: QWidget( parent ),
  mySMESHGUI( theModule ),
  myIsLocked( false )
{
  myEntityType = -1;

  QList<int> aTypes;
  aTypes.append(type);
  Init(aTypes);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SMESHGUI_FilterTable
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterTable::SMESHGUI_FilterTable( SMESHGUI* theModule,
                                            QWidget* parent,
                                            const QList<int>& types )
: QWidget( parent ),
  mySMESHGUI( theModule ),
  myIsLocked( false )
{
  myEntityType = -1;
  Init(types);
}

SMESHGUI_FilterTable::~SMESHGUI_FilterTable()
{
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Init
// Purpose : Create table corresponding to the specified type
//=======================================================================
void SMESHGUI_FilterTable::Init (const QList<int>& theTypes)
{
  if (theTypes.isEmpty())
    return;

  // Create buttons if necessary

  if (myTables.isEmpty())
  {
    // create main layout
    QVBoxLayout* aMainLay = new QVBoxLayout(this);
    aMainLay->setMargin( 0 );
    aMainLay->setSpacing( SPACING );

    // create switch of entity types
    myEntityTypeBox = new QGroupBox(tr("ENTITY_TYPE"), this);
    QHBoxLayout* myEntityTypeBoxLayout = new QHBoxLayout(myEntityTypeBox);
    myEntityTypeBoxLayout->setMargin( MARGIN );
    myEntityTypeBoxLayout->setSpacing( SPACING );
    myEntityTypeGrp = new QButtonGroup(this);

    const QMap<int, QString>& aSupportedTypes = getSupportedTypes();
    QMap<int, QString>::const_iterator anIter;
    for (anIter = aSupportedTypes.begin(); anIter != aSupportedTypes.end(); ++anIter)
    {
      QRadioButton* aBtn = new QRadioButton(anIter.value(), myEntityTypeBox);
      myEntityTypeGrp->addButton(aBtn, anIter.key());
      myEntityTypeBoxLayout->addWidget(aBtn);
    }

    myTableGrp = new QGroupBox(tr("FILTER"), this );

    // create table
    mySwitchTableGrp = new QWidget(myTableGrp);
    QVBoxLayout* mySwitchTableGrpLayout = new QVBoxLayout(mySwitchTableGrp);
    mySwitchTableGrpLayout->setMargin(0);
    mySwitchTableGrpLayout->setSpacing(0);

    QList<int>::const_iterator typeIt = theTypes.begin();
    for ( ; typeIt != theTypes.end(); ++typeIt ) {
      Table* aTable = createTable(mySwitchTableGrp, *typeIt);
      myTables[ *typeIt ] = aTable;
      mySwitchTableGrpLayout->addWidget(aTable);
      if ( typeIt != theTypes.begin() )
        aTable->hide();
    }

    // create buttons
    myAddBtn      = new QPushButton(tr("ADD"),       myTableGrp);
    myRemoveBtn   = new QPushButton(tr("REMOVE"),    myTableGrp);
    myClearBtn    = new QPushButton(tr("CLEAR"),     myTableGrp);
    myInsertBtn   = new QPushButton(tr("INSERT"),    myTableGrp);
    myCopyFromBtn = new QPushButton(tr("COPY_FROM"), myTableGrp);
    myAddToBtn    = new QPushButton(tr("ADD_TO"),    myTableGrp);

    myAddBtn->setAutoDefault(false);
    myRemoveBtn->setAutoDefault(false);
    myClearBtn->setAutoDefault(false);
    myInsertBtn->setAutoDefault(false);
    myCopyFromBtn->setAutoDefault(false);
    myAddToBtn->setAutoDefault(false);

    myCopyFromBtn->hide();
    myAddToBtn->hide();

    // layout widgets
    QGridLayout* aLay = new QGridLayout(myTableGrp);
    aLay->setMargin(MARGIN);
    aLay->setSpacing(SPACING);

    aLay->addWidget(mySwitchTableGrp, 0, 0, 7, 1);
    aLay->addWidget(myAddBtn,         0, 1);
    aLay->addWidget(myInsertBtn,      1, 1);
    aLay->addWidget(myRemoveBtn,      2, 1);
    aLay->addWidget(myClearBtn,       3, 1);
    aLay->addWidget(myCopyFromBtn,    5, 1);
    aLay->addWidget(myAddToBtn,       6, 1);
    aLay->addWidget(createAdditionalFrame(myTableGrp), 7, 0, 1, 2 );

    aLay->setRowMinimumHeight(4, 10);
    aLay->setRowStretch(4, 1);
    aLay->setColumnStretch(0, 1);
    aLay->setColumnStretch(1, 0);

    // layout 
    aMainLay->addWidget(myEntityTypeBox);
    aMainLay->addWidget(myTableGrp);
    
    // signals and slots
    connect(myAddBtn,    SIGNAL(clicked()), this, SLOT(onAddBtn()));
    connect(myInsertBtn, SIGNAL(clicked()), this, SLOT(onInsertBtn()));
    connect(myRemoveBtn, SIGNAL(clicked()), this, SLOT(onRemoveBtn()));
    connect(myClearBtn,  SIGNAL(clicked()), this, SLOT(onClearBtn()));

    connect(myCopyFromBtn, SIGNAL(clicked()), this, SLOT(onCopyFromBtn()));
    connect(myAddToBtn,    SIGNAL(clicked()), this, SLOT(onAddToBtn()));

    connect(myEntityTypeGrp, SIGNAL(buttonClicked(int)), this, SLOT(onEntityType(int)));

    myLibDlg = 0;
  }
  else
  {
    QList<int>::const_iterator typeIt = theTypes.begin();
    for ( ; typeIt != theTypes.end(); ++typeIt ) {
      if ( !myTables[ *typeIt ] ) {
        Table* aTable = createTable(mySwitchTableGrp, *typeIt);
        myTables[ *typeIt ] = aTable;
        ((QVBoxLayout*)mySwitchTableGrp->layout())->addWidget(myTables[ *typeIt ]);
        myEntityType = -1;
      }
    }
  }

  // Hide buttons of entity types if necessary
  const QMap<int, QString>& aSupportedTypes = getSupportedTypes();
  QMap<int, QString>::const_iterator anIt;
  for (anIt = aSupportedTypes.begin(); anIt != aSupportedTypes.end(); ++anIt)
  {
    QAbstractButton* aBtn = myEntityTypeGrp->button(anIt.key());
    if ( aBtn ) aBtn->setVisible( theTypes.contains(anIt.key()) );
  }

  // select first button if there is no selected buttons or it is hidden
  int aBtnId = myEntityTypeGrp->checkedId();
  if ( aBtnId == -1 || !theTypes.contains(aBtnId) ) {
    QAbstractButton* aBtn = myEntityTypeGrp->button(theTypes.first());
    if ( aBtn ) aBtn->setChecked(true);
  }

  myEntityTypeBox->setVisible(theTypes.count() > 1);

  myTableGrp->updateGeometry();
  int cType = myEntityTypeGrp->checkedId();
  onEntityType(cType);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::createAdditionalFrame
// Purpose : Get group box containing table. May be used for adding new widgets in it
//=======================================================================
QWidget* SMESHGUI_FilterTable::createAdditionalFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  QFrame* aLine1 = new QFrame(aFrame);
  QFrame* aLine2 = new QFrame(aFrame);
  aLine1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  aLine2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  aLine1->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  aLine2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  QLabel* aLabel = new QLabel(tr("ADDITIONAL_PARAMETERS"), aFrame);

  myWgStack = new QStackedWidget(aFrame);

  QGridLayout* aLay = new QGridLayout(aFrame);
  aLay->setMargin(0);
  aLay->setSpacing(SPACING);
  aLay->addWidget(aLine1,    0, 0);
  aLay->addWidget(aLabel,    0, 1);
  aLay->addWidget(aLine2,    0, 2);
  aLay->addWidget(myWgStack, 1, 0, 1, 3);

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetTableGrp
// Purpose : Get group box containing table. May be used for adding new widgets in it
//=======================================================================
QGroupBox* SMESHGUI_FilterTable::GetTableGrp()
{
  return myTableGrp;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onEntityType
// Purpose : SLOT. Called when entity type changed.
//           Display corresponding table
//=======================================================================
void SMESHGUI_FilterTable::onEntityType (int theType)
{
  if (myEntityType == theType)
    return;

  myIsValid = true;
  emit NeedValidation();
  if (!myIsValid)
  {
    myEntityTypeGrp->button(myEntityType)->setChecked(true);
    return;
  }

  myEntityType = theType;

  if (!myTables.contains(theType)) {
    myTables[ theType ] = createTable(mySwitchTableGrp, theType);
    ((QVBoxLayout*)mySwitchTableGrp->layout())->addWidget(myTables[ theType ]);
  }

  TableMap::iterator anIter;
  for (anIter = myTables.begin(); anIter != myTables.end(); ++anIter)
     anIter.value()->setVisible( myEntityType == anIter.key() );

  updateBtnState();
  qApp->processEvents();
  myTables[ myEntityType ]->updateGeometry();
  adjustSize();

  emit EntityTypeChanged(theType);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::IsValid
// Purpose : Verify validity of entered data
//=======================================================================
bool SMESHGUI_FilterTable::IsValid (const bool theMess, const int theEntityType) const
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;

  Table* aTable = myTables[ aType ];
  for (int i = 0, n = aTable->rowCount(); i < n; i++)
  {
    int aCriterion = GetCriterionType(i, aType);
    QString errMsg;
    if (aCriterion == SMESH::FT_GroupColor )
    {
      QtxColorButton* clrBtn = qobject_cast<QtxColorButton*>(aTable->cellWidget(i, 2));
      if (clrBtn && !clrBtn->color().isValid())
        errMsg = tr( "GROUPCOLOR_ERROR" );
    }
    else if (aCriterion == SMESH::FT_RangeOfIds ||
             aCriterion == SMESH::FT_BelongToMeshGroup ||
             aCriterion == SMESH::FT_BelongToGeom ||
             aCriterion == SMESH::FT_BelongToPlane ||
             aCriterion == SMESH::FT_BelongToCylinder ||
             aCriterion == SMESH::FT_BelongToGenSurface ||
             aCriterion == SMESH::FT_ElemGeomType ||
             aCriterion == SMESH::FT_EntityType ||
             aCriterion == SMESH::FT_CoplanarFaces ||
             aCriterion == SMESH::FT_LyingOnGeom ||
             aCriterion == SMESH::FT_ConnectedElements )
    {
      if (aTable->text(i, 2).isEmpty())
        errMsg = tr( "ERROR" );
    }
    else // check correctness of a numeric value
    {
      bool aRes = false;
      bool isSignalsBlocked = aTable->signalsBlocked();
      aTable->blockSignals(true);
      /*double  aThreshold =*/ aTable->text(i, 2).toDouble( &aRes );
      aTable->blockSignals(isSignalsBlocked);

      if (!aRes && aTable->isEditable(i, 2))
        errMsg = tr( "ERROR" );
    }

    if (!errMsg.isEmpty()) {
      if (theMess)
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"), errMsg );
      return false;
    }

    QTableWidgetItem* anItem = aTable->item(i, 0);
    if (myAddWidgets.contains(anItem) && !myAddWidgets[ anItem ]->IsValid())
      return false;
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetValidity
// Purpose : Set validity of the table
//=======================================================================
void SMESHGUI_FilterTable::SetValidity (const bool isValid)
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
void SMESHGUI_FilterTable::SetType (const int type)
{
  myEntityTypeGrp->button(type)->setChecked(true);
  onEntityType(type);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::RestorePreviousEntityType
// Purpose : Restore previous entity type
//=======================================================================
void SMESHGUI_FilterTable::RestorePreviousEntityType()
{
  SetType(myEntityType);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetCriterionType
// Purpose : Get type of criterion from specified row (corresponding enums in h-file)
//=======================================================================
int SMESHGUI_FilterTable::GetCriterionType (const int theRow, const int theType) const
{
  int aType = theType == -1 ? GetType() : theType;
  Table* aTable = myTables[ aType ];
  ComboItem* anItem = (ComboItem*)aTable->item(theRow, 0);
  return anItem != 0 ? anItem->value() : SMESH::FT_Undefined;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetCriterion
// Purpose : Get parameters of criterion from specified row
//=======================================================================
void SMESHGUI_FilterTable::GetCriterion (const int                 theRow,
                                         SMESH::Filter::Criterion& theCriterion,
                                         const int                 theEntityType) const
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;
  Table* aTable = myTables[ aType ];

  theCriterion.Type = ((ComboItem*)aTable->item(theRow, 0))->value();
  theCriterion.UnaryOp = ((CheckItem*)aTable->item(theRow, 3))->checked() ? SMESH::FT_LogicalNOT : SMESH::FT_Undefined;
  theCriterion.BinaryOp = theRow != aTable->rowCount() - 1 ?
    ((ComboItem*)aTable->item(theRow, 4))->value() : SMESH::FT_Undefined;
  theCriterion.TypeOfElement = (SMESH::ElementType)aType;

  int aCriterionType = GetCriterionType(theRow, aType);

  if ( aCriterionType == SMESH::FT_GroupColor )
  {
    QtxColorButton* clrBtn = qobject_cast<QtxColorButton*>(aTable->cellWidget(theRow, 2));
    if ( clrBtn )
    {
      Kernel_Utils::Localizer loc;
      const QColor qClr = clrBtn->color();
      QString clrStr = QString( "%1;%2;%3" ).
        arg( qClr.red()/256. ).arg( qClr.green()/256. ).arg( qClr.blue()/256. );
      theCriterion.ThresholdStr = clrStr.toLatin1().constData();
    }
  }
  else if ( aCriterionType == SMESH::FT_ElemGeomType ||
            aCriterionType == SMESH::FT_EntityType )
  {
    theCriterion.Threshold = (double)((ComboItem*)aTable->item(theRow, 2))->value();
  }
  else if ( aCriterionType == SMESH::FT_CoplanarFaces )
  {
    theCriterion.ThresholdID = aTable->text(theRow, 2).toLatin1().constData();
  }
  else if ( aCriterionType == SMESH::FT_ConnectedElements )
  {
    QString id = aTable->text(theRow, 5);
    if ( !id.isEmpty() ) // shape ID
    {
      theCriterion.ThresholdID = id.toLatin1().constData();
    }
    else
    {
      QString text = aTable->text(theRow, 2).trimmed();
      QString workText = text;
      for ( char c = '0'; c <= '9'; ++c )
        workText.remove( c );

      if ( workText.isEmpty() ) // node ID
        theCriterion.Threshold = text.toDouble();
      else // point coordinates
        theCriterion.ThresholdStr = text.toLatin1().constData();
    }
  }
  else if ( aCriterionType != SMESH::FT_RangeOfIds &&
            aCriterionType != SMESH::FT_BelongToMeshGroup &&
            aCriterionType != SMESH::FT_BelongToGeom &&
            aCriterionType != SMESH::FT_BelongToPlane &&
            aCriterionType != SMESH::FT_BelongToCylinder &&
            aCriterionType != SMESH::FT_BelongToGenSurface &&
            aCriterionType != SMESH::FT_LyingOnGeom )
  {
    theCriterion.Compare = ((ComboItem*)aTable->item(theRow, 1))->value();
    theCriterion.Threshold = aTable->item(theRow, 2)->text().toDouble();
  }
  else
  {
    theCriterion.ThresholdStr = aTable->text(theRow, 2).toLatin1().constData();
    if ( aCriterionType != SMESH::FT_RangeOfIds )
      theCriterion.ThresholdID = aTable->text( theRow, 5 ).toLatin1().constData();
  }

  QTableWidgetItem* anItem = aTable->item(theRow, 0);
  if (myAddWidgets.contains(anItem))
    theCriterion.Tolerance = myAddWidgets[ anItem ]->GetDouble(AdditionalWidget::Tolerance);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetCriterion
// Purpose : Set parameters of criterion of specified row
//=======================================================================
void SMESHGUI_FilterTable::SetCriterion (const int                       theRow,
                                         const SMESH::Filter::Criterion& theCriterion,
                                         const int                       theEntityType)
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;

  Table* aTable = myTables[ aType ];

  if (theRow > aTable->rowCount() - 1)
    return;

  ((ComboItem*)aTable->item(theRow, 0))->setValue(theCriterion.Type);
  onCriterionChanged(theRow, 0, aType);
  if ( theCriterion.Compare == SMESH::FT_Undefined )
    ((ComboItem*)aTable->item(theRow, 1))->setValue( SMESH::FT_EqualTo );
  else
    ((ComboItem*)aTable->item(theRow, 1))->setValue(theCriterion.Compare);
  ((CheckItem*)aTable->item(theRow, 3))->setChecked(theCriterion.UnaryOp == SMESH::FT_LogicalNOT);

  if (theCriterion.BinaryOp != SMESH::FT_Undefined)
  {
    if (!aTable->isEditable(theRow, 4))
      aTable->setItem(theRow, 4, getBinaryItem());
    ((ComboItem*)aTable->item(theRow, 4))->setValue(theCriterion.BinaryOp);
  }
  else
    aTable->setEditable(false, theRow, 4);

  if (theCriterion.Type == SMESH::FT_GroupColor )
  {
    QtxColorButton* clrBtn = qobject_cast<QtxColorButton*>(aTable->cellWidget(theRow, 2));
    if ( clrBtn )
    {
      QColor qClr;
      QString clrStr( theCriterion.ThresholdStr );
      QStringList clrVals = clrStr.split( ";" );
      if ( clrVals.count() > 2 )
        qClr.setRgb( (int)256*clrVals[0].toDouble(),
                     (int)256*clrVals[1].toDouble(),
                     (int)256*clrVals[2].toDouble() );
      clrBtn->setColor( qClr );
    }
  }
  else if (theCriterion.Type == SMESH::FT_ElemGeomType || 
           theCriterion.Type == SMESH::FT_EntityType )
  {
    ComboItem* typeBox = (ComboItem*)aTable->item(theRow, 2);
    typeBox->setValue( (int)(theCriterion.Threshold + 0.5) );
  }
  else if (theCriterion.Type == SMESH::FT_CoplanarFaces )
  {
    aTable->item( theRow, 2 )->setText( QString( theCriterion.ThresholdID ) );
  }
  else if (theCriterion.Type == SMESH::FT_ConnectedElements )
  {
    if ( strlen( theCriterion.ThresholdID ) > 0 ) // shape ID -> name
    {
      _PTR(SObject) sobj =
        SMESH::GetActiveStudyDocument()->FindObjectID( theCriterion.ThresholdID.in() );
      if ( !sobj )
        aTable->item( theRow, 2 )->setText( QString( theCriterion.ThresholdID ) );
      else
        aTable->item( theRow, 2 )->setText( QString( sobj->GetName().c_str() ));
    }
    else if ( strlen( theCriterion.ThresholdStr ) > 0 ) // point coords
    {
      aTable->item( theRow, 2 )->setText( QString( theCriterion.ThresholdStr ));
    }
    else // node ID
    {
      aTable->item( theRow, 2 )->setText( QString("%1").arg((int) theCriterion.Threshold ));
    }
  }
  else if (theCriterion.Type != SMESH::FT_RangeOfIds &&
           theCriterion.Type != SMESH::FT_BelongToMeshGroup &&
           theCriterion.Type != SMESH::FT_BelongToGeom &&
           theCriterion.Type != SMESH::FT_BelongToPlane &&
           theCriterion.Type != SMESH::FT_BelongToCylinder &&
           theCriterion.Type != SMESH::FT_BelongToGenSurface &&
           theCriterion.Type != SMESH::FT_LyingOnGeom &&
           theCriterion.Type != SMESH::FT_FreeBorders &&
           theCriterion.Type != SMESH::FT_FreeEdges &&
           theCriterion.Type != SMESH::FT_FreeNodes &&
           theCriterion.Type != SMESH::FT_FreeFaces &&
           theCriterion.Type != SMESH::FT_BadOrientedVolume &&
           theCriterion.Type != SMESH::FT_BareBorderFace &&
           theCriterion.Type != SMESH::FT_BareBorderVolume &&
           theCriterion.Type != SMESH::FT_OverConstrainedFace &&
           theCriterion.Type != SMESH::FT_OverConstrainedVolume &&
           theCriterion.Type != SMESH::FT_LinearOrQuadratic)
  {
    // Numberic criterion
    aTable->item( theRow, 2 )->setText(QString("%1").arg(theCriterion.Threshold, 0, 'g', 15));
  }
  else
  {
    aTable->item( theRow, 2 )->setText(QString(theCriterion.ThresholdStr));
    if ( theCriterion.Type != SMESH::FT_RangeOfIds )
      aTable->item( theRow, 5 )->setText( QString( theCriterion.ThresholdID ) );
  }

  if (theCriterion.Compare == SMESH::FT_EqualTo ||
      theCriterion.Type    == SMESH::FT_BelongToPlane ||
      theCriterion.Type    == SMESH::FT_BelongToCylinder ||
      theCriterion.Type    == SMESH::FT_BelongToGenSurface ||
      theCriterion.Type    == SMESH::FT_BelongToGeom ||
      theCriterion.Type    == SMESH::FT_LyingOnGeom ||
      theCriterion.Type    == SMESH::FT_CoplanarFaces ||
      theCriterion.Type    == SMESH::FT_EqualNodes)
  {
    QTableWidgetItem* anItem = aTable->item(theRow, 0);
    if (!myAddWidgets.contains(anItem))
    {
      myAddWidgets[ anItem ] = new AdditionalWidget(myWgStack);
      myAddWidgets[ anItem ]->SetPrecision( AdditionalWidget::Tolerance, getPrecision( theCriterion.Type ) );
      myWgStack->addWidget(myAddWidgets[ anItem ]);
    }
    myAddWidgets[ anItem ]->SetDouble(AdditionalWidget::Tolerance, theCriterion.Tolerance);
  }

  emit CriterionChanged(theRow, aType);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Update
// Purpose : Update table
//=======================================================================
void SMESHGUI_FilterTable::Update()
{
  Table* aTable = myTables[ GetType() ];
  int aCurrRow = aTable->currentRow();
  int numRows = aTable->rowCount();
  if ((aCurrRow < 0 || aCurrRow >= numRows) && numRows > 0)
    aTable->setCurrentCell(0, 0);
  updateAdditionalWidget();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::AddCriterion
// Purpose : Add criterion with parameters
//=======================================================================
void SMESHGUI_FilterTable::AddCriterion (const SMESH::Filter::Criterion& theCriterion,
                                         const int                       theEntityType)
{
  int aType = theEntityType == -1 ? GetType() : theEntityType;
  Table* aTable = myTables[ aType ];
  addRow(aTable, aType);
  SetCriterion(aTable->rowCount() - 1, theCriterion);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::NumRows
// Purpose : Get number of criterions of current type
//=======================================================================
int SMESHGUI_FilterTable::NumRows (const int theEntityType) const
{
  return myTables[ theEntityType == -1 ? GetType() : theEntityType ]->rowCount();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Clear
// Purpose : Clear current table
//=======================================================================
void SMESHGUI_FilterTable::Clear (const int theType)
{
  int aType = theType == -1 ? GetType() : theType;
  Table* aTable = myTables[ aType ];

  if (aTable->rowCount() == 0)
    return;

  while (aTable->rowCount() > 0)
  {
    removeAdditionalWidget(aTable, 0);
    aTable->removeRow(0);
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
  addRow(myTables[ aType ], aType);

  Update();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onInsertBtn
// Purpose : SLOT. Called then "Insert" button pressed.
//           Inserts new string before current one
//=======================================================================
void SMESHGUI_FilterTable::onInsertBtn()
{
  addRow(myTables[ GetType() ], GetType(), false);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onRemoveBtn
// Purpose : SLOT. Called then "Remove" button pressed.
//           Removes current string from table
//=======================================================================
void SMESHGUI_FilterTable::onRemoveBtn()
{
  Table* aTable = myTables[ GetType() ];

  if (aTable->rowCount() == 0)
    return;

  QList<QTableWidgetItem*> items = aTable->selectedItems();
  
  QList<int> aRows = aTable->selectedRows(); // already sorted
  int i;
  foreach( i, aRows )
  {
    removeAdditionalWidget(aTable, i);
    aTable->removeRow(i);
  }

  // remove control of binary logical operation from last row
  if (aTable->rowCount() > 0)
    aTable->setEditable(false, aTable->rowCount() - 1, 4);

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
  if (aRow < 0 || aRow >= aTable->rowCount())
  {
    myWgStack->setEnabled(false);
    return;
  }

  ComboItem* anItem = ((ComboItem*)aTable->item(aRow, 0));
  int aCriterion = GetCriterionType(aRow);
  bool isDbl = ( aCriterion == SMESH::FT_AspectRatio        ||
                 aCriterion == SMESH::FT_AspectRatio3D      ||
                 aCriterion == SMESH::FT_Warping            ||
                 aCriterion == SMESH::FT_MinimumAngle       ||
                 aCriterion == SMESH::FT_Taper              ||
                 aCriterion == SMESH::FT_Skew               ||
                 aCriterion == SMESH::FT_Area               ||
                 aCriterion == SMESH::FT_Volume3D           ||
                 aCriterion == SMESH::FT_MaxElementLength2D ||
                 aCriterion == SMESH::FT_MaxElementLength3D ||
                 aCriterion == SMESH::FT_Length             ||
                 aCriterion == SMESH::FT_Length2D           ||
                 aCriterion == SMESH::FT_BallDiameter );

  bool toEnable = (( isDbl && ((ComboItem*)aTable->item(aRow, 1))->value() == SMESH::FT_EqualTo) ||
                   aCriterion == SMESH::FT_BelongToPlane                                         ||
                   aCriterion == SMESH::FT_BelongToCylinder                                      ||
                   aCriterion == SMESH::FT_BelongToGenSurface                                    ||
                   aCriterion == SMESH::FT_BelongToGeom                                          ||
                   aCriterion == SMESH::FT_LyingOnGeom                                           ||
                   aCriterion == SMESH::FT_CoplanarFaces                                         ||
                   aCriterion == SMESH::FT_EqualNodes);

  if (!myAddWidgets.contains(anItem))
  {
    myAddWidgets[ anItem ] = new AdditionalWidget(myWgStack);
    myWgStack->addWidget(myAddWidgets[ anItem ]);
  }

  myWgStack->setCurrentWidget(myAddWidgets[ anItem ]);
  myAddWidgets[ anItem ]->SetPrecision( AdditionalWidget::Tolerance, getPrecision( aCriterion ) );
  myWgStack->setEnabled(toEnable);
}

const char* SMESHGUI_FilterTable::getPrecision( const int aType )
{
  const char* retval = 0;
  switch ( aType ) {
  case SMESH::FT_AspectRatio:
  case SMESH::FT_AspectRatio3D:
  case SMESH::FT_Taper:
    retval = "parametric_precision"; break;
  case SMESH::FT_Warping:
  case SMESH::FT_MinimumAngle:
  case SMESH::FT_Skew:
  case SMESH::FT_CoplanarFaces:
    retval = "angle_precision"; break;
  case SMESH::FT_Area:
    retval = "area_precision"; break;
  case SMESH::FT_BelongToGeom:
  case SMESH::FT_BelongToPlane:
  case SMESH::FT_BelongToCylinder:
  case SMESH::FT_BelongToGenSurface:
  case SMESH::FT_LyingOnGeom:
  case SMESH::FT_EqualNodes:
    retval = "len_tol_precision"; break;
  case SMESH::FT_Length:
  case SMESH::FT_Length2D:
  case SMESH::FT_MaxElementLength2D:
  case SMESH::FT_MaxElementLength3D:
  case SMESH::FT_BallDiameter:
    retval = "length_precision"; break;
  case SMESH::FT_Volume3D:
    retval = "vol_precision"; break;
  default:
    break;
  }
  return retval;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::removeAdditionalWidget
// Purpose : Remove widgets containing additional parameters from widget
//           stack and internal map
//=======================================================================
void SMESHGUI_FilterTable::removeAdditionalWidget (QTableWidget* theTable, const int theRow)
{
  QTableWidgetItem* anItem = theTable->item(theRow, 0);
  if (myAddWidgets.contains(anItem))
  {
    myWgStack->removeWidget(myAddWidgets[ anItem ]);
    myAddWidgets[ anItem ]->setParent(0);
    delete myAddWidgets[ anItem ];
    myAddWidgets.remove(anItem);
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onClearBtn
// Purpose : SLOT. Called then "Clear" button pressed.
//           Removes all strings from table
//=======================================================================
void SMESHGUI_FilterTable::onClearBtn()
{
  Table* aTable = myTables[ GetType() ];

  if (aTable->rowCount() == 0)
    return;

  while (aTable->rowCount() > 0)
  {
    removeAdditionalWidget(aTable, 0);
    aTable->removeRow(0);
  }

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCurrentChanged()
// Purpose : SLOT. Called when current cell changed
//=======================================================================
void SMESHGUI_FilterTable::onCurrentChanged (int theRow, int theCol)
{
  if( !myIsLocked )
    updateAdditionalWidget();
  emit CurrentChanged(theRow, theCol);
}

//=======================================================================
// name    : geomTypes
// Purpose : returns available geometry types of elements
//=======================================================================
static QList<int> geomTypes( const int theType )
{
  QList<int> typeIds;
  if ( theType == SMESH::NODE )
    typeIds.append( SMESH::Geom_POINT );
  if ( theType == SMESH::ALL || theType == SMESH::EDGE )
    typeIds.append( SMESH::Geom_EDGE );
  if ( theType == SMESH::ALL || theType == SMESH::FACE )
  {
    typeIds.append( SMESH::Geom_TRIANGLE );
    typeIds.append( SMESH::Geom_QUADRANGLE );
    typeIds.append( SMESH::Geom_POLYGON );
  }
  if ( theType == SMESH::ALL || theType == SMESH::VOLUME )
  {
    typeIds.append( SMESH::Geom_TETRA );
    typeIds.append( SMESH::Geom_PYRAMID );
    typeIds.append( SMESH::Geom_HEXA );
    typeIds.append( SMESH::Geom_PENTA );
    typeIds.append( SMESH::Geom_HEXAGONAL_PRISM );
    typeIds.append( SMESH::Geom_POLYHEDRA );
  }
  if ( theType == SMESH::ALL || theType == SMESH::ELEM0D )
  {
    typeIds.append( SMESH::Geom_POINT );
  }
  if ( theType == SMESH::ALL || theType == SMESH::BALL )
  {
    typeIds.append( SMESH::Geom_BALL );
  }
  return typeIds;
}

//=======================================================================
// name    : entityTypes
// Purpose : returns available entity types of elements
//=======================================================================

static QList<int> entityTypes( const int theType )
{
  QList<int> typeIds;

  switch ( theType )
  {
  case SMESH::NODE:
    typeIds.append( SMDSEntity_Node );
  case SMESH::EDGE:
    typeIds.append( SMDSEntity_Edge );
    typeIds.append( SMDSEntity_Quad_Edge );
    break;
  case SMESH::FACE:
    typeIds.append( SMDSEntity_Triangle );
    typeIds.append( SMDSEntity_Quad_Triangle );
    typeIds.append( SMDSEntity_BiQuad_Triangle );
    typeIds.append( SMDSEntity_Quadrangle );
    typeIds.append( SMDSEntity_Quad_Quadrangle );
    typeIds.append( SMDSEntity_BiQuad_Quadrangle );
    typeIds.append( SMDSEntity_Polygon );
    typeIds.append( SMDSEntity_Quad_Polygon );
    break;
  case SMESH::VOLUME:
    typeIds.append( SMDSEntity_Tetra );
    typeIds.append( SMDSEntity_Quad_Tetra );
    typeIds.append( SMDSEntity_Pyramid );
    typeIds.append( SMDSEntity_Quad_Pyramid );
    typeIds.append( SMDSEntity_Hexa );
    typeIds.append( SMDSEntity_Quad_Hexa );
    typeIds.append( SMDSEntity_TriQuad_Hexa );
    typeIds.append( SMDSEntity_Penta );
    typeIds.append( SMDSEntity_Quad_Penta );
    typeIds.append( SMDSEntity_Hexagonal_Prism );
    typeIds.append( SMDSEntity_Polyhedra );
    //typeIds.append( SMDSEntity_Quad_Polyhedra );
    break;
  case SMESH::ELEM0D:
    typeIds.append( SMDSEntity_0D );
    break;
  case SMESH::BALL:
    typeIds.append( SMDSEntity_Ball );
    break;
  }
  return typeIds;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCriterionChanged()
// Purpose : Provides reaction on change of criterion
//=======================================================================

void SMESHGUI_FilterTable::onCriterionChanged (const int row, const int col, const int entityType)
{
  int aType = entityType == -1 ? GetType() : entityType;
  Table* aTable = myTables[ aType ];
  ComboItem* aCompareItem = (ComboItem*)aTable->item(row, 1);

  // find out type of a existing threshould table item
  QtxColorButton* clrBtn = qobject_cast<QtxColorButton*>(aTable->cellWidget(row, 2));
  bool isComboItem       = false;
  bool isIntSpinItem     = false;
  bool isDoubleSpinItem  = false;
  if ( QTableWidgetItem* aTableItem = aTable->item(row, 2) )
  {
    int aTableType   = aTableItem->type();
    isComboItem      = ( aTableType == ComboItem::Type() );
    isIntSpinItem    = ( aTableType == IntSpinItem::Type() );
    isDoubleSpinItem = ( aTableType == DoubleSpinItem::Type() );
  }

  // find out a type of item required by a new criterion and other table features
  int aCriterionType       = GetCriterionType(row);
  bool anIsDoubleCriterion = false;
  bool anIsIntCriterion    = false;
  bool anIsComboCriterion  = false;
  // other features:
  QList<int> comboIDs; // values to show in a combo item
  int nbCompareSigns = 0; // possible values are 0,1,3
  bool isThresholdEditable = false; // actual for "simple" item types
  switch ( aCriterionType )
  {
  case SMESH::FT_AspectRatio:
  case SMESH::FT_AspectRatio3D:
  case SMESH::FT_Warping:
  case SMESH::FT_MinimumAngle:
  case SMESH::FT_Taper:
  case SMESH::FT_Skew:
  case SMESH::FT_Area:
  case SMESH::FT_Volume3D:
  case SMESH::FT_MaxElementLength2D:
  case SMESH::FT_MaxElementLength3D:
    anIsDoubleCriterion = true; break;

  case SMESH::FT_FreeBorders:
  case SMESH::FT_FreeEdges:
  case SMESH::FT_FreeNodes:
  case SMESH::FT_FreeFaces:
  case SMESH::FT_EqualNodes:
  case SMESH::FT_EqualEdges:
  case SMESH::FT_EqualFaces:
  case SMESH::FT_EqualVolumes: break;

  case SMESH::FT_NodeConnectivityNumber:
  case SMESH::FT_MultiConnection:
  case SMESH::FT_MultiConnection2D: anIsIntCriterion = true; nbCompareSigns = 3; break;

  case SMESH::FT_Length:
  case SMESH::FT_Length2D: anIsDoubleCriterion = true; break;

  case SMESH::FT_BelongToMeshGroup: break;

  case SMESH::FT_BelongToGeom:
  case SMESH::FT_BelongToPlane:
  case SMESH::FT_BelongToCylinder:
  case SMESH::FT_BelongToGenSurface:
  case SMESH::FT_LyingOnGeom: nbCompareSigns = 0; isThresholdEditable = true; break;

  case SMESH::FT_RangeOfIds: nbCompareSigns = 0; isThresholdEditable = true; break;

  case SMESH::FT_BadOrientedVolume:
  case SMESH::FT_BareBorderVolume:
  case SMESH::FT_BareBorderFace:
  case SMESH::FT_OverConstrainedVolume:
  case SMESH::FT_OverConstrainedFace:
  case SMESH::FT_LinearOrQuadratic: break;

  case SMESH::FT_GroupColor: nbCompareSigns = 1; isThresholdEditable = true; break;

  case SMESH::FT_ElemGeomType:
    comboIDs = geomTypes( aType ); anIsComboCriterion = true; nbCompareSigns = 1; break;

  case SMESH::FT_EntityType:
    comboIDs = entityTypes( aType ); anIsComboCriterion = true; nbCompareSigns = 1; break;

  case SMESH::FT_CoplanarFaces: isThresholdEditable = true; break;

  case SMESH::FT_BallDiameter: anIsDoubleCriterion = true; break;

  case SMESH::FT_ConnectedElements: isThresholdEditable = true; break;

  case SMESH::FT_LessThan:
  case SMESH::FT_MoreThan:
  case SMESH::FT_EqualTo:
  case SMESH::FT_LogicalNOT:
  case SMESH::FT_LogicalAND:
  case SMESH::FT_LogicalOR:
  case SMESH::FT_Undefined:
  default: return;
  }

  // get a precision of a double criterion
  int aPrecision = 0;
  if ( anIsDoubleCriterion ) {
    const char*     aPrecisionType = getPrecision( aCriterionType );
    SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr( mySMESHGUI );
    if( aPrecisionType && aResourceMgr )
      aPrecision = aResourceMgr->integerValue( "SMESH", aPrecisionType, aPrecision );
  }

  // check if the current item type satisfies the criterion
  bool itemTypeKO =
    ( aCriterionType == SMESH::FT_GroupColor && !clrBtn) ||
    ( anIsComboCriterion                     && !isComboItem ) ||
    ( anIsIntCriterion                       && !isIntSpinItem ) ||
    ( anIsDoubleCriterion                    && !isDoubleSpinItem );

  if ( !itemTypeKO )
  {
    if ( anIsDoubleCriterion )
    {
      // if the precision is to be changed we should remove the existing
      // spin item and create another one with new precision
      if ( DoubleSpinItem* aDoubleSpinItem = dynamic_cast<DoubleSpinItem*>( aTable->item( row, 2 )))
        itemTypeKO = ( aDoubleSpinItem->precision() != aPrecision );
      else
        itemTypeKO = true;
    }
    else if ( anIsComboCriterion )
    {
      if ( ComboItem* aComboItem = dynamic_cast<ComboItem*>( aTable->item( row, 2 )))
        itemTypeKO = ( aComboItem->count() != comboIDs.count() );
      else
        itemTypeKO = true;
    }
    else if ( !anIsIntCriterion && aCriterionType != SMESH::FT_GroupColor )
    {
      itemTypeKO = ( clrBtn || isComboItem || isIntSpinItem || isDoubleSpinItem );
    }
  }

  // update the table row

  bool isSignalsBlocked = aTable->signalsBlocked();
  aTable->blockSignals( true );

  // update threshold table item
  if ( itemTypeKO )
  {
    aTable->removeCellWidget( row, 2 );

    if ( aCriterionType == SMESH::FT_GroupColor ) // ---------------------- QtxColorButton
    {
      aTable->setCellWidget( row, 2, new QtxColorButton( aTable ) );
    }
    else if ( anIsComboCriterion ) // -------------------------------------------ComboItem
    {
      QString msgPrefix
        ( aCriterionType == SMESH::FT_ElemGeomType  ? "GEOM_TYPE_%1" :  "ENTITY_TYPE_%1" );
      QMap<int, QString> names;
      QList<int>::const_iterator id = comboIDs.begin();
      for ( ; id != comboIDs.end(); ++id )
      {
        QString name = msgPrefix.arg( *id );
        names[ *id ] = tr( name.toLatin1().data() );
      }
      ComboItem* comboBox = new ComboItem( names );
      aTable->setItem( row, 2, comboBox );
    }
    else if ( anIsIntCriterion ) // ------------------------------------------ IntSpinItem
    {
      IntSpinItem* intSpin = new IntSpinItem( 0 );
      aTable->setItem( row, 2, intSpin );
    }
    else if ( anIsDoubleCriterion ) // -------------------------------------DoubleSpinItem
    {
      DoubleSpinItem* dblSpin = new DoubleSpinItem( 0 );
      dblSpin->setPrecision( aPrecision );
      aTable->setItem( row, 2, dblSpin );
    }
    else // --------------------------------------------------------------QTableWidgetItem
    {
      aTable->setItem( row, 2, new QTableWidgetItem() );
    }
  }

  // set Compare
  if ( anIsDoubleCriterion )
    nbCompareSigns = 3;
  if ( aCompareItem->count() != nbCompareSigns )
  {
    switch ( nbCompareSigns ) {
    case 0: {
      aCompareItem->clear();
      break;
    }
    case 1: {
      QMap<int, QString> aMap;
      aMap[ SMESH::FT_EqualTo ] = tr("EQUAL_TO");
      aCompareItem->setItems(aMap);
      break;
    }
    case 3: {
      int oldValue = aCompareItem->value();
      aCompareItem->setItems(getCompare());
      if ( oldValue >= 0 )
        aCompareItem->setValue( oldValue );
      break;
    }
    }
  }
  aTable->setEditable( nbCompareSigns == 3, row, 1);

  // enable/desable Threshold
  if ( aCriterionType == SMESH::FT_GroupColor ||
       anIsComboCriterion ||
       anIsIntCriterion ||
       anIsDoubleCriterion )
  {
    isThresholdEditable = true;
  }
  if ( !isThresholdEditable )
  {
    aTable->setItem( row, 2, new QTableWidgetItem() );
  }
  aTable->setEditable( isThresholdEditable, row, 2);


  aTable->blockSignals( isSignalsBlocked );

  updateAdditionalWidget();

  emit CriterionChanged(row, entityType);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::onCriterionChanged()
// Purpose : SLOT. Called then contents of table changed
//           Provides reaction on change of criterion
//=======================================================================
void SMESHGUI_FilterTable::onCriterionChanged (int row, int col)
{
  if( col == 0 )
    onCriterionChanged(row, col, -1);
  else if ( col == 2 )
    emit ThresholdChanged(row, GetType());
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getFirstSelectedRow
// Purpose : Get first selected row
//=======================================================================
int SMESHGUI_FilterTable::getFirstSelectedRow() const
{
  Table* aTable = myTables[ GetType() ];

  QList<int> selRows = aTable->selectedRows(); // already sorted
  int aRow = selRows.count() > 0 ? selRows[0] : aTable->currentRow();

  return aRow >= 0 && aRow < aTable->rowCount() ? aRow : -1;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::addRow
// Purpose : Add row at the end of table
//=======================================================================
void SMESHGUI_FilterTable::addRow (Table* theTable, const int theType, const bool toTheEnd)
{
  int aCurrRow = 0;
  int aSelectedRow = getFirstSelectedRow();
  int aCurrCol = theTable->currentColumn();

  myIsLocked = true;
  if (toTheEnd || aSelectedRow == -1)
  {
    theTable->insertRows(theTable->rowCount());
    aCurrRow = theTable->rowCount() - 1;
  }
  else
  {
    theTable->insertRows(aSelectedRow);
    aCurrRow = aSelectedRow;
  }
  myIsLocked = false;

  // IPAL19372 - to prevent calling onCriterionChaged() slot before completion of setItem()
  bool isSignalsBlocked = theTable->signalsBlocked();
  theTable->blockSignals( true );

  // Criteria
  theTable->setItem(aCurrRow, 0, getCriterionItem(theType));

  // Compare
  theTable->setItem(aCurrRow, 1, getCompareItem());

  // Threshold
  theTable->setItem(aCurrRow, 2, new QTableWidgetItem());

  // Logical operation NOT
  theTable->setItem(aCurrRow, 3, getUnaryItem());

  // Logical operation AND / OR
  theTable->setItem(aCurrRow, 4, new QTableWidgetItem());

  theTable->setItem(aCurrRow, 5, new QTableWidgetItem());
    
  theTable->blockSignals( isSignalsBlocked );

  // Logical binary operation for previous value
  int anAddBinOpStr = -1;
  if (aCurrRow == theTable->rowCount() - 1)
    anAddBinOpStr = aCurrRow - 1;
  else if (aCurrRow >= 0 )
    anAddBinOpStr = aCurrRow;

  if (theTable->item(aCurrRow, 4) == 0 ||
       theTable->item(aCurrRow, 4)->type() != ComboItem::Type())
  {
    if (anAddBinOpStr >= 0 &&
         (theTable->item(anAddBinOpStr, 4) == 0 ||
           theTable->item(anAddBinOpStr, 4)->type() != ComboItem::Type()))
      theTable->setItem(anAddBinOpStr, 4, getBinaryItem());
  }

  theTable->setEditable(false, theTable->rowCount() - 1, 4);
  
  if (aCurrRow >=0 && aCurrRow < theTable->rowCount() &&
       aCurrCol >=0 && aCurrCol < theTable->rowCount())
  theTable->setCurrentCell(aCurrRow, aCurrCol);

  onCriterionChanged(aCurrRow, 0);

  updateBtnState();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getCriterionItem
// Purpose : Get combo table item for criteria of specified type
//=======================================================================
QTableWidgetItem* SMESHGUI_FilterTable::getCriterionItem (const int theType) const
{
  return new ComboItem(getCriteria(theType));
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getCompareItem
// Purpose : Get combo table item for operation of comparision
//=======================================================================
QTableWidgetItem* SMESHGUI_FilterTable::getCompareItem () const
{
  return new ComboItem(getCompare());
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getBinaryItem
// Purpose :
//=======================================================================
QTableWidgetItem* SMESHGUI_FilterTable::getBinaryItem () const
{
  static QMap<int, QString> aMap;
  if (aMap.isEmpty())
  {
    aMap[ SMESH::FT_LogicalAND ] = tr("AND");
    aMap[ SMESH::FT_LogicalOR  ] = tr("OR");
  }

  return new ComboItem(aMap);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getUnaryItem
// Purpose : Get check table item
//=======================================================================
QTableWidgetItem* SMESHGUI_FilterTable::getUnaryItem () const
{
  return new CheckItem(tr("NOT"));
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getSupportedTypes
// Purpose : Get all supported type
//=======================================================================
const QMap<int, QString>& SMESHGUI_FilterTable::getSupportedTypes() const
{
  static QMap<int, QString> aTypes;
  if (aTypes.isEmpty())
  {
    aTypes[ SMESH::NODE   ] = tr("NODES");
    aTypes[ SMESH::ELEM0D ] = tr("ELEM0D");
    aTypes[ SMESH::BALL   ] = tr("BALLS");
    aTypes[ SMESH::EDGE   ] = tr("EDGES");
    aTypes[ SMESH::FACE   ] = tr("FACES");
    aTypes[ SMESH::VOLUME ] = tr("VOLUMES");
    aTypes[ SMESH::ALL ]    = tr("ELEMENTS");
  }

  return aTypes;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::getCriteria
// Purpose : Get criteria for specified type
//=======================================================================
const QMap<int, QString>& SMESHGUI_FilterTable::getCriteria (const int theType) const
{
  if (theType == SMESH::NODE)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_BelongToPlane      ] = tr("BELONG_TO_PLANE");
      aCriteria[ SMESH::FT_BelongToCylinder   ] = tr("BELONG_TO_CYLINDER");
      aCriteria[ SMESH::FT_BelongToGenSurface ] = tr("BELONG_TO_GENSURFACE");
      aCriteria[ SMESH::FT_LyingOnGeom        ] = tr("LYING_ON_GEOM");
      aCriteria[ SMESH::FT_FreeNodes          ] = tr("FREE_NODES");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_EqualNodes         ] = tr("EQUAL_NODE");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
      aCriteria[ SMESH::FT_NodeConnectivityNumber ] = tr("NODE_CONN_NUMBER");
    }
    return aCriteria;
  }
  else if (theType == SMESH::EDGE)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_FreeBorders        ] = tr("FREE_BORDERS");
      aCriteria[ SMESH::FT_MultiConnection    ] = tr("MULTI_BORDERS");
      aCriteria[ SMESH::FT_Length             ] = tr("LENGTH");
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_BelongToPlane      ] = tr("BELONG_TO_PLANE");
      aCriteria[ SMESH::FT_BelongToCylinder   ] = tr("BELONG_TO_CYLINDER");
      aCriteria[ SMESH::FT_BelongToGenSurface ] = tr("BELONG_TO_GENSURFACE");
      aCriteria[ SMESH::FT_LyingOnGeom        ] = tr("LYING_ON_GEOM");
      aCriteria[ SMESH::FT_LinearOrQuadratic  ] = tr("LINEAR");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ElemGeomType       ] = tr("GEOM_TYPE");
      aCriteria[ SMESH::FT_EqualEdges         ] = tr("EQUAL_EDGE");
      aCriteria[ SMESH::FT_EntityType         ] = tr("ENTITY_TYPE");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
    }
    return aCriteria;
  }
  else if (theType == SMESH::FACE)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_AspectRatio        ] = tr("ASPECT_RATIO");
      aCriteria[ SMESH::FT_Warping            ] = tr("WARPING");
      aCriteria[ SMESH::FT_MinimumAngle       ] = tr("MINIMUM_ANGLE");
      aCriteria[ SMESH::FT_Taper              ] = tr("TAPER");
      aCriteria[ SMESH::FT_Skew               ] = tr("SKEW");
      aCriteria[ SMESH::FT_Area               ] = tr("AREA");
      aCriteria[ SMESH::FT_MaxElementLength2D ] = tr("MAX_ELEMENT_LENGTH_2D");
      aCriteria[ SMESH::FT_FreeEdges          ] = tr("FREE_EDGES");
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_BelongToPlane      ] = tr("BELONG_TO_PLANE");
      aCriteria[ SMESH::FT_BelongToCylinder   ] = tr("BELONG_TO_CYLINDER");
      aCriteria[ SMESH::FT_BelongToGenSurface ] = tr("BELONG_TO_GENSURFACE");
      aCriteria[ SMESH::FT_LyingOnGeom        ] = tr("LYING_ON_GEOM");
      aCriteria[ SMESH::FT_Length2D           ] = tr("LENGTH2D");
      aCriteria[ SMESH::FT_MultiConnection2D  ] = tr("MULTI2D_BORDERS");
      aCriteria[ SMESH::FT_FreeFaces          ] = tr("FREE_FACES");
      aCriteria[ SMESH::FT_BareBorderFace     ] = tr("BARE_BORDER_FACE");
      aCriteria[ SMESH::FT_OverConstrainedFace] = tr("OVER_CONSTRAINED_FACE");
      aCriteria[ SMESH::FT_LinearOrQuadratic  ] = tr("LINEAR");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ElemGeomType       ] = tr("GEOM_TYPE");
      aCriteria[ SMESH::FT_CoplanarFaces      ] = tr("COPLANAR_FACES");
      aCriteria[ SMESH::FT_EqualFaces         ] = tr("EQUAL_FACE");
      aCriteria[ SMESH::FT_EntityType         ] = tr("ENTITY_TYPE");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
    }
    return aCriteria;
  }
  else if (theType == SMESH::VOLUME)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_AspectRatio3D        ] = tr("ASPECT_RATIO_3D");
      aCriteria[ SMESH::FT_RangeOfIds           ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup    ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom         ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_LyingOnGeom          ] = tr("LYING_ON_GEOM");
      aCriteria[ SMESH::FT_BadOrientedVolume    ] = tr("BAD_ORIENTED_VOLUME");
      aCriteria[ SMESH::FT_BareBorderVolume     ] = tr("BARE_BORDER_VOLUME");
      aCriteria[ SMESH::FT_OverConstrainedVolume] = tr("OVER_CONSTRAINED_VOLUME");
      aCriteria[ SMESH::FT_Volume3D             ] = tr("VOLUME_3D");
      aCriteria[ SMESH::FT_MaxElementLength3D   ] = tr("MAX_ELEMENT_LENGTH_3D");
      aCriteria[ SMESH::FT_LinearOrQuadratic    ] = tr("LINEAR");
      aCriteria[ SMESH::FT_GroupColor           ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ElemGeomType         ] = tr("GEOM_TYPE");
      aCriteria[ SMESH::FT_EqualVolumes         ] = tr("EQUAL_VOLUME");
      aCriteria[ SMESH::FT_EntityType           ] = tr("ENTITY_TYPE");
      aCriteria[ SMESH::FT_ConnectedElements    ] = tr("CONNECTED_ELEMS");
    }
    return aCriteria;
  }
  else if (theType == SMESH::ELEM0D)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_BelongToPlane      ] = tr("BELONG_TO_PLANE");
      aCriteria[ SMESH::FT_BelongToCylinder   ] = tr("BELONG_TO_CYLINDER");
      aCriteria[ SMESH::FT_BelongToGenSurface ] = tr("BELONG_TO_GENSURFACE");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
    }
    return aCriteria;
  }
  else if (theType == SMESH::BALL)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_BallDiameter       ] = tr("BALL_DIAMETER");
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_BelongToPlane      ] = tr("BELONG_TO_PLANE");
      aCriteria[ SMESH::FT_BelongToCylinder   ] = tr("BELONG_TO_CYLINDER");
      aCriteria[ SMESH::FT_BelongToGenSurface ] = tr("BELONG_TO_GENSURFACE");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
    }
    return aCriteria;
  }
  else if (theType == SMESH::ELEM0D)
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_BelongToPlane      ] = tr("BELONG_TO_PLANE");
      aCriteria[ SMESH::FT_BelongToCylinder   ] = tr("BELONG_TO_CYLINDER");
      aCriteria[ SMESH::FT_BelongToGenSurface ] = tr("BELONG_TO_GENSURFACE");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
    }
    return aCriteria;
  }
  else // SMESH::ALL
  {
    static QMap<int, QString> aCriteria;
    if (aCriteria.isEmpty())
    {
      aCriteria[ SMESH::FT_RangeOfIds         ] = tr("RANGE_OF_IDS");
      aCriteria[ SMESH::FT_BelongToMeshGroup  ] = tr("BELONG_TO_MESH_GROUP");
      aCriteria[ SMESH::FT_BelongToGeom       ] = tr("BELONG_TO_GEOM");
      aCriteria[ SMESH::FT_LyingOnGeom        ] = tr("LYING_ON_GEOM");
      aCriteria[ SMESH::FT_LinearOrQuadratic  ] = tr("LINEAR");
      aCriteria[ SMESH::FT_GroupColor         ] = tr("GROUP_COLOR");
      aCriteria[ SMESH::FT_ElemGeomType       ] = tr("GEOM_TYPE");
      aCriteria[ SMESH::FT_ConnectedElements  ] = tr("CONNECTED_ELEMS");
    }

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

  if (aMap.isEmpty())
  {
    aMap[ SMESH::FT_LessThan ] = tr("LESS_THAN");
    aMap[ SMESH::FT_MoreThan ] = tr("MORE_THAN");
    aMap[ SMESH::FT_EqualTo  ] = tr("EQUAL_TO" );
  }

  return aMap;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::createTable
// Purpose : Create table
//=======================================================================
SMESHGUI_FilterTable::Table* SMESHGUI_FilterTable::createTable (QWidget*  theParent,
                                                                const int theType)
{
  // create table
  Table* aTable= new Table(0, 6, theParent);

  QHeaderView* aHeaders = aTable->horizontalHeader();

  QFontMetrics aMetrics(aHeaders->font());

  // append spaces to the header of criteria in order to
  // provide visibility of criterion inside comboboxes
  static int aMaxLenCr = 0;

  if (aMaxLenCr == 0)
  {
    const QMap<int, QString>& aSupportedTypes = getSupportedTypes();
    QMap<int, QString>::const_iterator anIter;
    for (anIter = aSupportedTypes.begin(); anIter != aSupportedTypes.end(); ++anIter)
      aMaxLenCr = qMax(maxLength(getCriteria(anIter.key()), aMetrics), aMaxLenCr);
  }

  static int aLenCr = qAbs( aMaxLenCr -
                            aMetrics.width(tr("CRITERION"))) / aMetrics.width(' ') + 5;

  QString aCrStr;
  aCrStr.fill(' ', aLenCr);
  QString aCoStr;
  aCoStr.fill(' ', 10);

  QStringList aHeaderLabels;
  aHeaderLabels.append( tr("CRITERION") + aCrStr );
  aHeaderLabels.append( tr("COMPARE")   + aCoStr );
  aHeaderLabels.append( tr("THRESHOLD_VALUE") );
  aHeaderLabels.append( tr("UNARY") );
  aHeaderLabels.append( tr("BINARY") + "  " );
  aHeaderLabels.append( tr("ID") );
  aTable->setHorizontalHeaderLabels( aHeaderLabels );
  
  // set geometry of the table
  for (int i = 0; i <= 4; i++)
    aTable->resizeColumnToContents(i);

  // set the ID column invisible
  aTable->hideColumn( 5 );

  aTable->updateGeometry();
  QSize aSize = aTable->sizeHint();
  int aWidth = aSize.width();
  aTable->setMinimumSize(QSize(aWidth, aWidth / 2));
  aTable->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

  connect(aTable, SIGNAL(cellChanged(int, int)),
          this,   SLOT(onCriterionChanged(int, int)));

  connect(aTable, SIGNAL(currentCellChanged(int, int, int, int)),
          this,   SLOT(onCurrentChanged(int, int)));
  
  return aTable;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::updateBtnState
// Purpose : Update button state
//=======================================================================
void SMESHGUI_FilterTable::updateBtnState()
{
  myRemoveBtn->setEnabled(myTables[ GetType() ]->rowCount() > 0);
  myClearBtn->setEnabled(myTables[ GetType() ]->rowCount() > 0);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetEditable
// Purpose : Set read only flag for tables. Show/hide buttons for work with rows
//=======================================================================
void SMESHGUI_FilterTable::SetEditable (const bool isEditable)
{
  TableMap::iterator anIter;
  for (anIter = myTables.begin(); anIter != myTables.end(); ++anIter)
  {
    anIter.value()->setReadOnly(!isEditable);

    // Set Flags for CheckItems directly IPAL 19974
    Table* aTable = anIter.value();
    for (int i = 0, n = aTable->rowCount(); i < n; i++)
      for (int j = 0, m = aTable->columnCount(); j < m; j++)
        {
          QTableWidgetItem* anItem = aTable->item(i, j);
          if ( dynamic_cast<SMESHGUI_FilterTable::CheckItem*>( anItem ) ) {
            Qt::ItemFlags f = anItem->flags();
            if (!isEditable) f = f & ~Qt::ItemIsUserCheckable;
            else f = f | Qt::ItemIsUserCheckable;
            anItem->setFlags( f );
          }
        }
    //end of IPAL19974

    if (isEditable)
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

  QMap<QTableWidgetItem*, AdditionalWidget*>::iterator anIter2;
  for (anIter2 = myAddWidgets.begin(); anIter2 != myAddWidgets.end(); ++anIter2)
    anIter2.value()->SetEditable(isEditable);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetEnabled
// Purpose : Enable/Disable table. Switching type of elements already enabled
//=======================================================================
void SMESHGUI_FilterTable::SetEnabled (const bool isEnabled)
{
  myAddBtn->setEnabled(isEnabled);
  myInsertBtn->setEnabled(isEnabled);
  myRemoveBtn->setEnabled(isEnabled);
  myClearBtn->setEnabled(isEnabled);

  if (isEnabled)
    updateBtnState();

  QMap<QTableWidgetItem*, AdditionalWidget*>::iterator anIter2;
  for (anIter2 = myAddWidgets.begin(); anIter2 != myAddWidgets.end(); ++anIter2)
    anIter2.value()->setEnabled(isEnabled);
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
void SMESHGUI_FilterTable::SetLibsEnabled (const bool isEnabled)
{
  if (isEnabled)
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
  if (myLibDlg == 0)
    myLibDlg = new SMESHGUI_FilterLibraryDlg(
      mySMESHGUI, this, GetType(), SMESHGUI_FilterLibraryDlg::COPY_FROM);
  else
    myLibDlg->Init(GetType(), SMESHGUI_FilterLibraryDlg::COPY_FROM);

  if (myLibDlg->exec() == QDialog::Accepted)
  {
    Copy(myLibDlg->GetTable());
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
  if (!IsValid(true))
    return;
  if (myLibDlg == 0)
    myLibDlg = new SMESHGUI_FilterLibraryDlg(
      mySMESHGUI, this, GetType(), SMESHGUI_FilterLibraryDlg::ADD_TO);
  else
    myLibDlg->Init(GetType(), SMESHGUI_FilterLibraryDlg::ADD_TO);

  myLibDlg->SetTable(this);

  myLibDlg->exec();
}

//=======================================================================
// name    : SMESHGUI_FilterTable::Copy
// Purpose : Initialise table with values of other table
//=======================================================================
void SMESHGUI_FilterTable::Copy (const SMESHGUI_FilterTable* theTable)
{
  Clear();

  for (int i = 0, n = theTable->NumRows(); i < n; i++)
  {
    SMESH::Filter::Criterion aCriterion = SMESHGUI_FilterDlg::createCriterion();
    theTable->GetCriterion(i, aCriterion);
    AddCriterion(aCriterion);
  }
}

//=======================================================================
// name    : SMESHGUI_FilterTable::CurrentCell
// Purpose : Returns current cell
//=======================================================================
bool SMESHGUI_FilterTable::CurrentCell (int& theRow, int& theCol) const
{
  theRow = myTables[ GetType() ]->currentRow();
  theCol = myTables[ GetType() ]->currentColumn();
  return theRow >= 0 && theCol >= 0;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetText
// Purpose : Set text and internal value in cell of threshold value
//=======================================================================
void SMESHGUI_FilterTable::SetThreshold (const int      theRow,
                                         const QString& theText,
                                         const int      theEntityType)
{
  Table* aTable = myTables[ theEntityType == -1 ? GetType() : theEntityType ];

  bool isSignalsBlocked = aTable->signalsBlocked();
  aTable->blockSignals(true);
  
  aTable->item( theRow, 2 )->setText(theText);

  aTable->blockSignals(isSignalsBlocked);
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetText
// Purpose : Get text and internal value from cell of threshold value
//=======================================================================
bool SMESHGUI_FilterTable::GetThreshold (const int      theRow,
                                         QString&       theText,
                                         const int      theEntityType)
{
  Table* aTable = myTables[ theEntityType == -1 ? GetType() : theEntityType ];
  QTableWidgetItem* anItem = aTable->item(theRow, 2);
  if (anItem != 0)
  {
    theText = anItem->text();
    return true;
  }
  else
   return false;
}

//=======================================================================
// name    : SMESHGUI_FilterTable::SetID
// Purpose : Set text and internal value in cell of ID value 
//=======================================================================
void SMESHGUI_FilterTable::SetID( const int      theRow,
                                  const QString& theText,
                                  const int      theEntityType )
{
  Table* aTable = myTables[ theEntityType == -1 ? GetType() : theEntityType ];
  aTable->item( theRow, 5 )->setText( theText );
}

//=======================================================================
// name    : SMESHGUI_FilterTable::GetID
// Purpose : Get text and internal value from cell of ID value
//=======================================================================
bool SMESHGUI_FilterTable::GetID( const int      theRow,
                                  QString&       theText,
                                  const int      theEntityType )
{
  Table* aTable = myTables[ theEntityType == -1 ? GetType() : theEntityType ];
  QTableWidgetItem* anItem = aTable->item( theRow, 5 );
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
SMESHGUI_FilterDlg::SMESHGUI_FilterDlg( SMESHGUI*         theModule,
                                        const QList<int>& theTypes )
: QDialog( SMESH::GetDesktop( theModule ) ),
  mySMESHGUI( theModule ),
  mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
  myInitSourceWgOnApply( true ),
  myInsertEnabled( true ),
  myDiffSourcesEnabled( true )
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    mySelector = aViewWindow->GetSelector();

  construct(theTypes);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SMESHGUI_FilterDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_FilterDlg::SMESHGUI_FilterDlg( SMESHGUI*   theModule,
                                        const int   theType )
: QDialog( SMESH::GetDesktop( theModule ) ),
  mySMESHGUI( theModule ),
  mySelectionMgr( SMESH::GetSelectionMgr( theModule ) ),
  myInitSourceWgOnApply( true ),
  myInsertEnabled( true ),
  myDiffSourcesEnabled( true )
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    mySelector = aViewWindow->GetSelector();
  QList<int> aTypes;
  aTypes.append(theType);
  construct(aTypes);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::construct
// Purpose : Construct dialog (called by constructor)
//=======================================================================
void SMESHGUI_FilterDlg::construct (const QList<int>& theTypes)
{
  myTypes = theTypes;

  setModal(false);
  //setAttribute(Qt::WA_DeleteOnClose, true); // VSR ??? is it required?
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  myMainFrame         = createMainFrame  (this);
  QWidget* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(myMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(myMainFrame, 1);

  myHelpFileName = "selection_filter_library_page.html";

  Init(myTypes);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_FilterDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aMainFrame = new QWidget(theParent);
  QVBoxLayout* aMainLay = new QVBoxLayout(aMainFrame);
  aMainLay->setMargin(0);
  aMainLay->setSpacing(SPACING);

  // filter frame

  myTable = new SMESHGUI_FilterTable( mySMESHGUI, aMainFrame, myTypes );
  myTable->SetLibsEnabled(true);

  QGroupBox* aGrp = myTable->GetTableGrp();
  QGridLayout* aLay = qobject_cast<QGridLayout*>( aGrp->layout() );
  int rows = aLay->rowCount();
  int cols = aLay->columnCount();

  // This line looks strange when all additional parameters and mySetInViewer are hidden
  QFrame* aLine = new QFrame(aGrp);
  aLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  aLay->addWidget(aLine, rows++, 0, 1, cols);

  mySetInViewer = new QCheckBox(tr("SET_IN_VIEWER"), aGrp);
  mySetInViewer->setChecked(true);
  aLay->addWidget(mySetInViewer, rows++, 0, 1, cols);

  // other controls
  QWidget* aSourceGrp = createSourceGroup(aMainFrame);

  connect(myTable, SIGNAL(CriterionChanged(const int, const int)),
                    SLOT(onCriterionChanged(const int, const int)));

  connect(myTable, SIGNAL(CurrentChanged(int, int)),
                    SLOT(onCurrentChanged(int, int)));

  aMainLay->addWidget(myTable);
  aMainLay->addWidget(aSourceGrp);
  
  return aMainFrame;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createSourceFrame
// Purpose : Create frame containing source radio button
//=======================================================================
QWidget* SMESHGUI_FilterDlg::createSourceGroup (QWidget* theParent)
{
  QGroupBox* aBox = new QGroupBox(tr("SOURCE"), theParent);
  QHBoxLayout* aLay = new QHBoxLayout(aBox);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  mySourceGrp = new QButtonGroup(theParent);

  QRadioButton* aMeshBtn = new QRadioButton(tr("MESH"),          aBox);
  QRadioButton* aSelBtn  = new QRadioButton(tr("SELECTION"),     aBox);
  QRadioButton* aDlgBtn  = new QRadioButton(tr("CURRENT_DIALOG"),aBox);

  aLay->addWidget(aMeshBtn);
  aLay->addWidget(aSelBtn);
  aLay->addWidget(aDlgBtn);

  mySourceGrp->addButton(aMeshBtn, Mesh);
  mySourceGrp->addButton(aSelBtn,  Selection);
  mySourceGrp->addButton(aDlgBtn,  Dialog);

  aMeshBtn->setChecked(true);

  return aBox;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::updateMainButtons
// Purpose : Update visibility of main buttons (OK, Cancel, Close ...)
//=======================================================================
void SMESHGUI_FilterDlg::updateMainButtons()
{
  myButtons[ BTN_Close  ]->show();
  if (myTypes.count() == 1)
  {
    myButtons[ BTN_Apply  ]->hide();
  }
  else
  {
    myButtons[ BTN_Apply  ]->show();
  }
//  updateGeometry();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QWidget* SMESHGUI_FilterDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aGrp = new QGroupBox(theParent);
  QHBoxLayout* aLay = new QHBoxLayout(aGrp);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  myButtons[ BTN_OK     ] = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aGrp);
  myButtons[ BTN_Apply  ] = new QPushButton(tr("SMESH_BUT_APPLY"),           aGrp);
  myButtons[ BTN_Close  ] = new QPushButton(tr("SMESH_BUT_CLOSE"),           aGrp);
  myButtons[ BTN_Help   ] = new QPushButton(tr("SMESH_BUT_HELP"),            aGrp);

  aLay->addWidget(myButtons[ BTN_OK     ]);
  aLay->addSpacing(10);
  aLay->addWidget(myButtons[ BTN_Apply  ]);
  aLay->addSpacing(10);
  aLay->addStretch();
  aLay->addWidget(myButtons[ BTN_Close  ]);
  aLay->addWidget(myButtons[ BTN_Help   ]);

  connect(myButtons[ BTN_OK     ], SIGNAL(clicked()), SLOT(onOk()));
  connect(myButtons[ BTN_Close  ], SIGNAL(clicked()), SLOT(reject()));
  connect(myButtons[ BTN_Apply  ], SIGNAL(clicked()), SLOT(onApply()));
  connect(myButtons[ BTN_Help   ], SIGNAL(clicked()), SLOT(onHelp()));

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
void SMESHGUI_FilterDlg::Init (const int type, const bool setInViewer)
{
  QList<int> aTypes;
  aTypes.append(type);
  Init(aTypes,setInViewer);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FilterDlg::Init (const QList<int>& theTypes, const bool setInViewer)
{
  if ( theTypes.empty() )
  {
    Init( SMESH::ALL, setInViewer );
    return;
  }
  mySourceWg  = 0;
  myTypes     = theTypes;
  myMesh      = SMESH::SMESH_Mesh::_nil();
  myGroup     = SMESH::SMESH_GroupOnFilter::_nil();
  myIObjects.Clear();
  myIsSelectionChanged = false;
  myToRestoreSelMode = false;

  myTable->Init(theTypes);

  // set caption
  if (theTypes.count() == 1)
  {
    int aType = theTypes.first();
    if      (aType == SMESH::NODE  ) setWindowTitle(tr("NODES_TLT"));
    else if (aType == SMESH::ELEM0D) setWindowTitle(tr("ELEM0D_TLT"));
    else if (aType == SMESH::BALL  ) setWindowTitle(tr("BALL_TLT"));
    else if (aType == SMESH::EDGE  ) setWindowTitle(tr("EDGES_TLT"));
    else if (aType == SMESH::FACE  ) setWindowTitle(tr("FACES_TLT"));
    else if (aType == SMESH::VOLUME) setWindowTitle(tr("VOLUMES_TLT"));
    else if (aType == SMESH::ALL)    setWindowTitle(tr("TLT"));
  }
  else
    setWindowTitle(tr("TLT"));

  qApp->processEvents();
  updateGeometry();
  adjustSize();
  setEnabled(true);

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()), SLOT(onCloseView()));
  
  updateMainButtons();
  updateSelection();

  // Initialise filter table with values of previous filter
  QList<int>::const_iterator anIter;
  for (anIter = theTypes.begin(); anIter != theTypes.end(); ++anIter)
  {
    myTable->Clear(*anIter);
    if (!myFilter[ *anIter ]->_is_nil())
    {
      SMESH::Filter::Criteria_var aCriteria = new SMESH::Filter::Criteria;
      if (myFilter[ *anIter ]->GetCriteria(aCriteria))
      {
        for (int i = 0, n = aCriteria->length(); i < n; i++)
          myTable->AddCriterion(aCriteria[ i ], *anIter);
      }
    }
  }

  if (myInsertState.contains(theTypes.first()))
    mySetInViewer->setChecked(myInsertState[ theTypes.first() ]);
  else
    mySetInViewer->setChecked(setInViewer);

  mySourceGrp->button(myApplyToState.contains(theTypes.first()) ? 
                      myApplyToState[ theTypes.first() ] :
                      Mesh)->setChecked(true);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onOk
// Purpose : SLOT called when "Ok" button pressed.
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_FilterDlg::onOk()
{
  if (onApply())
  {
    restoreSelMode();
    mySelectionMgr->clearFilters();
    disconnect(mySMESHGUI, 0, this, 0);
    disconnect(mySelectionMgr, 0, this, 0);
    mySMESHGUI->ResetState();
    accept();
    emit Accepted();
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::reject
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_FilterDlg::reject()
{
  restoreSelMode();
  // Restore previously selected object
  if (mySelectionMgr)
  {
    SALOME_ListIO aList;
    mySelectionMgr->clearFilters();
    mySelectionMgr->clearSelected();
    SALOME_DataMapIteratorOfDataMapOfIOMapOfInteger anIter (myIObjects);
    for ( ; anIter.More(); anIter.Next())
    {
      aList.Append(anIter.Key());

      TColStd_MapOfInteger aResMap;
      const TColStd_IndexedMapOfInteger& anIndMap = anIter.Value();
      for (int i = 1, n = anIndMap.Extent(); i <= n; i++)
        aResMap.Add(anIndMap(i));

      mySelector->AddOrRemoveIndex( anIter.Key(), aResMap, false);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
        aViewWindow->highlight( anIter.Key(), true, true );
    }
    mySelectionMgr->setSelectedObjects(aList, false);
  }

  disconnect(mySMESHGUI, 0, this, 0);
  disconnect(mySelectionMgr, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_FilterDlg::onOpenView()
{
  if ( mySelector ) {
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_FilterDlg::onCloseView()
{
  mySelector = 0;
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_FilterDlg::onHelp()
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
// name    : SMESHGUI_FilterDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_FilterDlg::onDeactivate()
{
  setEnabled(false);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_FilterDlg::enterEvent (QEvent*)
{
//  mySMESHGUI->EmitSignalDeactivateDialog();
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  mySMESHGUI->ResetState();
  setEnabled(true);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getIdsFromWg
// Purpose : Retrieve list of ids from given widget
//=======================================================================
void SMESHGUI_FilterDlg::getIdsFromWg (const QWidget* theWg, QList<int>& theRes) const
{
  theRes.clear();
  if (theWg == 0)
    return;

  if (theWg->inherits("QListWidget"))
  {
    const QListWidget* aListBox = qobject_cast<const QListWidget*>( theWg );
    bool b;
    for (int i = 0, n = aListBox->count(); i < n; i++)
    {
      int anId = aListBox->item(i)->text().toInt(&b);
      if (b)
        theRes.append(anId);
    }
  }
  else if (theWg->inherits("QLineEdit"))
  {
    const QLineEdit* aLineEdit = qobject_cast<const QLineEdit*>( theWg );
    QString aStr = aLineEdit->text();
    QRegExp aRegExp("(\\d+)");
    bool b;
    int aPos = 0;
    while (aPos >= 0)
    {
      aPos = aRegExp.indexIn(aStr, aPos);
      if (aPos > -1)
      {
        int anId = aRegExp.cap(1).toInt(&b);
        if (b)
          theRes.append(anId);
        aPos += aRegExp.matchedLength();
      }
    }
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::getSelMode
// Purpose : Get selection mode of specified type
//=======================================================================
Selection_Mode SMESHGUI_FilterDlg::getSelMode (const int theType) const
{
  switch (theType)
  {
    case SMESH::NODE   : return NodeSelection;
    case SMESH::ELEM0D : return Elem0DSelection;
    case SMESH::BALL   : return BallSelection;
    case SMESH::EDGE   : return EdgeSelection;
    case SMESH::FACE   : return FaceSelection;
    case SMESH::VOLUME : return VolumeSelection;
    case SMESH::ALL    : return CellSelection;
    default            : return ActorSelection;
  }

}

//=======================================================================
// name    : SMESHGUI_FilterDlg::setIdsToWg
// Purpose : Insert identifiers in specified widgets
//=======================================================================
void SMESHGUI_FilterDlg::setIdsToWg (QWidget* theWg, const QList<int>& theIds)
{
  if (theWg == 0)
    return;

  QStringList aStrList;
  foreach(int id, theIds)
    aStrList << QString::number(id);

  if (theWg->inherits("QListWidget"))
  {
    qobject_cast<QListWidget*>(theWg)->clear();
    qobject_cast<QListWidget*>(theWg)->addItems(aStrList);
  }
  else if (theWg->inherits("QLineEdit"))
  {
    qobject_cast<QLineEdit*>( theWg )->setText(aStrList.join(" "));
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_FilterDlg::isValid() const
{
  if (!myTable->IsValid())
    return false;

  for (int i = 0, n = myTable->NumRows(); i < n; i++)
  {
    int aType = myTable->GetCriterionType(i);
    if (aType == SMESH::FT_BelongToGeom ||
        aType == SMESH::FT_BelongToPlane ||
        aType == SMESH::FT_BelongToCylinder ||
        aType == SMESH::FT_BelongToGenSurface ||
        aType == SMESH::FT_LyingOnGeom)
    {
      QString aName;
      myTable->GetThreshold(i, aName);

      std::vector<_PTR(SObject)> aList =
        SMESH::GetActiveStudyDocument()->FindObjectByName(aName.toLatin1().constData(), "GEOM");
      if (aList.size() == 0) {
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                     tr("BAD_SHAPE_NAME").arg(aName));
        return false;
      }

      if (aType == SMESH::FT_BelongToCylinder ||
          aType == SMESH::FT_BelongToPlane    ||
          aType == SMESH::FT_BelongToGenSurface ) {
        CORBA::Object_var     anObject = SMESH::SObjectToObject(aList[ 0 ]);
        GEOM::GEOM_Object_var aGeomObj = GEOM::GEOM_Object::_narrow(anObject);
        if (!aGeomObj->_is_nil()) {
          TopoDS_Shape aFace;
          if (!GEOMBase::GetShape(aGeomObj, aFace) ||
               aFace.IsNull() ||
               aFace.ShapeType() != TopAbs_FACE) {
            SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                         tr("SHAPE_IS_NOT_A_FACE").arg(aName));
            return false;
          }

          Handle(Geom_Surface) aSurf = BRep_Tool::Surface(TopoDS::Face(aFace));
          if (aSurf.IsNull()) {
            SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                         tr("SHAPE_IS_NOT_A_FACE").arg(aName));
            return false;
          }

          if (aType == SMESH::FT_BelongToPlane && !aSurf->IsKind(STANDARD_TYPE(Geom_Plane))) {
            SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                         tr("SHAPE_IS_NOT_A_PLANE").arg(aName));
            return false;
          }

          if (aType == SMESH::FT_BelongToCylinder && !aSurf->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
            SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                         tr("SHAPE_IS_NOT_A_CYLINDER").arg(aName));
            return false;
          }
        }
      }
    }
    else if (aType == SMESH::FT_CoplanarFaces)
    {
      QString faceID;
      myTable->GetThreshold(i, faceID);
      if ( faceID.isEmpty() )
      {
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                     tr("FACE_ID_NOT_SELECTED"));
        return false;
      }
      if ( myMesh->_is_nil() )
      {
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                     tr("MESH_IS_NOT_SELECTED"));
        return false;
      }
      if ( myMesh->GetElementType( faceID.toLong(), /*iselem=*/true) != SMESH::FACE )
      {
        SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INSUFFICIENT_DATA"),
                                     tr("NOT_FACE_ID").arg(faceID));
        return false;
      }
    }
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetSourceWg
// Purpose : Set widget of parent dialog containing ids to be filtered if
//           user select corresponding source radio button
//=======================================================================
void SMESHGUI_FilterDlg::SetSourceWg (QWidget* theWg,
                                      const bool initOnApply)
{
  mySourceWg = theWg;
  myInitSourceWgOnApply = initOnApply;
}

//=======================================================================
//function : EnableFiltering
//purpose  : Enables "Insert filter in the viewer"
//           and different "Source"s (Mesh, Initial Selection, Current Group)
//=======================================================================

void SMESHGUI_FilterDlg::SetEnabled( bool setInViewer, bool diffSources )
{
  myInsertEnabled = setInViewer;
  myDiffSourcesEnabled = diffSources;

  mySetInViewer->setVisible( myInsertEnabled );
  mySourceGrp->button(0)->parentWidget()->setVisible( myDiffSourcesEnabled );
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetMesh
// Purpose : Set mesh
//=======================================================================
void SMESHGUI_FilterDlg::SetMesh (SMESH::SMESH_Mesh_var theMesh)
{
  if ( !theMesh->_is_nil() ) {
    myMesh = theMesh;
    if ( !myFilter[ myTable->GetType() ]->_is_nil())
      myFilter[ myTable->GetType() ]->SetMesh( theMesh );
  }
  const bool isEnable = !(myMesh->_is_nil());
  myButtons[BTN_OK]->setEnabled(isEnable);
  myButtons[BTN_Apply]->setEnabled(isEnable);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetGroup
// Purpose : Set a group being edited
//=======================================================================
void SMESHGUI_FilterDlg::SetGroup(SMESH::SMESH_GroupOnFilter_var group)
{
  myGroup = group;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::SetSelection
// Purpose : Get filtered ids
//=======================================================================
void SMESHGUI_FilterDlg::SetSelection()
{
  if (mySelectionMgr)
    disconnect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(onSelectionDone()));

  if (mySelectionMgr) {
    myIObjects.Clear();
    const SALOME_ListIO& anObjs = mySelector->StoredIObjects(); 
    SALOME_ListIteratorOfListIO anIter (anObjs);
    for ( ; anIter.More(); anIter.Next()) {
      TColStd_IndexedMapOfInteger aMap;
      mySelector->GetIndex(anIter.Value(), aMap);
      myIObjects.Bind(anIter.Value(), aMap);
    }

    connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));

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
  if (!isValid())
    return false;

  SUIT_OverrideCursor wc;
  try {
    int aCurrType = myTable->GetType();

    if (!createFilter(aCurrType))
      return false;

    insertFilterInViewer();

    if (!myFilter[ aCurrType ]->GetPredicate()->_is_nil()) {
      //
      bool toFilter = (( getActor() ) ||
                       ( myInitSourceWgOnApply && mySourceWg ) ||
                       ( mySourceGrp->checkedId() == Dialog && mySourceWg ));
      if ( toFilter ) {
        QList<int> aResultIds;
        filterSource(aCurrType, aResultIds);
        // select in viewer
        selectInViewer(aCurrType, aResultIds);
        // set ids to the dialog
        if ( myInitSourceWgOnApply || mySourceGrp->checkedId() == Dialog )
          setIdsToWg(mySourceWg, aResultIds);
      }
    }

    myInsertState[ aCurrType ] = mySetInViewer->isChecked();
    myApplyToState[ aCurrType ] = mySourceGrp->checkedId();
  }
  catch(const SALOME::SALOME_Exception& S_ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
    return false;
  }
  catch(...)
  {
    return false;
  }

  return true;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createFilter
// Purpose : Create predicate for given type
//=======================================================================
bool SMESHGUI_FilterDlg::createFilter (const int theType)
{
  SMESH::FilterManager_var aFilterMgr = SMESH::GetFilterManager();
  if (aFilterMgr->_is_nil())
    return false;

  int n = myTable->NumRows();

  SMESH::Filter::Criteria_var aCriteria = new SMESH::Filter::Criteria;
  aCriteria->length(n);

  long aPrecision = -1;
  SUIT_ResourceMgr* mgr = SMESH::GetResourceMgr( mySMESHGUI );

  if ( mgr && mgr->booleanValue( "SMESH", "use_precision", false ) )
    aPrecision = mgr->integerValue( "SMESH", "controls_precision", aPrecision );

  for ( int i = 0; i < n; i++) {
    SMESH::Filter::Criterion aCriterion = createCriterion();
    myTable->GetCriterion(i, aCriterion);
    aCriterion.Precision = aPrecision;
    aCriteria[ i ] = aCriterion;
  }

  if ( !myFilter[ theType ]->_is_nil() )
    myFilter[ theType ]->UnRegister();
  myFilter[ theType ] = aFilterMgr->CreateFilter();
  myFilter[ theType ]->SetCriteria(aCriteria.inout());

  return true;
}

//================================================================================
/*!
 * \brief Return the current filter
 */
//================================================================================

SMESH::Filter_var SMESHGUI_FilterDlg::GetFilter() const
{
  SMESH::Filter_var filter;
  try {
    int aCurrType = myTable->GetType();
    filter = myFilter[ aCurrType ];
  }
  catch(...)
  {
  }
  return filter._retn();
}

//================================================================================
/*!
 * \brief Sets a filter to the table
 */
//================================================================================

void SMESHGUI_FilterDlg::SetFilter(SMESH::Filter_var filter, int type)
{
  if ( !filter->_is_nil() )
    filter->Register();
  if ( !myFilter[ type ]->_is_nil() )
    myFilter[ type ]->UnRegister();

  myFilter[ type ] = filter;
}

//================================================================================
/*!
 * \brief call UnRegister() for myFilter's
 */
//================================================================================

void SMESHGUI_FilterDlg::UnRegisterFilters()
{
  QMap< int, SMESH::Filter_var >::iterator i_f = myFilter.begin();
  for ( ; i_f != myFilter.end(); ++i_f )
    if ( !i_f.value()->_is_nil() )
      i_f.value()->UnRegister();
  myFilter.clear();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::insertFilterInViewer
// Purpose : Insert filter in viewer
//=======================================================================
void SMESHGUI_FilterDlg::insertFilterInViewer()
{
  if (SVTK_Selector* aSelector = SMESH::GetSelector())
  {
    SMESH::ElementType anEntType = (SMESH::ElementType)myTable->GetType();

    if (myFilter[ myTable->GetType() ]->_is_nil() ||
        myFilter[ myTable->GetType() ]->GetPredicate()->_is_nil() ||
        !mySetInViewer->isChecked() ||
        !myInsertEnabled )
    {
      SMESH::RemoveFilter(getFilterId(anEntType), aSelector);
    }
    else
    {
      Handle(SMESHGUI_PredicateFilter) aFilter = new SMESHGUI_PredicateFilter();
      aFilter->SetPredicate(myFilter[ myTable->GetType() ]->GetPredicate());
      SMESH::RemoveFilter(getFilterId(anEntType), aSelector); //skl for IPAL12631
      SMESH::SetFilter(aFilter, aSelector);
    }
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::filterSource
// Purpose : Filter source ids
//=======================================================================
void SMESHGUI_FilterDlg::filterSource (const int theType,
                                       QList<int>& theResIds)
{
  theResIds.clear();
  if (myFilter[ theType ]->_is_nil())
    return;

  int aSourceId = mySourceGrp->checkedId();

  if (aSourceId == Mesh || !myDiffSourcesEnabled )
  {
    if (myMesh->_is_nil())
      return;
    SMESH::long_array_var anIds = myFilter[ theType ]->GetElementsId(myMesh);
    for (int i = 0, n = anIds->length(); i < n; i++)
      theResIds.append(anIds[ i ]);
  }
  else if (aSourceId == Selection)
  {
    filterSelectionSource(theType, theResIds);
  }
  else if (aSourceId == Dialog)
  {
    // retrieve ids from dialog
    QList<int> aDialogIds;
    getIdsFromWg(mySourceWg, aDialogIds);

    if (myMesh->_is_nil())
    {
      theResIds = aDialogIds;
      return;
    }

    // filter ids
    SMESH::Predicate_ptr aPred = myFilter[ theType ]->GetPredicate();
    myFilter[ theType ]->SetMesh(myMesh);
    QList<int>::const_iterator anIter;
    for (anIter = aDialogIds.begin(); anIter != aDialogIds.end(); ++ anIter)
      if (aPred->IsSatisfy(*anIter))
        theResIds.append(*anIter);
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::filterSelectionSource
// Purpose : Filter source selection
//=======================================================================
void SMESHGUI_FilterDlg::filterSelectionSource (const int theType,
                                                QList<int>& theResIds)
{
  theResIds.clear();
  if (myMesh->_is_nil() || mySelectionMgr == 0)
    return;

  // Create map of entities to be filtered
  TColStd_MapOfInteger aToBeFiltered;
  SALOME_DataMapIteratorOfDataMapOfIOMapOfInteger anIter(myIObjects);

  for ( ; anIter.More(); anIter.Next())
  {
    // process sub mesh
    SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIter.Key());
    if (!aSubMesh->_is_nil())
    {
      if (aSubMesh->GetFather()->GetId() == myMesh->GetId())
      {
        SMESH::long_array_var anIds =
          theType == SMESH::NODE ? aSubMesh->GetNodesId() : aSubMesh->GetElementsId();
        for (int i = 0, n = anIds->length(); i < n; i++)
          aToBeFiltered.Add(anIds[ i ]);
      }
    }

    // process group
    SMESH::SMESH_GroupBase_var aGroup =
      SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIter.Key());
    if (!aGroup->_is_nil())
    {
      if (aGroup->GetType() == theType && aGroup->GetMesh()->GetId() == myMesh->GetId())
      {
        SMESH::long_array_var anIds = aGroup->GetListOfID();
        for (int i = 0, n = anIds->length(); i < n; i++)
          aToBeFiltered.Add(anIds[ i ]);
      }
    }

    // process mesh
    SMESH::SMESH_Mesh_var aMeshPtr = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIter.Key());
    if (!aMeshPtr->_is_nil() && aMeshPtr->GetId() == myMesh->GetId())
    {
      const TColStd_IndexedMapOfInteger& aSelMap = anIter.Value();

      if (aSelMap.Extent() > 0)
      {
        if(SMESH::FindActorByEntry(anIter.Key()->getEntry()))
        {
          for (int i = 1; i <= aSelMap.Extent(); i++)
            aToBeFiltered.Add(aSelMap(i));
        }
      }
    }
  }

  // Filter entities
  SMESH::Predicate_ptr aPred = myFilter[ theType ]->GetPredicate();
  myFilter[ theType ]->SetMesh(myMesh);
  TColStd_MapIteratorOfMapOfInteger aResIter(aToBeFiltered);
  for ( ; aResIter.More(); aResIter.Next())
    if (aPred->IsSatisfy(aResIter.Key()))
      theResIds.append(aResIter.Key());
}

//=======================================================================
//function : getActor
//purpose  : Returns an actor to show filtered entities
//=======================================================================

SMESH_Actor* SMESHGUI_FilterDlg::getActor()
{
  SMESH_Actor* meshActor = SMESH::FindActorByObject( myMesh );
  if ( meshActor && meshActor->GetVisibility() )
    return meshActor;

  SALOME_DataMapIteratorOfDataMapOfIOMapOfInteger anIter(myIObjects);
  for ( ; anIter.More(); anIter.Next())
  {
    Handle(SALOME_InteractiveObject) io = anIter.Key();
    if ( io->hasEntry() )
    {
      SMESH_Actor* actor = SMESH::FindActorByEntry( io->getEntry() );
      if ( !actor )
        continue;
      if ( actor->GetVisibility() )
        return actor;
      if ( !meshActor )
        meshActor = actor;
    }
  }
  return meshActor;
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::selectInViewer
// Purpose : Select given entities in viewer
//=======================================================================
void SMESHGUI_FilterDlg::selectInViewer (const int theType, const QList<int>& theIds)
{
  if (mySelectionMgr == 0 || myMesh->_is_nil() )
    return;

  mySelectionMgr->clearFilters();

  // Set new selection mode if necessary
  Selection_Mode aSelMode = getSelMode(theType);
  SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
  if ( aViewWindow && aViewWindow->SelectionMode() != aSelMode )
  {
    mySelectionMgr->clearSelected();
    mySelectionMgr->clearFilters();
    SMESH::SetPointRepresentation( aSelMode == NodeSelection );
    aViewWindow->SetSelectionMode(aSelMode);
  }

  // Clear selection
  SMESH_Actor* anActor = getActor();
  if ( !anActor || !anActor->hasIO() )
    return;

  Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
  SALOME_ListIO aList;
  aList.Append(anIO);
  mySelectionMgr->setSelectedObjects(aList, false);

  // Remove filter corresponding to the current type from viewer
  int aType = myTable->GetType();
  int aFilterId = SMESH::UnknownFilter;
  if      (aType == SMESH::EDGE  ) aFilterId = SMESH::EdgeFilter;
  else if (aType == SMESH::FACE  ) aFilterId = SMESH::FaceFilter;
  else if (aType == SMESH::VOLUME) aFilterId = SMESH::VolumeFilter;
  Handle(VTKViewer_Filter) aFilter = SMESH::GetFilter(aFilterId);
  SMESH::RemoveFilter(aFilterId);

  // get vtk ids
  TColStd_MapOfInteger aMap;
  QList<int>::const_iterator anIter;
  for (anIter = theIds.begin(); anIter != theIds.end(); ++anIter) {
    aMap.Add(*anIter);
  }

  // Set new selection
  mySelector->AddOrRemoveIndex(anIO, aMap, false);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->highlight( anIO, true, true );

  // insert previously stored filter in viewer if necessary
  if (!aFilter.IsNull())
    SMESH::SetFilter(aFilter);
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::createCriterion
// Purpose : Create criterion structure with default values
//=======================================================================
SMESH::Filter::Criterion SMESHGUI_FilterDlg::createCriterion()
{
   SMESH::Filter::Criterion aCriterion;

  aCriterion.Type          = SMESH::FT_Undefined;
  aCriterion.Compare       = SMESH::FT_Undefined;
  aCriterion.Threshold     = 0;
  aCriterion.UnaryOp       = SMESH::FT_Undefined;
  aCriterion.BinaryOp      = SMESH::FT_Undefined;
  aCriterion.ThresholdStr  = "";
  aCriterion.ThresholdID   = "";
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
  const SALOME_ListIO& aList = mySelector->StoredIObjects();
  if ( myMesh->_is_nil() && aList.Extent()>0 )
  {
    myMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(aList.First());
    if ( !(myMesh->_is_nil()) ) {
      myButtons[BTN_OK]->setEnabled(true);
      myButtons[BTN_Apply]->setEnabled(true);
    }
  }
  int aRow, aCol;
  if (aList.Extent() != 1 || !myTable->CurrentCell(aRow, aCol))
    return;

  const int type = myTable->GetCriterionType(aRow);
  QList<int> types; 
  types << SMESH::FT_BelongToGeom      << SMESH::FT_BelongToPlane 
        << SMESH::FT_BelongToCylinder  << SMESH::FT_BelongToGenSurface
        << SMESH::FT_LyingOnGeom       << SMESH::FT_CoplanarFaces
        << SMESH::FT_ConnectedElements << SMESH::FT_BelongToMeshGroup;
  if ( !types.contains( type ))
    return;

  Handle(SALOME_InteractiveObject) anIO = aList.First();
  switch ( type )
  {
  case SMESH::FT_CoplanarFaces: // get ID of a selected mesh face
    {
      QString aString;
      int nbElems = SMESH::GetNameOfSelectedElements(mySelector, anIO, aString);
      if (nbElems == 1)
        myTable->SetThreshold(aRow, aString);
      break;
    }
  case SMESH::FT_ConnectedElements: // get either VERTEX or a node ID
    {
      GEOM::GEOM_Object_var anObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
      if (!anObj->_is_nil())
      {
        myTable->SetThreshold(aRow, GEOMBase::GetName(anObj));
        myTable->SetID       (aRow, anIO->getEntry());
      }
      else
      {
        QString aString;
        int nbElems = SMESH::GetNameOfSelectedElements(mySelector, anIO, aString);
        if (nbElems == 1)
          myTable->SetThreshold(aRow, aString);
      }
      break;
    }
  case SMESH::FT_BelongToMeshGroup: // get a group Name and Entry
    {
      SMESH::SMESH_GroupBase_var grp = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(anIO);
      if ( !grp->_is_nil() )
      {
        if ( !myMesh->_is_nil() )
        {
          SMESH::SMESH_Mesh_var mesh = grp->GetMesh();
          if ( ! myMesh->_is_equivalent( mesh ))
            return;
        }
        if ( !myGroup->_is_nil() && myGroup->IsInDependency( grp ))
          return; // avoid cyclic dependencies between Groups on Filter

        myTable->SetThreshold(aRow, SMESH::toQStr( grp->GetName() ));
        myTable->SetID       (aRow, anIO->getEntry() );
      }
    }
  default: // get a GEOM object
    {
      GEOM::GEOM_Object_var anObj = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
      if (!anObj->_is_nil())
      {
        myTable->SetThreshold(aRow, GEOMBase::GetName(anObj));
        myTable->SetID       (aRow, anIO->getEntry());
      }
    }
  }
}


//=======================================================================
// name    : SMESHGUI_FilterDlg::onCriterionChanged
// Purpose : SLOT called when cretarion of current row changed. Update selection
//=======================================================================
void SMESHGUI_FilterDlg::onCriterionChanged (const int, const int)
{
  updateSelection();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onThresholdChanged
// Purpose : SLOT called when a threshold value is changed by the user and
//           not by myTable->SetThreshold()
//=======================================================================
void SMESHGUI_FilterDlg::onThresholdChanged( const int row, const int type )
{
  if ( myTable->GetCriterionType( row, type ) == SMESH::FT_ConnectedElements )
  {
    // to differ the text entered by the user from that got from selection
    myTable->SetID( row, "", type );
  }
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::onCurrentChanged
// Purpose : SLOT called when current row changed. Update selection
//=======================================================================
void SMESHGUI_FilterDlg::onCurrentChanged (int, int)
{
  updateSelection();
}

//=======================================================================
// name    : SMESHGUI_FilterDlg::updateSelection
// Purpose : UpdateSelection in accordance with current row
//=======================================================================
void SMESHGUI_FilterDlg::updateSelection()
{
  if (mySelectionMgr == 0)
    return;

//   TColStd_MapOfInteger allTypes;
//   for( int i=0; i<10; i++ )
//     allTypes.Add( i );
  SalomeApp_Study* aStudy = dynamic_cast<SalomeApp_Study*>( mySMESHGUI->application()->activeStudy() );
  if( !aStudy )
    return;

  restoreSelMode();

  mySelectionMgr->clearFilters();

  int aRow, aCol;
  
  bool isCurrentCell = myTable->CurrentCell(aRow, aCol);
  int aCriterionType = myTable->GetCriterionType(aRow);
  if ( isCurrentCell &&
      (aCriterionType == SMESH::FT_BelongToGeom ||
       aCriterionType == SMESH::FT_BelongToPlane ||
       aCriterionType == SMESH::FT_BelongToCylinder ||
       aCriterionType == SMESH::FT_BelongToGenSurface ||
       aCriterionType == SMESH::FT_LyingOnGeom))
  {
    if (aCriterionType == SMESH::FT_BelongToGeom ||
        aCriterionType == SMESH::FT_BelongToGenSurface ||
        aCriterionType == SMESH::FT_LyingOnGeom) {

      mySelectionMgr->installFilter(new GEOM_SelectionFilter( aStudy, true ));

    } else if (aCriterionType == SMESH::FT_BelongToPlane) {
      mySelectionMgr->installFilter(new GEOM_FaceFilter( aStudy, StdSelect_Plane ) );

    } else if (aCriterionType == SMESH::FT_BelongToCylinder) {
      mySelectionMgr->installFilter(new GEOM_FaceFilter( aStudy, StdSelect_Cylinder ) );
    }
    myIsSelectionChanged = true;

  }
  else if ( aCriterionType == SMESH::FT_BelongToMeshGroup )
  {
    SMESH_TypeFilter* typeFilter = 0;
    switch ( myTable->GetType() )
    {
    case SMESH::NODE   : typeFilter = new SMESH_TypeFilter( SMESH::GROUP_NODE   ); break;
    case SMESH::ELEM0D : typeFilter = new SMESH_TypeFilter( SMESH::GROUP_0D     ); break;
    case SMESH::BALL   : typeFilter = new SMESH_TypeFilter( SMESH::GROUP_BALL   ); break;
    case SMESH::EDGE   : typeFilter = new SMESH_TypeFilter( SMESH::GROUP_EDGE   ); break;
    case SMESH::FACE   : typeFilter = new SMESH_TypeFilter( SMESH::GROUP_FACE   ); break;
    case SMESH::VOLUME : typeFilter = new SMESH_TypeFilter( SMESH::GROUP_VOLUME ); break;
    case SMESH::ALL    : typeFilter = new SMESH_TypeFilter( SMESH::GROUP        ); break;
    default            : typeFilter = 0;
    }
    if ( typeFilter )
      mySelectionMgr->installFilter( typeFilter );
  }
  else if ( aCriterionType == SMESH::FT_ConnectedElements )
  {
    QList<SUIT_SelectionFilter*> fList;
    fList.push_back( new SMESH_NumberFilter( "GEOM", TopAbs_SHAPE, 1, TopAbs_VERTEX ));
    fList.push_back( new SMESH_TypeFilter( SMESH::IDSOURCE ));
    mySelectionMgr->installFilter
      ( new SMESH_LogicalFilter( fList, SMESH_LogicalFilter::LO_OR,
                                 !mySelectionMgr->autoDeleteFilter() ));

    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    {
      mySelModeToRestore = aViewWindow->SelectionMode();
      aViewWindow->SetSelectionMode( NodeSelection );
      myToRestoreSelMode = ( mySelModeToRestore != NodeSelection );
      if ( myToRestoreSelMode )
        SMESH::SetPointRepresentation( true );
    }
  }
  else
  {
    mySelector->SetSelectionMode( getSelMode( myTable->GetType() ));

    if (myIsSelectionChanged) {
      // mySelectionMgr->installFilter( new GEOM_TypeFilter( aStudy, -1 ) ); // This filter deactivates selection
      // Impossible to select any object in the OB on the second opening of FilterDlg
    }
  }
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_FilterDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}

//================================================================================
/*!
 * \brief Restores a selection mode if it was changed to set up some criterion
 */
//================================================================================

void SMESHGUI_FilterDlg::restoreSelMode()
{
  if ( myToRestoreSelMode )
  {
    SMESH::SetPointRepresentation( mySelModeToRestore == NodeSelection );

    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
      aViewWindow->SetSelectionMode( mySelModeToRestore );

    myToRestoreSelMode = false;
  }
}
