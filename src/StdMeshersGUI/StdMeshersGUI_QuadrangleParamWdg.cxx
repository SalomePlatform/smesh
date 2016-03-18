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
//  File   : StdMeshersGUI_QuadrangleParamWdg.cxx
//  Author : Open CASCADE S.A.S. (jfa)
//  SMESH includes

#include "StdMeshersGUI_QuadrangleParamWdg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "StdMeshersGUI_SubShapeSelectorWdg.h"

#include <GEOMBase.h>
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_ResourceMgr.h>

// Qt includes
#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeWidget>
#include <QVBoxLayout>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_BasicHypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)


#define SPACING 6
#define MARGIN 11

enum { TAB_TRANSITION, TAB_VERTEX, TAB_ENF_POINTS };

//================================================================================
// function : Constructor
// purpose  :
//================================================================================

StdMeshersGUI_QuadrangleParamCreator::StdMeshersGUI_QuadrangleParamCreator(const QString& aHypType)
  : StdMeshersGUI_StdHypothesisCreator( aHypType )
{
}

//=======================================================================
//function : helpPage
//purpose  : 
//=======================================================================

QString  StdMeshersGUI_QuadrangleParamCreator::helpPage() const
{
  return "a2d_meshing_hypo_page.html#hypo_quad_params_anchor";
}

//=======================================================================
//function : buildFrame
//purpose  :
//=======================================================================

QFrame*  StdMeshersGUI_QuadrangleParamCreator::buildFrame()
{
  QFrame* fr = new QFrame();

  QGridLayout* lay = new QGridLayout( fr );
  lay->setMargin( MARGIN );
  lay->setSpacing( SPACING );
  int row = 0;

  myName = 0;
  if ( isCreation() )
  {
    myName = new QLineEdit( fr );
    QLabel* nameLab = new QLabel( tr("SMESH_NAME"));
    lay->addWidget( nameLab, row, 0 );
    lay->addWidget( myName,  row, 1 );
    ++row;
  }

  // Transition type

  myTypeWdg = new StdMeshersGUI_QuadrangleParamWdg( fr );

  // Vertexes

  myVertexSelWdg = new StdMeshersGUI_SubShapeSelectorWdg( fr, TopAbs_VERTEX );
  myVertexSelWdg->layout()->setMargin( MARGIN );

  // Enforced Points

  QWidget* pointsFrame = new QWidget( fr );
  QVBoxLayout* pointsLay = new QVBoxLayout( pointsFrame );
  pointsLay->setMargin(MARGIN);
  pointsLay->setSpacing(SPACING);

  // shapes
  QGroupBox* shapesGroup = new QGroupBox( tr("SHAPES"), pointsFrame );
  myShapesList = new QListWidget( shapesGroup );
  myAddShapeBut = new QPushButton( tr("SMESH_BUT_ADD"), shapesGroup );
  QPushButton* remShapeBut = new QPushButton( tr("SMESH_BUT_REMOVE"), shapesGroup );
  //
  QGridLayout* shapesLay = new QGridLayout( shapesGroup );
  shapesLay->setMargin(MARGIN);
  shapesLay->setSpacing(SPACING);
  shapesLay->addWidget( myShapesList,  0, 0, 3, 2 );
  shapesLay->addWidget( myAddShapeBut, 0, 2 );
  shapesLay->addWidget( remShapeBut,   1, 2 );
  shapesLay->setColumnStretch( 0, 1 );
  shapesLay->setRowStretch   ( 2, 1 );

  // coords
  QGroupBox* coordsGroup = new QGroupBox( tr("POINTS"), pointsFrame );
  myCoordsTreeWdg = new QTreeWidget( coordsGroup );
  myCoordsTreeWdg->setColumnCount ( 3 );
  myCoordsTreeWdg->setHeaderLabels( QStringList() << "X" << "Y" << "Z" );
  myCoordsTreeWdg->setItemDelegate( new ItemDelegate( myCoordsTreeWdg ));
  QPushButton* addCoordBut = new QPushButton( tr("SMESH_BUT_ADD"), coordsGroup );
  QPushButton* remCoordBut = new QPushButton( tr("SMESH_BUT_REMOVE"), coordsGroup );
  //
  QGridLayout* coordsLay = new QGridLayout( coordsGroup );
  coordsLay->setMargin(MARGIN);
  coordsLay->setSpacing(SPACING);
  coordsLay->addWidget( myCoordsTreeWdg, 0, 0, 3, 2 );
  coordsLay->addWidget( addCoordBut,     0, 2 );
  coordsLay->addWidget( remCoordBut,     1, 2 );
  coordsLay->setColumnStretch( 0, 1 );
  coordsLay->setRowStretch   ( 2, 1 );

  pointsLay->addWidget( shapesGroup );
  pointsLay->addWidget( coordsGroup );

  // Tabs
  myTabs = new QTabWidget( fr );
  myTabs->addTab( myTypeWdg,      tr("TRANSITION"));
  myTabs->addTab( myVertexSelWdg, tr("SMESH_BASE_VERTEX"));
  myTabs->addTab( pointsFrame,    tr("ENF_NODES"));

  lay->addWidget( myTabs, row, 0, 2, 3 );

  // signals
  connect( myTypeWdg,     SIGNAL( typeChanged(int)),   SLOT( onTypeChanged(int)));
  connect( myAddShapeBut, SIGNAL( clicked()),          SLOT( onAddShape() ));
  connect( remShapeBut,   SIGNAL( clicked()),          SLOT( onRemoveShape() ));
  connect( addCoordBut,   SIGNAL( clicked()),          SLOT( onAddPoint() ));
  connect( remCoordBut,   SIGNAL( clicked()),          SLOT( onRemovePoint() ));
  connect( myTabs,        SIGNAL( currentChanged(int)),SLOT( onTabChanged(int)));

  LightApp_SelectionMgr* selMgr = SMESHGUI::GetSMESHGUI()->selectionMgr();
  connect( selMgr, SIGNAL(currentSelectionChanged()), SLOT( onSelectionChanged()));

  return fr;
}

