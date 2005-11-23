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
//  File   : SMESHGUI_aParameter.cxx
//  Module : SMESH
//  $Header$

#include "SMESHGUI_aParameter.h"

#include <qspinbox.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtable.h>
#include <qvalidator.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <QtxDblSpinBox.h>

SMESHGUI_aParameter::SMESHGUI_aParameter( const QString& label, const bool preview )
: _needPreview( preview ),
  _label(label)
{
}

SMESHGUI_aParameter::~SMESHGUI_aParameter()
{
}

bool SMESHGUI_aParameter::needPreview() const
{
  return _needPreview;
}

QString& SMESHGUI_aParameter::Label()
{
  return _label;
}

QString SMESHGUI_aParameter::sigValueChanged() const
{
  return QString::null;
}

//=================================================================================
// class    : SMESHGUI_intParameter
// purpose  :
//=================================================================================
SMESHGUI_intParameter::SMESHGUI_intParameter (const int      theInitValue,
                                              const QString& theLabel,
                                              const int      theBottom,
                                              const int      theTop,
					      const bool     preview  )
     :SMESHGUI_aParameter(theLabel, preview),
       _top(theTop), _bottom(theBottom), _initValue(theInitValue),
       _newValue( theInitValue )
{
}
SMESHGUI_aParameter::Type SMESHGUI_intParameter::GetType() const
{
  return SMESHGUI_aParameter::INT;
}
bool SMESHGUI_intParameter::GetNewInt (int & theValue) const
{
  theValue = _newValue;
  return _newValue != _initValue;
}
bool SMESHGUI_intParameter::GetNewDouble (double & Value) const
{
  return false;
}
bool SMESHGUI_intParameter::GetNewText (QString & Value) const
{
  return false;
}

QWidget* SMESHGUI_intParameter::CreateWidget( QWidget* parent ) const
{
  return new QSpinBox( parent );
}
  
void SMESHGUI_intParameter::InitializeWidget (QWidget* theQWidget) const
{
  QSpinBox * aSpin = dynamic_cast< QSpinBox *>(theQWidget);
  if (aSpin) {
    aSpin->setMinValue(_bottom);
    aSpin->setMaxValue(_top);
    aSpin->setValue(_initValue);
  }
}
void SMESHGUI_intParameter::TakeValue (QWidget* theQWidget)
{
  QSpinBox * aSpin = dynamic_cast< QSpinBox *>(theQWidget);
  if (aSpin)
    _newValue = aSpin->value();
}

QString SMESHGUI_intParameter::sigValueChanged() const
{
  return SIGNAL( valueChanged( int ) );
}

//=================================================================================
// class    : SMESHGUI_doubleParameter
// purpose  :
//=================================================================================
SMESHGUI_doubleParameter::SMESHGUI_doubleParameter (const double   theInitValue,
                                                    const QString& theLabel,
                                                    const double   theBottom,
                                                    const double   theTop,
                                                    const double   theStep,
                                                    const int      theDecimals,
						    const bool     preview )
     :SMESHGUI_aParameter(theLabel, preview),
       _top(theTop), _bottom(theBottom), _step(theStep),
       _initValue(theInitValue), _decimals(theDecimals)
{
}
SMESHGUI_aParameter::Type SMESHGUI_doubleParameter::GetType() const
{
  return SMESHGUI_aParameter::DOUBLE;
}
bool SMESHGUI_doubleParameter::GetNewInt (int & theValue) const
{
  return false;
}
bool SMESHGUI_doubleParameter::GetNewDouble (double & Value) const
{
  Value = _newValue;
  return _newValue != _initValue;
}
bool SMESHGUI_doubleParameter::GetNewText (QString & Value) const
{
  return false;
}

QWidget* SMESHGUI_doubleParameter::CreateWidget( QWidget* parent ) const
{
  return new QtxDblSpinBox( parent );
}

void SMESHGUI_doubleParameter::InitializeWidget (QWidget* theQWidget) const
{
  QtxDblSpinBox* aSpin = dynamic_cast<QtxDblSpinBox*>(theQWidget);
  if (aSpin) {
    aSpin->setPrecision(_decimals);
#ifdef NEW_GUI
    aSpin->setDblPrecision(_bottom);
#endif
    aSpin->setRange(_bottom, _top);
    aSpin->setValue(_initValue);
    aSpin->setLineStep(_step);
  }
}
void SMESHGUI_doubleParameter::TakeValue (QWidget* theQWidget)
{
  QtxDblSpinBox* aSpin = dynamic_cast<QtxDblSpinBox*>(theQWidget);
  if (aSpin)
    _newValue = aSpin->value();
}

