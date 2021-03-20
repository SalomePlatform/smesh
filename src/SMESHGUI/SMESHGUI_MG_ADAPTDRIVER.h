// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

// SMESH SMESHGUI : GUI for the adaptation in the SMESH component
// File : SMESHGUI_MG_ADAPTDRIVER.h
//
#ifndef SMESHGUI_MG_ADAPTDRIVER_H
#define SMESHGUI_MG_ADAPTDRIVER_H

#include <set>
// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QDialog>
#include <QTreeWidget>
#include<QItemDelegate>

#include <QThread>

#include "LightApp_DataOwner.h"
#include "SalomeApp_Application.h"
#include <SALOMEconfig.h>
#include <SALOME_ListIO.hxx>
#include "SalomeApp_Module.h"
#include "SalomeApp_Study.h"
#include <med.h>
#include <QObject>
// model
#include "MG_ADAPTGUI.hxx"

#include CORBA_SERVER_HEADER(MG_ADAPT)

class SUIT_ViewWindow;
class SUIT_Desktop;
class SUIT_Study;
class SUIT_ResourceMgr;

class CAM_Module;

class SALOMEDSClient_Study;
class SALOMEDSClient_SObject;

class SalomeApp_Study;
class SalomeApp_Module;
class LightApp_SelectionMgr;
class SUIT_SelectionFilter;


class QButtonGroup;
class QLineEdit;
class QGroupBox;
class QRadioButton;
class QLabel;
class QCheckBox;
class QGridLayout;
class QTabWidget;
class QDoubleSpinBox;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;
class QSpacerItem;
class QHBoxLayout;
class QItemDelegate;
class QComboBox;
class QObject;


// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Gen)
class SMESHGUI;
class SMESHGUI_MgAdaptDlg;
class SMESHGUI_IdValidator;
class SMESHGUI_FilterDlg;
class MgAdapt;
class QHeaderView;
class QFileDialog;


int IObjectCount();
const SALOME_ListIO& selectedIO();
_PTR(Study) getStudy();
Handle(SALOME_InteractiveObject) firstIObject();
bool createAndPublishMed(QString fileName);
bool createMgAdaptObject(MgAdapt* myMgAdapt = 0);


class  SMESHGUI_MG_ADAPTDRIVER : public SMESHGUI_MgAdaptDlg
{
    Q_OBJECT;

public :
    SMESHGUI_MG_ADAPTDRIVER( SMESHGUI*, SMESH::MG_ADAPT_ptr, bool isCreation = true );
    void setMyMesh(SMESH::SMESH_Mesh_var);
    SMESH::SMESH_Mesh_var getMyMesh() ;

private :

    SMESHGUI*                     mySMESHGUI;              /* Current SMESHGUI object */
    LightApp_SelectionMgr* selMgr ;
    SUIT_ResourceMgr* resMgr;
    SUIT_ResourceMgr* resourceMgr();
    LightApp_SelectionMgr* selectionMgr();
    SMESH::SMESH_Mesh_var myMesh ;



    void                   Init( bool = true );
    void                   enterEvent( QEvent* );           /* mouse enter the QWidget */
    void                   keyPressEvent( QKeyEvent* );
    QString                getErrorMsg( SMESH::string_array_var invalidEntries,
                                        QStringList &           entriesToBrowse );

    bool                   isValid();
    bool                   createMeshInObjectBrowser();
    void                   setIsApplyAndClose( const bool theFlag );
    bool                   isApplyAndClose() const;
    bool                   execute();
    SMESHGUI_IdValidator*  myIdValidator;
    int                    myNbOkElements;          /* to check when elements are defined */

    SVTK_Selector*         mySelector;

    bool                   myBusy;
    GEOM::GEOM_Object_var  myNewGeometry;
    //~SMESH_Actor*           myActor;  //
    SUIT_SelectionFilter*  myIdSourceFilter;

    SMESH::SMESH_IDSource_var mySelectedObject;

    QTabWidget*           myTabWidget;
    QButtonGroup*          GroupConstructors;

    QGroupBox*             ConstructorsBox;
    QGroupBox*             GroupArguments;
    QGroupBox*             GroupButtons;

    QPushButton*           buttonOk;
    QPushButton*           buttonCancel;
    QPushButton*           buttonApply;
    QPushButton*           buttonHelp;

    QLabel*                myTextLabelElements;
    QLabel*                myGeomLabel;
    QLineEdit*             myLineEditElements;
    QLineEdit*             myMeshNameEdit;
    QLineEdit*             myGeomNameEdit;
    QCheckBox*             myIdSourceCheck;
    QCheckBox*             myCopyGroupsCheck;
    QCheckBox*             myReuseHypCheck;
    QCheckBox*             myCopyElementsCheck;
    QCheckBox*             myKeepIdsCheck;

    QPushButton*           myFilterBtn;
    SMESHGUI_FilterDlg*    myFilterDlg;

    QString                myHelpFileName;

    bool                   myIsApplyAndClose;

    QString             inputMeshName;
    QString           outputMeshName;
private slots:
    void selectionChanged();
    void updateSelection();

protected slots :

private slots:

    void exportMED(const char* );

    virtual bool PushOnApply();
    virtual void PushOnOK();
    virtual void PushOnHelp();

    void                   deactivateActiveDialog();
    void                   activateThisDialog();
    void                   onConstructor( int );
    //~void                   onTextChange( const QString& );
    //~void                   onSelectIdSource( bool );
    void                   setFilters();
    void                   onOpenView();
    void                   onCloseView();


};




#endif // SMESHGUI_MG_ADAPTDRIVER_H
