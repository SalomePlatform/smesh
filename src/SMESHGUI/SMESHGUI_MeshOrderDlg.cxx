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

// File   : SMESHGUI_MeshOrderDlg.cxx
// Author : Pavel TELKOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshOrderDlg.h"

// Qt includes
#include <Qt>
#include <QFrame>
#include <QLabel>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QToolButton>
#include <QListWidget>
#include <QListWidgetItem>

#define SPACING 6
#define MARGIN  11

/*! 
 * Enumeartion of list widget item types (mesh name or separator)
 */
enum MeshOrderItemType { MeshItem = QListWidgetItem::UserType, SeparatorItem };

// =========================================================================================
/*!
 * \brief Constructor
 */
// =========================================================================================

SMESHGUI_MeshOrderBox::SMESHGUI_MeshOrderBox(QWidget* theParent)
: QGroupBox( theParent ), myIsChanged( false ), myUpBtn(0), myDownBtn(0)
{
  QHBoxLayout* hBoxLayout = new QHBoxLayout(this);
  hBoxLayout->setMargin( MARGIN );
  hBoxLayout->setSpacing( SPACING );
  
  myMeshNames = new QListWidget(this);
  myMeshNames->setSelectionMode(QAbstractItemView::SingleSelection);
  myMeshNames->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding ));
  hBoxLayout->addWidget(myMeshNames);
  
  QGroupBox* btnGrp = new QGroupBox(this);
  hBoxLayout->addWidget(btnGrp);

  myUpBtn   = new QToolButton(btnGrp);
  myDownBtn = new QToolButton(btnGrp);
  myUpBtn->  setArrowType( Qt::UpArrow );
  myDownBtn->setArrowType( Qt::DownArrow );
  
  QVBoxLayout* vBoxLayout = new QVBoxLayout(btnGrp);
  vBoxLayout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
  vBoxLayout->addWidget( myUpBtn );
  vBoxLayout->addWidget( myDownBtn );
  vBoxLayout->addSpacerItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

  connect( myUpBtn,   SIGNAL( clicked() ), this, SLOT( onMoveItem() ) );
  connect( myDownBtn, SIGNAL( clicked() ), this, SLOT( onMoveItem() ) );
  connect( myMeshNames, SIGNAL( itemSelectionChanged() ), this, SLOT( onSelectionChanged() ) );
  
  onSelectionChanged();
}

// =========================================================================================
/*!
 * \brief Destructor
 */
//=======================================================================

SMESHGUI_MeshOrderBox::~SMESHGUI_MeshOrderBox()
{
}

// =========================================================================================
/*!
 * \brief add separator item
 */
// =========================================================================================

static void addSeparator( QListWidget* theList )
{
  QListWidgetItem* item = new QListWidgetItem( theList, SeparatorItem );
  QFrame* hline = new QFrame( theList );
  hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  theList->addItem( item );
  theList->setItemWidget( item, hline );
}

// =========================================================================================
/*!
 * \brief add sub-mesh item
 */
// =========================================================================================

static void addMeshItem( QListWidget* theList,
                         const QString& theName,
                         const int      theId )
{
  QListWidgetItem* item = new QListWidgetItem( theName, theList, MeshItem );
  item->setData( Qt::UserRole, theId );
  theList->addItem( item );
}

// =========================================================================================
/*!
 * \brief Clear submesh names and indeces
 */
// =========================================================================================

void SMESHGUI_MeshOrderBox::Clear()
{
  myMeshNames->clear();
  myIsChanged = false;
}

// =========================================================================================
/*!
 * \brief Set submesh names and indeces
 */
// =========================================================================================

void SMESHGUI_MeshOrderBox::SetMeshes(const ListListName& theMeshNames,
                                      const ListListId&   theMeshIds)
{
  Clear();
  ListListName::const_iterator nLIt = theMeshNames.constBegin();
  ListListId::const_iterator idLIt  = theMeshIds.constBegin();
  for ( ; nLIt != theMeshNames.constEnd(); ++nLIt, ++idLIt )
  {
    const QStringList& names = (*nLIt);
    const QList<int>& ids = (*idLIt);
    if ( myMeshNames->count() )
      addSeparator( myMeshNames );
    QStringList::const_iterator nameIt = names.constBegin();
    QList<int>::const_iterator idIt = ids.constBegin();
    for ( ; nameIt != names.constEnd(); ++nameIt, ++idIt )
      addMeshItem( myMeshNames, *nameIt, *idIt );
  }
}

// =========================================================================================
/*!
 * \brief cehck that item exists and not a separator
 */
