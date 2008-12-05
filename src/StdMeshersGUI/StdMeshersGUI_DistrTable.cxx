//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File   : StdMeshersGUI_DistrTable.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "StdMeshersGUI_DistrTable.h"

// Qt incldues
#include <QItemDelegate>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#define SPACING 6

/*!
  \brief Sort list of points by ascending order.
  \internal
*/
static void sortData( QList<double>& d )
{
  typedef QPair<double, double> Pair;
  QList<Pair> pairs;
  for ( int i = 0; i < d.count() / 2; i++ )
    pairs.append( Pair( d[i*2], d[i*2+1] ) );
  
  qSort( pairs );

  d.clear();

  Pair p;
  double prevX = 0.0, prevY = 0.0;

  d.append( prevX );
  d.append( pairs.count() > 0 ? pairs[0].second : prevY );

  foreach( p, pairs ) {
    if ( p.first > prevX ) {
      d.append( p.first  );
      d.append( p.second );
      prevY = p.second;
    }
    prevX = p.first;
  }

  if ( prevX < 1.0 ) {
    d.append( 1.0 );
    d.append( prevY );
  }
}

/*!
  \class StdMeshersGUI_DistrTableFrame::SpinBoxDelegate
  \brief Custom item delegate (uses double spin box to edit table item)
  \internal
*/

class StdMeshersGUI_DistrTableFrame::SpinBoxDelegate : public QItemDelegate
{
public:
  SpinBoxDelegate( StdMeshersGUI_DistrTableFrame::Table* );
  ~SpinBoxDelegate();

  QWidget* createEditor( QWidget*,
			 const QStyleOptionViewItem&,
			 const QModelIndex& ) const;
  void     setEditorData( QWidget*, const QModelIndex&) const;
  void     setModelData( QWidget*, QAbstractItemModel*, 
			 const QModelIndex& ) const;
  void     updateEditorGeometry( QWidget*,
				 const QStyleOptionViewItem&, 
				 const QModelIndex& ) const;

private:
  StdMeshersGUI_DistrTableFrame::Table* myTable;
};

/*!
  \class StdMeshersGUI_DistrTableFrame::Table
  \brief Table function widget
  \internal
*/

class StdMeshersGUI_DistrTableFrame::Table : public QTableWidget
{
private:
  struct EditorData
  { 
    int r, c;
    QDoubleSpinBox* sb;
    EditorData() { reset(); }
    void reset() { r = -1; c = -1; sb = 0; }
  };

public:
  Table( QWidget*, int = 2 );
  ~Table();

  QList<double> data();
  void          setData( const QList<double>& );

  double        value( int, int ) const;
  void          setValue( int, int, double );

  double        argMinimum( int ) const;
  double        argMaximum( int ) const;
  double        argStep( int ) const;
  double        funcMinimum( int ) const;
  double        funcMaximum( int ) const;
  double        funcStep( int ) const;

  void          setFuncMinValue( double );

  QSize         sizeHint() const;

  void          addRow();
  void          deleteRow();

  void          setEditor( int, int, QDoubleSpinBox* );

protected:
  void          closeEditor( QWidget*, QAbstractItemDelegate::EndEditHint );

private:
  void          setUpRows( bool = false );
  QSize         cachedSizeHint() const;
  void          setCachedSizeHint( const QSize& ) const;
  QList<int>    selectedRows();

private:
  double        myFuncMin;
  QSize         myCachedSizeHint;
  EditorData    myEditorData;
};

// ---
// StdMeshersGUI_DistrTableFrame::SpinBoxDelegate implementation
// ---

StdMeshersGUI_DistrTableFrame::SpinBoxDelegate::
SpinBoxDelegate( StdMeshersGUI_DistrTableFrame::Table* parent )
  : QItemDelegate( parent ), myTable( parent )
{
}

StdMeshersGUI_DistrTableFrame::SpinBoxDelegate::
~SpinBoxDelegate()
{
}

