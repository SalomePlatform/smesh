// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File   : SMESHGUI_ConvToQuadOp.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_CONVTOQUADOP_H
#define SMESHGUI_CONVTOQUADOP_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_SelectionOp.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI_ConvToQuadDlg;
class SMESHGUI_MeshEditPreview;

class SMESHGUI_EXPORT SMESHGUI_ConvToQuadOp : public SMESHGUI_SelectionOp
{ 
  Q_OBJECT

public:      
  enum MeshDestinationType { Linear = 1, Quadratic = 2, BiQuadratic = 4 };

public:
  SMESHGUI_ConvToQuadOp();
  virtual ~SMESHGUI_ConvToQuadOp();
  
  virtual LightApp_Dialog*       dlg() const;  

  static MeshDestinationType     DestinationMesh( const SMESH::SMESH_IDSource_var& ,
                                                  bool* isMixOrder = 0);
protected:
  virtual void                   startOperation();
  virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;

protected slots:
  virtual bool                   onApply();
  void                           ConnectRadioButtons( int );
  void                           onWarningWinFinished();

private:
  SMESHGUI_ConvToQuadDlg*        myDlg;
  SMESHGUI_MeshEditPreview*      myBadElemsPreview;
};

#endif // SMESHGUI_CONVTOQUADOP_H
