// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESHGUI : GUI for SMESH component
//  File   : SMESHGUI_GroupOnShapeDlg.h
//  Author : Edard AGAPOV
//  Module : SMESH
//
#ifndef SMESHGUI_GroupOnShapeDlg_H_H
#define SMESHGUI_GroupOnShapeDlg_H_H

#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_SelectionOp.h"

class QLineEdit;
class QListWidget;
class QPushButton;
class SMESHGUI_GroupOnShapeDlg;

//=================================================================================
// class    : SMESHGUI_GroupOnShapeOp
// purpose  : create groups on shapes of nodes and element at once, Issue 19970
//=================================================================================

class SMESHGUI_EXPORT SMESHGUI_GroupOnShapeOp : public SMESHGUI_SelectionOp
{
    Q_OBJECT

public:
    SMESHGUI_GroupOnShapeOp();
    ~SMESHGUI_GroupOnShapeOp();

    virtual LightApp_Dialog*    dlg() const;  
    static QString              GetDefaultName(const QString& theOperation);

public slots:

protected:
  
  virtual void                  startOperation();
  virtual void                  selectionDone();
  virtual SUIT_SelectionFilter* createFilter( const int ) const;
  //virtual bool                  isValid( SUIT_Operation* ) const;

private slots:

    bool                        onApply();
    void                        onButtonClick();


//     void                        onSelectColor();


private:

    void                        init();
//     void                        setGroupColor( const SALOMEDS::Color& );
//     SALOMEDS::Color             getGroupColor() const;

//     void                        setGroupQColor( const QColor& );
//     QColor                      getGroupQColor() const;

//     void                        setDefaultGroupColor();

private:

  SMESHGUI_GroupOnShapeDlg*     myDlg;

  QString                       myMeshID;
  QStringList                   myElemGeoIDs, myNodeGeoIDs;
  //GEOM::ListOfGO_var            myElemGObj;
};

class SMESHGUI_EXPORT SMESHGUI_GroupOnShapeDlg : public SMESHGUI_Dialog
{
    Q_OBJECT
    
public:
    SMESHGUI_GroupOnShapeDlg();
    virtual ~SMESHGUI_GroupOnShapeDlg();

public slots:

  void                          updateButtons();
  void                          init();

private:

  //QLineEdit*                    myGrpNameLine;

  QPushButton*                  myMeshBtn;
  QLineEdit*                    myMeshLine;

  QPushButton*                  myElemGeomBtn;
  QListWidget*                     myElemGeomList;

  QPushButton*                  myNodeGeomBtn;
  QListWidget*                     myNodeGeomList;

//   QPushButton*                  myColorBtn;

//   bool                          myCreate, myIsBusy;

//   QString                       myHelpFileName;

  friend class SMESHGUI_GroupOnShapeOp;
};

#endif // SMESHGUI_GroupOnShapeDlg_H_H
