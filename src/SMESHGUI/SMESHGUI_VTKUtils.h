// Copyright (C) 2007-2014  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : SMESHGUI_VTKUtils.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_VTKUTILS_H
#define SMESHGUI_VTKUTILS_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESH_TypeDefs.hxx"
#include "SMDS_MeshNode.hxx"

#include "SMESHGUI_Utils.h"

#include <SMESH_Object.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_definitions.hxx>

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>
#include <VTKViewer_Filter.h>

class TColStd_IndexedMapOfInteger;

class SALOMEDSClient_Study;

class SUIT_ViewWindow;
class SUIT_ViewManager;

class SVTK_ViewWindow;
class SVTK_Selector;

class LightApp_SelectionMgr;
class SalomeApp_Module;

class SMESHGUI;
class SMESH_Actor;
class SALOME_Actor;

class vtkActor;
class SMDS_Mesh;
class SMDS_MeshNode;
class gp_Pln;
class gp_Lin;
class gp_Dir;
class gp_Pnt2d;
class gp_Vec2d;
class Handle(Geom_Line);
class Handle(Geom_Plane);
namespace SMESH
{
  //----------------------------------------------------------------------------
  typedef std::pair<int,std::string> TKeyOfVisualObj;
  
SMESHGUI_EXPORT
  TVisualObjPtr GetVisualObj( int, const char*, bool nulData =false );
SMESHGUI_EXPORT
  void OnVisuException(); // PAL16631

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  SVTK_ViewWindow* GetViewWindow( const SalomeApp_Module* = 0,
                                  bool = false );
SMESHGUI_EXPORT
  SVTK_ViewWindow* FindVtkViewWindow( SUIT_ViewManager*, SUIT_ViewWindow* );
SMESHGUI_EXPORT
  SVTK_ViewWindow* GetVtkViewWindow( SUIT_ViewWindow* );

SMESHGUI_EXPORT
  SVTK_ViewWindow* GetCurrentVtkView();

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  void RepaintCurrentView();
SMESHGUI_EXPORT
  void RepaintViewWindow( SVTK_ViewWindow* );
SMESHGUI_EXPORT
  void RenderViewWindow( SVTK_ViewWindow* );
SMESHGUI_EXPORT
  void FitAll();

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  SMESH_Actor* FindActorByEntry( SUIT_ViewWindow*, const char* );
SMESHGUI_EXPORT
  SMESH_Actor* FindActorByEntry( const char* );

SMESHGUI_EXPORT
  SMESH_Actor* FindActorByObject( CORBA::Object_ptr );

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  SMESH_Actor* CreateActor( _PTR(Study), const char*, int = false );
SMESHGUI_EXPORT
  void DisplayActor( SUIT_ViewWindow*, SMESH_Actor* );
SMESHGUI_EXPORT
  void RemoveActor( SUIT_ViewWindow*, SMESH_Actor* );
SMESHGUI_EXPORT
  void RemoveVisuData( int );

  //----------------------------------------------------------------------------
  enum EDisplaing { eDisplayAll, eDisplay, eDisplayOnly, eErase, eEraseAll };

SMESHGUI_EXPORT
  bool UpdateView( SUIT_ViewWindow*, EDisplaing, const char* = "" );
SMESHGUI_EXPORT            
  bool UpdateView( EDisplaing, const char* = "" );

SMESHGUI_EXPORT
  void UpdateView();

SMESHGUI_EXPORT
  bool UpdateNulData( const Handle(SALOME_InteractiveObject)& theIO, bool theDisplay);

SMESHGUI_EXPORT
  bool Update( const Handle(SALOME_InteractiveObject)& theIO, bool theDisplay);

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  void SetPointRepresentation( bool );

SMESHGUI_EXPORT
  void SetPickable( SMESH_Actor* = 0 );

SMESHGUI_EXPORT
  void UpdateSelectionProp( SMESHGUI* );

SMESHGUI_EXPORT
  void UpdateFontProp( SMESHGUI* );

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  SVTK_Selector* GetSelector( SUIT_ViewWindow* = GetActiveWindow() );

SMESHGUI_EXPORT
  void SetFilter( const Handle(VTKViewer_Filter)&, SVTK_Selector* = GetSelector() );
SMESHGUI_EXPORT
  Handle(VTKViewer_Filter) GetFilter( int, SVTK_Selector* = GetSelector() );
SMESHGUI_EXPORT
  bool IsFilterPresent( int, SVTK_Selector* = GetSelector() );
SMESHGUI_EXPORT
  void RemoveFilter( int, SVTK_Selector* = GetSelector() );

SMESHGUI_EXPORT
  void RemoveFilters( SVTK_Selector* = GetSelector());

SMESHGUI_EXPORT
  bool IsValid( SALOME_Actor*, int, SVTK_Selector* = GetSelector() );

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  int GetNameOfSelectedSortedNodes( SMDSAbs_EntityType,
                                    SVTK_Selector*,
                                    SMESH_Actor*,
                                    int,
                                    QString& );
SMESHGUI_EXPORT
  int GetNameOfSelectedNodes( SVTK_Selector*,
                              const Handle(SALOME_InteractiveObject)&,
                              QString& );
SMESHGUI_EXPORT
  int GetNameOfSelectedElements( SVTK_Selector*,
                                 const Handle(SALOME_InteractiveObject)&,
                                 QString& );
SMESHGUI_EXPORT
  int GetEdgeNodes( SVTK_Selector*, const TVisualObjPtr&, int&, int& );

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT  
  int GetNameOfSelectedNodes( LightApp_SelectionMgr*,
                              const Handle(SALOME_InteractiveObject)&,
                              QString& );
SMESHGUI_EXPORT
  int GetNameOfSelectedNodes( LightApp_SelectionMgr*, QString& );
SMESHGUI_EXPORT
  int GetNameOfSelectedElements( LightApp_SelectionMgr*,
                                 const Handle(SALOME_InteractiveObject)&,
                                 QString& );
SMESHGUI_EXPORT
  int GetNameOfSelectedElements( LightApp_SelectionMgr*, QString& );
SMESHGUI_EXPORT
  int GetSelected( LightApp_SelectionMgr*, TColStd_IndexedMapOfInteger&, 
                   const bool = true );

SMESHGUI_EXPORT
  int GetEdgeNodes( LightApp_SelectionMgr*, int&, int& );

SMESHGUI_EXPORT
  void SetControlsPrecision( const long );

#ifndef DISABLE_PLOT2DVIEWER
SMESHGUI_EXPORT
  void ClearPlot2Viewers( SUIT_ViewWindow* theWindow );
#endif

