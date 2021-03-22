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

#include "SMESHGUI_MG_ADAPTDRIVER.h"

#include "SMESHGUI.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESH_TryCatch.hxx"

#include <LightApp_SelectionMgr.h>
#include <SUIT_Application.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Session.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Application.h>

#include <QLineEdit>
#include <QRadioButton>
#include <QKeyEvent>

// SALOME KERNEL includes

const int SPACING = 6; // layout spacing
const int MARGIN  = 9; // layout margin

// bool createMgAdaptObject(MgAdapt *myMgAdapt )
// {
//   // SMESH::SMESH_Mesh_var newMesh = SMESHGUI::GetSMESHGen()->CreateEmptyMesh();

//   // _PTR(SObject) aHypothesis;
//   _PTR(Study) aStudy = SMESH::getStudy();
//   QStringList anEntryList;
//   _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
//   _PTR(SComponent) mgadapt = aStudy->FindComponent("MG-ADAPT");
//   _PTR(GenericAttribute) ga;
//   if (!aBuilder->FindAttribute(mgadapt, ga, "AttributeName") )
//   {
//     mgadapt = aBuilder->NewComponent("MG-ADAPT");
//     _PTR(AttributeName) Name = aBuilder->FindOrCreateAttribute(mgadapt, "AttributeName");
//     Name->SetValue("MG-ADAPT");
//     _PTR(AttributePixMap) myPixmap = aBuilder->FindOrCreateAttribute( mgadapt, "AttributePixMap" );
//     myPixmap->SetPixMap( "ICON_MG_ADAPT" );
//     anEntryList.append( mgadapt->GetID().c_str() );
//   }

//   _PTR(SObject) obj = aBuilder->NewObject(mgadapt);
//   _PTR(AttributeName) myName = aBuilder->FindOrCreateAttribute(obj, "AttributeName");
//   myName->SetValue("hypo");
//   _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( obj, "AttributePixMap" );
//   aPixmap->SetPixMap( "ICON_SMESH_TREE_HYPO" );
//   anEntryList.append( obj->GetID().c_str() );

//   SMESHGUI::GetSMESHGUI()->updateObjBrowser();

//   // // browse to the published meshes
//   if( LightApp_Application* anApp =
//               dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
//       anApp->browseObjects( anEntryList );
//   return true;
// }


SMESHGUI_MG_ADAPTDRIVER::SMESHGUI_MG_ADAPTDRIVER( SMESHGUI* theModule, SMESH::MG_ADAPT_ptr myModel, bool isCreation )
  : SMESHGUI_MgAdaptDlg((SalomeApp_Module*)theModule, myModel, SMESHGUI::desktop(), isCreation),
    mySMESHGUI( theModule ),
    myIsApplyAndClose( false )
{

  //resMgr = mySMESHGUI->resourceMgr();

  selMgr = selectionMgr();

  // connections
  connect(myArgs, SIGNAL(updateSelection()), this, SLOT(updateSelection()));
  connect(myArgs, SIGNAL(toExportMED(const char*)), this, SLOT(exportMED(const char*)));
}

// SUIT_ResourceMgr* SMESHGUI_MG_ADAPTDRIVER::resourceMgr()
// {
//   return dynamic_cast<SUIT_ResourceMgr*>( SUIT_Session::session()->resourceMgr() );
// }

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

  SMESH::SetPointRepresentation( false );
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow() )
    aViewWindow->SetSelectionMode( ActorSelection );
  if (myArgs->aBrowser->isChecked())
  {
    connect( selMgr, SIGNAL( currentSelectionChanged() ), this, SLOT( selectionChanged() ));
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
    myMesh = mesh;

    SMESH::SMESH_IDSource_var sSelectedObj = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>( IO );
    if ( sSelectedObj->_is_nil() )
      return;
  }
  else
    return;

  SMESH::GetNameOfSelectedIObjects( selMgr, aString );
  if ( aString.isEmpty() ) aString = " ";
  else                     aString = aString.trimmed();


  //bool ok = !aString.isEmpty();
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
  bool toOverwrite = true;
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
// function : PushOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::PushOnOK()
{
  setIsApplyAndClose( true );
  bool ret = PushOnApply();
//   std::cout  << "SMESHGUI_MG_ADAPTDRIVER::PushOnOK ret : " <<ret<<std::endl;
  if ( ret ) reject();
}

bool SMESHGUI_MG_ADAPTDRIVER::PushOnApply()
{
  if ( SMESHGUI::isStudyLocked() )
    return false;
  if( !isValid() )
    return false;

  bool ok = SMESHGUI_MgAdaptDlg::PushOnApply();
//   std::cout  << "SMESHGUI_MG_ADAPTDRIVER::PushOnApply ok 1 : " <<ok<<std::endl;

  if ( ok )
  {
    ok = execute();
    if (getModel()->getPublish()) this->createMeshInObjectBrowser();
//     std::cout  << "SMESHGUI_MG_ADAPTDRIVER::PushOnApply ok 2 : " <<ok<<std::endl;
    if ( ok )
    {
      QMessageBox::information( 0, QObject::tr(""),
                                   QObject::tr("MG_ADAPT_DIAG_1") );
    }
    else
    {
      QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                                QObject::tr("MG_ADAPT_DIAG_2") );
    }
  }

  return ok;
}

// macro used to initialize errStr by exception description
// returned by SMESH_CATCH( SMESH::returnError )
#undef SMESH_CAUGHT
#define SMESH_CAUGHT errStr =

#undef SMY_OWN_CATCH
#define SMY_OWN_CATCH catch ( SALOME::SALOME_Exception & e ) { errStr = e.details.text; }

bool SMESHGUI_MG_ADAPTDRIVER::execute()
{
  int err = 1;
  std::string errStr;
  SMESH_TRY;
  {
    getModel()->compute();
    err = 0;
    errStr = SMESH::toStdStr( getModel()->getErrMsg() );
  }
  SMESH_CATCH( SMESH::returnError );

  std::string msg = " ok";
  if ( !errStr.empty() || err != 0 )
  {
    msg = "Not ok \n" + errStr + "\n";
    std::cerr << msg;
    err = 1;
  }
  return err == 0;
}
#undef SMESH_CAUGHT
#define SMESH_CAUGHT

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

  if ( e->key() == Qt::Key_F1 )
  {
    e->accept();
    PushOnHelp();
  }

}

//=================================================================================
// function : PushOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MG_ADAPTDRIVER::PushOnHelp()
{

  QString aHelpFile = "adaptation.html#_mg_adapt_anchor";

  SMESH::ShowHelpFile( aHelpFile );

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
  QString filename( SMESH::toQStr( getModel()->getMedFileOut() ));
  QStringList errors;
  QStringList anEntryList;
  bool isEmpty = false;
  //  bool ok = false;
  SMESH::SMESH_Gen_var SMESH_Gen_ptr = SMESHGUI::GetSMESHGen();
  if ( SMESH_Gen_ptr->_is_nil() ) {
    QMessageBox::critical( 0, QObject::tr("MG_ADAPT_ERROR"),
                           QObject::tr("Could not retrieve SMESH_Gen_ptr") );
    return false;
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
    if ( aMeshSO )
    {
      _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
      _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" );
      aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" ); // put REFINED mesh ico
      anEntryList.append( aMeshSO->GetID().c_str() );
    }
    else
    {
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
}
//================================================================
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
  // if (isEnabled())
  // {
  //   mySMESHGUI->ResetState();
  //   mySMESHGUI->SetActiveDialogBox(0);
  //   setEnabled( false );
  // }
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
