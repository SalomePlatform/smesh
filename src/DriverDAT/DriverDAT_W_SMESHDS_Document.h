//  SMESH DriverDAT : driver to read and write 'dat' files
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : DriverDAT_W_SMESHDS_Document.h
//  Module : SMESH

#ifndef _INCLUDE_DRIVERDAT_W_SMESHDS_DOCUMENT
#define _INCLUDE_DRIVERDAT_W_SMESHDS_DOCUMENT

#include <stdio.h>
#include <string>

#include "SMESHDS_Document.hxx"
#include "Document_Writer.h"

class DriverDAT_W_SMESHDS_Document : public Document_Writer {
  
public :
  DriverDAT_W_SMESHDS_Document();
  ~DriverDAT_W_SMESHDS_Document();
 
  void Write();
  //void SetFile(string);
  //void SetDocument(Handle(SMESHDS_Document)&);

private :
  //Handle_SMESHDS_Document myDocument;
  //string myFile; 

};
#endif
