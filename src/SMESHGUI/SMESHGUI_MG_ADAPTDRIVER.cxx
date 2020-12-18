// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
// File : SMESHGUI_MG_ADAPTDRIVER.cxx

#include "SMESHGUI_MG_ADAPTDRIVER.h"

#include "SUIT_Desktop.h"
#include "SUIT_Application.h"
#include "SUIT_Session.h"

#include "SalomeApp_Application.h"
#include "SalomeApp_Module.h"
#include "SalomeApp_Study.h"

#include "SMESH_Comment.hxx"
#include "SMESH_Actor.h"
#include "SMESHGUI.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_Selection.h"
#include <SUIT_MessageBox.h>
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_VTKUtils.h"
#include <SMESH_TypeFilter.hxx>
#include <SMESH_MeshAlgos.hxx>
#include <SMESH_LogicalFilter.hxx>
#include <SMDS_Mesh.hxx>
#include <SMDS_MeshNode.hxx>
#include "SMESHGUI_SpinBox.h"

#include <LightApp_SelectionMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_FileDlg.h>
#include "SMESHGUI_MeshUtils.h"


#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSpacerItem>
#include <QString>
#include <QHeaderView>
#include <QItemDelegate>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>

#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkDataSetMapper.h>
#include <VTKViewer_CellLocationsArray.h>
#include <vtkProperty.h>

#include <ElCLib.hxx>
// SALOME KERNEL includes
#include <SALOMEDS_SComponent.hxx>
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDS_Study.hxx>
#include <SALOMEDS_wrap.hxx>
#include "SalomeApp_Tools.h"
#include <SALOMEconfig.h>
#include <med.h>
#include <utilities.h>

#include <TCollection_AsciiString.hxx>

const int SPACING = 6;            // layout spacing
const int MARGIN  = 9;            // layout margin

SALOME_ListIO mySelected;


//================================================================
// Function : firstIObject
// Purpose  :  Return the first selected object in the selected object list
//================================================================
Handle(SALOME_InteractiveObject) firstIObject()
{
    const SALOME_ListIO& aList = selectedIO();
    return aList.Extent() > 0 ? aList.First() : Handle(SALOME_InteractiveObject)();
}
//================================================================
// Function : selectedIO
// Return the list of selected SALOME_InteractiveObject's
//================================================================
const SALOME_ListIO& selectedIO()
{
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* > ( SUIT_Session::session()->activeApplication() );
    LightApp_SelectionMgr* aSelectionMgr = app->selectionMgr();
    if( aSelectionMgr )
    {
        aSelectionMgr->selectedObjects( mySelected );
        for (SALOME_ListIteratorOfListIO it (mySelected); it.More(); it.Next())
            SCRUTE(it.Value()->getEntry());
    };
    return mySelected;
}
//================================================================
// Function : getStudy
// Returne un pointeur sur l'etude active
//================================================================
_PTR(Study) getStudy()
{
    static _PTR(Study) _study;
    if(!_study)
        _study = SalomeApp_Application::getStudy();
    return _study;
}

