// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : StdMeshersGUI_FixedPointsParamWdg.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "StdMeshersGUI_FixedPointsParamWdg.h"

#include <SMESHGUI_SpinBox.h>

#include <SalomeApp_IntSpinBox.h>

// Qt includes
#include <QPushButton>
#include <QIntValidator>
#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QItemDelegate>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QItemDelegate>
#include <QKeyEvent>

#define SPACING 6
#define MARGIN 0
#define SAME_TEXT "-/-"

#define TOLERANCE 1e-7
#define EQUAL_DBL(a,b) (fabs(a-b)<TOLERANCE)
#define LT_DBL(a,b) ((a<b)&&!EQUAL_DBL(a,b))
#define GT_DBL(a,b) ((a>b)&&!EQUAL_DBL(a,b))

/*
 * class : Tree Widget Item Delegate
 * purpose  : Custom item delegate
 */

class StdMeshersGUI_FixedPointsParamWdg::LineDelegate : public QItemDelegate
{
public:
  LineDelegate( QTreeWidget* );
  ~LineDelegate() {}

  QWidget*     createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
  void         setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;

private:
  QTreeWidget* myTreeWidget;
};

StdMeshersGUI_FixedPointsParamWdg::LineDelegate::LineDelegate( QTreeWidget* parent )
  : QItemDelegate( parent ),
    myTreeWidget( parent )
{
}

QWidget* StdMeshersGUI_FixedPointsParamWdg::LineDelegate::createEditor( QWidget* parent,
                                                                        const QStyleOptionViewItem& option,
                                                                        const QModelIndex& index ) const
{
  QWidget* w = 0;
  if ( (index.column() == 1 ) ) {
    SalomeApp_IntSpinBox* sb = new SalomeApp_IntSpinBox( parent );
    sb->setAcceptNames( false ); // No Notebook variables allowed
    sb->setFrame( false );
    sb->setRange( 1, 999);
    w = sb;
  }

  return w;
}

void StdMeshersGUI_FixedPointsParamWdg::LineDelegate::setModelData( QWidget* editor, 
                                                                    QAbstractItemModel* model, 
                                                                    const QModelIndex& index ) const
{
  model->setData( index, qobject_cast<SalomeApp_IntSpinBox*>( editor )->value(), Qt::EditRole );
  model->setData( index, qobject_cast<SalomeApp_IntSpinBox*>( editor )->value(), Qt::UserRole );
}

//================================================================================
/*!
 *  Constructor
 */
//================================================================================

StdMeshersGUI_FixedPointsParamWdg
::StdMeshersGUI_FixedPointsParamWdg( QWidget * parent ): 
  QWidget( parent )
{
  QGridLayout* edgesLayout = new QGridLayout( this );
  edgesLayout->setMargin( MARGIN );
  edgesLayout->setSpacing( SPACING );
  
  myListWidget   = new QListWidget( this );
  myTreeWidget   = new QTreeWidget( this );
  mySpinBox      = new SMESHGUI_SpinBox( this );
  myAddButton    = new QPushButton( tr( "SMESH_BUT_ADD" ),    this );
  myRemoveButton = new QPushButton( tr( "SMESH_BUT_REMOVE" ), this );      
  mySameValues   = new QCheckBox( tr("SMESH_SAME_NB_SEGMENTS"), this);

  myListWidget->setSelectionMode( QListWidget::ExtendedSelection );

  myTreeWidget->setColumnCount(2);
  myTreeWidget->setHeaderLabels( QStringList() << tr( "SMESH_RANGE" ) << tr( "SMESH_NB_SEGMENTS" ) );
  myTreeWidget->setColumnWidth( 1, 40 );
  myTreeWidget->setColumnWidth( 2, 30 );
  myTreeWidget->setItemDelegate( new LineDelegate( myTreeWidget ) );

  edgesLayout->addWidget(myListWidget,   0, 0, 4, 1);
  edgesLayout->addWidget(mySpinBox,      0, 1);
  edgesLayout->addWidget(myAddButton,    1, 1);
  edgesLayout->addWidget(myRemoveButton, 2, 1);
  edgesLayout->addWidget(myTreeWidget,   0, 2, 4, 1);
  edgesLayout->addWidget(mySameValues,   4, 0, 1, 3);
  edgesLayout->setRowStretch( 3, 5 );
  edgesLayout->setColumnStretch(0, 1);
  edgesLayout->setColumnStretch(1, 0);
  edgesLayout->setColumnStretch(2, 2);

  myListWidget->setMinimumWidth( 80 );
  myTreeWidget->setMinimumWidth( 200 );

  mySpinBox->setAcceptNames( false ); // No Notebook variables allowed
  mySpinBox->RangeStepAndValidator( 0., 1., .1, "parametric_precision" );
  myListWidget->setMinimumWidth( 70 );

  connect( myAddButton,    SIGNAL( clicked() ),              SLOT( onAdd() ) );
  connect( myRemoveButton, SIGNAL( clicked() ),              SLOT( onRemove() ) );
  connect( mySameValues,   SIGNAL( stateChanged( int ) ),    SLOT( onCheckBoxChanged() ) );
  connect( mySpinBox,      SIGNAL( valueChanged( double ) ), SLOT( updateState() ) );
  connect( myListWidget,   SIGNAL( itemSelectionChanged() ), SLOT( updateState() ) );
  myListWidget->installEventFilter( this );

  clear();
}

