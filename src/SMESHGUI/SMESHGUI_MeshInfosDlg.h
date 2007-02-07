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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_MeshInfosDlg.h
//  Author : Nicolas BARBEROU
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_MESHINFOSDLG_H
#define SMESHGUI_MESHINFOSDLG_H

#include "SMESH_SMESHGUI.hxx"

// QT Includes
#include <qdialog.h>

class QGroupBox;
class QLabel;
class QPushButton;
class QWidgetStack;

class LightApp_SelectionMgr;
class SMESHGUI;

class SMESHGUI_EXPORT SMESHGUI_MeshInfosDlg : public QDialog
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
    void keyPressEvent( QKeyEvent* e );
    void windowActivationChange( bool oldActive );
    void DumpMeshInfos();

private slots:
    void onSelectionChanged();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog();
    void onStartSelection();
    void onHelp();

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
    QLabel*       myMeshNbEdges1;
    QLabel*       myMeshNbEdges2;
    QGroupBox*    myMeshFacesGroup;
    QLabel*       myMeshNbFaces;
    QLabel*       myMeshNbFaces1;
    QLabel*       myMeshNbFaces2;
    QLabel*       myMeshNbTriangles;
    QLabel*       myMeshNbTriangles1;
    QLabel*       myMeshNbTriangles2;
    QLabel*       myMeshNbQuadrangles;
    QLabel*       myMeshNbQuadrangles1;
    QLabel*       myMeshNbQuadrangles2;
    QLabel*       myMeshNbPolygones;
    QGroupBox*    myMeshVolumesGroup;
    QLabel*       myMeshNbVolumes;
    QLabel*       myMeshNbVolumes1;
    QLabel*       myMeshNbVolumes2;
    QLabel*       myMeshNbTetra;
    QLabel*       myMeshNbTetra1;
    QLabel*       myMeshNbTetra2;
    QLabel*       myMeshNbHexa;
    QLabel*       myMeshNbHexa1;
    QLabel*       myMeshNbHexa2;
    QLabel*       myMeshNbPyra;
    QLabel*       myMeshNbPyra1;
    QLabel*       myMeshNbPyra2;
    QLabel*       myMeshNbPrism;
    QLabel*       myMeshNbPrism1;
    QLabel*       myMeshNbPrism2;
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

    QGroupBox*    myButtonsGroup;
    QPushButton*  myOkBtn;
    QPushButton*  myHelpBtn;

    QString       myHelpFileName;
};

#endif // SMESHGUI_MESHINFOSDLG_H
