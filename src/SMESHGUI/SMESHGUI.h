//  SMESH SMESHGUI : GUI for SMESH component
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
//  File   : SMESHGUI.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_HeaderFile
#define SMESHGUI_HeaderFile

#include "TColStd_MapOfInteger.hxx"
#include <map>

#include "SMESHDS_Document.hxx"

// SALOME Includes
#include "SALOMEGUI.h"
#include "QAD_Desktop.h"
#include "SALOME_Selection.h"
#include "SALOME_InteractiveObject.hxx"

#include "SMESHGUI_StudyAPI.h"
#include "SMESHGUI_Hypotheses.h"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SALOMEDS_Attributes)
#include CORBA_SERVER_HEADER(SMESH_Filter)

// QT Includes
#include <qstringlist.h>

// VTK Inlcludes

class vtkActorCollection;
class vtkActor2DCollection;
class vtkScalarBarActor;
class vtkActor;

class SMESH_Actor;

//=================================================================================
// class    : SMESHGUI
// purpose  :
//=================================================================================
class SMESHGUI : public SALOMEGUI
{
  Q_OBJECT

private :
    
  QAD_Desktop*              myDesktop;
  QAD_Study*                myActiveStudy;
  SMESH::SMESH_Gen_var      myComponentMesh;
  GEOM::GEOM_Gen_var        myComponentGeom;

  QDialog*                  myActiveDialogBox;
  int                       myNbMesh ;
  int                       myState ;     

  vtkActorCollection*       mySimulationActors;
  vtkActor2DCollection*     mySimulationActors2D;

  SMESH_Actor*              myCurrentMesh;

  SALOMEDS::Study_var       myStudy;
  int                       myStudyId;

  SMESHGUI_StudyAPI         myStudyAPI;

  //  vtkScalarBarActor*    myScalarBar;

  SMESHDS_Document *        myDocument;//NBU

  bool                      myAutomaticUpdate;

  SMESH::FilterManager_var  myFilterMgr;

  // Hypotheses/algorithms from plugin libraries
  map<string, HypothesisData*>                    myHypothesesMap;
  map<string, HypothesisData*>                    myAlgorithmsMap;
  map<string, SMESHGUI_GenericHypothesisCreator*> myHypCreatorMap;

public :

  SMESHGUI( const QString& name = "", QObject* parent = 0 );
  ~SMESHGUI();

  static SMESHGUI*    GetOrCreateSMESHGUI( QAD_Desktop* desktop );
  static SMESHGUI*    GetSMESHGUI() ;

  QAD_Study*          GetActiveStudy() ;
  QAD_Desktop*        GetDesktop() ;
  SALOMEDS::Study_ptr GetStudy();
  SMESHGUI_StudyAPI   GetStudyAPI();

  QDialog*            GetActiveDialogBox() ;               
  void                SetActiveDialogBox(QDialog* aDlg) ;  

  void                SetState(int aState) ;
  void                ResetState() ;                       
  bool                DefineDlgPosition(QWidget* aDlg, int& x, int& y) ;


  virtual bool OnGUIEvent        (int theCommandID, QAD_Desktop* parent);
  virtual bool OnMousePress      (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame);
  virtual bool OnMouseMove       (QMouseEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame);
  virtual bool OnKeyPress        (QKeyEvent* pe, QAD_Desktop* parent, QAD_StudyFrame* studyFrame);
  virtual bool ActiveStudyChanged( QAD_Desktop* parent );
  virtual bool SetSettings       ( QAD_Desktop* parent );
  virtual void DefinePopup       ( QString & theContext, QString & theParent, QString & theObject );
  virtual bool CustomPopup       ( QAD_Desktop* parent, QPopupMenu* popup, const QString & theContext,
                                   const QString & theParent, const QString & theObject );
  virtual void BuildPresentation ( const Handle(SALOME_InteractiveObject)& theIO );
  virtual void SupportedViewType (int* buffer, int bufferSize);
  virtual void Deactivate        ();

  void OnEditDelete();

  /* Mesh Management */
  SMESH::SMESH_Mesh_ptr       InitMesh( GEOM::GEOM_Shape_ptr aShape, QString NameMesh );
  SMESH::SMESH_subMesh_ptr    AddSubMesh( SMESH::SMESH_Mesh_ptr aMesh, GEOM::GEOM_Shape_ptr aShape, QString NameMesh );
  SMESH::SMESH_Group_ptr      AddGroup( SMESH::SMESH_Mesh_ptr aMesh, SMESH::ElementType aType, QString aName );

  /* Hypotheses and Algorithms Management */
  void InitAvailableHypotheses ();
  QStringList GetAvailableHypotheses (const bool isAlgo);
  HypothesisData* GetHypothesisData (const char* aHypType);
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator (const QString& aHypType);

  SMESH::SMESH_Hypothesis_ptr CreateHypothesis (const QString& aHypType,
						const QString& aHypName,
						const bool     isAlgo = false);

  bool AddHypothesisOnMesh (SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp);
  bool AddAlgorithmOnMesh  (SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp);

  bool AddHypothesisOnSubMesh (SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp);
  bool AddAlgorithmOnSubMesh  (SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp);

  bool RemoveHypothesisOrAlgorithmOnMesh (const Handle(SALOME_InteractiveObject)& IObject);
  bool RemoveHypothesisOrAlgorithmOnMesh (SALOMEDS::SObject_ptr MorSM,
					  SMESH::SMESH_Hypothesis_ptr anHyp);

