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
 * \brief Dialog to compute a mesh and show computation errors
 */

class SMESHGUI_ComputeDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
                               SMESHGUI_ComputeDlg();

  void                         SetMeshInfo(int nbNodes, int nbEdges, int nbFaces, int nbVolums);

private:

  QFrame*                      createMainFrame   (QWidget*);

  QTable*                      myTable;
  QPushButton*                 myShowBtn;
  QPushButton*                 myPublishBtn;

  QLabel*                      myNbNodesLabel;
  QLabel*                      myNbEdgesLabel;
  QLabel*                      myNbFacesLabel;
  QLabel*                      myNbVolumLabel;

  QString                      myHelpFileName;

  friend class SMESHGUI_ComputeOp;

};

#endif
