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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_FileInfoDlg.cxx
// Author : Alexandre SOLOVYOV, Open CASCADE S.A.S. (alexander.solovyov@opencascade.com)
//
#include "SMESHGUI_FileInfoDlg.h"

#include "MED_Common.hxx"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

SMESHGUI_FileInfoDlg::SMESHGUI_FileInfoDlg( QWidget* parent, SMESH::MedFileInfo* inf )
  : QtxDialog( parent, true, true, QtxDialog::OK )
{
  setWindowTitle( tr( "CAPTION" ) );
  setSizeGripEnabled( true );
  
  QLineEdit* fname = new QLineEdit( mainFrame() );
  fname->setReadOnly( true );
  QLineEdit* fsize = new QLineEdit( mainFrame() );
  fsize->setReadOnly( true );
  QLineEdit* medversion = new QLineEdit( mainFrame() );
  medversion->setReadOnly( true );
  fname->setMinimumWidth( 300 );
  
  QGridLayout* lay = new QGridLayout( mainFrame() );
  lay->setMargin( 5 ); lay->setSpacing( 5 );
  lay->addWidget( new QLabel( tr( "FILE_NAME" ), mainFrame() ), 0, 0 );
  lay->addWidget( fname, 0, 1 );
  lay->addWidget( new QLabel( tr( "FILE_SIZE" ), mainFrame() ), 1, 0 );
  lay->addWidget( fsize, 1, 1 );
  lay->addWidget( new QLabel( tr( "MED_VERSION" ), mainFrame() ), 2, 0 );
  lay->addWidget( medversion, 2, 1 );

  fname->setText( (char*)inf->fileName );
  fname->home( false );
  fsize->setText( QString::number( inf->fileSize ) );

  QString version;
  if( inf->major>=0 )
  {
    version = QString::number( inf->major );
    if( inf->minor>=0 )
    {
      version += "." + QString::number( inf->minor );
      if( inf->release>=0 )
        version += "." + QString::number( inf->release );
    }
  }
  medversion->setText( version );
}

SMESHGUI_FileInfoDlg::~SMESHGUI_FileInfoDlg()
{
}
