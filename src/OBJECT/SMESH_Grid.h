//  File      : SMESH_Grid.h
//  Created   : Fri Sep 27 15:30:51 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
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
