//  SMESH SMESHGUI : GUI for SMESH component
//
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
//  File   : SMESHGUI_MeshInfosDlg.h
//  Author : Nicolas BARBEROU
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_MESHINFOSDLG_H
#define SMESHGUI_MESHINFOSDLG_H

// QT Includes
#include <qdialog.h>

class QGroupBox;
class QLabel;
class QPushButton;
class QWidgetStack;

class LightApp_SelectionMgr;
class SMESHGUI;

class SMESHGUI_MeshInfosDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_MeshInfosDlg( SMESHGUI* theModule, 
			   const char* name = 0, 
			   bool modal = FALSE, 
			   WFlags fl = 0 );
    ~SMESHGUI_MeshInfosDlg();

protected:
    void closeEvent( QCloseEvent* e );
    void windowActivationChange( bool oldActive );
    void DumpMeshInfos();

private slots:
    void onSelectionChanged();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog();
    void onStartSelection();

private:
    SMESHGUI*               mySMESHGUI;
    LightApp_SelectionMgr*  mySelectionMgr; 
    bool                    myStartSelection;
    bool                    myIsActiveWindow;

    QPushButton*  mySelectBtn;
    QLabel*       mySelectLab;

    QWidgetStack* myWGStack;

    QWidget*      myMeshWidget;
    QLabel*       myMeshName;
    QLabel*       myMeshNbNodes;
    QLabel*       myMeshNbEdges;
    QGroupBox*    myMeshFacesGroup;
    QLabel*       myMeshNbFaces;
    QLabel*       myMeshNbTriangles;
    QLabel*       myMeshNbQuadrangles;
    QLabel*       myMeshNbPolygones;
    QGroupBox*    myMeshVolumesGroup;
    QLabel*       myMeshNbVolumes;
    QLabel*       myMeshNbTetra;
    QLabel*       myMeshNbHexa;
    QLabel*       myMeshNbPyra;
    QLabel*       myMeshNbPrism;
    QLabel*       myMeshNbPolyhedrones;

    QWidget*      mySubMeshWidget;
    QLabel*       mySubMeshName;
    QLabel*       mySubMeshNbNodes;
    QGroupBox*    mySubMeshElementsGroup;
    QLabel*       mySubMeshNbElements;
    QLabel*       mySubMeshNbEdges;
    QLabel*       mySubMeshNbFaces;
    QLabel*       mySubMeshNbVolumes;

    QWidget*      myGroupWidget;
    QLabel*       myGroupName;
    QLabel*       myGroupType;
    QLabel*       myGroupNb;

    QGroupBox*   myButtonsGroup;
    QPushButton* myOkBtn;
};

#endif // SMESHGUI_MESHINFOSDLG_H
