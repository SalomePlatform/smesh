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
//  File   : SMESHGUI.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI.h"

#include "SMESH_Client.hxx"
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI_ClippingDlg.h"
#include "SMESHGUI_GroupDlg.h"
#include "SMESHGUI_RemoveNodesDlg.h"
#include "SMESHGUI_RemoveElementsDlg.h"
#include "SMESHGUI_MeshInfosDlg.h"
#include "SMESHGUI_StandardMeshInfosDlg.h"
#include "SMESHGUI_Preferences_ColorDlg.h"
#include "SMESHGUI_Preferences_ScalarBarDlg.h"
#include "SMESHGUI_Preferences_SelectionDlg.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_MoveNodesDlg.h"
#include "SMESHGUI_AddMeshElementDlg.h"
#include "SMESHGUI_AddQuadraticElementDlg.h"
#include "SMESHGUI_EditHypothesesDlg.h"
#include "SMESHGUI_CreateHypothesesDlg.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_FilterLibraryDlg.h"
#include "SMESHGUI_SingleEditDlg.h"
#include "SMESHGUI_MultiEditDlg.h"
#include "SMESHGUI_GroupOpDlg.h"
#include "SMESHGUI_DeleteGroupDlg.h"
#include "SMESHGUI_SmoothingDlg.h"
#include "SMESHGUI_RenumberingDlg.h"
#include "SMESHGUI_ExtrusionDlg.h"
#include "SMESHGUI_ExtrusionAlongPathDlg.h"
#include "SMESHGUI_RevolutionDlg.h"
#include "SMESHGUI_TranslationDlg.h"
#include "SMESHGUI_RotationDlg.h"
#include "SMESHGUI_SymmetryDlg.h"
#include "SMESHGUI_SewingDlg.h"
#include "SMESHGUI_MergeNodesDlg.h"
#include "SMESHGUI_EditMeshDlg.h"
#include "SMESHGUI_MeshPatternDlg.h"
#include "SMESHGUI_PrecisionDlg.h"
#include "SMESHGUI_Selection.h"
#include "SMESHGUI_CreatePolyhedralVolumeDlg.h"
#include "SMESHGUI_MeshOp.h"
#include "SMESHGUI_Displayer.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_HypothesesUtils.h"

#include "SMESH_Actor.h"
#include "SMESH_Object.h"
#include "SMESH_TypeFilter.hxx"

#include "SalomeApp_Tools.h"
#include "SalomeApp_Study.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_CheckFileDlg.h"
#include "SalomeApp_ImportOperation.h"

#include "LightApp_DataOwner.h"
#include "LightApp_Preferences.h"
#include "LightApp_VTKSelector.h"
#include "LightApp_Operation.h"
#include "LightApp_UpdateFlags.h"
#include "LightApp_NameDlg.h"

#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SVTK_InteractorStyle.h>
#include <SVTK_RenderWindowInteractor.h>
#include <SVTK_ViewManager.h>

#include "OB_Browser.h"

#include "SUIT_Tools.h"
#include "SUIT_MessageBox.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_FileDlg.h"
#include "SUIT_Desktop.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_Study.h"
#include "SUIT_Session.h"

#include "QtxPopupMgr.h"

#include "SALOME_ListIO.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "SALOME_NamingService.hxx"
#include "SALOME_LifeCycleCORBA.hxx"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)

// QT Includes
#define	 INCLUDE_MENUITEM_DEF
#include <qpopupmenu.h>
#include <qstring.h>
#include <qwidget.h>
#include <qaction.h>

// BOOST Includes
#include <boost/shared_ptr.hpp>

// VTK Includes
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkActorCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkUnstructuredGrid.h>

#include "utilities.h"

#include "SALOMEDS_Study.hxx"
#include "SALOMEDSClient_StudyBuilder.hxx"
#include "SALOMEDSClient_SComponent.hxx"

using namespace std;

namespace{
  // Declarations
  //=============================================================
  void ImportMeshesFromFile(SMESH::SMESH_Gen_ptr theComponentMesh,
			    int theCommandID);

  void ExportMeshToFile(int theCommandID);

  void SetDisplayMode(int theCommandID);

  void SetDisplayEntity(int theCommandID);

  void Control( int theCommandID );


