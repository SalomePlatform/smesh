//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  CEA
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_IdValidator.h
//  Author : Edward AGAPOV
//  Module : SMESH
//  $Header: /dn05/salome/PAL/SMESH/SMESH_SRC/src/SMESHGUI/SMESHGUI_IdValidator.h

#ifndef SMESHGUI_IdValidator_HeaderFile
#define SMESHGUI_IdValidator_HeaderFile

#include "SMESH_SMESHGUI.hxx"

#include <qvalidator.h>

// validator for manual input of Ids

class SMESHGUI_EXPORT SMESHGUI_IdValidator: public QValidator
{
 public:

  SMESHGUI_IdValidator(QWidget * parent, const char * name = 0, const int maxNbId = 0):
    QValidator(parent,name), myMaxNbId(maxNbId) {}

  State validate ( QString & text, int & pos) const
  { 
    text.replace( QRegExp(" *[^0-9]+ *"), " " ); 
    if ( myMaxNbId && text.length() > myMaxNbId) { // truncate extra ids
      int ind = 0, nbId = 0;
      while ( ind < text.length() ) {
	if ( text.at( ind ) != ' ' ) {
	  if ( ++nbId > myMaxNbId ) {
	    text.truncate( ind );
	    break;
	  }
	  ind = text.find( ' ', ind );
	  if ( ind < 0 ) break;
	}
	ind++;
      }
    }
    if ( pos > text.length() )
      pos = text.length();
    return Acceptable;
  }

 private:
  int myMaxNbId;
};

#endif
