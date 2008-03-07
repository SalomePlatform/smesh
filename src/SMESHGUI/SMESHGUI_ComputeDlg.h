//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_ComputeDlg.h
//  Author : Edward AGAPOV
//  Module : SMESH


#ifndef SMESHGUI_ComputeDlg_H
#define SMESHGUI_ComputeDlg_H

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

#include "VTKViewer.h"

#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(GEOM_Gen)

#include <qgroupbox.h>

class QFrame;
class QPushButton;
class QTable;
class QLabel;
class SMESHGUI_ComputeDlg;
class GEOM_Actor;

namespace SMESH {
  class TShapeDisplayer;
}

/*!
 * \brief Operation to compute a mesh and show computation errors
 */
class SMESHGUI_ComputeOp: public SMESHGUI_Operation
{
  Q_OBJECT

public:
  SMESHGUI_ComputeOp();
  virtual ~SMESHGUI_ComputeOp();

  virtual LightApp_Dialog*       dlg() const;

protected:

  virtual void                   startOperation();
  virtual void                   stopOperation();

protected slots:
  virtual bool                   onApply();

private slots:

  void                           onPreviewShape();
  void                           onPublishShape();
  void                           currentCellChanged();

private:

  QTable*                        table();

  SMESHGUI_ComputeDlg*      myDlg;

  GEOM::GEOM_Object_var     myMainShape;
  SMESH::TShapeDisplayer*   myTShapeDisplayer;
};

/*!
 * \brief Box showing mesh info
 */

class SMESHGUI_MeshInfosBox : public QGroupBox
{
  Q_OBJECT
public:

  SMESHGUI_MeshInfosBox(const bool full, QWidget* theParent);
  void SetInfoByMesh(SMESH::SMESH_Mesh_var mesh);

private:

  bool    myFull;
  QLabel* myNbNode;
  QLabel* myNbEdge,  *myNbLinEdge,  *myNbQuadEdge;
  QLabel* myNbTrai,  *myNbLinTrai,  *myNbQuadTrai;
  QLabel* myNbQuad,  *myNbLinQuad,  *myNbQuadQuad;
  QLabel* myNbFace,  *myNbLinFace,  *myNbQuadFace;
  QLabel* myNbPolyg;
  QLabel* myNbHexa,  *myNbLinHexa,  *myNbQuadHexa;
  QLabel* myNbTetra, *myNbLinTetra, *myNbQuadTetra;
  QLabel* myNbPyra,  *myNbLinPyra,  *myNbQuadPyra;
  QLabel* myNbPrism, *myNbLinPrism, *myNbQuadPrism;
  QLabel* myNbVolum, *myNbLinVolum, *myNbQuadVolum;
  QLabel* myNbPolyh;
};

/*!
 * \brief Dialog to compute a mesh and show computation errors
 */

class SMESHGUI_ComputeDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
                               SMESHGUI_ComputeDlg();

private:

  QFrame*                      createMainFrame   (QWidget*);

  QLabel*                      myMeshName;
  QGroupBox*                   myMemoryLackGroup;
  QGroupBox*                   myCompErrorGroup;
  QGroupBox*                   myHypErrorGroup;
  QLabel*                      myHypErrorLabel;
  QTable*                      myTable;
  QPushButton*                 myShowBtn;
  QPushButton*                 myPublishBtn;

  SMESHGUI_MeshInfosBox*       myBriefInfo;
  SMESHGUI_MeshInfosBox*       myFullInfo;

  friend class SMESHGUI_ComputeOp;

};

#endif