  // Definitions
  //=============================================================
  void ImportMeshesFromFile(SMESH::SMESH_Gen_ptr theComponentMesh,
			    int theCommandID)
  {
    QStringList filter;
    string myExtension;

    if(theCommandID == 113){
      filter.append(QObject::tr("MED files (*.med)"));
      filter.append(QObject::tr("All files (*)"));
    }else if (theCommandID == 112){
      filter.append(QObject::tr("IDEAS files (*.unv)"));
    }else if (theCommandID == 111){
      filter.append(QObject::tr("DAT files (*.dat)"));
    }
    QString filename = SUIT_FileDlg::getFileName(SMESHGUI::desktop(),
						"",
						filter,
						QObject::tr("Import mesh"),
						true);
    if(!filename.isEmpty()) {
      SUIT_OverrideCursor wc;
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

      try {
	SMESH::mesh_array_var aMeshes = new SMESH::mesh_array;
	switch ( theCommandID ) {
	case 112:
	  {
	    aMeshes->length( 1 );
	    aMeshes[0] = theComponentMesh->CreateMeshesFromUNV(filename.latin1());
	    break;
	  }
	case 113:
	  {
	    SMESH::DriverMED_ReadStatus res;
	    aMeshes = theComponentMesh->CreateMeshesFromMED(filename.latin1(),res);
	    if ( res != SMESH::DRS_OK ) {
	      wc.suspend();
	      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
				    QObject::tr("SMESH_WRN_WARNING"),
				    QObject::tr(QString("SMESH_DRS_%1").arg(res)),
				    QObject::tr("SMESH_BUT_OK"));
	      aMeshes->length( 0 );
	      wc.resume();
	    }
	    break;
	  }
	}

	bool isEmpty = false;
	for ( int i = 0, iEnd = aMeshes->length(); i < iEnd; i++ ) {
	  _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshes[i] );
	  if ( aMeshSO ) {
	    _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
	    _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" );
	    aPixmap->SetPixMap("ICON_SMESH_TREE_MESH_IMPORTED");
	    if ( theCommandID == 112 ) // mesh names aren't taken from the file for UNV import
	      SMESH::SetName( aMeshSO, QFileInfo(filename).fileName() );
	  } else
	    isEmpty = true;
	}

	if ( isEmpty ) {
	  wc.suspend();
	  SUIT_MessageBox::warn1(SMESHGUI::desktop(),
				QObject::tr("SMESH_WRN_WARNING"),
				QObject::tr("SMESH_DRS_EMPTY"),
				QObject::tr("SMESH_BUT_OK"));
	  wc.resume();
	}

	SMESHGUI::GetSMESHGUI()->updateObjBrowser();
      }
      catch (const SALOME::SALOME_Exception& S_ex){
	wc.suspend();
	SalomeApp_Tools::QtCatchCorbaException(S_ex);
	wc.resume();
      }
    }
  }


  void ExportMeshToFile( int theCommandID )
  {
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    if(selected.Extent()){
      Handle(SALOME_InteractiveObject) anIObject = selected.First();
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
      if ( !aMesh->_is_nil() ) {
	QString aFilter, aTitle = QObject::tr("Export mesh");
	QMap<QString, SMESH::MED_VERSION> aFilterMap;
	switch ( theCommandID ) {
	case 125:
	case 122:
          {
	    if (aMesh->HasDuplicatedGroupNamesMED()) {
              int aRet = SUIT_MessageBox::warn2
                (SMESHGUI::desktop(),
                 QObject::tr("SMESH_WRN_WARNING"),
                 QObject::tr("SMESH_EXPORT_MED_DUPLICATED_GRP").arg(anIObject->getName()),
                 QObject::tr("SMESH_BUT_YES"),  QObject::tr("SMESH_BUT_NO"),
                 0, 1, 0);
              if (aRet)
                return;
            }

            aFilterMap.insert( QObject::tr("MED 2.1 (*.med)"), SMESH::MED_V2_1 );
            aFilterMap.insert( QObject::tr("MED 2.2 (*.med)"), SMESH::MED_V2_2 );
          }
	  break;
	case 124:
	case 121:
	  aFilter = QObject::tr("DAT files (*.dat)");
	  break;
	case 126:
	case 123:
          {
            if (aMesh->NbPyramids()) {
              int aRet = SUIT_MessageBox::warn2
                (SMESHGUI::desktop(),
                 QObject::tr("SMESH_WRN_WARNING"),
                 QObject::tr("SMESH_EXPORT_UNV").arg(anIObject->getName()),
                 QObject::tr("SMESH_BUT_YES"),  QObject::tr("SMESH_BUT_NO"),
                 0, 1, 0);
              if (aRet)
                return;
            }
            aFilter = QObject::tr("IDEAS files (*.unv)");
          }
	  break;
	default:
	  return;
	}

	QString aFilename;
	SMESH::MED_VERSION aFormat;
	// Init the parameter with the default value
	bool toCreateGroups = false;
	SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	if ( resMgr )
	  toCreateGroups = resMgr->booleanValue( "SMESH", "auto_groups", false );
	
	if ( theCommandID != 122 && theCommandID != 125 )
	  aFilename = SUIT_FileDlg::getFileName(SMESHGUI::desktop(), "", aFilter, aTitle, false);
	else {
          QStringList filters;
          QMap<QString, SMESH::MED_VERSION>::const_iterator it = aFilterMap.begin();
          for ( ; it != aFilterMap.end(); ++it )
            filters.push_back( it.key() );

          //SUIT_FileDlg* fd = new SUIT_FileDlg( SMESHGUI::desktop(), false, true, true );
          SalomeApp_CheckFileDlg* fd = new SalomeApp_CheckFileDlg
            ( SMESHGUI::desktop(), false, QObject::tr("SMESH_AUTO_GROUPS") ,true, true );
          fd->setCaption( aTitle );
          fd->setFilters( filters );
          fd->setSelectedFilter( QObject::tr("MED 2.2 (*.med)") );
          fd->SetChecked(toCreateGroups);
          bool is_ok = false;
          while (!is_ok) {
            fd->exec();
            aFilename = fd->selectedFile();
            aFormat = aFilterMap[fd->selectedFilter()];
            is_ok = true;
            if ( !aFilename.isEmpty()
                 && (aMesh->NbPolygons()>0 or aMesh->NbPolyhedrons()>0)
                 && aFormat==SMESH::MED_V2_1) {
              int aRet = SUIT_MessageBox::warn2(SMESHGUI::desktop(),
                                                QObject::tr("SMESH_WRN_WARNING"),
                                                QObject::tr("SMESH_EXPORT_MED_V2_1").arg(anIObject->getName()),
                                                QObject::tr("SMESH_BUT_YES"),
                                                QObject::tr("SMESH_BUT_NO"),
                                                0,1,0);
              if (aRet) {
                is_ok = false;
              }
            }
          }
          toCreateGroups = fd->IsChecked();
          delete fd;
        }
	if ( !aFilename.isEmpty() ) {
	  // Check whether the file already exists and delete it if yes
	  QFile aFile( aFilename );
	  if ( aFile.exists() )
	    aFile.remove();
	  SUIT_OverrideCursor wc;
	  switch ( theCommandID ) {
	  case 125:
	  case 122:
	    aMesh->ExportToMED( aFilename.latin1(), toCreateGroups, aFormat );
	    break;
	  case 124:
	  case 121:
	    aMesh->ExportDAT( aFilename.latin1() );
	    break;
	  case 126:
	  case 123:
	    aMesh->ExportUNV( aFilename.latin1() );
	    break;
	  default:
	    break;
	  }
	}
      }
    }
  }

  inline void InverseEntityMode(unsigned int& theOutputMode,
				unsigned int theMode)
  {
    bool anIsNotPresent = ~theOutputMode & theMode;
    if(anIsNotPresent)
      theOutputMode |= theMode;
    else
      theOutputMode &= ~theMode;
  }

  void SetDisplayEntity(int theCommandID){
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    if(selected.Extent() >= 1){
      SALOME_ListIteratorOfListIO It( selected );
      for(; It.More(); It.Next()){
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	if(IObject->hasEntry()){
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
	    unsigned int aMode = anActor->GetEntityMode();
	    switch(theCommandID){
	    case 217:
	      InverseEntityMode(aMode,SMESH_Actor::eEdges);
	      break;
	    case 218:
	      InverseEntityMode(aMode,SMESH_Actor::eFaces);
	      break;
	    case 219:
	      InverseEntityMode(aMode,SMESH_Actor::eVolumes);
	      break;
	    case 220:
	      aMode = SMESH_Actor::eAllEntity;
	      break;
	    }
	    if(aMode)
	      anActor->SetEntityMode(aMode);
	  }
	}
      }
    }
  }

  void SetDisplayMode(int theCommandID){
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    if(selected.Extent() >= 1){
      switch(theCommandID){
      case 1134:{
	SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
	new SMESHGUI_ClippingDlg( SMESHGUI::GetSMESHGUI(), "", false );
	return;
      }
      case 1133:{
	SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
	new SMESHGUI_TransparencyDlg( SMESHGUI::GetSMESHGUI(), "", false );
	return;
      }}
      SALOME_ListIteratorOfListIO It( selected );
      for(; It.More(); It.Next()){
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	if(IObject->hasEntry()){
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
	    switch(theCommandID){
	    case 211:
	      anActor->SetRepresentation(SMESH_Actor::eEdge);
	      break;
	    case 212:
	      anActor->SetRepresentation(SMESH_Actor::eSurface);
	      break;
	    case 213:
	      if(anActor->IsShrunk())
		anActor->UnShrink();
	      else
		anActor->SetShrink();
	      break;
	    case 215:
	      anActor->SetRepresentation(SMESH_Actor::ePoint);
	      break;
	    case 1132:{
	      float color[3];
	      anActor->GetSufaceColor(color[0], color[1], color[2]);
	      int c0 = int (color[0] * 255);
	      int c1 = int (color[1] * 255);
	      int c2 = int (color[2] * 255);
	      QColor c(c0, c1, c2);

	      float edgecolor[3];
	      anActor->GetEdgeColor(edgecolor[0], edgecolor[1], edgecolor[2]);
	      c0 = int (edgecolor[0] * 255);
	      c1 = int (edgecolor[1] * 255);
	      c2 = int (edgecolor[2] * 255);
	      QColor e(c0, c1, c2);

	      float backfacecolor[3];
	      anActor->GetBackSufaceColor(backfacecolor[0], backfacecolor[1], backfacecolor[2]);
	      c0 = int (backfacecolor[0] * 255);
	      c1 = int (backfacecolor[1] * 255);
	      c2 = int (backfacecolor[2] * 255);
	      QColor b(c0, c1, c2);

	      float nodecolor[3];
	      anActor->GetNodeColor(nodecolor[0], nodecolor[1], nodecolor[2]);
	      c0 = int (nodecolor[0] * 255);
	      c1 = int (nodecolor[1] * 255);
	      c2 = int (nodecolor[2] * 255);
	      QColor n(c0, c1, c2);

	      int Edgewidth = (int)anActor->GetLineWidth();
	      if(Edgewidth == 0)
		Edgewidth = 1;
	      int intValue = int(anActor->GetNodeSize());
	      float Shrink = anActor->GetShrinkFactor();

	      SMESHGUI_Preferences_ColorDlg *aDlg =
		new SMESHGUI_Preferences_ColorDlg( SMESHGUI::GetSMESHGUI(), "" );
	      aDlg->SetColor(1, c);
	      aDlg->SetColor(2, e);
	      aDlg->SetColor(3, n);
	      aDlg->SetColor(4, b);
	      aDlg->SetIntValue(1, Edgewidth);
	      aDlg->SetIntValue(2, intValue);
	      aDlg->SetIntValue(3, int(Shrink*100.));
	      if(aDlg->exec()){
		QColor color = aDlg->GetColor(1);
		QColor edgecolor = aDlg->GetColor(2);
		QColor nodecolor = aDlg->GetColor(3);
		QColor backfacecolor = aDlg->GetColor(4);
		/* actor color and backface color */
		anActor->SetSufaceColor(float (color.red()) / 255.,
					 float (color.green()) / 255.,
					 float (color.blue()) / 255.);
		anActor->SetBackSufaceColor(float (backfacecolor.red()) / 255.,
					     float (backfacecolor.green()) / 255.,
					     float (backfacecolor.blue()) / 255.);

		/* edge color */
		anActor->SetEdgeColor(float (edgecolor.red()) / 255.,
				       float (edgecolor.green()) / 255.,
				       float (edgecolor.blue()) / 255.);

		/* Shrink factor and size edges */
		anActor->SetShrinkFactor(aDlg->GetIntValue(3) / 100.);
		anActor->SetLineWidth(aDlg->GetIntValue(1));

		/* Nodes color and size */
		anActor->SetNodeColor(float (nodecolor.red()) / 255.,
			       float (nodecolor.green()) / 255.,
				       float (nodecolor.blue()) / 255.);
		anActor->SetNodeSize(aDlg->GetIntValue(2));

		delete aDlg;
	      }
	      break;
	    }}
	  }
	}
      }
      SMESH::RepaintCurrentView();
    }
  }

  void Control( int theCommandID )
  {
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    if( !selected.IsEmpty() ){
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      if(!anIO.IsNull()){
	QString aTitle;
	SMESH_Actor::eControl aControl = SMESH_Actor::eNone;
	if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIO->getEntry())){
	  switch ( theCommandID ){
	  case 6001:
	    aTitle = QObject::tr( "LENGTH_EDGES" );
	    aControl = SMESH_Actor::eLength;
	    break;
	  case 6018:
	    aTitle = QObject::tr( "LENGTH2D_EDGES" );
	    aControl = SMESH_Actor::eLength2D;
	    break;
	  case 6002:
	    aTitle = QObject::tr( "FREE_EDGES" );
	    aControl = SMESH_Actor::eFreeEdges;
	    break;
	  case 6003:
	    aTitle = QObject::tr( "FREE_BORDERS" );
	    aControl = SMESH_Actor::eFreeBorders;
	    break;
	  case 6004:
	    aTitle = QObject::tr( "MULTI_BORDERS" );
	    aControl = SMESH_Actor::eMultiConnection;
	    break;
	  case 6019:
	    aTitle = QObject::tr( "MULTI2D_BORDERS" );
	    aControl = SMESH_Actor::eMultiConnection2D;
	    break;
	  case 6011:
	    aTitle = QObject::tr( "AREA_ELEMENTS" );
	    aControl = SMESH_Actor::eArea;
	    break;
	  case 6012:
	    aTitle = QObject::tr( "TAPER_ELEMENTS" );
	    aControl = SMESH_Actor::eTaper;
	    break;
	  case 6013:
	    aTitle = QObject::tr( "ASPECTRATIO_ELEMENTS" );
	    aControl = SMESH_Actor::eAspectRatio;
	    break;
	  case 6017:
	    aTitle = QObject::tr( "ASPECTRATIO_3D_ELEMENTS" );
	    aControl = SMESH_Actor::eAspectRatio3D;
	    break;
	  case 6014:
	    aTitle = QObject::tr( "MINIMUMANGLE_ELEMENTS" );
	    aControl = SMESH_Actor::eMinimumAngle;
	    break;
	  case 6015:
	    aTitle = QObject::tr( "WARP_ELEMENTS" );
	    aControl = SMESH_Actor::eWarping;
	    break;
	  case 6016:
	    aTitle = QObject::tr( "SKEW_ELEMENTS" );
	    aControl = SMESH_Actor::eSkew;
	    break;
	  case 6009:
	    aTitle = QObject::tr( "SMESH_VOLUME" );
	    aControl = SMESH_Actor::eVolume3D;
	    break;
	  }
	  anActor->SetControlMode(aControl);
	  anActor->GetScalarBarActor()->SetTitle(aTitle.latin1());
	  SMESH::RepaintCurrentView();
	}
      }
    }
  }


  bool CheckOIType(const Handle(SALOME_InteractiveObject) & theIO,
		   MeshObjectType                           theType,
		   const QString                            theInTypeName,
		   QString &                                theOutTypeName)
  {
    SMESH_TypeFilter aTypeFilter( theType );
    QString entry;
    if( !theIO.IsNull() )
    {
      entry = theIO->getEntry();
      LightApp_DataOwner owner( entry );
      if ( aTypeFilter.isOk( &owner )) {
        theOutTypeName = theInTypeName;
        return true;
      }
    }
    return false;
  }


  QString CheckTypeObject(const Handle(SALOME_InteractiveObject) & theIO)
  {
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    _PTR(SObject) aSObj = aStudy->FindObjectID(theIO->getEntry());
    if (aSObj) {
      _PTR(SComponent) aSComp = aSObj->GetFatherComponent();
      CORBA::String_var anID = aSComp->GetID().c_str();
      if (!strcmp(anID.in(),theIO->getEntry()))
	return "Component";
    }

    QString aTypeName;
    if (
	CheckOIType ( theIO, HYPOTHESIS,    "Hypothesis", aTypeName ) ||
	CheckOIType ( theIO, ALGORITHM,     "Algorithm",  aTypeName ) ||
	CheckOIType ( theIO, MESH,          "Mesh",       aTypeName ) ||
	CheckOIType ( theIO, SUBMESH,       "SubMesh",    aTypeName ) ||
	CheckOIType ( theIO, GROUP,         "Group",      aTypeName )
	)
      return aTypeName;

    return "NoType";
  }


  QString CheckHomogeneousSelection()
  {
    //SUIT_Study* aStudy = SMESH::GetActiveStudy();
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    QString RefType = CheckTypeObject(selected.First());
    SALOME_ListIteratorOfListIO It(selected);
    for (; It.More(); It.Next())
      {
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	QString Type = CheckTypeObject(IObject);
	if (Type.compare(RefType) != 0)
	  return "Heterogeneous Selection";
      }

    return RefType;
  }


  void SMESHGUI::OnEditDelete()
  {
    // VSR 17/11/04: check if all objects selected belong to SMESH component --> start
    LightApp_SelectionMgr* aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected; aSel->selectedObjects( selected, QString::null, false );

    QString aParentComponent = QString::null;
    for( SALOME_ListIteratorOfListIO anIt( selected ); anIt.More(); anIt.Next() )
    {
      QString cur = anIt.Value()->getComponentDataType();
      if( aParentComponent.isNull() )
        aParentComponent = cur;
      else if( !aParentComponent.isEmpty() && aParentComponent!=cur )
        aParentComponent = "";
    }

    if ( aParentComponent != SMESHGUI::GetSMESHGUI()->name() )  {
      SUIT_MessageBox::warn1 ( SMESHGUI::desktop(),
			      QObject::tr("ERR_ERROR"),
			      QObject::tr("NON_SMESH_OBJECTS_SELECTED").arg( SMESHGUI::GetSMESHGUI()->moduleName() ),
			      QObject::tr("BUT_OK") );
      return;
    }
    // VSR 17/11/04: check if all objects selected belong to SMESH component <-- finish
    if (SUIT_MessageBox::warn2
	(SMESHGUI::desktop(),
	 QObject::tr("SMESH_WRN_WARNING"),
	 QObject::tr("SMESH_REALLY_DELETE"),
	 QObject::tr("SMESH_BUT_YES"), QObject::tr("SMESH_BUT_NO"), 1, 0, 0) != 1)
      return;

    SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
    SUIT_ViewManager* vm = anApp->activeViewManager();
    int nbSf = vm->getViewsCount();

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    _PTR(StudyBuilder) aStudyBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr;
    _PTR(AttributeIOR) anIOR;

    SALOME_ListIteratorOfListIO It(selected);

    aStudyBuilder->NewCommand();  // There is a transaction
    for(; It.More(); It.Next()){
      Handle(SALOME_InteractiveObject) IObject = It.Value();
      if(IObject->hasEntry()){
	_PTR(SObject) SO = aStudy->FindObjectID(IObject->getEntry());

	// disable removal of "SMESH" component object
	if(SO->FindAttribute(anAttr, "AttributeIOR")){
	  anIOR = anAttr;
	  if ( !strcmp( (char*)anIOR->Value().c_str(), engineIOR().latin1() ) )
	    continue;
	}

	/* Erase child graphical objects */
	_PTR(ChildIterator) it = aStudy->NewChildIterator(SO);
	for(it->InitEx(true); it->More(); it->Next()){
	  _PTR(SObject) CSO = it->Value();
	  if(CSO->FindAttribute(anAttr, "AttributeIOR")){
	    anIOR = anAttr;

	    QPtrVector<SUIT_ViewWindow> aViews = vm->getViews();
	    for(int i = 0; i < nbSf; i++){
	      SUIT_ViewWindow *sf = aViews[i];
	      CORBA::String_var anEntry = CSO->GetID().c_str();
	      if(SMESH_Actor* anActor = SMESH::FindActorByEntry(sf,anEntry.in())){
		SMESH::RemoveActor(sf,anActor);
	      }
	    }
	  }
	}

	/* Erase main graphical object */
	QPtrVector<SUIT_ViewWindow> aViews = vm->getViews();
	for(int i = 0; i < nbSf; i++){
	  SUIT_ViewWindow *sf = aViews[i];
	  if(SMESH_Actor* anActor = SMESH::FindActorByEntry(sf,IObject->getEntry())){
	    SMESH::RemoveActor(sf,anActor);
	  }
	}

	// Remove object(s) from data structures
	_PTR(SObject) obj = aStudy->FindObjectID(IObject->getEntry());
	if(obj){
	  SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( SMESH::SObjectToObject( obj ) );
	  SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow( SMESH::SObjectToObject( obj ) );
          QString objType = CheckTypeObject(IObject);
	  if ( !aGroup->_is_nil() ) {                          // DELETE GROUP
	    SMESH::SMESH_Mesh_var aMesh = aGroup->GetMesh();
	    aMesh->RemoveGroup( aGroup );
	  }
	  else if ( !aSubMesh->_is_nil() ) {                   // DELETE SUBMESH
	    SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
	    aMesh->RemoveSubMesh( aSubMesh );
	  }
	  else if ( objType == "Hypothesis" || objType == "Algorithm" ) {// DELETE HYPOTHESIS
            SMESH::RemoveHypothesisOrAlgorithmOnMesh(IObject);
	    aStudyBuilder->RemoveObjectWithChildren( obj );
	  }
	  else {// default action: remove SObject from the study
	    // san - it's no use opening a transaction here until UNDO/REDO is provided in SMESH
	    //SUIT_Operation *op = new SALOMEGUI_ImportOperation(myActiveStudy);
	    //op->start();
	    aStudyBuilder->RemoveObjectWithChildren( obj );
	    //op->finish();
	  }
	}

      } /* IObject->hasEntry() */
    } /* more/next */
    aStudyBuilder->CommitCommand();

    /* Clear any previous selection */
    SALOME_ListIO l1;
    aSel->setSelectedObjects( l1 );

    SMESHGUI::GetSMESHGUI()->updateObjBrowser();
  }
}

