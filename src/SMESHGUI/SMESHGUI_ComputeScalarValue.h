//  File      : SMESHGUI_ComputeScalarValue.h
//  Created   : Mon Jun 24 14:06:00 2002
//  Author    : Nicolas REJNERI

//  Project   : SALOME
//  Module    : SMESH
//  Copyright : Open CASCADE 2002
//  $Header$

#ifndef SMESHGUI_CONTROLALGORITHMS_H
#define SMESHGUI_CONTROLALGORITHMS_H

#include <vtkCell.h> 

class SMESHGUI_ComputeScalarValue {
 public:
  static double LengthEdges(vtkCell* theCell);
  static double AreaElements(vtkCell* theCell);
  static double Taper(vtkCell* theCell);
  static double AspectRatio(vtkCell* theCell);
  static double MinimumAngle(vtkCell* theCell);
  static double Skew(vtkCell* theCell);
  static double Warp(vtkCell* theCell);
};

#endif
