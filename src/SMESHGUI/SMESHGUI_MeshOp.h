// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
/**
*  SMESH SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_MeshOp.h
*  Author : Sergey LITONIN
*  Module : SMESHGUI
*/

#ifndef SMESHGUI_MeshOp_H
#define SMESHGUI_MeshOp_H

#include "SMESHGUI_SelectionOp.h"
#include <qstringlist.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI_MeshDlg;
class SMESH_TypeFilter;
class SMESH_NumberFilter;


/*!
 * \brief Operation for mech creation or editing
 *
 *  This operation is used for mech creation or editing. 
*/
class SMESHGUI_MeshOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT
      
  enum HypType{ Algo = 0, MainHyp, AddHyp };

public:
  SMESHGUI_MeshOp( const bool theToCreate, const bool theIsMesh = true );
  virtual ~SMESHGUI_MeshOp();
  
  virtual LightApp_Dialog*       dlg() const;  

protected:
  virtual void                   startOperation();
  virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;
  virtual bool                   isValid( SUIT_Operation* ) const;

protected slots:
  virtual bool                   onApply();
  void                           onCreateHyp( const int theHypType, const int theIndex );
  void                           onEditHyp( const int theHypType, const int theIndex );
  void                           onHypoSet( const QString& theSetName );

private:
  bool                           isValid( QString& ) const;
  void                           availableHyps( const int theDim, 
                                                const int theHypType, 
                                                QStringList& theHyps ) const;
  void                           existingHyps( const int theDim, 
                                               const int theHypType, 
                                               _PTR(SObject) theFather,
                                               QStringList& theHyps, 
                                               QValueList<SMESH::SMESH_Hypothesis_var>& theHypVars );
  
  bool                           createMesh( QString& );
  bool                           createSubMesh( QString& );
  bool                           editMeshOrSubMesh( QString& );
  
  int                            currentHyp( const int, const int ) const;
  bool                           isAccessibleDim( const int ) const;
  void                           setCurrentHyp( const int, const int, const int );
  void                           setDefaultName() const;
  SMESH::SMESH_Hypothesis_var    getAlgo( const int );
  void                           readMesh();
  QString                        name( _PTR(SObject) ) const;
  int                            find( const SMESH::SMESH_Hypothesis_var&,
                                       const QValueList<SMESH::SMESH_Hypothesis_var>& ) const;

private:
  typedef QMap< int, QValueList<SMESH::SMESH_Hypothesis_var> > IdToHypListMap;
  typedef QMap< int, IdToHypListMap > DimToHypMap;
  
  SMESHGUI_MeshDlg*              myDlg;
  bool                           myToCreate;
  bool                           myIsMesh;
  
  DimToHypMap                    myExistingHyps; //!< all hypothesis of SMESH module
  DimToHypMap                    myObjHyps;      //!< hypothesis assigned to the current 
                                                 //   edited mesh/sub-mesh
};

#endif
