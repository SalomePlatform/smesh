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

// File   : SMESHGUI_ReorientFacesDlg.h
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_ReorientFacesDlg_H
#define SMESHGUI_ReorientFacesDlg_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_SelectionOp.h"

class QButtonGroup;
class QCheckBox;
class QLineEdit;
class SMESHGUI_SpinBox;
class SMESHGUI_ReorientFacesDlg;

/*!
 * \brief Operation to reorient faces acoording to some criterion
 */
class SMESHGUI_EXPORT SMESHGUI_ReorientFacesOp: public SMESHGUI_SelectionOp
{
  Q_OBJECT

public:
  SMESHGUI_ReorientFacesOp();
  virtual ~SMESHGUI_ReorientFacesOp();

  virtual LightApp_Dialog*       dlg() const;

protected:

  virtual void                   startOperation();
  virtual void                   stopOperation();

  virtual SUIT_SelectionFilter*  createFilter( const int ) const;
  virtual void                   selectionDone();

  bool                           isValid( QString& );

protected slots:
  virtual bool                   onApply();

private slots:
  virtual void                   onActivateObject( int );
  void                           redisplayPreview();
  void                           onTextChange( const QString& );

private:
  SMESHGUI_ReorientFacesDlg*    myDlg;

  //SMESHGUI_MeshEditPreview*     myVectorPreview;
  SMESH_Actor*                  myObjectActor;
  int                           mySelectionMode;

  SMESH::SMESH_IDSource_var     myObject;
  SMESH::SMESH_IDSource_var     myVolumeObj;
};

/*!
 * \brief Dialog to reorient faces acoording to vector
 */

class SMESHGUI_EXPORT SMESHGUI_ReorientFacesDlg : public SMESHGUI_Dialog
{
  Q_OBJECT

public:
  SMESHGUI_ReorientFacesDlg();

public slots:
  void constructorChange(int id);

private:
  QWidget*                      createMainFrame( QWidget* );
  
  QButtonGroup*                 myConstructorGrp;
  QFrame*                       myFaceFrm;
  QFrame*                       myPointFrm;
  QFrame*                       myDirFrm;
  QFrame*                       myVolumFrm;
  QCheckBox*                    myOutsideChk;
  SMESHGUI_SpinBox*             myX;
  SMESHGUI_SpinBox*             myY;
  SMESHGUI_SpinBox*             myZ;
  //QPushButton*                  myIdBtn;
  //QLineEdit*                    myId;
  SMESHGUI_SpinBox*             myDX;
  SMESHGUI_SpinBox*             myDY;
  SMESHGUI_SpinBox*             myDZ;

  QString                       myHelpFileName;

  friend class SMESHGUI_ReorientFacesOp;

  //private slots:
  //void                          ButtonToggled( bool );
};

#endif // SMESHGUI_ReorientFacesDlg_H
