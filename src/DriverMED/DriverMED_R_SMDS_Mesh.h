//  SMESH DriverMED : driver to read and write 'med' files
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
//  File   : DriverMED_R_SMDS_Mesh.h
//  Module : SMESH

#ifndef _INCLUDE_DRIVERMED_R_SMDS_MESH
#define _INCLUDE_DRIVERMED_R_SMDS_MESH

#include "SMDS_Mesh.hxx"
#include "Mesh_Reader.h"

extern "C"
{
#include <med.h>
}

class DriverMED_R_SMDS_Mesh : public Mesh_Reader {

  public :
    DriverMED_R_SMDS_Mesh();
    ~DriverMED_R_SMDS_Mesh();

    void Add();
    void Read();
    void SetMesh(Handle(SMDS_Mesh)& aMesh);
    void SetFile(string);

    void SetFileId(med_idt);
    void SetMeshId(int);

private :
    Handle_SMDS_Mesh myMesh;
    string myFile; 
    med_idt myFileId;
    int myMeshId;

};
#endif