bool createAndPublishMed(QString fileName)
{

    SMESH::DriverMED_ReadStatus res;
    SMESH::mesh_array_var aMeshes = new SMESH::mesh_array;
    // SMESHGUI aGui;

    aMeshes = SMESHGUI::GetSMESHGen()->CreateMeshesFromMED( fileName.toUtf8().constData(), res );
    _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshes[0] );
    _PTR(Study) aStudy = SMESH::getStudy();
    QStringList anEntryList;
    // bool isEmpty;
    if ( aMeshSO ) {
        _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
        _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" );
        aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" );
        anEntryList.append( aMeshSO->GetID().c_str() );
    }
    else {
        // isEmpty = true;
        return false;
    }
    SMESHGUI::GetSMESHGUI()->updateObjBrowser();

    // browse to the published meshes
    if( LightApp_Application* anApp =
                dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
        anApp->browseObjects( anEntryList );
    return true;
}
bool createMgAdaptObject(MgAdapt *myMgAdapt )
{
    // SMESH::SMESH_Mesh_var newMesh = SMESHGUI::GetSMESHGen()->CreateEmptyMesh();

    // _PTR(SObject) aHypothesis;
    _PTR(Study) aStudy = SMESH::getStudy();
    QStringList anEntryList;
    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
    _PTR(SComponent) mgadapt = aStudy->FindComponent("MG-ADAPT");
    _PTR(GenericAttribute) ga;
    if (!aBuilder->FindAttribute(mgadapt, ga, "AttributeName") )
    {
        mgadapt = aBuilder->NewComponent("MG-ADAPT");
        _PTR(AttributeName) Name = aBuilder->FindOrCreateAttribute(mgadapt, "AttributeName");
        Name->SetValue("MG-ADAPT");
        _PTR(AttributePixMap) myPixmap = aBuilder->FindOrCreateAttribute( mgadapt, "AttributePixMap" );
        myPixmap->SetPixMap( "ICON_MG_ADAPT" );
        anEntryList.append( mgadapt->GetID().c_str() );
    }

    _PTR(SObject) obj =  aBuilder->NewObject(mgadapt);
    _PTR(AttributeName) myName = aBuilder->FindOrCreateAttribute(obj, "AttributeName");
    myName->SetValue("hypo");
    _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( obj, "AttributePixMap" );
    aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO" );
    anEntryList.append( obj->GetID().c_str() );

    SMESHGUI::GetSMESHGUI()->updateObjBrowser();

    // // browse to the published meshes
    if( LightApp_Application* anApp =
                dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
        anApp->browseObjects( anEntryList );
    return true;
}


// MG ADAPT UTILS
//================================================================
// Function : IObjectCount
// Return the number of selected objects
//================================================================
int IObjectCount()
{
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
    LightApp_SelectionMgr* aSelectionMgr = app->selectionMgr();
    if( aSelectionMgr )
    {
        aSelectionMgr->selectedObjects( mySelected );
        SCRUTE(mySelected.Extent());
        return mySelected.Extent();
    }
    return 0;
}


SMESHGUI_MG_AdaptComputeDlg_QThread::SMESHGUI_MG_AdaptComputeDlg_QThread(MgAdapt* aModel)
{
    model = aModel;
    myResult = -1;
}

void SMESHGUI_MG_AdaptComputeDlg_QThread::run()
{

    int err;
    std::string errStr;
    errStr = model->compute(errStr);
    std::string msg = err == 0 ? " ok" : std::string("Not ok \n")+ errStr;
    exec();
}

int SMESHGUI_MG_AdaptComputeDlg_QThread::result()
{
    return myResult;
}

void SMESHGUI_MG_AdaptComputeDlg_QThread::cancel()
{
    //~model->cancel();
}

SMESHGUI_MG_ADAPTDRIVER::SMESHGUI_MG_ADAPTDRIVER( SMESHGUI* theModule, MgAdapt* myModel, bool isCreation )
    : mySMESHGUI( theModule ),
      myFilterDlg(0),
      myIsApplyAndClose( false ),
      SMESHGUI_MgAdaptDlg((SalomeApp_Module*)theModule, myModel, SMESHGUI::desktop(), isCreation)
{

    resMgr = resourceMgr();

    selMgr = selectionMgr();

    // connections
    connect(myArgs, SIGNAL(updateSelection()), this, SLOT(updateSelection()));
    connect(myArgs, SIGNAL(toExportMED(const char*)), this, SLOT(exportMED(const char*)));
}

SUIT_ResourceMgr* SMESHGUI_MG_ADAPTDRIVER::resourceMgr()
{
    return dynamic_cast<SUIT_ResourceMgr*>( SUIT_Session::session()->resourceMgr() );
}

LightApp_SelectionMgr* SMESHGUI_MG_ADAPTDRIVER::selectionMgr()
{
    SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
    if( anApp )
        return dynamic_cast<LightApp_SelectionMgr*>( anApp->selectionMgr() );
    else
        return 0;
}

void SMESHGUI_MG_ADAPTDRIVER::updateSelection()
{
    disconnect( selMgr, 0, this, 0 );
    selMgr->clearFilters();

    SMESH::SetPointRepresentation( true );
    if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
        aViewWindow->SetSelectionMode( ActorSelection );
    if (myArgs->aBrowser->isChecked())
    {
        connect( selMgr,  SIGNAL( currentSelectionChanged() ), this, SLOT( selectionChanged() ));
        selectionChanged();
    }

}
void SMESHGUI_MG_ADAPTDRIVER::selectionChanged()
{
    //~ get selected mesh
    SALOME_ListIO aList;
    selMgr->selectedObjects(aList);
    QString aString = "";
    int nbSel = aList.Extent();
    if (nbSel != 1)
        return;

    Handle(SALOME_InteractiveObject) IO = aList.First();
    SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO(IO);
    if ( !mesh->_is_nil() )
    {
        myMesh  = mesh;

        mySelectedObject = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
        if ( mySelectedObject->_is_nil() )
            return;

    }
    else
        return;

    SMESH::GetNameOfSelectedIObjects( selMgr, aString );
    if ( aString.isEmpty() ) aString = " ";
    else                     aString = aString.trimmed();


    bool ok = !aString.isEmpty();
    if ( !mesh->_is_nil() )
    {
        myArgs->aBrowserObject->setText( aString );
        myArgs->meshNameLineEdit->setText( aString );
        myArgs->selectOutMedFileLineEdit->setText(aString+QString(".med"));
        ADAPTATION_MODE aMode;
        int nbVolumes = myMesh->NbVolumes();
        int nbFaces = myMesh->NbFaces();
        if(nbFaces > 0 && nbVolumes > 0) aMode = ADAPTATION_MODE::BOTH;
        else if(nbFaces > 0) aMode = ADAPTATION_MODE::SURFACE;
        else aMode = ADAPTATION_MODE::VOLUME;
        emit myArgs->meshDimSignal(aMode);
    }

}
void SMESHGUI_MG_ADAPTDRIVER::exportMED(const char* tmp_file)
{
    bool toOverwrite  = true;
    bool toFindOutDim = true;
    myMesh->ExportMED(tmp_file, false, -1, toOverwrite, toFindOutDim);
}
void SMESHGUI_MG_ADAPTDRIVER::setMyMesh(SMESH::SMESH_Mesh_var mesh)
{
    myMesh = mesh;
}
SMESH::SMESH_Mesh_var SMESHGUI_MG_ADAPTDRIVER::getMyMesh()
{
    return myMesh;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::clickOnOk()
{
    setIsApplyAndClose( true );
    clickOnApply();
    reject();
}
bool SMESHGUI_MG_ADAPTDRIVER::clickOnApply()
{

    if ( SMESHGUI::isStudyLocked() )
        return false;
    if( !isValid() )
        return false;

    SMESHGUI_MgAdaptDlg::clickOnApply();

    bool ok = execute();
    //~SMESHGUI_MG_AdaptComputeDlg_QThread atest(getModel());
    //~atest.start();
    //~atest.quit();
    if (getModel()->getPublish()) this->createMeshInObjectBrowser();

    return ok;
}

bool SMESHGUI_MG_ADAPTDRIVER::execute()
{

    int err;
    std::string errStr;
    try
    {
        err = getModel()->compute(errStr);
        std::string msg =  err == 0 ? " ok" : std::string("Not ok \n")+errStr ;
    }
    catch (const std::exception& e)
    {
        std::cerr<<e.what();
    }
    return err == 0? true: false;
}

//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::Init (bool ResetControls)
{
    myBusy = false;

    if ( ResetControls )
    {
        myLineEditElements->clear();
        myNbOkElements = 0;

        buttonOk->setEnabled(false);
        buttonApply->setEnabled(false);

        //~myActor = 0;
        myMesh = SMESH::SMESH_Mesh::_nil();

        myIdSourceCheck->setChecked(true);

        onConstructor( 0 );
    }

}

//=======================================================================
//function : onConstructor
//purpose  : switch operation mode
//=======================================================================

void SMESHGUI_MG_ADAPTDRIVER::onConstructor( int withGeom )
{

    myGeomLabel        ->setVisible( withGeom );
    myGeomNameEdit     ->setVisible( withGeom );
    myReuseHypCheck    ->setVisible( withGeom );
    myCopyElementsCheck->setVisible( withGeom );
    myFilterBtn        ->setVisible( !withGeom );
    myIdSourceCheck    ->setVisible( !withGeom );

    if ( !withGeom )
        myMeshNameEdit->setText( SMESH::UniqueMeshName("Mesh"));

}


//~void SMESHGUI_MG_ADAPTDRIVER::onSelectIdSource( bool )
//~{}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::enterEvent (QEvent*)
{

    // if ( !ConstructorsBox->isEnabled() ) {
    //   SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    //   if ( aViewWindow && !mySelector ) {
    //     mySelector = aViewWindow->GetSelector();
    //   }
    //   activateThisDialog();
    // }

}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::keyPressEvent( QKeyEvent* e )
{

    QDialog::keyPressEvent( e );
    if ( e->isAccepted() )
        return;

    if ( e->key() == Qt::Key_F1 ) {
        e->accept();
        clickOnHelp();
    }

}

//=================================================================================
// function : clickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::clickOnHelp()
{

    LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
    if (app)
        app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
    else {
        QString platform;
#ifdef WIN32
        platform = "winapplication";
#else
        platform = "application";
#endif
        SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                                 tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                                 arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                         platform)).
                                 arg(myHelpFileName));
    }

}