QString SMESHGUI_doubleParameter::sigValueChanged() const
{
  return SIGNAL( valueChanged( double ) );
}

//=================================================================================
// class    : SMESHGUI_strParameter
// purpose  :
//=================================================================================
SMESHGUI_strParameter::SMESHGUI_strParameter (const QString& theInitValue,
                                              const QString& theLabel,
					      const bool preview )
     :SMESHGUI_aParameter(theLabel, preview),
      _initValue(theInitValue)
{
}
SMESHGUI_aParameter::Type SMESHGUI_strParameter::GetType() const
{
  return SMESHGUI_aParameter::STRING;
}

bool SMESHGUI_strParameter::GetNewInt (int & theValue) const
{
  return false;
}
bool SMESHGUI_strParameter::GetNewDouble (double & Value) const
{
  return false;
}
bool SMESHGUI_strParameter::GetNewText (QString & theValue) const
{
  theValue = _newValue;
  return _newValue != _initValue;
}

QWidget* SMESHGUI_strParameter::CreateWidget( QWidget* parent ) const
{
  return new QLineEdit( parent );
}

void SMESHGUI_strParameter::InitializeWidget (QWidget* theQWidget) const
{
  QLineEdit* anEdit = dynamic_cast< QLineEdit* >(theQWidget);
  if (anEdit) {
    anEdit->setText(_initValue);
  }
}
void SMESHGUI_strParameter::TakeValue (QWidget* theQWidget)
{
  QLineEdit* anEdit = dynamic_cast< QLineEdit* >(theQWidget);
  if (anEdit)
    _newValue = anEdit->text();
}

QString SMESHGUI_strParameter::sigValueChanged() const
{
  return SIGNAL( textChanged( const QString& ) );
}



//=================================================================================
// class    : SMESHGUI_dependParameter
// purpose  :
//=================================================================================
SMESHGUI_dependParameter::SMESHGUI_dependParameter( const QString& label, const bool     preview )
: SMESHGUI_aParameter( label, preview )
{
}

const SMESHGUI_dependParameter::ShownMap& SMESHGUI_dependParameter::shownMap() const
{
  return myShownMap;
}

SMESHGUI_dependParameter::ShownMap& SMESHGUI_dependParameter::shownMap()
{
  return myShownMap;
}


  


//=================================================================================
// class    : SMESHGUI_enumParameter
// purpose  :
//=================================================================================
SMESHGUI_enumParameter::SMESHGUI_enumParameter( const QStringList& values,
                                                const int initValue,
                                                const QString& label,
					        const bool     preview )
: SMESHGUI_dependParameter( label, preview ),
  myInitValue( initValue ),
  myValue( initValue ),
  myValues( values )
{
}

SMESHGUI_enumParameter::~SMESHGUI_enumParameter()
{
}

SMESHGUI_aParameter::Type SMESHGUI_enumParameter::GetType() const
{
  return SMESHGUI_aParameter::ENUM;
}

bool SMESHGUI_enumParameter::GetNewInt( int& v ) const
{
  v = myValue;
  return myValue!=myInitValue;
}

bool SMESHGUI_enumParameter::GetNewDouble( double& ) const
{
  return false;
}

bool SMESHGUI_enumParameter::GetNewText( QString& v ) const
{
  bool res = myValue>=0 && myValue<Count();

  if( res )
    v = myValues[ myValue ];

  return res && v!=myInitValue;
}

QWidget* SMESHGUI_enumParameter::CreateWidget( QWidget* parent ) const
{
  return new QComboBox( parent );
}

void SMESHGUI_enumParameter::InitializeWidget( QWidget* w ) const
{
  if( w && w->inherits( "QComboBox" ) )
  {
    QComboBox* c = ( QComboBox* ) w;
    c->clear();
    c->insertStringList( myValues );
    c->setCurrentItem( myInitValue );
  }
}