  //----------------------------------------------------------------------------
SMESHGUI_EXPORT
  bool ComputeClippingPlaneParameters( std::list<vtkActor*> theActorList,
                                       double theNormal[3],
                                       double theDist,
                                       double theBounds[6],
                                       double theOrigin[3] );

  bool ComputeBounds( std::list<vtkActor*> theActorList,
                      double theBounds[6]);

  void PositionToDistance( double theBounds[6],
                           double theDirection[3],
                           double thePos[3],
                           double& theDist );
  typedef std::pair<SMESH_TNodeXYZ, double> TNodeOfDist;
  typedef std::pair<SMESH_TNodeXYZ, std::pair<double, double> > TNodeOfAngleAndDist, TNodeOfDistToPlaneAndDist;
  typedef std::pair<int, std::pair<double, double> > TIdOfDistToPlaneAndDist;
  bool CreatePlaneOnThreePoints( const gp_Pnt& thePoint1,
                                 const gp_Pnt& thePoint2,
                                 const gp_Pnt& thePoint3,
                                 gp_Pln& thePlane );

  void FindNbLowestPoint( std::list<gp_Pnt2d> theList, gp_Pnt2d& theNode );
  bool IsNotPlaneIntersection( std::vector<SMESH_TNodeXYZ>& theVector,
                               const gp_Pln& thePlane );
  bool GetCorrectSequenceOfId( std::vector<SMESH_TNodeXYZ>& theVector );
  void GetCorrectSequenceTwoPlaneOfId( std::vector<SMESH_TNodeXYZ>& thePlane1,
                                       std::vector<SMESH_TNodeXYZ>& thePlane2,
                                       std::list<int>& theResultListId );
  void GetSortedNodesOnPolygon( std::vector<SMESH_TNodeXYZ>& theVectorOfNode,
                                std::list<int>& theResultListId );
  void GetSortedNodesOnPyramid( std::vector<SMESH_TNodeXYZ>& theVectorOfNode,
                                std::list<int>& theResultListId );
  void GetSortedNodesOnPrism( std::vector<SMESH_TNodeXYZ>& theVectorOfNode,
                              std::list<int>& theResultListId );
  bool Get2BasePlane( std::vector<SMESH_TNodeXYZ>& theVector,
                      std::vector<SMESH_TNodeXYZ>& thePlane1,
                      std::vector<SMESH_TNodeXYZ>& thePlane2);
  bool GetNextCombination ( std::vector<int> & theVector1,
                            std::vector<int> & theVector2,
                            int theNbPoint );
  static bool CompareNodeOfAngleAndDist ( const TNodeOfAngleAndDist& first,
                                          const TNodeOfAngleAndDist& second );
  static bool CompareNodeOfDist ( const TNodeOfAngleAndDist& first,
                                  const TNodeOfAngleAndDist& second );
  static bool CompareDistOfPlane ( const TNodeOfDistToPlaneAndDist& first,
                                   const TNodeOfDistToPlaneAndDist& second );
  static bool CompareDistOfPlaneById ( const TIdOfDistToPlaneAndDist& first,
                                       const TIdOfDistToPlaneAndDist& second );
  static bool CompareDistForCorrectPlane ( const TNodeOfDist& first,
                                           const TNodeOfDist& second );

 SMESHGUI_EXPORT
   void RemoveVisualObjectWithActors( const char* theEntry, bool fromAllViews = false );
};

#endif // SMESHGUI_VTKUTILS_H
