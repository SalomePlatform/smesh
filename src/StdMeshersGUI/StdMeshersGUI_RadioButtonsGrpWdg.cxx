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
#include "StdMeshersGUI_RadioButtonsGrpWdg.h"

#include "SMESHGUI.h"

#include <SUIT_ResourceMgr.h>

#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStringList>

#define SPACING 6
#define MARGIN  11

//================================================================================
/*!
 * \brief Creates a QGroupBox with a given title
 */
//================================================================================

StdMeshersGUI_RadioButtonsGrpWdg::StdMeshersGUI_RadioButtonsGrpWdg( const QString& title )
  : QGroupBox( title )
{
  myButtonGrp = new QButtonGroup( this );
}

//================================================================================
/*!
 * \brief Creates a given nubmer of button labels with given labels (QString's)
 */
//================================================================================

void StdMeshersGUI_RadioButtonsGrpWdg::setButtonLabels( const QStringList& buttonLabels,
                                                        const QStringList& buttonIcons )
{
  QGridLayout* layout = new QGridLayout( this );
  layout->setSpacing(SPACING);
  layout->setMargin(MARGIN);

  for ( int id = 0; id < buttonLabels.size(); ++id )
  {
    QRadioButton* button = new QRadioButton( buttonLabels.at(id), this );
    layout->addWidget( button, id, 0 );
    myButtonGrp->addButton( button, id );

    if ( id < buttonIcons.count() )
    {
      QPixmap pmi (SMESHGUI::resourceMgr()->loadPixmap("SMESH", buttonIcons.at(id)));
      if ( !pmi.isNull() ) {
        QLabel* pixLabel = new QLabel( this );
        pixLabel->setPixmap( pmi );
        layout->addWidget( pixLabel, id, 1 );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Set checked a radio button with a give id.
 */
//================================================================================

void StdMeshersGUI_RadioButtonsGrpWdg::setChecked(int id)
{
  if ( QAbstractButton* but = myButtonGrp->button( id ))
    but->setChecked( true );
}

//================================================================================
/*!
 * \brief Return id (zero based) of a checked radio button
 */
//================================================================================

int StdMeshersGUI_RadioButtonsGrpWdg::checkedId () const
{
  return myButtonGrp->checkedId();
}