QWidget* 
StdMeshersGUI_DistrTableFrame::SpinBoxDelegate::
createEditor( QWidget* parent,
	      const QStyleOptionViewItem& /*option*/,
	      const QModelIndex& index ) const
{
  QDoubleSpinBox* sb = new QDoubleSpinBox( parent );
  sb->setFrame(false);
  sb->setMinimum( index.column() == StdMeshersGUI_DistrTableFrame::ArgColumn ? 
		  myTable->argMinimum( index.row() ) : 
		  myTable->funcMinimum( index.row() ) );
  sb->setMaximum( index.column() == StdMeshersGUI_DistrTableFrame::ArgColumn ? 
		  myTable->argMaximum( index.row() ) : 
		  myTable->funcMaximum( index.row() ) );
  sb->setSingleStep( index.column() == StdMeshersGUI_DistrTableFrame::ArgColumn ? 
		     myTable->argStep( index.row() ) : 
		     myTable->funcStep( index.row() ) );
  myTable->setEditor( index.row(), index.column(), sb );
  return sb;
}

void 
StdMeshersGUI_DistrTableFrame::SpinBoxDelegate::
setEditorData( QWidget* editor, const QModelIndex& index ) const
{
  QString value = index.model()->data(index, Qt::DisplayRole).toString();
  QDoubleSpinBox* sb = static_cast<QDoubleSpinBox*>(editor);

  bool bOk = false;
  double v = value.toDouble( &bOk );
  if ( !bOk ) v = sb->minimum();

  sb->setValue( v );
}

void
StdMeshersGUI_DistrTableFrame::SpinBoxDelegate::
setModelData( QWidget* editor, QAbstractItemModel* model, 
	      const QModelIndex& index ) const
{
  QDoubleSpinBox* sb = static_cast<QDoubleSpinBox*>(editor);
  model->setData( index, QString::number( sb->value() ), Qt::DisplayRole );
}

void 
StdMeshersGUI_DistrTableFrame::SpinBoxDelegate::
updateEditorGeometry( QWidget* editor,
		      const QStyleOptionViewItem& option, 
		      const QModelIndex& /*index*/ ) const
{
  editor->setGeometry( option.rect );
}

// ---
// StdMeshersGUI_DistrTableFrame::Table implementation
// ---

StdMeshersGUI_DistrTableFrame::Table::
Table( QWidget* parent, int rows )
  : QTableWidget( parent ), myFuncMin( 0.0 )
{
  setItemDelegate( new StdMeshersGUI_DistrTableFrame::SpinBoxDelegate( this ) );

  setColumnCount( 2 );

  QStringList labs;
  labs << "t" << "f(t)";
  setHorizontalHeaderLabels( labs );

  while( rows-- )
    addRow();

  setUpRows( true );
}

void
StdMeshersGUI_DistrTableFrame::Table::
setEditor( int r, int c, QDoubleSpinBox* sb )
{
  myEditorData.r  = r;
  myEditorData.c  = c;
  myEditorData.sb = sb;
}

StdMeshersGUI_DistrTableFrame::Table::
~Table()
{
}

QList<double>
StdMeshersGUI_DistrTableFrame::Table::
data()
{
  closePersistentEditor( currentItem() );

  QList<double> d;
  for ( int r = 0; r < rowCount(); r++ ) {
    d.append( value( r, ArgColumn ) );
    d.append( value( r, FuncColumn ) );
  }
  return d;
}

void
StdMeshersGUI_DistrTableFrame::Table::
setData( const QList<double>& d )
{
  closePersistentEditor( currentItem() );

  setRowCount( d.count() / 2 );
  for ( int r = 0; r < rowCount(); r++ ) {
    setValue( r, ArgColumn,  d[r*2]   );
    setValue( r, FuncColumn, d[r*2+1] );
  }
}

double
StdMeshersGUI_DistrTableFrame::Table::
value( int r, int c ) const
{
  if ( r < 0 || r > rowCount() || c < 0 || c > columnCount() || !item( r, c ) )
    return 0.0;

  return item( r, c )->text().toDouble();
}

void
StdMeshersGUI_DistrTableFrame::Table::
setValue( int r, int c, double v )
{
  if ( r < 0 || r > rowCount() || c < 0 || c > columnCount() )
    return;

  if ( c == FuncColumn && v < funcMinimum( r ) )
    v = funcMinimum( r ); // correct func value according to the valid min value
  if ( c == FuncColumn && v > funcMaximum( r ) )
    v = funcMaximum( r ); // correct func value according to the valid max value
  else if ( r == ArgColumn && v < argMinimum( r ) )
    v = argMinimum( r );  // correct arg  value according to the valid min value
  else if ( r == ArgColumn && v > argMaximum( r ) )
    v = argMaximum( r );  // correct arg  value according to the valid max value

  if ( !item( r, c ) )
    setItem( r, c, new QTableWidgetItem );
  item( r, c )->setText( QString::number( v ) );
}

