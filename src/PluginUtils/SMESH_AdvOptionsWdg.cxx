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

// File   : SMESH_AdvOptionsWdg.h
// Author : Open CASCADE S.A.S.

#include "SMESH_AdvOptionsWdg.h"

#include <QTableWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QCheckBox>
#include <QHeaderView>

#define SPACING 6
#define MARGIN  11

namespace
{
  const int IS_CUSTOM = Qt::UserRole;
}

SMESH_AdvOptionsWdg::SMESH_AdvOptionsWdg( QWidget* parent )
  : QWidget( parent )
{
  myTable = new QTableWidget( /*nbrows=*/0, /*nbcol=*/3, this );
  QPushButton* addBtn = new QPushButton( tr("ADD_OPTION_BTN"), this );

  myTable->setHorizontalHeaderLabels
    ( QStringList() << tr("CHOICE") << tr("OPTION_NAME") << tr("OPTION_VALUE") );
  QHeaderView * header = myTable->horizontalHeader();
  header->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
  header->setSectionResizeMode( 1, QHeaderView::Stretch );
  header->setSectionResizeMode( 2, QHeaderView::ResizeToContents );

  QGridLayout* lay = new QGridLayout( this );
  lay->setMargin( MARGIN );
  lay->setSpacing( SPACING );
  lay->addWidget( myTable, 0,0, 1,3 );
  lay->addWidget( addBtn,  1,0 );

  connect( addBtn, SIGNAL( clicked() ), SLOT( onAdd() ));
}

SMESH_AdvOptionsWdg::~SMESH_AdvOptionsWdg()
{
}

void SMESH_AdvOptionsWdg::AddOption( QString name, QString value, bool isDefault, bool isCustom )
{
  int row = myTable->rowCount();
  myTable->insertRow( row );

  QTableWidgetItem*  nameItem = new QTableWidgetItem( name );
  QTableWidgetItem* valueItem = new QTableWidgetItem( value );
  if ( !name.isEmpty() )
    nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );
  myTable->setItem( row, 1, nameItem );
  myTable->setItem( row, 2, valueItem );
  nameItem->setData( IS_CUSTOM, isCustom );

  QCheckBox* chkBox = new QCheckBox();
  QWidget*      wdg = new QWidget();
  QHBoxLayout*  lay = new QHBoxLayout( wdg );
  lay->setContentsMargins(0,0,0,0);
  lay->addStretch();
  lay->addWidget(chkBox);
  lay->addStretch();
  myTable->setCellWidget( row, 0, wdg );
  connect( chkBox, SIGNAL(toggled(bool)), this, SLOT(onToggle()));
  myTable->setCurrentCell( row, 1, QItemSelectionModel::NoUpdate );
  chkBox->setChecked( !isDefault );

  if ( name.isEmpty() )
    myTable->editItem( nameItem );
}

void SMESH_AdvOptionsWdg::SetCustomOptions( const QString& text )
{
  QStringList nameVals = text.split(" ");
  for ( int i = 1; i < nameVals.count(); i += 2 )
    AddOption( nameVals[i-1], nameVals[i], false, true );
}

void SMESH_AdvOptionsWdg::onAdd()
{
  AddOption( "", "", false, true );
  
}
void SMESH_AdvOptionsWdg::onToggle()
{
  int row = myTable->currentRow();
  QTableWidgetItem* valueItem = myTable->item( row, 2 );

  bool isActive = isChecked( row );
  int c = isActive ? 0 : 150;
  valueItem->setForeground( QBrush( QColor( c, c, c )));
  if ( isActive )
    valueItem->setFlags( valueItem->flags() | Qt::ItemIsEditable );
  else
    valueItem->setFlags( valueItem->flags() & ~Qt::ItemIsEditable );
}

void SMESH_AdvOptionsWdg::GetOption( int      row,
                                     QString& name,
                                     QString& value,
                                     bool&    isDefault,
                                     bool &   isCustom)
{
  if ( row < myTable->rowCount() )
  {
    name      = myTable->item( row, 1 )->text();
    value     = myTable->item( row, 2 )->text();
    isDefault = !isChecked( row );
    isCustom  = myTable->item( row, 1 )->data( IS_CUSTOM ).toInt();
  }
}

QString SMESH_AdvOptionsWdg::GetCustomOptions()
{
  QString text, value, name;
  bool isDefault, isCustom;
  for ( int row = 0; row < myTable->rowCount(); ++row )
  {
    GetOption( row, name, value, isDefault, isCustom );
    if ( !name.isEmpty() && !value.isEmpty() && isCustom && !isDefault )
      text += name + " " + value + " ";
  }
  return text;
}

bool SMESH_AdvOptionsWdg::isChecked( int row )
{
  QCheckBox* cb = myTable->cellWidget( row, 0 )->findChild<QCheckBox *>();
  return cb->isChecked();
}
