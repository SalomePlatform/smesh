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
*  File   : SMESHGUI_ConvToQuadDlg.h
*  Module : SMESH
*/


#ifndef SMESHGUI_ConvToQuadDlg_H
#define SMESHGUI_ConvToQuadDlg_H

#include "SMESHGUI_Dialog.h"
/*#include <qframe.h>
#include <qstringlist.h>
#include <qmap.h>*/

//idl headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCheckBox;
class QRadioButton;
class QButtonGroup;

class SMESHGUI_ConvToQuadDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT       
        
public:
           SMESHGUI_ConvToQuadDlg();
  virtual ~SMESHGUI_ConvToQuadDlg();

  bool     IsMediumNdsOnGeom() const;
  void     SetMediumNdsOnGeom(const bool theCheck);
  bool     IsEnabledCheck() const;
  void     SetEnabledCheck( const bool theCheck );
  void     SetEnabledGroup( const bool theCheck );
  void     SetEnabledControls( const bool theCheck );
  void     SetEnabledRB( const int idx, const bool theCheck );
  int      CurrentRB();//returns the ID of the selected toggle button

signals:
  void     onClicked( int );

private:

  QCheckBox* myMedNdsOnGeom;
  QButtonGroup* myBG;
  QRadioButton* myRB1;
  QRadioButton* myRB2;

};

#endif




