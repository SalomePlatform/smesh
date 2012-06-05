// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
//  File   : SMESHGUI.cxx
//  Author : Nicolas REJNERI, Open CASCADE S.A.S.

#include <Standard_math.hxx>  // E.A. must be included before Python.h to fix compilation on windows
#ifdef HAVE_FINITE
#undef HAVE_FINITE            // VSR: avoid compilation warning on Linux : "HAVE_FINITE" redefined
#endif
#include "Python.h"
//  SMESH includes
#include "SMESHGUI.h"
#include "SMESHGUI_AddMeshElementDlg.h"
#include "SMESHGUI_AddQuadraticElementDlg.h"
#include "SMESHGUI_BuildCompoundDlg.h"
#include "SMESHGUI_ClippingDlg.h"
#include "SMESHGUI_ComputeDlg.h"
#include "SMESHGUI_ConvToQuadOp.h"
#include "SMESHGUI_CreatePolyhedralVolumeDlg.h"
#include "SMESHGUI_DeleteGroupDlg.h"
#include "SMESHGUI_Displayer.h"
#include "SMESHGUI_MergeDlg.h"
#include "SMESHGUI_ExtrusionAlongPathDlg.h"
#include "SMESHGUI_ExtrusionDlg.h"
#include "SMESHGUI_FileInfoDlg.h"
#include "SMESHGUI_FileValidator.h"
#include "SMESHGUI_FilterDlg.h"
#include "SMESHGUI_FilterLibraryDlg.h"
#include "SMESHGUI_FindElemByPointDlg.h"
#include "SMESHGUI_GroupDlg.h"
#include "SMESHGUI_GroupOnShapeDlg.h"
#include "SMESHGUI_GroupOpDlg.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_Make2DFrom3DOp.h"
#include "SMESHGUI_MakeNodeAtPointDlg.h"
#include "SMESHGUI_Measurements.h"
#include "SMESHGUI_MeshInfo.h"
#include "SMESHGUI_MeshOp.h"
#include "SMESHGUI_MeshOrderOp.h"
#include "SMESHGUI_MeshPatternDlg.h"
#include "SMESHGUI_MultiEditDlg.h"
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI_Preferences_ColorDlg.h"
#include "SMESHGUI_Preferences_ScalarBarDlg.h"
#include "SMESHGUI_RemoveElementsDlg.h"
#include "SMESHGUI_RemoveNodesDlg.h"
#include "SMESHGUI_RenumberingDlg.h"
#include "SMESHGUI_RevolutionDlg.h"
#include "SMESHGUI_RotationDlg.h"
#include "SMESHGUI_Selection.h"
#include "SMESHGUI_SewingDlg.h"
#include "SMESHGUI_SingleEditDlg.h"
#include "SMESHGUI_SmoothingDlg.h"
#include "SMESHGUI_SymmetryDlg.h"
#include "SMESHGUI_TranslationDlg.h"
#include "SMESHGUI_ScaleDlg.h"
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI_DuplicateNodesDlg.h"
#include "SMESHGUI_CopyMeshDlg.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_HypothesesUtils.h"

#include <SMESH_version.h>

#include <SMESH_Client.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ScalarBarActor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_TypeFilter.hxx>
#include "SMESH_ControlsDef.hxx"

// SALOME GUI includes
#include <SalomeApp_Tools.h>
#include <SalomeApp_Study.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_CheckFileDlg.h>
#include <SalomeApp_DataObject.h>

#include <LightApp_DataOwner.h>
#include <LightApp_Preferences.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <LightApp_NameDlg.h>

#include <SVTK_ViewWindow.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewManager.h>

#include <VTKViewer_Algorithm.h>

#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_FileDlg.h>
#include <SUIT_Desktop.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>

#include <QtxPopupMgr.h>
#include <QtxFontEdit.h>

#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>

#ifndef DISABLE_PLOT2DVIEWER
#include <SPlot2d_ViewModel.h>
#include <SPlot2d_Histogram.h>
#endif

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(SMESH_MeshEditor)
#include CORBA_CLIENT_HEADER(SMESH_Measurements)

// Qt includes
// #define       INCLUDE_MENUITEM_DEF // VSR commented ????????
#include <QApplication>
#include <QMenu>
#include <QTextStream>

// BOOST includes
#include <boost/shared_ptr.hpp>

// VTK includes
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkPlane.h>
#include <vtkCallbackCommand.h>
#include <vtkLookupTable.h>

// SALOME KERNEL includes
#include <SALOMEDS_Study.hxx>
#include <SALOMEDSClient_StudyBuilder.hxx>
#include <SALOMEDSClient_SComponent.hxx>
#include <SALOMEDSClient_ClientFactory.hxx>
#include <SALOMEDSClient_IParameters.hxx>

// OCCT includes
#include <Standard_ErrorHandler.hxx>
#include <NCollection_DataMap.hxx>

//To disable automatic genericobj management, the following line should be commented.
//Otherwise, it should be uncommented. Refer to KERNEL_SRC/src/SALOMEDSImpl/SALOMEDSImpl_AttributeIOR.cxx
#define WITHGENERICOBJ

