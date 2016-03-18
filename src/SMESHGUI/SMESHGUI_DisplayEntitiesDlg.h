// Copyright (C) 2014-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI_DisplayEntitiesDlg.h
//  Author : Alexander KOVALEV, Open CASCADE S.A.S. (alexander.kovalev@opencascade.com)

#ifndef SMESHGUI_DISPLAYENTITIES_H
#define SMESHGUI_DISPLAYENTITIES_H

#include "SMESHGUI_Dialog.h"
#include "SMESH_SMESHGUI.hxx"

#include <SMESH_Actor.h>

class QCheckBox;

class SMESHGUI_EXPORT SMESHGUI_DisplayEntitiesDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT
    
public:
  SMESHGUI_DisplayEntitiesDlg( QWidget* parent );
  ~SMESHGUI_DisplayEntitiesDlg();

private:
  void InverseEntityMode( unsigned int& theOutputMode,
                          unsigned int theMode );

private slots:
  void              onOk();
  void              onHelp();
  void              onChangeEntityMode( bool isChecked );

private:
  Handle(SALOME_InteractiveObject) myIObject;
  unsigned int myEntityMode;
  SMESH_Actor *myActor;
  int myNbCheckedButtons;
  QCheckBox* my0DElemsTB;
  QCheckBox* myEdgesTB;
  QCheckBox* myFacesTB;
  QCheckBox* myVolumesTB;
  QCheckBox* myBallsTB;
};

#endif // SMESHGUI_DISPLAYENTITIES_H