//=======================================================================
//function : retrieveParams
//purpose  :
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::retrieveParams() const
{
  StdMeshers::StdMeshers_QuadrangleParams_var h =
    StdMeshers::StdMeshers_QuadrangleParams::_narrow( initParamsHypothesis() );

  // name
  if( myName )
    myName->setText( hypName() );

  // main shape
  myVertexSelWdg->SetMaxSize(1);
  QString anEntry    = SMESHGUI_GenericHypothesisCreator::getShapeEntry();
  QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
  if ( anEntry.isEmpty() )
    anEntry = h->GetObjectEntry();
  myVertexSelWdg->SetGeomShapeEntry(anEntry,aMainEntry);

  if ( !isCreation())
  {
    // type
    myTypeWdg->SetType(int(h->GetQuadType()));

    // vertex
    int vertID = h->GetTriaVertex();
    if (vertID > 0) {
      SMESH::long_array_var aVec = new SMESH::long_array;
      aVec->length(1);
      aVec[0] = vertID;
      myVertexSelWdg->SetListOfIDs(aVec);
    }

    // enforced nodes
    GEOM::ListOfGO_var     shapes;
    SMESH::nodes_array_var points;
    h->GetEnforcedNodes( shapes, points );
    for ( size_t i = 0; i < shapes->length(); ++i )
    {
      CORBA::String_var name  = shapes[i]->GetName();
      CORBA::String_var entry = shapes[i]->GetStudyEntry();
      QListWidgetItem* item = new QListWidgetItem( name.in() );
      item->setData( Qt::UserRole, entry.in() );
      myShapesList->addItem( item );
    }
    for ( size_t i = 0; i < points->length(); ++i )
    {
      QTreeWidgetItem* item = new QTreeWidgetItem
        ( QStringList()
          << QString::number( points[i].x )
          << QString::number( points[i].y )
          << QString::number( points[i].z ));
      item->setFlags( item->flags() | Qt::ItemIsEditable );
      myCoordsTreeWdg->addTopLevelItem( item );
    }
  }
  ((StdMeshersGUI_QuadrangleParamCreator*) this)->onSelectionChanged();
}

