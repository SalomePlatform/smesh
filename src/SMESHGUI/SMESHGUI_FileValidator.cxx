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
// File   : SMESHGUI_FileValidator.cxx
// Author : Oleg UVAROV
// SMESH includes
//
#include "SMESHGUI_FileValidator.h"

// SALOME GUI includes
#include <SUIT_MessageBox.h>
#include <SUIT_Tools.h>

// Qt includes
#include <QFileInfo>

//=======================================================================
//function : SMESHGUI_FileValidator
//purpose  : 
//=======================================================================
SMESHGUI_FileValidator::SMESHGUI_FileValidator( QWidget* parent )
: SUIT_FileValidator( parent ),
  myIsOverwrite( true )
{
}
  
//=======================================================================
//function : canSave
//purpose  : 
//=======================================================================
bool SMESHGUI_FileValidator::canSave( const QString& fileName, bool checkPermission ) 
{
  if ( QFile::exists( fileName ) ) {
    if ( parent() ) {
      int anAnswer = SUIT_MessageBox::question( parent(), QObject::tr( "SMESH_WRN_WARNING" ),
                                                QObject::tr( "SMESH_FILE_EXISTS" ).arg( fileName ),
                                                QObject::tr( "SMESH_BUT_OVERWRITE" ),
                                                QObject::tr( "SMESH_BUT_ADD" ),
                                                QObject::tr( "SMESH_BUT_CANCEL" ), 0, 2 );
      if( anAnswer == 2 )
        return false;
      myIsOverwrite = anAnswer == 0;
    }

    // copied from SUIT_FileValidator
    if ( checkPermission && !QFileInfo( fileName ).isWritable() ) {
      if ( parent() ) 
        SUIT_MessageBox::critical( parent(), QObject::tr( "SMESH_ERROR" ),
                                   QObject::tr( "NO_PERMISSION" ).arg( fileName ) );
      return false; 
    }
  }
  else {
    return SUIT_FileValidator::canSave( fileName, checkPermission );
  }
  return true;
}
