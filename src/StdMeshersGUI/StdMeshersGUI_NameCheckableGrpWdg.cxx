// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
#include "StdMeshersGUI_NameCheckableGrpWdg.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#define SPACING 6
#define MARGIN  11

//================================================================================
/*!
 * \brief Creates a QGroupBox with a given title
 */
//================================================================================

StdMeshersGUI_NameCheckableGrpWdg::StdMeshersGUI_NameCheckableGrpWdg( const QString& groupTitle,
                                                                      const QString& nameLabel )
  : QGroupBox( groupTitle ),
    myNameLineEdit( new QLineEdit( this ))
{
  setCheckable( true );

  QLabel* label = new QLabel( nameLabel );

  QGridLayout* layout = new QGridLayout( this );
  layout->setSpacing(SPACING);
  layout->setMargin(MARGIN);

  layout->addWidget( label,          0, 0 );
  layout->addWidget( myNameLineEdit, 0, 1 );

  connect( this, SIGNAL( toggled( bool )), myNameLineEdit, SLOT( setEnabled( bool )));
}

QString StdMeshersGUI_NameCheckableGrpWdg::getName()
{
  return isChecked() ? myNameLineEdit->text() : QString();
}

void StdMeshersGUI_NameCheckableGrpWdg::setName( CORBA::String_var name )
{
  myNameLineEdit->setText( name.in() );
  setChecked( ! myNameLineEdit->text().isEmpty() );
}

void StdMeshersGUI_NameCheckableGrpWdg::setDefaultName( QString name )
{
  myNameLineEdit->setText( name );
  setChecked( ! myNameLineEdit->text().isEmpty() );
}
