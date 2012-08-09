// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
// File   : SMESHGUI_MeshOp.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MESHOP_H
#define SMESHGUI_MESHOP_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_SelectionOp.h"

class HypothesesSet;
class SMESHGUI_MeshDlg;
class SMESHGUI_ShapeByMeshOp;
class HypothesisData;

/*!
 * \brief Operation for mech creation or editing
 *
 *  This operation is used for mech creation or editing. 
*/
class SMESHGUI_EXPORT SMESHGUI_MeshOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT
      
public:
  enum HypType{ Algo = 0, MainHyp, AddHyp, NbHypTypes };

  typedef std::pair<SMESH::SMESH_Hypothesis_var, QString> THypItem;
  typedef QList< THypItem > THypList;

  typedef int THypType;
  typedef QMap< THypType, THypList > TType2HypList;

  typedef int THypDim;
  typedef QMap< THypDim, TType2HypList > TDim2Type2HypList;

public:
  SMESHGUI_MeshOp( const bool, const bool = true );
  virtual ~SMESHGUI_MeshOp();
  
  virtual LightApp_Dialog*       dlg() const;  

protected:
  virtual void                   startOperation();
  virtual void                   commitOperation();
  virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;
  virtual bool                   isValid( SUIT_Operation* ) const;

protected slots:
  virtual bool                   onApply();
  void                           onCreateHyp( const int, const int );
  void                           onEditHyp( const int, const int );
  void                           onHypoSet( const QString& );
  void                           onGeomSelectionByMesh( bool );
  void                           onPublishShapeByMeshDlg( SUIT_Operation* );
  void                           onCloseShapeByMeshDlg( SUIT_Operation* );
  void                           onAlgoSelected( const int, const int = -1 );
  void                           processSet();
  void                           onHypoCreated( int );
  void                           onHypoEdited( int );

private:
  typedef QList<HypothesisData*> THypDataList; // typedef: list of hypothesis data

  bool                           isValid( QString& ) const;
  void                           availableHyps( const int, 
                                                const int,
                                                QStringList&,
                                                THypDataList&,
                                                HypothesisData* = 0 ) const;
  static void                    existingHyps( const int, 
                                               const int, 
                                               _PTR(SObject),
                                               QStringList&, 
                                               THypList&,
                                               HypothesisData* = 0 );
  HypothesisData*                hypData( const int,
                                          const int,
                                          const int ); // access to myAvailableHypData

  void                           createHypothesis( const int, const int,
                                                   const QString& );

  bool                           createMesh( QString&, QStringList& );
  bool                           createSubMesh( QString&, QStringList& );
  bool                           editMeshOrSubMesh( QString& );

  int                            currentHyp( const int, const int ) const;
  bool                           isAccessibleDim( const int ) const;
  void                           setCurrentHyp( const int, const int, const int );
  void                           setDefaultName() const;
  SMESH::SMESH_Hypothesis_var    getAlgo( const int );
  void                           readMesh();
  QString                        name( _PTR(SObject) ) const;
  int                            find( const SMESH::SMESH_Hypothesis_var&,
                                       const THypList& ) const;
  SMESH::SMESH_Hypothesis_var    getInitParamsHypothesis( const QString&,
                                                          const QString& ) const;
  bool                           isSubshapeOk() const;
  char*                          isSubmeshIgnored() const;
  _PTR(SObject)                  getSubmeshByGeom() const;
  void                           selectObject( _PTR(SObject) ) const;

private:
  SMESHGUI_MeshDlg*              myDlg;
  SMESHGUI_ShapeByMeshOp*        myShapeByMeshOp;
  bool                           myToCreate;
  bool                           myIsMesh;
  bool                           myIsOnGeometry; //!< TRUE if edited mesh accotiated with geometrical object

  TDim2Type2HypList              myExistingHyps; //!< all hypothesis of SMESH module
  TDim2Type2HypList              myObjHyps;      //!< hypothesis assigned to the current 
                                                 //   edited mesh/sub-mesh

  // hypdata corresponding to hypotheses present in myDlg
  THypDataList                   myAvailableHypData[4][NbHypTypes];

  bool                           myIgnoreAlgoSelection;
  HypothesesSet* myHypoSet;
  int myDim, myType;

  QString                        myObjectToSelect;
};

#endif // SMESHGUI_MESHOP_H
