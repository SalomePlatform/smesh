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
// File   : SMESHGUI_ConvToQuadDlg.h
// Author : Open CASCADE S.A.S.
//
#ifndef SMESHGUI_CONVTOQUADDLG_H
#define SMESHGUI_CONVTOQUADDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"

class QCheckBox;
class QRadioButton;
class QButtonGroup;
class QGroupBox;
class QLabel;

class SMESHGUI_EXPORT SMESHGUI_ConvToQuadDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT       
        
public:
  SMESHGUI_ConvToQuadDlg();
  virtual ~SMESHGUI_ConvToQuadDlg();

  bool          IsMediumNdsOnGeom() const;
  void          SetMediumNdsOnGeom( const bool );
  bool          IsEnabledCheck() const;
  void          SetEnabledCheck( const bool );
  void          SetEnabledGroup( const bool );
  void          SetEnabledControls( const bool );
  void          SetEnabledRB( const int, const bool );
  int           CurrentRB(); //returns the ID of the selected toggle button
  void          ShowWarning(bool);
  bool          isWarningShown();
  bool          IsBiQuadratic() const;

signals:
  void          onClicked( int );

private:
  QCheckBox*    myMedNdsOnGeom;
  QGroupBox*    myBGBox;
  QButtonGroup* myBG;
  QRadioButton* myRB2Lin;
  QRadioButton* myRB2Quad;
  QRadioButton* myRB2BiQua;
  QLabel* myWarning;
};

#endif // SMESHGUI_CONVTOQUADDLG_H
