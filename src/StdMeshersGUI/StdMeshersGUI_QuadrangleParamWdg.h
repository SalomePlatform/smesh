// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : StdMeshersGUI_QuadrangleParamWdg.h
//  Author : Open CASCADE S.A.S. (jfa)

#ifndef STDMESHERSGUI_QUADRANGLEPARAMWDG_H
#define STDMESHERSGUI_QUADRANGLEPARAMWDG_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

// Qt includes
#include <QWidget>

class QButtonGroup;

class STDMESHERSGUI_EXPORT StdMeshersGUI_QuadrangleParamWdg : public QWidget
{
  Q_OBJECT

public:
  StdMeshersGUI_QuadrangleParamWdg (QWidget* parent = 0);
  ~StdMeshersGUI_QuadrangleParamWdg();

  void SetType (int theType);
  int  GetType ();

private:
  // Quadranle preference, Triangle preference, Reduced
  QButtonGroup* myType;
};

#endif // STDMESHERSGUI_QUADRANGLEPARAMWDG_H
