// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : StdMeshersGUI_BlockRenumberCreator.cxx
// Author : Open CASCADE S.A.S.

#include "StdMeshersGUI_BlockRenumberCreator.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_HypothesesUtils.h"

#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include <GEOMBase.h>
#include <LightApp_SelectionMgr.h>
#include <SALOMEDSClient_SObject.hxx>
#include <SALOMEDSClient_Study.hxx>
#include <SALOME_InteractiveObject.hxx>
#include <SALOME_ListIO.hxx>
#include <SUIT_ResourceMgr.h>
#include <SalomeApp_Tools.h>

#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#define SPACING 6
#define MARGIN  11

namespace {

  //================================================================================
  /*!
   * \brief Textual representation of GEOM_Object in QTreeWidget
   */
  //================================================================================

  QString toText( const GEOM::GEOM_Object_var& theGO,
                  const QString &              /*theLabel*/,
                  QString&                     theEntry )
  {
    QString txt;
    if ( !theGO->_is_nil() )
    {
      CORBA::String_var  name = theGO->GetName();
      CORBA::String_var entry = theGO->GetStudyEntry();
      theEntry = entry.in();

      txt = name.in();
      //txt = theLabel + name.in() + " [" + theEntry + "]";
      //txt += " ";//" (";
      //txt += entry.in();
      //txt += ")";
    }
    return txt;
  }

  //================================================================================
  /*!
   * \brief Find GEOM_Object by study entry
   */
  //================================================================================

  GEOM::GEOM_Object_var toGeom( const QString& entry )
  {
    return SMESH::EntryToInterface<GEOM::GEOM_Object>( entry );
  }


  //================================================================================
  /*!
   * \brief Find GEOM_Object in the tree
   */
  //================================================================================

  QTreeWidgetItem* findSolidInTree( const GEOM::GEOM_Object_var& go, QTreeWidget* tree )
  {
    if ( go->_is_nil() || !tree )
      return nullptr;

    for ( int i = 0; i < tree->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem* item = tree->topLevelItem( i );
      if ( item->data( 1, Qt::UserRole ).toString() == SMESH::toQStr( go->GetStudyEntry() ))
        return item;
    }
    return nullptr;
  }
}

//================================================================================
/*!
 * \brief StdMeshersGUI_BlockRenumberCreator constructor
 */
//================================================================================

StdMeshersGUI_BlockRenumberCreator::StdMeshersGUI_BlockRenumberCreator(const QString& aHypType)
  : StdMeshersGUI_StdHypothesisCreator( aHypType ),
    mySolidFilter( "", TopAbs_SOLID, 1, TopAbs_SOLID ),
    myVertexFilter( "", TopAbs_VERTEX, 1, TopAbs_VERTEX )
{
}

//================================================================================
/*!
 * \brief StdMeshersGUI_BlockRenumberCreator destructor
 */
//================================================================================

StdMeshersGUI_BlockRenumberCreator::~StdMeshersGUI_BlockRenumberCreator()
{
  SMESHGUI::selectionMgr()->clearFilters();
}

//================================================================================
/*!
 * \brief Create widgets
 */
//================================================================================

