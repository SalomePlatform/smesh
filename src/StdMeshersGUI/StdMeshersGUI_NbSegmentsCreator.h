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

// File   : StdMeshersGUI_NbSegmentsCreator.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_NBSEGMENTSCREATOR_H
#define STDMESHERSGUI_NBSEGMENTSCREATOR_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

#include "StdMeshersGUI_StdHypothesisCreator.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SalomeApp_IntSpinBox;
class QtxComboBox;
class SMESHGUI_SpinBox;
class StdMeshersGUI_DistrTableFrame;
#ifndef DISABLE_PLOT2DVIEWER
  class StdMeshersGUI_DistrPreview;
#endif
class QLineEdit;
class QButtonGroup;
class QGroupBox;
class QGridLayout;
class QRadioButton;
class StdMeshersGUI_SubShapeSelectorWdg;
class StdMeshersGUI_PropagationHelperWdg;

typedef struct
{
  int                 myNbSeg, myDistrType, myConv;
  double              myScale;
  SMESH::double_array myTable;
  QString             myName, myExpr;
  QString             myNbSegVarName, myScaleVarName;

} NbSegmentsHypothesisData;

class STDMESHERSGUI_EXPORT StdMeshersGUI_NbSegmentsCreator : public StdMeshersGUI_StdHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_NbSegmentsCreator();
  virtual ~StdMeshersGUI_NbSegmentsCreator();

  virtual bool checkParams( QString& ) const;

protected:
  virtual QFrame*  buildFrame();
  virtual void     retrieveParams() const;
  virtual QString  storeParams() const;

protected slots:
  virtual void     onValueChanged();

private:
  bool readParamsFromHypo( NbSegmentsHypothesisData& ) const;
  bool readParamsFromWidgets( NbSegmentsHypothesisData& ) const;
  bool storeParamsToHypo( const NbSegmentsHypothesisData& ) const;

private:
  SalomeApp_IntSpinBox*   myNbSeg;
  QtxComboBox*     myDistr;
  SMESHGUI_SpinBox*   myScale;
  StdMeshersGUI_DistrTableFrame*  myTable;
#ifndef DISABLE_PLOT2DVIEWER
  StdMeshersGUI_DistrPreview* myPreview;
#endif
  QLineEdit       *myName, *myExpr;
  QGroupBox*       myConvBox;
  QButtonGroup*    myConv;
  QLabel          *myLScale, *myLTable, *myLExpr, *myInfo;
  QGridLayout*     myGroupLayout;
  int              myTableRow, myPreviewRow;
  //QRadioButton*    myCutNeg;
  QGroupBox*       myReversedEdgesBox;

  StdMeshersGUI_SubShapeSelectorWdg*    myDirectionWidget;
  StdMeshersGUI_PropagationHelperWdg*   myReversedEdgesHelper;
};

#endif // STDMESHERSGUI_NBSEGMENTSCREATOR_H