extern "C" {
  Standard_EXPORT CAM_Module* createModule()
  {
    return new SMESHGUI();
  }
}

SMESH::SMESH_Gen_var SMESHGUI::myComponentSMESH = SMESH::SMESH_Gen::_nil();

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::SMESHGUI() :
SalomeApp_Module( "SMESH" )
{  
  if ( CORBA::is_nil( myComponentSMESH ) )
  {
    CORBA::Boolean anIsEmbeddedMode;
    myComponentSMESH = SMESH_Client::GetSMESHGen(getApp()->orb(),anIsEmbeddedMode);
  }

  myActiveDialogBox = 0;
  myState = -1;
  myDisplayer = 0;

  SMESH::GetFilterManager();
  SMESH::GetPattern();

  /* load resources for all available meshers */
  SMESH::InitAvailableHypotheses();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::~SMESHGUI()
{
  SMESH::GetFilterManager()->Destroy();
  SMESH::GetFilterManager() = SMESH::FilterManager::_nil();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
LightApp_SelectionMgr* SMESHGUI::selectionMgr()
{
  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
  if( anApp )
    return dynamic_cast<LightApp_SelectionMgr*>( anApp->selectionMgr() );
  else
    return 0;
}

bool SMESHGUI::automaticUpdate()
{
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
  if ( !resMgr )
    return false;

  return resMgr->booleanValue( "SMESH", "auto_update", false );
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SUIT_ResourceMgr* SMESHGUI::resourceMgr()
{
  return dynamic_cast<SUIT_ResourceMgr*>( SUIT_Session::session()->resourceMgr() );
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI* SMESHGUI::GetSMESHGUI()
{
  SMESHGUI* smeshMod = 0;
  SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>(SUIT_Session::session()->activeApplication());
  if ( app )
  {
    CAM_Module* module = app->module( "Mesh" );
    smeshMod = dynamic_cast<SMESHGUI*>( module );
  }

  if ( smeshMod && smeshMod->application() && smeshMod->application()->activeStudy() )
  {
    SalomeApp_Study* study = dynamic_cast<SalomeApp_Study*>( smeshMod->application()->activeStudy() );
    if ( study )
    {
      _PTR(Study) aStudy = study->studyDS();
      if ( aStudy )
	GetSMESHGen()->SetCurrentStudy( _CAST(Study,aStudy)->GetStudy() );
    }
  }

  return smeshMod;
}

extern "C"
{
  Standard_EXPORT SMESHGUI* GetComponentGUI()
  {
    return SMESHGUI::GetSMESHGUI();
  }
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetState(int aState)
{
  myState = aState;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::ResetState()
{
  myState = -1;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalDeactivateDialog()
{
  emit SignalDeactivateActiveDialog();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalStudyFrameChanged()
{
  emit SignalStudyFrameChanged();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalCloseAllDialogs()
{
  emit SignalCloseAllDialogs();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
QDialog *SMESHGUI::GetActiveDialogBox()
{
  return myActiveDialogBox;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::SetActiveDialogBox(QDialog * aDlg)
{
  myActiveDialogBox = (QDialog *) aDlg;
  return;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SUIT_Desktop* SMESHGUI::desktop()
{
  SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
  if( app )
    return app->desktop();
  else
    return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SalomeApp_Study* SMESHGUI::activeStudy()
{
  SUIT_Application* app = SUIT_Session::session()->activeApplication();
  if( app )
    return dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
  else
    return NULL;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::DefineDlgPosition(QWidget * aDlg, int &x, int &y)
{
  /* Here the position is on the bottom right corner - 10 */
  // aDlg->resize(QSize().expandedTo(aDlg->minimumSizeHint()));
  aDlg->adjustSize();
  SUIT_Desktop *PP = desktop();
  x = abs(PP->x() + PP->size().width() - aDlg->size().width() - 10);
  y = abs(PP->y() + PP->size().height() - aDlg->size().height() - 10);
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
static int isStudyLocked(_PTR(Study) theStudy){
  return theStudy->GetProperties()->IsLocked();
}

static bool checkLock(_PTR(Study) theStudy) {
  if (isStudyLocked(theStudy)) {
    SUIT_MessageBox::warn1 ( SMESHGUI::desktop(),
			   QObject::tr("WRN_WARNING"),
			   QObject::tr("WRN_STUDY_LOCKED"),
			   QObject::tr("BUT_OK") );
    return true;
  }
  return false;
}

//=======================================================================
//function : CheckActiveStudyLocked
//purpose  :
//=======================================================================

bool SMESHGUI::isActiveStudyLocked()
{
  _PTR(Study) aStudy = activeStudy()->studyDS();
  return checkLock( aStudy );
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnGUIEvent( int theCommandID )
{
  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( application() );
  if( !anApp )
    return false;

  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument(); //Document OCAF de l'etude active
  SUIT_ResourceMgr* mgr = resourceMgr();
  if( !mgr )
    return false;

  if (CORBA::is_nil(GetSMESHGen()->GetCurrentStudy())) {
    GetSMESHGen()->SetCurrentStudy(_CAST(Study,aStudy)->GetStudy());
  }

  SUIT_ViewWindow* view = application()->desktop()->activeWindow();
  SVTK_ViewWindow* vtkwnd = dynamic_cast<SVTK_ViewWindow*>( view );

  //QAction* act = action( theCommandID );

  switch (theCommandID)	{
  case 33:					// DELETE
    if(checkLock(aStudy)) break;
    OnEditDelete();
    break;

  case 113:					// IMPORT
  case 112:
  case 111:
    {
      if(checkLock(aStudy)) break;
      ::ImportMeshesFromFile(GetSMESHGen(),theCommandID);
      break;
    }

  case 122:					// EXPORT MED
  case 121:
  case 123:
  case 124:
  case 125:
  case 126:
    {
      ::ExportMeshToFile(theCommandID);
      break;
    }

  case 200:					// SCALAR BAR
    {
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      if( selected.Extent() ) {
	Handle(SALOME_InteractiveObject) anIO = selected.First();
	if( anIO->hasEntry() ) {
	  if( SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() ) ) {
	    anActor->SetControlMode( SMESH_Actor::eNone );
	  }
	}
      }
      break;
    }
  case 201:
    {
      SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties( this );
      break;
    }

  case 1134: // Clipping
  case 1133: // Tranparency
  case 1132: // Colors / Size

    // Display Mode
  case 215: // Nodes
  case 213: // Nodes
  case 212: // Nodes
  case 211: // Nodes
    ::SetDisplayMode(theCommandID);
  break;

    // Display Entity
  case 217: // Edges
  case 218: // Faces
  case 219: // Volumes
  case 220: // All Entity
    ::SetDisplayEntity(theCommandID);
  break;

  case 214:					// UPDATE
    {
      if(checkLock(aStudy)) break;
      SMESH::UpdateView();

      SALOME_ListIO l;
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      aSel->selectedObjects( l );
      aSel->setSelectedObjects( l );
      break;
    }

  case 300:					// ERASE
  case 301:					// DISPLAY
  case 302:					// DISPLAY ONLY
    {
      SMESH::EDisplaing anAction;
      switch (theCommandID) {
      case 300:	anAction = SMESH::eErase; break;
      case 301:	anAction = SMESH::eDisplay; break;
      case 302:	anAction = SMESH::eDisplayOnly; break;
      }

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO sel_objects, to_process;
      if (aSel)
        aSel->selectedObjects( sel_objects );

      if( theCommandID==302 )
	startOperation( myEraseAll );

      extractContainers( sel_objects, to_process );

      if (vtkwnd) {
	SALOME_ListIteratorOfListIO It( to_process );
	for (; It.More(); It.Next()) {
	  Handle(SALOME_InteractiveObject) IOS = It.Value();
	  if (IOS->hasEntry()) {
            SMESH::UpdateView(anAction, IOS->getEntry());
            if (anAction == SMESH::eDisplayOnly)
              anAction = SMESH::eDisplay;
	  }
	}
      }

      if (anAction == SMESH::eErase) {
	SALOME_ListIO l1;
	aSel->setSelectedObjects( l1 );
      }
      else
	aSel->setSelectedObjects( to_process );
      break;
    }

  case 400:					// NODES
    {
      if(checkLock(aStudy)) break;

      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();

	new SMESHGUI_NodesDlg(this);
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"),
			      tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }

  case 2151:  // FILTER
  {
    if ( vtkwnd )
    {
      EmitSignalDeactivateDialog();
      new SMESHGUI_FilterDlg( this, SMESH::EDGE );
    }
    break;
  }

  case 406:					// MOVE NODE
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if(checkLock(aStudy)) break;
      new SMESHGUI_MoveNodesDlg(this);
      break;
    }

  case 701:					// COMPUTE MESH
    {
      if(checkLock(aStudy)) break;

      LightApp_SelectionMgr *Sel = selectionMgr();
      SALOME_ListIO selected; Sel->selectedObjects( selected );

      if ( vtkwnd ) {
	int nbSel = selected.Extent();
	if (nbSel != 1){
	  break;
	}

	SMESH::SMESH_Mesh_var aMesh;
	SMESH::SMESH_subMesh_var aSubMesh;
	Handle(SALOME_InteractiveObject) IObject = selected.First();
	if (IObject->hasEntry()) {
	  _PTR(SObject) aMeshSObj = aStudy->FindObjectID(IObject->getEntry());
	  GEOM::GEOM_Object_var aShapeObject = SMESH::GetShapeOnMeshOrSubMesh( aMeshSObj );
	  if ( aShapeObject->_is_nil() ) {
	    // imported mesh
	    break;
	  }
	  if( aMeshSObj ) {
	    aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>(aMeshSObj);
	    aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>(aMeshSObj);
            if ( !aSubMesh->_is_nil() )
	      aMesh = aSubMesh->GetFather();

	    if (!aMesh->_is_nil()) {
//               if(!GetSMESHGen()->IsReadyToCompute(aMesh,aShapeObject)) {
//                 SUIT_MessageBox::warn1(desktop(),
//                                        tr("SMESH_WRN_WARNING"),
//                                        tr("SMESH_WRN_MISSING_PARAMETERS"),
//                                        tr("SMESH_BUT_OK"));
//                 break;
//               }
              SMESH::algo_error_array_var errors = GetSMESHGen()->GetAlgoState(aMesh,aShapeObject);
              if ( errors->length() > 0 ) {
                SUIT_MessageBox::warn1(desktop(),
                                       tr("SMESH_WRN_WARNING"),
                                       SMESH::GetMessageOnAlgoStateErrors( errors.in() ),
                                       tr("SMESH_BUT_OK"));
                break;
              }
              try{
                if (GetSMESHGen()->Compute(aMesh,aShapeObject))
                  SMESH::ModifiedMesh(aMeshSObj,true);
                else
                  SUIT_MessageBox::warn1(desktop(),
                                         tr("SMESH_WRN_WARNING"),
                                         tr("SMESH_WRN_COMPUTE_FAILED"),
                                         tr("SMESH_BUT_OK"));
              }
              catch(const SALOME::SALOME_Exception & S_ex){
                SalomeApp_Tools::QtCatchCorbaException(S_ex);
              }
            }
	  }
	}
        CORBA::Long anId = aStudy->StudyId();
        TVisualObjPtr aVisualObj = SMESH::GetVisualObj(anId,IObject->getEntry());
        if ( automaticUpdate() && aVisualObj){
          aVisualObj->Update();
          SMESH_Actor* anActor = SMESH::FindActorByEntry(IObject->getEntry());
          if(!anActor){
            anActor = SMESH::CreateActor(aStudy,IObject->getEntry());
            if(anActor){
	      SMESH::DisplayActor(view,anActor); //apo
	      SMESH::FitAll();
	    }
	  }
	  SMESH::RepaintCurrentView();
	}
      }else{
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"),
			      tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      updateObjBrowser();
      Sel->setSelectedObjects( selected );
      break;
    }

  case 702:  // Create mesh
    startOperation( 702 );
    break;
  case 703:  // Create sub-mesh
    startOperation( 703 );
    break;
  case 704: // Edit mesh/sub-mesh
    startOperation( 704 );
    break;
  case 407: // DIAGONAL INVERSION
  case 408: // Delete diagonal
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      /*Standard_Boolean aRes;
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IObject);
      if ( aMesh->_is_nil() )
      {
        SUIT_MessageBox::warn1(GetDesktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "SMESH_BAD_SELECTION" ), tr( "SMESH_BUT_OK" ) );
        break;
      }
      */
      EmitSignalDeactivateDialog();
      if ( theCommandID == 407 )
        new SMESHGUI_TrianglesInversionDlg(this);
      else
        new SMESHGUI_UnionOfTwoTrianglesDlg(this);
      break;
    }
    case 409: // Change orientation
    case 410: // Union of triangles
    case 411: // Cutting of quadrangles
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();
      SMESHGUI_MultiEditDlg* aDlg = NULL;
      if ( theCommandID == 409 )
        aDlg = new SMESHGUI_ChangeOrientationDlg(this);
      else if ( theCommandID == 410 )
        aDlg = new SMESHGUI_UnionOfTrianglesDlg(this);
      else
        aDlg = new SMESHGUI_CuttingOfQuadsDlg(this);

      aDlg->show();
      break;
    }
  case 412: // Smoothing
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_SmoothingDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 413: // Extrusion
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_ExtrusionDlg ( this );
      } else {
	SUIT_MessageBox::warn1(desktop(),
                               tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
                               tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 414: // Revolution
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_RevolutionDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 415: // Pattern mapping
    {
      if ( checkLock( aStudy ) )
        break;
      if ( vtkwnd )
      {
        EmitSignalDeactivateDialog();
        new SMESHGUI_MeshPatternDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 416: // Extrusion along a path
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_ExtrusionAlongPathDlg( this );
      } else {
	SUIT_MessageBox::warn1(desktop(),
                               tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
                               tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 801:                                     // CREATE GROUP
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_nil();

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      int nbSel = selected.Extent();
      if (nbSel == 1) {
	// check if mesh is selected
        aMesh = SMESH::GetMeshByIO( selected.First() );
      }
      SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, "", aMesh);
      aDlg->show();
      break;
    }

  case 802:                                     // CONSTRUCT GROUP
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      int nbSel = selected.Extent();
      if (nbSel == 1) {
	// check if submesh is selected
	Handle(SALOME_InteractiveObject) IObject = selected.First();
	if (IObject->hasEntry()) {
	  _PTR(SObject) aSObj = aStudy->FindObjectID(IObject->getEntry());
	  if( aSObj ) {
	    SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( SMESH::SObjectToObject( aSObj ) );
	    if (!aSubMesh->_is_nil()) {
	      try {
		SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
		// get submesh elements list by types
		SMESH::long_array_var aNodes = aSubMesh->GetElementsByType(SMESH::NODE);
		SMESH::long_array_var aEdges = aSubMesh->GetElementsByType(SMESH::EDGE);
		SMESH::long_array_var aFaces = aSubMesh->GetElementsByType(SMESH::FACE);
		SMESH::long_array_var aVolumes = aSubMesh->GetElementsByType(SMESH::VOLUME);
		// create group for each type o elements
		QString aName = IObject->getName();
		if (aNodes->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::NODE, aName + "_Nodes");
		  aGroup->Add(aNodes.inout());
		}
		if (aEdges->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::EDGE, aName + "_Edges");
		  aGroup->Add(aEdges.inout());
		}
		if (aFaces->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::FACE, aName + "_Faces");
		  aGroup->Add(aFaces.inout());
		}
		if (aVolumes->length() > 0) {
		  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::VOLUME, aName + "_Volumes");
		  aGroup->Add(aVolumes.inout());
		}
		updateObjBrowser();

	      }catch(const SALOME::SALOME_Exception & S_ex){
		SalomeApp_Tools::QtCatchCorbaException(S_ex);
	      }
	    }
	  }
	}
      }
      break;
    }

  case 803:                                     // EDIT GROUP
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      SALOME_ListIteratorOfListIO It (selected);
      int nbSelectedGroups = 0;
      for ( ; It.More(); It.Next() )
      {
        SMESH::SMESH_GroupBase_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(It.Value());
        if (!aGroup->_is_nil()) {
	  nbSelectedGroups++;
          SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, "", aGroup);
          aDlg->show();
	}
      }
      if (nbSelectedGroups == 0)
	{
	  SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, "", SMESH::SMESH_GroupBase::_nil());
	  aDlg->show();
	}
      break;
    }

  case 804:                                     // Add elements to group
    {
      if(checkLock(aStudy)) break;
      if (myState == 800) {
	SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) myActiveDialogBox;
	if (aDlg) aDlg->onAdd();
      }
      break;
    }

  case 805:                                     // Remove elements from group
    {
      if(checkLock(aStudy)) break;
      if (myState == 800) {
	SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) myActiveDialogBox;
	if (aDlg) aDlg->onRemove();
      }
      break;
    }

    case 810: // Union Groups
    case 811: // Intersect groups
    case 812: // Cut groups
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();

      int aMode;
      if      ( theCommandID == 810 ) aMode = SMESHGUI_GroupOpDlg::UNION;
      else if ( theCommandID == 811 ) aMode = SMESHGUI_GroupOpDlg::INTERSECT;
      else                            aMode = SMESHGUI_GroupOpDlg::CUT;

      ( new SMESHGUI_GroupOpDlg( this, aMode ) )->show();
      break;
    }

    case 813: // Delete groups with their contents
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "NOT_A_VTK_VIEWER" ),tr( "SMESH_BUT_OK" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();

      new SMESHGUI_DeleteGroupDlg( this );
      break;
    }

  case 900:					// MESH INFOS
    {
      EmitSignalDeactivateDialog();
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      if ( selected.Extent() > 1 ) { // a dlg for each IO
        SALOME_ListIO IOs;
        SALOME_ListIteratorOfListIO It (selected);
        for ( ; It.More(); It.Next() ) {
	  IOs.Clear(); IOs.Append( It.Value() );
	  aSel->setSelectedObjects( IOs );
          new SMESHGUI_MeshInfosDlg(this, "", false);
        }
        // restore selection
        aSel->setSelectedObjects( selected );
      }
      else
        new SMESHGUI_MeshInfosDlg(this, "", false);
      break;
    }

  case 902:					// STANDARD MESH INFOS
    {
      EmitSignalDeactivateDialog();
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      if ( selected.Extent() > 1 ) { // a dlg for each IO
        SALOME_ListIO IOs;
        SALOME_ListIteratorOfListIO It (selected);
        for ( ; It.More(); It.Next() ) {
	  IOs.Clear();
	  IOs.Append( It.Value() );
	  aSel->setSelectedObjects( IOs );
          new SMESHGUI_StandardMeshInfosDlg( this, "", false);
        }
        // restore selection
	aSel->setSelectedObjects( selected );
      }
      else
        new SMESHGUI_StandardMeshInfosDlg( this, "", false);
      break;
    }

  case 1100:					// EDIT HYPOTHESIS
    {
      if(checkLock(aStudy)) break;

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      int nbSel = selected.Extent();

      if (nbSel == 1) {
	SMESH::SMESH_Hypothesis_var Hyp = SMESH::IObjectToInterface<SMESH::SMESH_Hypothesis>(selected.First());

        /* Look for all mesh objects that have this hypothesis affected in order to flag as ModifiedMesh */
        /* At end below '...->updateObjBrowser(true)' will change icon of mesh objects                   */
        /* Warning : however by internal mechanism all subMeshes icons are changed !                     */
        if ( !Hyp->_is_nil() )
        {
          char* sName = Hyp->GetName();
          SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator(sName);
          if (aCreator)
            aCreator->edit( Hyp.in(), desktop() );
          else
          {
            // report error
          }
        }
      }
      updateObjBrowser( true );
      break;
    }

  case 1101:					// RENAME
    {
      if ( checkLock( aStudy ) )
        break;

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      SALOME_ListIteratorOfListIO It( selected );
      for ( ; It.More(); It.Next() )
      {
        Handle(SALOME_InteractiveObject) IObject = It.Value();
        _PTR(SObject) obj = aStudy->FindObjectID( IObject->getEntry() );
        _PTR(GenericAttribute) anAttr;
        _PTR(AttributeName) aName;
        if ( obj )
        {
          if ( obj->FindAttribute(anAttr, "AttributeName") )
          {
            aName = anAttr;
            QString newName = QString(aName->Value().c_str());
            newName = LightApp_NameDlg::getName(desktop(), newName);
            if ( !newName.isEmpty() )
            {
              //old source: aStudy->renameIObject( IObject, newName );
	      aName->SetValue( newName.latin1() );

              // if current object is group update group's name
	      SMESH::SMESH_GroupBase_var aGroup =
                SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IObject);
              if (!aGroup->_is_nil() )
                aGroup->SetName( newName.latin1() );

	      updateObjBrowser();
            }
          }
        }
      }
      break;
    }

  case 1102:					// REMOVE HYPOTHESIS / ALGORITHMS
    {
      if(checkLock(aStudy)) break;
      SUIT_OverrideCursor wc;

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected, QString::null, false );

      SALOME_ListIteratorOfListIO It(selected);
      for (int i = 0; It.More(); It.Next(), i++) {
	Handle(SALOME_InteractiveObject) IObject = It.Value();
	SMESH::RemoveHypothesisOrAlgorithmOnMesh(IObject);
      }
      SALOME_ListIO l1;
      aSel->setSelectedObjects( l1 );
      updateObjBrowser();
      break;
    }

  case 401:					// GEOM::EDGE
  case 4021:					// TRIANGLE
  case 4022:					// QUAD
  case 4023:					// POLYGON
  case 4031:					// TETRA
  case 4032:					// HEXA
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();
        SMDSAbs_ElementType type    = SMDSAbs_Edge;
        int                 nbNodes = 2;
        switch (theCommandID) {
        case 4021:                                      // TRIANGLE
          type = SMDSAbs_Face; nbNodes = 3; break;
        case 4022:                                      // QUAD
          type = SMDSAbs_Face; nbNodes = 4; break;
        case 4031:                                      // TETRA
          type = SMDSAbs_Volume; nbNodes = 4; break;
	case 4023:                                      // POLYGON
	  type = SMDSAbs_Face; nbNodes = 5; break;     // 5 - identificator for POLYGON
        case 4032:                                      // HEXA
          type = SMDSAbs_Volume; nbNodes = 8; break;
	case 4033:                                      // POLYHEDRE
	  type = SMDSAbs_Volume; nbNodes = 9; break; // 9 - identificator for POLYHEDRE
        default:;
        }
	new SMESHGUI_AddMeshElementDlg( this, "", type, nbNodes);
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4033:					// POLYHEDRON
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_CreatePolyhedralVolumeDlg(this, "", FALSE );
      }
      else {
	SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4034:     // QUADRATIC EDGE
  case 4035:     // QUADRATIC TRIANGLE
  case 4036:     // QUADRATIC QUADRANGLE
  case 4037:     // QUADRATIC TETRAHEDRON
  case 4038:     // QUADRATIC PYRAMID
  case 4039:     // QUADRATIC PENTAHEDRON
  case 4040:     // QUADRATIC HEXAHEDRON
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();
	int type;

	switch (theCommandID) {
	case 4034:                                      
	  type = QUAD_EDGE; break;
	case 4035:                                      
	  type = QUAD_TRIANGLE; break;
	case 4036:                                     
	  type = QUAD_QUADRANGLE; break;
	case 4037:                                     
	  type = QUAD_TETRAHEDRON; break;
	case 4038:                                     
	  type = QUAD_PYRAMID; break; 
	case 4039:                                     
	  type = QUAD_PENTAHEDRON; break; 
	case 4040:
	  type = QUAD_HEXAHEDRON;
	  break;
	default:;
	}
	 new SMESHGUI_AddQuadraticElementDlg( this, type );
      }
      else {
	SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			       tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			       tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4041:					// REMOVES NODES
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_RemoveNodesDlg(this);
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4042:					// REMOVES ELEMENTS
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_RemoveElementsDlg(this);
      }
      else
	{
	  SUIT_MessageBox::warn1(desktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }
  case 4051:					// RENUMBERING NODES
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_RenumberingDlg( this, "", 0);
      }
      else
	{
	  SUIT_MessageBox::warn1(desktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }
  case 4052:					// RENUMBERING ELEMENTS
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_RenumberingDlg( this, "", 1);
      }
      else
	{
	  SUIT_MessageBox::warn1(desktop(),
				tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
				tr("SMESH_BUT_OK"));
	}
      break;
    }
  case 4061:                                   // TRANSLATION
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_TranslationDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4062:                                   // ROTATION
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_RotationDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4063:                                   // SYMMETRY
    {
      if(checkLock(aStudy)) break;
      if(vtkwnd) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_SymmetryDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4064:                                   // SEWING
    {
      if(checkLock(aStudy)) break;
      if(vtkwnd) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_SewingDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4065:                                   // MERGE NODES
    {
      if(checkLock(aStudy)) break;
      if(vtkwnd) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_MergeNodesDlg( this );
      }
      else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }
  case 4066: // MERGE EQUAL ELEMENTS
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
	EmitSignalDeactivateDialog();
	new SMESHGUI_EditMeshDlg(this,
                                 "SMESH_MERGE_ELEMENTS_TITLE",
                                 "ICON_DLG_MERGE_ELEMENTS",
                                 1); // MergeEqualElemets
      } else {
	SUIT_MessageBox::warn1(desktop(),
			      tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"),
			      tr("SMESH_BUT_OK"));
      }
      break;
    }

  case 5105: // Library of selection filters
  {
    static QValueList<int> aTypes;
    if ( aTypes.isEmpty() )
    {
      aTypes.append( SMESH::NODE );
      aTypes.append( SMESH::EDGE );
      aTypes.append( SMESH::FACE );
      aTypes.append( SMESH::VOLUME );
    }
    new SMESHGUI_FilterLibraryDlg( this, SMESH::GetDesktop( this ), aTypes, SMESHGUI_FilterLibraryDlg::EDIT );
  }
  break;

  case 6017:					// CONTROLS
  case 6016:
  case 6015:
  case 6014:
  case 6013:
  case 6012:
  case 6011:
  case 6001:
  case 6018:
  case 6019:
  case 6002:
  case 6003:
  case 6004:
  case 6009:
    if ( vtkwnd ) {

      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      if ( selected.Extent() == 1 && selected.First()->hasEntry() ) {
	_PTR(SObject) SO = aStudy->FindObjectID( selected.First()->getEntry() );
	if ( SO ) {
	  CORBA::Object_var aObject = SMESH::SObjectToObject( SO );
	  SMESH::SMESH_Mesh_var      aMesh    = SMESH::SMESH_Mesh::_narrow( aObject );
	  SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow( aObject );
	  SMESH::SMESH_GroupBase_var aGroup   = SMESH::SMESH_GroupBase::_narrow( aObject );
	  if ( !aMesh->_is_nil() || !aSubMesh->_is_nil() || !aGroup->_is_nil() ) {
	    ::Control( theCommandID );
	    break;
	  }
	}
      }
      SUIT_MessageBox::warn1(desktop(),
			    tr( "SMESH_WRN_WARNING" ),
			    tr( "SMESH_BAD_SELECTION" ),
			    tr( "SMESH_BUT_OK" ) );
      break;
    }
    else {
      SUIT_MessageBox::warn1(desktop(),
			    tr( "SMESH_WRN_WARNING" ),
			    tr( "NOT_A_VTK_VIEWER" ),
			    tr( "SMESH_BUT_OK" ) );
    }
    break;
  case 9010:
    {
      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      if (selected.Extent() == 1)	{
	Handle(SALOME_InteractiveObject) anIObject = selected.First();
	if(anIObject->hasEntry())
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
	    anActor->SetPointsLabeled( !anActor->GetPointsLabeled() );
	  }
      }
      break;
    }
  case 9011:
    {
      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      if (selected.Extent() == 1)	{
	Handle(SALOME_InteractiveObject) anIObject = selected.First();
	if(anIObject->hasEntry())
	  if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
	    anActor->SetCellsLabeled( !anActor->GetCellsLabeled() );
	  }
      }
      break;
    }
  }

  anApp->updateActions(); //SRN: To update a Save button in the toolbar
  //updateObjBrowser();
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnMousePress( QMouseEvent * pe, SUIT_ViewWindow * wnd )
{
  return false;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnMouseMove( QMouseEvent * pe, SUIT_ViewWindow * wnd )
{
  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::OnKeyPress( QKeyEvent * pe, SUIT_ViewWindow * wnd )
{
  return true;
}

//=============================================================================
/*! Method:  BuildPresentation(const Handle(SALOME_InteractiveObject)& theIO)
 *  Purpose: ensures that the actor for the given <theIO> exists in the active VTK view
 */
//=============================================================================
void SMESHGUI::BuildPresentation( const Handle(SALOME_InteractiveObject) & theIO,
                                  SUIT_ViewWindow* wnd )
{
  if(theIO->hasEntry()){
    //SUIT_ViewWindow* wnd = SMESH::GetActiveWindow();
    SMESH::UpdateView(wnd,SMESH::eDisplay,theIO->getEntry());
  }
}

//=======================================================================
// function : createSMESHAction
// purpose  :
//=======================================================================
void SMESHGUI::createSMESHAction( const int id, const QString& po_id, const QString& icon_id, const int key, const bool toggle  )
{
  QIconSet icon;
  QWidget* parent = application()->desktop();
  SUIT_ResourceMgr* resMgr = resourceMgr();
  QPixmap pix;
  if ( icon_id.length() )
    pix = resMgr->loadPixmap( "SMESH", tr( icon_id ) );
  else
    pix = resMgr->loadPixmap( "SMESH", tr( QString( "ICO_" )+po_id ), false );
  if ( !pix.isNull() )
    icon = QIconSet( pix );

  QString tooltip    = tr( QString( "TOP_" )+po_id ),
          menu       = tr( QString( "MEN_" )+po_id ),
          status_bar = tr( QString( "STB_" )+po_id );

  createAction( id, tooltip, icon, menu, status_bar, key, parent, toggle, this, SLOT( OnGUIEvent() )  );
}

//=======================================================================
// function : createPopupItem
// purpose  :
//=======================================================================
void SMESHGUI::createPopupItem( const int id,
                                const QString& clients,
                                const QString& types,
                                const QString& theRule,
				const int pId )
{
  int parentId = pId;
  if( pId!=-1 )
    parentId = popupMgr()->actionId( action( pId ) );

  if( !popupMgr()->contains( popupMgr()->actionId( action( id ) ) ) )
    popupMgr()->insert( action( id ), parentId, 0 );

  QChar lc = QtxPopupMgr::Selection::defEquality();
  QString rule = "(%1) and (%2) and (%3)";
  rule = rule.arg( QString( "%1>0" ).arg( QtxPopupMgr::Selection::defSelCountParam() ) );
  if( clients.isEmpty() )
    rule = rule.arg( QString( "true" ) );
  else
    rule = rule.arg( QString( "%1client in {%2}" ).arg( lc ).arg( clients ) );
  rule = rule.arg( QString( "%1type in {%2}" ).arg( lc ).arg( types ) );
  rule += theRule;

  bool cont = myRules.contains( id );
  if( cont )
    rule = QString( "%1 or (%2)" ).arg( myRules[ id ] ).arg( rule );

  popupMgr()->setRule( action( id ), rule, true );
  myRules[ id ] = QString( cont ? "%1" : "(%1)" ).arg( rule );
}

//=======================================================================
// function : initialize
// purpose  :
//=======================================================================
void SMESHGUI::initialize( CAM_Application* app )
{
  SalomeApp_Module::initialize( app );

//   SUIT_ResourceMgr* mgr = app->resourceMgr();
//   if ( mgr )
  /* Automatic Update flag */
//     myAutomaticUpdate = mgr->booleanValue( "SMESH", "AutomaticUpdate", myAutomaticUpdate );

  // ----- create actions --------------

  createSMESHAction(  111, "DAT", "", (CTRL+Key_B) );
  createSMESHAction(  112, "UNV", "", (CTRL+Key_U) );
  createSMESHAction(  113, "MED", "", (CTRL+Key_M) );
  createSMESHAction(  114, "NUM" );
  createSMESHAction(  121, "DAT" );
  createSMESHAction(  122, "MED" );
  createSMESHAction(  123, "UNV" );
  createSMESHAction(  124, "EXPORT_DAT" );
  createSMESHAction(  125, "EXPORT_MED" );
  createSMESHAction(  126, "EXPORT_UNV" );
  createSMESHAction(   33, "DELETE",          "ICON_DELETE" );
  createSMESHAction( 5105, "SEL_FILTER_LIB" );
  createSMESHAction(  701, "COMPUTE",         "ICON_COMPUTE" );
  createSMESHAction(  702, "CREATE_MESH",     "ICON_DLG_INIT_MESH" );
  createSMESHAction(  703, "CREATE_SUBMESH",  "ICON_DLG_ADD_SUBMESH" );
  createSMESHAction(  704, "EDIT_MESHSUBMESH","ICON_DLG_EDIT_MESH" );
  createSMESHAction(  801, "CREATE_GROUP",    "ICON_SMESH_TREE_GROUP" );
  createSMESHAction(  802, "CONSTRUCT_GROUP", "ICON_CONSTRUCT_GROUP" );
  createSMESHAction(  803, "EDIT_GROUP",      "ICON_EDIT_GROUP" );
  createSMESHAction(  804, "ADD" );
  createSMESHAction(  805, "REMOVE" );
  createSMESHAction(  810, "UN_GROUP",        "ICON_UNION" );
  createSMESHAction(  811, "INT_GROUP",       "ICON_INTERSECT" );
  createSMESHAction(  812, "CUT_GROUP",       "ICON_CUT" );
  createSMESHAction(  813, "DEL_GROUP",       "ICON_DEL_GROUP" );
  createSMESHAction(  900, "ADV_INFO",        "ICON_ADV_INFO" );
  createSMESHAction(  902, "STD_INFO",        "ICON_STD_INFO" );
  createSMESHAction( 6001, "LENGTH",          "ICON_LENGTH" ,       0, true );
  createSMESHAction( 6002, "FREE_EDGE",       "ICON_FREE_EDGE" ,    0, true );
  createSMESHAction( 6003, "FREE_BORDER",     "ICON_FREE_EDGE_2D" , 0, true );
  createSMESHAction( 6004, "CONNECTION",      "ICON_CONNECTION" ,   0, true );
  createSMESHAction( 6011, "AREA",            "ICON_AREA" ,         0, true );
  createSMESHAction( 6012, "TAPER",           "ICON_TAPER" ,        0, true );
  createSMESHAction( 6013, "ASPECT",          "ICON_ASPECT" ,       0, true );
  createSMESHAction( 6014, "MIN_ANG",         "ICON_ANGLE" ,        0, true );
  createSMESHAction( 6015, "WARP",            "ICON_WARP" ,         0, true );
  createSMESHAction( 6016, "SKEW",            "ICON_SKEW",          0, true );
  createSMESHAction( 6017, "ASPECT_3D",       "ICON_ASPECT_3D",     0, true );
  createSMESHAction( 6018, "LENGTH_2D",       "ICON_LENGTH_2D",     0, true );
  createSMESHAction( 6019, "CONNECTION_2D",   "ICON_CONNECTION_2D", 0, true );
  createSMESHAction( 6009, "VOLUME_3D",       "ICON_VOLUME_3D",     0, true );
  createSMESHAction(  400, "NODE",            "ICON_DLG_NODE" );
  createSMESHAction(  401, "EDGE",            "ICON_DLG_EDGE" );
  createSMESHAction( 4021, "TRIANGLE",        "ICON_DLG_TRIANGLE" );
  createSMESHAction( 4022, "QUAD",            "ICON_DLG_QUADRANGLE" );
  createSMESHAction( 4023, "POLYGON",         "ICON_DLG_POLYGON" );
  createSMESHAction( 4031, "TETRA",           "ICON_DLG_TETRAS" );
  createSMESHAction( 4032, "HEXA",            "ICON_DLG_HEXAS" );
  createSMESHAction( 4041, "REMOVE_NODES",    "ICON_DLG_REM_NODE" );
  createSMESHAction( 4042, "REMOVE_ELEMENTS", "ICON_DLG_REM_ELEMENT" );
  createSMESHAction( 4051, "RENUM_NODES",     "ICON_DLG_RENUMBERING_NODES" );
  createSMESHAction( 4052, "RENUM_ELEMENTS",  "ICON_DLG_RENUMBERING_ELEMENTS" );
  createSMESHAction( 4061, "TRANS",           "ICON_SMESH_TRANSLATION_VECTOR" );
  createSMESHAction( 4062, "ROT",             "ICON_DLG_ROTATION" );
  createSMESHAction( 4063, "SYM",             "ICON_SMESH_SYMMETRY_PLANE" );
  createSMESHAction( 4064, "SEW",             "ICON_SMESH_SEWING_FREEBORDERS" );
  createSMESHAction( 4065, "MERGE",           "ICON_SMESH_MERGE_NODES" );
  createSMESHAction( 4066, "MERGE_ELEMENTS",  "ICON_DLG_MERGE_ELEMENTS" );
  createSMESHAction(  406, "MOVE",            "ICON_DLG_MOVE_NODE" );
  createSMESHAction(  407, "INV",             "ICON_DLG_MESH_DIAGONAL" );
  createSMESHAction(  408, "UNION2",          "ICON_UNION2TRI" );
  createSMESHAction(  409, "ORIENT",          "ICON_DLG_MESH_ORIENTATION" );
  createSMESHAction(  410, "UNION",           "ICON_UNIONTRI" );
  createSMESHAction(  411, "CUT",             "ICON_CUTQUAD" );
  createSMESHAction(  412, "SMOOTH",          "ICON_DLG_SMOOTHING" );
  createSMESHAction(  413, "EXTRUSION",       "ICON_EXTRUSION" );
  createSMESHAction(  414, "REVOLUTION",      "ICON_REVOLUTION" );
  createSMESHAction(  415, "MAP",             "ICON_MAP" );
  createSMESHAction(  416, "EXTRUSION_ALONG", "ICON_EXTRUSION_ALONG" );
  createSMESHAction(  200, "RESET" );
  createSMESHAction(  201, "SCALAR_BAR_PROP" );
  createSMESHAction(  211, "WIRE",           "ICON_WIRE", 0, true );
  createSMESHAction(  212, "SHADE",          "ICON_SHADE", 0, true );
  createSMESHAction(  213, "SHRINK",         "ICON_SHRINK", 0, true );
  createSMESHAction(  214, "UPDATE",         "ICON_UPDATE" );
  createSMESHAction(  215, "NODES",          "ICON_POINTS", 0, true );
  createSMESHAction(  217, "EDGES",          "ICON_DLG_EDGE", 0, true );
  createSMESHAction(  218, "FACES",          "ICON_DLG_TRIANGLE", 0, true );
  createSMESHAction(  219, "VOLUMES",        "ICON_DLG_TETRAS", 0, true );
  createSMESHAction(  220, "ALL" );
  createSMESHAction( 1100, "EDIT_HYPO" );
  createSMESHAction( 1101, "RENAME" );
  createSMESHAction( 1102, "UNASSIGN" );
  createSMESHAction( 9010, "NUM_NODES", "", 0, true );
  createSMESHAction( 9011, "NUM_ELEMENTS", "", 0, true );
  createSMESHAction( 1131, "DISPMODE" );
  createSMESHAction( 1132, "COLORS" );
  createSMESHAction( 1133, "TRANSP" );
  createSMESHAction( 1134, "CLIP" );
  createSMESHAction( 1135, "DISP_ENT" );
  createSMESHAction( 2000, "CTRL" );

  createSMESHAction( 300, "ERASE" );
  createSMESHAction( 301, "DISPLAY" );
  createSMESHAction( 302, "DISPLAY_ONLY" );
  createSMESHAction( 4033, "POLYHEDRON", "ICON_DLG_POLYHEDRON" );
  createSMESHAction( 4034, "QUADRATIC_EDGE", "ICON_DLG_QUADRATIC_EDGE" );
  createSMESHAction( 4035, "QUADRATIC_TRIANGLE", "ICON_DLG_QUADRATIC_TRIANGLE" );
  createSMESHAction( 4036, "QUADRATIC_QUADRANGLE", "ICON_DLG_QUADRATIC_QUADRANGLE" );
  createSMESHAction( 4037, "QUADRATIC_TETRAHEDRON", "ICON_DLG_QUADRATIC_TETRAHEDRON" );
  createSMESHAction( 4038, "QUADRATIC_PYRAMID", "ICON_DLG_QUADRATIC_PYRAMID" );
  createSMESHAction( 4039, "QUADRATIC_PENTAHEDRON", "ICON_DLG_QUADRATIC_PENTAHEDRON" );
  createSMESHAction( 4040, "QUADRATIC_HEXAHEDRON", "ICON_DLG_QUADRATIC_HEXAHEDRON" );

  // ----- create menu --------------
  int fileId   = createMenu( tr( "MEN_FILE" ),   -1,  1 ),
      editId   = createMenu( tr( "MEN_EDIT" ),   -1,  3 ),
      toolsId  = createMenu( tr( "MEN_TOOLS" ),  -1,  5, 50 ),
      meshId   = createMenu( tr( "MEN_MESH" ),   -1, 70, 10 ),
      ctrlId   = createMenu( tr( "MEN_CTRL" ),   -1, 60, 10 ),
      modifyId = createMenu( tr( "MEN_MODIFY" ), -1, 40, 10 ),
      viewId   = createMenu( tr( "MEN_VIEW" ),   -1,  2 );

  createMenu( separator(), fileId );

  int importId = createMenu( tr( "MEN_IMPORT" ), fileId, 11, 10 ),
      exportId = createMenu( tr( "MEN_EXPORT" ), fileId, 12, 10 ),
      addId    = createMenu( tr( "MEN_ADD" ),    modifyId, 402 ),
      removeId = createMenu( tr( "MEN_REMOVE" ), modifyId, 403 ),
      renumId  = createMenu( tr( "MEN_RENUM" ),  modifyId, 404 ),
      transfId = createMenu( tr( "MEN_TRANSF" ), modifyId, 405 );

  createMenu( 111, importId, -1 );
  createMenu( 112, importId, -1 );
  createMenu( 113, importId, -1 );

  createMenu( 121, exportId, -1 );
  createMenu( 122, exportId, -1 );
  createMenu( 123, exportId, -1 );

  createMenu( separator(), fileId, 10 );

  createMenu( 33, editId, -1 );

  createMenu( 5105, toolsId, -1 );

  createMenu( 702, meshId, -1 );
  createMenu( 703, meshId, -1 );
  createMenu( 704, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 701, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 801, meshId, -1 );
  createMenu( 802, meshId, -1 );
  createMenu( 803, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 810, meshId, -1 );
  createMenu( 811, meshId, -1 );
  createMenu( 812, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 813, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 900, meshId, -1 );
  createMenu( 902, meshId, -1 );
  createMenu( separator(), meshId, -1 );

  createMenu( 6003, ctrlId, -1 );
  createMenu( 6001, ctrlId, -1 );
  createMenu( 6004, ctrlId, -1 );
  createMenu( separator(), ctrlId, -1 );
  createMenu( 6002, ctrlId, -1 );
  createMenu( 6018, ctrlId, -1 );
  createMenu( 6019, ctrlId, -1 );
  createMenu( 6011, ctrlId, -1 );
  createMenu( 6012, ctrlId, -1 );
  createMenu( 6013, ctrlId, -1 );
  createMenu( 6014, ctrlId, -1 );
  createMenu( 6015, ctrlId, -1 );
  createMenu( 6016, ctrlId, -1 );
  createMenu( separator(), ctrlId, -1 );
  createMenu( 6017, ctrlId, -1 );
  createMenu( 6009, ctrlId, -1 );
  createMenu( separator(), ctrlId, -1 );

  createMenu( 400, addId, -1 );
  createMenu( 401, addId, -1 );
  createMenu( 4021, addId, -1 );
  createMenu( 4022, addId, -1 );
  createMenu( 4023, addId, -1 );
  createMenu( 4031, addId, -1 );
  createMenu( 4032, addId, -1 );
  createMenu( 4033, addId, -1 );
  createMenu( separator(), addId, -1 );
  createMenu( 4034, addId, -1 );
  createMenu( 4035, addId, -1 );
  createMenu( 4036, addId, -1 );
  createMenu( 4037, addId, -1 );
  createMenu( 4038, addId, -1 );
  createMenu( 4039, addId, -1 );
  createMenu( 4040, addId, -1 );

  createMenu( 4041, removeId, -1 );
  createMenu( 4042, removeId, -1 );

  createMenu( 4051, renumId, -1 );
  createMenu( 4052, renumId, -1 );

  createMenu( 4061, transfId, -1 );
  createMenu( 4062, transfId, -1 );
  createMenu( 4063, transfId, -1 );
  createMenu( 4064, transfId, -1 );
  createMenu( 4065, transfId, -1 );
  createMenu( 4066, transfId, -1 );

  createMenu( 406, modifyId, -1 );
  createMenu( 407, modifyId, -1 );
  createMenu( 408, modifyId, -1 );
  createMenu( 409, modifyId, -1 );
  createMenu( 410, modifyId, -1 );
  createMenu( 411, modifyId, -1 );
  createMenu( 412, modifyId, -1 );
  createMenu( 413, modifyId, -1 );
  createMenu( 416, modifyId, -1 );
  createMenu( 414, modifyId, -1 );
  createMenu( 415, modifyId, -1 );

  createMenu( 214, viewId, -1 );

  // ----- create toolbars --------------
  int meshTb     = createTool( tr( "TB_MESH" ) ),
      ctrlTb     = createTool( tr( "TB_CTRL" ) ),
      addRemTb   = createTool( tr( "TB_ADD_REMOVE" ) ),
      modifyTb   = createTool( tr( "TB_MODIFY" ) ),
      dispModeTb = createTool( tr( "TB_DISP_MODE" ) );

  createTool( 702, meshTb );
  createTool( 703, meshTb );
  createTool( 704, meshTb );
  createTool( separator(), meshTb );
  createTool( 701, meshTb );
  createTool( separator(), meshTb );
  createTool( 801, meshTb );
  createTool( 802, meshTb );
  createTool( 803, meshTb );
  createTool( separator(), meshTb );
  createTool( 900, meshTb );
  createTool( 902, meshTb );
  createTool( separator(), meshTb );

  createTool( 6001, ctrlTb );
  createTool( 6003, ctrlTb );
  createTool( 6004, ctrlTb );
  createTool( separator(), ctrlTb );
  createTool( 6002, ctrlTb );
  createTool( 6018, ctrlTb );
  createTool( 6019, ctrlTb );
  createTool( 6011, ctrlTb );
  createTool( 6012, ctrlTb );
  createTool( 6013, ctrlTb );
  createTool( 6014, ctrlTb );
  createTool( 6015, ctrlTb );
  createTool( 6016, ctrlTb );
  createTool( separator(), ctrlTb );
  createTool( 6017, ctrlTb );
  createTool( 6009, ctrlTb );
  createTool( separator(), ctrlTb );

  createTool( 400, addRemTb );
  createTool( 401, addRemTb );
  createTool( 4021, addRemTb );
  createTool( 4022, addRemTb );
  createTool( 4023, addRemTb );
  createTool( 4031, addRemTb );
  createTool( 4032, addRemTb );
  createTool( 4033, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4034, addRemTb );
  createTool( 4035, addRemTb );
  createTool( 4036, addRemTb );
  createTool( 4037, addRemTb );
  createTool( 4038, addRemTb );
  createTool( 4039, addRemTb );
  createTool( 4040, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4041, addRemTb );
  createTool( 4042, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4051, addRemTb );
  createTool( 4052, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4061, addRemTb );
  createTool( 4062, addRemTb );
  createTool( 4063, addRemTb );
  createTool( 4064, addRemTb );
  createTool( 4065, addRemTb );
  createTool( 4066, addRemTb );
  createTool( separator(), addRemTb );

  createTool( 406, modifyTb );
  createTool( 407, modifyTb );
  createTool( 408, modifyTb );
  createTool( 409, modifyTb );
  createTool( 410, modifyTb );
  createTool( 411, modifyTb );
  createTool( 412, modifyTb );
  createTool( 413, modifyTb );
  createTool( 416, modifyTb );
  createTool( 414, modifyTb );
  createTool( 415, modifyTb );

  createTool( 214, dispModeTb );


  myRules.clear();
  QString OB = "'ObjectBrowser'",
	  View = "'" + SVTK_Viewer::Type() + "'",
	  pat = "'%1'",
	  mesh    = pat.arg( SMESHGUI_Selection::typeName( MESH ) ),
	  group   = pat.arg( SMESHGUI_Selection::typeName( GROUP ) ),
	  hypo    = pat.arg( SMESHGUI_Selection::typeName( HYPOTHESIS ) ),
	  algo    = pat.arg( SMESHGUI_Selection::typeName( ALGORITHM ) ),
	  elems   = QString( "'%1' '%2' '%3' '%4' '%5' '%6'" ).
                       arg( SMESHGUI_Selection::typeName( SUBMESH_VERTEX ) ).
		       arg( SMESHGUI_Selection::typeName( SUBMESH_EDGE ) ).
		       arg( SMESHGUI_Selection::typeName( SUBMESH_FACE ) ).
		       arg( SMESHGUI_Selection::typeName( SUBMESH_SOLID ) ).
		       arg( SMESHGUI_Selection::typeName( SUBMESH_COMPOUND ) ).
		       arg( SMESHGUI_Selection::typeName( SUBMESH ) ),
          subMesh = elems,
	  mesh_group = mesh + " " + subMesh + " " + group,
	  hyp_alg = hypo + " " + algo;

  // popup for object browser

  createPopupItem( 704, OB, mesh, "&& isComputable");      // EDIT_MESHSUBMESH
  createPopupItem( 704, OB, subMesh, "&& isComputable" );  // EDIT_MESHSUBMESH
  createPopupItem( 803, OB, group );                       // EDIT_GROUP
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 701, OB, mesh, "&& isComputable" );     // COMPUTE
  createPopupItem( 214, OB, mesh_group );                  // UPDATE
  createPopupItem( 900, OB, mesh_group );                  // ADV_INFO
  createPopupItem( 902, OB, mesh );                        // STD_INFO
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 801, OB, mesh );                        // CREATE_GROUP
  createPopupItem( 802, OB, subMesh );                     // CONSTRUCT_GROUP
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 1100, OB, hypo, "" /*"&& $hasReference={false}"*/ );   // EDIT HYPOTHESIS
  createPopupItem( 1102, OB, hyp_alg ); // REMOVE HYPOTHESIS / ALGORITHMS
  createPopupItem( 1101, OB, mesh_group + " " + hyp_alg, "" /*"&& $hasReference={false}"*/ ); // RENAME
  popupMgr()->insert( separator(), -1, 0 );

  QString only_one_non_empty = QString( " && %1=1 && numberOfNodes>0" ).arg( QtxPopupMgr::Selection::defSelCountParam() );
  createPopupItem( 125, OB, mesh, only_one_non_empty );    // EXPORT_MED
  createPopupItem( 126, OB, mesh, only_one_non_empty );    // EXPORT_UNV
  createPopupItem( 33, OB, subMesh + " " + group );        // DELETE
  popupMgr()->insert( separator(), -1, 0 );

  // popup for viewer
  createPopupItem( 803, View, group ); // EDIT_GROUP
  createPopupItem( 804, View, elems ); // ADD
  createPopupItem( 805, View, elems ); // REMOVE
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 214, View, mesh_group ); // UPDATE
  createPopupItem( 900, View, mesh_group ); // ADV_INFO
  createPopupItem( 902, View, mesh );       // STD_INFO
  popupMgr()->insert( separator(), -1, 0 );

  int anId;
  QString
    isInvisible("not( isVisible )"),
    isEmpty("numberOfNodes = 0"),
    isNotEmpty("numberOfNodes <> 0"),

    // has nodes, edges, etc in VISIBLE! actor
    hasNodes("(numberOfNodes > 0 )"),//&& isVisible)"),
    hasElems("(count( elemTypes ) > 0)"),
    hasDifferentElems("(count( elemTypes ) > 1)"),
    hasEdges("({'Edge'} in elemTypes)"),
    hasFaces("({'Face'} in elemTypes)"),
    hasVolumes("({'Volume'} in elemTypes)");

  QString lc = QtxPopupMgr::Selection::defEquality();
  QString aClient = QString( "%1client in {%2}" ).arg( lc ).arg( "'VTKViewer'" );
  QString aType = QString( "%1type in {%2}" ).arg( QtxPopupMgr::Selection::defEquality() );
  aType = aType.arg( mesh_group );
  QString aMeshInVTK = aClient + "&&" + aType;

  //-------------------------------------------------
  // Numbering
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_NUM" ), -1, -1 );

  popupMgr()->insert( action( 9010 ), anId, -1 );
  popupMgr()->setRule( action( 9010 ), aMeshInVTK + "&& isVisible &&" + hasNodes, true );
  popupMgr()->setRule( action( 9010 ), "{'Point'} in labeledTypes", false );

  popupMgr()->insert( action( 9011 ), anId, -1 );
  popupMgr()->setRule( action( 9011 ), aMeshInVTK + "&& isVisible &&" + hasElems, true );
  popupMgr()->setRule( action( 9011 ), "{'Cell'} in labeledTypes", false );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Display Mode
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_DISPMODE" ), -1, -1 );

  popupMgr()->insert( action( 211 ), anId, -1 ); // WIRE
  popupMgr()->setRule( action( 211 ), aMeshInVTK + "&&" + hasElems, true );
  popupMgr()->setRule( action( 211 ), "displayMode = 'eEdge'", false );

  popupMgr()->insert( action( 212 ), anId, -1 ); // SHADE
  popupMgr()->setRule( action( 212 ),aMeshInVTK+ "&& (" + hasFaces + "||" + hasVolumes + ")",true);
  popupMgr()->setRule( action( 212 ), "displayMode = 'eSurface'", false );

  popupMgr()->insert( action( 215 ), anId, -1 ); // POINTS
  popupMgr()->setRule( action( 215 ), aMeshInVTK + "&&" + hasNodes, true );
  popupMgr()->setRule( action( 215 ), "displayMode = 'ePoint'", false );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 213 ), anId, -1 ); // SHRINK
  popupMgr()->setRule( action( 213 ), aMeshInVTK + "&& shrinkMode <> 'IsNotShrinkable' && displayMode <> 'ePoint'",true);
  popupMgr()->setRule( action( 213 ), "shrinkMode = 'IsShrunk'", false );

  //-------------------------------------------------
  // Display Entity
  //-------------------------------------------------
  QString aDiffElemsInVTK = aMeshInVTK + "&&" + hasDifferentElems;

  anId = popupMgr()->insert( tr( "MEN_DISP_ENT" ), -1, -1 );

  popupMgr()->insert( action( 217 ), anId, -1 ); // EDGES
  popupMgr()->setRule( action( 217 ), aDiffElemsInVTK + "&& isVisible &&" + hasEdges, true );
  popupMgr()->setRule( action( 217 ), "{'Edge'} in entityMode", false );

  popupMgr()->insert( action( 218 ), anId, -1 ); // FACES
  popupMgr()->setRule( action( 218 ), aDiffElemsInVTK + "&& isVisible &&" + hasFaces, true );
  popupMgr()->setRule( action( 218 ), "{'Face'} in entityMode", false );

  popupMgr()->insert( action( 219 ), anId, -1 ); // VOLUMES
  popupMgr()->setRule( action( 219 ), aDiffElemsInVTK + "&& isVisible &&" + hasVolumes, true );
  popupMgr()->setRule( action( 219 ), "{'Volume'} in entityMode", false );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 220 ), anId, -1 ); // ALL
  popupMgr()->setRule( action( 220 ), aDiffElemsInVTK + "&& isVisible && not( elemTypes in entityMode )", true );

  //-------------------------------------------------
  // Color / Size
  //-------------------------------------------------
  popupMgr()->insert( action( 1132 ), -1, -1 );
  popupMgr()->setRule( action( 1132 ), aMeshInVTK + "&& isVisible", true );

  //-------------------------------------------------
  // Transparency
  //-------------------------------------------------
  popupMgr()->insert( action( 1133 ), -1, -1 );
  popupMgr()->setRule( action( 1133 ), aMeshInVTK + "&& isVisible", true );

  //-------------------------------------------------
  // Clipping
  //-------------------------------------------------
  popupMgr()->insert( action( 1134 ), -1, -1 );
  popupMgr()->setRule( action( 1134 ), aMeshInVTK + "&& isVisible", true );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Controls
  //-------------------------------------------------
  QString
    aMeshInVtkHasEdges = aMeshInVTK + "&&" + hasEdges,
    aMeshInVtkHasFaces = aMeshInVTK + "&&" + hasFaces,
    aMeshInVtkHasVolumes = aMeshInVTK + "&&" + hasVolumes;

  anId = popupMgr()->insert( tr( "MEN_CTRL" ), -1, -1 );

  popupMgr()->insert( action( 200 ), anId, -1 ); // RESET
  popupMgr()->setRule( action( 200 ), aMeshInVTK + "&& controlMode <> 'eNone'", true );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 6003 ), anId, -1 ); // FREE_BORDER
  popupMgr()->setRule( action( 6003 ), aMeshInVtkHasEdges, true );
  popupMgr()->setRule( action( 6003 ), "controlMode = 'eFreeEdges'", false );

  popupMgr()->insert( action( 6001 ), anId, -1 ); // LENGTH
  popupMgr()->setRule( action( 6001 ), aMeshInVtkHasEdges, true );
  popupMgr()->setRule( action( 6001 ), "controlMode = 'eLength'", false );

  popupMgr()->insert( action( 6004 ), anId, -1 ); // CONNECTION
  popupMgr()->setRule( action( 6004 ), aMeshInVtkHasEdges, true );
  popupMgr()->setRule( action( 6004 ), "controlMode = 'eMultiConnection'", false );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 6002 ), anId, -1 ); // FREE_EDGE
  popupMgr()->setRule( action( 6002 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6002 ), "controlMode = 'eFreeBorders'", false );

  popupMgr()->insert( action( 6018 ), anId, -1 ); // LENGTH_2D
  popupMgr()->setRule( action( 6018 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6018 ), "controlMode = 'eLength2D'", false );

  popupMgr()->insert( action( 6019 ), anId, -1 ); // CONNECTION_2D
  popupMgr()->setRule( action( 6019 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6019 ), "controlMode = 'eMultiConnection2D'", false );

  popupMgr()->insert( action( 6011 ), anId, -1 ); // AREA
  popupMgr()->setRule( action( 6011 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6011 ), "controlMode = 'eArea'", false );

  popupMgr()->insert( action( 6012 ), anId, -1 ); // TAPER
  popupMgr()->setRule( action( 6012 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6012 ), "controlMode = 'eTaper'", false );

  popupMgr()->insert( action( 6013 ), anId, -1 ); // ASPECT
  popupMgr()->setRule( action( 6013 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6013 ), "controlMode = 'eAspectRatio'", false );

  popupMgr()->insert( action( 6014 ), anId, -1 ); // MIN_ANG
  popupMgr()->setRule( action( 6014 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6014 ), "controlMode = 'eMinimumAngle'", false );

  popupMgr()->insert( action( 6015 ), anId, -1 ); // WARP
  popupMgr()->setRule( action( 6015 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6015 ), "controlMode = 'eWarping'", false );

  popupMgr()->insert( action( 6016 ), anId, -1 ); // SKEW
  popupMgr()->setRule( action( 6016 ), aMeshInVtkHasFaces, true );
  popupMgr()->setRule( action( 6016 ), "controlMode = 'eSkew'", false );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 6017 ), anId, -1 ); // ASPECT_3D
  popupMgr()->setRule( action( 6017 ), aMeshInVtkHasVolumes, true );
  popupMgr()->setRule( action( 6017 ), "controlMode = 'eAspectRatio3D'", false );

  popupMgr()->insert ( action( 6009 ), anId, -1 ); // VOLUME_3D
  popupMgr()->setRule( action( 6009 ), aMeshInVtkHasVolumes, true );
  popupMgr()->setRule( action( 6009 ), "controlMode = 'eVolume3D'", false );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 201 ), anId, -1 ); // SCALAR_BAR_PROP
  popupMgr()->setRule( action( 201 ), aMeshInVTK + "&& controlMode <> 'eNone'", true );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Display / Erase
  //-------------------------------------------------
  aClient = "($client in {'VTKViewer' 'ObjectBrowser'})";
  QString anActiveVTK = QString("activeView = '%1'").arg(SVTK_Viewer::Type());
  QString aSelCount = QString( "%1 > 0" ).arg( QtxPopupMgr::Selection::defSelCountParam() );

  QString aRule = "$component={'SMESH'} and ( type='Component' or (" + aClient + " and " +
    aType + " and " + aSelCount + " and " + anActiveVTK + " and " + isNotEmpty + " %1 ) )";
  popupMgr()->insert( action( 301 ), -1, -1 ); // DISPLAY
  popupMgr()->setRule( action( 301 ), aRule.arg( "and (not isVisible)" ), true);

  popupMgr()->insert( action( 300 ), -1, -1 ); // ERASE
  popupMgr()->setRule( action( 300 ), aRule.arg( "and isVisible" ), true );

  popupMgr()->insert( action( 302 ), -1, -1 ); // DISPLAY_ONLY
  popupMgr()->setRule( action( 302 ), aRule.arg( "" ), true );

  popupMgr()->insert( separator(), -1, -1 );

  connect( application(), SIGNAL( viewManagerActivated( SUIT_ViewManager* ) ),
	   this, SLOT( onViewManagerActivated( SUIT_ViewManager* ) ) );
}