QFrame* StdMeshersGUI_BlockRenumberCreator::buildFrame()
{
  QFrame* fr = new QFrame();
  //fr->setMinimumWidth(460);

  QGridLayout* frLayout = new QGridLayout( fr );
  frLayout->setSpacing( SPACING );
  frLayout->setMargin( MARGIN );

  // name
  myName = 0;
  int row = 0;
  if( isCreation() )
  {
    myName = new QLineEdit( fr );
    frLayout->addWidget( new QLabel( tr( "SMESH_NAME" ), fr ), row, 0 );
    frLayout->addWidget( myName,                               row, 1 );
    row++;
  }

  QGroupBox* groupBox = new QGroupBox( tr( "BLOCK_CS_GROUPBOX" ), fr );
  frLayout->addWidget( groupBox, row, 0, 1, 2 );

  QGridLayout* layout = new QGridLayout( groupBox );
  layout->setSpacing( SPACING );
  layout->setMargin( MARGIN );
  layout->setColumnStretch( 0, 1 );

  // tree
  row = 0;
  myBlockTree = new QTreeWidget( groupBox );
  myBlockTree->setColumnCount( 2 );
  myBlockTree->header()->hide();
  // myBlockTree->setHeaderLabels( QStringList()
  //                               << tr("COLUMN_BLOCK") << tr("COLUMN_V000") << tr("COLUMN_V001"));
  layout->addWidget( myBlockTree, row, 0, row + 7, 1 );

  // selection widgets
  myButGroup = new QButtonGroup( groupBox );
  myButGroup->setExclusive( true );
  QLabel* label[3] = { new QLabel( tr( "SOLID" ), groupBox ),
                       new QLabel( tr( "V000" ),  groupBox ),
                       new QLabel( tr( "V001" ),  groupBox ) };
  QIcon icon( SMESHGUI::resourceMgr()->loadPixmap("SMESH", tr("ICON_SELECT")));
  for ( int i = 0; i < 3; ++i )
  {
    myShapeSelectBut[i] = new QPushButton( icon, "", groupBox );
    myLineEdit      [i] = new QLineEdit( groupBox );
    layout->addWidget( label[i],            row, 1 );
    layout->addWidget( myShapeSelectBut[i], row, 2 );
    layout->addWidget( myLineEdit[i],       row, 3 );
    myShapeSelectBut[i]->setCheckable( true );
    myButGroup->addButton( myShapeSelectBut[i], i );
    row++;
  }

  // buttons
  myAddBut    = new QPushButton( tr("ADD"),    groupBox );
  myModifBut  = new QPushButton( tr("MODIFY"),       groupBox );
  myRemoveBut = new QPushButton( tr("SMESH_REMOVE"), groupBox );
  layout->addWidget( myAddBut, row, 1, 1, 3 );
  row++;
  layout->addWidget( myModifBut, row, 1, 1, 3 );
  row++;
  layout->addWidget( myRemoveBut, row, 1, 1, 3 );
  row++;
  layout->setRowStretch( row, 1 );

  LightApp_SelectionMgr* selMgr = SMESH::GetSelectionMgr( SMESHGUI::GetSMESHGUI() );

  connect( selMgr,              SIGNAL( currentSelectionChanged()), SLOT( onSelectionChange()));
  connect( myBlockTree,         SIGNAL( itemSelectionChanged() ),   SLOT( onTreeSelectionChange()));
  connect( myShapeSelectBut[0], SIGNAL( clicked(bool)),             SLOT( onSelectBtnClick() ));
  connect( myShapeSelectBut[1], SIGNAL( clicked(bool)),             SLOT( onSelectBtnClick() ));
  connect( myShapeSelectBut[2], SIGNAL( clicked(bool)),             SLOT( onSelectBtnClick() ));
  connect( myAddBut,            SIGNAL( clicked() ),                SLOT( onAddBtnClick() ));
  connect( myModifBut,          SIGNAL( clicked() ),                SLOT( onModifBtnClick() ));
  connect( myRemoveBut,         SIGNAL( clicked() ),                SLOT( onRemoveBtnClick() ));

  return fr;
}

//================================================================================
/*!
 * \brief Set myGO's to the tree
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::setBlockToTree(QTreeWidgetItem* solidItem)
{
  if ( !myGO[0]->_is_nil() && !myGO[1]->_is_nil() && !myGO[2]->_is_nil() )
  {
    if ( !solidItem ) solidItem = new QTreeWidgetItem( myBlockTree );
    solidItem->setExpanded( true );
    QTreeWidgetItem* item = solidItem;

    QFont boldFont = item->font( 0 );
    boldFont.setBold( true );

    QString entry, label[3] = { tr("BLOCK_LABEL"), tr("V000_LABEL"), tr("V001_LABEL") };
    for ( int i = 0; i < 3; ++i )
    {
      if ( i > 0 && ! (item = solidItem->child( i - 1 )))
        item = new QTreeWidgetItem( solidItem );

      item->setText( 0, label[i] );
      item->setText( 1, toText( myGO[i], label[i], entry ));
      item->setData( 1, Qt::UserRole, entry );
      item->setFont( 1, boldFont );
      item->setToolTip( 1, entry );
    }
    myBlockTree->resizeColumnToContents( 0 );
    myBlockTree->resizeColumnToContents( 1 );
  }
  for ( int i = 0; i < 3; ++i )
  {
    myGO[i] = GEOM::GEOM_Object::_nil();
    myLineEdit[i]->setText("");
  }
}

//================================================================================
/*!
 * \brief Transfer parameters from hypothesis to widgets
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::retrieveParams() const
{
  StdMeshersGUI_BlockRenumberCreator* me = const_cast<StdMeshersGUI_BlockRenumberCreator*>( this );

  StdMeshers::StdMeshers_BlockRenumber_var h =
    StdMeshers::StdMeshers_BlockRenumber::_narrow( initParamsHypothesis() );

  if( myName )
    me->myName->setText( hypName() );

  me->myBlockTree->clear();

  StdMeshers::blockcs_array_var blkArray = h->GetBlocksOrientation();
  for ( CORBA::ULong i = 0; i < blkArray->length(); ++i )
  {
    me->myGO[0] = GEOM::GEOM_Object::_duplicate( blkArray[i].solid.in()     );
    me->myGO[1] = GEOM::GEOM_Object::_duplicate( blkArray[i].vertex000.in() );
    me->myGO[2] = GEOM::GEOM_Object::_duplicate( blkArray[i].vertex001.in() );
    me->setBlockToTree();
  }

  me->myShapeSelectBut[0]->click();

  me->updateButtons();
}

//================================================================================
/*!
 * \brief Transfer parameters from widgets to hypothesis
 */
//================================================================================

