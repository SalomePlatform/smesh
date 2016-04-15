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
// File   : SMESHGUI_RotationDlg.h
// Author : Roman NIKOLAEV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_PREVIEWDLG_H
#define SMESHGUI_PREVIEWDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class SMESHGUI;
class SMESHGUI_MeshEditPreview;
class QCheckBox;

class SMESHGUI_EXPORT SMESHGUI_PreviewDlg : public QDialog {
  Q_OBJECT
public:
  SMESHGUI_PreviewDlg( SMESHGUI* );
  ~SMESHGUI_PreviewDlg();

  void showPreview();
  void hidePreview();

protected:
  void connectPreviewControl();

  virtual void setIsApplyAndClose( const bool theFlag );
  virtual bool isApplyAndClose() const;

protected slots:
 void                      toDisplaySimulation();
 void                      onCloseView();
 void                      onOpenView();
 virtual void              onDisplaySimulation( bool );
  
protected:
  SMESHGUI*                 mySMESHGUI;              /* Current SMESHGUI object */  
  SMESHGUI_MeshEditPreview* mySimulation;
  QCheckBox*                myPreviewCheckBox;
  bool                      myIsApplyAndClose;
};

class SMESHGUI_EXPORT SMESHGUI_MultiPreviewDlg : public QDialog {
  Q_OBJECT
public:
  SMESHGUI_MultiPreviewDlg( SMESHGUI* );
  ~SMESHGUI_MultiPreviewDlg();

  void showPreview();
  void hidePreview();

  void setSimulationPreview( QList<SMESH::MeshPreviewStruct_var>& );

protected:
  void connectPreviewControl();

  virtual void setIsApplyAndClose( const bool theFlag );
  virtual bool isApplyAndClose() const;

protected slots:
 void                      toDisplaySimulation();
 void                      onCloseView();
 virtual void              onDisplaySimulation( bool );

  
protected:
  SMESHGUI*                        mySMESHGUI;              /* Current SMESHGUI object */  
  QList<SMESHGUI_MeshEditPreview*> mySimulationList;
  QCheckBox*                       myPreviewCheckBox;
  bool                             myIsApplyAndClose;
};

#endif
