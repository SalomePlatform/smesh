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
// File   : SMESHGUI_SplitBiQuad.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SplitBiQuad_H
#define SMESHGUI_SplitBiQuad_H

#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

class SMESHGUI_SplitBiQuadOp;

/*!
 * \brief Dialog performing SMESH_MeshEditor::SplitBiQuadraticIntoLinear()
 */
class SMESHGUI_EXPORT SMESHGUI_SplitBiQuadDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT       
        
    public:
  SMESHGUI_SplitBiQuadDlg();
  virtual ~SMESHGUI_SplitBiQuadDlg();

  friend class SMESHGUI_SplitBiQuadOp;
};

class SMESHGUI_EXPORT SMESHGUI_SplitBiQuadOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:      
  SMESHGUI_SplitBiQuadOp();
  virtual ~SMESHGUI_SplitBiQuadOp();
  
  virtual LightApp_Dialog*       dlg() const;  

protected:
  virtual void                   startOperation();
  //virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;

protected slots:
  virtual bool                   onApply();

private:
  SMESHGUI_SplitBiQuadDlg*       myDlg;
};

#endif // SMESHGUI_SplitBiQuad_H