void SMESHGUI_enumParameter::TakeValue( QWidget* w )
{
  if( w && w->inherits( "QComboBox" ) )
  {
    QComboBox* c = ( QComboBox* ) w;
    myValue = c->currentItem();
  }
}

int SMESHGUI_enumParameter::Count() const
{
  return myValues.count();
}

QString SMESHGUI_enumParameter::sigValueChanged() const
{
  return SIGNAL( activated( int ) );
}


//=================================================================================
// class    : SMESHGUI_boolParameter
// purpose  :
//=================================================================================
SMESHGUI_boolParameter::SMESHGUI_boolParameter( const bool initValue,
                                                const QString& label,
					        const bool     preview )
: SMESHGUI_dependParameter( label, preview ),
  myInitValue( initValue ),
  myValue( myInitValue )
{
}

SMESHGUI_boolParameter::~SMESHGUI_boolParameter()
{
}

SMESHGUI_aParameter::Type SMESHGUI_boolParameter::GetType() const
{
  return BOOL;
}

bool SMESHGUI_boolParameter::GetNewInt( int& v ) const
{
  if( myValue )
    v = 1;
  else
    v = 0;
  return v!=myInitValue;
}

bool SMESHGUI_boolParameter::GetNewDouble( double& ) const
{
  return false;
}

bool SMESHGUI_boolParameter::GetNewText( QString& ) const
{
  return false;
}

QWidget* SMESHGUI_boolParameter::CreateWidget( QWidget* parent ) const
{
  return new QCheckBox( parent );
}

void SMESHGUI_boolParameter::InitializeWidget( QWidget* w ) const
{
  if( w && w->inherits( "QCheckBox" ) )
  {
    QCheckBox* box = ( QCheckBox* )w;
    box->setChecked( myInitValue );
  }
}

void SMESHGUI_boolParameter::TakeValue( QWidget* w )
{
  if( w && w->inherits( "QCheckBox" ) )
  {
    QCheckBox* box = ( QCheckBox* )w;
    myValue = box->isChecked();
  }  
}

QString SMESHGUI_boolParameter::sigValueChanged() const
{
  return SIGNAL( stateChanged( int ) );
}




//=================================================================================
// class    : SMESHGUI_doubleItem
// purpose  : Custom table item which contains double and has double validator
//=================================================================================
class SMESHGUI_doubleItem: public QTableItem
{
public:
  SMESHGUI_doubleItem( QTable*, EditType, const double );
  virtual ~SMESHGUI_doubleItem();

  void setValidator( const double, const double, const int );
  void validator( double&, double&, int& );
  virtual QWidget* createEditor() const;

private:
  QDoubleValidator*  myValidator;
};

SMESHGUI_doubleItem::SMESHGUI_doubleItem( QTable* t, EditType e, const double num )
: QTableItem( t, e, QString( "%1" ).arg( num ) ),
  myValidator( new QDoubleValidator( 0.0, 1.0, 3, t ) )
{
}

SMESHGUI_doubleItem::~SMESHGUI_doubleItem()
{
}

void SMESHGUI_doubleItem::setValidator( const double bot, const double top, const int dec )
{
  myValidator->setBottom( bot );
  myValidator->setTop( top );
  myValidator->setDecimals( dec );
}

void SMESHGUI_doubleItem::validator( double& bot, double& top, int& dec )
{
  bot = myValidator->bottom();
  top = myValidator->top();
  dec = myValidator->decimals();
}

QWidget* SMESHGUI_doubleItem::createEditor() const
{
  QWidget* res = QTableItem::createEditor();
  if( res && res->inherits( "QLineEdit" ) )
  {
    QLineEdit* l = ( QLineEdit* )res;
    l->setValidator( myValidator );
  }
  return res;
}


//=================================================================================
// class    : SMESHGUI_Table
// purpose  :
//=================================================================================
SMESHGUI_Table::SMESHGUI_Table( const SMESHGUI_tableParameter* tab, int numRows, int numCols,
			        QWidget* parent, const char* name )
: QTable( numRows, numCols, parent, name ),
  myParam( ( SMESHGUI_tableParameter* )tab )
{
}

SMESHGUI_Table::~SMESHGUI_Table()
{
}

QSize SMESHGUI_Table::sizeHint() const
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

void SMESHGUI_Table::stopEditing()
{
  endEdit( currEditRow(), currEditCol(), false, false );
}