double
StdMeshersGUI_DistrTableFrame::Table::
argMinimum( int r ) const
{
  // for the first row the minimum value is always 0.0
  // for the other rows the minumum value is the above row's value
  double val = 0.0;
  if ( r > 0 && r < rowCount() )
    val = value( r-1, ArgColumn );
  return val;
}

double
StdMeshersGUI_DistrTableFrame::Table::
argMaximum( int r ) const
{
  // for the last row the maximum value is always 1.0
  // for the other rows the maxumum value is the below row's value
  double val = 1.0;
  if ( r >= 0 && r < rowCount()-1 ) {
    val = value( r+1, ArgColumn );
  }
  return val;
}

double
StdMeshersGUI_DistrTableFrame::Table::
argStep( int /*r*/ ) const
{
  // correct this to provide more smart behaviour if needed
  return 0.1;
}

double
StdMeshersGUI_DistrTableFrame::Table::
funcMinimum( int /*r*/ ) const
{
  // correct this to provide more smart behaviour if needed
  return myFuncMin;
}

double
StdMeshersGUI_DistrTableFrame::Table::
funcMaximum( int /*r*/ ) const
{
  // correct this to provide more smart behaviour if needed
  return 1e20;
}

double
StdMeshersGUI_DistrTableFrame::Table::
funcStep( int /*r*/ ) const
{
  // correct this to provide more smart behaviour if needed
  return 1.0;
}

void
StdMeshersGUI_DistrTableFrame::Table::
setFuncMinValue( double val )
{
  myFuncMin = val;

  QTableWidgetItem* i = currentItem();
  if ( i && 
       i->row()    == myEditorData.r && 
       i->column() == myEditorData.c && 
       i->column() == FuncColumn     &&
       myEditorData.sb ) {
    myEditorData.sb->setMinimum( myFuncMin );
  }
  else {
    closePersistentEditor( currentItem() );
  }

  for ( int r = 0; r < rowCount(); r++ ) {
    double v = item( r, FuncColumn )->text().toDouble();
    if ( v < myFuncMin ) 
      item( r, FuncColumn )->setText( QString::number( myFuncMin ) );
  }
}

QSize
StdMeshersGUI_DistrTableFrame::Table::
sizeHint() const
{
  if( cachedSizeHint().isValid() )
    return cachedSizeHint();

  QSize sh = QTableWidget::sizeHint();
  if( sh.width() < 400 )
    sh.setWidth( 400 );
  if( sh.height() < 200 )
    sh.setHeight( 200 );

  setCachedSizeHint( sh );
  return sh;
}

void
StdMeshersGUI_DistrTableFrame::Table::
addRow()
{
  int r = currentRow() >= 0 ? currentRow() : ( rowCount() > 0 ? rowCount() - 1 : 0 );
  insertRow( r );

  double argMin  = argMinimum( r );
  double funcMin = funcMinimum( r );
  
  setItem( r, ArgColumn,  new QTableWidgetItem( QString::number( argMin ) ) );
  setItem( r, FuncColumn, new QTableWidgetItem( QString::number( funcMin ) ) );
}

void
StdMeshersGUI_DistrTableFrame::Table::
deleteRow()
{
  QList<int> selRows = selectedRows();
  for ( int r = selRows.count()-1; r >= 0; r-- )
    removeRow( r );
}

void
StdMeshersGUI_DistrTableFrame::Table::
closeEditor( QWidget* editor, QAbstractItemDelegate::EndEditHint hint )
{
  myEditorData.reset();
  QTableWidget::closeEditor( editor, hint );
}

void
StdMeshersGUI_DistrTableFrame::Table::
setUpRows( bool autoset )
{
  if ( rowCount() < 1 )
    return;
  if ( autoset ) {
    double s = argMaximum( rowCount()-1 ) / rowCount();
    for ( int r = 0; r < rowCount()-1; r++ )
      setValue( r, ArgColumn, r * s );
    setValue( rowCount()-1, ArgColumn, argMaximum( rowCount()-1 ) );
  }
  else {
    // TODO
  }
}