//================================================================================
/*!
 *  Destructor
 */
//================================================================================

StdMeshersGUI_FixedPointsParamWdg::~StdMeshersGUI_FixedPointsParamWdg()
{
}

//================================================================================
/*!
 *  Event filter
 */
//================================================================================
bool StdMeshersGUI_FixedPointsParamWdg::eventFilter( QObject* o, QEvent* e )
{
  if ( o == myListWidget && e->type() == QEvent::KeyPress ) {
    QKeyEvent* ke = (QKeyEvent*)e;
    if ( ke->key() == Qt::Key_Delete )
      removePoints();
  }
  return QWidget::eventFilter( o, e );
}

//================================================================================
/*!
 *  Clear widget
 */
//================================================================================
void StdMeshersGUI_FixedPointsParamWdg::clear()
{
  myTreeWidget->clear();
  myListWidget->clear();
  myTreeWidget->addTopLevelItem( newTreeItem( 0, 1 ) );
  mySpinBox->setValue( 0. );
  onCheckBoxChanged();
  updateState();
}

//=================================================================================
// function : onAdd()
// purpose  : Called when Add Button Clicked
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::onAdd()
{
  addPoint( mySpinBox->value() );
}
         
//=================================================================================
// function : onRemove()
// purpose  : Called when Remove Button Clicked
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::onRemove()
{
  removePoints();
}

//=================================================================================
// function : newTreeItem()
// purpose  : Called to create TreeItem
//=================================================================================

QTreeWidgetItem* StdMeshersGUI_FixedPointsParamWdg::newTreeItem( double v1, double v2 )
{
  QTreeWidgetItem* anItem = new QTreeWidgetItem();
  anItem->setText( 0, treeItemText( v1, v2 ) );
  anItem->setText( 1, QString::number( 1 ) );
  anItem->setData( 1, Qt::UserRole, 1 );
  return anItem;
}

//=================================================================================
// function : newListItem()
// purpose  : Called to create ListItem
//=================================================================================

QListWidgetItem* StdMeshersGUI_FixedPointsParamWdg::newListItem( double v )
{
  QListWidgetItem* anItem = new QListWidgetItem( QString::number( v ) );
  anItem->setData( Qt::UserRole, v );
  return anItem;
}

//=================================================================================
// function : itemText()
// purpose  : Called to convert Values to Text
//=================================================================================

QString StdMeshersGUI_FixedPointsParamWdg::treeItemText( double v1, double v2 )
{
  return QString( "%1 - %2" ).arg( v1 ).arg( v2 );
}

//=================================================================================
// function : addPoint()
// purpose  : Called to Add new Point
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::addPoint( double v)
{
  if ( GT_DBL(v, 0.0) && LT_DBL(v, 1.0)) {
    bool toInsert = true;
    int idx = myTreeWidget->topLevelItemCount()-1;
    for ( int i = 0 ; i < myListWidget->count(); i++ ) {
      double lv = point( i );
      if ( EQUAL_DBL(lv, v) ) { toInsert = false; break; }
      else if ( GT_DBL(lv, v) ) {
        idx = i; break;
      }
    }
    if ( toInsert ) {
      double v1 = idx == 0 ? 0 : point( idx-1 );
      double v2 = idx == myTreeWidget->topLevelItemCount()-1 ? 1 : point( idx );
      myTreeWidget->insertTopLevelItem( idx, newTreeItem( v1, v ) );
      myTreeWidget->topLevelItem( idx+1 )->setText( 0, treeItemText( v, v2 ) );
      myListWidget->insertItem( idx, newListItem( v ) );
      onCheckBoxChanged();
    }
  }
  updateState();
}

