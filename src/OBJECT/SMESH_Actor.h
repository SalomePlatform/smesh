//  File      : SMESH_Actor.h
//  Created   : Mon May 13 22:30:51 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$


#ifndef SMESH_ACTOR_H
#define SMESH_ACTOR_H

#include "SALOME_Actor.h"

// VTK Includes
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkUnstructuredGridReader.h>

// Open CASCADE Includes
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_DataMapOfIntegerInteger.hxx>

typedef struct rgbStruct
{
  float r;
  float g;
  float b;
} RGBStruct;

class SMESH_Actor : public SALOME_Actor {

 public:
  vtkTypeMacro(SMESH_Actor,SALOME_Actor);

  static SMESH_Actor* New();

  // Description:
  // This causes the actor to be rendered. It, in turn, will render the actor`s
  // property and then mapper.
  virtual void Render(vtkRenderer *, vtkMapper *);

  // Description:
  // This method is used internally by the rendering process.
  // We overide the superclass method to properly set the estimated render time.
  int RenderOpaqueGeometry(vtkViewport *viewport);

  void ShallowCopy(vtkProp *prop);

  void setReader(vtkUnstructuredGridReader* r) ;
  vtkUnstructuredGridReader* getReader();

  // Highlight
  virtual bool hasHighlight() { return true; }  

  vtkMapper* getMapper();

  void setDisplayMode(int);

  void SetColor(float r,float g,float b);
  void GetColor(float& r,float& g,float& b);
  void SetHighlightColor(float r,float g,float b);
  void GetHighlightColor(float& r,float& g,float& b);
  void SetPreselectedColor(float r,float g,float b);
  void GetPreselectedColor(float& r,float& g,float& b);

  void SetEdgeColor(float r,float g,float b);
  void GetEdgeColor(float& r,float& g,float& b);
  void SetEdgeHighlightColor(float r,float g,float b);
  void GetEdgeHighlightColor(float& r,float& g,float& b);
  void SetEdgePreselectedColor(float r,float g,float b);
  void GetEdgePreselectedColor(float& r,float& g,float& b);

  void SetNodeColor(float r,float g,float b);
  void GetNodeColor(float& r,float& g,float& b);
 
  void SetNodeSize(int size) ;
  int  GetNodeSize() ;


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

  vtkDataSet* DataSource;
  vtkActor*   EdgeDevice;
  vtkActor*   EdgeShrinkDevice;

  float GetShrinkFactor();
  void  SetShrinkFactor(float value );

  void GetChildActors(vtkActorCollection*);

  void SetVisibility(bool visibility);

 protected:

  SMESH_Actor();
  ~SMESH_Actor();
  SMESH_Actor(const SMESH_Actor&) {};
  void operator=(const SMESH_Actor&) {};

  vtkUnstructuredGridReader* myReader;

  float myShrinkFactor;

  RGBStruct edgeColor;
  RGBStruct edgeHighlightColor;
  RGBStruct edgePreselectedColor;

  RGBStruct actorColor;
  RGBStruct actorHighlightColor;
  RGBStruct actorPreselectedColor;

  RGBStruct actorNodeColor; // LPN
  int       actorNodeSize;  // LPN

};
#endif //SMESH_ACTOR_H
