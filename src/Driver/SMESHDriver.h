//  SMESH Driver : implementaion of driver for reading and writing	
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
//  File   : SMESHDriver.h
//  Module : SMESH

#ifndef _INCLUDE_SMESHDRIVER
#define _INCLUDE_SMESHDRIVER

#include "Document_Reader.h"
#include "Document_Writer.h"
#include "Mesh_Reader.h"
#include "Mesh_Writer.h"

class SMESHDriver
{
  public :
    static Document_Reader* GetDocumentReader(string Extension);
    static Document_Writer* GetDocumentWriter(string Extension);

    static Mesh_Reader* GetMeshReader(string Extension);
    static Mesh_Writer* GetMeshWriter(string Extension);

  private:
	static void * getMeshDriver(string Extension, string type);
	static void * getMeshDocumentDriver(string Extension);
};
#endif
