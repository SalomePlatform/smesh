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

#include "SMESHDS_Document.hxx"

// SALOME Includes
#include "QAD_Desktop.h"
#include "SALOME_Selection.h"
#include "SALOME_InteractiveObject.hxx"

#include "SMESH_Actor.h"

#include "SMESHGUI_StudyAPI.h"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SALOMEDS_Attributes)

// QT Includes
#include <qstringlist.h>

// VTK Inlcudes
#include <vtkScalarBarActor.h>

//=================================================================================
// class    : SMESHGUI
// purpose  :
//=================================================================================
class SMESHGUI : public QObject
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

  bool                      myAutomaticUpdate;

public :

  SMESHGUI();
  ~SMESHGUI();

  static SMESHGUI*    GetOrCreateSMESHGUI( QAD_Desktop* desktop );
  static SMESHGUI*    GetSMESHGUI() ;

  QAD_Study*          GetActiveStudy() ;
  QAD_Desktop*        GetDesktop() ;
  SALOMEDS::Study_ptr GetStudy();
  SMESHGUI_StudyAPI   GetStudyAPI();

  vtkScalarBarActor*  GetScalarBar();

  QDialog*            GetActiveDialogBox() ;               
  void                SetActiveDialogBox(QDialog* aDlg) ;  

  void                SetState(int aState) ;
  void                ResetState() ;                       
  bool                DefineDlgPosition(QWidget* aDlg, int& x, int& y) ;


  /* Managed by IAPP */
  Standard_EXPORT  static bool OnGUIEvent ( int theCommandID, QAD_Desktop* parent) ; 
  Standard_EXPORT  static bool OnMousePress ( QMouseEvent* pe, QAD_Desktop* parent, 
					      QAD_StudyFrame* studyFrame );
  Standard_EXPORT  static bool OnMouseMove  ( QMouseEvent* pe, QAD_Desktop* parent, 
					      QAD_StudyFrame* studyFrame );
  Standard_EXPORT  static bool OnKeyPress   ( QKeyEvent* pe, QAD_Desktop* parent, 
					      QAD_StudyFrame* studyFrame );
  Standard_EXPORT  static void activeStudyChanged ( QAD_Desktop* parent ); 
  Standard_EXPORT  static bool SetSettings ( QAD_Desktop* parent );
  Standard_EXPORT  static void DefinePopup( QString & theContext, 
					    QString & theParent, 
					    QString & theObject );
  Standard_EXPORT  static bool CustomPopup ( QAD_Desktop* parent,
					     QPopupMenu* popup,
					     const QString& theContext,
					     const QString& theParent,
					     const QString& theObject );
  Standard_EXPORT  static void BuildPresentation(const Handle(SALOME_InteractiveObject)&);

  void OnEditDelete();

  /* Mesh Management */
  SMESH::SMESH_Mesh_ptr       InitMesh( GEOM::GEOM_Shape_ptr aShape, QString NameMesh );
  SMESH::SMESH_subMesh_ptr    AddSubMesh( SMESH::SMESH_Mesh_ptr aMesh, GEOM::GEOM_Shape_ptr aShape, QString NameMesh );

  /* Hypothesis Management */
  SMESH::SMESH_Hypothesis_ptr CreateHypothesis( QString TypeHypothesis, QString NameHypothesis );
  void AddHypothesisOnMesh( SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp ) ;
  void AddHypothesisOnSubMesh( SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp ) ;

  void RemoveHypothesisOrAlgorithmOnMesh( const Handle(SALOME_InteractiveObject)& IObject ) ;
  void RemoveHypothesisOrAlgorithmOnMesh( SALOMEDS::SObject_ptr MorSM, SMESH::SMESH_Hypothesis_ptr anHyp ) ;

  void CreateLocalLength( QString TypeHypothesis, QString NameHypothesis, double Length );
  void CreateNbSegments( QString TypeHypothesis, QString NameHypothesis, int nbSegments );
  void CreateMaxElementArea( QString TypeHypothesis, QString NameHypothesis, double MaxArea );
  void CreateMaxElementVolume( QString TypeHypothesis, QString NameHypothesis, double MaxVolume );

  /* Algorithms Management */
  void AddAlgorithmOnMesh( SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp );
  void AddAlgorithmOnSubMesh( SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp );
  void CreateAlgorithm( QString TypeAlgo, QString NameAlgo );

  /* NODES */
  void ViewNodes();
  vtkActor* SimulationMoveNode(SMESH_Actor* Mactor, int idnode);
  void MoveNode( SMESH::SMESH_Mesh_ptr aMesh, int idnode, float x, float y, float z);
  void AddNode(SMESH_Actor*, int idnode, float x, float y, float z) ; 
  void AddNodes( SMESH_Actor* Mactor, int number, 
		 const SMESH::double_array& coords, const SMESH::long_array& indexes);

  void DisplaySimulationNode( SMESH::SMESH_Mesh_ptr aMesh, float x, float y, float z);
  void DisplaySimulationMoveNode( vtkActor* ac, int idnode, float x, float y, float z);

  void RemoveNode(SMESH_Actor*, int idnode) ;
  void RemoveNodes(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex) ;
  void RemoveNodes(SMESH_Actor* Mactor, int number, 
		   const SMESH::double_array& coords, const SMESH::long_array& indexes);

  /* EDGES */
  void AddEdge(SMESH_Actor*, int idedge, int idnode1, int idnode2) ;
  void AddEdges( SMESH_Actor* Mactor, int number, 
		 const SMESH::double_array& coords, const SMESH::long_array& indexes);
  void DisplayEdges(SMESH_Actor* ac, bool visibility = true);
  void DisplayEdgesConnectivityLegendBox(vtkActor *ac);
  void DisplaySimulationEdge( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );

  /* TRIANGLES */
  void AddTriangle(SMESH_Actor*, int idtri, int idnode1, int idnode2, int idnode3) ;
  void AddTriangles( SMESH_Actor* Mactor, int number, 
		     const SMESH::double_array& coords, const SMESH::long_array& indexes);
  void DisplaySimulationTriangle( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse );

  /* QUADRANGLES */
  void AddQuadrangle(SMESH_Actor*, int idquad, int idnode1, int idnode2, 
		      int idnode3, int idnode4) ;
  void AddQuadrangles( SMESH_Actor* Mactor, int number, 
		       const SMESH::double_array& coords, const SMESH::long_array& indexes);
  void DisplaySimulationQuadrangle( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex, bool reverse );

  /* VOLUMES */
  void AddTetra(SMESH_Actor*, int idtetra, int idnode1, int idnode2, 
		 int idnode3, int idnode4) ;
  void AddHexaedre(SMESH_Actor*, int idhexa, int idnode1, int idnode2, 
		    int idnode3, int idnode4, int idnode5, int idnode6, int idnode7, int idnode8) ;
  void AddTetras( SMESH_Actor* Mactor, int number, 
		       const SMESH::double_array& coords, const SMESH::long_array& indexes);
  void AddHexaedres( SMESH_Actor* Mactor, int number, 
		       const SMESH::double_array& coords, const SMESH::long_array& indexes);
  void DisplaySimulationTetra( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );
  void DisplaySimulationHexa( SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex );

  /* ELEMENTS */
  void RemoveElement(SMESH_Actor*, int idnode);
  void RemoveElements(SMESH::SMESH_Mesh_ptr aMesh, const TColStd_MapOfInteger& MapIndex) ;
  void RemoveElements(SMESH_Actor* Mactor, int number, 
		      const SMESH::double_array& coords, const SMESH::long_array& indexes);
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

  SMESH_Actor* FindActor(SMESH::SMESH_Mesh_ptr aMesh, 
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
  void DisplayScalarBar(bool visibility);
  void UpdateScalarBar(float MinRange, float MaxRange);

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
  void Import_Document(QAD_Desktop* parent, int theCommandID);
  static void Export_Mesh(QAD_Desktop* parent, int theCommandID);

signals:
  void SignalDeactivateActiveDialog() ;
  void SignalCloseAllDialogs() ;
};

#endif