//=======================================================================
//function : storeParams
//purpose  :
//=======================================================================

QString  StdMeshersGUI_QuadrangleParamCreator::storeParams() const
{
  StdMeshers::StdMeshers_QuadrangleParams_var h =
    StdMeshers::StdMeshers_QuadrangleParams::_narrow( hypothesis() );

  // name
  if( myName )
    SMESH::SetName( SMESH::FindSObject( h ), myName->text().toLatin1().constData() );

  // transition
  h->SetQuadType( StdMeshers::QuadType( myTypeWdg->GetType()) );

  // vertex
  if ( myVertexSelWdg->GetListSize() > 0 )
  {
    h->SetTriaVertex( myVertexSelWdg->GetListOfIDs()[0] ); // getlist must be called once
    h->SetObjectEntry( myVertexSelWdg->GetMainShapeEntry() );
  }
  else
  {
    h->SetTriaVertex( -1 );
  }

  // enfored nodes

  GEOM::ListOfGO_var goList = new GEOM::ListOfGO;
  int nbShapes = 0;
  goList->length( myShapesList->count() );
  for ( int i = 0; i < myShapesList->count(); ++i )
  {
    QListWidgetItem* item = myShapesList->item(i);
    QString         entry = item->data( Qt::UserRole ).toString();
    Handle(SALOME_InteractiveObject) io =
      new SALOME_InteractiveObject( entry.toStdString().c_str(), "GEOM" );
    GEOM::GEOM_Object_var go = GEOMBase::ConvertIOinGEOMObject( io );
    if ( !go->_is_nil() )
      goList[ nbShapes++ ] = go;
  }
  goList->length( nbShapes );
  
  SMESH::nodes_array_var points = new SMESH::nodes_array;
  points->length( myCoordsTreeWdg->topLevelItemCount() );
  for ( int i = 0; i < myCoordsTreeWdg->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem* item = myCoordsTreeWdg->topLevelItem( i );
    points[i].x = item->text(0).toInt();
    points[i].y = item->text(1).toInt();
    points[i].z = item->text(2).toInt();
  }
  h->SetEnforcedNodes( goList, points );

  return "";
}

//=======================================================================
//function : onTypeChanged
//purpose  :
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onTypeChanged(int type)
{
  myTabs->setTabEnabled( TAB_ENF_POINTS, ( type != StdMeshers::QUAD_REDUCED ));
}

//=======================================================================
//function : onAddShape
//purpose  : 
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onAddShape()
{
  if ( !mySelectedShapeIO.IsNull() )
  {
    QListWidgetItem* item = new QListWidgetItem( mySelectedShapeIO->getName() );
    item->setData( Qt::UserRole, mySelectedShapeIO->getEntry() );
    myShapesList->addItem( item );
    mySelectedShapeIO.Nullify();
    myAddShapeBut->setEnabled( false );
  }
}

//=======================================================================
//function : onRemoveShape
//purpose  : 
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onRemoveShape()
{
  if ( QListWidgetItem * item = myShapesList->currentItem() )
    delete item;
  onSelectionChanged();
}

//=======================================================================
//function : onAddPoint
//purpose  : 
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onAddPoint()
{
  QTreeWidgetItem* item = new QTreeWidgetItem( QStringList() << "0" << "0" << "0" );
  item->setFlags( item->flags() | Qt::ItemIsEditable );
  myCoordsTreeWdg->addTopLevelItem( item );
}

//=======================================================================
//function : onRemovePoint
//purpose  :
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onRemovePoint()
{
  if ( myCoordsTreeWdg->topLevelItemCount() )
    delete myCoordsTreeWdg->currentItem();
}

