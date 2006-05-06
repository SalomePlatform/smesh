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
*  File   : SMESHGUI_ConvToQuadOp.h
*  Module : SMESHGUI
*/

#ifndef SMESHGUI_ConvToQuadOp_H
#define SMESHGUI_ConvToQuadOp_H

#include <SMESHGUI_SelectionOp.h>
//#include <qstringlist.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI_ConvToQuadDlg;

class SMESHGUI_ConvToQuadOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT

public:      
enum MeshType{ Comp = 0, Linear, Quadratic };

public:
  SMESHGUI_ConvToQuadOp();
  virtual ~SMESHGUI_ConvToQuadOp();
  
  virtual LightApp_Dialog*       dlg() const;  

protected:
  virtual void                   startOperation();
  virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;
  MeshType                       ConsistMesh( const SMESH::SMESH_Mesh_var& ) const;

protected slots:
  virtual bool                   onApply();
  void                           ConnectRadioButtons( int);

private:
  SMESHGUI_ConvToQuadDlg*        myDlg;
};

#endif

