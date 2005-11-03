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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_SingleEditDlg.h
//  Author : Sergey LITONIN
//  Module : SMESH


#ifndef SMESHGUI_SingleEditDlg_H
#define SMESHGUI_SingleEditDlg_H

#include <qdialog.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QCloseEvent;
class QFrame;
class QLineEdit;
class SMESHGUI_SpinBox;
class QPushButton;

class SMESHGUI;
class SMESH_Actor;
class SVTK_Selector;
class SVTK_ViewWindow;
class LightApp_SelectionMgr;

/*!
 *  Class       : SMESHGUI_SingleEditDlg
 *  Description : Base class for dialogs of diagonal inversion and 
 *                union of two neighboring triangles
 */
class SMESHGUI_SingleEditDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_SingleEditDlg(SMESHGUI* theModule, 
			 const char* theName = 0);
  virtual ~SMESHGUI_SingleEditDlg();

  void Init();

protected slots:
  void                    onOk();
  virtual bool            onApply();
  void                    onClose();

  void                    onDeactivate();

  void                    onSelectionDone();
  void                    onTextChange (const QString&);

protected:
  void                    closeEvent (QCloseEvent*);
  void                    enterEvent (QEvent*);
  void                    hideEvent (QHideEvent*);                        /* ESC key */
  QFrame*                 createButtonFrame (QWidget*);
  QFrame*                 createMainFrame (QWidget*);
  bool                    isValid (const bool) const;
  bool                    getNodeIds (const QString&, int&, int&) const;
  virtual bool            process (SMESH::SMESH_MeshEditor_ptr, const int, const int) = 0;

protected:
  bool                    myBusy;
  QPushButton*            myOkBtn;
  QPushButton*            myApplyBtn;
  QPushButton*            myCloseBtn;
  QLineEdit*              myEdge;
  SMESH_Actor*            myActor;

  LightApp_SelectionMgr*  mySelectionMgr;
  SVTK_Selector*          mySelector;
  SMESHGUI*               mySMESHGUI;
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
  SMESHGUI_TrianglesInversionDlg(SMESHGUI* theModule, 
				 const char* theName = 0);
  virtual ~SMESHGUI_TrianglesInversionDlg();

protected:
  virtual bool process (SMESH::SMESH_MeshEditor_ptr, const int, const int);
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
  SMESHGUI_UnionOfTwoTrianglesDlg(SMESHGUI* theModule, 
				  const char* theName = 0);
  virtual ~SMESHGUI_UnionOfTwoTrianglesDlg();

protected:
  virtual bool process (SMESH::SMESH_MeshEditor_ptr, const int, const int);
};

#endif