QSize
StdMeshersGUI_DistrTableFrame::Table::
cachedSizeHint() const
{
  return myCachedSizeHint;
}

void
StdMeshersGUI_DistrTableFrame::Table::
setCachedSizeHint( const QSize& s ) const
{
  Table* that = const_cast<Table*>( this );
  that->myCachedSizeHint = s;
}

QList<int>
StdMeshersGUI_DistrTableFrame::Table::
selectedRows()
{
  QList<int> l;
  QList<QTableWidgetItem*> selItems = selectedItems();
  QTableWidgetItem* i;
  foreach( i, selItems )
    if ( !l.contains( i->row() ) ) l.append( i->row() );
  qSort( l );
  return l;
}

/*!
  \class StdMeshersGUI_DistrTableFrame
  \brief Distribution table widget
*/

StdMeshersGUI_DistrTableFrame::
StdMeshersGUI_DistrTableFrame( QWidget* parent )
  : QWidget( parent )
{
  QVBoxLayout* main = new QVBoxLayout( this );
  main->setMargin( 0 );
  main->setSpacing( 0 );

  // ---
  myTable = new Table( this );
  connect( myTable, SIGNAL( valueChanged( int, int ) ), this, SIGNAL( valueChanged( int, int ) ) );
  
  // ---
  QWidget* aButFrame = new QWidget( this );
  QHBoxLayout* butLay = new QHBoxLayout( aButFrame );
  butLay->setContentsMargins( 0, SPACING, 0, SPACING );
  butLay->setSpacing( SPACING );

  myButtons[ InsertRowBtn ] = new QPushButton( tr( "SMESH_INSERT_ROW" ), aButFrame );
  myButtons[ RemoveRowBtn ] = new QPushButton( tr( "SMESH_REMOVE_ROW" ), aButFrame );

  butLay->addWidget( myButtons[ InsertRowBtn ] );
  butLay->addWidget( myButtons[ RemoveRowBtn ] );
  butLay->addStretch();

  // ---
  main->addWidget( myTable );
  main->addWidget( aButFrame );
  
  // ---
  connect( myButtons[ InsertRowBtn ], SIGNAL( clicked() ), this, SLOT( onInsert() ) );
  connect( myButtons[ RemoveRowBtn ], SIGNAL( clicked() ), this, SLOT( onRemove() ) );
  connect( myTable, SIGNAL( currentCellChanged( int, int, int, int ) ),
	   this,    SIGNAL( currentChanged( int, int ) ) );
  connect( myTable, SIGNAL( cellChanged( int, int ) ),
	   this,    SIGNAL( valueChanged( int, int ) ) );
}

StdMeshersGUI_DistrTableFrame::
~StdMeshersGUI_DistrTableFrame()
{
}

void
StdMeshersGUI_DistrTableFrame::
showButton( const TableButton b, const bool on )
{
  if ( button( b ) ) button( b )->setVisible( on );
}

bool
StdMeshersGUI_DistrTableFrame::
isButtonShown( const TableButton b ) const
{
  return button( b ) ? button( b )->isVisible() : false;
}
  
void
StdMeshersGUI_DistrTableFrame::
data( DataArray& array ) const
{
  QList<double> d = myTable->data();
  sortData( d );

  array.length( d.count() );
  for ( int i = 0; i < d.count(); i++ )
    array[i] = d[i];
}

void
StdMeshersGUI_DistrTableFrame::
setData( const DataArray& array )
{
  QList<double> d;
  for ( int i = 0; i < array.length(); i++ )
    d.append( array[i] );

  sortData( d );
  myTable->setData( d );
}

void
StdMeshersGUI_DistrTableFrame::
setFuncMinValue( double v )
{
  myTable->setFuncMinValue( v );
}

QPushButton*
StdMeshersGUI_DistrTableFrame::
button( const TableButton b ) const
{
  return myButtons.contains( b ) ? myButtons[ b ] : 0;
}

void
StdMeshersGUI_DistrTableFrame::
onInsert()
{
  myTable->addRow();
}

void
StdMeshersGUI_DistrTableFrame::
onRemove()
{
  myTable->deleteRow();
}
