//  SMESH StdMeshersGUI
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
//  File   : StdMeshersGUI_DistrTable.cxx
//  Module : SMESH
//  $Header$

#include "StdMeshersGUI_DistrTable.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvalidator.h>
#include <qlineedit.h>

//=================================================================================
// class    : StdMeshersGUI_DistrTable
// purpose  :
//=================================================================================
StdMeshersGUI_DistrTable::StdMeshersGUI_DistrTable( const int rows, QWidget* parent, const char* name )
: QTable( rows, 2, parent, name )
{
  horizontalHeader()->setLabel( 0, "t" );
  horizontalHeader()->setLabel( 1, "f(t)" );
  myArgV = new QDoubleValidator( 0.0, 1.0, 3, this );
  myFuncV = new QDoubleValidator( 0.0, 1E10, 3, this );
}

StdMeshersGUI_DistrTable::~StdMeshersGUI_DistrTable()
{
}

QSize StdMeshersGUI_DistrTable::sizeHint() const
{
  if( cachedSizeHint().isValid() )
    return cachedSizeHint();

  constPolish();

  QSize sh = QScrollView::sizeHint();
  if( sh.width()<400 )
    sh.setWidth( 400 );
  if( sh.height()<200 )
    sh.setHeight( 200 );

  setCachedSizeHint( sh );
  return sh;
}

void StdMeshersGUI_DistrTable::stopEditing( const bool accept )
{
  endEdit( currEditRow(), currEditCol(), accept, false );
}

void StdMeshersGUI_DistrTable::edit( const int r, const int c )
{
  if( isEditing() )
    endEdit( currEditRow(), currEditCol(), true, false );
  clearSelection();
  setCurrentCell( r, c );
  if( beginEdit( r, c, false ) )
    setEditMode( Editing, r, c );
  QTableSelection sel;
  sel.init( r, c );
  sel.expandTo( r, c );
  addSelection( sel );
}

bool StdMeshersGUI_DistrTable::eventFilter( QObject* o, QEvent* e )
{
  if( e && e->type()==QEvent::KeyPress )
  {
    QKeyEvent* ke = ( QKeyEvent* )e;
    int k = ke->key();
    if( k==Qt::Key_Tab || k==Qt::Key_BackTab || k==Qt::Key_Return || k==Qt::Key_Up || k==Qt::Key_Down )
    {
      keyPressEvent( ke );
      return true;
    }
  }
  return QTable::eventFilter( o, e );
}

void StdMeshersGUI_DistrTable::keyPressEvent( QKeyEvent* e )
{
  if( e )
  {
    int r = currentRow(), c = currentColumn(), nr, nc;
    bool shift = e->state() & Qt::ShiftButton, cr = false;
    switch( e->key() )
    {
    case Qt::Key_Tab:
      nc = c+1;
      nr = r;
      break;

    case Qt::Key_BackTab:
      nc = c-1;
      nr = r;
      break;

    case Qt::Key_Return:
      nc = 0;
      nr = shift ? r-1 : r+1;
      cr = true;
      break;

    case Qt::Key_Up:
      nc = c;
      nr = r-1;
      break;

    case Qt::Key_Down:
      nc = c;
      nr = r+1;
      break;

    default:
      QTable::keyPressEvent( e );
      return;
    }

    if( nc<0 )
    {
      nc=1; nr--;
    }
    if( nc>1 )
    {
      nc=0; nr++;
    }

    if( nr>=numRows() && cr )
    {
      if( isEditing() )
	endEdit( currEditRow(), currEditCol(), true, false );
      onEdit( INSERT_ROW, nr );
    }

    else if( nr<0 || nr>=numRows() )
    {
      nr = r; nc = c;	
    }
    edit( nr, nc );
    e->accept();
  }
}

QWidget* StdMeshersGUI_DistrTable::createEditor( int r, int c, bool init ) const
{
  QWidget* w = QTable::createEditor( r, c, init );
  if( w )
  {
    //w->installEventFilter( this );
    if( w->inherits( "QLineEdit" ) )
    {
      QLineEdit* le = ( QLineEdit* )w;
      le->setValidator( c==0 ? myArgV : myFuncV );
    }
  }
  
  return w;
}

void StdMeshersGUI_DistrTable::onEdit( TableButton b, int cur )
{
  switch( b )
  {     
  case INSERT_ROW:
    setNumRows( numRows()+1 );
    for( int i=numRows()-1; i>=cur; i-- )
      for( int j=0; j<numCols(); j++ )
	if( i>cur )
	  setText( i, j, text( i-1, j ) );
        else
	  setText( i, j, "0" );
    emit( valueChanged( cur, 0 ) );
    break;
      
  case REMOVE_ROW:
    if( numRows()>1 )
    {
      for( int i=cur; i<numRows(); i++ )
	for( int j=0; j<numCols(); j++ )
	  setText( i, j, text( i+1, j ) );
      setNumRows( numRows()-1 );
    }
    emit( valueChanged( cur, 0 ) );
    break;
  }
}