void SMESHGUI_Table::validator( const int row, const int col, double& minV, double& maxV, int& dec )
{
  SMESHGUI_doubleItem* it = dynamic_cast<SMESHGUI_doubleItem*>( item( row, col ) );
  if( it )
    it->validator( minV, maxV, dec );
}

void SMESHGUI_Table::setValidator( const double minV, const double maxV, const int dec,
                                   const int rmin, const int rmax,
                                   const int cmin, const int cmax )
{
  int r1 = rmin>=0 ? rmin : 0,
      r2 = rmax>=0 ? rmax : numRows(),
      c1 = cmin>=0 ? cmin : 0,
      c2 = cmax>=0 ? cmax : numCols();

  for( int i=r1; i<=r2; i++ )
    for( int j=c1; j<=c2; j++ )
    {
      SMESHGUI_doubleItem* it = dynamic_cast<SMESHGUI_doubleItem*>( item( i, j ) );
      if( it )
        it->setValidator( minV, maxV, dec );
    }
}

bool SMESHGUI_Table::eventFilter( QObject* o, QEvent* e )
{
  if( o && e && e->type()==QEvent::KeyPress )
  {
    QKeyEvent* ke = ( QKeyEvent* )e;
    if( ke->key()==Qt::Key_Tab || ke->key()==Qt::Key_Backtab || ke->key()==Qt::Key_Return )
    {
      keyPressEvent( ke );
      return true;
    }
  }

  return QTable::eventFilter( o, e );
}

void SMESHGUI_Table::keyPressEvent( QKeyEvent* e )
{
  if( e )
  {
    bool shift = ( e->state() & Qt::ShiftButton );
    int col = currentColumn(), row = currentRow();
    if( e->key()==Qt::Key_Tab || e->key()==Qt::Key_Backtab )
    {
      if( e->key()==Qt::Key_Tab )
	col++;
      else 
	col--;
      if( col<0 )
      {
	col = numCols()-1;
	row--;
	if( row<0 )
	{
	  col = 0;
	  row = 0;
	}
      }
      if( col>=numCols() )
      {
	col = 0;
	row++;
	if( row>=numRows() )
	  row = numRows()-1;
      }
      e->accept();
    }
    else if( e->key()==Qt::Key_Return )
    {
      col = 0;
      if( shift )
	row--;
      else
	row++;
      if( row<0 )
	row = 0;
      else if( row>=numRows() )
      {
	//add row
	myParam->onEdit( this, SMESHGUI_TableFrame::ADD_ROW, 1 );
      }
      e->accept();
    }
    if( e->isAccepted() )
    {
      clearSelection();
      setCurrentCell( row, col );
    }
    else
      QTable::keyPressEvent( e );
  }
}

QWidget* SMESHGUI_Table::createEditor( int r, int c, bool init ) const
{
  QWidget* w = QTable::createEditor( r, c, init );
  if( w )
    w->installEventFilter( this );
  return w;
}



