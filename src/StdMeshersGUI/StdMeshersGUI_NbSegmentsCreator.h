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

#ifndef NB_SEGMENTS_CREATOR_HEADER
#define NB_SEGMENTS_CREATOR_HEADER

#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QtxIntSpinBox;
class QtxComboBox;
class SMESHGUI_SpinBox;
class StdMeshersGUI_DistrTableFrame;
class StdMeshersGUI_DistrPreview;
class QLineEdit;
class QButtonGroup;
class QGridLayout;
class QRadioButton;

typedef struct
{
  int                 myNbSeg, myDistrType, myConv;
  double              myScale;
  SMESH::double_array myTable;
  QString             myName, myExpr;

} NbSegmentsHypothesisData;

class StdMeshersGUI_NbSegmentsCreator : public StdMeshersGUI_StdHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_NbSegmentsCreator();
  virtual ~StdMeshersGUI_NbSegmentsCreator();

  virtual bool checkParams() const;

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
  QtxIntSpinBox*   myNbSeg;
  QtxComboBox*     myDistr;
  SMESHGUI_SpinBox*   myScale;
  StdMeshersGUI_DistrTableFrame*  myTable;
  StdMeshersGUI_DistrPreview* myPreview;
  QLineEdit       *myName, *myExpr;
  QButtonGroup*    myConv;
  QLabel          *myLScale, *myLTable, *myLExpr, *myLConv, *myInfo;
  QGridLayout*     myGroupLayout;
  int              myTableRow, myPreviewRow;
  QRadioButton*    myCutNeg;
};

#endif