//namespace{
  // Declarations
  //=============================================================
  void ImportMeshesFromFile(SMESH::SMESH_Gen_ptr theComponentMesh,
                            int theCommandID);

  void ExportMeshToFile(int theCommandID);

  void SetDisplayMode(int theCommandID, SMESHGUI_StudyId2MarkerMap& theMarkerMap);

  void SetDisplayEntity(int theCommandID);

  void Control( int theCommandID );


  // Definitions
  //=============================================================
  void ImportMeshesFromFile( SMESH::SMESH_Gen_ptr theComponentMesh,
                             int theCommandID )
  {
    QStringList filter;
    std::string myExtension;

    if ( theCommandID == 113 ) {
      filter.append( QObject::tr( "MED_FILES_FILTER" ) + " (*.med)" );
      filter.append( QObject::tr( "ALL_FILES_FILTER" ) + " (*)" );
    }
    else if ( theCommandID == 112 ) {
      filter.append( QObject::tr( "IDEAS_FILES_FILTER" ) + " (*.unv)" );
    }
    else if ( theCommandID == 111 ) {
      filter.append( QObject::tr( "DAT_FILES_FILTER" ) + " (*.dat)" );
    }
    else if ( theCommandID == 115 ) {
      filter.append( QObject::tr( "STL_ASCII_FILES_FILTER" ) + " (*.stl)" );
    }
    else if ( theCommandID == 116 ) {
      filter.append( QObject::tr( "CGNS_FILES_FILTER" ) + " (*.cgns)" );
    }
    else if ( theCommandID == 117 ) {
      filter.append( QObject::tr( "SAUV files (*.sauv*)" ) );
      filter.append( QObject::tr( "All files (*)" ) );
    }

    QString anInitialPath = "";
    if ( SUIT_FileDlg::getLastVisitedPath().isEmpty() )
      anInitialPath = QDir::currentPath();

    QStringList filenames = SUIT_FileDlg::getOpenFileNames( SMESHGUI::desktop(),
                                                            anInitialPath,
                                                            filter,
                                                            QObject::tr( "SMESH_IMPORT_MESH" ) );
    if ( filenames.count() > 0 ) {
      SUIT_OverrideCursor wc;
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

      QStringList errors;
      QStringList anEntryList;
      bool isEmpty = false;
      for ( QStringList::ConstIterator it = filenames.begin(); it != filenames.end(); ++it ) {
        QString filename = *it;
        SMESH::mesh_array_var aMeshes = new SMESH::mesh_array;
        try {
          switch ( theCommandID ) {
          case 111:
            {
              // DAT format (currently unsupported)
              errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                             arg( QObject::tr( "SMESH_ERR_NOT_SUPPORTED_FORMAT" ) ) );
              break;
            }
          case 112:
            {
              // UNV format
              aMeshes->length( 1 );
              aMeshes[0] = theComponentMesh->CreateMeshesFromUNV( filename.toLatin1().constData() );
              if ( aMeshes[0]->_is_nil() )
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( "SMESH_ERR_UNKNOWN_IMPORT_ERROR" ) ) );
              break;
            }
          case 113:
            {
              // MED format
              SMESH::DriverMED_ReadStatus res;
              aMeshes = theComponentMesh->CreateMeshesFromMED( filename.toLatin1().constData(), res );
              if ( res != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
              }
              break;
            }
          case 115:
            {
              // STL format
              aMeshes->length( 1 );
              aMeshes[0] = theComponentMesh->CreateMeshesFromSTL( filename.toLatin1().constData() );
              if ( aMeshes[0]->_is_nil() ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( "SMESH_ERR_UNKNOWN_IMPORT_ERROR" ) ) );
              }
              break;
            }
          case 116:
            {
              // CGNS format
              SMESH::DriverMED_ReadStatus res;
              aMeshes = theComponentMesh->CreateMeshesFromCGNS( filename.toLatin1().constData(), res );
              if ( res != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
              }
              break;
            }
          case 117:
            {
              // SAUV format
              SMESH::DriverMED_ReadStatus res;
              aMeshes = theComponentMesh->CreateMeshesFromSAUV( filename.toLatin1().constData(), res );
              if ( res != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
              }
              break;
            }
          }
        }
        catch ( const SALOME::SALOME_Exception& S_ex ) {
          errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                         arg( QObject::tr( "SMESH_ERR_UNKNOWN_IMPORT_ERROR" ) ) );
        }

        for ( int i = 0, iEnd = aMeshes->length(); i < iEnd; i++ ) {
          _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshes[i] );
          if ( aMeshSO ) {
            _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
            _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" );
            aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" );
            if ( theCommandID == 112 ) // mesh names aren't taken from the file for UNV import
              SMESH::SetName( aMeshSO, QFileInfo(filename).fileName() );

            anEntryList.append( aMeshSO->GetID().c_str() );

#ifdef WITHGENERICOBJ
            // obj has been published in study. Its refcount has been incremented.
            // It is safe to decrement its refcount
            // so that it will be destroyed when the entry in study will be removed
            aMeshes[i]->UnRegister();
#endif
          }
          else {
            isEmpty = true;
          }
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
    }
  }

  //================================================================================
  /*!
   * \brief Export selected meshes or groups into a file
   */
  //================================================================================

  void ExportMeshToFile( int theCommandID )
  {
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    const bool isMED = ( theCommandID == 122 || theCommandID == 125 );
    const bool isDAT = ( theCommandID == 121 || theCommandID == 124 );
    const bool isUNV = ( theCommandID == 123 || theCommandID == 126 );
    const bool isSTL = ( theCommandID == 140 || theCommandID == 141 );
    const bool isCGNS= ( theCommandID == 142 || theCommandID == 143 );
    const bool isSAUV= ( theCommandID == 144 || theCommandID == 145 );

    // actually, the following condition can't be met (added for insurance)
    if( selected.Extent() == 0 ||
        ( selected.Extent() > 1 && !isMED && !isSTL ))
      return;

    // get mesh object from selection and check duplication of their names
    bool hasDuplicatedMeshNames = false;
    QList< QPair< SMESH::SMESH_IDSource_var, QString > > aMeshList;
    QList< QPair< SMESH::SMESH_IDSource_var, QString > >::iterator aMeshIter;
    SALOME_ListIteratorOfListIO It( selected );
    for( ; It.More(); It.Next() )
    {
      Handle(SALOME_InteractiveObject) anIObject = It.Value();
      SMESH::SMESH_IDSource_var aMeshItem = SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(anIObject);
      if ( aMeshItem->_is_nil() ) {
        SUIT_MessageBox::warning( SMESHGUI::desktop(),
                                  QObject::tr( "SMESH_WRN_WARNING" ),
                                  QObject::tr( "SMESH_BAD_MESH_SELECTION" ));
        return;
      }

      QString aMeshName = anIObject->getName();

      // check for name duplications
      if ( !hasDuplicatedMeshNames )
        for( aMeshIter = aMeshList.begin(); aMeshIter != aMeshList.end(); aMeshIter++ ) {
          if( aMeshName == (*aMeshIter).second ) {
            hasDuplicatedMeshNames = true;
            break;
          }
        }

      aMeshList.append( QPair< SMESH::SMESH_IDSource_var, QString >( aMeshItem, aMeshName ) );
    }

    if( hasDuplicatedMeshNames && isMED ) {
      int aRet = SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                          QObject::tr("SMESH_WRN_WARNING"),
                                          QObject::tr("SMESH_EXPORT_MED_DUPLICATED_MESH_NAMES"),
                                          QObject::tr("SMESH_BUT_YES"),
                                          QObject::tr("SMESH_BUT_NO"), 0, 1);
      if (aRet != 0)
        return;
    }

    aMeshIter = aMeshList.begin();
    SMESH::SMESH_IDSource_var aMeshOrGroup = (*aMeshIter).first;
    SMESH::SMESH_Mesh_var aMesh = aMeshOrGroup->GetMesh();
    QString aMeshName = (*aMeshIter).second;

    if ( isMED || isCGNS || isSAUV )
    {
      // check for equal group names within each mesh
      for( aMeshIter = aMeshList.begin(); aMeshIter != aMeshList.end(); aMeshIter++ ) {
        SMESH::SMESH_Mesh_var aMeshItem = SMESH::SMESH_Mesh::_narrow( (*aMeshIter).first );
        if ( !aMeshItem->_is_nil() && aMeshItem->HasDuplicatedGroupNamesMED()) {
          int aRet = SUIT_MessageBox::warning
            (SMESHGUI::desktop(),
             QObject::tr("SMESH_WRN_WARNING"),
             QObject::tr("SMESH_EXPORT_MED_DUPLICATED_GRP").arg((*aMeshIter).second),
             QObject::tr("SMESH_BUT_YES"),
             QObject::tr("SMESH_BUT_NO"), 0, 1);
          if (aRet != 0)
            return;
        }
      }
    }
    else if ( isUNV )
    {
      // warn the user about presence of not supported elements
      SMESH::long_array_var nbElems = aMeshOrGroup->GetMeshInfo();
      int nbNotSupported = ( nbElems[ SMESH::Entity_Pyramid ] +
                             nbElems[ SMESH::Entity_Quad_Pyramid ] +
                             nbElems[ SMESH::Entity_Hexagonal_Prism ] +
                             nbElems[ SMESH::Entity_Polygon ] +
                             nbElems[ SMESH::Entity_Polyhedra ] );
      if ( nbNotSupported > 0 ) {
        int aRet = SUIT_MessageBox::warning
          (SMESHGUI::desktop(),
           QObject::tr("SMESH_WRN_WARNING"),
           QObject::tr("SMESH_EXPORT_UNV").arg(aMeshName),
           QObject::tr("SMESH_BUT_YES"),
           QObject::tr("SMESH_BUT_NO"), 0, 1);
        if (aRet != 0)
          return;
      }
    }

    // Get parameters of export operation

    QString aFilename;
    SMESH::MED_VERSION aFormat;
    // Init the parameters with the default values
    bool aIsASCII_STL = true;
    bool toCreateGroups = false;
    SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
    if ( resMgr )
      toCreateGroups = resMgr->booleanValue( "SMESH", "auto_groups", false );
    bool toOverwrite = true;

    QString aFilter, aTitle = QObject::tr("SMESH_EXPORT_MESH");
    QString anInitialPath = "";
    if ( SUIT_FileDlg::getLastVisitedPath().isEmpty() )
      anInitialPath = QDir::currentPath();

    if ( isUNV || isDAT )
    {
      if ( isUNV )
        aFilter = QObject::tr( "IDEAS_FILES_FILTER" ) + " (*.unv)";
      else
        aFilter = QObject::tr( "DAT_FILES_FILTER" ) + " (*.dat)";
      if ( anInitialPath.isEmpty() ) anInitialPath = SUIT_FileDlg::getLastVisitedPath();
      aFilename = SUIT_FileDlg::getFileName(SMESHGUI::desktop(),
                                            anInitialPath + QString("/") + aMeshName,
                                            aFilter, aTitle, false);
    }
    else if ( isCGNS )// Export to CGNS
    {
      SUIT_FileDlg* fd = new SUIT_FileDlg( SMESHGUI::desktop(), false, true, true );
      fd->setWindowTitle( aTitle );
      fd->setNameFilter( QObject::tr( "CGNS_FILES_FILTER" ) + " (*.cgns)" );
      if ( !anInitialPath.isEmpty() )
        fd->setDirectory( anInitialPath );
      fd->selectFile(aMeshName);
      SMESHGUI_FileValidator* fv = new SMESHGUI_FileValidator( fd );
      fd->setValidator( fv );

      if ( fd->exec() )
        aFilename = fd->selectedFile();
      toOverwrite = fv->isOverwrite();

      delete fd;
    }
    else if ( isSTL ) // Export to STL
    {
      QMap<QString, int> aFilterMap;
      aFilterMap.insert( QObject::tr( "STL_ASCII_FILES_FILTER" ) + " (*.stl)", 1 );
      aFilterMap.insert( QObject::tr( "STL_BIN_FILES_FILTER" )   + " (*.stl)", 0 );

      QStringList filters;
      QMap<QString, int>::const_iterator it = aFilterMap.begin();
      for ( ; it != aFilterMap.end(); ++it )
        filters.push_back( it.key() );

      SUIT_FileDlg* fd = new SUIT_FileDlg( SMESHGUI::desktop(), false, true, true );
      fd->setWindowTitle( aTitle );
      fd->setNameFilters( filters );
      fd->selectNameFilter( QObject::tr( "STL_ASCII_FILES_FILTER" ) + " (*.stl)" );
      if ( !anInitialPath.isEmpty() )
        fd->setDirectory( anInitialPath );
      fd->selectFile(aMeshName);
      bool is_ok = false;
      while (!is_ok) {
        if ( fd->exec() )
          aFilename = fd->selectedFile();
        aIsASCII_STL = (aFilterMap[fd->selectedNameFilter()]) == 1 ? true: false;
        is_ok = true;
      }
      delete fd;
    }
    else if ( isMED || isSAUV ) // Export to MED or SAUV
    {
      QMap<QString, SMESH::MED_VERSION> aFilterMap;
      //QString v21 (aMesh->GetVersionString(SMESH::MED_V2_1, 2));
      if ( isMED ) {
        QString v22 (aMesh->GetVersionString(SMESH::MED_V2_2, 2));
        //aFilterMap.insert( QObject::tr( "MED_VX_FILES_FILTER" ).arg( v21 ) + " (*.med)", SMESH::MED_V2_1 );
        aFilterMap.insert( QObject::tr( "MED_VX_FILES_FILTER" ).arg( v22 ) + " (*.med)", SMESH::MED_V2_2 );
      }
      else { // isSAUV
        aFilterMap.insert("All files (*)", SMESH::MED_V2_1 );
        aFilterMap.insert("SAUV files (*.sauv)", SMESH::MED_V2_2 );
        aFilterMap.insert("SAUV files (*.sauve)", SMESH::MED_V2_1 );
      }

      QStringList filters;
      QString aDefaultFilter;
      QMap<QString, SMESH::MED_VERSION>::const_iterator it = aFilterMap.begin();
      for ( ; it != aFilterMap.end(); ++it ) {
        filters.push_back( it.key() );
        if (it.value() == SMESH::MED_V2_2)
          aDefaultFilter = it.key();
      }

      SalomeApp_CheckFileDlg* fd = new SalomeApp_CheckFileDlg
        ( SMESHGUI::desktop(), false, QObject::tr("SMESH_AUTO_GROUPS"), true, true );
      fd->setWindowTitle( aTitle );
      fd->setNameFilters( filters );
      fd->selectNameFilter(aDefaultFilter);
      fd->SetChecked(toCreateGroups);
      if ( !anInitialPath.isEmpty() )
        fd->setDirectory( anInitialPath );
      fd->selectFile(aMeshName);

      SMESHGUI_FileValidator* fv = new SMESHGUI_FileValidator( fd );
      fd->setValidator( fv );

      bool is_ok = false;
      while (!is_ok) {
        if ( fd->exec() )
          aFilename = fd->selectedFile();
        else {
          aFilename = QString::null;
          break;
        }
        aFormat = aFilterMap[fd->selectedNameFilter()];
        toOverwrite = fv->isOverwrite();
        is_ok = true;
        if ( !aFilename.isEmpty() ) {
          // med-2.1 does not support poly elements
          if ( aFormat==SMESH::MED_V2_1 )
            for( aMeshIter = aMeshList.begin(); aMeshIter != aMeshList.end(); aMeshIter++ ) {
              SMESH::SMESH_IDSource_var aMeshItem = (*aMeshIter).first;
              SMESH::long_array_var nbElems = aMeshItem->GetMeshInfo();
              if ( nbElems[ SMESH::Entity_Polygon   ] + nbElems[ SMESH::Entity_Quad_Polygon   ] +
                   nbElems[ SMESH::Entity_Polyhedra ] + nbElems[ SMESH::Entity_Quad_Polyhedra ])
              {
                int aRet = SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                                    QObject::tr("SMESH_WRN_WARNING"),
                                                    QObject::tr("SMESH_EXPORT_MED_V2_1").arg((*aMeshIter).second),
                                                    QObject::tr("SMESH_BUT_YES"),
                                                    QObject::tr("SMESH_BUT_NO"), 0, 1);
                if (aRet != 0) {
                  is_ok = false;
                  break;
                }
              }
            }
          if( !toOverwrite ) {
            // can't append to an existing using other format
            SMESH::MED_VERSION aVersion = SMESH::MED_V2_1;
            bool isVersionOk = SMESHGUI::GetSMESHGen()->GetMEDVersion( aFilename.toLatin1().constData(), aVersion );
            if( !isVersionOk || aVersion != aFormat ) {
              int aRet = SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                                  QObject::tr("SMESH_WRN_WARNING"),
                                                  QObject::tr("SMESH_EXPORT_MED_VERSION_COLLISION").arg(aFilename),
                                                  QObject::tr("SMESH_BUT_YES"),
                                                  QObject::tr("SMESH_BUT_NO"), 0, 1);
              if (aRet == 0)
                toOverwrite = true;
              else
                is_ok = false;
            }

            QStringList aMeshNamesCollisionList;
            SMESH::string_array_var aMeshNames = SMESHGUI::GetSMESHGen()->GetMeshNames( aFilename.toLatin1().constData() );
            for( int i = 0, n = aMeshNames->length(); i < n; i++ ) {
              QString anExistingMeshName( aMeshNames[ i ] );
              for( aMeshIter = aMeshList.begin(); aMeshIter != aMeshList.end(); aMeshIter++ ) {
                QString anExportMeshName = (*aMeshIter).second;
                if( anExportMeshName == anExistingMeshName ) {
                  aMeshNamesCollisionList.append( anExportMeshName );
                  break;
                }
              }
            }
            if( !aMeshNamesCollisionList.isEmpty() ) {
              QString aMeshNamesCollisionString = aMeshNamesCollisionList.join( ", " );
              int aRet = SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                                  QObject::tr("SMESH_WRN_WARNING"),
                                                  QObject::tr("SMESH_EXPORT_MED_MESH_NAMES_COLLISION").arg(aMeshNamesCollisionString),
                                                  QObject::tr("SMESH_BUT_YES"),
                                                  QObject::tr("SMESH_BUT_NO"),
                                                  QObject::tr("SMESH_BUT_CANCEL"), 0, 2);
              if (aRet == 0)
                toOverwrite = true;
              else if (aRet == 2)
                is_ok = false;
            }
          }
        }
      }
      toCreateGroups = fd->IsChecked();
      delete fd;
    }
    else
    {
      return;
    }

    // Perform export

    if ( !aFilename.isEmpty() ) {
      // Check whether the file already exists and delete it if yes
      QFile aFile( aFilename );
      if ( aFile.exists() && toOverwrite )
        aFile.remove();
      SUIT_OverrideCursor wc;

      try {
        // Renumbering is not needed since SMDS redesign in V6.2.0 (Nov 2010)
//         bool Renumber = false;
//         // PAL 14172  : Check of we have to renumber or not from the preferences before export
//         if (resMgr)
//           Renumber= resMgr->booleanValue("SMESH","renumbering");
//         if (Renumber){
//           SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
//           aMeshEditor->RenumberNodes();
//           aMeshEditor->RenumberElements();
//           if ( SMESHGUI::automaticUpdate() )
//             SMESH::UpdateView();
//         }
        if ( isMED )
        {
          aMeshIter = aMeshList.begin();
          for( int aMeshIndex = 0; aMeshIter != aMeshList.end(); aMeshIter++, aMeshIndex++ )
          {
            SMESH::SMESH_IDSource_var aMeshOrGroup = (*aMeshIter).first;
            SMESH::SMESH_Mesh_var        aMeshItem = aMeshOrGroup->GetMesh();
            if ( aMeshOrGroup->_is_equivalent( aMeshItem ))
              aMeshItem->ExportToMEDX( aFilename.toLatin1().data(), toCreateGroups,
                                       aFormat, toOverwrite && aMeshIndex == 0 );
            else
              aMeshItem->ExportPartToMED( aMeshOrGroup, aFilename.toLatin1().data(), toCreateGroups,
                                          aFormat, toOverwrite && aMeshIndex == 0 );
          }
        }
        else if ( isSAUV )
        {
          for( aMeshIter = aMeshList.begin(); aMeshIter != aMeshList.end(); aMeshIter++ )
          {
            SMESH::SMESH_Mesh_var aMeshItem = SMESH::SMESH_Mesh::_narrow( (*aMeshIter).first );
            if( !aMeshItem->_is_nil() )
              aMeshItem->ExportSAUV( aFilename.toLatin1().data(), toCreateGroups );
          }
        }
        else if ( isDAT )
        {
          if ( aMeshOrGroup->_is_equivalent( aMesh ))
            aMesh->ExportDAT( aFilename.toLatin1().data() );
          else
            aMesh->ExportPartToDAT( aMeshOrGroup, aFilename.toLatin1().data() );
        }
        else if ( isUNV )
        {
          if ( aMeshOrGroup->_is_equivalent( aMesh ))
            aMesh->ExportUNV( aFilename.toLatin1().data() );
          else
            aMesh->ExportPartToUNV( aMeshOrGroup, aFilename.toLatin1().data() );
        }
        else if ( isSTL )
        {
          if ( aMeshOrGroup->_is_equivalent( aMesh ))
            aMesh->ExportSTL( aFilename.toLatin1().data(), aIsASCII_STL );
          else
            aMesh->ExportPartToSTL( aMeshOrGroup, aFilename.toLatin1().data(), aIsASCII_STL );
        }
        else if ( isCGNS )
        {
          aMeshIter = aMeshList.begin();
          for( int aMeshIndex = 0; aMeshIter != aMeshList.end(); aMeshIter++, aMeshIndex++ )
          {
            SMESH::SMESH_IDSource_var aMeshOrGroup = (*aMeshIter).first;
            SMESH::SMESH_Mesh_var        aMeshItem = aMeshOrGroup->GetMesh();
            aMeshItem->ExportCGNS( aMeshOrGroup,
                                   aFilename.toLatin1().data(),
                                   toOverwrite && aMeshIndex == 0 );
          }
        }
      }
      catch (const SALOME::SALOME_Exception& S_ex){
        wc.suspend();
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 QObject::tr("SMESH_WRN_WARNING"),
                                 QObject::tr("SMESH_EXPORT_FAILED"));
        wc.resume();
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
      for( ; It.More(); It.Next()){
        Handle(SALOME_InteractiveObject) IObject = It.Value();
        if(IObject->hasEntry()){
          if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
            unsigned int aMode = anActor->GetEntityMode();
            switch(theCommandID){
            case 216:
              InverseEntityMode(aMode,SMESH_Actor::e0DElements);
              break;
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

  void AutoColor(){
    SALOME_ListIO selected;
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
    if( !app )
      return;

    LightApp_SelectionMgr* aSel = app->selectionMgr();
    SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
    if( !aSel || !appStudy )
      return;

    aSel->selectedObjects( selected );
    if( selected.IsEmpty() )
      return;

    Handle(SALOME_InteractiveObject) anIObject = selected.First();

    _PTR(Study) aStudy = appStudy->studyDS();
    _PTR(SObject) aMainSObject( aStudy->FindObjectID( anIObject->getEntry() ) );
    SMESH::SMESH_Mesh_var aMainObject = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
    if( aMainObject->_is_nil() )
      return;

    aMainObject->SetAutoColor( true ); // mesh groups are re-colored here

    SMESH::ListOfGroups aListOfGroups = *aMainObject->GetGroups();
    for( int i = 0, n = aListOfGroups.length(); i < n; i++ )
    {
      SMESH::SMESH_GroupBase_var aGroupObject = aListOfGroups[i];
      SALOMEDS::Color aColor = aGroupObject->GetColor();
      _PTR(SObject) aGroupSObject = SMESH::FindSObject(aGroupObject);
      if (aGroupSObject) {
        if(SMESH_Actor *anActor = SMESH::FindActorByEntry(aGroupSObject->GetID().c_str())) {
          if( aGroupObject->GetType() == SMESH::NODE )
            anActor->SetNodeColor( aColor.R, aColor.G, aColor.B );
          else if( aGroupObject->GetType() == SMESH::EDGE )
            anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B );
          else if( aGroupObject->GetType() == SMESH::ELEM0D )
            anActor->Set0DColor( aColor.R, aColor.G, aColor.B );
          else {
            QColor c;
            int delta;
            SMESH::GetColor("SMESH", "fill_color", c, delta, "0,170,255|-100");
            anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B, delta );
          }
        }
      }
    }

    SMESH::RepaintCurrentView();
  }

  QString functorToString( SMESH::Controls::FunctorPtr f )
  {
    QString type = QObject::tr( "UNKNOWN_CONTROL" );
    if ( dynamic_cast< SMESH::Controls::Volume* >( f.get() ) )
      type = QObject::tr( "VOLUME_3D_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::MaxElementLength2D* >( f.get() ) )
      type = QObject::tr( "MAX_ELEMENT_LENGTH_2D" );
    else if ( dynamic_cast< SMESH::Controls::MaxElementLength3D* >( f.get() ) )
      type = QObject::tr( "MAX_ELEMENT_LENGTH_3D" );
    else if ( dynamic_cast< SMESH::Controls::MinimumAngle* >( f.get() ) )
      type = QObject::tr( "MINIMUMANGLE_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::AspectRatio* >( f.get() ) )
      type = QObject::tr( "ASPECTRATIO_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::AspectRatio3D* >( f.get() ) )
      type = QObject::tr( "ASPECTRATIO_3D_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::Warping* >( f.get() ) )
      type = QObject::tr( "WARP_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::Taper* >( f.get() ) )
      type = QObject::tr( "TAPER_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::Skew* >( f.get() ) )
      type = QObject::tr( "SKEW_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::Area* >( f.get() ) )
      type = QObject::tr( "AREA_ELEMENTS" );
    else if ( dynamic_cast< SMESH::Controls::Length* >( f.get() ) )
      type = QObject::tr( "LENGTH_EDGES" );
    else if ( dynamic_cast< SMESH::Controls::Length2D* >( f.get() ) )
      type = QObject::tr( "LENGTH2D_EDGES" );
    else if ( dynamic_cast< SMESH::Controls::MultiConnection* >( f.get() ) )
      type = QObject::tr( "MULTI_BORDERS" );
    else if ( dynamic_cast< SMESH::Controls::MultiConnection2D* >( f.get() ) )
      type = QObject::tr( "MULTI2D_BORDERS" );
    else if ( dynamic_cast< SMESH::Controls::FreeNodes* >( f.get() ) )
      type = QObject::tr( "FREE_NODES" );
    else if ( dynamic_cast< SMESH::Controls::FreeEdges* >( f.get() ) )
      type = QObject::tr( "FREE_EDGES" );
    else if ( dynamic_cast< SMESH::Controls::FreeBorders* >( f.get() ) )
      type = QObject::tr( "FREE_BORDERS" );
    else if ( dynamic_cast< SMESH::Controls::FreeFaces* >( f.get() ) )
      type = QObject::tr( "FREE_FACES" );
    else if ( dynamic_cast< SMESH::Controls::BareBorderVolume* >( f.get() ) )
      type = QObject::tr( "BARE_BORDER_VOLUME" );
    else if ( dynamic_cast< SMESH::Controls::BareBorderFace* >( f.get() ) )
      type = QObject::tr( "BARE_BORDER_FACE" );
    else if ( dynamic_cast< SMESH::Controls::OverConstrainedVolume* >( f.get() ) )
      type = QObject::tr( "OVER_CONSTRAINED_VOLUME" );
    else if ( dynamic_cast< SMESH::Controls::OverConstrainedFace* >( f.get() ) )
      type = QObject::tr( "OVER_CONSTRAINED_FACE" );
    else if ( dynamic_cast< SMESH::Controls::CoincidentNodes* >( f.get() ) )
      type = QObject::tr( "EQUAL_NODE" );
    else if ( dynamic_cast< SMESH::Controls::CoincidentElements1D* >( f.get() ) )
      type = QObject::tr( "EQUAL_EDGE" );
    else if ( dynamic_cast< SMESH::Controls::CoincidentElements2D* >( f.get() ) )
      type = QObject::tr( "EQUAL_FACE" );
    else if ( dynamic_cast< SMESH::Controls::CoincidentElements3D* >( f.get() ) )
      type = QObject::tr( "EQUAL_VOLUME" );
    return type;
  }

  void SaveDistribution()
  {
    LightApp_SelectionMgr* aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if ( aSel )
      aSel->selectedObjects( selected );

    if ( selected.Extent() == 1 ) {
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      if ( anIO->hasEntry() ) {
        SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() );
        if ( anActor && anActor->GetScalarBarActor() && anActor->GetControlMode() != SMESH_Actor::eNone ) {
          SMESH_ScalarBarActor* aScalarBarActor = anActor->GetScalarBarActor();
          SMESH::Controls::FunctorPtr aFunctor = anActor->GetFunctor();
          if ( aScalarBarActor && aFunctor ) {
            SMESH::Controls::NumericalFunctor* aNumFun = dynamic_cast<SMESH::Controls::NumericalFunctor*>( aFunctor.get() );
            if ( aNumFun ) {
              std::vector<int> elements;
              SMESH::SMESH_Mesh_var mesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIO);
              if ( mesh->_is_nil() ) {
                SMESH::SMESH_IDSource_var idSource =
                  SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(anIO);
                if ( !idSource->_is_nil() )
                {
                  SMESH::long_array_var ids = idSource->GetIDs();
                  elements.resize( ids->length() );
                  for ( unsigned i = 0; i < elements.size(); ++i )
                    elements[i] = ids[i];
                }
              }
              int nbIntervals = aScalarBarActor->GetMaximumNumberOfColors();
              vtkLookupTable* lookupTable =
                static_cast<vtkLookupTable*>(aScalarBarActor->GetLookupTable());
              double * minmax = lookupTable->GetRange();
              std::vector<int>    nbEvents;
              std::vector<double> funValues;
              aNumFun->GetHistogram( nbIntervals, nbEvents, funValues, elements, minmax );
              QString anInitialPath = "";
              if ( SUIT_FileDlg::getLastVisitedPath().isEmpty() )
                anInitialPath = QDir::currentPath();
              QString aMeshName = anIO->getName();
              QStringList filter;
              filter.append( QObject::tr( "TEXT_FILES_FILTER" ) + " (*.txt)" );
              filter.append( QObject::tr( "ALL_FILES_FILTER" ) + " (*)" );
              QString aFilename = anInitialPath + "/" + aMeshName + "_" + 
                functorToString( aFunctor ).toLower().simplified().replace( QRegExp( " |-" ), "_" ) + ".txt";
              aFilename = SUIT_FileDlg::getFileName( SMESHGUI::desktop(),
                                                     aFilename,
                                                     filter,
                                                     QObject::tr( "SMESH_SAVE_DISTRIBUTION" ),
                                                     false );
              if ( !aFilename.isEmpty() ) {
                QFile f( aFilename );
                if ( f.open( QFile::WriteOnly | QFile::Truncate ) ) {
                  QTextStream out( &f );
                  out << "# Mesh: " << aMeshName << endl;
                  out << "# Control: " << functorToString( aFunctor ) << endl;
                  out << "#" << endl;
                  out.setFieldWidth( 10 );
                  for ( int i = 0; i < qMin( nbEvents.size(), funValues.size()-1 ); i++ )
                    out << funValues[i] << "\t" << funValues[i+1] << "\t" << nbEvents[i] << endl;
                  f.close();
                }
              }
            }
          }
        }
      }
    }
  }

  void ShowDistribution() {
    LightApp_SelectionMgr* aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if ( aSel )
      aSel->selectedObjects( selected );
    
    if ( selected.Extent() == 1 ) {
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      if ( anIO->hasEntry() ) {
        SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() );
        if ( anActor && anActor->GetScalarBarActor() && anActor->GetControlMode() != SMESH_Actor::eNone ) {
          SMESH_ScalarBarActor *aScalarBarActor = anActor->GetScalarBarActor();
          aScalarBarActor->SetDistributionVisibility(!aScalarBarActor->GetDistributionVisibility());
        }
      }
    }
  }

#ifndef DISABLE_PLOT2DVIEWER
 void PlotDistribution() {
   SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
   if( !app )
     return;

   LightApp_SelectionMgr* aSel = SMESHGUI::selectionMgr();
   SALOME_ListIO selected;
   if ( aSel )
     aSel->selectedObjects( selected );
    
   if ( selected.Extent() == 1 ) {
     Handle(SALOME_InteractiveObject) anIO = selected.First();
     if ( anIO->hasEntry() ) {
       //Find Actor by entry before getting Plot2d viewer,
       //because after call getViewManager( Plot2d_Viewer::Type(), true ) active window is Plot2d Viewer
       SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() );

       SUIT_ViewManager* aViewManager = app->getViewManager( Plot2d_Viewer::Type(), true ); // create if necessary

       if( !aViewManager )
         return;
       
       SPlot2d_Viewer* aView = dynamic_cast<SPlot2d_Viewer*>(aViewManager->getViewModel());
       if ( !aView )
         return;

       Plot2d_ViewFrame* aPlot = aView->getActiveViewFrame();
       if ( !aPlot )
         return;

       if ( anActor && anActor->GetControlMode() != SMESH_Actor::eNone ) {
         SPlot2d_Histogram* aHistogram = anActor->UpdatePlot2Histogram();
         QString functorName = functorToString( anActor->GetFunctor());
         QString aHistogramName("%1 : %2");
         aHistogramName = aHistogramName.arg(anIO->getName()).arg(functorName);
         aHistogram->setName(aHistogramName);
         aHistogram->setHorTitle(functorName);
         aHistogram->setVerTitle(QObject::tr("DISTRIBUTION_NB_ENT"));
         aPlot->displayObject(aHistogram, true);
       }
     }
   }
 }
#endif //DISABLE_PLOT2DVIEWER

  void DisableAutoColor(){
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    if(selected.Extent()){
      Handle(SALOME_InteractiveObject) anIObject = selected.First();
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
      if ( !aMesh->_is_nil() ) {
        aMesh->SetAutoColor( false );
      }
    }
  }

  void SetDisplayMode(int theCommandID, SMESHGUI_StudyId2MarkerMap& theMarkerMap){
    SALOME_ListIO selected;
    SalomeApp_Application* app = dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
    if( !app )
      return;

    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
    if( !aSel || !appStudy )
      return;

    if( theCommandID == 1134 ) { // Clipping dialog can be activated without selection
      if( SMESHGUI* aModule = SMESHGUI::GetSMESHGUI() ) {
        aModule->EmitSignalDeactivateDialog();
        if( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( aModule ) )
          (new SMESHGUI_ClippingDlg( aModule, aViewWindow ))->show();
      }
      return;
    }

    _PTR(Study) aStudy = appStudy->studyDS();

    aSel->selectedObjects( selected );

    if(selected.Extent() >= 1){
      switch(theCommandID){
      case 1133:{
        SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
        (new SMESHGUI_TransparencyDlg( SMESHGUI::GetSMESHGUI() ))->show();
        return;
      }
      case 1132:{
        QColor c, e, b, n, c0D, o, outl, selection, preselection;
        int delta;
        int size0D = 0;
        int Edgewidth = 0;
        vtkFloatingPointType Shrink = 0.0;
        vtkFloatingPointType faces_orientation_scale = 0.0;
        bool faces_orientation_3dvectors = false;

        VTK::MarkerType aMarkerTypeCurrent = VTK::MT_NONE;
        VTK::MarkerScale aMarkerScaleCurrent = VTK::MS_NONE;
        int aMarkerTextureCurrent = 0;

        SALOME_ListIteratorOfListIO It( selected );
        for( ; It.More(); It.Next()){
          Handle(SALOME_InteractiveObject) IObject = It.Value();
          if(IObject->hasEntry()){
            if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
              vtkFloatingPointType color[3];
              anActor->GetSufaceColor(color[0], color[1], color[2],delta);
              int c0 = int (color[0] * 255);
              int c1 = int (color[1] * 255);
              int c2 = int (color[2] * 255);
              c.setRgb(c0, c1, c2);

              vtkFloatingPointType edgecolor[3];
              anActor->GetEdgeColor(edgecolor[0], edgecolor[1], edgecolor[2]);
              c0 = int (edgecolor[0] * 255);
              c1 = int (edgecolor[1] * 255);
              c2 = int (edgecolor[2] * 255);
              e.setRgb(c0, c1, c2);

              vtkFloatingPointType nodecolor[3];
              anActor->GetNodeColor(nodecolor[0], nodecolor[1], nodecolor[2]);
              c0 = int (nodecolor[0] * 255);
              c1 = int (nodecolor[1] * 255);
              c2 = int (nodecolor[2] * 255);
              n.setRgb(c0, c1, c2);

              vtkFloatingPointType color0D[3];
              anActor->Get0DColor(color0D[0], color0D[1], color0D[2]);
              c0 = int (color0D[0] * 255);
              c1 = int (color0D[1] * 255);
              c2 = int (color0D[2] * 255);
              c0D.setRgb(c0, c1, c2);

              vtkFloatingPointType outlineColor[3];
              anActor->GetOutlineColor(outlineColor[0], outlineColor[1], outlineColor[2]);
              c0 = int (outlineColor[0] * 255);
              c1 = int (outlineColor[1] * 255);
              c2 = int (outlineColor[2] * 255);
              outl.setRgb(c0, c1, c2);

              vtkFloatingPointType hColor[3];
              anActor->GetHighlightColor(hColor[0], hColor[1], hColor[2]);
              c0 = int (hColor[0] * 255);
              c1 = int (hColor[1] * 255);
              c2 = int (hColor[2] * 255);
              selection.setRgb(c0, c1, c2);

              vtkFloatingPointType phColor[3];
              anActor->GetPreHighlightColor(phColor[0], phColor[1], phColor[2]);
              c0 = int (phColor[0] * 255);
              c1 = int (phColor[1] * 255);
              c2 = int (phColor[2] * 255);
              preselection.setRgb(c0, c1, c2);

              size0D = (int)anActor->Get0DSize();
              if(size0D == 0)
                size0D = 1;
              Edgewidth = (int)anActor->GetLineWidth();
              if(Edgewidth == 0)
                Edgewidth = 1;
              Shrink = anActor->GetShrinkFactor();

              vtkFloatingPointType faces_orientation_color[3];
              anActor->GetFacesOrientationColor(faces_orientation_color);
              c0 = int (faces_orientation_color[0] * 255);
              c1 = int (faces_orientation_color[1] * 255);
              c2 = int (faces_orientation_color[2] * 255);
              o.setRgb(c0, c1, c2);

              faces_orientation_scale = anActor->GetFacesOrientationScale();
              faces_orientation_3dvectors = anActor->GetFacesOrientation3DVectors();

              aMarkerTypeCurrent = anActor->GetMarkerType();
              aMarkerScaleCurrent = anActor->GetMarkerScale();
              aMarkerTextureCurrent = anActor->GetMarkerTexture();

              // even if there are multiple objects in the selection,
              // we need only the first one to get values for the dialog
              break;
            }
          }
        }

        SMESHGUI_Preferences_ColorDlg *aDlg =
          new SMESHGUI_Preferences_ColorDlg( SMESHGUI::GetSMESHGUI() );
        aDlg->SetColor(1, c);
        aDlg->SetColor(2, e);
        aDlg->SetColor(3, n);
        aDlg->SetColor(4, outl);
        aDlg->SetDeltaBrightness(delta);
        aDlg->SetColor(5, c0D);
        aDlg->SetColor(6, o);
        aDlg->SetIntValue(1, Edgewidth);
        aDlg->SetIntValue(2, int(Shrink*100.));
        aDlg->SetIntValue(3, size0D);
        aDlg->SetDoubleValue(1, faces_orientation_scale);
        aDlg->SetBooleanValue(1, faces_orientation_3dvectors);
        aDlg->SetColor(7, selection);
        aDlg->SetColor(8, preselection);
 
        aDlg->setCustomMarkerMap( theMarkerMap[ aStudy->StudyId() ] );

        if( aMarkerTypeCurrent != VTK::MT_USER )
          aDlg->setStandardMarker( aMarkerTypeCurrent, aMarkerScaleCurrent );
        else
          aDlg->setCustomMarker( aMarkerTextureCurrent );

        if(aDlg->exec()){
          QColor color = aDlg->GetColor(1);
          QColor edgecolor = aDlg->GetColor(2);
          QColor nodecolor = aDlg->GetColor(3);
          QColor outlinecolor = aDlg->GetColor(4);
          QColor color0D = aDlg->GetColor(5);
          QColor faces_orientation_color = aDlg->GetColor(6);
          QColor selectioncolor = aDlg->GetColor(7);
          QColor preSelectioncolor = aDlg->GetColor(8);
          int delta = aDlg->GetDeltaBrightness();

          /* Point marker */
          theMarkerMap[ aStudy->StudyId() ] = aDlg->getCustomMarkerMap();

          SALOME_ListIteratorOfListIO It( selected );
          for( ; It.More(); It.Next()){
            Handle(SALOME_InteractiveObject) IObject = It.Value();
            if(IObject->hasEntry()){
              if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
                /* actor color and backface color */
                anActor->SetSufaceColor(vtkFloatingPointType (color.red()) / 255.,
                                        vtkFloatingPointType (color.green()) / 255.,
                                        vtkFloatingPointType (color.blue()) / 255.,
                                        delta);
                /* edge color */
                anActor->SetEdgeColor(vtkFloatingPointType (edgecolor.red()) / 255.,
                                      vtkFloatingPointType (edgecolor.green()) / 255.,
                                      vtkFloatingPointType (edgecolor.blue()) / 255.);
                /* edge outline */
                anActor->SetOutlineColor(vtkFloatingPointType (outlinecolor.red()) / 255.,
                                         vtkFloatingPointType (outlinecolor.green()) / 255.,
                                         vtkFloatingPointType (outlinecolor.blue()) / 255.);

                /* selection */
                anActor->SetHighlightColor(vtkFloatingPointType (selectioncolor.red()) / 255.,
                                           vtkFloatingPointType (selectioncolor.green()) / 255.,
                                           vtkFloatingPointType (selectioncolor.blue()) / 255.);
                /* pre-selection */
                anActor->SetPreHighlightColor(vtkFloatingPointType (preSelectioncolor.red()) / 255.,
                                              vtkFloatingPointType (preSelectioncolor.green()) / 255.,
                                              vtkFloatingPointType (preSelectioncolor.blue()) / 255.);
                

                /* Shrink factor and size edges */
                anActor->SetShrinkFactor(aDlg->GetIntValue(2) / 100.);
                anActor->SetLineWidth(aDlg->GetIntValue(1));

                /* Nodes color and size */
                anActor->SetNodeColor(vtkFloatingPointType (nodecolor.red()) / 255.,
                                      vtkFloatingPointType (nodecolor.green()) / 255.,
                                      vtkFloatingPointType (nodecolor.blue()) / 255.);

                /* 0D elements */
                anActor->Set0DColor(vtkFloatingPointType (color0D.red()) / 255.,
                                    vtkFloatingPointType (color0D.green()) / 255.,
                                    vtkFloatingPointType (color0D.blue()) / 255.);
                anActor->Set0DSize(aDlg->GetIntValue(3));

                /* Faces orientation */
                vtkFloatingPointType c[3] = {vtkFloatingPointType(faces_orientation_color.redF()),
                                             vtkFloatingPointType(faces_orientation_color.greenF()),
                                             vtkFloatingPointType(faces_orientation_color.blueF())};
                anActor->SetFacesOrientationColor(c);
                anActor->SetFacesOrientationScale(aDlg->GetDoubleValue(1));
                anActor->SetFacesOrientation3DVectors(aDlg->GetBooleanValue(1));

                VTK::MarkerType aMarkerTypeNew = aDlg->getMarkerType();
                VTK::MarkerScale aMarkerScaleNew = aDlg->getStandardMarkerScale();
                int aMarkerTextureNew = aDlg->getCustomMarkerID();
                if( aMarkerTypeNew != VTK::MT_USER )
                  anActor->SetMarkerStd( aMarkerTypeNew, aMarkerScaleNew );
                else {
                  const VTK::MarkerMap& aMarkerMap = theMarkerMap[ aStudy->StudyId() ];
                  VTK::MarkerMap::const_iterator anIter = aMarkerMap.find( aMarkerTextureNew );
                  if( anIter != aMarkerMap.end() )
                    anActor->SetMarkerTexture( aMarkerTextureNew, anIter->second.second );
                }

                SMESH::SMESH_GroupBase_var aGroupObject = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IObject);
                if( !aGroupObject->_is_nil() )
                {
                  SMESH::ElementType anElementType = aGroupObject->GetType();
                  QColor aColor;
                  switch( anElementType )
                  {
                    case SMESH::NODE: aColor = nodecolor; break;
                    case SMESH::EDGE: aColor = edgecolor; break;
                    default: aColor = color; break;
                  }

                  SALOMEDS::Color aGroupColor;
                  aGroupColor.R = (float)aColor.red() / 255.0;
                  aGroupColor.G = (float)aColor.green() / 255.0;
                  aGroupColor.B = (float)aColor.blue() / 255.0;
                  aGroupObject->SetColor( aGroupColor );
                }
              }
            }
          }
          SMESH::RepaintCurrentView();
        }
        delete aDlg;
        return;
      }
      }
      SALOME_ListIteratorOfListIO It( selected );
      for( ; It.More(); It.Next()){
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
            case 231:
              if(anActor->GetQuadratic2DRepresentation() != SMESH_Actor::eLines)
                anActor->SetQuadratic2DRepresentation(SMESH_Actor::eLines);
              break;
            case 232:
              if(anActor->GetQuadratic2DRepresentation() != SMESH_Actor::eArcs)
                anActor->SetQuadratic2DRepresentation(SMESH_Actor::eArcs);
              break;
            }
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
        SMESH_Actor::eControl aControl = SMESH_Actor::eNone;
        if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIO->getEntry())) {
          switch ( theCommandID ){
          case 6001:
            aControl = SMESH_Actor::eLength;
            break;
          case 6018:
            aControl = SMESH_Actor::eLength2D;
            break;
          case 6002:
            aControl = SMESH_Actor::eFreeEdges;
            break;
          case 6003:
            aControl = SMESH_Actor::eFreeBorders;
            break;
          case 6004:
            aControl = SMESH_Actor::eMultiConnection;
            break;
          case 6005:
            aControl = SMESH_Actor::eFreeNodes;
            break;
          case 6019:
            aControl = SMESH_Actor::eMultiConnection2D;
            break;
          case 6011:
            aControl = SMESH_Actor::eArea;
            break;
          case 6012:
            aControl = SMESH_Actor::eTaper;
            break;
          case 6013:
            aControl = SMESH_Actor::eAspectRatio;
            break;
          case 6017:
            aControl = SMESH_Actor::eAspectRatio3D;
            break;
          case 6014:
            aControl = SMESH_Actor::eMinimumAngle;
            break;
          case 6015:
            aControl = SMESH_Actor::eWarping;
            break;
          case 6016:
            aControl = SMESH_Actor::eSkew;
            break;
          case 6009:
            aControl = SMESH_Actor::eVolume3D;
            break;
          case 6021:
            aControl = SMESH_Actor::eFreeFaces;
            break;
          case 6022:
            aControl = SMESH_Actor::eMaxElementLength2D;
            break;
          case 6023:
            aControl = SMESH_Actor::eMaxElementLength3D;
            break;
          case 6024:
            aControl = SMESH_Actor::eBareBorderVolume;
            break;
          case 6025:
            aControl = SMESH_Actor::eBareBorderFace;
            break;
          case 6026:
            aControl = SMESH_Actor::eOverConstrainedVolume;
            break;
          case 6027:
            aControl = SMESH_Actor::eOverConstrainedFace;
            break;
          case 6028:
            aControl = SMESH_Actor::eCoincidentNodes;
            break;
          case 6029:
            aControl = SMESH_Actor::eCoincidentElems1D;
            break;
          case 6030:
            aControl = SMESH_Actor:: eCoincidentElems2D;
            break;
          case 6031:
            aControl = SMESH_Actor::eCoincidentElems3D;
            break;
          }
            
          anActor->SetControlMode(aControl);
          anActor->GetScalarBarActor()->SetTitle( functorToString( anActor->GetFunctor() ).toLatin1().constData() );
          SMESH::RepaintCurrentView();
#ifndef DISABLE_PLOT2DVIEWER
          if(anActor->GetPlot2Histogram()) {
            SPlot2d_Histogram* aHistogram = anActor->UpdatePlot2Histogram();
            QString functorName = functorToString( anActor->GetFunctor());
            QString aHistogramName("%1 : %2");
            aHistogramName = aHistogramName.arg(anIO->getName()).arg(functorName);
            aHistogram->setName(aHistogramName);
            aHistogram->setHorTitle(functorName);
            SMESH::ProcessIn2DViewers(anActor);
          }
#endif
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
    for ( ; It.More(); It.Next())
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

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    _PTR(StudyBuilder) aStudyBuilder = aStudy->NewBuilder();
    _PTR(GenericAttribute) anAttr;
    _PTR(AttributeIOR) anIOR;

    int objectCount = 0;
    QString aNameList;
    QString aParentComponent = QString::null;
    Handle(SALOME_InteractiveObject) anIO;
    for( SALOME_ListIteratorOfListIO anIt( selected ); anIt.More(); anIt.Next() )
    {
      anIO = anIt.Value();
      QString cur = anIO->getComponentDataType();
      _PTR(SObject) aSO = aStudy->FindObjectID(anIO->getEntry());
      if (aSO) {
        // check if object is reference
        _PTR(SObject) aRefSObj;
        aNameList.append("\n    - ");
        if ( aSO->ReferencedObject( aRefSObj ) ) {
          QString aRefName = QString::fromStdString ( aRefSObj->GetName() );
          aNameList.append( aRefName );
          cur = QString::fromStdString ( aRefSObj->GetFatherComponent()->ComponentDataType() );
        }
        else
          aNameList.append(anIO->getName());
        objectCount++;
      }

      if( aParentComponent.isNull() )
        aParentComponent = cur;
      else if( !aParentComponent.isEmpty() && aParentComponent!=cur )
        aParentComponent = "";
    }

    if ( objectCount == 0 )
      return; // No Valid Objects Selected

    if ( aParentComponent != SMESHGUI::GetSMESHGUI()->name() )  {
      SUIT_MessageBox::warning( SMESHGUI::desktop(),
                                QObject::tr("ERR_ERROR"),
                                QObject::tr("NON_SMESH_OBJECTS_SELECTED").arg( SMESHGUI::GetSMESHGUI()->moduleName() ) );
      return;
    }
    // VSR 17/11/04: check if all objects selected belong to SMESH component <-- finish
    if (SUIT_MessageBox::warning
        (SMESHGUI::desktop(),
         QObject::tr("SMESH_WRN_WARNING"),
         QObject::tr("SMESH_REALLY_DELETE").arg( objectCount ).arg( aNameList ),
         SUIT_MessageBox::Yes | SUIT_MessageBox::No,
         SUIT_MessageBox::Yes) != SUIT_MessageBox::Yes)
      return;

    SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );

    // Put the whole hierarchy of sub-objects of the selected SO's into a list and
    // then treat them all starting from the deepest objects (at list back)
    std::list< _PTR(SObject) > listSO;
    SALOME_ListIteratorOfListIO It(selected);
    for( ; It.More(); It.Next()) // loop on selected IO's
    {
      Handle(SALOME_InteractiveObject) IObject = It.Value();
      if(IObject->hasEntry()) {
        _PTR(SObject) aSO = aStudy->FindObjectID(IObject->getEntry());

        // disable removal of "SMESH" component object
        if(aSO->FindAttribute(anAttr, "AttributeIOR")){
          anIOR = anAttr;
          if ( engineIOR() == anIOR->Value().c_str() )
            continue;
        }
        //Check the referenced object
        _PTR(SObject) aRefSObject;
        if ( aSO && aSO->ReferencedObject( aRefSObject ) )
          aSO = aRefSObject; // Delete main Object instead of reference

        listSO.push_back( aSO );
        std::list< _PTR(SObject) >::iterator itSO = listSO.begin();
        for ( ; itSO != listSO.end(); ++itSO ) {
          _PTR(ChildIterator) it = aStudy->NewChildIterator( *itSO );
          for (it->InitEx(false); it->More(); it->Next())
            listSO.push_back( it->Value() );
        }
      }
    }
    // Check if none of objects to delete is referred from outside
    std::list< _PTR(SObject) >::reverse_iterator ritSO;
    for ( ritSO = listSO.rbegin(); ritSO != listSO.rend(); ++ritSO )
    {
      _PTR(SObject) SO = *ritSO;
      if ( !SO ) continue;
      std::vector<_PTR(SObject)> aReferences = aStudy->FindDependances( *ritSO  );
      for (size_t i = 0; i < aReferences.size(); i++) {
        _PTR(SComponent) aComponent = aReferences[i]->GetFatherComponent();
        std::string type = aComponent->ComponentDataType();
        if ( type != "SMESH" )
        {
          SUIT_MessageBox::warning( anApp->desktop(),
                                    QObject::tr("WRN_WARNING"),
                                    QObject::tr("DEP_OBJECT") );
          return; // outside SMESH, there is an object depending on a SMESH object 
        }
      }
    }

    // Treat SO's in the list starting from the back
    aStudyBuilder->NewCommand();  // There is a transaction
    for ( ritSO = listSO.rbegin(); ritSO != listSO.rend(); ++ritSO )
    {
      _PTR(SObject) SO = *ritSO;
      if ( !SO ) continue;
      std::string anEntry = SO->GetID();

      /** Erase graphical object **/
      if(SO->FindAttribute(anAttr, "AttributeIOR")){
        ViewManagerList aViewMenegers = anApp->viewManagers();
        ViewManagerList::const_iterator it = aViewMenegers.begin();
        for( ; it != aViewMenegers.end(); it++) {         
          SUIT_ViewManager* vm = *it;
          int nbSf = vm ? vm->getViewsCount() : 0;
          if(vm) {
            QVector<SUIT_ViewWindow*> aViews = vm->getViews();
            for(int i = 0; i < nbSf; i++){
              SUIT_ViewWindow *sf = aViews[i];
              if(SMESH_Actor* anActor = SMESH::FindActorByEntry(sf,anEntry.c_str())){
                SMESH::RemoveActor(sf,anActor);
              }
            }
          }
        }
      }
      /** Remove an object from data structures **/
      SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_narrow( SMESH::SObjectToObject( SO ));
      SMESH::SMESH_subMesh_var   aSubMesh = SMESH::SMESH_subMesh::_narrow( SMESH::SObjectToObject( SO ));
      if ( !aGroup->_is_nil() ) {                          // DELETE GROUP
        SMESH::SMESH_Mesh_var aMesh = aGroup->GetMesh();
        aMesh->RemoveGroup( aGroup );
      }
      else if ( !aSubMesh->_is_nil() ) {                   // DELETE SUBMESH
        SMESH::SMESH_Mesh_var aMesh = aSubMesh->GetFather();
        aMesh->RemoveSubMesh( aSubMesh );

        _PTR(SObject) aMeshSO = SMESH::FindSObject(aMesh);
        if (aMeshSO)
          SMESH::ModifiedMesh(aMeshSO, false, aMesh->NbNodes()==0);
      }
      else {
        Handle(SALOME_InteractiveObject) IObject = new SALOME_InteractiveObject
          ( anEntry.c_str(), engineIOR().toLatin1().data(), SO->GetName().c_str() );
        QString objType = CheckTypeObject(IObject);
        if ( objType == "Hypothesis" || objType == "Algorithm" ) {// DELETE HYPOTHESIS
          SMESH::RemoveHypothesisOrAlgorithmOnMesh(IObject);
          aStudyBuilder->RemoveObjectWithChildren( SO );
        }
        else {// default action: remove SObject from the study
              // san - it's no use opening a transaction here until UNDO/REDO is provided in SMESH
              //SUIT_Operation *op = new SALOMEGUI_ImportOperation(myActiveStudy);
              //op->start();
          aStudyBuilder->RemoveObjectWithChildren( SO );
          //op->finish();
        }
      }
    } /* listSO back loop */

    aStudyBuilder->CommitCommand();

    /* Clear any previous selection */
    SALOME_ListIO l1;
    aSel->setSelectedObjects( l1 );

    SMESHGUI::GetSMESHGUI()->updateObjBrowser();
  }
//} namespace

extern "C" {
  SMESHGUI_EXPORT CAM_Module* createModule()
  {
    return new SMESHGUI();
  }

  SMESHGUI_EXPORT  char* getModuleVersion() {
    return (char*)SMESH_VERSION_STR;
  }
}

SMESH::SMESH_Gen_var SMESHGUI::myComponentSMESH = SMESH::SMESH_Gen::_nil();

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESHGUI::SMESHGUI() :
SalomeApp_Module( "SMESH" ),
LightApp_Module( "SMESH" )
{
  if ( CORBA::is_nil( myComponentSMESH ) )
  {
    CORBA::Boolean anIsEmbeddedMode;
    myComponentSMESH = SMESH_Client::GetSMESHGen(getApp()->orb(),anIsEmbeddedMode);
    MESSAGE("-------------------------------> anIsEmbeddedMode=" << anIsEmbeddedMode);

    //  0019923: EDF 765 SMESH : default values of hypothesis
    SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr(this);
    int nbSeg = aResourceMgr->integerValue( "SMESH", "segmentation", 10 );
    myComponentSMESH->SetBoundaryBoxSegmentation( nbSeg );
    nbSeg = aResourceMgr->integerValue( "SMESH", "nb_segments_per_edge", 15 );
    myComponentSMESH->SetDefaultNbSegments( nbSeg );

    const char* options[] = { "historical_python_dump", "forget_mesh_on_hyp_modif" };
    for ( size_t i = 0; i < sizeof(options)/sizeof(char*); ++i )
      if ( aResourceMgr->hasValue( "SMESH", options[i] ))
      {
        QString val = aResourceMgr->stringValue( "SMESH", options[i] );
        myComponentSMESH->SetOption( options[i], val.toLatin1().constData() );
      }
  }

  myActiveDialogBox = 0;
  myFilterLibraryDlg = 0;
  myState = -1;
  myDisplayer = 0;

  myEventCallbackCommand = vtkCallbackCommand::New();
  myEventCallbackCommand->Delete();
  myEventCallbackCommand->SetClientData( this );
  myEventCallbackCommand->SetCallback( SMESHGUI::ProcessEvents );
  myPriority = 0.0;

  SMESH::GetFilterManager();
  SMESH::GetPattern();
  SMESH::GetMeasurements();

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
#ifdef WITHGENERICOBJ
  SMESH::GetFilterManager()->UnRegister();
  SMESH::GetMeasurements()->UnRegister();
#endif
  SMESH::GetFilterManager() = SMESH::FilterManager::_nil();
  SMESH::GetMeasurements() = SMESH::Measurements::_nil();
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

//=============================================================================
/*!
 *
 */
//=============================================================================
bool SMESHGUI::automaticUpdate(unsigned int requestedSize, bool* limitExceeded)
{
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
  if ( !resMgr )
    return false;

  bool autoUpdate  = resMgr->booleanValue( "SMESH", "auto_update",  false );
  long updateLimit = resMgr->integerValue( "SMESH", "update_limit", 500000 );
  bool exceeded = updateLimit > 0 && requestedSize > updateLimit;
  if ( limitExceeded ) *limitExceeded = autoUpdate && exceeded;
  return autoUpdate && !exceeded;
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
void SMESHGUI::EmitSignalVisibilityChanged()
{
  emit SignalVisibilityChanged();
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
void SMESHGUI::Modified( bool theIsUpdateActions )
{
  if( SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() ) ) {
    if( SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( app->activeStudy() ) ) {
      appStudy->Modified();
      if( theIsUpdateActions )
        app->updateActions();
    }
  }
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
    SUIT_MessageBox::warning( SMESHGUI::desktop(),
                              QObject::tr("WRN_WARNING"),
                              QObject::tr("WRN_STUDY_LOCKED") );
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

  switch (theCommandID) {
  case 33:                                      // DELETE
    if(checkLock(aStudy)) break;
    OnEditDelete();
    break;

  case 116:
  case 115:
  case 117:
  case 113:
  case 112:
  case 111:                                     // IMPORT
    {
      if(checkLock(aStudy)) break;
      ::ImportMeshesFromFile(GetSMESHGen(),theCommandID);
      break;
    }

  case 150:    //MED FILE INFORMATION
    {
      SALOME_ListIO selected;
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      if( aSel )
        aSel->selectedObjects( selected );
      if( selected.Extent() )
      {
        Handle(SALOME_InteractiveObject) anIObject = selected.First();
        SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
        if ( !aMesh->_is_nil() )
        {
          SMESHGUI_FileInfoDlg dlg( desktop(), aMesh->GetMEDFileInfo() );
          dlg.exec();
        }
      }
      break;
    }

  case 122:                                     // EXPORT MED
  case 121:
  case 123:
  case 124:
  case 125:
  case 126:
  case 140:
  case 141:
  case 142:
  case 143:
  case 144:
  case 145:
    {
      ::ExportMeshToFile(theCommandID);
      break;
    }

  case 200:                                     // SCALAR BAR
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
#ifndef DISABLE_PLOT2DVIEWER
            SMESH::ProcessIn2DViewers(anActor,SMESH::RemoveFrom2dViewer);
#endif
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
  case 2021:
    {
      // dump control distribution data to the text file
      ::SaveDistribution();
      break;
    }

  case 2022:
    {
      // show/ distribution
      ::ShowDistribution();
      break;
    }

#ifndef DISABLE_PLOT2DVIEWER
  case 2023:
    {
      // plot distribution
      ::PlotDistribution();
      break;
    }
#endif

    // Auto-color
  case 1136:
    ::AutoColor();
  break;

  case 1137:
    ::DisableAutoColor();
  break;

  case 1134: // Clipping
  case 1133: // Tranparency
  case 1132: // Display preferences (colors, shrink size, line width, ...)

    // Display Mode
  case 215: // Nodes
  case 213: // Nodes
  case 212: // Nodes
  case 211: // Nodes
    ::SetDisplayMode(theCommandID, myMarkerMap);
  break;

  //2D quadratic representation
  case 231:
  case 232:
    ::SetDisplayMode(theCommandID, myMarkerMap);
  break;

  // Display Entity
  case 216: // 0D elements
  case 217: // Edges
  case 218: // Faces
  case 219: // Volumes
  case 220: // All Entity
    ::SetDisplayEntity(theCommandID);
  break;

  case 221: // Orientation of faces
    {
      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      SALOME_ListIteratorOfListIO it(selected);
      for( ; it.More(); it.Next()) {
        Handle(SALOME_InteractiveObject) anIObject = it.Value();
        if(anIObject->hasEntry()) {
          if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
            anActor->SetFacesOriented( !anActor->GetFacesOriented() );
          }
        }
      }
      break;
    }

  case 214:                                     // UPDATE
    {
      if(checkLock(aStudy)) break;
      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        SMESH::UpdateView();
      }
      catch (std::bad_alloc) { // PAL16774 (Crash after display of many groups)
        SMESH::OnVisuException();
      }
      catch (...) { // PAL16774 (Crash after display of many groups)
        SMESH::OnVisuException();
      }

      SALOME_ListIO l;
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      aSel->selectedObjects( l );
      aSel->setSelectedObjects( l );
      break;
    }

  case 300:                                     // ERASE
  case 301:                                     // DISPLAY
  case 302:                                     // DISPLAY ONLY
    {
      SMESH::EDisplaing anAction;
      switch (theCommandID) {
      case 300: anAction = SMESH::eErase; break;
      case 301: anAction = SMESH::eDisplay; break;
      case 302: anAction = SMESH::eDisplayOnly; break;
      }

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO sel_objects, to_process;
      if (aSel)
        aSel->selectedObjects( sel_objects );

      if( theCommandID==302 )
      {
        MESSAGE("anAction = SMESH::eDisplayOnly");
        startOperation( myEraseAll );
      }

      extractContainers( sel_objects, to_process );

      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        if (vtkwnd) {
          SALOME_ListIteratorOfListIO It( to_process );
          for ( ; It.More(); It.Next()) {
                MESSAGE("---");
            Handle(SALOME_InteractiveObject) IOS = It.Value();
            if (IOS->hasEntry()) {
                MESSAGE("---");
              if (!SMESH::UpdateView(anAction, IOS->getEntry())) {
                SMESHGUI::GetSMESHGUI()->EmitSignalVisibilityChanged();
                break; // PAL16774 (Crash after display of many groups)
              }
              if (anAction == SMESH::eDisplayOnly)
              {
                MESSAGE("anAction = SMESH::eDisplayOnly");
                anAction = SMESH::eDisplay;
              }
            }
          }
        }

        // PAL13338 + PAL15161 -->
        if ( ( theCommandID==301 || theCommandID==302 ) && !checkLock(aStudy)) {
                MESSAGE("anAction = SMESH::eDisplayOnly");
          SMESH::UpdateView();
          SMESHGUI::GetSMESHGUI()->EmitSignalVisibilityChanged();
        }
        // PAL13338 + PAL15161 <--
      }
      catch (...) { // PAL16774 (Crash after display of many groups)
        SMESH::OnVisuException();
      }

      if (anAction == SMESH::eErase) {
        MESSAGE("anAction == SMESH::eErase");
        SALOME_ListIO l1;
        aSel->setSelectedObjects( l1 );
      }
      else
        aSel->setSelectedObjects( to_process );

      break;
    }

  case 4000:                                    // NODES
    {
      if(checkLock(aStudy)) break;

      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();

        ( new SMESHGUI_NodesDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"),
                                 tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }

  case 2151:  // FILTER
  {
    if ( vtkwnd )
    {
      EmitSignalDeactivateDialog();
      ( new SMESHGUI_FilterDlg( this, SMESH::EDGE ) )->show();
    }
    break;
  }

  case 701:                                     // COMPUTE MESH
  case 711:                                     // PRECOMPUTE MESH
  case 712:                                     // EVALUATE MESH
  case 713:                                     // MESH ORDER
    {
      if (checkLock(aStudy)) break;
      startOperation( theCommandID );
    }
    break;

  case 702: // Create mesh
  case 703: // Create sub-mesh
  case 704: // Edit mesh/sub-mesh
    startOperation( theCommandID );
    break;
  case 705: // copy mesh
    {
      if (checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      ( new SMESHGUI_CopyMeshDlg( this ) )->show();
    }
    break;
  case 710: // Build compound mesh
    {
      if (checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      ( new SMESHGUI_BuildCompoundDlg( this ) )->show();
    }
    break;

  case 407: // DIAGONAL INVERSION
  case 408: // Delete diagonal
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      /*Standard_Boolean aRes;
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IObject);
      if ( aMesh->_is_nil() )
      {
        SUIT_MessageBox::warning(GetDesktop(), tr( "SMESH_WRN_WARNING" ),
          tr( "SMESH_BAD_SELECTION" ) );
        break;
      }
      */
      EmitSignalDeactivateDialog();
      if ( theCommandID == 407 )
        ( new SMESHGUI_TrianglesInversionDlg( this ) )->show();
      else
        ( new SMESHGUI_UnionOfTwoTrianglesDlg( this ) )->show();
      break;
    }
  case 409: // Change orientation
  case 410: // Union of triangles
  case 411: // Cutting of quadrangles
  case 419: // Splitting volumes into tetrahedra
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
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
      else if ( theCommandID == 419 )
        aDlg = new SMESHGUI_CuttingIntoTetraDlg(this);
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
        ( new SMESHGUI_SmoothingDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 413: // Extrusion
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_ExtrusionDlg ( this ) )->show();
      } else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 414: // Revolution
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RevolutionDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
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
        ( new SMESHGUI_MeshPatternDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 416: // Extrusion along a path
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_ExtrusionAlongPathDlg( this ) )->show();
      } else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 417: // Convert mesh to quadratic
    {
    startOperation( 417 );
      /*      if (checkLock(aStudy)) break;
      if (vtkwnd) {
        EmitSignalDeactivateDialog();
        new SMESHGUI_ConvToQuadDlg();
      } else {
        SUIT_MessageBox::warning(desktop(),
                               tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
                               }*/
      break;
    }
  case 418: // create 2D mesh from 3D
    {
      startOperation( 418 );
      break;
    }
  case 806:                                     // CREATE GEO GROUP
    {
      startOperation( 806 );
      break;
    }
  case 801:                                     // CREATE GROUP
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
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
      SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, aMesh);
      aDlg->show();
      break;
    }

  case 802:                                     // CONSTRUCT GROUP
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
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
                QStringList anEntryList;
                if (aNodes->length() > 0) {
                  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::NODE, aName + "_Nodes");
                  aGroup->Add(aNodes.inout());
                  if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aGroup ) )
                    anEntryList.append( aSObject->GetID().c_str() );
                }
                if (aEdges->length() > 0) {
                  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::EDGE, aName + "_Edges");
                  aGroup->Add(aEdges.inout());
                  if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aGroup ) )
                    anEntryList.append( aSObject->GetID().c_str() );
                }
                if (aFaces->length() > 0) {
                  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::FACE, aName + "_Faces");
                  aGroup->Add(aFaces.inout());
                  if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aGroup ) )
                    anEntryList.append( aSObject->GetID().c_str() );
                }
                if (aVolumes->length() > 0) {
                  SMESH::SMESH_Group_var aGroup = SMESH::AddGroup(aMesh, SMESH::VOLUME, aName + "_Volumes");
                  aGroup->Add(aVolumes.inout());
                  if( _PTR(SObject) aSObject = SMESH::ObjectToSObject( aGroup ) )
                    anEntryList.append( aSObject->GetID().c_str() );
                }
                updateObjBrowser();
                anApp->browseObjects( anEntryList );
              }
              catch(const SALOME::SALOME_Exception & S_ex){
                SalomeApp_Tools::QtCatchCorbaException(S_ex);
              }
            }
          }
        }
      }
      else if(nbSel==0) {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"),
                                 tr("SMESH_WRN_NO_AVAILABLE_DATA"));
      }
      break;
    }

  case 803:                                     // EDIT GROUP
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
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
          SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, aGroup);
          aDlg->show();
        }
      }
      if (nbSelectedGroups == 0)
        {
          SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, SMESH::SMESH_GroupBase::_nil());
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

  case 815:                                     // Edit GEOM GROUP as standalone
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if(checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      SALOME_ListIteratorOfListIO It (selected);
      for ( ; It.More(); It.Next() )
      {
        SMESH::SMESH_GroupOnGeom_var aGroup =
          SMESH::IObjectToInterface<SMESH::SMESH_GroupOnGeom>(It.Value());
        if (!aGroup->_is_nil()) {
          SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, aGroup, true );
          aDlg->show();
        }
        else
        {
          SMESH::SMESH_GroupOnFilter_var aGroup =
            SMESH::IObjectToInterface<SMESH::SMESH_GroupOnFilter>(It.Value());
          if (!aGroup->_is_nil()) {
            SMESHGUI_GroupDlg *aDlg = new SMESHGUI_GroupDlg( this, aGroup, true );
            aDlg->show();
          }
        }
      }
      break;
    }

    case 810: // Union Groups
    case 811: // Intersect groups
    case 812: // Cut groups
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();

      SMESHGUI_GroupOpDlg* aDlg = 0;
      if ( theCommandID == 810 )
        aDlg = new SMESHGUI_UnionGroupsDlg( this );
      else if ( theCommandID == 811 )
        aDlg = new SMESHGUI_IntersectGroupsDlg( this );
      else
        aDlg = new SMESHGUI_CutGroupsDlg( this );

      aDlg->show();

      break;
    }

    case 814: // Create groups of entities from existing groups of superior dimensions
    {
      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();
      SMESHGUI_GroupOpDlg* aDlg = new SMESHGUI_DimGroupDlg( this );
      aDlg->show();

      break;
    }

    case 813: // Delete groups with their contents
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                                  tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();

      ( new SMESHGUI_DeleteGroupDlg( this ) )->show();
      break;
    }

  case 900:                                     // MESH INFOS
  case 903:                                     // WHAT IS
    {
      int page = theCommandID == 900 ? SMESHGUI_MeshInfoDlg::BaseInfo : SMESHGUI_MeshInfoDlg::ElemInfo;
      EmitSignalDeactivateDialog();
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      if ( selected.Extent() > 1 ) { // a dlg for each IO
        SALOME_ListIteratorOfListIO It( selected );
        for ( ; It.More(); It.Next() ) {
          SMESHGUI_MeshInfoDlg* dlg = new SMESHGUI_MeshInfoDlg( SMESHGUI::desktop(), page );
          dlg->showInfo( It.Value() ); 
          dlg->show();
        }
      }
      else {
        SMESHGUI_MeshInfoDlg* dlg = new SMESHGUI_MeshInfoDlg( SMESHGUI::desktop(), page );
        dlg->show();
      }
      break;
    }

  case 904:                                     // FIND ELEM
    {
      startOperation( theCommandID );
      break;
    }

  case 1100:                                    // EDIT HYPOTHESIS
    {
      if(checkLock(aStudy)) break;

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      int nbSel = selected.Extent();

      if (nbSel == 1) {
        Handle(SALOME_InteractiveObject) anIObject = selected.First();
        SMESH::SMESH_Hypothesis_var aHypothesis = SMESH::IObjectToInterface<SMESH::SMESH_Hypothesis>(anIObject);

        /* Look for all mesh objects that have this hypothesis affected in order to flag as ModifiedMesh */
        /* At end below '...->updateObjBrowser(true)' will change icon of mesh objects                   */
        /* Warning : however by internal mechanism all subMeshes icons are changed !                     */
        if ( !aHypothesis->_is_nil() )
        {
          // BUG 0020378
          //SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator(aHypothesis->GetName());
          SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator(aHypothesis->GetName());
          if (aCreator) {
            aCreator->edit( aHypothesis.in(), anIObject->getName(), desktop(), this, SLOT( onHypothesisEdit( int ) ) );
          }
          else
          {
            // report error
          }
        }
      }
      break;
    }
  case 1102:                                    // REMOVE HYPOTHESIS / ALGORITHMS
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

  case 4009:                                    // ELEM0D
  case 4010:                                    // EDGE
  case 4021:                                    // TRIANGLE
  case 4022:                                    // QUAD
  case 4023:                                    // POLYGON
  case 4031:                                    // TETRA
  case 4032:                                    // HEXA
  case 4133:                                    // PENTA
  case 4134:                                    // PYRAMID
  case 4135:                                    // OCTA12
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        SMDSAbs_EntityType type = SMDSEntity_Edge;
        switch (theCommandID) {
        case 4009:
          type = SMDSEntity_0D; break;
        case 4021:
          type = SMDSEntity_Triangle; break;
        case 4022:
          type = SMDSEntity_Quadrangle; break;
        case 4031:
          type = SMDSEntity_Tetra; break;
        case 4023:
          type = SMDSEntity_Polygon; break;
        case 4032:
          type = SMDSEntity_Hexa; break;
        case 4133:
          type = SMDSEntity_Penta; break;
        case 4134:
          type = SMDSEntity_Pyramid; break;
        case 4135:
          type = SMDSEntity_Hexagonal_Prism; break;
        default:;
        }
        ( new SMESHGUI_AddMeshElementDlg( this, type ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4033:                                    // POLYHEDRON
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_CreatePolyhedralVolumeDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4034:     // QUADRATIC EDGE
  case 4035:     // QUADRATIC TRIANGLE
  case 4036:     // QUADRATIC QUADRANGLE
  case 4136:     // BIQUADRATIC QUADRANGLE
  case 4037:     // QUADRATIC TETRAHEDRON
  case 4038:     // QUADRATIC PYRAMID
  case 4039:     // QUADRATIC PENTAHEDRON
  case 4040:     // QUADRATIC HEXAHEDRON
  case 4140:     // TRIQUADRATIC HEXAHEDRON
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        SMDSAbs_EntityType type;

        switch (theCommandID) {
        case 4034:
          type = SMDSEntity_Quad_Edge; break;
        case 4035:
          type = SMDSEntity_Quad_Triangle; break;
        case 4036:
          type = SMDSEntity_Quad_Quadrangle; break;
        case 4136:
          type = SMDSEntity_BiQuad_Quadrangle; break;
        case 4037:
          type = SMDSEntity_Quad_Tetra; break;
        case 4038:
          type = SMDSEntity_Quad_Pyramid; break;
        case 4039:
          type = SMDSEntity_Quad_Penta; break;
        case 4040:
          type = SMDSEntity_Quad_Hexa;
        case 4140:
          type = SMDSEntity_TriQuad_Hexa;
          break;
        default:;
        }
         ( new SMESHGUI_AddQuadraticElementDlg( this, type ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4041:                                    // REMOVES NODES
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RemoveNodesDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4042:                                    // REMOVES ELEMENTS
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RemoveElementsDlg( this ) )->show();
      }
      else
        {
          SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                   tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
        }
      break;
    }
  case 4043: {                                // CLEAR_MESH

    if(checkLock(aStudy)) break;

    SALOME_ListIO selected;
    if( LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr() )
      aSel->selectedObjects( selected );

    SUIT_OverrideCursor wc;
    SALOME_ListIteratorOfListIO It (selected);
    for ( ; It.More(); It.Next() )
    {
      Handle(SALOME_InteractiveObject) IOS = It.Value();
      SMESH::SMESH_Mesh_var aMesh =
        SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IOS);
      if ( aMesh->_is_nil()) continue;
      try {
        SMESH::RemoveVisualObjectWithActors(IOS->getEntry(), true);
        aMesh->Clear();
        _PTR(SObject) aMeshSObj = SMESH::FindSObject(aMesh);
        SMESH::ModifiedMesh( aMeshSObj, false, true);
        // hide groups and submeshes
        _PTR(ChildIterator) anIter =
          SMESH::GetActiveStudyDocument()->NewChildIterator( aMeshSObj );
        for ( anIter->InitEx(true); anIter->More(); anIter->Next() )
        {
          _PTR(SObject) so = anIter->Value();
          SMESH::RemoveVisualObjectWithActors(so->GetID().c_str(), true);
        }
      }
      catch (const SALOME::SALOME_Exception& S_ex){
        wc.suspend();
        SalomeApp_Tools::QtCatchCorbaException(S_ex);
        wc.resume();
      }
    }
    SMESH::UpdateView();
    updateObjBrowser();
    break;
  }
  case 4044:                                     // REMOVE ORPHAN NODES
    {
      if(checkLock(aStudy)) break;
      SALOME_ListIO selected;
      if( LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr() )
        aSel->selectedObjects( selected );
      if ( selected.Extent() == 1 ) {
        Handle(SALOME_InteractiveObject) anIO = selected.First();
        SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(anIO);
        if ( !aMesh->_is_nil() ) {
          bool confirm = SUIT_MessageBox::question( SMESHGUI::desktop(),
                                                    tr( "SMESH_WARNING" ),
                                                    tr( "REMOVE_ORPHAN_NODES_QUESTION"),
                                                    SUIT_MessageBox::Yes |
                                                    SUIT_MessageBox::No,
                                                    SUIT_MessageBox::No ) == SUIT_MessageBox::Yes;
          if( confirm ) {
            try {
              SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
              int removed = aMeshEditor->RemoveOrphanNodes();
              SUIT_MessageBox::information(SMESHGUI::desktop(),
                                           tr("SMESH_INFORMATION"),
                                           tr("NB_NODES_REMOVED").arg(removed));
              if ( removed > 0 ) {
                SMESH::UpdateView();
                SMESHGUI::Modified();
              }
            }
            catch (const SALOME::SALOME_Exception& S_ex) {
              SalomeApp_Tools::QtCatchCorbaException(S_ex);
            }
            catch (...) {
            }
          }
        }
      }
      break;
    }
  case 4051:                                    // RENUMBERING NODES
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RenumberingDlg( this, 0 ) )->show();
      }
      else
        {
          SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                   tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
        }
      break;
    }
  case 4052:                                    // RENUMBERING ELEMENTS
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RenumberingDlg( this, 1 ) )->show();
      }
      else
        {
          SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                   tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
        }
      break;
    }
  case 4061:                                   // TRANSLATION
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_TranslationDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4062:                                   // ROTATION
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RotationDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4063:                                   // SYMMETRY
    {
      if(checkLock(aStudy)) break;
      if(vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_SymmetryDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4064:                                   // SEWING
    {
      if(checkLock(aStudy)) break;
      if(vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_SewingDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4065:                                   // MERGE NODES
    {
      if(checkLock(aStudy)) break;
      if(vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_MergeDlg( this, 0 ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case 4066:                                   // MERGE EQUAL ELEMENTS
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_MergeDlg( this, 1 ) )->show();
      } else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }

  case 4067: // MAKE MESH PASS THROUGH POINT
    startOperation( 4067 );
    break;

  case 4068: // SCALE
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_ScaleDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }

  case 4069: // DUPLICATE NODES
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_DuplicateNodesDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }

  case 5105: // Library of selection filters
  {
    static QList<int> aTypes;
    if ( aTypes.isEmpty() )
    {
      aTypes.append( SMESH::NODE );
      aTypes.append( SMESH::EDGE );
      aTypes.append( SMESH::FACE );
      aTypes.append( SMESH::VOLUME );
    }
    if (!myFilterLibraryDlg)
      myFilterLibraryDlg = new SMESHGUI_FilterLibraryDlg( this, SMESH::GetDesktop( this ), aTypes, SMESHGUI_FilterLibraryDlg::EDIT );
    else if (myFilterLibraryDlg->isHidden())
      myFilterLibraryDlg->Init( aTypes, SMESHGUI_FilterLibraryDlg::EDIT );
    myFilterLibraryDlg->raise();
  }
  break;

  case 6017:                                    // CONTROLS
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
  case 6005:
  case 6009:
  case 6021:
  case 6022:
  case 6023:
  case 6024:
  case 6025:
  case 6026:
  case 6027:
  case 6028:
  case 6029:
  case 6030:
  case 6031:
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
      SUIT_MessageBox::warning(desktop(),
                               tr( "SMESH_WRN_WARNING" ),
                               tr( "SMESH_BAD_SELECTION" ) );
      break;
    }
    else {
      SUIT_MessageBox::warning(desktop(),
                               tr( "SMESH_WRN_WARNING" ),
                               tr( "NOT_A_VTK_VIEWER" ) );
    }
    break;
  case 9010:
    {
      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      SALOME_ListIteratorOfListIO it(selected);
      for( ; it.More(); it.Next()) {
        Handle(SALOME_InteractiveObject) anIObject = it.Value();
        if(anIObject->hasEntry()) {
          if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
            anActor->SetPointsLabeled( !anActor->GetPointsLabeled() );
          }
        }
      }
      break;
    }
  case 9011:
    {
      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      SALOME_ListIteratorOfListIO it(selected);
      for( ; it.More(); it.Next()) {
        Handle(SALOME_InteractiveObject) anIObject = it.Value();
        if(anIObject->hasEntry())
          if(SMESH_Actor *anActor = SMESH::FindActorByEntry(anIObject->getEntry())){
            anActor->SetCellsLabeled( !anActor->GetCellsLabeled() );
          }
      }
      break;
    }
  case 501:
  case 502:
    {
      int page = theCommandID == 501 ? SMESHGUI_MeasureDlg::MinDistance : SMESHGUI_MeasureDlg::BoundingBox;
      EmitSignalDeactivateDialog();
      SMESHGUI_MeasureDlg* dlg = new SMESHGUI_MeasureDlg( SMESHGUI::desktop(), page );
      dlg->show();
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
void SMESHGUI::createSMESHAction( const int id, const QString& po_id, const QString& icon_id,
                                  const int key, const bool toggle, const QString& shortcutAction  )
{
  QIcon icon;
  QWidget* parent = application()->desktop();
  SUIT_ResourceMgr* resMgr = resourceMgr();
  QPixmap pix;
  if ( !icon_id.isEmpty() )
    pix = resMgr->loadPixmap( "SMESH", tr( icon_id.toLatin1().data() ) );
  else
    pix = resMgr->loadPixmap( "SMESH", tr( QString( "ICO_%1" ).arg( po_id ).toLatin1().data() ), false );
  if ( !pix.isNull() )
    icon = QIcon( pix );

  QString tooltip    = tr( QString( "TOP_%1" ).arg( po_id ).toLatin1().data() ),
          menu       = tr( QString( "MEN_%1" ).arg( po_id ).toLatin1().data() ),
          status_bar = tr( QString( "STB_%1" ).arg( po_id ).toLatin1().data() );

  createAction( id, tooltip, icon, menu, status_bar, key, parent,
                toggle, this, SLOT( OnGUIEvent() ), shortcutAction );
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

  QString lc = "$";        // VSR : instead of QtxPopupSelection::defEquality();
  QString dc = "selcount"; // VSR : insetad of QtxPopupSelection::defSelCountParam()
  QString rule = "(%1) and (%2) and (%3)";
  rule = rule.arg( QString( "%1>0" ).arg( dc ) );
  if( clients.isEmpty() )
    rule = rule.arg( QString( "true" ) );
  else
    rule = rule.arg( QString( "%1client in {%2}" ).arg( lc ).arg( clients ) );
  rule = rule.arg( QString( "%1type in {%2}" ).arg( lc ).arg( types ) );
  rule += theRule;

  bool cont = myRules.contains( id );
  if( cont )
    rule = QString( "%1 or (%2)" ).arg( myRules[ id ] ).arg( rule );

  popupMgr()->setRule( action( id ), rule, QtxPopupMgr::VisibleRule );
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

  createSMESHAction(  111, "IMPORT_DAT", "", (Qt::CTRL+Qt::Key_B) );
  createSMESHAction(  112, "IMPORT_UNV", "", (Qt::CTRL+Qt::Key_U) );
  createSMESHAction(  113, "IMPORT_MED", "", (Qt::CTRL+Qt::Key_M) );
  createSMESHAction(  114, "NUM" );
  createSMESHAction(  115, "IMPORT_STL" );
  createSMESHAction(  116, "IMPORT_CGNS" );
  createSMESHAction(  117, "IMPORT_SAUV" );
  createSMESHAction(  121, "DAT" );
  createSMESHAction(  122, "MED" );
  createSMESHAction(  123, "UNV" );
  createSMESHAction(  140, "STL" );
  createSMESHAction(  142, "CGNS" );
  createSMESHAction(  144, "SAUV" );
  createSMESHAction(  124, "EXPORT_DAT" );
  createSMESHAction(  125, "EXPORT_MED" );
  createSMESHAction(  126, "EXPORT_UNV" );
  createSMESHAction(  141, "EXPORT_STL" );
  createSMESHAction(  143, "EXPORT_CGNS" );
  createSMESHAction(  145, "EXPORT_SAUV" );
  createSMESHAction(  150, "FILE_INFO" );
  createSMESHAction(   33, "DELETE",          "ICON_DELETE", Qt::Key_Delete );
  createSMESHAction( 5105, "SEL_FILTER_LIB" );
  createSMESHAction(  701, "COMPUTE",         "ICON_COMPUTE" );
  createSMESHAction(  702, "CREATE_MESH",     "ICON_DLG_INIT_MESH" );
  createSMESHAction(  703, "CREATE_SUBMESH",  "ICON_DLG_ADD_SUBMESH" );
  createSMESHAction(  704, "EDIT_MESHSUBMESH","ICON_DLG_EDIT_MESH" );
  createSMESHAction(  705, "COPY_MESH",       "ICON_COPY_MESH" );
  createSMESHAction(  710, "BUILD_COMPOUND",  "ICON_BUILD_COMPOUND" );
  createSMESHAction(  711, "PRECOMPUTE",      "ICON_PRECOMPUTE" );
  createSMESHAction(  712, "EVALUATE",        "ICON_COMPUTE" );
  createSMESHAction(  713, "MESH_ORDER",      "ICON_COMPUTE" );
  createSMESHAction(  806, "CREATE_GEO_GROUP","ICON_CREATE_GEO_GROUP" );
  createSMESHAction(  801, "CREATE_GROUP",    "ICON_CREATE_GROUP" );
  createSMESHAction(  802, "CONSTRUCT_GROUP", "ICON_CONSTRUCT_GROUP" );
  createSMESHAction(  803, "EDIT_GROUP",      "ICON_EDIT_GROUP" );
  createSMESHAction(  815, "EDIT_GEOMGROUP_AS_GROUP", "ICON_EDIT_GROUP" );
  createSMESHAction(  804, "ADD" );
  createSMESHAction(  805, "REMOVE" );
  createSMESHAction(  810, "UN_GROUP",        "ICON_UNION" );
  createSMESHAction(  811, "INT_GROUP",       "ICON_INTERSECT" );
  createSMESHAction(  812, "CUT_GROUP",       "ICON_CUT" );
  createSMESHAction(  814, "UNDERLYING_ELEMS","ICON_UNDERLYING_ELEMS" );
  createSMESHAction(  813, "DEL_GROUP",       "ICON_DEL_GROUP" );
  createSMESHAction(  900, "ADV_INFO",        "ICON_ADV_INFO" );
  //createSMESHAction(  902, "STD_INFO",        "ICON_STD_INFO" );
  //createSMESHAction(  903, "WHAT_IS",         "ICON_WHAT_IS" ); // VSR: issue #0021242 (eliminate "Mesh Element Information" command)
  createSMESHAction(  904, "FIND_ELEM",       "ICON_FIND_ELEM" );
  createSMESHAction( 6001, "LENGTH",          "ICON_LENGTH",        0, true );
  createSMESHAction( 6002, "FREE_EDGE",       "ICON_FREE_EDGE",     0, true );
  createSMESHAction( 6021, "FREE_FACES",      "ICON_FREE_FACES",    0, true );
  createSMESHAction( 6022, "MAX_ELEMENT_LENGTH_2D",  "ICON_MAX_ELEMENT_LENGTH_2D",   0, true );
  createSMESHAction( 6023, "MAX_ELEMENT_LENGTH_3D",  "ICON_MAX_ELEMENT_LENGTH_3D",   0, true );
  createSMESHAction( 6024, "BARE_BORDER_VOLUME",     "ICON_BARE_BORDER_VOLUME",      0, true );
  createSMESHAction( 6025, "BARE_BORDER_FACE",       "ICON_BARE_BORDER_FACE",        0, true );
  createSMESHAction( 6026, "OVER_CONSTRAINED_VOLUME","ICON_OVER_CONSTRAINED_VOLUME", 0, true );
  createSMESHAction( 6027, "OVER_CONSTRAINED_FACE",  "ICON_OVER_CONSTRAINED_FACE",   0, true );
  createSMESHAction( 6028, "EQUAL_NODE",      "ICON_EQUAL_NODE",    0, true );
  createSMESHAction( 6029, "EQUAL_EDGE",      "ICON_EQUAL_EDGE",    0, true );
  createSMESHAction( 6030, "EQUAL_FACE",      "ICON_EQUAL_FACE",    0, true );
  createSMESHAction( 6031, "EQUAL_VOLUME",    "ICON_EQUAL_VOLUME",  0, true );
  createSMESHAction( 6003, "FREE_BORDER",     "ICON_FREE_EDGE_2D",  0, true );
  createSMESHAction( 6004, "CONNECTION",      "ICON_CONNECTION",    0, true );
  createSMESHAction( 6005, "FREE_NODE",       "ICON_FREE_NODE",     0, true );
  createSMESHAction( 6011, "AREA",            "ICON_AREA",          0, true );
  createSMESHAction( 6012, "TAPER",           "ICON_TAPER",         0, true );
  createSMESHAction( 6013, "ASPECT",          "ICON_ASPECT",        0, true );
  createSMESHAction( 6014, "MIN_ANG",         "ICON_ANGLE",         0, true );
  createSMESHAction( 6015, "WARP",            "ICON_WARP",          0, true );
  createSMESHAction( 6016, "SKEW",            "ICON_SKEW",          0, true );
  createSMESHAction( 6017, "ASPECT_3D",       "ICON_ASPECT_3D",     0, true );
  createSMESHAction( 6018, "LENGTH_2D",       "ICON_LENGTH_2D",     0, true );
  createSMESHAction( 6019, "CONNECTION_2D",   "ICON_CONNECTION_2D", 0, true );
  createSMESHAction( 6009, "VOLUME_3D",       "ICON_VOLUME_3D",     0, true );
  createSMESHAction( 4000, "NODE",            "ICON_DLG_NODE" );
  createSMESHAction( 4009, "ELEM0D",          "ICON_DLG_ELEM0D" );
  createSMESHAction( 4010, "EDGE",            "ICON_DLG_EDGE" );
  createSMESHAction( 4021, "TRIANGLE",        "ICON_DLG_TRIANGLE" );
  createSMESHAction( 4022, "QUAD",            "ICON_DLG_QUADRANGLE" );
  createSMESHAction( 4023, "POLYGON",         "ICON_DLG_POLYGON" );
  createSMESHAction( 4031, "TETRA",           "ICON_DLG_TETRAS" );
  createSMESHAction( 4032, "HEXA",            "ICON_DLG_HEXAS" );
  createSMESHAction( 4133, "PENTA",           "ICON_DLG_PENTA" );
  createSMESHAction( 4134, "PYRAMID",         "ICON_DLG_PYRAMID" );
  createSMESHAction( 4135, "OCTA",            "ICON_DLG_OCTA" );
  createSMESHAction( 4033, "POLYHEDRON",              "ICON_DLG_POLYHEDRON" );
  createSMESHAction( 4034, "QUADRATIC_EDGE",          "ICON_DLG_QUADRATIC_EDGE" );
  createSMESHAction( 4035, "QUADRATIC_TRIANGLE",      "ICON_DLG_QUADRATIC_TRIANGLE" );
  createSMESHAction( 4036, "QUADRATIC_QUADRANGLE",    "ICON_DLG_QUADRATIC_QUADRANGLE" );
  createSMESHAction( 4136, "BIQUADRATIC_QUADRANGLE",  "ICON_DLG_BIQUADRATIC_QUADRANGLE" );
  createSMESHAction( 4037, "QUADRATIC_TETRAHEDRON",   "ICON_DLG_QUADRATIC_TETRAHEDRON" );
  createSMESHAction( 4038, "QUADRATIC_PYRAMID",       "ICON_DLG_QUADRATIC_PYRAMID" );
  createSMESHAction( 4039, "QUADRATIC_PENTAHEDRON",   "ICON_DLG_QUADRATIC_PENTAHEDRON" );
  createSMESHAction( 4040, "QUADRATIC_HEXAHEDRON",    "ICON_DLG_QUADRATIC_HEXAHEDRON" );
  createSMESHAction( 4140, "TRIQUADRATIC_HEXAHEDRON", "ICON_DLG_TRIQUADRATIC_HEXAHEDRON" );
  createSMESHAction( 4041, "REMOVE_NODES",          "ICON_DLG_REM_NODE" );
  createSMESHAction( 4042, "REMOVE_ELEMENTS",       "ICON_DLG_REM_ELEMENT" );
  createSMESHAction( 4044, "REMOVE_ORPHAN_NODES",   "ICON_DLG_REM_ORPHAN_NODES" );
  createSMESHAction( 4043, "CLEAR_MESH"    ,  "ICON_CLEAR_MESH" );
  createSMESHAction( 4051, "RENUM_NODES",     "ICON_DLG_RENUMBERING_NODES" );
  createSMESHAction( 4052, "RENUM_ELEMENTS",  "ICON_DLG_RENUMBERING_ELEMENTS" );
  createSMESHAction( 4061, "TRANS",           "ICON_SMESH_TRANSLATION_VECTOR" );
  createSMESHAction( 4062, "ROT",             "ICON_DLG_MESH_ROTATION" );
  createSMESHAction( 4063, "SYM",             "ICON_SMESH_SYMMETRY_PLANE" );
  createSMESHAction( 4064, "SEW",             "ICON_SMESH_SEWING_FREEBORDERS" );
  createSMESHAction( 4065, "MERGE",           "ICON_SMESH_MERGE_NODES" );
  createSMESHAction( 4066, "MERGE_ELEMENTS",  "ICON_DLG_MERGE_ELEMENTS" );
  createSMESHAction( 4067, "MESH_THROU_POINT","ICON_DLG_MOVE_NODE" );
  createSMESHAction( 4068, "SCALE",           "ICON_DLG_MESH_SCALE" );
  createSMESHAction( 4069, "DUPLICATE_NODES", "ICON_SMESH_DUPLICATE_NODES" );
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
  createSMESHAction(  417, "CONV_TO_QUAD",    "ICON_CONV_TO_QUAD" );
  createSMESHAction(  418, "2D_FROM_3D",      "ICON_2D_FROM_3D" );
  createSMESHAction(  419, "SPLIT_TO_TETRA",  "ICON_SPLIT_TO_TETRA" );
  createSMESHAction(  200, "RESET" );
  createSMESHAction(  201, "SCALAR_BAR_PROP" );
  createSMESHAction(  2021, "SAVE_DISTRIBUTION" );
  createSMESHAction(  2022, "SHOW_DISTRIBUTION","",0, true );
#ifndef DISABLE_PLOT2DVIEWER
  createSMESHAction(  2023, "PLOT_DISTRIBUTION" );
#endif
  createSMESHAction(  211, "WIRE",           "ICON_WIRE", 0, true );
  createSMESHAction(  212, "SHADE",          "ICON_SHADE", 0, true );
  createSMESHAction(  213, "SHRINK",         "ICON_SHRINK", 0, true );
  createSMESHAction(  214, "UPDATE",         "ICON_UPDATE" );
  createSMESHAction(  215, "NODES",          "ICON_POINTS", 0, true );
  createSMESHAction(  216, "ELEMS0D",        "ICON_DLG_ELEM0D", 0, true );
  createSMESHAction(  217, "EDGES",          "ICON_DLG_EDGE", 0, true );
  createSMESHAction(  218, "FACES",          "ICON_DLG_TRIANGLE", 0, true );
  createSMESHAction(  219, "VOLUMES",        "ICON_DLG_TETRAS", 0, true );
  createSMESHAction(  220, "ALL" );
  createSMESHAction(  221, "FACE_ORIENTATION", "", 0, true );

  createSMESHAction(  231, "LINE_REPRESENTATION", "", 0, true );
  createSMESHAction(  232, "ARC_REPRESENTATION", "", 0, true );

  createSMESHAction( 1100, "EDIT_HYPO" );
  createSMESHAction( 1102, "UNASSIGN" );
  createSMESHAction( 9010, "NUM_NODES", "", 0, true );
  createSMESHAction( 9011, "NUM_ELEMENTS", "", 0, true );
  createSMESHAction( 1131, "DISPMODE" );
  createSMESHAction( 1132, "COLORS" );
  createSMESHAction( 1133, "TRANSP" );
  createSMESHAction( 1134, "CLIP" );
  createSMESHAction( 1135, "DISP_ENT" );
  createSMESHAction( 1136, "AUTO_COLOR" );
  createSMESHAction( 1137, "DISABLE_AUTO_COLOR" );
  createSMESHAction( 2000, "CTRL" );

  createSMESHAction( 501, "MEASURE_MIN_DIST", "ICON_MEASURE_MIN_DIST" );
  createSMESHAction( 502, "MEASURE_BND_BOX",  "ICON_MEASURE_BND_BOX" );

  createSMESHAction( 300, "HIDE" );
  createSMESHAction( 301, "SHOW" );
  createSMESHAction( 302, "DISPLAY_ONLY" );

  // ----- create menu --------------
  int fileId    = createMenu( tr( "MEN_FILE" ),    -1,  1 ),
      editId    = createMenu( tr( "MEN_EDIT" ),    -1,  3 ),
      toolsId   = createMenu( tr( "MEN_TOOLS" ),   -1,  5, 50 ),
      meshId    = createMenu( tr( "MEN_MESH" ),    -1, 70, 10 ),
      ctrlId    = createMenu( tr( "MEN_CTRL" ),    -1, 60, 10 ),
      modifyId  = createMenu( tr( "MEN_MODIFY" ),  -1, 40, 10 ),
      measureId = createMenu( tr( "MEN_MEASURE" ), -1, 50, 10 ),
      viewId    = createMenu( tr( "MEN_VIEW" ),    -1,  2 );

  createMenu( separator(), fileId );

  int importId = createMenu( tr( "MEN_IMPORT" ), fileId, -1, 10 ),
      exportId = createMenu( tr( "MEN_EXPORT" ), fileId, -1, 10 ),
      nodeId   = createMenu( tr( "MEN_NODE_CTRL" ), ctrlId, -1, 10 ),
      edgeId   = createMenu( tr( "MEN_EDGE_CTRL" ), ctrlId, -1, 10 ),
      faceId   = createMenu( tr( "MEN_FACE_CTRL" ), ctrlId, -1, 10 ),
      volumeId = createMenu( tr( "MEN_VOLUME_CTRL" ), ctrlId, -1, 10 ),
      addId    = createMenu( tr( "MEN_ADD" ),    modifyId, 402 ),
      removeId = createMenu( tr( "MEN_REMOVE" ), modifyId, 403 ),
      renumId  = createMenu( tr( "MEN_RENUM" ),  modifyId, 404 ),
      transfId = createMenu( tr( "MEN_TRANSF" ), modifyId, 405 );

  createMenu( 111, importId, -1 );
  createMenu( 112, importId, -1 );
  createMenu( 113, importId, -1 );
  createMenu( 115, importId, -1 );
#ifdef WITH_CGNS
  createMenu( 116, importId, -1 );
#endif
  createMenu( 117, importId, -1 );
  createMenu( 121, exportId, -1 );
  createMenu( 122, exportId, -1 );
  createMenu( 123, exportId, -1 );
  createMenu( 140, exportId, -1 ); // export to STL
#ifdef WITH_CGNS
  createMenu( 142, exportId, -1 ); // export to CGNS
#endif
  createMenu( 144, exportId, -1 ); // export to SAUV
  createMenu( separator(), fileId, 10 );

  createMenu( 33, editId, -1 );

  createMenu( 5105, toolsId, -1 );

  createMenu( 702, meshId, -1 ); // "Mesh" menu
  createMenu( 703, meshId, -1 );
  createMenu( 704, meshId, -1 );
  createMenu( 710, meshId, -1 );
  createMenu( 705, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 701, meshId, -1 );
  createMenu( 711, meshId, -1 );
  createMenu( 712, meshId, -1 );
  createMenu( 713, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 801, meshId, -1 );
  createMenu( 806, meshId, -1 );
  createMenu( 802, meshId, -1 );
  createMenu( 803, meshId, -1 );
  createMenu( 815, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 810, meshId, -1 );
  createMenu( 811, meshId, -1 );
  createMenu( 812, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 814, meshId, -1 );
  createMenu( separator(), meshId, -1 );
  createMenu( 900, meshId, -1 );
  //createMenu( 902, meshId, -1 );
  //createMenu( 903, meshId, -1 ); // VSR: issue #0021242 (eliminate "Mesh Element Information" command)
  createMenu( 904, meshId, -1 );
  createMenu( separator(), meshId, -1 );

  createMenu( 6005, nodeId, -1 );
  createMenu( 6028, nodeId, -1 );
  createMenu( 6002, edgeId, -1 );
  createMenu( 6003, edgeId, -1 );
  createMenu( 6001, edgeId, -1 );
  createMenu( 6004, edgeId, -1 );
  createMenu( 6029, edgeId, -1 );
  createMenu( 6021, faceId, -1 );
  createMenu( 6025, faceId, -1 );
  createMenu( 6027, faceId, -1 );
  createMenu( 6018, faceId, -1 );
  createMenu( 6019, faceId, -1 );
  createMenu( 6011, faceId, -1 );
  createMenu( 6012, faceId, -1 );
  createMenu( 6013, faceId, -1 );
  createMenu( 6014, faceId, -1 );
  createMenu( 6015, faceId, -1 );
  createMenu( 6016, faceId, -1 );
  createMenu( 6022, faceId, -1 );
  createMenu( 6030, faceId, -1 );
  createMenu( 6017, volumeId, -1 );
  createMenu( 6009, volumeId, -1 );
  createMenu( 6023, volumeId, -1 );
  createMenu( 6024, volumeId, -1 );
  createMenu( 6026, volumeId, -1 );
  createMenu( 6031, volumeId, -1 );

  createMenu( 4000, addId, -1 );
  createMenu( 4009, addId, -1 );
  createMenu( 4010, addId, -1 );
  createMenu( 4021, addId, -1 );
  createMenu( 4022, addId, -1 );
  createMenu( 4023, addId, -1 );
  createMenu( 4031, addId, -1 );
  createMenu( 4032, addId, -1 );
  createMenu( 4133, addId, -1 );
  createMenu( 4134, addId, -1 );
  createMenu( 4135, addId, -1 );
  createMenu( 4033, addId, -1 );
  createMenu( separator(), addId, -1 );
  createMenu( 4034, addId, -1 );
  createMenu( 4035, addId, -1 );
  createMenu( 4036, addId, -1 );
  createMenu( 4136, addId, -1 );
  createMenu( 4037, addId, -1 );
  createMenu( 4038, addId, -1 );
  createMenu( 4039, addId, -1 );
  createMenu( 4040, addId, -1 );
  createMenu( 4140, addId, -1 );

  createMenu( 4041, removeId, -1 );
  createMenu( 4042, removeId, -1 );
  createMenu( 4044, removeId, -1 );
  createMenu( separator(), removeId, -1 );
  createMenu( 813, removeId, -1 );
  createMenu( separator(), removeId, -1 );
  createMenu( 4043, removeId, -1 );

  createMenu( 4051, renumId, -1 );
  createMenu( 4052, renumId, -1 );

  createMenu( 4061, transfId, -1 );
  createMenu( 4062, transfId, -1 );
  createMenu( 4063, transfId, -1 );
  createMenu( 4068, transfId, -1 );
  createMenu( 4064, transfId, -1 );
  createMenu( 4065, transfId, -1 );
  createMenu( 4066, transfId, -1 );
  createMenu( 4069, transfId, -1 );

  createMenu( 4067,modifyId, -1 );
  createMenu( 407, modifyId, -1 );
  createMenu( 408, modifyId, -1 );
  createMenu( 409, modifyId, -1 );
  createMenu( 410, modifyId, -1 );
  createMenu( 411, modifyId, -1 );
  createMenu( 419, modifyId, -1 );
  createMenu( 412, modifyId, -1 );
  createMenu( 413, modifyId, -1 );
  createMenu( 416, modifyId, -1 );
  createMenu( 414, modifyId, -1 );
  createMenu( 415, modifyId, -1 );
  createMenu( 417, modifyId, -1 );
  createMenu( 418, modifyId, -1 );

  createMenu( 501, measureId, -1 );
  createMenu( 502, measureId, -1 );
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
  createTool( 710, meshTb );
  createTool( 705, meshTb );
  createTool( separator(), meshTb );
  createTool( 701, meshTb );
  createTool( 711, meshTb );
  createTool( 712, meshTb );
  createTool( 713, meshTb );
  createTool( separator(), meshTb );
  createTool( 801, meshTb );
  createTool( 806, meshTb );
  createTool( 802, meshTb );
  createTool( 803, meshTb );
  //createTool( 815, meshTb );
  createTool( separator(), meshTb );
  createTool( 900, meshTb );
  //createTool( 902, meshTb );
  //createTool( 903, meshTb ); // VSR: issue #0021242 (eliminate "Mesh Element Information" command)
  createTool( 904, meshTb );
  createTool( separator(), meshTb );

  createTool( 6005, ctrlTb );
  createTool( 6028, ctrlTb );
  createTool( separator(), ctrlTb );
  createTool( 6002, ctrlTb );
  createTool( 6003, ctrlTb );
  createTool( 6001, ctrlTb );
  createTool( 6004, ctrlTb );
  createTool( 6029, ctrlTb );
  createTool( separator(), ctrlTb );
  createTool( 6021, ctrlTb );
  createTool( 6025, ctrlTb );
  createTool( 6027, ctrlTb );
  createTool( 6018, ctrlTb );
  createTool( 6019, ctrlTb );
  createTool( 6011, ctrlTb );
  createTool( 6012, ctrlTb );
  createTool( 6013, ctrlTb );
  createTool( 6014, ctrlTb );
  createTool( 6015, ctrlTb );
  createTool( 6016, ctrlTb );
  createTool( 6022, ctrlTb );
  createTool( 6030, ctrlTb );
  createTool( separator(), ctrlTb );
  createTool( 6017, ctrlTb );
  createTool( 6009, ctrlTb );
  createTool( 6023, ctrlTb );
  createTool( 6024, ctrlTb );
  createTool( 6026, ctrlTb );
  createTool( 6031, ctrlTb );
  createTool( separator(), ctrlTb );

  createTool( 4000, addRemTb );
  createTool( 4009, addRemTb );
  createTool( 4010, addRemTb );
  createTool( 4021, addRemTb );
  createTool( 4022, addRemTb );
  createTool( 4023, addRemTb );
  createTool( 4031, addRemTb );
  createTool( 4032, addRemTb );
  createTool( 4133, addRemTb );
  createTool( 4134, addRemTb );
  createTool( 4135, addRemTb );
  createTool( 4033, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4034, addRemTb );
  createTool( 4035, addRemTb );
  createTool( 4036, addRemTb );
  createTool( 4136, addRemTb );
  createTool( 4037, addRemTb );
  createTool( 4038, addRemTb );
  createTool( 4039, addRemTb );
  createTool( 4040, addRemTb );
  createTool( 4140, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4041, addRemTb );
  createTool( 4042, addRemTb );
  createTool( 4044, addRemTb );
  createTool( 4043, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4051, addRemTb );
  createTool( 4052, addRemTb );
  createTool( separator(), addRemTb );
  createTool( 4061, addRemTb );
  createTool( 4062, addRemTb );
  createTool( 4063, addRemTb );
  createTool( 4068, addRemTb );
  createTool( 4064, addRemTb );
  createTool( 4065, addRemTb );
  createTool( 4066, addRemTb );
  createTool( 4069, addRemTb );
  createTool( separator(), addRemTb );

  createTool( 4067,modifyTb );
  createTool( 407, modifyTb );
  createTool( 408, modifyTb );
  createTool( 409, modifyTb );
  createTool( 410, modifyTb );
  createTool( 411, modifyTb );
  createTool( 419, modifyTb );
  createTool( 412, modifyTb );
  createTool( 413, modifyTb );
  createTool( 416, modifyTb );
  createTool( 414, modifyTb );
  createTool( 415, modifyTb );
  createTool( 417, modifyTb );
  createTool( 418, modifyTb );

  createTool( 214, dispModeTb );

  QString lc = "$";        // VSR : instead of QtxPopupSelection::defEquality();
  QString dc = "selcount"; // VSR : instead of QtxPopupSelection::defSelCountParam()

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
          mesh_part = mesh + " " + subMesh + " " + group,
          mesh_group = mesh + " " + group,
          hyp_alg = hypo + " " + algo;

  // popup for object browser
  QString
    isInvisible("not( isVisible )"),
    isEmpty("numberOfNodes = 0"),
    isNotEmpty("numberOfNodes <> 0"),

    // has nodes, edges, etc in VISIBLE! actor
    hasNodes("(numberOfNodes > 0 )"),//&& isVisible)"),
    hasElems("(count( elemTypes ) > 0)"),
    hasDifferentElems("(count( elemTypes ) > 1)"),
    hasElems0d("({'Elem0d'} in elemTypes)"),
    hasEdges("({'Edge'} in elemTypes)"),
    hasFaces("({'Face'} in elemTypes)"),
    hasVolumes("({'Volume'} in elemTypes)");

  createPopupItem( 150, OB, mesh, "&& selcount=1 && isImported" );      // FILE INFORMATION
  createPopupItem( 703, OB, mesh, "&& isComputable");      // CREATE_SUBMESH
  createPopupItem( 704, OB, mesh, "&& isComputable");      // EDIT_MESHSUBMESH
  createPopupItem( 704, OB, subMesh, "&& isComputable" );  // EDIT_MESHSUBMESH
  createPopupItem( 803, OB, group );                       // EDIT_GROUP
  createPopupItem( 815, OB, group, "&& groupType != 'Group'" ); // EDIT AS STANDALONE

  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 701, OB, mesh, "&& isComputable" );     // COMPUTE
  createPopupItem( 711, OB, mesh, "&& isComputable && isPreComputable" ); // PRECOMPUTE
  createPopupItem( 712, OB, mesh, "&& isComputable" );     // EVALUATE
  createPopupItem( 713, OB, mesh, "&& isComputable" );     // MESH ORDER
  createPopupItem( 214, OB, mesh_part );                   // UPDATE
  createPopupItem( 900, OB, mesh_part );                   // ADV_INFO
  createPopupItem( 904, OB, mesh_group );                  // FIND_ELEM
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 801, OB, mesh );                        // CREATE_GROUP
  createPopupItem( 806, OB, mesh );                        // CREATE_GEO_GROUP
  createPopupItem( 802, OB, subMesh );                     // CONSTRUCT_GROUP
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 1100, OB, hypo);                        // EDIT HYPOTHESIS
  createPopupItem( 1102, OB, hyp_alg );                    // REMOVE HYPOTHESIS / ALGORITHMS
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 4043, OB, mesh );                       // CLEAR_MESH
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 417, OB, mesh + " " + subMesh );        // convert to quadratic
  createPopupItem( 418, OB, mesh + " " + group,            // create 2D mesh from 3D
                   "&& dim>=2");
  popupMgr()->insert( separator(), -1, 0 );

  QString only_one_non_empty = QString( " && %1=1 && numberOfNodes>0" ).arg( dc );
  QString multiple_non_empty = QString( " && %1>0 && numberOfNodes>0" ).arg( dc );
  QString only_one_2D        = only_one_non_empty + " && dim>1";

  createPopupItem( 125, OB, mesh_group, multiple_non_empty );   // EXPORT_MED
  createPopupItem( 126, OB, mesh_group, only_one_non_empty );   // EXPORT_UNV
  createPopupItem( 141, OB, mesh_group, only_one_2D );          // EXPORT_STL
