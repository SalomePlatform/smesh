// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QCheckBox;
class QLineEdit;
class SMESHGUI_SpinBox;
class SMESHGUI_ReorientFacesDlg;
class SMESH_TypeFilter;

/*! ================================================================================
 * \brief Operation to reorient faces according to some criterion
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
  void                           setRefFiltersByConstructor();
  int                            constructorID();


 protected slots:
  virtual bool                   onApply();

 private slots:
  virtual void                   onActivateObject( int );
  void                           redisplayPreview();
  void                           onTextChange( const QString& );
  bool                           onlyOneObjAllowed();

 private:

  SMESHGUI_ReorientFacesDlg* myDlg;

  SMESH_Actor*               myObjectActor;
  int                        mySelectionMode;

  SMESH_TypeFilter*          myRefGroupFilter;
  SMESH_TypeFilter*          myRefSubMeshFilter;
  SMESH_TypeFilter*          myRefMeshFilter;

  SMESH::ListOfIDSources_var myObjects;
  SMESH::ListOfIDSources_var myRefGroups;
};

/*! ================================================================================
 * \brief Dialog to reorient faces according to vector
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
  void                          setLabel( int object, const char* text );
  
  QButtonGroup*                 myConstructorGrp;
  QFrame*                       myFaceFrm;
  QFrame*                       myPointFrm;
  QFrame*                       myDirFrm;
  QFrame*                       myRefGroupFrm;
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
