// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

// SMESH SMESHGUI : GUI for the adaptation in the SMESH component
// File   : SMESHGUI_AdaptDlg.h
// Author : Gérald NICOLAS, EDF
//
#ifndef SMESHGUI_ADAPTDLG_H
#define SMESHGUI_ADAPTDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI;

//=================================================================================
// class    : SMESHGUI_AdaptDlg
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_AdaptDlg : public QWidget
{
public:
  SMESHGUI_AdaptDlg( SMESHGUI*,
                     int theCommandID,
                     SMESH::SMESH_Mesh_ptr = SMESH::SMESH_Mesh::_nil() );
  ~SMESHGUI_AdaptDlg();

  void action (int theCommandID);
  virtual bool OnGUIEvent (int theCommandID);

//   static ADAPT::ADAPT_Gen_var InitAdaptGen(SalomeApp_Application*);

public slots:

protected slots:

private slots:

private:

//   void recupPreferences();
  SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */

  QString _ObjectName;
  QString _LanguageShort ;
  int _PublisMeshIN ;
  int _PublisMeshOUT ;
  int _YACSMaxIter ;
  int _YACSMaxNode ;
  int _YACSMaxElem ;
  int _YACSTypeTest ;
};

#endif // SMESHGUI_ADAPTDLG_H