  void SetPickable(SMESH_Actor* theActor = NULL);

  /* NODES */
  void ViewNodes();
  vtkActor* SimulationMoveNode(SMESH_Actor* Mactor, int idnode);
  void MoveNode( SMESH::SMESH_Mesh_ptr aMesh, int idnode, float x, float y, float z);

  void DisplaySimulationNode( SMESH::SMESH_Mesh_ptr aMesh, float x, float y, float z);
  void DisplaySimulationMoveNode( vtkActor* ac, int idnode, float x, float y, float z);

  void RemoveNodes(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex) ;

  /* EDGES */
  void DisplayEdges(SMESH_Actor* ac, bool visibility = true);
  void DisplayEdgesConnectivityLegendBox(vtkActor *ac);
  void DisplaySimulationEdge( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );

  /* TRIANGLES */
  void DisplaySimulationTriangle( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse );

  /* QUADRANGLES */
  void DisplaySimulationQuadrangle( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse );

  /* VOLUMES */
  void DisplaySimulationTetra( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );
  void DisplaySimulationHexa( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );

  /* ELEMENTS */
  void RemoveElements(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex) ;
  void OrientationElements(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex);
  void DiagonalInversion(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex);

  void AddFace( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse );
  void AddEdge( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );
  void AddVolume( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );
  void AddNode( SMESH::SMESH_Mesh_ptr aMesh, float x, float y, float z);

  /* Simulation management */
  void EraseSimulationActors() ;    

  /* Non modal dialog boxes magement */
  void EmitSignalDeactivateDialog() ;
  void EmitSignalCloseAllDialogs() ;

  /* Selection management */
  int GetNameOfSelectedIObjects( SALOME_Selection* Sel, QString& aName ) ;
  int GetNameOfSelectedNodes( SALOME_Selection* Sel, QString& aName ) ;
  int GetNameOfSelectedElements( SALOME_Selection* Sel, QString& aName ) ;
  int GetNameOfSelectedEdges( SALOME_Selection* Sel, QString& aName ) ;

  SMESH::SMESH_Hypothesis_ptr ConvertIOinSMESHHypothesis( const Handle(SALOME_InteractiveObject)& IO, 
							  Standard_Boolean& testResult ) ;

  SMESH::SMESH_Mesh_ptr       ConvertIOinMesh(const Handle(SALOME_InteractiveObject)& IO, Standard_Boolean& testResult) ;
  SMESH::SMESH_subMesh_ptr    ConvertIOinSubMesh(const Handle(SALOME_InteractiveObject)& IO, Standard_Boolean& testResult) ;
  SMESH::SMESH_Group_ptr      ConvertIOinSMESHGroup(const Handle(SALOME_InteractiveObject)& IO, Standard_Boolean& testResult) ;

  /* Geometry Client */
  GEOM::GEOM_Shape_ptr        ConvertIOinGEOMShape( const Handle(SALOME_InteractiveObject)& IO, 
						    Standard_Boolean& testResult ) ;

  /* Popup management */
  QString CheckTypeObject(const Handle(SALOME_InteractiveObject)& IO);
  QString CheckHomogeneousSelection();

  /* Scripts management */
  SMESH_Actor* ReadScript(SMESH::SMESH_Mesh_ptr aMesh);

  /* Actors management */
  void Dump(SMESH_Actor* Mactor);

  void SetViewMode(int commandId);
  void ChangeRepresentation( SMESH_Actor* ac, int type );

  SMESH_Actor* FindActor(CORBA::Object_ptr theObj, 
			 Standard_Boolean& testResult,
			 bool onlyInActiveView);
  SMESH_Actor* FindActorByEntry(QString entry, 
				Standard_Boolean& testResult,
				bool onlyInActiveView);

  void InitActor(SMESH::SMESH_Mesh_ptr aMesh);
  void DisplayActor(SMESH_Actor* ac, bool visibility = true);
  void EraseActor(SMESH_Actor* ac);
  bool AddActorInSelection(SMESH_Actor* ac);

  void UpdateView();

  void Update();
  void Update(const Handle(SALOME_InteractiveObject)& IO);

  void ScalarVisibilityOff();


  /* Settings management */
  void SetSettingsScalarBar(vtkScalarBarActor* theScalarBar,
			    QString Bold, QString Italic, QString Shadow, QString Font, 
			    QString Orientation, float Width, float Height, 
			    int NbColors, int NbLabels);
  void SetDisplaySettings();    

  SALOMEDS::Study::ListOfSObject* GetMeshesUsingAlgoOrHypothesis( SMESH::SMESH_Hypothesis_ptr AlgoOrHyp ) ;

  /* Control management */
  void Control(int theCommandID);

  /* Parameter functions */
  double Parameter( Standard_Boolean& res, const double aValue,
  		    const char* aLabel,    const char* aTitle,
		    const double bottom,   const double top, const int decimals ) ;
  int    Parameter( Standard_Boolean& res, const int aValue,
  		    const char* aLabel,    const char* aTitle,
		    const int bottom,      const int top ) ;

  static void setOrb();

  /* Import/Export */ //NBU
  static void Import_Mesh(QAD_Desktop* parent, int theCommandID);
  static void Export_Mesh(QAD_Desktop* parent, int theCommandID);

  /* Filter manager */
  SMESH::FilterManager_ptr GetFilterMgr();

signals:
  void SignalDeactivateActiveDialog() ;
  void SignalCloseAllDialogs() ;
};

#endif