bool SMESHGUI::activateModule( SUIT_Study* study )
{
  bool res = SalomeApp_Module::activateModule( study );

  setMenuShown( true );
  setToolShown( true );

  return res;
}

bool SMESHGUI::deactivateModule( SUIT_Study* study )
{
  setMenuShown( false );
  setToolShown( false );

  EmitSignalCloseAllDialogs();

  return SalomeApp_Module::deactivateModule( study );
}

void SMESHGUI::OnGUIEvent()
{
  const QObject* obj = sender();
  if ( !obj || !obj->inherits( "QAction" ) )
    return;
  int id = actionId((QAction*)obj);
  if ( id != -1 )
    OnGUIEvent( id );
}

SMESH::SMESH_Gen_var SMESHGUI::GetSMESHGen()
{
  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument(); //Document OCAF de l'etude active
  if ( CORBA::is_nil( myComponentSMESH ) )
    {
      SMESHGUI aGUI; //SRN BugID: IPAL9186: Create an instance of SMESHGUI to initialize myComponentSMESH
      if ( aStudy )
        aGUI.myComponentSMESH->SetCurrentStudy(_CAST(Study,aStudy)->GetStudy());
      return aGUI.myComponentSMESH;
    }
  if ( aStudy )
    myComponentSMESH->SetCurrentStudy(_CAST(Study,aStudy)->GetStudy());
  return myComponentSMESH;
}

