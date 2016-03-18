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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_IdValidator.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_IDVALIDATOR_H
#define SMESHGUI_IDVALIDATOR_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QValidator>

// validator for manual input of Ids

class SMESHGUI_EXPORT SMESHGUI_IdValidator : public QValidator
{
public:
  SMESHGUI_IdValidator( QWidget* parent, const int maxNbId = 0 ) :
    QValidator( parent ), myMaxNbId( maxNbId ) {}

  State validate( QString& input, int& pos ) const
  { 
    input.replace( QRegExp(" *[^0-9]+ *"), " " ); 
    if ( myMaxNbId && input.length() > myMaxNbId ) {
      // truncate extra ids
      int ind = 0, nbId = 0;
      while ( ind < input.length() ) {
        if ( input.at( ind ) != ' ' ) {
          if ( ++nbId > myMaxNbId ) {
            input.truncate( ind );
            break;
          }
          ind = input.indexOf( ' ', ind );
          if ( ind < 0 ) break;
        }
        ind++;
      }
    }
    if ( pos > input.length() )
      pos = input.length();
    return Acceptable;
  }

private:
  int myMaxNbId;
};

#endif // SMESHGUI_IDVALIDATOR_H
