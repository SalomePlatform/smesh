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

// File   : SMESHGUI_SingleEditDlg.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SINGLEEDITDLG_H
#define SMESHGUI_SINGLEEDITDLG_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

class QLineEdit;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;
class LightApp_SelectionMgr;

/*!
 *  Class       : SMESHGUI_SingleEditDlg
 *  Description : Base class for dialogs of diagonal inversion and 
 *                union of two neighboring triangles
 */
class SMESHGUI_EXPORT SMESHGUI_SingleEditDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_SingleEditDlg( SMESHGUI* );
  virtual ~SMESHGUI_SingleEditDlg();

  void                    Init();

protected slots:
  void                    onOk();
  virtual bool            onApply();
  virtual void            reject();
  void                    onHelp();

  void                    onDeactivate();

  void                    onSelectionDone();
  void                    onTextChange( const QString& );

private slots:
  void                    onOpenView();
  void                    onCloseView();


protected:
  void                    enterEvent( QEvent* );
  void                    keyPressEvent( QKeyEvent* );
  QWidget*                createButtonFrame( QWidget* );
  QWidget*                createMainFrame( QWidget* );
  bool                    isValid( const bool ) const;
  bool                    getNodeIds( const QString&, int&, int& ) const;
  virtual bool            process( SMESH::SMESH_MeshEditor_ptr, const int, const int ) = 0;

protected:
  bool                    myBusy;
  QPushButton*            myOkBtn;
  QPushButton*            myApplyBtn;
  QPushButton*            myCloseBtn;
  QPushButton*            myHelpBtn;
  QLineEdit*              myEdge;
  SMESH_Actor*            myActor;

  LightApp_SelectionMgr*  mySelectionMgr;
  SVTK_Selector*          mySelector;
  SMESHGUI*               mySMESHGUI;
  
  QString                 myHelpFileName;
};

/*!
 *  Class       : SMESHGUI_TrianglesInversionDlg
 *  Description : Inversion of the diagonal of a pseudo-quadrangle formed by 
 *                2 neighboring triangles with 1 common edge
 */
class SMESHGUI_TrianglesInversionDlg : public SMESHGUI_SingleEditDlg
{
  Q_OBJECT

public:
  SMESHGUI_TrianglesInversionDlg( SMESHGUI* );
  virtual ~SMESHGUI_TrianglesInversionDlg();

protected:
  virtual bool process( SMESH::SMESH_MeshEditor_ptr, const int, const int );
};

/*!
 *  Class       : SMESHGUI_UnionOfTwoTrianglesDlg
 *  Description : Construction of a quadrangle by deletion of the 
 *                common border of 2 neighboring triangles
 */
class SMESHGUI_UnionOfTwoTrianglesDlg : public SMESHGUI_SingleEditDlg
{
  Q_OBJECT

public:
  SMESHGUI_UnionOfTwoTrianglesDlg( SMESHGUI* );
  virtual ~SMESHGUI_UnionOfTwoTrianglesDlg();

protected:
  virtual bool process( SMESH::SMESH_MeshEditor_ptr, const int, const int );
};

#endif // SMESHGUI_SINGLEEDITDLG_H