//=======================================================================
//function : onSelectionChanged
//purpose  : 
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onSelectionChanged()
{
  mySelectedShapeIO.Nullify();

  // find a sole selected geometry
  LightApp_SelectionMgr* selMgr = SMESHGUI::GetSMESHGUI()->selectionMgr();
  SALOME_ListIO          selList;
  selMgr->selectedObjects( selList );
  SALOME_ListIteratorOfListIO selIt( selList );
  for ( ; selIt.More(); selIt.Next() )
  {
    GEOM::GEOM_Object_var go = GEOMBase::ConvertIOinGEOMObject( selIt.Value() );
    if ( !go->_is_nil() )
    {
      if ( !mySelectedShapeIO.IsNull() )
      {
        mySelectedShapeIO.Nullify();
        break;
      }
      mySelectedShapeIO = selIt.Value();
      if ( !mySelectedShapeIO->getName() || !mySelectedShapeIO->getName()[0] )
        mySelectedShapeIO.Nullify();
    }
  }
  // check if a selected geometry is not already in myShapesList
  if ( !mySelectedShapeIO.IsNull() )
  {
    for ( int i = 0; i < myShapesList->count(); ++i )
      if ( myShapesList->item(i)->data( Qt::UserRole ) == mySelectedShapeIO->getEntry() )
      {
        mySelectedShapeIO.Nullify();
        break;
      }
  }
  myAddShapeBut->setEnabled( !mySelectedShapeIO.IsNull() );
}

//=======================================================================
//function : onTabChanged
//purpose  : 
//=======================================================================

void StdMeshersGUI_QuadrangleParamCreator::onTabChanged(int i)
{
  myVertexSelWdg->ShowPreview( i == TAB_VERTEX );
}

//================================================================================
// function : Constructor
// purpose  :
//================================================================================

StdMeshersGUI_QuadrangleParamWdg::StdMeshersGUI_QuadrangleParamWdg (QWidget * parent)
  : QWidget(parent), myType(0)
{
  myType = new QButtonGroup (this);

  QGridLayout* typeLay = new QGridLayout( this );

  typeLay->setMargin(MARGIN);
  typeLay->setSpacing(SPACING);

  QString aTypeKey ("SMESH_QUAD_TYPE_%1");
  QString aPictKey ("ICON_StdMeshers_Quadrangle_Params_%1");

  int itype = 0;
  for (; itype < int(StdMeshers::QUAD_NB_TYPES); itype++) {
    QRadioButton* rbi = new QRadioButton (tr(aTypeKey.arg(itype).toLatin1()), this);
    QPixmap pmi (SMESHGUI::resourceMgr()->loadPixmap("SMESH", tr(aPictKey.arg(itype).toLatin1())));
    QLabel* pli = new QLabel (this);
    pli->setPixmap(pmi);
    typeLay->addWidget(rbi, itype, 0, 1, 1);
    typeLay->addWidget(pli, itype, 1, 1, 1);
    myType->addButton(rbi, itype);
  }
  myType->button(0)->setChecked(true);

  setLayout(typeLay);
  setMinimumWidth(300);

  connect( myType, SIGNAL( buttonClicked(int)), this, SIGNAL( typeChanged(int)));
}

//================================================================================
// function : Destructor
// purpose  :
//================================================================================
StdMeshersGUI_QuadrangleParamWdg::~StdMeshersGUI_QuadrangleParamWdg()
{
}

//=================================================================================
// function : SetType
// purpose  :
//=================================================================================
void StdMeshersGUI_QuadrangleParamWdg::SetType (int theType)
{
  myType->button(theType)->setChecked(true);
}

//=================================================================================
// function : GetType
// purpose  :
//=================================================================================
int StdMeshersGUI_QuadrangleParamWdg::GetType()
{
  return myType->checkedId();
}

//================================================================================
/*!
  \brief Constructor
*/
StdMeshersGUI_QuadrangleParamCreator::
ItemDelegate::ItemDelegate( QObject* parent ) : QItemDelegate( parent )
{
}
//================================================================================
/*!
  \brief Create item editor widget
*/
QWidget* StdMeshersGUI_QuadrangleParamCreator::
ItemDelegate::createEditor( QWidget*                    parent,
                            const QStyleOptionViewItem& option,
                            const QModelIndex&          index ) const
{
  SMESHGUI_SpinBox* sb = new SMESHGUI_SpinBox( parent );
  sb->RangeStepAndValidator( COORD_MIN, COORD_MAX, 10 );
  return sb;
}