// =========================================================================================

static bool checkItem(QListWidgetItem* theItem)
{
  return theItem && (int)theItem->type() != (int)SeparatorItem;
}

// =========================================================================================
/*!
 * \brief Returns result (ordered by user) mesh names
 */
// =========================================================================================

ListListId SMESHGUI_MeshOrderBox::GetMeshIds() const
{
  ListListId aLLIds;
  aLLIds.append( QList<int>() );
  for ( int i = 0, n = myMeshNames->count(); i < n; i++ )
  {
    QListWidgetItem* it = myMeshNames->item( i );
    if (checkItem( it ))
      aLLIds.last().append( it->data( Qt::UserRole ).toInt() );
    else // separator before next list of mesh items
      aLLIds.append( QList<int>() );
  }
  return aLLIds;
}

// =========================================================================================
/*!
 * \brief Returns result (ordered by user) mesh indeces
 */
// =========================================================================================

ListListName SMESHGUI_MeshOrderBox::GetMeshNames() const
{
  ListListName aLLNames;
  aLLNames.append( QStringList() );
  for ( int i = 0, n = myMeshNames->count(); i < n; i++ )
  {
    QListWidgetItem* it = myMeshNames->item( i );
    if (checkItem( it ))
      aLLNames.last().append( it->text() );
    else // separator before next list of mesh items
      aLLNames.append( QStringList() );
  }
  return aLLNames;
}

// =========================================================================================
/*!
 * \brief update state of arrow buttons according to selection
 */
// =========================================================================================

void SMESHGUI_MeshOrderBox::onSelectionChanged()
{
  bool isUp = false;
  bool isDown = false;
  QList<QListWidgetItem *> items = myMeshNames->selectedItems();
  if ( !items.isEmpty() )
  {
    QListWidgetItem* selItem = (*(items.begin()));
    if (checkItem(selItem))
    {
      const int rowId = myMeshNames->row( selItem );
      isUp   = checkItem( myMeshNames->item( rowId - 1 ) );
      isDown = checkItem( myMeshNames->item( rowId + 1 ) );
    }
  }
  myUpBtn->  setEnabled( isUp );
  myDownBtn->setEnabled( isDown );
}

// =========================================================================================
/*!
 * \brief move item according to clicked arrow button
 */
// =========================================================================================

void SMESHGUI_MeshOrderBox::onMoveItem()
{
  moveItem( sender() == myUpBtn );
}

// =========================================================================================
/*!
 * \brief move mesh in order up or down
 */
// =========================================================================================

void SMESHGUI_MeshOrderBox::moveItem(const bool theIsUp)
{
  // move selected list item up or down
  QList<QListWidgetItem *> items = myMeshNames->selectedItems();
  if ( items.isEmpty() )
    return;
  QListWidgetItem * selItem = (*(items.begin()));
  if (!checkItem(selItem))
    return;
  int rowId = myMeshNames->row( selItem );
  if ( rowId == -1 )
    return;

  // move item in list widget
  myIsChanged = true;
  myMeshNames->takeItem( rowId );
  myMeshNames->insertItem(theIsUp ? rowId-1 : rowId+1, selItem );

  // restore selection and current status
  selItem->setSelected( true );
  myMeshNames->setCurrentItem( selItem );
}

// =========================================================================================
/*!
 * \brief returns status is order changed by user
 */
// =========================================================================================

bool SMESHGUI_MeshOrderBox:: IsOrderChanged() const
{
  return myIsChanged;
}

// =========================================================================================
/*!
 * \brief Constructor
 */
// =========================================================================================

SMESHGUI_MeshOrderDlg::SMESHGUI_MeshOrderDlg(QWidget* theParent)
: SMESHGUI_Dialog( theParent, true, false, OK | Cancel | Help )
{
  setWindowTitle( tr( "SMESH_MESHORDER_TITLE") );
  QFrame* main = mainFrame();

  QVBoxLayout* aDlgLay = new QVBoxLayout (main);
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );

  myBox = new SMESHGUI_MeshOrderBox( main );

  aDlgLay->addWidget(myBox);
  aDlgLay->setStretchFactor(main, 1);
}

// =========================================================================================
/*!
 * \brief Destructor
 */
// =========================================================================================

SMESHGUI_MeshOrderDlg::~SMESHGUI_MeshOrderDlg()
{
}

// =========================================================================================
/*!
 * \brief return Box widget to show mesh order
 */
// =========================================================================================

SMESHGUI_MeshOrderBox* SMESHGUI_MeshOrderDlg::GetMeshOrderBox() const
{
  return myBox;
}
