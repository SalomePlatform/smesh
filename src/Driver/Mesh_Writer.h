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
//  File   : Mesh_Writer.h
//  Module : SMESH

#ifndef _INCLUDE_MESH_WRITER
#define _INCLUDE_MESH_WRITER

#include <string>
#include "Handle_SMDS_Mesh.hxx"

class Mesh_Writer {

  public :
    virtual void Add() =0;
    virtual void Write() =0;
    virtual void SetMesh(Handle(SMDS_Mesh)&) =0;
    virtual void SetFile(string) =0;
    virtual void SetMeshId(int) =0;

};
#endif
