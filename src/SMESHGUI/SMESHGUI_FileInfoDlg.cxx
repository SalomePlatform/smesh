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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : SMESHGUI_FileInfoDlg.cxx
//  Author : Alexandre SOLOVYOV
//  Module : SMESH
//  $Header$

#include <SMESHGUI_FileInfoDlg.h>

#include <MED_Common.hxx>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>

SMESHGUI_FileInfoDlg::SMESHGUI_FileInfoDlg( QWidget* parent, SALOME_MED::MedFileInfo* inf )
  : QtxDialog( parent, 0, true, false, QtxDialog::OK )
{
  setCaption( tr( "CAPTION" ) );
  
  QLineEdit* fname = new QLineEdit( mainFrame() );
  fname->setReadOnly( true );
  QLineEdit* fsize = new QLineEdit( mainFrame() );
  fsize->setReadOnly( true );
  QLineEdit* medversion = new QLineEdit( mainFrame() );
  medversion->setReadOnly( true );
  
  QGridLayout* lay = new QGridLayout( mainFrame(), 4, 2, 5, 5 );
  lay->addWidget( new QLabel( tr( "FILE_NAME" ), mainFrame() ), 0, 0 );
  lay->addWidget( fname, 0, 1 );
  lay->addWidget( new QLabel( tr( "FILE_SIZE" ), mainFrame() ), 1, 0 );
  lay->addWidget( fsize, 1, 1 );
  lay->addWidget( new QLabel( tr( "MED_VERSION" ), mainFrame() ), 2, 0 );
  lay->addWidget( medversion, 2, 1 );

  fname->setText( (char*)inf->fileName );
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
  setFixedSize( 640, 480 );
}

SMESHGUI_FileInfoDlg::~SMESHGUI_FileInfoDlg()
{
}