#ifdef WITH_CGNS
  createPopupItem( 143, OB, mesh_group, multiple_non_empty );   // EXPORT_CGNS
#endif
  createPopupItem( 145, OB, mesh_group, multiple_non_empty );   // EXPORT_SAUV
  createPopupItem(  33, OB, mesh_part + " " + hyp_alg );        // DELETE
  createPopupItem( 813, OB, group );                            // DEL_GROUP with contents
  popupMgr()->insert( separator(), -1, 0 );

  // popup for viewer
  createPopupItem( 803, View, group ); // EDIT_GROUP
  createPopupItem( 804, View, elems ); // ADD
  createPopupItem( 805, View, elems ); // REMOVE

  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( 214, View, mesh_part );  // UPDATE
  createPopupItem( 900, View, mesh_part );  // ADV_INFO
  createPopupItem( 904, View, mesh );       // FIND_ELEM
  popupMgr()->insert( separator(), -1, 0 );

  createPopupItem( 1136, OB + " " + View, mesh, "&& (not isAutoColor)" ); // AUTO_COLOR
  createPopupItem( 1137, OB + " " + View, mesh, "&& isAutoColor" );       // DISABLE_AUTO_COLOR
  popupMgr()->insert( separator(), -1, 0 );

  int anId;
  QString aClient = QString( "%1client in {%2}" ).arg( lc ).arg( "'VTKViewer'" );
  QString aType = QString( "%1type in {%2}" ).arg( lc );
  aType = aType.arg( mesh_part );
  QString aMeshInVTK = aClient + "&&" + aType;

  aClient = "($client in {'VTKViewer' 'ObjectBrowser'})";
  QString anActiveVTK = QString("activeView = '%1'").arg(SVTK_Viewer::Type());
  QString aSelCount = QString( "%1 > 0" ).arg( dc );

  //-------------------------------------------------
  // Numbering
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_NUM" ), -1, -1 );

  popupMgr()->insert( action( 9010 ), anId, -1 );
  popupMgr()->setRule( action( 9010 ), aMeshInVTK + "&& isVisible &&" + hasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 9010 ), "{'Point'} in labeledTypes", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 9011 ), anId, -1 );
  popupMgr()->setRule( action( 9011 ), aMeshInVTK + "&& isVisible &&" + hasElems, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 9011 ), "{'Cell'} in labeledTypes", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Display Mode
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_DISPMODE" ), -1, -1 );

  popupMgr()->insert( action( 211 ), anId, -1 ); // WIRE
  popupMgr()->setRule( action( 211 ), aMeshInVTK + "&&" + hasElems, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 211 ), "displayMode = 'eEdge'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 212 ), anId, -1 ); // SHADE
  popupMgr()->setRule( action( 212 ),aMeshInVTK+ "&& (" + hasFaces + "||" + hasVolumes + ")", QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( 212 ), "displayMode = 'eSurface'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 215 ), anId, -1 ); // POINTS
  popupMgr()->setRule( action( 215 ), aMeshInVTK + "&&" + hasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 215 ), "displayMode = 'ePoint'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 213 ), anId, -1 ); // SHRINK
  popupMgr()->setRule( action( 213 ), aMeshInVTK + "&& shrinkMode <> 'IsNotShrinkable' && displayMode <> 'ePoint'", QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( 213 ), "shrinkMode = 'IsShrunk'", QtxPopupMgr::ToggleRule );

  //-------------------------------------------------
  // Display Entity
  //-------------------------------------------------
  QString aDiffElemsInVTK = aMeshInVTK + "&&" + hasDifferentElems;

  anId = popupMgr()->insert( tr( "MEN_DISP_ENT" ), -1, -1 );

  popupMgr()->insert( action(216), anId, -1 ); // ELEMS 0D
  popupMgr()->setRule(action(216), aDiffElemsInVTK + "&& isVisible &&" + hasElems0d, QtxPopupMgr::VisibleRule);
  popupMgr()->setRule(action(216), "{'Elem0d'} in entityMode", QtxPopupMgr::ToggleRule);

  popupMgr()->insert( action( 217 ), anId, -1 ); // EDGES
  popupMgr()->setRule( action( 217 ), aDiffElemsInVTK + "&& isVisible &&" + hasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 217 ), "{'Edge'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 218 ), anId, -1 ); // FACES
  popupMgr()->setRule( action( 218 ), aDiffElemsInVTK + "&& isVisible &&" + hasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 218 ), "{'Face'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 219 ), anId, -1 ); // VOLUMES
  popupMgr()->setRule( action( 219 ), aDiffElemsInVTK + "&& isVisible &&" + hasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 219 ), "{'Volume'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 220 ), anId, -1 ); // ALL
  popupMgr()->setRule( action( 220 ), aDiffElemsInVTK + "&& isVisible && not( elemTypes in entityMode )", QtxPopupMgr::VisibleRule );


  //-------------------------------------------------
  // Representation of the 2D Quadratic elements
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_QUADRATIC_REPRESENT" ), -1, -1 );
  popupMgr()->insert( action( 231 ), anId, -1 ); // LINE REPRESENTATION
  popupMgr()->setRule( action( 231 ), aMeshInVTK + "and isVisible",QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 231 ), "quadratic2DMode = 'eLines'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 232 ), anId, -1 ); // ARC REPRESENTATION
  popupMgr()->setRule( action( 232 ), aMeshInVTK + "and isVisible", QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 232 ), "quadratic2DMode = 'eArcs'", QtxPopupMgr::ToggleRule );

  //-------------------------------------------------
  // Orientation of faces
  //-------------------------------------------------
  popupMgr()->insert( action( 221 ), -1, -1 );
  popupMgr()->setRule( action( 221 ), aMeshInVTK + "&& isVisible", QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( 221 ), "facesOrientationMode = 'IsOriented'", QtxPopupMgr::ToggleRule );

  //-------------------------------------------------
  // Color / Size
  //-------------------------------------------------
  popupMgr()->insert( action( 1132 ), -1, -1 );
  popupMgr()->setRule( action( 1132 ), aMeshInVTK + "&& isVisible", QtxPopupMgr::VisibleRule );

  //-------------------------------------------------
  // Transparency
  //-------------------------------------------------
  popupMgr()->insert( action( 1133 ), -1, -1 );
  popupMgr()->setRule( action( 1133 ), aMeshInVTK + "&& isVisible", QtxPopupMgr::VisibleRule );

  //-------------------------------------------------
  // Controls
  //-------------------------------------------------
  QString
    aMeshInVtkHasNodes = aMeshInVTK + "&&" + hasNodes,
    aMeshInVtkHasEdges = aMeshInVTK + "&&" + hasEdges,
    aMeshInVtkHasFaces = aMeshInVTK + "&&" + hasFaces,
    aMeshInVtkHasVolumes = aMeshInVTK + "&&" + hasVolumes;

  anId = popupMgr()->insert( tr( "MEN_CTRL" ), -1, -1 );

  popupMgr()->insert( action( 200 ), anId, -1 ); // RESET
  popupMgr()->setRule( action( 200 ), aMeshInVTK + "&& controlMode <> 'eNone'", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), anId, -1 );

  int aSubId = popupMgr()->insert( tr( "MEN_NODE_CTRL" ), anId, -1 ); // NODE CONTROLS

  popupMgr()->insert( action( 6005 ), aSubId, -1 ); // FREE_NODE
  popupMgr()->setRule( action( 6005 ), aMeshInVtkHasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6005 ), "controlMode = 'eFreeNodes'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6028 ), aSubId, -1 ); // EQUAL_NODE
  popupMgr()->setRule( action( 6028 ), aMeshInVtkHasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6028 ), "controlMode = 'eCoincidentNodes'", QtxPopupMgr::ToggleRule);

  aSubId = popupMgr()->insert( tr( "MEN_EDGE_CTRL" ), anId, -1 ); // EDGE CONTROLS

  popupMgr()->insert( action( 6002 ), aSubId, -1 ); // FREE_EDGE
  popupMgr()->setRule( action( 6002 ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6002 ), "controlMode = 'eFreeEdges'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 6003 ), aSubId, -1 ); // FREE_BORDER
  popupMgr()->setRule( action( 6003 ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6003 ), "controlMode = 'eFreeBorders'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 6001 ), aSubId, -1 ); // LENGTH
  popupMgr()->setRule( action( 6001 ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6001 ), "controlMode = 'eLength'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( 6004 ), aSubId, -1 ); // CONNECTION
  popupMgr()->setRule( action( 6004 ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6004 ), "controlMode = 'eMultiConnection'", QtxPopupMgr::ToggleRule );
  popupMgr()->insert ( action( 6029 ), aSubId, -1 ); // EQUAL_EDGE
  popupMgr()->setRule( action( 6029 ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6029 ), "controlMode = 'eCoincidentElems1D'", QtxPopupMgr::ToggleRule);

  aSubId = popupMgr()->insert( tr( "MEN_FACE_CTRL" ), anId, -1 ); // FACE CONTROLS

  popupMgr()->insert ( action( 6021 ), aSubId, -1 ); // FREE_FACE
  popupMgr()->setRule( action( 6021 ), aMeshInVtkHasFaces /*aMeshInVtkHasVolumes*/,
                                       QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6021 ), "controlMode = 'eFreeFaces'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6018 ), aSubId, -1 ); // LENGTH_2D
  popupMgr()->setRule( action( 6018 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6018 ), "controlMode = 'eLength2D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6019 ), aSubId, -1 ); // CONNECTION_2D
  popupMgr()->setRule( action( 6019 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6019 ), "controlMode = 'eMultiConnection2D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6011 ), aSubId, -1 ); // AREA
  popupMgr()->setRule( action( 6011 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6011 ), "controlMode = 'eArea'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6012 ), aSubId, -1 ); // TAPER
  popupMgr()->setRule( action( 6012 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6012 ), "controlMode = 'eTaper'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6013 ), aSubId, -1 ); // ASPECT
  popupMgr()->setRule( action( 6013 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6013 ), "controlMode = 'eAspectRatio'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6014 ), aSubId, -1 ); // MIN_ANG
  popupMgr()->setRule( action( 6014 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6014 ), "controlMode = 'eMinimumAngle'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6015 ), aSubId, -1 ); // WARP
  popupMgr()->setRule( action( 6015 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6015 ), "controlMode = 'eWarping'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6016 ), aSubId, -1 ); // SKEW
  popupMgr()->setRule( action( 6016 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6016 ), "controlMode = 'eSkew'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6022 ), aSubId, -1 ); // MAX_ELEMENT_LENGTH_2D
  popupMgr()->setRule( action( 6022 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6022 ), "controlMode = 'eMaxElementLength2D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6025 ), aSubId, -1 ); // BARE_BORDER_FACE
  popupMgr()->setRule( action( 6025 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6025 ), "controlMode = 'eBareBorderFace'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6027 ), aSubId, -1 ); // OVER_CONSTRAINED_FACE
  popupMgr()->setRule( action( 6027 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6027 ), "controlMode = 'eOverConstrainedFace'", QtxPopupMgr::ToggleRule );
  popupMgr()->insert ( action( 6030 ), aSubId, -1 ); // EQUAL_FACE
  popupMgr()->setRule( action( 6030 ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6030 ), "controlMode = 'eCoincidentElems2D'", QtxPopupMgr::ToggleRule );

  aSubId = popupMgr()->insert( tr( "MEN_VOLUME_CTRL" ), anId, -1 ); // VOLUME CONTROLS

  popupMgr()->insert ( action( 6017 ), aSubId, -1 ); // ASPECT_3D
  popupMgr()->setRule( action( 6017 ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6017 ), "controlMode = 'eAspectRatio3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6009 ), aSubId, -1 ); // VOLUME_3D
  popupMgr()->setRule( action( 6009 ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6009 ), "controlMode = 'eVolume3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6023 ), aSubId, -1 ); // MAX_ELEMENT_LENGTH_3D
  popupMgr()->setRule( action( 6023 ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6023 ), "controlMode = 'eMaxElementLength3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6024 ), aSubId, -1 ); // BARE_BORDER_VOLUME
  popupMgr()->setRule( action( 6024 ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6024 ), "controlMode = 'eBareBorderVolume'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6026 ), aSubId, -1 ); // OVER_CONSTRAINED_VOLUME
  popupMgr()->setRule( action( 6026 ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6026 ), "controlMode = 'eOverConstrainedVolume'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( 6031 ), aSubId, -1 ); // EQUAL_VOLUME
  popupMgr()->setRule( action( 6031 ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 6031 ), "controlMode = 'eCoincidentElems3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( 201 ), anId, -1 ); // SCALAR_BAR_PROP
  popupMgr()->setRule( action( 201 ), aMeshInVTK + "&& controlMode <> 'eNone'", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), anId, -1 );

  aSubId = popupMgr()->insert( tr( "MEN_DISTRIBUTION_CTRL" ), anId, -1 ); // NODE CONTROLS

  popupMgr()->insert( action( 2021 ), aSubId, -1 ); // SAVE_DISTRIBUTION
  popupMgr()->setRule( action( 2021 ), aMeshInVTK + "&& isNumFunctor", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( action( 2022 ), aSubId, -1 ); // SHOW_DISTRIBUTION
  popupMgr()->setRule( action( 2022 ), aMeshInVTK + "&& isNumFunctor", QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( 2022 ), aMeshInVTK + "&& isNumFunctor && isDistributionVisible", QtxPopupMgr::ToggleRule);

#ifndef DISABLE_PLOT2DVIEWER
  popupMgr()->insert( action( 2023 ), aSubId, -1 ); // PLOT_DISTRIBUTION
  popupMgr()->setRule( action( 2023 ), aMeshInVTK + "&& isNumFunctor", QtxPopupMgr::VisibleRule );
#endif

  //-------------------------------------------------
  // Display / Erase
  //-------------------------------------------------
  popupMgr()->insert( separator(), -1, -1 );
  QString aRule = "$component={'SMESH'} and ( type='Component' or (" + aClient + " and " +
    aType + " and " + aSelCount + " and " + anActiveVTK + " and " + isNotEmpty + " %1 ) )";
  popupMgr()->insert( action( 301 ), -1, -1 ); // DISPLAY
  popupMgr()->setRule( action( 301 ), aRule.arg( "and (not isVisible)" ), QtxPopupMgr::VisibleRule );

  popupMgr()->insert( action( 300 ), -1, -1 ); // ERASE
  popupMgr()->setRule( action( 300 ), aRule.arg( "and isVisible" ), QtxPopupMgr::VisibleRule );

  popupMgr()->insert( action( 302 ), -1, -1 ); // DISPLAY_ONLY
  popupMgr()->setRule( action( 302 ), aRule.arg( "" ), QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Clipping
  //-------------------------------------------------
  popupMgr()->insert( action( 1134 ), -1, -1 );
  popupMgr()->setRule( action( 1134 ), "client='VTKViewer'", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), -1, -1 );

  connect( application(), SIGNAL( viewManagerActivated( SUIT_ViewManager* ) ),
           this, SLOT( onViewManagerActivated( SUIT_ViewManager* ) ) );

  connect( application(), SIGNAL( viewManagerRemoved( SUIT_ViewManager* ) ),
           this, SLOT( onViewManagerRemoved( SUIT_ViewManager* ) ) );
}

//================================================================================
/*!
 * \brief Return true if SMESH or GEOM objects are selected.
 * Is called form LightApp_Module::activateModule() which clear selection if
 * not isSelectionCompatible()
 */
//================================================================================

bool SMESHGUI::isSelectionCompatible()
{
  bool isCompatible = true;
  SALOME_ListIO selected;
  if ( LightApp_SelectionMgr *Sel = selectionMgr() )
    Sel->selectedObjects( selected );

  SALOME_ListIteratorOfListIO It( selected );
  for ( ; isCompatible && It.More(); It.Next())
    isCompatible =
      ( strcmp("GEOM", It.Value()->getComponentDataType()) == 0 ) ||
      ( strcmp("SMESH", It.Value()->getComponentDataType()) == 0 );

  return isCompatible;
}


bool SMESHGUI::reusableOperation( const int id )
{
  // compute, evaluate and precompute are not reusable operations
  return ( id == 701 || id == 711 || id == 712 ) ? false : SalomeApp_Module::reusableOperation( id );
}

bool SMESHGUI::activateModule( SUIT_Study* study )
{
  bool res = SalomeApp_Module::activateModule( study );

  setMenuShown( true );
  setToolShown( true );

  // import Python module that manages SMESH plugins (need to be here because SalomePyQt API uses active module)
  PyGILState_STATE gstate = PyGILState_Ensure();
  PyObject* pluginsmanager=PyImport_ImportModuleNoBlock((char*)"salome_pluginsmanager");
  if(pluginsmanager==NULL)
    PyErr_Print();
  else
    {
      PyObject* result=PyObject_CallMethod( pluginsmanager, (char*)"initialize", (char*)"isss",1,"smesh",tr("MEN_MESH").toStdString().c_str(),tr("SMESH_PLUGINS_OTHER").toStdString().c_str());
      if(result==NULL)
        PyErr_Print();
      Py_XDECREF(result);
    }
  PyGILState_Release(gstate);
  // end of GEOM plugins loading

  // Reset actions accelerator keys
  action(111)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B)); // Import DAT
  action(112)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U)); // Import UNV
  action(113)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M)); // Import MED

  action(  33)->setEnabled(true); // Delete: Key_Delete

  //  0020210. Make SMESH_Gen update meshes at switching GEOM->SMESH
  GetSMESHGen()->SetCurrentStudy(SALOMEDS::Study::_nil());
  if ( SalomeApp_Study* s = dynamic_cast<SalomeApp_Study*>( study ))
    if ( _PTR(Study) aStudy = s->studyDS()) {
      GetSMESHGen()->SetCurrentStudy( _CAST(Study,aStudy)->GetStudy() );
      updateObjBrowser(); // objects can be removed
    }

  // get all view currently opened in the study and connect their signals  to
  // the corresponding slots of the class.
  SUIT_Desktop* aDesk = study->application()->desktop();
  if ( aDesk ) {
    QList<SUIT_ViewWindow*> wndList = aDesk->windows();
    SUIT_ViewWindow* wnd;
    foreach ( wnd, wndList )
      connectView( wnd );
  }

  return res;
}

bool SMESHGUI::deactivateModule( SUIT_Study* study )
{
  setMenuShown( false );
  setToolShown( false );

  EmitSignalCloseAllDialogs();

  // Unset actions accelerator keys
  action(111)->setShortcut(QKeySequence()); // Import DAT
  action(112)->setShortcut(QKeySequence()); // Import UNV
  action(113)->setShortcut(QKeySequence()); // Import MED

  action(  33)->setEnabled(false); // Delete: Key_Delete

  return SalomeApp_Module::deactivateModule( study );
}

void SMESHGUI::studyClosed( SUIT_Study* s )
{
  SMESH::RemoveVisuData( s->id() );
  SalomeApp_Module::studyClosed( s );
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
  return QString( anIOR.in() );
}

void SMESHGUI::contextMenuPopup( const QString& client, QMenu* menu, QString& title )
{
  SalomeApp_Module::contextMenuPopup( client, menu, title );
  SALOME_ListIO lst;
  selectionMgr()->selectedObjects( lst );
  if ( ( client == "OCCViewer" || client == "VTKViewer" ) && lst.Extent() == 1 ) {
    Handle(SALOME_InteractiveObject) io = lst.First();
    SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( application()->activeStudy() );
    _PTR(Study) study = appStudy->studyDS();
    _PTR(SObject) obj = study->FindObjectID( io->getEntry() );
    if ( obj ) {
      QString aName = QString( obj->GetName().c_str() );
      while ( aName.at( aName.length() - 1 ) == ' ' ) // Remove extraspaces in Name of Popup
          aName.remove( (aName.length() - 1), 1 );
      title = aName;
    }
  }
}

LightApp_Selection* SMESHGUI::createSelection() const
{
  return new SMESHGUI_Selection();
}

void SMESHGUI::windows( QMap<int, int>& aMap ) const
{
  aMap.insert( SalomeApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
  aMap.insert( SalomeApp_Application::WT_PyConsole, Qt::BottomDockWidgetArea );
}

void SMESHGUI::viewManagers( QStringList& list ) const
{
  list.append( SVTK_Viewer::Type() );
}

void SMESHGUI::onViewManagerActivated( SUIT_ViewManager* mgr )
{
  if ( dynamic_cast<SVTK_ViewManager*>( mgr ) ) {
    SMESH::UpdateSelectionProp( this );

    QVector<SUIT_ViewWindow*> aViews = mgr->getViews();
    for(int i = 0; i < aViews.count() ; i++){
      SUIT_ViewWindow *sf = aViews[i];
      connectView( sf );
    }
  }
}

void SMESHGUI::onViewManagerRemoved( SUIT_ViewManager* theViewManager )
{
  if( theViewManager && theViewManager->getType() == SVTK_Viewer::Type() )
    myClippingPlaneInfoMap.erase( theViewManager );
}

void SMESHGUI::addActorAsObserver( SMESH_Actor* theActor )
{
  theActor->AddObserver( SMESH::DeleteActorEvent,
                         myEventCallbackCommand.GetPointer(),
                         myPriority );
}

void SMESHGUI::ProcessEvents( vtkObject* theObject,
                              unsigned long theEvent,
                              void* theClientData,
                              void* theCallData )
{
  if( SMESHGUI* aSMESHGUI = reinterpret_cast<SMESHGUI*>( theClientData ) ) {
    if( theObject && theEvent == SMESH::DeleteActorEvent ) {
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( theObject ) ) {
        SMESHGUI_ClippingPlaneInfoMap& aClippingPlaneInfoMap = aSMESHGUI->getClippingPlaneInfoMap();
        SMESHGUI_ClippingPlaneInfoMap::iterator anIter1 = aClippingPlaneInfoMap.begin();
        for( ; anIter1 != aClippingPlaneInfoMap.end(); anIter1++ ) {
          SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = anIter1->second;
          SMESHGUI_ClippingPlaneInfoList::iterator anIter2 = aClippingPlaneInfoList.begin();
          for( ; anIter2 != aClippingPlaneInfoList.end(); anIter2++ ) {
            SMESH::ClippingPlaneInfo& aClippingPlaneInfo = *anIter2;
            std::list<vtkActor*>& anActorList = aClippingPlaneInfo.ActorList;
            SMESH::TActorList::iterator anIter3 = anActorList.begin();
            for ( ; anIter3 != anActorList.end(); anIter3++ ) {
              if( anActor == *anIter3 ) {
                anActorList.erase( anIter3 );
                break;
              }
            }
          }
        }
      }
    }
  }
}

void SMESHGUI::createPreferences()
{
  // General tab ------------------------------------------------------------------------
  int genTab = addPreference( tr( "PREF_TAB_GENERAL" ) );

  int autoUpdate = addPreference( tr( "PREF_AUTO_UPDATE" ), genTab, LightApp_Preferences::Auto, "SMESH", "auto_update" );
  int lim = addPreference( tr( "PREF_UPDATE_LIMIT" ), autoUpdate, LightApp_Preferences::IntSpin, "SMESH", "update_limit" );
  setPreferenceProperty( lim, "min",  0 );
  setPreferenceProperty( lim, "max",  100000000 );
  setPreferenceProperty( lim, "step", 1000 );
  setPreferenceProperty( lim, "special", tr( "PREF_UPDATE_LIMIT_NOLIMIT" ) );

  int qaGroup = addPreference( tr( "PREF_GROUP_QUALITY" ), genTab );
  setPreferenceProperty( qaGroup, "columns", 2 );
  addPreference( tr( "PREF_DISPLAY_ENTITY" ), qaGroup, LightApp_Preferences::Bool, "SMESH", "display_entity" );
  addPreference( tr( "PREF_PRECISION_USE" ), qaGroup, LightApp_Preferences::Bool, "SMESH", "use_precision" );
  int prec = addPreference( tr( "PREF_PRECISION_VALUE" ), qaGroup, LightApp_Preferences::IntSpin, "SMESH", "controls_precision" );
  setPreferenceProperty( prec, "min", 0 );
  setPreferenceProperty( prec, "max", 16 );
  int doubleNodesTol = addPreference( tr( "PREF_EQUAL_NODES_TOL" ), qaGroup, LightApp_Preferences::DblSpin, "SMESH", "equal_nodes_tolerance" );
  setPreferenceProperty( doubleNodesTol, "precision", 10 );
  setPreferenceProperty( doubleNodesTol, "min", 0.0000000001 );
  setPreferenceProperty( doubleNodesTol, "max", 1000000.0 );
  setPreferenceProperty( doubleNodesTol, "step", 0.0000001 );

  int dispgroup = addPreference( tr( "PREF_DISPLAY_MODE" ), genTab );
  setPreferenceProperty( dispgroup, "columns", 2 );
  int dispmode = addPreference( tr( "PREF_DISPLAY_MODE" ), dispgroup, LightApp_Preferences::Selector, "SMESH", "display_mode" );
  QStringList modes;
  modes.append( tr("MEN_WIRE") );
  modes.append( tr("MEN_SHADE") );
  modes.append( tr("MEN_NODES") );
  modes.append( tr("MEN_SHRINK") );
  QList<QVariant> indices;
  indices.append( 0 );
  indices.append( 1 );
  indices.append( 2 );
  indices.append( 3 );
  setPreferenceProperty( dispmode, "strings", modes );
  setPreferenceProperty( dispmode, "indexes", indices );

  int arcgroup = addPreference( tr( "QUADRATIC_REPRESENT_MODE" ), genTab );
  setPreferenceProperty( arcgroup, "columns", 2 );
  int quadraticmode = addPreference( tr( "QUADRATIC_REPRESENT_MODE" ), arcgroup, LightApp_Preferences::Selector, "SMESH", "quadratic_mode" );
  QStringList quadraticModes;
  quadraticModes.append(tr("MEN_LINE_REPRESENTATION"));
  quadraticModes.append(tr("MEN_ARC_REPRESENTATION"));
  indices.clear();
  indices.append( 0 );
  indices.append( 1 );
  setPreferenceProperty( quadraticmode, "strings", quadraticModes );
  setPreferenceProperty( quadraticmode, "indexes", indices );

  int maxAngle = addPreference( tr( "MAX_ARC_ANGLE" ), arcgroup, LightApp_Preferences::IntSpin,
                              "SMESH", "max_angle" );
  setPreferenceProperty( maxAngle, "min", 1 );
  setPreferenceProperty( maxAngle, "max", 90 );



  int exportgroup = addPreference( tr( "PREF_GROUP_EXPORT" ), genTab );
  setPreferenceProperty( exportgroup, "columns", 2 );
  addPreference( tr( "PREF_AUTO_GROUPS" ), exportgroup, LightApp_Preferences::Bool, "SMESH", "auto_groups" );
  addPreference( tr( "PREF_RENUMBER" ), exportgroup, LightApp_Preferences::Bool, "SMESH", "renumbering" );

  int computeGroup = addPreference( tr( "PREF_GROUP_COMPUTE" ), genTab );
  setPreferenceProperty( computeGroup, "columns", 2 );
  int notifyMode = addPreference( tr( "PREF_NOTIFY_MODE" ), computeGroup, LightApp_Preferences::Selector, "SMESH", "show_result_notification" );
  modes.clear();
  modes.append( tr( "PREF_NOTIFY_NEVER" ) );
  modes.append( tr( "PREF_NOTIFY_ERROR" ) );
  modes.append( tr( "PREF_NOTIFY_ALWAYS" ) );
  indices.clear();
  indices.append( 0 );
  indices.append( 1 );
  indices.append( 2 );
  setPreferenceProperty( notifyMode, "strings", modes );
  setPreferenceProperty( notifyMode, "indexes", indices );

  int infoGroup = addPreference( tr( "PREF_GROUP_INFO" ), genTab );
  setPreferenceProperty( infoGroup, "columns", 4 );
  int elemInfo = addPreference( tr( "PREF_ELEM_INFO" ), infoGroup, LightApp_Preferences::Selector, "SMESH", "mesh_elem_info" );
  modes.clear();
  modes.append( tr( "PREF_ELEM_INFO_SIMPLE" ) );
  modes.append( tr( "PREF_ELEM_INFO_TREE" ) );
  indices.clear();
  indices.append( 0 );
  indices.append( 1 );
  setPreferenceProperty( elemInfo, "strings", modes );
  setPreferenceProperty( elemInfo, "indexes", indices );
  int nodesLim = addPreference( tr( "PREF_GPP_NODES_LIMIT" ), infoGroup, LightApp_Preferences::IntSpin, "SMESH", "info_groups_nodes_limit" );
  setPreferenceProperty( nodesLim, "min", 0 );
  setPreferenceProperty( nodesLim, "max", 10000000 );
  setPreferenceProperty( nodesLim, "step", 10000 );
  setPreferenceProperty( nodesLim, "special", tr( "PREF_UPDATE_LIMIT_NOLIMIT" ) );

  int segGroup = addPreference( tr( "PREF_GROUP_SEGMENT_LENGTH" ), genTab );
  setPreferenceProperty( segGroup, "columns", 2 );
  int segLen = addPreference( tr( "PREF_SEGMENT_LENGTH" ), segGroup, LightApp_Preferences::IntSpin,
                              "SMESH", "segmentation" );
  setPreferenceProperty( segLen, "min", 1 );
  setPreferenceProperty( segLen, "max", 10000000 );
  int nbSeg = addPreference( tr( "PREF_NB_SEGMENTS" ), segGroup, LightApp_Preferences::IntSpin,
                             "SMESH", "nb_segments_per_edge" );
  setPreferenceProperty( nbSeg, "min", 1 );
  setPreferenceProperty( nbSeg, "max", 10000000 );

  int loadGroup = addPreference( tr( "SMESH_PREF_MESH_LOADING" ), genTab );
  addPreference( tr( "PREF_FORGET_MESH_AT_HYP_MODIF" ), loadGroup, LightApp_Preferences::Bool,
                 "SMESH", "forget_mesh_on_hyp_modif" );


  // Quantities with individual precision settings
  int precGroup = addPreference( tr( "SMESH_PREF_GROUP_PRECISION" ), genTab );
  setPreferenceProperty( precGroup, "columns", 2 );

  const int nbQuantities = 6;
  int precs[nbQuantities], ii = 0;
  precs[ii++] = addPreference( tr( "SMESH_PREF_length_precision" ), precGroup,
                            LightApp_Preferences::IntSpin, "SMESH", "length_precision" );
  precs[ii++] = addPreference( tr( "SMESH_PREF_angle_precision" ), precGroup,
                            LightApp_Preferences::IntSpin, "SMESH", "angle_precision" );
  precs[ii++] = addPreference( tr( "SMESH_PREF_len_tol_precision" ), precGroup,
                            LightApp_Preferences::IntSpin, "SMESH", "len_tol_precision" );
  precs[ii++] = addPreference( tr( "SMESH_PREF_parametric_precision" ), precGroup,
                            LightApp_Preferences::IntSpin, "SMESH", "parametric_precision" );
  precs[ii++] = addPreference( tr( "SMESH_PREF_area_precision" ), precGroup,
                            LightApp_Preferences::IntSpin, "SMESH", "area_precision" );
  precs[ii  ] = addPreference( tr( "SMESH_PREF_vol_precision" ), precGroup,
                            LightApp_Preferences::IntSpin, "SMESH", "vol_precision" );

  // Set property for precision value for spinboxes
  for ( ii = 0; ii < nbQuantities; ii++ ){
    setPreferenceProperty( precs[ii], "min", -14 );
    setPreferenceProperty( precs[ii], "max", 14 );
    setPreferenceProperty( precs[ii], "precision", 2 );
  }

  int previewGroup = addPreference( tr( "SMESH_PREF_GROUP_PREVIEW" ), genTab );
  setPreferenceProperty( previewGroup, "columns", 2 );
  int chunkSize = addPreference( tr( "PREF_PREVIEW_CHUNK_SIZE" ), previewGroup, LightApp_Preferences::IntSpin, "SMESH", "preview_actor_chunk_size" );
  setPreferenceProperty( chunkSize, "min",  0 );
  setPreferenceProperty( chunkSize, "max",  1000 );
  setPreferenceProperty( chunkSize, "step", 50 );

  int pyDumpGroup = addPreference( tr( "PREF_PYTHON_DUMP" ), genTab );
  addPreference( tr( "PREF_HISTORICAL_PYTHON_DUMP" ), pyDumpGroup, LightApp_Preferences::Bool, "SMESH", "historical_python_dump" );

  // Mesh tab ------------------------------------------------------------------------
  int meshTab = addPreference( tr( "PREF_TAB_MESH" ) );
  int nodeGroup = addPreference( tr( "PREF_GROUP_NODES" ), meshTab );
  setPreferenceProperty( nodeGroup, "columns", 3 );

  addPreference( tr( "PREF_COLOR" ), nodeGroup, LightApp_Preferences::Color, "SMESH", "node_color" );

  int typeOfMarker = addPreference( tr( "PREF_TYPE_OF_MARKER" ), nodeGroup, LightApp_Preferences::Selector, "SMESH", "type_of_marker" );

  SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr(this);
  QList<QVariant> aMarkerTypeIndicesList;
  QList<QVariant> aMarkerTypeIconsList;
  for ( int i = VTK::MT_POINT; i < VTK::MT_USER; i++ ) {
    QString icoFile = QString( "ICON_VERTEX_MARKER_%1" ).arg( i );
    QPixmap pixmap = aResourceMgr->loadPixmap( "VTKViewer", tr( qPrintable( icoFile ) ) );
    aMarkerTypeIndicesList << i;
    aMarkerTypeIconsList << pixmap;
  }
  setPreferenceProperty( typeOfMarker, "indexes", aMarkerTypeIndicesList );
  setPreferenceProperty( typeOfMarker, "icons",   aMarkerTypeIconsList );

  int markerScale = addPreference( tr( "PREF_MARKER_SCALE" ), nodeGroup, LightApp_Preferences::Selector, "SMESH", "marker_scale" );

  QList<QVariant> aMarkerScaleIndicesList;
  QStringList     aMarkerScaleValuesList;
  for ( int i = VTK::MS_10; i <= VTK::MS_70; i++ ) {
    aMarkerScaleIndicesList << i;
    aMarkerScaleValuesList  << QString::number( (i-(int)VTK::MS_10)*0.5 + 1.0 );
  }
  setPreferenceProperty( markerScale, "strings", aMarkerScaleValuesList );
  setPreferenceProperty( markerScale, "indexes", aMarkerScaleIndicesList );

  int elemGroup = addPreference( tr( "PREF_GROUP_ELEMENTS" ), meshTab );
  setPreferenceProperty( elemGroup, "columns", 2 );

  int ColorId = addPreference( tr( "PREF_FILL"     ), elemGroup, LightApp_Preferences::BiColor, "SMESH", "fill_color" );
  addPreference( tr( "PREF_COLOR_0D" ), elemGroup, LightApp_Preferences::Color, "SMESH", "elem0d_color" );

  addPreference( tr( "PREF_OUTLINE"  ), elemGroup, LightApp_Preferences::Color, "SMESH", "outline_color" );
  addPreference( tr( "PREF_WIREFRAME"  ), elemGroup, LightApp_Preferences::Color, "SMESH", "wireframe_color" );

  setPreferenceProperty( ColorId, "text", tr("PREF_BACKFACE") );

  int grpGroup = addPreference( tr( "PREF_GROUP_GROUPS" ), meshTab );
  setPreferenceProperty( grpGroup, "columns", 2 );

  addPreference( tr( "PREF_GRP_NAMES" ), grpGroup, LightApp_Preferences::Color, "SMESH", "group_name_color" );

  int size0d = addPreference(tr("PREF_SIZE_0D"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "elem0d_size");
  int elemW  = addPreference(tr("PREF_WIDTH"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "element_width");
  int shrink = addPreference(tr("PREF_SHRINK_COEFF"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "shrink_coeff");

  setPreferenceProperty( size0d, "min", 1 );
  setPreferenceProperty( size0d, "max", 10 );

  setPreferenceProperty( elemW, "min", 1 );
  setPreferenceProperty( elemW, "max", 5 );

  setPreferenceProperty( shrink, "min", 0 );
  setPreferenceProperty( shrink, "max", 100 );

  int orientGroup = addPreference( tr( "PREF_GROUP_FACES_ORIENTATION" ), meshTab );
  setPreferenceProperty( orientGroup, "columns", 1 );

  addPreference( tr( "PREF_ORIENTATION_COLOR" ), orientGroup, LightApp_Preferences::Color, "SMESH", "orientation_color" );
  int orientScale = addPreference( tr( "PREF_ORIENTATION_SCALE" ), orientGroup, LightApp_Preferences::DblSpin, "SMESH", "orientation_scale" );

  setPreferenceProperty( orientScale, "min", 0.05 );
  setPreferenceProperty( orientScale, "max", 0.5 );
  setPreferenceProperty( orientScale, "step", 0.05 );

  addPreference( tr( "PREF_ORIENTATION_3D_VECTORS" ), orientGroup, LightApp_Preferences::Bool, "SMESH", "orientation_3d_vectors" );

  // Selection tab ------------------------------------------------------------------------
  int selTab = addPreference( tr( "PREF_TAB_SELECTION" ) );

  int selGroup = addPreference( tr( "PREF_GROUP_SELECTION" ), selTab );
  setPreferenceProperty( selGroup, "columns", 2 );

  addPreference( tr( "PREF_OBJECT_COLOR" ), selGroup, LightApp_Preferences::Color, "SMESH", "selection_object_color" );
  addPreference( tr( "PREF_ELEMENT_COLOR" ), selGroup, LightApp_Preferences::Color, "SMESH", "selection_element_color" );

  int preGroup = addPreference( tr( "PREF_GROUP_PRESELECTION" ), selTab );
  setPreferenceProperty( preGroup, "columns", 2 );

  addPreference( tr( "PREF_HIGHLIGHT_COLOR" ), preGroup, LightApp_Preferences::Color, "SMESH", "highlight_color" );

  int precSelGroup = addPreference( tr( "PREF_GROUP_PRECISION" ), selTab );
  setPreferenceProperty( precSelGroup, "columns", 2 );

  addPreference( tr( "PREF_NODES" ), precSelGroup, LightApp_Preferences::Double, "SMESH", "selection_precision_node" );
  addPreference( tr( "PREF_ELEMENTS" ), precSelGroup, LightApp_Preferences::Double, "SMESH", "selection_precision_element" );
  addPreference( tr( "PREF_OBJECTS" ), precSelGroup, LightApp_Preferences::Double, "SMESH", "selection_precision_object" );

  // Scalar Bar tab ------------------------------------------------------------------------
  int sbarTab = addPreference( tr( "SMESH_SCALARBAR" ) );
  int fontGr = addPreference( tr( "SMESH_FONT_SCALARBAR" ), sbarTab );
  setPreferenceProperty( fontGr, "columns", 2 );

  addVtkFontPref( tr( "SMESH_TITLE" ), fontGr, "scalar_bar_title_font" );
  addPreference( tr( "PREF_TITLE_COLOR" ), fontGr, LightApp_Preferences::Color, "SMESH", "scalar_bar_title_color" );

  addVtkFontPref( tr( "SMESH_LABELS" ), fontGr, "scalar_bar_label_font" );
  addPreference( tr( "PREF_LABELS_COLOR" ), fontGr, LightApp_Preferences::Color, "SMESH", "scalar_bar_label_color" );

  int colorsLabelsGr = addPreference( tr( "SMESH_LABELS_COLORS_SCALARBAR" ), sbarTab );
  setPreferenceProperty( colorsLabelsGr, "columns", 2 );

  int numcol = addPreference( tr( "SMESH_NUMBEROFCOLORS" ), colorsLabelsGr, LightApp_Preferences::IntSpin, "SMESH", "scalar_bar_num_colors" );
  setPreferenceProperty( numcol, "min", 2 );
  setPreferenceProperty( numcol, "max", 256 );

  int numlab = addPreference( tr( "SMESH_NUMBEROFLABELS" ), colorsLabelsGr, LightApp_Preferences::IntSpin, "SMESH", "scalar_bar_num_labels" );
  setPreferenceProperty( numlab, "min", 2 );
  setPreferenceProperty( numlab, "max", 65 );

  int orientGr = addPreference( tr( "SMESH_ORIENTATION" ), sbarTab );
  setPreferenceProperty( orientGr, "columns", 2 );
  int orient = addPreference( tr( "SMESH_ORIENTATION" ), orientGr, LightApp_Preferences::Selector, "SMESH", "scalar_bar_orientation" );
  QStringList orients;
  orients.append( tr( "SMESH_VERTICAL" ) );
  orients.append( tr( "SMESH_HORIZONTAL" ) );
  indices.clear(); indices.append( 0 ); indices.append( 1 );
  setPreferenceProperty( orient, "strings", orients );
  setPreferenceProperty( orient, "indexes", indices );

  int posVSizeGr = addPreference( tr( "SMESH_POSITION_SIZE_SCALARBAR" ) + " " + tr( "SMESH_VERTICAL" ), sbarTab );
  setPreferenceProperty( posVSizeGr, "columns", 2 );
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
  setPreferenceProperty( posHSizeGr, "columns", 2 );
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

  int distributionGr = addPreference( tr( "SMESH_DISTRIBUTION_SCALARBAR" ), sbarTab, LightApp_Preferences::Auto, "SMESH", "distribution_visibility" );
  int coloringType = addPreference( tr( "SMESH_DISTRIBUTION_COLORING_TYPE" ), distributionGr, LightApp_Preferences::Selector, "SMESH", "distribution_coloring_type" );
  setPreferenceProperty( distributionGr, "columns", 3 );
  QStringList types;
  types.append( tr( "SMESH_MONOCOLOR" ) );
  types.append( tr( "SMESH_MULTICOLOR" ) );
  indices.clear(); indices.append( 0 ); indices.append( 1 );
  setPreferenceProperty( coloringType, "strings", types );
  setPreferenceProperty( coloringType, "indexes", indices );
  addPreference( tr( "SMESH_DISTRIBUTION_COLOR" ), distributionGr, LightApp_Preferences::Color, "SMESH", "distribution_color" );

}

void SMESHGUI::preferencesChanged( const QString& sect, const QString& name )
{
  if( sect=="SMESH" ) {
    float sbX1,sbY1,sbW,sbH;
    float aTol = 1.00000009999999;
    std::string aWarning;
    SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr(this);
    if( name=="selection_object_color" || name=="selection_element_color" ||
        name=="highlight_color" ||
        name=="selection_precision_node" || name=="selection_precision_element" ||
        name=="selection_precision_object")
      SMESH::UpdateSelectionProp( this );
    else if (name == QString("scalar_bar_vertical_x") || name == QString("scalar_bar_vertical_width")){
      sbX1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_x", sbX1);
      sbW = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_width", sbW);
      if(sbX1+sbW > aTol){
        aWarning = "Origin and Size Vertical: X+Width > 1\n";
        sbX1=0.01;
        sbW=0.08;
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_x", sbX1);
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_width", sbW);
      }
    }
    else if(name == QString("scalar_bar_vertical_y") || name == QString("scalar_bar_vertical_height")){
      sbY1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_y", sbY1);
      sbH = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_height",sbH);
      if(sbY1+sbH > aTol){
        aWarning = "Origin and Size Vertical: Y+Height > 1\n";
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_y", sbY1);
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_height",sbH);
      }
    }
    else if(name ==  QString("scalar_bar_horizontal_x") || name ==  QString("scalar_bar_horizontal_width")){
      sbX1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_x", sbX1);
      sbW = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_width", sbW);
      if(sbX1+sbW > aTol){
        aWarning = "Origin and Size Horizontal: X+Width > 1\n";
        sbX1=0.1;
        sbW=0.08;
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
        sbH=0.08;
        aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_y", sbY1);
        aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_height",sbH);
      }
    }
    else if ( name == "segmentation" ) {
      int nbSeg = aResourceMgr->integerValue( "SMESH", "segmentation", 10 );
      myComponentSMESH->SetBoundaryBoxSegmentation( nbSeg );
    }
    else if ( name == "nb_segments_per_edge" ) {
      int nbSeg = aResourceMgr->integerValue( "SMESH", "nb_segments_per_edge", 15 );
      myComponentSMESH->SetDefaultNbSegments( nbSeg );
    }
    else if ( name == "historical_python_dump" ||
              name == "forget_mesh_on_hyp_modif") {
      QString val = aResourceMgr->stringValue( "SMESH", name );
      myComponentSMESH->SetOption( name.toLatin1().constData(), val.toLatin1().constData() );
    }

    if(aWarning.size() != 0){
      aWarning += "The default values are applied instead.";
      SUIT_MessageBox::warning(SMESHGUI::desktop(),
                               QObject::tr("SMESH_ERR_SCALARBAR_PARAMS"),
                               QObject::tr(aWarning.c_str()));
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
  if ( (flags & UF_Viewer) | (flags & UF_Forced) )
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
    case 417: //convert to quadratic
      op = new SMESHGUI_ConvToQuadOp();
    break;
    case 418: // create 2D mesh as boundary on 3D
      op = new SMESHGUI_Make2DFrom3DOp();
    break;
    case 701: // Compute mesh
      op = new SMESHGUI_ComputeOp();
    break;
    case 702: // Create mesh
      op = new SMESHGUI_MeshOp( true, true );
    break;
    case 703: // Create sub-mesh
      op = new SMESHGUI_MeshOp( true, false );
    break;
    case 704: // Edit mesh/sub-mesh
      op = new SMESHGUI_MeshOp( false );
    break;
    case 711: // Precompute mesh
      op = new SMESHGUI_PrecomputeOp();
    break;
    case 712: // Evaluate mesh
      op = new SMESHGUI_EvaluateOp();
    break;
    case 713: // Evaluate mesh
      op = new SMESHGUI_MeshOrderOp();
    break;
    case 806: // Create group on geom
      op = new SMESHGUI_GroupOnShapeOp();
      break;
    case 904: // Find element
      op = new SMESHGUI_FindElemByPointOp();
      break;
    case 4067: // make mesh pass through point
      op = new SMESHGUI_MakeNodeAtPointOp();
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

SALOMEDS::Color SMESHGUI::getUniqueColor( const QList<SALOMEDS::Color>& theReservedColors )
{
  int aHue = -1;
  int aTolerance = 64;
  int anIterations = 0;
  int aPeriod = 5;

  while( 1 )
  {
    anIterations++;
    if( anIterations % aPeriod == 0 )
    {
      aTolerance /= 2;
      if( aTolerance < 1 )
        break;
    }

    aHue = (int)( 360.0 * rand() / RAND_MAX );

    bool ok = true;
    QList<SALOMEDS::Color>::const_iterator it = theReservedColors.constBegin();
    QList<SALOMEDS::Color>::const_iterator itEnd = theReservedColors.constEnd();
    for( ; it != itEnd; ++it )
    {
      SALOMEDS::Color anAutoColor = *it;
      QColor aQColor( (int)( anAutoColor.R * 255.0 ), (int)( anAutoColor.G * 255.0 ), (int)( anAutoColor.B * 255.0 ) );

      int h, s, v;
      aQColor.getHsv( &h, &s, &v );
      if( abs( h - aHue ) < aTolerance )
      {
        ok = false;
        break;
      }
    }

    if( ok )
      break;
  }

  QColor aColor;
  aColor.setHsv( aHue, 255, 255 );

  SALOMEDS::Color aSColor;
  aSColor.R = (double)aColor.red() / 255.0;
  aSColor.G = (double)aColor.green() / 255.0;
  aSColor.B = (double)aColor.blue() / 255.0;

  return aSColor;
}

const char gSeparator = '_'; // character used to separate parameter names
const char gDigitsSep = ':'; // character used to separate numeric parameter values (color = r:g:b)
const char gPathSep   = '|'; // character used to separate paths

/*!
 * \brief Store visual parameters
 *
 * This method is called just before the study document is saved.
 * Store visual parameters in AttributeParameter attribue(s)
 */
void SMESHGUI::storeVisualParameters (int savePoint)
{
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>(application()->activeStudy());
  if (!appStudy || !appStudy->studyDS())
    return;
  _PTR(Study) studyDS = appStudy->studyDS();

  // componentName is used for encoding of entries when storing them in IParameters
  std::string componentName = myComponentSMESH->ComponentDataType();
  //_PTR(SComponent) aSComponent = studyDS->FindComponent("SMESH");
  //if (!aSComponent) return;

  // IParameters
  _PTR(AttributeParameter) ap = studyDS->GetModuleParameters("Interface Applicative",
                                                             componentName.c_str(),
                                                             savePoint);
  _PTR(IParameters) ip = ClientFactory::getIParameters(ap);

  // store map of custom markers
  const VTK::MarkerMap& aMarkerMap = myMarkerMap[ studyDS->StudyId() ];
  if( !aMarkerMap.empty() )
  {
    VTK::MarkerMap::const_iterator anIter = aMarkerMap.begin();
    for( ; anIter != aMarkerMap.end(); anIter++ )
    {
      int anId = anIter->first;
      VTK::MarkerData aMarkerData = anIter->second;
      std::string aMarkerFileName = aMarkerData.first;
      VTK::MarkerTexture aMarkerTexture = aMarkerData.second;
      if( aMarkerTexture.size() < 3 )
        continue; // should contain at least width, height and the first value

      QString aPropertyName( "texture" );
      aPropertyName += gSeparator;
      aPropertyName += QString::number( anId );

      QString aPropertyValue = aMarkerFileName.c_str();
      aPropertyValue += gPathSep;

      VTK::MarkerTexture::const_iterator aTextureIter = aMarkerTexture.begin();
      ushort aWidth = *aTextureIter++;
      ushort aHeight = *aTextureIter++;
      aPropertyValue += QString::number( aWidth ); aPropertyValue += gDigitsSep;
      aPropertyValue += QString::number( aHeight ); aPropertyValue += gDigitsSep;
      for( ; aTextureIter != aMarkerTexture.end(); aTextureIter++ )
        aPropertyValue += QString::number( *aTextureIter );

      ip->setProperty( aPropertyName.toStdString(), aPropertyValue.toStdString() );
    }
  }

  // viewers counters are used for storing view_numbers in IParameters
  int vtkViewers = 0;

  // main cycle to store parameters of displayed objects
  QList<SUIT_ViewManager*> lst;
  QList<SUIT_ViewManager*>::Iterator it;
  getApp()->viewManagers(lst);
  for (it = lst.begin(); it != lst.end(); it++)
  {
    SUIT_ViewManager* vman = *it;
    QString vType = vman->getType();

    // saving VTK actors properties
    if (vType == SVTK_Viewer::Type())
    {
      // store the clipping planes attached to the view manager
      SMESHGUI_ClippingPlaneInfoList aClippingPlaneInfoList;
      SMESHGUI_ClippingPlaneInfoMap::const_iterator anIter = myClippingPlaneInfoMap.find( vman );
      if( anIter != myClippingPlaneInfoMap.end() )
        aClippingPlaneInfoList = anIter->second;

      if( !aClippingPlaneInfoList.empty() ) {
        SMESHGUI_ClippingPlaneInfoList::const_iterator anIter = aClippingPlaneInfoList.begin();
        for( int anId = 0; anIter != aClippingPlaneInfoList.end(); anIter++, anId++ )
        {
          const SMESH::ClippingPlaneInfo& aClippingPlaneInfo = *anIter;
          SMESH::OrientedPlane* aPlane = aClippingPlaneInfo.Plane;

          QString aPropertyName( "ClippingPlane" );
          aPropertyName += gSeparator;
          aPropertyName += QString::number( vtkViewers );
          aPropertyName += gSeparator;
          aPropertyName += QString::number( anId );

          QString aPropertyValue = QString::number( (int)aPlane->GetOrientation() ).toLatin1().constData();
          aPropertyValue += gDigitsSep;
          aPropertyValue += QString::number( aPlane->GetDistance() ).toLatin1().constData();
          aPropertyValue += gDigitsSep;
          aPropertyValue += QString::number( aPlane->myAngle[0] ).toLatin1().constData();
          aPropertyValue += gDigitsSep;
          aPropertyValue += QString::number( aPlane->myAngle[1] ).toLatin1().constData();

          ip->setProperty( aPropertyName.toStdString(), aPropertyValue.toStdString() );
        }
      }

      QVector<SUIT_ViewWindow*> views = vman->getViews();
      for (int i = 0, iEnd = vman->getViewsCount(); i < iEnd; i++)
      {
        if (SVTK_ViewWindow* vtkView = dynamic_cast<SVTK_ViewWindow*>(views[i]))
        {
          VTK::ActorCollectionCopy aCopy(vtkView->getRenderer()->GetActors());
          vtkActorCollection* allActors = aCopy.GetActors();
          allActors->InitTraversal();
          while (vtkActor* actor = allActors->GetNextActor())
          {
            if (actor->GetVisibility()) // store only visible actors
            {
              SMESH_Actor* aSmeshActor = 0;
              if (actor->IsA("SMESH_Actor"))
                aSmeshActor = SMESH_Actor::SafeDownCast(actor);
              if (aSmeshActor && aSmeshActor->hasIO())
              {
                Handle(SALOME_InteractiveObject) io = aSmeshActor->getIO();
                if (io->hasEntry())
                {
                  // entry is "encoded" = it does NOT contain component adress,
                  // since it is a subject to change on next component loading
                  std::string entry = ip->encodeEntry(io->getEntry(), componentName);

                  std::string param, vtkParam = vType.toLatin1().data();
                  vtkParam += gSeparator;
                  vtkParam += QString::number(vtkViewers).toLatin1().data();
                  vtkParam += gSeparator;

                  // Visibility
                  param = vtkParam + "Visibility";
                  ip->setParameter(entry, param, "On");

                  // Representation
                  param = vtkParam + "Representation";
                  ip->setParameter(entry, param, QString::number
                                   ((int)aSmeshActor->GetRepresentation()).toLatin1().data());

                  // IsShrunk
                  param = vtkParam + "IsShrunk";
                  ip->setParameter(entry, param, QString::number
                                   ((int)aSmeshActor->IsShrunk()).toLatin1().data());

                  // Displayed entities
                  unsigned int aMode = aSmeshActor->GetEntityMode();
                  bool isE = aMode & SMESH_Actor::eEdges;
                  bool isF = aMode & SMESH_Actor::eFaces;
                  bool isV = aMode & SMESH_Actor::eVolumes;

                  QString modeStr ("e");
                  modeStr += gDigitsSep; modeStr += QString::number(isE);
                  modeStr += gDigitsSep; modeStr += "f";
                  modeStr += gDigitsSep; modeStr += QString::number(isF);
                  modeStr += gDigitsSep; modeStr += "v";
                  modeStr += gDigitsSep; modeStr += QString::number(isV);

                  param = vtkParam + "Entities";
                  ip->setParameter(entry, param, modeStr.toLatin1().data());

                  // Colors (surface:edge:)
                  vtkFloatingPointType r, g, b;
                  int delta;

                  aSmeshActor->GetSufaceColor(r, g, b, delta);
                  QString colorStr ("surface");
                  colorStr += gDigitsSep; colorStr += QString::number(r);
                  colorStr += gDigitsSep; colorStr += QString::number(g);
                  colorStr += gDigitsSep; colorStr += QString::number(b);

                  colorStr += gDigitsSep; colorStr += "backsurface";
                  colorStr += gDigitsSep; colorStr += QString::number(delta);


                  aSmeshActor->GetEdgeColor(r, g, b);
                  colorStr += gDigitsSep; colorStr += "edge";
                  colorStr += gDigitsSep; colorStr += QString::number(r);
                  colorStr += gDigitsSep; colorStr += QString::number(g);
                  colorStr += gDigitsSep; colorStr += QString::number(b);

                  aSmeshActor->GetNodeColor(r, g, b);
                  colorStr += gDigitsSep; colorStr += "node";
                  colorStr += gDigitsSep; colorStr += QString::number(r);
                  colorStr += gDigitsSep; colorStr += QString::number(g);
                  colorStr += gDigitsSep; colorStr += QString::number(b);

                  aSmeshActor->GetOutlineColor(r, g, b);
                  colorStr += gDigitsSep; colorStr += "outline";
                  colorStr += gDigitsSep; colorStr += QString::number(r);
                  colorStr += gDigitsSep; colorStr += QString::number(g);
                  colorStr += gDigitsSep; colorStr += QString::number(b);

                  param = vtkParam + "Colors";
                  ip->setParameter(entry, param, colorStr.toLatin1().data());

                  // Sizes of lines and points
                  QString sizeStr ("line");
                  sizeStr += gDigitsSep; sizeStr += QString::number((int)aSmeshActor->GetLineWidth());
                  sizeStr += gDigitsSep; sizeStr += "shrink";
                  sizeStr += gDigitsSep; sizeStr += QString::number(aSmeshActor->GetShrinkFactor());

                  param = vtkParam + "Sizes";
                  ip->setParameter(entry, param, sizeStr.toLatin1().data());

                  // Point marker
                  QString markerStr;

                  VTK::MarkerType aMarkerType = aSmeshActor->GetMarkerType();
                  if( aMarkerType == VTK::MT_USER ) {
                    markerStr += "custom";
                    markerStr += gDigitsSep;
                    markerStr += QString::number( aSmeshActor->GetMarkerTexture() );
                  }
                  else {
                    markerStr += "std";
                    markerStr += gDigitsSep;
                    markerStr += QString::number( (int)aMarkerType );
                    markerStr += gDigitsSep;
                    markerStr += QString::number( (int)aSmeshActor->GetMarkerScale() );
                  }

                  param = vtkParam + "PointMarker";
                  ip->setParameter(entry, param, markerStr.toLatin1().data());

                  // Opacity
                  param = vtkParam + "Opacity";
                  ip->setParameter(entry, param,
                                   QString::number(aSmeshActor->GetOpacity()).toLatin1().data());

                  // Clipping
                  param = vtkParam + "ClippingPlane";
                  int aPlaneId = 0;
                  if( !aClippingPlaneInfoList.empty() ) {
                    SMESHGUI_ClippingPlaneInfoList::const_iterator anIter1 = aClippingPlaneInfoList.begin();
                    for( int anId = 0; anIter1 != aClippingPlaneInfoList.end(); anIter1++, anId++ )
                    {
                      const SMESH::ClippingPlaneInfo& aClippingPlaneInfo = *anIter1;
                      std::list<vtkActor*> anActorList = aClippingPlaneInfo.ActorList;
                      SMESH::TActorList::iterator anIter2 = anActorList.begin();
                      for ( ; anIter2 != anActorList.end(); anIter2++ ) {
                        if( aSmeshActor == *anIter2 ) {
                          ip->setParameter( entry, param + QString::number( ++aPlaneId ).toLatin1().constData(),
                                            QString::number( anId ).toLatin1().constData() );
                          break;
                        }
                      }
                    }
                  }
                  if( aPlaneId == 0 )
                    ip->setParameter( entry, param, "Off" );
                } // if (io->hasEntry())
              } // SMESH_Actor && hasIO
            } // isVisible
          } // while.. actors traversal
        } // if (vtkView)
      } // for (views)
      vtkViewers++;
    } // if (SVTK view model)
  } // for (viewManagers)
}

// data structures for clipping planes processing
typedef struct {
  int Id;
  vtkIdType Orientation;
  vtkFloatingPointType Distance;
  vtkFloatingPointType Angle[2];
} TPlaneData;
typedef std::list<TPlaneData>         TPlaneDataList;
typedef std::map<int, TPlaneDataList> TPlaneDataMap;

typedef std::list<vtkActor*>          TActorList;
typedef struct {
  int PlaneId;
  TActorList ActorList;
  SUIT_ViewManager* ViewManager;
} TPlaneInfo;
typedef std::list<TPlaneInfo>         TPlaneInfoList;
typedef std::map<int, TPlaneInfoList> TPlaneInfoMap;

/*!
 * \brief Restore visual parameters
 *
 * This method is called after the study document is opened.
 * Restore visual parameters from AttributeParameter attribue(s)
 */
void SMESHGUI::restoreVisualParameters (int savePoint)
{
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>(application()->activeStudy());
  if (!appStudy || !appStudy->studyDS())
    return;
  _PTR(Study) studyDS = appStudy->studyDS();

  // componentName is used for encoding of entries when storing them in IParameters
  std::string componentName = myComponentSMESH->ComponentDataType();
  //_PTR(SComponent) aSComponent = studyDS->FindComponent("GEOM");
  //if (!aSComponent) return;

  // IParameters
  _PTR(AttributeParameter) ap = studyDS->GetModuleParameters("Interface Applicative",
                                                             componentName.c_str(),
                                                             savePoint);
  _PTR(IParameters) ip = ClientFactory::getIParameters(ap);

  // restore map of custom markers and map of clipping planes
  VTK::MarkerMap& aMarkerMap = myMarkerMap[ studyDS->StudyId() ];
  TPlaneDataMap aPlaneDataMap;

  std::vector<std::string> properties = ip->getProperties();
  for (std::vector<std::string>::iterator propIt = properties.begin(); propIt != properties.end(); ++propIt)
  {
    std::string property = *propIt;
    QString aPropertyName( property.c_str() );
    QString aPropertyValue( ip->getProperty( property ).c_str() );

    QStringList aPropertyNameList = aPropertyName.split( gSeparator, QString::SkipEmptyParts );
    if( aPropertyNameList.isEmpty() )
      continue;

    QString aPropertyType = aPropertyNameList[0];
    if( aPropertyType == "texture" )
    {
      if( aPropertyNameList.size() != 2 )
        continue;

      bool ok = false;
      int anId = aPropertyNameList[1].toInt( &ok );
      if( !ok || anId < 1 )
        continue;

      QStringList aPropertyValueList = aPropertyValue.split( gPathSep, QString::SkipEmptyParts );
      if( aPropertyValueList.size() != 2 )
        continue;

      std::string aMarkerFileName = aPropertyValueList[0].toStdString();
      QString aMarkerTextureString = aPropertyValueList[1];
      QStringList aMarkerTextureStringList = aMarkerTextureString.split( gDigitsSep, QString::SkipEmptyParts );
      if( aMarkerTextureStringList.size() != 3 )
        continue;

      ok = false;
      ushort aWidth = aMarkerTextureStringList[0].toUShort( &ok );
      if( !ok )
        continue;

      ok = false;
      ushort aHeight = aMarkerTextureStringList[1].toUShort( &ok );
      if( !ok )
        continue;

      VTK::MarkerTexture aMarkerTexture;
      aMarkerTexture.push_back( aWidth );
      aMarkerTexture.push_back( aHeight );

      QString aMarkerTextureData = aMarkerTextureStringList[2];
      for( int i = 0, n = aMarkerTextureData.length(); i < n; i++ )
      {
        QChar aChar = aMarkerTextureData.at( i );
        if( aChar.isDigit() )
          aMarkerTexture.push_back( aChar.digitValue() );
      }

      aMarkerMap[ anId ] = VTK::MarkerData( aMarkerFileName, aMarkerTexture );
    }
    else if( aPropertyType == "ClippingPlane" )
    {
      if( aPropertyNameList.size() != 3 )
        continue;

      bool ok = false;
      int aViewId = aPropertyNameList[1].toInt( &ok );
      if( !ok || aViewId < 0 )
        continue;

      ok = false;
      int aClippingPlaneId = aPropertyNameList[2].toInt( &ok );
      if( !ok || aClippingPlaneId < 0 )
        continue;

      QStringList aPropertyValueList = aPropertyValue.split( gDigitsSep, QString::SkipEmptyParts );
      if( aPropertyValueList.size() != 4 )
        continue;

      TPlaneData aPlaneData;
      aPlaneData.Id = aClippingPlaneId;

      ok = false;
      aPlaneData.Orientation = aPropertyValueList[0].toInt( &ok );
      if( !ok )
        continue;

      ok = false;
      aPlaneData.Distance = aPropertyValueList[1].toDouble( &ok );
      if( !ok )
        continue;

      ok = false;
      aPlaneData.Angle[0] = aPropertyValueList[2].toDouble( &ok );
      if( !ok )
        continue;

      ok = false;
      aPlaneData.Angle[1] = aPropertyValueList[3].toDouble( &ok );
      if( !ok )
        continue;

      TPlaneDataList& aPlaneDataList = aPlaneDataMap[ aViewId ];
      aPlaneDataList.push_back( aPlaneData );
    }
  }

  TPlaneInfoMap aPlaneInfoMap;

  std::vector<std::string> entries = ip->getEntries();

  for (std::vector<std::string>::iterator entIt = entries.begin(); entIt != entries.end(); ++entIt)
  {
    // entry is a normal entry - it should be "decoded" (setting base adress of component)
    QString entry (ip->decodeEntry(*entIt).c_str());

    // Check that the entry corresponds to a real object in the Study
    // as the object may be deleted or modified after the visual state is saved.
    _PTR(SObject) so = studyDS->FindObjectID(entry.toLatin1().data());
    if (!so) continue; //Skip the not existent entry

    std::vector<std::string> paramNames = ip->getAllParameterNames( *entIt );
    std::vector<std::string> paramValues = ip->getAllParameterValues( *entIt );

    std::vector<std::string>::iterator namesIt = paramNames.begin();
    std::vector<std::string>::iterator valuesIt = paramValues.begin();

    // actors are stored in a map after displaying of them for
    // quicker access in the future: map < viewID to actor >
    NCollection_DataMap<int, SMESH_Actor*> vtkActors;

    for (; namesIt != paramNames.end(); ++namesIt, ++valuesIt)
    {
      // visual parameters are stored in strings as follows: ViewerType_ViewIndex_ParamName.
      // '_' is used as separator and should not be used in viewer type or parameter names.
      QStringList lst = QString((*namesIt).c_str()).split(gSeparator, QString::SkipEmptyParts);
      if (lst.size() != 3)
        continue;

      QString viewerTypStr = lst[0];
      QString viewIndexStr = lst[1];
      QString paramNameStr = lst[2];

      bool ok;
      int viewIndex = viewIndexStr.toUInt(&ok);
      if (!ok) // bad conversion of view index to integer
        continue;

      // viewers
      if (viewerTypStr == SVTK_Viewer::Type())
      {
        SMESH_Actor* aSmeshActor = 0;
        if (vtkActors.IsBound(viewIndex))
          aSmeshActor = vtkActors.Find(viewIndex);

        QList<SUIT_ViewManager*> lst;
        getApp()->viewManagers(viewerTypStr, lst);

        // SVTK ViewManager always has 1 ViewWindow, so view index is index of view manager
        SUIT_ViewManager* vman = NULL;
        if (viewIndex >= 0 && viewIndex < lst.count())
          vman = lst.at(viewIndex);

        if (paramNameStr == "Visibility")
        {
          if (!aSmeshActor && displayer() && vman)
          {
            SUIT_ViewModel* vmodel = vman->getViewModel();
            // SVTK view model can be casted to SALOME_View
            displayer()->Display(entry, true, dynamic_cast<SALOME_View*>(vmodel));

            // store displayed actor in a temporary map for quicker
            // access later when restoring other parameters
            SVTK_ViewWindow* vtkView = (SVTK_ViewWindow*) vman->getActiveView();
            vtkRenderer* Renderer = vtkView->getRenderer();
            VTK::ActorCollectionCopy aCopy(Renderer->GetActors());
            vtkActorCollection* theActors = aCopy.GetActors();
            theActors->InitTraversal();
            bool isFound = false;
            vtkActor *ac = theActors->GetNextActor();
            for (; ac != NULL && !isFound; ac = theActors->GetNextActor()) {
              if (ac->IsA("SMESH_Actor")) {
                SMESH_Actor* aGeomAc = SMESH_Actor::SafeDownCast(ac);
                if (aGeomAc->hasIO()) {
                  Handle(SALOME_InteractiveObject) io =
                    Handle(SALOME_InteractiveObject)::DownCast(aGeomAc->getIO());
                  if (io->hasEntry() && strcmp(io->getEntry(), entry.toLatin1().data()) == 0) {
                    isFound = true;
                    vtkActors.Bind(viewIndex, aGeomAc);
                  }
                }
              }
            }
          }
        } // if (paramNameStr == "Visibility")
        else
        {
          // the rest properties "work" with SMESH_Actor
          if (aSmeshActor)
          {
            QString val ((*valuesIt).c_str());

            // Representation
            if (paramNameStr == "Representation") {
              aSmeshActor->SetRepresentation((SMESH_Actor::EReperesent)val.toInt());
            }
            // IsShrunk
            else if (paramNameStr == "IsShrunk") {
              if (val.toInt()) {
                if (!aSmeshActor->IsShrunk())
                  aSmeshActor->SetShrink();
              }
              else {
                if (aSmeshActor->IsShrunk())
                  aSmeshActor->UnShrink();
              }
            }
            // Displayed entities
            else if (paramNameStr == "Entities") {
              QStringList mode = val.split(gDigitsSep, QString::SkipEmptyParts);
              if (mode.count() == 6) {
                if (mode[0] != "e" || mode[2]  != "f" || mode[4] != "v") {
                  MESSAGE("Invalid order of data in Entities, must be: "
                          "e:0/1:f:0/1:v:0/1");
                }
                else {
                  unsigned int aMode = aSmeshActor->GetEntityMode();
                  unsigned int aNewMode =
                    (int(SMESH_Actor::eEdges  ) * mode[1].toInt()) |
                    (int(SMESH_Actor::eFaces  ) * mode[3].toInt()) |
                    (int(SMESH_Actor::eVolumes) * mode[5].toInt());
                  if (aNewMode != aMode)
                    aSmeshActor->SetEntityMode(aNewMode);
                }
              }
            }
            // Colors
            else if (paramNameStr == "Colors") {
              QStringList colors = val.split(gDigitsSep, QString::SkipEmptyParts);
              if (colors.count() == 16 || colors.count() == 18 ) {
                if (colors[0] != "surface" || colors[4]  != "backsurface" ||
                    (colors[8] != "edge" && colors[6] != "edge" ) || (colors[12] != "node" && colors[10] != "node") ||
                    (colors.count() == 18 && colors[14] != "outline")) {
                  MESSAGE("Invalid order of data in Colors, must be: "
                          "surface:r:g:b:backsurface:r:g:b:edge:r:g:b:node:r:g:b or surface:r:g:b:backsurface:delta:edge:r:g:b:node:r:g:b:outline:r:g:b");
                }
                else {
                  int delta = 0;
                  float er,eg,eb;
                  float nr,ng,nb;
                  vtkFloatingPointType otr,otg,otb;
                  //Old case backsurface color is independent
                  if( colors.count() == 16 ) {
                    QColor ffc;
                    SMESH::GetColor( "SMESH", "fill_color", ffc, delta, "0,170,255|-100" ) ;
                    er = colors[9].toFloat();
                    eg = colors[10].toFloat();
                    eb = colors[11].toFloat();

                    nr = colors[13].toFloat();
                    ng = colors[14].toFloat();
                    nb = colors[15].toFloat();
                    SMESH::GetColor("SMESH", "outline_color", otr, otg, otb, QColor( 0, 70, 0 ) );
                  } else {
                    //New case backsurface color depends on surface color
                    delta = colors[5].toInt();

                    er = colors[7].toFloat();
                    eg = colors[8].toFloat();
                    eb = colors[9].toFloat();

                    nr = colors[11].toFloat();
                    ng = colors[12].toFloat();
                    nb = colors[13].toFloat();

                    otr = colors[15].toFloat();
                    otg = colors[16].toFloat();
                    otb = colors[17].toFloat();
                  }
                  aSmeshActor->SetSufaceColor(colors[1].toFloat(), colors[2].toFloat(), colors[3].toFloat(), delta);
                  aSmeshActor->SetEdgeColor(er,eg,eb);
                  aSmeshActor->SetNodeColor(nr,ng,nb);
                  aSmeshActor->SetOutlineColor(otr,otg,otb);
                }
              }
            }
            // Sizes of lines and points
            else if (paramNameStr == "Sizes") {
              QStringList sizes = val.split(gDigitsSep, QString::SkipEmptyParts);
              if (sizes.count() == 4) {
                if (sizes[0] != "line" || sizes[2] != "shrink") {
                  MESSAGE("Invalid order of data in Sizes, must be: "
                          "line:int:shrink:float");
                }
                else {
                  aSmeshActor->SetLineWidth(sizes[1].toInt());
                  aSmeshActor->SetShrinkFactor(sizes[3].toFloat());
                }
              }
              else if (sizes.count() == 6) { // just to support old format
                if (sizes[0] != "line" || sizes[2]  != "node" || sizes[4] != "shrink") {
                  MESSAGE("Invalid order of data in Sizes, must be: "
                          "line:int:node:int:shrink:float");
                }
                else {
                  aSmeshActor->SetLineWidth(sizes[1].toInt());
                  //aSmeshActor->SetNodeSize(sizes[3].toInt()); // made obsolete
                  aSmeshActor->SetShrinkFactor(sizes[5].toFloat());
                }
              }
            }
            // Point marker
            else if (paramNameStr == "PointMarker") {
              QStringList data = val.split(gDigitsSep, QString::SkipEmptyParts);
              if( data.count() >= 2 ) {
                bool ok = false;
                int aParam1 = data[1].toInt( &ok );
                if( ok ) {
                  if( data[0] == "std" && data.count() == 3 ) {
                    int aParam2 = data[2].toInt( &ok );
                    aSmeshActor->SetMarkerStd( (VTK::MarkerType)aParam1, (VTK::MarkerScale)aParam2 );
                  }
                  else if( data[0] == "custom" ) {
                    VTK::MarkerMap::const_iterator markerIt = aMarkerMap.find( aParam1 );
                    if( markerIt != aMarkerMap.end() ) {
                      VTK::MarkerData aMarkerData = markerIt->second;
                      aSmeshActor->SetMarkerTexture( aParam1, aMarkerData.second );
                    }
                  }
                }
              }
            }
            // Opacity
            else if (paramNameStr == "Opacity") {
              aSmeshActor->SetOpacity(val.toFloat());
            }
            // Clipping
            else if (paramNameStr.startsWith("ClippingPlane")) {
              QStringList vals = val.split(gDigitsSep, QString::SkipEmptyParts);
              // old format - val looks like "Off" or "0:0.5:0:0" (orientation, distance, two angles)
              // new format - val looks like "Off" or "0" (plane id)
              // (note: in new format "Off" value is used only for consistency,
              //  so it is processed together with values in old format)
              bool anIsOldFormat = ( vals.count() == 4 || val == "Off" );
              if( anIsOldFormat ) {
                if (paramNameStr == "ClippingPlane1" || val == "Off")
                  aSmeshActor->RemoveAllClippingPlanes();
                if (val != "Off") {
                  SMESH::Orientation anOrientation = (SMESH::Orientation)vals[0].toInt();
                  double aDistance = vals[1].toFloat();
                  vtkFloatingPointType anAngle[2];
                  anAngle[0] = vals[2].toFloat();
                  anAngle[1] = vals[3].toFloat();

                  QList<SUIT_ViewManager*> lst;
                  getApp()->viewManagers(viewerTypStr, lst);
                  // SVTK ViewManager always has 1 ViewWindow, so view index is index of view manager
                  if (viewIndex >= 0 && viewIndex < lst.count()) {
                    SUIT_ViewManager* vman = lst.at(viewIndex);
                    SVTK_ViewWindow* vtkView = (SVTK_ViewWindow*) vman->getActiveView();

                    SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = myClippingPlaneInfoMap[ vman ];

                    SMESH::TActorList anActorList;
                    anActorList.push_back( aSmeshActor );
                    SMESH::OrientedPlane* aPlane =
                      SMESHGUI_ClippingDlg::AddPlane(anActorList, vtkView, anOrientation, aDistance, anAngle);
                    if( aPlane ) {
                      SMESH::ClippingPlaneInfo aClippingPlaneInfo;
                      aClippingPlaneInfo.Plane = aPlane;
                      aClippingPlaneInfo.ActorList = anActorList;
                      aClippingPlaneInfoList.push_back( aClippingPlaneInfo );
                    }
                  }
                }
              }
              else {
                bool ok = false;
                int aPlaneId = val.toInt( &ok );
                if( ok && aPlaneId >= 0 ) {
                  bool anIsDefinedPlane = false;
                  TPlaneInfoList& aPlaneInfoList = aPlaneInfoMap[ viewIndex ];
                  TPlaneInfoList::iterator anIter = aPlaneInfoList.begin();
                  for( ; anIter != aPlaneInfoList.end(); anIter++ ) {
                    TPlaneInfo& aPlaneInfo = *anIter;
                    if( aPlaneInfo.PlaneId == aPlaneId ) {
                      aPlaneInfo.ActorList.push_back( aSmeshActor );
                      anIsDefinedPlane = true;
                      break;
                    }
                  }
                  if( !anIsDefinedPlane ) {
                    TPlaneInfo aPlaneInfo;
                    aPlaneInfo.PlaneId = aPlaneId;
                    aPlaneInfo.ActorList.push_back( aSmeshActor );
                    aPlaneInfo.ViewManager = vman;

                    // to make the list sorted by plane id
                    anIter = aPlaneInfoList.begin();
                    for( ; anIter != aPlaneInfoList.end(); anIter++ ) {
                      const TPlaneInfo& aPlaneInfoRef = *anIter;
                      if( aPlaneInfoRef.PlaneId > aPlaneId )
                        break;
                    }
                    aPlaneInfoList.insert( anIter, aPlaneInfo );
                  }
                }
              }
            }
          } // if (aSmeshActor)
        } // other parameters than Visibility
      }
    } // for names/parameters iterator
  } // for entries iterator

  // take into account planes with empty list of actors referred to them
  QList<SUIT_ViewManager*> aVMList;
  getApp()->viewManagers(SVTK_Viewer::Type(), aVMList);

  TPlaneDataMap::const_iterator aPlaneDataIter = aPlaneDataMap.begin();
  for( ; aPlaneDataIter != aPlaneDataMap.end(); aPlaneDataIter++ ) {
    int aViewId = aPlaneDataIter->first;
    if( aViewId >= 0 && aViewId < aVMList.count() ) {
      SUIT_ViewManager* aViewManager = aVMList.at( aViewId );

      const TPlaneDataList& aPlaneDataList = aPlaneDataIter->second;

      TPlaneInfoList& aPlaneInfoList = aPlaneInfoMap[ aViewId ];
      TPlaneDataList::const_iterator anIter2 = aPlaneDataList.begin();
      for( ; anIter2 != aPlaneDataList.end(); anIter2++ ) {
        const TPlaneData& aPlaneData = *anIter2;
        int aPlaneId = aPlaneData.Id;

        bool anIsFound = false;
        TPlaneInfoList::const_iterator anIter3 = aPlaneInfoList.begin();
        for( ; anIter3 != aPlaneInfoList.end(); anIter3++ ) {
          const TPlaneInfo& aPlaneInfo = *anIter3;
          if( aPlaneInfo.PlaneId == aPlaneId ) {
            anIsFound = true;
            break;
          }
        }

        if( !anIsFound ) {
          TPlaneInfo aPlaneInfo; // ActorList field is empty
          aPlaneInfo.PlaneId = aPlaneId;
          aPlaneInfo.ViewManager = aViewManager;

          // to make the list sorted by plane id
          TPlaneInfoList::iterator anIter4 = aPlaneInfoList.begin();
          for( ; anIter4 != aPlaneInfoList.end(); anIter4++ ) {
            const TPlaneInfo& aPlaneInfoRef = *anIter4;
            if( aPlaneInfoRef.PlaneId > aPlaneId )
              break;
          }
          aPlaneInfoList.insert( anIter4, aPlaneInfo );
        }
      }
    }
  }

  // add clipping planes to actors according to the restored parameters
  // and update the clipping plane map
  TPlaneInfoMap::const_iterator anIter1 = aPlaneInfoMap.begin();
  for( ; anIter1 != aPlaneInfoMap.end(); anIter1++ ) {
    int aViewId = anIter1->first;
    const TPlaneInfoList& aPlaneInfoList = anIter1->second;

    TPlaneDataMap::const_iterator anIter2 = aPlaneDataMap.find( aViewId );
    if( anIter2 == aPlaneDataMap.end() )
      continue;
    const TPlaneDataList& aPlaneDataList = anIter2->second;

    TPlaneInfoList::const_iterator anIter3 = aPlaneInfoList.begin();
    for( ; anIter3 != aPlaneInfoList.end(); anIter3++ ) {
      const TPlaneInfo& aPlaneInfo = *anIter3;
      int aPlaneId = aPlaneInfo.PlaneId;
      const TActorList& anActorList = aPlaneInfo.ActorList;
      SUIT_ViewManager* aViewManager = aPlaneInfo.ViewManager;
      if( !aViewManager )
        continue;

      SVTK_ViewWindow* aViewWindow = dynamic_cast<SVTK_ViewWindow*>( aViewManager->getActiveView() );
      if( !aViewWindow )
        continue;

      SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = myClippingPlaneInfoMap[ aViewManager ];

      TPlaneDataList::const_iterator anIter4 = aPlaneDataList.begin();
      for( ; anIter4 != aPlaneDataList.end(); anIter4++ ) {
        const TPlaneData& aPlaneData = *anIter4;
        if( aPlaneData.Id == aPlaneId ) {
          SMESH::OrientedPlane* aPlane =
            SMESHGUI_ClippingDlg::AddPlane( anActorList,
                                            aViewWindow,
                                            (SMESH::Orientation)aPlaneData.Orientation,
                                            aPlaneData.Distance,
                                            aPlaneData.Angle );
          if( aPlane ) {
            SMESH::ClippingPlaneInfo aClippingPlaneInfo;
            aClippingPlaneInfo.Plane = aPlane;
            aClippingPlaneInfo.ActorList = anActorList;
            aClippingPlaneInfoList.push_back( aClippingPlaneInfo );
          }
          break;
        }
      }
    }
  }

  // update all VTK views
  QList<SUIT_ViewManager*> lst;
  getApp()->viewManagers(lst);
  for (QList<SUIT_ViewManager*>::Iterator it = lst.begin(); it != lst.end(); it++) {
    SUIT_ViewModel* vmodel = (*it)->getViewModel();
    if (vmodel && vmodel->getType() == SVTK_Viewer::Type()) {
      SVTK_ViewWindow* vtkView = (SVTK_ViewWindow*) (*it)->getActiveView();
      vtkView->getRenderer()->ResetCameraClippingRange();
      vtkView->Repaint();
    }
  }
}

/*!
  \brief Adds preferences for dfont of VTK viewer
  \param label label
  \param pIf group identifier
  \param param parameter
  \return identifier of preferences
*/
int SMESHGUI::addVtkFontPref( const QString& label, const int pId, const QString& param )
{
  int tfont = addPreference( label, pId, LightApp_Preferences::Font, "SMESH", param );

  setPreferenceProperty( tfont, "mode", QtxFontEdit::Custom );

  QStringList fam;
  fam.append( tr( "SMESH_FONT_ARIAL" ) );
  fam.append( tr( "SMESH_FONT_COURIER" ) );
  fam.append( tr( "SMESH_FONT_TIMES" ) );

  setPreferenceProperty( tfont, "fonts", fam );

  int f = QtxFontEdit::Family | QtxFontEdit::Bold | QtxFontEdit::Italic | QtxFontEdit::Shadow;
  setPreferenceProperty( tfont, "features", f );

  return tfont;
}

/*!
  \brief Actions after hypothesis edition
  Updates object browser after hypothesis edition
*/
void SMESHGUI::onHypothesisEdit( int result )
{
  if( result == 1 )
    SMESHGUI::Modified();
  updateObjBrowser( true );
}


/*!
  \brief Signal handler closing(SUIT_ViewWindow*) of a view
  \param pview view being closed
*/
void SMESHGUI::onViewClosed( SUIT_ViewWindow* pview ) {
#ifndef DISABLE_PLOT2DVIEWER
  //Crear all Plot2d Viewers if need.
  SMESH::ClearPlot2Viewers(pview);
#endif
}

void SMESHGUI::message( const QString& msg )
{
  // dispatch message
  QStringList data = msg.split("/");
  if ( data.count() > 0 ) {
    if ( data.first() == "mesh_loading" ) {
      // get mesh entry
      QString entry = data.count() > 1 ? data[1] : QString();
      if ( entry.isEmpty() )
        return;
      // get study
      _PTR(Study) study = dynamic_cast<SalomeApp_Study*>( application()->activeStudy() )->studyDS();
      // get mesh name
      _PTR(SObject) obj = study->FindObjectID( entry.toLatin1().constData() );
      QString name;
      if ( obj )
        name = obj->GetName().c_str();
      if ( name.isEmpty() )
        return;
      
      if ( data.last() == "stop" )
        application()->putInfo( tr( "MESH_LOADING_MSG_FINISHED" ).arg( name ) );
      else
        application()->putInfo( tr( "MESH_LOADING_MSG" ).arg( name ) );
      QApplication::processEvents();
    }
  }
}

/*!
  \brief Connects or disconnects signals about activating and cloning view on the module slots
  \param pview view which is connected/disconnected
*/
void SMESHGUI::connectView( const SUIT_ViewWindow* pview ) {
  if(!pview)
    return;

  SUIT_ViewManager* viewMgr = pview->getViewManager();
  if ( viewMgr ) {
    disconnect( viewMgr, SIGNAL( deleteView( SUIT_ViewWindow* ) ),
                this, SLOT( onViewClosed( SUIT_ViewWindow* ) ) );

    connect( viewMgr, SIGNAL( deleteView( SUIT_ViewWindow* ) ),
             this, SLOT( onViewClosed( SUIT_ViewWindow* ) ) );
  }
}

/*!
  \brief Return \c true if object can be renamed
*/
bool SMESHGUI::renameAllowed( const QString& entry) const {
  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( application() );
  if( !anApp )
    return false;

  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( anApp->activeStudy() );
  if( !appStudy )
    return false;

  SalomeApp_DataObject* obj = dynamic_cast<SalomeApp_DataObject*>(appStudy->findObjectByEntry(entry));
  
  if(!obj)
    return false;

  if(appStudy->isComponent(entry) || obj->isReference())
    return false;

  // check type to prevent renaming of inappropriate objects
  int aType = SMESHGUI_Selection::type(qPrintable(entry), SMESH::GetActiveStudyDocument());
  if (aType == MESH || aType == GROUP ||
      aType == SUBMESH || aType == SUBMESH_COMPOUND ||
      aType == SUBMESH_SOLID || aType == SUBMESH_FACE ||
      aType == SUBMESH_EDGE || aType == SUBMESH_VERTEX ||
      aType == HYPOTHESIS || aType == ALGORITHM)
    return true;

  return false;
}

/*!
  Rename object by entry.
  \param entry entry of the object
  \param name new name of the object
  \brief Return \c true if rename operation finished successfully, \c false otherwise.
*/
bool SMESHGUI::renameObject( const QString& entry, const QString& name) {

  SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( application() );
  if( !anApp )
    return false;
    
  SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( anApp->activeStudy() );

  if(!appStudy)
    return false;
  
  _PTR(Study) aStudy = appStudy->studyDS();
  
  if(!aStudy)
    return false;
  
  bool aLocked = (_PTR(AttributeStudyProperties)(appStudy->studyDS()->GetProperties()))->IsLocked();
  if ( aLocked ) {
    SUIT_MessageBox::warning ( anApp->desktop(), QObject::tr("WRN_WARNING"), QObject::tr("WRN_STUDY_LOCKED") );
    return false;
  }


  _PTR(SObject) obj = aStudy->FindObjectID( qPrintable(entry) );
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName) aName;
  if ( obj ) {
    if ( obj->FindAttribute(anAttr, "AttributeName") ) {
      aName = anAttr;
      // check type to prevent renaming of inappropriate objects
      int aType = SMESHGUI_Selection::type( qPrintable(entry), SMESH::GetActiveStudyDocument() );
      if (aType == MESH || aType == GROUP ||
          aType == SUBMESH || aType == SUBMESH_COMPOUND ||
          aType == SUBMESH_SOLID || aType == SUBMESH_FACE ||
          aType == SUBMESH_EDGE || aType == SUBMESH_VERTEX ||
          aType == HYPOTHESIS || aType == ALGORITHM) {
        if ( !name.isEmpty() ) {
          SMESHGUI::GetSMESHGen()->SetName(obj->GetIOR().c_str(), qPrintable(name) );

          // update name of group object and its actor
          Handle(SALOME_InteractiveObject) IObject =
            new SALOME_InteractiveObject ( qPrintable(entry), "SMESH", qPrintable(name) );

          SMESH::SMESH_GroupBase_var aGroupObject = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IObject);
          if( !aGroupObject->_is_nil() ) {
            aGroupObject->SetName( qPrintable(name) );
            if ( SMESH_Actor *anActor = SMESH::FindActorByEntry( qPrintable(entry) ) )
              anActor->setName( qPrintable(name) );
          }
          return true;
        }
      }
    }
  }
  return false;
}