void StdMeshersGUI_DistrTable::sortData( SMESH::double_array& arr )
{
  QValueList< QPair<double,double> > aData;
  if( arr.length()%2==1 )
    arr.length( arr.length()-1 );

  int aLen = arr.length();
  for( int i=0; i<aLen/2; i++ )
    aData.append( QPair<double,double>( arr[2*i], arr[2*i+1] ) );

  qHeapSort( aData );

  QValueList< QPair<double,double> >::const_iterator anIt = aData.begin(), aLast = aData.end();
  QValueList<double> unique_values;
  double prev; int i=0;
  if( (*anIt).first>0.0 )
  {
    unique_values.append( 0.0 );
    unique_values.append( (*anIt).second );
    i++; prev = 0.0;
  }
  for( ; anIt!=aLast; anIt++ )
  {
    if( i==0 || (*anIt).first>prev )
    {
      unique_values.append( (*anIt).first );
      unique_values.append( (*anIt).second );
      i++;
    }
    prev = (*anIt).first;
  }
  if( prev<1.0 )
  {
    unique_values.append( 1.0 );
    anIt--;
    unique_values.append( (*anIt).second );
  }

  arr.length( unique_values.count() );
  QValueList<double>::const_iterator anIt1 = unique_values.begin(), aLast1 = unique_values.end();
  for( int j=0; anIt1!=aLast1; anIt1++, j++ )
    arr[j] = *anIt1;
}

void StdMeshersGUI_DistrTable::data( SMESH::double_array& v )
{
  stopEditing( true );
  v.length( 2*numRows() );
  for( int i=0; i<numRows(); i++ )
    for( int j=0; j<numCols(); j++ )
      v[numCols()*i+j] = text( i, j ).toDouble();
  sortData( v );
}

void StdMeshersGUI_DistrTable::setData( const SMESH::double_array& d )
{
  stopEditing( false );
  setNumRows( d.length()/2 );
  for( int i=0; i<d.length(); i++ )
    setText( i/2, i%2, QString( "%1" ).arg( d[i] ) );
}

//=================================================================================
// class    : StdMeshersGUI_DistrTableFrame
// purpose  :
//=================================================================================
StdMeshersGUI_DistrTableFrame::StdMeshersGUI_DistrTableFrame( QWidget* parent )
: QFrame( parent )
{
  QVBoxLayout* main = new QVBoxLayout( this, 0, 0 );

  myTable = new StdMeshersGUI_DistrTable( 1, this );
  connect( myTable, SIGNAL( valueChanged( int, int ) ), this, SIGNAL( valueChanged( int, int ) ) );
  connect( this, SIGNAL( toEdit( TableButton, int ) ), myTable, SLOT( onEdit( TableButton, int ) ) );
  
  QFrame* aButFrame = new QFrame( this );
  QHBoxLayout* butLay = new QHBoxLayout( aButFrame, 5, 5 );

  myInsertRow = new QPushButton( tr( "SMESH_INSERT_ROW" ), aButFrame );
  myRemoveRow = new QPushButton( tr( "SMESH_REMOVE_ROW" ), aButFrame );

  butLay->addWidget( myInsertRow, 0 );
  butLay->addWidget( myRemoveRow, 0 );
  butLay->addStretch( 1 );

  main->addWidget( myTable, 1 );
  main->addWidget( aButFrame, 0 );

  connect( myInsertRow, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( myRemoveRow, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
}

StdMeshersGUI_DistrTableFrame::~StdMeshersGUI_DistrTableFrame()
{
}

StdMeshersGUI_DistrTable* StdMeshersGUI_DistrTableFrame::table() const
{
  return myTable;
}

void StdMeshersGUI_DistrTableFrame::setShown( const TableButton b, const bool sh )
{
  if( button( b ) )
    button( b )->setShown( sh );
}

bool StdMeshersGUI_DistrTableFrame::isShown( const TableButton b ) const
{
  bool res = false;
  if( button( b ) )
    res = button( b )->isShown();
  return res;
}

QButton* StdMeshersGUI_DistrTableFrame::button( const TableButton b ) const
{
  QButton* res = 0;
  switch( b )
  {
    case INSERT_ROW:
      res = myInsertRow;
      break;

    case REMOVE_ROW:
      res = myRemoveRow;
      break;
  }
  return res;
}

void StdMeshersGUI_DistrTableFrame::onButtonClicked()
{
  if( sender()==button( INSERT_ROW ) )
    emit toEdit( INSERT_ROW, table()->currentRow() );
    
  else if( sender()==button( REMOVE_ROW ) )
    emit toEdit( REMOVE_ROW, table()->currentRow() );
}