//=======================================================================
//function : getErrorMsg
//purpose  : Return an error message and entries of invalid smesh object
//=======================================================================

QString SMESHGUI_MG_ADAPTDRIVER::getErrorMsg( SMESH::string_array_var theInvalidEntries,
        QStringList &           theEntriesToBrowse )
{

    if ( theInvalidEntries->length() == 0 )
        return tr("OPERATION_FAILED");

    // theInvalidEntries - SObject's that hold geometry objects whose
    // counterparts are not found in the newGeometry, followed by SObject's
    // holding mesh sub-objects that are invalid because they depend on a not found
    // preceding sub-shape

    QString msg = tr("SUBSHAPES_NOT_FOUND_MSG") + "\n";

    QString objString;
    for ( CORBA::ULong i = 0; i < theInvalidEntries->length(); ++i )
    {
        _PTR(SObject) so = SMESH::getStudy()->FindObjectID( theInvalidEntries[i].in() );

        int objType = SMESHGUI_Selection::type( theInvalidEntries[i].in() );
        if ( objType < 0 ) // geom object
        {
            objString += "\n";
            if ( so )
                objString += so->GetName().c_str();
            else
                objString += theInvalidEntries[i].in(); // it's something like "FACE #2"
        }
        else // smesh object
        {
            theEntriesToBrowse.push_back( theInvalidEntries[i].in() );

            objString += "\n   ";
            switch ( objType ) {
            case SMESH::MESH:
                objString += tr("SMESH_MESH");
                break;
            case SMESH::HYPOTHESIS:
                objString += tr("SMESH_HYPOTHESIS");
                break;
            case SMESH::ALGORITHM:
                objString += tr("SMESH_ALGORITHM");
                break;
            case SMESH::SUBMESH_VERTEX:
            case SMESH::SUBMESH_EDGE:
            case SMESH::SUBMESH_FACE:
            case SMESH::SUBMESH_SOLID:
            case SMESH::SUBMESH_COMPOUND:
            case SMESH::SUBMESH:
                objString += tr("SMESH_SUBMESH");
                break;
            case SMESH::GROUP:
                objString += tr("SMESH_GROUP");
                break;
            default:
                ;
            }
            objString += " \"";
            if ( so )
                objString += so->GetName().c_str();
            objString += "\" (";
            objString += theInvalidEntries[i].in();
            objString += ")";
        }
    }
    if ( !objString.isEmpty() )
        msg += objString;

    return msg;
}