QString StdMeshersGUI_BlockRenumberCreator::storeParams() const
{
  StdMeshers::StdMeshers_BlockRenumber_var h =
    StdMeshers::StdMeshers_BlockRenumber::_narrow( hypothesis() );

  try
  {
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), myName->text().toUtf8().constData() );

    StdMeshers::blockcs_array_var array = new StdMeshers::blockcs_array();
    array->length( myBlockTree->topLevelItemCount() );

    for ( int i = 0; i < myBlockTree->topLevelItemCount(); ++i )
    {
      StdMeshers::BlockCS& bcs = array[i];
      QTreeWidgetItem*   item0 = myBlockTree->topLevelItem( i );
      QTreeWidgetItem*   item1 = item0->child( 0 );
      QTreeWidgetItem*   item2 = item0->child( 1 );

      bcs.solid     = toGeom( item0->data( 1, Qt::UserRole ).toString() )._retn();
      bcs.vertex000 = toGeom( item1->data( 1, Qt::UserRole ).toString() )._retn();
      bcs.vertex001 = toGeom( item2->data( 1, Qt::UserRole ).toString() )._retn();
    }
    h->SetBlocksOrientation( array );

  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
  }
  return "";
}

//================================================================================
/*!
 * \brief Take selected object
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::onSelectionChange()
{
  SALOME_ListIO list;
  SMESHGUI::GetSMESHGUI()->selectionMgr()->selectedObjects( list );

  int shapeID = myButGroup->checkedId();
  if ( shapeID < 0 || shapeID > 2 )
    shapeID = 0;

  myGO[ shapeID ] = GEOM::GEOM_Object::_nil();
  myLineEdit[ shapeID ]->clear();

  if ( list.IsEmpty() )
    return;

  Handle(SALOME_InteractiveObject) io = list.First();
  if ( !io->hasEntry() )
    return;

  myGO[ shapeID ] = SMESH::IObjectToInterface<GEOM::GEOM_Object>( io );

  if ( !myGO[ shapeID ]->_is_nil() )
    myLineEdit[ shapeID ]->setText( SMESH::toQStr( myGO[ shapeID ]->GetName() ));

  updateButtons();
}

//================================================================================
/*!
 * \brief Display selected block CS in myLineEdit's
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::onTreeSelectionChange()
{
  QList<QTreeWidgetItem *> items = myBlockTree->selectedItems();
  for ( QTreeWidgetItem* item : items )
  {
    if ( item->parent() )
      item = item->parent();

    QTreeWidgetItem* items[3] = { item, item->child( 0 ), item->child( 1 ) };
    if ( items[1] && items[2] )
      for ( int i = 0; i < 3; ++i )
      {
        myGO[i] = toGeom( items[i]->data( 1, Qt::UserRole ).toString() );
        myLineEdit[i]->setText( items[i]->text( 1 ));
      }

    break;
  }

  updateButtons();
}

//================================================================================
/*!
 * \brief Activate/deactivate buttons
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::updateButtons()
{
  bool isSolidInTree = findSolidInTree( myGO[0], myBlockTree );
  myAddBut   ->setEnabled( !isSolidInTree &&
                           !myGO[0]->_is_nil() && !myGO[1]->_is_nil() && !myGO[2]->_is_nil() );
  myModifBut ->setEnabled( isSolidInTree );
  myRemoveBut->setEnabled( !myBlockTree->selectedItems().isEmpty() ||
                           myBlockTree->topLevelItemCount() == 1 );
}

//================================================================================
/*!
 * \brief Install filter upon activation of another object selection
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::onSelectBtnClick()
{
  int shapeID = myButGroup->checkedId();
  LightApp_SelectionMgr* selMgr = SMESHGUI::selectionMgr();
  selMgr->clearFilters();
  selMgr->installFilter( shapeID > 0 ? &myVertexFilter : &mySolidFilter );
}

//================================================================================
/*!
 * \brief Add shapes to the tree
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::onAddBtnClick()
{
  setBlockToTree();
  updateButtons();
}

//================================================================================
/*!
 * \brief Modify a current block
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::onModifBtnClick()
{
  if ( QTreeWidgetItem* item = findSolidInTree( myGO[0], myBlockTree ))
  {
    setBlockToTree( item );
  }
  updateButtons();
}

//================================================================================
/*!
 * \brief Remove selected block from the tree
 */
//================================================================================

void StdMeshersGUI_BlockRenumberCreator::onRemoveBtnClick()
{
  QList<QTreeWidgetItem *> items = myBlockTree->selectedItems();
  if ( items.isEmpty() && myBlockTree->topLevelItemCount() == 1 )
    items.push_back( myBlockTree->topLevelItem( 0 ));

  for ( QTreeWidgetItem* item : items )
  {
    if ( item->parent() )
      item = item->parent();
    delete item;
  }

  updateButtons();
}


//================================================================================
/*!
 * \brief Validate parameters
 */
//================================================================================

bool StdMeshersGUI_BlockRenumberCreator::checkParams( QString& /*msg*/ ) const
{
  return true;
}

//================================================================================
/*!
 * \brief Returns a name of help page
 */
//================================================================================

QString StdMeshersGUI_BlockRenumberCreator::helpPage() const
{
  return "3d_meshing_hypo.html#renumber-hypothesis";
}

//================================================================================
/*!
 * \brief Type name
 */
//================================================================================

QString  StdMeshersGUI_BlockRenumberCreator::hypTypeName( const QString& ) const
{
  return "BLOCK_RENUMBER";
}