//=================================================================================
// class    : SMESHGUI_TableFrame
// purpose  :
//=================================================================================
SMESHGUI_TableFrame::SMESHGUI_TableFrame( const SMESHGUI_tableParameter* param, QWidget* parent )
: QFrame( parent )
{
  QVBoxLayout* main = new QVBoxLayout( this, 0, 0 );

  myTable = new SMESHGUI_Table( param, 1, 1, this );
  connect( myTable, SIGNAL( valueChanged( int, int ) ), this, SIGNAL( valueChanged( int, int ) ) );
  
  QFrame* aButFrame = new QFrame( this );
  QHBoxLayout* butLay = new QHBoxLayout( aButFrame, 5, 5 );

  myAddColumn = new QPushButton( "Add column", aButFrame );

  myRemoveColumn = new QPushButton( "Remove column", aButFrame );

  myAddRow = new QPushButton( "Add row", aButFrame );

  myRemoveRow = new QPushButton( "Remove row", aButFrame );

  butLay->addWidget( myAddColumn, 0 );
  butLay->addWidget( myRemoveColumn, 0 );
  butLay->addWidget( myAddRow, 0 );
  butLay->addWidget( myRemoveRow, 0 );
  butLay->addStretch( 1 );

  main->addWidget( myTable, 1 );
  main->addWidget( aButFrame, 0 );

  connect( myAddColumn,    SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( myRemoveColumn, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( myAddRow,       SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  connect( myRemoveRow,    SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
}

SMESHGUI_TableFrame::~SMESHGUI_TableFrame()
{
}

SMESHGUI_Table* SMESHGUI_TableFrame::table() const
{
  return myTable;
}

void SMESHGUI_TableFrame::setShown( const Button b, const bool sh )
{
  if( button( b ) )
    button( b )->setShown( sh );
}

bool SMESHGUI_TableFrame::isShown( const Button b ) const
{
  bool res = false;
  if( button( b ) )
    res = button( b )->isShown();
  return res;
}

QButton* SMESHGUI_TableFrame::button( const Button b ) const
{
  QButton* res = 0;
  switch( b )
  {
    case ADD_COLUMN:
      res = myAddColumn;
      break;

    case REMOVE_COLUMN:
      res = myRemoveColumn;
      break;

    case ADD_ROW:
      res = myAddRow;
      break;

    case REMOVE_ROW:
      res = myRemoveRow;
      break;
  }
  return res;
}

void SMESHGUI_TableFrame::onButtonClicked()
{
  if( sender()==button( ADD_COLUMN ) )
    emit toEdit( ADD_COLUMN, table()->currentColumn() );
    
  else if( sender()==button( REMOVE_COLUMN ) )
    emit toEdit( REMOVE_COLUMN, table()->currentColumn() );
    
  else if( sender()==button( ADD_ROW ) )
    emit toEdit( ADD_ROW, table()->currentRow() );
    
  else if( sender()==button( REMOVE_ROW ) )
    emit toEdit( REMOVE_ROW, table()->currentRow() );
}


//=================================================================================
// class    : SMESHGUI_tableParameter
// purpose  :
//=================================================================================
SMESHGUI_tableParameter::SMESHGUI_tableParameter( const double init,
                                                  const QString& label,
						  const bool preview )
: SMESHGUI_aParameter( label, preview ),
  myInitValue( init ),
  myColsInt( 1 ),
  myRowsInt( 1 ),
  myEditCols( false ),
  myEditRows( false )
{
}

SMESHGUI_tableParameter::~SMESHGUI_tableParameter()
{
}

bool operator<( const QPair<double,double>& p1, const QPair<double,double>& p2 )
{
  return p1.first < p2.first;
}

void SMESHGUI_tableParameter::sortData( SMESH::double_array& arr )
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
    unique_values.append( 0.0 );
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
    unique_values.append( 0.0 );
  }

  arr.length( unique_values.count() );
  QValueList<double>::const_iterator anIt1 = unique_values.begin(), aLast1 = unique_values.end();
  for( int j=0; anIt1!=aLast1; anIt1++, j++ )
    arr[j] = *anIt1;
}

SMESHGUI_aParameter::Type SMESHGUI_tableParameter::GetType() const
{
  return TABLE;
}

bool SMESHGUI_tableParameter::GetNewInt( int& ) const
{
  return false;
}

bool SMESHGUI_tableParameter::GetNewDouble( double& ) const
{
  return false;
}

bool SMESHGUI_tableParameter::GetNewText( QString& ) const
{
  return false;
}

QWidget* SMESHGUI_tableParameter::CreateWidget( QWidget* par ) const
{
  SMESHGUI_TableFrame* t = new SMESHGUI_TableFrame( this, par );
  connect( t,    SIGNAL( toEdit( SMESHGUI_TableFrame::Button, int ) ),
           this, SLOT  ( onEdit( SMESHGUI_TableFrame::Button, int ) ) );
  
  update( t );
  return t;
}

void SMESHGUI_tableParameter::setItems( QWidget* w,
                                        int old_row, int new_row,
                                        int old_col, int new_col ) const
{
  if( w && w->inherits( "SMESHGUI_TableFrame" ) )
  {
    QTable* tab = ( ( SMESHGUI_TableFrame* )w )->table();
    
    if( old_row<0 )
      old_row = 0;
    if( new_row<0 )
      new_row = tab->numRows();
    if( old_col<0 )
      old_col = 0;
    if( new_col<0 )
      new_col = tab->numCols();
    
    for( int i=old_row, m=new_row; i<m; i++ )
      for( int j=0, n=new_col; j<n; j++ )
        tab->setItem( i, j, new SMESHGUI_doubleItem( tab, QTableItem::WhenCurrent, myInitValue ) );
    
    for( int i=0, m=new_row; i<m; i++ )
      for( int j=old_col, n=new_col; j<n; j++ )
        tab->setItem( i, j, new SMESHGUI_doubleItem( tab, QTableItem::WhenCurrent, myInitValue ) );

    for( int j=old_col; j<new_col; j++ )
      tab->setColumnWidth( j, 50 );
  }
}

void SMESHGUI_tableParameter::InitializeWidget( QWidget* w ) const
{
  setItems( w );

  if( w && w->inherits( "SMESHGUI_TableFrame" ) )
  {
    SMESHGUI_Table* tab = ( ( SMESHGUI_TableFrame* )w )->table();
    tab->stopEditing();

    int col = tab->numCols(),
        row = tab->numRows();
    
    for( int i=0, m=row; i<m; i++ )
      for( int j=0, n=col; j<n; j++ )
        if( row*j+i<myData.length() )
          tab->item( i, j )->setText( QString( "%1" ).arg( myData[col*i+j] ) );
  }
}

void SMESHGUI_tableParameter::TakeValue( QWidget* w )
{
  if( w && w->inherits( "SMESHGUI_TableFrame" ) )
  {
    QTable* tab = ( ( SMESHGUI_TableFrame* )w )->table();

    int col = tab->numCols(),
        row = tab->numRows();

    myData.length( col*row );
    for( int i=0; i<row; i++ )
      for( int j=0; j<col; j++ )
        myData[ col*i+j ] = tab->text( i, j ).toDouble();
  }
}

void SMESHGUI_tableParameter::data( SMESH::double_array& v ) const
{
  v = myData;
  sortData( v );
}

void SMESHGUI_tableParameter::setData( const SMESH::double_array& d )
{
  myData = d;
  sortData( myData );
}

void SMESHGUI_tableParameter::update( QWidget* w ) const
{
  if( w && w->inherits( "SMESHGUI_TableFrame" ) )
  {
    SMESHGUI_TableFrame* tabfr = ( SMESHGUI_TableFrame* ) w;
    SMESHGUI_Table* tab = tabfr->table();

    int old_col = tab->numCols(),
        old_row = tab->numRows();
        
    int col = myColsInt, row = myRowsInt;
    if( myCols.get() )
      myCols->GetNewInt( col );
        
    if( myRows.get() )
      myRows->GetNewInt( row );

    if( col<=0 )
      col = 1;
    if( row<=0 )
      row = 1;

    if( col!=tab->numCols() )
      tab->setNumCols( col );

    if( row!=tab->numRows() )
      tab->setNumRows( row );

    tabfr->setShown( SMESHGUI_TableFrame::ADD_COLUMN, myEditCols );
    tabfr->setShown( SMESHGUI_TableFrame::REMOVE_COLUMN, myEditCols );
    tabfr->setShown( SMESHGUI_TableFrame::ADD_ROW, myEditRows );
    tabfr->setShown( SMESHGUI_TableFrame::REMOVE_ROW, myEditRows );

    setItems( w, old_row, row, old_col, col );

    QMap< int, QString >::const_iterator aNIt = myColNames.begin(),
                                         aNLast = myColNames.end();
    for( ; aNIt!=aNLast; aNIt++ )
      tab->horizontalHeader()->setLabel( aNIt.key(), aNIt.data() );
    
    ValidatorsMap::const_iterator anIt = myValidators.begin(),
                                  aLast = myValidators.end();
    for( ; anIt!=aLast; anIt++ )
    {
      int row = anIt.key(), dec;
      double minV, maxV;
      validator( row, minV, maxV, dec );
      tab->setValidator( minV, maxV, dec, -1, -1, col, col );
    }

    QSize s = tab->sizeHint();
    tab->resize( s.width(), s.height() );
  }
}

void SMESHGUI_tableParameter::setColCount( const int c, QWidget* w )
{
  myColsInt = c;
  update( w );
}

void SMESHGUI_tableParameter::setRowCount( const int c, QWidget* w )
{
  myRowsInt = c;
  update( w );
}

void SMESHGUI_tableParameter::setColCount( const SMESHGUI_aParameterPtr p, QWidget* w )
{
  if( p.get() )
  {
    myCols = p;
    update( w );
  }
}

void SMESHGUI_tableParameter::setRowCount( const SMESHGUI_aParameterPtr p, QWidget* w )
{
  if( p.get() )
  {
    myRows = p;
    update( w );
  }
}

QString SMESHGUI_tableParameter::sigValueChanged() const
{
  return SIGNAL( valueChanged( int, int ) );
}

void SMESHGUI_tableParameter::setValidator( const int ind, const double minV, const double maxV, const int dec )
{
  ValidatorInfo inf;
  inf.myMin = minV;
  inf.myMax = maxV;
  inf.myDecimals = dec;
  myValidators[ ind ] = inf;
}

void SMESHGUI_tableParameter::validator( const int ind, double& minV, double& maxV, int& dec ) const
{
  if( myValidators.contains( ind ) )
  {
    const ValidatorInfo& inf = myValidators[ ind ];
    minV = inf.myMin;
    maxV = inf.myMax;
    dec = inf.myDecimals;
  }
}

void SMESHGUI_tableParameter::setEditCols( const bool ed )
{
  myEditCols = ed;
}

void SMESHGUI_tableParameter::setEditRows( const bool ed )
{
  myEditRows = ed;
}

void SMESHGUI_tableParameter::setColName( const int ind, const QString& name )
{
  myColNames.insert( ind, name );
}

QString SMESHGUI_tableParameter::colName( const int ind ) const
{
  if( myColNames.contains( ind ) )
    return myColNames[ ind ];
  else
    return QString::null;
}

void SMESHGUI_tableParameter::onEdit( SMESHGUI_TableFrame::Button b, int n )
{
  if( sender() && sender()->inherits( "SMESHGUI_TableFrame" ) )
  {
    SMESHGUI_TableFrame* fr = ( SMESHGUI_TableFrame* )sender();
    SMESHGUI_Table* tab = fr->table();
    onEdit( tab, b, n );
  }
}

void SMESHGUI_tableParameter::onEdit( SMESHGUI_Table* tab, SMESHGUI_TableFrame::Button b, int n )
{
  if( !tab )
    return;

  SMESHGUI_TableFrame* fr = dynamic_cast<SMESHGUI_TableFrame*>( tab->parent() );

  switch( b )
  {
  case SMESHGUI_TableFrame::ADD_COLUMN:
    {
      if( !myEditCols || myCols.get() )
	return;

      myColsInt++; update( fr );
      if( n>=0 )
	for( int i=0; i<myRowsInt; i++ )
	  for( int j=myColsInt-1; j>=n; j-- )
	    if( j==n )
	      tab->setText( i, j, QString( "%1" ).arg( myInitValue ) );
	    else
	      tab->setText( i, j, tab->text( i, j-1 ) );
      break;
    }
    
  case SMESHGUI_TableFrame::REMOVE_COLUMN:
    {
      if( !myEditCols || myCols.get() || myColsInt<=1 )
	return;

      if( n>=0 )
	for( int i=0; i<myRowsInt; i++ )
	  for( int j=n; j<myColsInt-1; j++ )
	    tab->setText( i, j, tab->text( i, j+1 ) );
      myColsInt--; update( fr );
      
      break;
    }
      
  case SMESHGUI_TableFrame::ADD_ROW:
    {
      if( !myEditRows || myRows.get() )
	return;

      myRowsInt++; update( fr );
      if( n>=0 )
	for( int i=myRowsInt-1; i>=n; i-- )
	  for( int j=0; j<myColsInt; j++ )
	    if( i==n )
	      tab->setText( i, j, QString( "%1" ).arg( myInitValue ) );
	    else
	      tab->setText( i, j, tab->text( i-1, j ) );
      break;        
    }
      
  case SMESHGUI_TableFrame::REMOVE_ROW:
    {
      if( !myEditRows || myRows.get() || myRowsInt<=1 )
	return;

      if( n>=0 )
	for( int i=n; i<myRowsInt-1; i++ )
	  for( int j=0; j<myColsInt; j++ )
	    tab->setText( i, j, tab->text( i+1, j ) );
      myRowsInt--; update( fr );
      
      break;
    }
  }
}