QString SMESHGUI::engineIOR() const
{
  CORBA::ORB_var anORB = getApp()->orb();
  CORBA::String_var anIOR = anORB->object_to_string(GetSMESHGen());
  return anIOR.in();
}

void SMESHGUI::contextMenuPopup( const QString& client, QPopupMenu* menu, QString& /*title*/ )
{
  SMESHGUI_Selection sel;
  sel.init( client, selectionMgr() );
  popupMgr()->updatePopup( menu, &sel );
}

void SMESHGUI::windows( QMap<int, int>& aMap ) const
{
  aMap.insert( SalomeApp_Application::WT_ObjectBrowser, Qt::DockLeft );
  aMap.insert( SalomeApp_Application::WT_PyConsole, Qt::DockBottom );
}

void SMESHGUI::viewManagers( QStringList& list ) const
{
  list.append( SVTK_Viewer::Type() );
}

void SMESHGUI::onViewManagerActivated( SUIT_ViewManager* mgr )
{
  if ( dynamic_cast<SVTK_ViewManager*>( mgr ) )
    SMESH::UpdateSelectionProp( this );
}

void SMESHGUI::createPreferences()
{
  int genTab = addPreference( tr( "PREF_TAB_GENERAL" ) );

  int updateGroup = addPreference( tr( "PREF_GROUP_UPDATE" ), genTab );
  addPreference( tr( "PREF_AUTO_UPDATE" ), updateGroup, LightApp_Preferences::Bool, "SMESH", "auto_update" );

  int qaGroup = addPreference( tr( "PREF_GROUP_QUALITY" ), genTab );
  addPreference( tr( "PREF_DISPLAY_ENTITY" ), qaGroup, LightApp_Preferences::Bool, "SMESH", "display_entity" );
  addPreference( tr( "PREF_PRECISION_USE" ), qaGroup, LightApp_Preferences::Bool, "SMESH", "use_precision" );

  int precGroup = addPreference( tr( "PREF_GROUP_PRECISION" ), genTab );
  setPreferenceProperty( precGroup, "columns", 1 );

  int prec = addPreference( tr( "PREF_PRECISION_VALUE" ), precGroup, LightApp_Preferences::IntSpin, "SMESH", "controls_precision" );

  setPreferenceProperty( prec, "min", 0 );
  setPreferenceProperty( prec, "max", 16 );

  int dispgroup = addPreference( tr( "PREF_DISPLAY_MODE" ), genTab );
  int dispmode = addPreference( tr( "PREF_DISPLAY_MODE" ), dispgroup, LightApp_Preferences::Selector, "SMESH", "display_mode" );
  QStringList modes;
  modes.append( "Wireframe" );
  modes.append( "Shading" );
  modes.append( "Nodes" );
  modes.append( "Shrink" );
  QValueList<QVariant> indices;
  indices.append( 0 );
  indices.append( 1 );
  indices.append( 2 );
  indices.append( 3 );
  setPreferenceProperty( dispmode, "strings", modes );
  setPreferenceProperty( dispmode, "indexes", indices );

  int exportgroup = addPreference( tr( "PREF_GROUP_EXPORT" ), genTab );
  addPreference( tr( "PREF_AUTO_GROUPS" ), exportgroup, LightApp_Preferences::Bool, "SMESH", "auto_groups" );
  
  int meshTab = addPreference( tr( "PREF_TAB_MESH" ) );
  int nodeGroup = addPreference( tr( "PREF_GROUP_NODES" ), meshTab );

  addPreference( tr( "PREF_COLOR" ), nodeGroup, LightApp_Preferences::Color, "SMESH", "node_color" );
  int nodeSz = addPreference( tr( "PREF_SIZE" ), nodeGroup, LightApp_Preferences::IntSpin, "SMESH", "node_size" );

  setPreferenceProperty( nodeSz, "min", 1 );
  setPreferenceProperty( nodeSz, "max", 5 );

  int elemGroup = addPreference( tr( "PREF_GROUP_ELEMENTS" ), meshTab );

  addPreference( tr( "PREF_FILL" ), elemGroup, LightApp_Preferences::Color, "SMESH", "fill_color" );
  addPreference( tr( "PREF_OUTLINE" ), elemGroup, LightApp_Preferences::Color, "SMESH", "outline_color" );
  addPreference( tr( "PREF_BACKFACE" ), elemGroup, LightApp_Preferences::Color, "SMESH", "backface_color" );
  addPreference( "", elemGroup, LightApp_Preferences::Space );

  int elemW = addPreference( tr( "PREF_WIDTH" ), elemGroup, LightApp_Preferences::IntSpin, "SMESH", "element_width" );
  int shrink = addPreference( tr( "PREF_SHRINK_COEFF" ), elemGroup, LightApp_Preferences::IntSpin, "SMESH", "shrink_coeff" );

  setPreferenceProperty( elemW, "min", 1 );
  setPreferenceProperty( elemW, "max", 5 );

  setPreferenceProperty( shrink, "min", 0 );
  setPreferenceProperty( shrink, "max", 100 );

  int selTab = addPreference( tr( "PREF_TAB_SELECTION" ) );

  int selGroup = addPreference( tr( "PREF_GROUP_SELECTION" ), selTab );

  addPreference( tr( "PREF_OBJECT_COLOR" ), selGroup, LightApp_Preferences::Color, "SMESH", "selection_object_color" );
  addPreference( tr( "PREF_ELEMENT_COLOR" ), selGroup, LightApp_Preferences::Color, "SMESH", "selection_element_color" );
  int selW = addPreference( tr( "PREF_WIDTH" ), selGroup, LightApp_Preferences::IntSpin, "SMESH", "selection_width" );

  setPreferenceProperty( selW, "min", 1 );
  setPreferenceProperty( selW, "max", 5 );

  int preGroup = addPreference( tr( "PREF_GROUP_PRESELECTION" ), selTab );

  addPreference( tr( "PREF_HIGHLIGHT_COLOR" ), preGroup, LightApp_Preferences::Color, "SMESH", "highlight_color" );
  int preW = addPreference( tr( "PREF_WIDTH" ), preGroup, LightApp_Preferences::IntSpin, "SMESH", "highlight_width" );

  setPreferenceProperty( preW, "min", 1 );
  setPreferenceProperty( preW, "max", 5 );

  int precSelGroup = addPreference( tr( "PREF_GROUP_PRECISION" ), selTab );

  addPreference( tr( "PREF_NODES" ), precSelGroup, LightApp_Preferences::Double, "SMESH", "selection_precision_node" );
  addPreference( tr( "PREF_ELEMENTS" ), precSelGroup, LightApp_Preferences::Double, "SMESH", "selection_precision_element" );

  int sbarTab = addPreference( tr( "SMESH_SCALARBAR" ) );
  int fontGr = addPreference( tr( "SMESH_FONT_SCALARBAR" ), sbarTab );

  int tfont = addPreference( tr( "SMESH_TITLE" ), fontGr, LightApp_Preferences::Font, "SMESH", "scalar_bar_title_font" );
  addPreference( tr( "SMESH_TITLE" ), fontGr, LightApp_Preferences::Color, "SMESH", "scalar_bar_title_color" );
  int lfont = addPreference( tr( "SMESH_LABELS" ), fontGr, LightApp_Preferences::Font, "SMESH", "scalar_bar_label_font" );
  addPreference( tr( "SMESH_LABELS" ), fontGr, LightApp_Preferences::Color, "SMESH", "scalar_bar_label_color" );

  QStringList fam;
  fam.append( tr( "SMESH_FONT_ARIAL" ) );
  fam.append( tr( "SMESH_FONT_COURIER" ) );
  fam.append( tr( "SMESH_FONT_TIMES" ) );
  int wflag = ( QtxListResourceEdit::FontItem::All & ( ~( QtxListResourceEdit::FontItem::Size | QtxListResourceEdit::FontItem::UserSize ) ) );

  setPreferenceProperty( tfont, "families", fam );
  setPreferenceProperty( tfont, "system", false );
  setPreferenceProperty( tfont, "widget_flags", wflag );
  setPreferenceProperty( lfont, "families", fam );
  setPreferenceProperty( lfont, "system", false );
  setPreferenceProperty( lfont, "widget_flags", wflag );

  int colorsLabelsGr = addPreference( tr( "SMESH_LABELS_COLORS_SCALARBAR" ), sbarTab );

  int numcol = addPreference( tr( "SMESH_NUMBEROFCOLORS" ), colorsLabelsGr, LightApp_Preferences::IntSpin, "SMESH", "scalar_bar_num_colors" );
  setPreferenceProperty( numcol, "min", 2 );
  setPreferenceProperty( numcol, "max", 256 );

  int numlab = addPreference( tr( "SMESH_NUMBEROFLABELS" ), colorsLabelsGr, LightApp_Preferences::IntSpin, "SMESH", "scalar_bar_num_labels" );
  setPreferenceProperty( numlab, "min", 2 );
  setPreferenceProperty( numlab, "max", 65 );

  int orientGr = addPreference( tr( "SMESH_ORIENTATION" ), sbarTab );
  int orient = addPreference( tr( "SMESH_ORIENTATION" ), orientGr, LightApp_Preferences::Selector, "SMESH", "scalar_bar_orientation" );
  QStringList orients;
  orients.append( tr( "SMESH_VERTICAL" ) );
  orients.append( tr( "SMESH_HORIZONTAL" ) );
  indices.clear(); indices.append( 0 ); indices.append( 1 );
  setPreferenceProperty( orient, "strings", orients );
  setPreferenceProperty( orient, "indexes", indices );

  int posVSizeGr = addPreference( tr( "SMESH_POSITION_SIZE_SCALARBAR" ) + " " + tr( "SMESH_VERTICAL" ), sbarTab );
  int xv = addPreference( tr( "SMESH_X_SCALARBAR" ), posVSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_vertical_x" );
  int yv = addPreference( tr( "SMESH_Y_SCALARBAR" ), posVSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_vertical_y" );
  int wv = addPreference( tr( "SMESH_WIDTH" ), posVSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_vertical_width" );
  int hv = addPreference( tr( "SMESH_HEIGHT" ), posVSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_vertical_height" );
  setPreferenceProperty( xv, "step", 0.1 );
  setPreferenceProperty( xv, "min", 0.0 );
  setPreferenceProperty( xv, "max", 1.0 );
  setPreferenceProperty( yv, "step", 0.1 );
  setPreferenceProperty( yv, "min", 0.0 );
  setPreferenceProperty( yv, "max", 1.0 );
  setPreferenceProperty( wv, "step", 0.1 );
  setPreferenceProperty( wv, "min", 0.0 );
  setPreferenceProperty( wv, "max", 1.0 );
  setPreferenceProperty( hv, "min", 0.0 );
  setPreferenceProperty( hv, "max", 1.0 );
  setPreferenceProperty( hv, "step", 0.1 );

  int posHSizeGr = addPreference( tr( "SMESH_POSITION_SIZE_SCALARBAR" ) + " " + tr( "SMESH_HORIZONTAL" ), sbarTab );
  int xh = addPreference( tr( "SMESH_X_SCALARBAR" ), posHSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_horizontal_x" );
  int yh = addPreference( tr( "SMESH_Y_SCALARBAR" ), posHSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_horizontal_y" );
  int wh = addPreference( tr( "SMESH_WIDTH" ), posHSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_horizontal_width" );
  int hh = addPreference( tr( "SMESH_HEIGHT" ), posHSizeGr, LightApp_Preferences::DblSpin, "SMESH", "scalar_bar_horizontal_height" );
  setPreferenceProperty( xv, "min", 0.0 );
  setPreferenceProperty( xv, "max", 1.0 );
  setPreferenceProperty( xv, "step", 0.1 );
  setPreferenceProperty( xh, "min", 0.0 );
  setPreferenceProperty( xh, "max", 1.0 );
  setPreferenceProperty( xh, "step", 0.1 );
  setPreferenceProperty( yh, "min", 0.0 );
  setPreferenceProperty( yh, "max", 1.0 );
  setPreferenceProperty( yh, "step", 0.1 );
  setPreferenceProperty( wh, "min", 0.0 );
  setPreferenceProperty( wh, "max", 1.0 );
  setPreferenceProperty( wh, "step", 0.1 );
  setPreferenceProperty( hh, "min", 0.0 );
  setPreferenceProperty( hh, "max", 1.0 );
  setPreferenceProperty( hh, "step", 0.1 );
}

void SMESHGUI::preferencesChanged( const QString& sect, const QString& name )
{
  if( sect=="SMESH" ){
    float sbX1,sbY1,sbW,sbH;
    float aTol = 1.00000009999999;
    std::string aWarning;
    SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr(this);
    if( name=="selection_object_color" || name=="selection_element_color" || 
        name=="selection_width" || name=="highlight_color" || name=="highlight_width" ||
        name=="selection_precision_node" || name=="selection_precision_element" )
      SMESH::UpdateSelectionProp( this );
    else if (name == QString("scalar_bar_vertical_x") || name == QString("scalar_bar_vertical_width")){
      sbX1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_x", sbX1);
      sbW = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_width", sbW);
      if(sbX1+sbW > aTol){
	aWarning = "Origin and Size Vertical: X+Width > 1\n";	
	sbX1=0.01;
	sbW=0.05;
	aResourceMgr->setValue("SMESH", "scalar_bar_vertical_x", sbX1);
	aResourceMgr->setValue("SMESH", "scalar_bar_vertical_width", sbW);
      }
    }
    else if(name == QString("scalar_bar_vertical_y") || name == QString("scalar_bar_vertical_height")){
      sbY1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_y", sbY1);
      sbH = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_height",sbH);
      if(sbY1+sbH > aTol){
	aWarning = "Origin and Size Vertical: Y+Height > 1\n";
	sbY1=0.01;
	sbH=0.5;
	aResourceMgr->setValue("SMESH", "scalar_bar_vertical_y", sbY1);
	aResourceMgr->setValue("SMESH", "scalar_bar_vertical_height",sbH);
      }
    }
    else if(name ==  QString("scalar_bar_horizontal_x") || name ==  QString("scalar_bar_horizontal_width")){
      sbX1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_x", sbX1);
      sbW = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_width", sbW);
      if(sbX1+sbW > aTol){
	aWarning = "Origin and Size Horizontal: X+Width > 1\n";
	sbX1=0.2;
	sbW=0.6;
	aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_x", sbX1);
	aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_width", sbW);
      }
    }
    else if(name ==  QString("scalar_bar_horizontal_y") || name ==  QString("scalar_bar_horizontal_height")){
      sbY1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_y", sbY1);
      sbH = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_height",sbH);
      if(sbY1+sbH > aTol){
	aWarning = "Origin and Size Horizontal: Y+Height > 1\n";
	sbY1=0.01;
	sbH=0.12;
	aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_y", sbY1);
	aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_height",sbH);
      }
    }
    
    if(aWarning.size() != 0){
      aWarning += "The default values are applied instead.";
      SUIT_MessageBox::warn1(SMESHGUI::desktop(),
			     QObject::tr("SMESH_ERR_SCALARBAR_PARAMS"),
			     QObject::tr(aWarning.c_str()),
			     QObject::tr("SMESH_BUT_OK"));
    }
  }
}

//================================================================================
/*!
 * \brief Update something in accordance with update flags
  * \param theFlags - update flags
*
* Update viewer or/and object browser etc. in accordance with update flags ( see
* LightApp_UpdateFlags enumeration ).
*/
//================================================================================
void SMESHGUI::update( const int flags )
{
  if ( flags & UF_Viewer | flags & UF_Forced )
    SMESH::UpdateView();
  else
    SalomeApp_Module::update( flags );
}

//================================================================================
/*!
 * \brief Set default selection mode
*
* SLOT called when operation commited. Sets default selection mode
*/
//================================================================================
void SMESHGUI::onOperationCommited( SUIT_Operation* )
{
  SVTK_ViewWindow* vtkWnd =
    dynamic_cast<SVTK_ViewWindow*>( application()->desktop()->activeWindow() );
  if ( vtkWnd )
    vtkWnd->SetSelectionMode( ActorSelection );
}

//================================================================================
/*!
 * \brief Set default selection mode
*
* SLOT called when operation aborted. Sets default selection mode
*/
//================================================================================
void SMESHGUI::onOperationAborted( SUIT_Operation* )
{
  SVTK_ViewWindow* vtkWnd =
    dynamic_cast<SVTK_ViewWindow*>( application()->desktop()->activeWindow() );
  if ( vtkWnd )
    vtkWnd->SetSelectionMode( ActorSelection );
}

//================================================================================
/*!
 * \brief Creates operation with given identifier
  * \param id - identifier of operation to be started
  * \return Pointer on created operation or NULL if operation is not created
*
* Virtual method redefined from the base class creates operation with given id.
* It is called called automatically from startOperation method of base class.
*/
//================================================================================
LightApp_Operation* SMESHGUI::createOperation( const int id ) const
{
  LightApp_Operation* op = 0;
  // to do : create operation here
  switch( id )
  {
    case 702: // Create mesh
      op = new SMESHGUI_MeshOp( true, true );
    break;
    case 703: // Create sub-mesh
      op = new SMESHGUI_MeshOp( true, false );
    break;
    case 704: // Edit mesh/sub-mesh
      op = new SMESHGUI_MeshOp( false );
    break;
    default:
    break;
  }

  if( !op )
    op = SalomeApp_Module::createOperation( id );
  return op;
}

//================================================================================
/*!
 * \brief Stops current operations and starts a given one
  * \param id - The id of the operation to start
 */
//================================================================================

void SMESHGUI::switchToOperation(int id)
{
  if ( _PTR(Study) aStudy = SMESH::GetActiveStudyDocument() )
    activeStudy()->abortAllOperations();
  startOperation( id );
}

LightApp_Displayer* SMESHGUI::displayer()
{
  if( !myDisplayer )
    myDisplayer = new SMESHGUI_Displayer( getApp() );
  return myDisplayer;
}