//=================================================================================
// function : removePoints()
// purpose  : Called to remove selected points
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::removePoints()
{
  QList<QListWidgetItem*> selItems = myListWidget->selectedItems();
  QListWidgetItem* item;
  foreach ( item, selItems ) {
    int idx = myListWidget->row( item );
    delete myTreeWidget->topLevelItem( idx );
    delete item;
    myTreeWidget->topLevelItem( idx )->setText( 0, treeItemText( idx == 0 ? 0 : point( idx-1 ),
                                                                 idx > myListWidget->count()-1 ? 1 : point( idx ) ) );
  }
  onCheckBoxChanged();
  updateState();
}

double StdMeshersGUI_FixedPointsParamWdg::point( int idx ) const
{
  return idx >= 0 && idx < myListWidget->count() ? myListWidget->item( idx )->data( Qt::UserRole ).toDouble() : 0.;
}

void StdMeshersGUI_FixedPointsParamWdg::setNbSegments( int idx, int val )
{
  if ( idx >= 0 && idx < myTreeWidget->topLevelItemCount() ) {
    myTreeWidget->topLevelItem( idx )->setData( 1, Qt::UserRole, val );
    myTreeWidget->topLevelItem( idx )->setText( 1, idx > 0 && mySameValues->isChecked() ? QString( SAME_TEXT ) : QString::number( val ) );
  }
}

int StdMeshersGUI_FixedPointsParamWdg::nbSegments( int idx ) const
{
  return idx >= 0 && idx < myTreeWidget->topLevelItemCount() ? myTreeWidget->topLevelItem( idx )->data( 1, Qt::UserRole ).toInt() : 1;
}

//=================================================================================
// function : onCheckBoxChanged()
// purpose  : Called when Check Box Clicked
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::onCheckBoxChanged()
{
  for ( int i = 0; i < myTreeWidget->topLevelItemCount(); i++ ) {
    QTreeWidgetItem* anItem = myTreeWidget->topLevelItem(i);
    setNbSegments( i, nbSegments( i ) );
    anItem->setFlags( mySameValues->isChecked() && i > 0 ? anItem->flags() & ~Qt::ItemIsEditable : anItem->flags() | Qt::ItemIsEditable );
  }
}

//=================================================================================
// function : updateState()
// purpose  : Update widgets state
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::updateState()
{
  double v = mySpinBox->value();
  myAddButton->setEnabled( GT_DBL(v, 0.0) && LT_DBL(v, 1.0) );
  myRemoveButton->setEnabled( myListWidget->selectedItems().count() > 0 );
}

//=================================================================================
// function : GetListOfPoints
// purpose  : Called to get the list of Edges IDs
//=================================================================================
SMESH::double_array_var StdMeshersGUI_FixedPointsParamWdg::GetListOfPoints()
{
  SMESH::double_array_var anArray = new SMESH::double_array;
  int size = myListWidget->count();
  anArray->length( size );
  for (int i = 0; i < size; i++) {
    anArray[i] = point(i);
  }
  return anArray;
}

//=================================================================================
// function : SetListOfPoints
// purpose  : Called to set the list of Points
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::SetListOfPoints( SMESH::double_array_var thePoints)
{
  clear();
  for ( CORBA::ULong i = 0; i < thePoints->length(); i++ ) {
    addPoint( thePoints[ i ] );
  }
}

//=================================================================================
// function : GetListOfSegments
// purpose  : Called to get the list Number of Segments
//=================================================================================
SMESH::long_array_var StdMeshersGUI_FixedPointsParamWdg::GetListOfSegments()
{
  SMESH::long_array_var anArray = new SMESH::long_array;
  int size = mySameValues->isChecked() ? 1 : myTreeWidget->topLevelItemCount();
  anArray->length( size );
  for (int i = 0; i < size; i++) {
    anArray[i] = nbSegments( i );
  }
  return anArray;
}

//=================================================================================
// function : SetListOfPoints
// purpose  : Called to set the list of Points
//=================================================================================
void StdMeshersGUI_FixedPointsParamWdg::SetListOfSegments( SMESH::long_array_var theSegments)
{
  if ( myListWidget->count() > 0 && theSegments->length() == 1)
    mySameValues->setChecked(true);
  for ( CORBA::ULong i = 0; i < theSegments->length(); i++ ) {
    setNbSegments( i, theSegments[i] );
  }
}