//=================================================================================
// function : isValid
// purpose  :
//=================================================================================

bool SMESHGUI_MG_ADAPTDRIVER::isValid()
{
    bool ok = true;
    return ok;
}

bool SMESHGUI_MG_ADAPTDRIVER::createMeshInObjectBrowser()
{
    QString filename(getModel()->getMedFileOut().c_str());
    QStringList errors;
    QStringList anEntryList;
    bool isEmpty = false;
    bool ok = false;
    SMESH::SMESH_Gen_var SMESH_Gen_ptr = SMESHGUI::GetSMESHGen();
    if (!SMESH_Gen_ptr) {
        std::cerr << "Could not retrieve SMESH_Gen_ptr" << std::endl;
        throw SALOME_Exception(LOCALIZED("Could not retrieve SMESH::GetSMESHGen()"));
    }
    SMESH::mesh_array_var aMeshes = new SMESH::mesh_array;
    aMeshes->length( 1 ); // one mesh only
    SMESH::DriverMED_ReadStatus res;
    aMeshes = SMESH_Gen_ptr->CreateMeshesFromMED( filename.toUtf8().constData(), res );
    if ( res != SMESH::DRS_OK ) {
        errors.append( QString( "%1 :\n\t%2" ).arg( filename ).arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
    }
    _PTR(Study) aStudy = SMESH::getStudy();
    for ( int i = 0, iEnd = aMeshes->length(); i < iEnd; i++ )
    {
        _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshes[i] );
        if ( aMeshSO ) {
            _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
            _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" );
            aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" ); // put REFINED mesh ico
            anEntryList.append( aMeshSO->GetID().c_str() );
        }
        else {
            isEmpty = true;
        }
    }
    // update Object browser
    SMESHGUI::GetSMESHGUI()->updateObjBrowser();
    // browse to the published meshes
    if( LightApp_Application* anApp =
                dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
        anApp->browseObjects( anEntryList );

    // show Error message box if there were errors
    if ( errors.count() > 0 ) {
        SUIT_MessageBox::critical( SMESHGUI::desktop(),
                                   QObject::tr( "SMESH_ERROR" ),
                                   QObject::tr( "SMESH_IMPORT_ERRORS" ) + "\n" + errors.join( "\n" ) );
    }

    // show warning message box, if some imported mesh is empty
    if ( isEmpty ) {
        SUIT_MessageBox::warning( SMESHGUI::desktop(),
                                  QObject::tr( "SMESH_WRN_WARNING" ),
                                  QObject::tr( "SMESH_DRS_SOME_EMPTY" ) );
    }
    return true;
}

//================================================================
// function : setIsApplyAndClose
// Purpose  : Set value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
void SMESHGUI_MG_ADAPTDRIVER::setIsApplyAndClose( const bool theFlag )
{
    myIsApplyAndClose = theFlag;
}//================================================================
// function : isApplyAndClose
// Purpose  : Get value of the flag indicating that the dialog is
//            accepted by Apply & Close button
//================================================================
bool SMESHGUI_MG_ADAPTDRIVER::isApplyAndClose() const
{
    return myIsApplyAndClose;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::deactivateActiveDialog()
{

    if (ConstructorsBox->isEnabled()) {
        ConstructorsBox->setEnabled(false);
        GroupArguments->setEnabled(false);
        GroupButtons->setEnabled(false);
        mySMESHGUI->ResetState();
        mySMESHGUI->SetActiveDialogBox(0);
        if ( selMgr )
            selMgr->removeFilter( myIdSourceFilter );
    }
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::activateThisDialog()
{

    /* Emit a signal to deactivate the active dialog */
    // mySMESHGUI->EmitSignalDeactivateDialog();
    // ConstructorsBox->setEnabled(true);
    // GroupArguments->setEnabled(true);
    // GroupButtons->setEnabled(true);

    // mySMESHGUI->SetActiveDialogBox((QDialog*)this);

    // onSelectIdSource( myIdSourceCheck->isChecked() );

    // SelectionIntoArgument();
}

//=================================================================================
// function : setFilters()
// purpose  : SLOT. Called when "Filter" button pressed.
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::setFilters()
{
    if(myMesh->_is_nil()) {
        SUIT_MessageBox::critical(this,
                                  tr("SMESH_ERROR"),
                                  tr("NO_MESH_SELECTED"));
        return;
    }
    if ( !myFilterDlg )
        myFilterDlg = new SMESHGUI_FilterDlg( mySMESHGUI, SMESH::ALL );

    QList<int> types;
    if ( myMesh->NbEdges()     ) types << SMESH::EDGE;
    if ( myMesh->NbFaces()     ) types << SMESH::FACE;
    if ( myMesh->NbVolumes()   ) types << SMESH::VOLUME;
    if ( myMesh->NbBalls()     ) types << SMESH::BALL;
    if ( myMesh->Nb0DElements()) types << SMESH::ELEM0D;
    if ( types.count() > 1 )     types << SMESH::ALL;

    myFilterDlg->Init( types );
    myFilterDlg->SetSelection();
    myFilterDlg->SetMesh( myMesh );
    myFilterDlg->SetSourceWg( myLineEditElements );

    myFilterDlg->show();
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::onOpenView()
{
    if ( mySelector ) {
        SMESH::SetPointRepresentation(false);
    }
    else {
        mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
        activateThisDialog();
    }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::onCloseView()
{
    deactivateActiveDialog();
    mySelector = 0;
}
