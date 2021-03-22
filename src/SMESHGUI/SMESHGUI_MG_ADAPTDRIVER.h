// Copyright (C) 2020-2021  CEA/DEN, EDF R&D
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

#ifndef SMESHGUI_MG_ADAPTDRIVER_H
#define SMESHGUI_MG_ADAPTDRIVER_H

#include "SMESH_SMESHGUI.hxx"

// model
#include "SMESHGUI_MgAdaptDlg.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(MG_ADAPT)

class SMESHGUI;
class LightApp_SelectionMgr;
//class MgAdapt;

// int IObjectCount();
// const SALOME_ListIO& selectedIO();
// _PTR(Study) getStudy();
// Handle(SALOME_InteractiveObject) firstIObject();
// bool createAndPublishMed(QString fileName);
// bool createMgAdaptObject(MgAdapt* myMgAdapt = 0);


class  SMESHGUI_MG_ADAPTDRIVER : public SMESHGUI_MgAdaptDlg
{
    Q_OBJECT

public :
    SMESHGUI_MG_ADAPTDRIVER( SMESHGUI*, SMESH::MG_ADAPT_ptr, bool isCreation = true );
    void setMyMesh(SMESH::SMESH_Mesh_var);
    SMESH::SMESH_Mesh_var getMyMesh() ;

private :

    SMESHGUI*              mySMESHGUI;              /* Current SMESHGUI object */
    LightApp_SelectionMgr* selMgr ;
    LightApp_SelectionMgr* selectionMgr();
    SMESH::SMESH_Mesh_var  myMesh ;
    bool                   myIsApplyAndClose;

    void                   enterEvent( QEvent* );           /* mouse enter the QWidget */
    void                   keyPressEvent( QKeyEvent* );

    bool                   isValid();
    bool                   createMeshInObjectBrowser();
    void                   setIsApplyAndClose( const bool theFlag );
    bool                   isApplyAndClose() const;
    bool                   execute();


private slots:
    void selectionChanged();
    void updateSelection();

protected slots :

private slots:

    void exportMED(const char* );

    virtual bool PushOnApply();
    virtual void PushOnOK();
    virtual void PushOnHelp();

    void         deactivateActiveDialog();
    void         activateThisDialog();
};

#endif // SMESHGUI_MG_ADAPTDRIVER_H
