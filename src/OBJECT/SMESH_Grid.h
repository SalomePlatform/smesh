//  SMESH OBJECT : interactive object for SMESH visualization
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
//  File   : SMESH_Grid.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESH_GRID_H
#define SMESH_GRID_H

// VTK Includes
#include <vtkUnstructuredGrid.h>

// Open CASCADE Includes
#include <TColStd_DataMapOfIntegerInteger.hxx>

class SMESH_Grid : public vtkUnstructuredGrid {

 public:
  vtkTypeMacro(SMESH_Grid, vtkUnstructuredGrid);

  static SMESH_Grid* New();

  void ClearNode();
  void ClearElement();

  void RemoveNode(int idSMESHDSnode);
  void RemoveElement(int idSMESHDSelement);

  void AddNode(int idSMESHDSnode, int idVTKnode);
  void AddElement(int idSMESHDSelement, int idVTKelement);

  int GetIdVTKNode(int idSMESHDSnode);
  int GetIdVTKElement(int idSMESHDSelement);

  int GetIdSMESHDSNode(int idVTKnode);
  int GetIdSMESHDSElement(int idVTKelement);

  void SetIdsVTKNode(const TColStd_DataMapOfIntegerInteger& mapVTK);
  void SetIdsSMESHDSNode(const TColStd_DataMapOfIntegerInteger& mapSMESHDS);

  void SetIdsVTKElement(const TColStd_DataMapOfIntegerInteger& mapVTK);
  void SetIdsSMESHDSElement(const TColStd_DataMapOfIntegerInteger& mapSMESHDS);

  virtual void DeepCopy (vtkDataObject *src);
  void CopyMaps(SMESH_Grid *src);

 protected:
  SMESH_Grid() : vtkUnstructuredGrid() {};

  TColStd_DataMapOfIntegerInteger myMapNodeVTKtoSMDS;
  TColStd_DataMapOfIntegerInteger myMapNodeSMDStoVTK;

  TColStd_DataMapOfIntegerInteger myMapElementVTKtoSMDS;
  TColStd_DataMapOfIntegerInteger myMapElementSMDStoVTK;
};

#endif
