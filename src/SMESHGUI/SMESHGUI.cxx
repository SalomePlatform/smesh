// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI.cxx
//  Author : Nicolas REJNERI, Open CASCADE S.A.S.

#include <Standard_math.hxx>  // E.A. must be included before Python.h to fix compilation on windows
#ifdef HAVE_FINITE
#undef HAVE_FINITE            // VSR: avoid compilation warning on Linux : "HAVE_FINITE" redefined
#endif
#include "Python.h"

//  SMESH includes
#include "SMESHGUI.h"
#include "SMESHGUI_Add0DElemsOnAllNodesDlg.h"
#include "SMESHGUI_AddMeshElementDlg.h"
#include "SMESHGUI_AddQuadraticElementDlg.h"
#include "SMESHGUI_BuildCompoundDlg.h"
#include "SMESHGUI_ClippingDlg.h"
#include "SMESHGUI_ComputeDlg.h"
#include "SMESHGUI_ConvToQuadOp.h"
#include "SMESHGUI_CopyMeshDlg.h"
#include "SMESHGUI_CreatePolyhedralVolumeDlg.h"
#include "SMESHGUI_DeleteGroupDlg.h"
#include "SMESHGUI_Displayer.h"
#include "SMESHGUI_DuplicateNodesDlg.h"
#include "SMESHGUI_ExtrusionAlongPathDlg.h"
#include "SMESHGUI_ExtrusionDlg.h"
#include "SMESHGUI_FieldSelectorWdg.h"
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
#include "SMESHGUI_MergeDlg.h"
#include "SMESHGUI_MeshInfo.h"
#include "SMESHGUI_MeshOp.h"
#include "SMESHGUI_MeshOrderOp.h"
#include "SMESHGUI_MeshPatternDlg.h"
#include "SMESHGUI_MultiEditDlg.h"
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI_Operations.h"
#include "SMESHGUI_Preferences_ScalarBarDlg.h"
#include "SMESHGUI_PropertiesDlg.h"
#include "SMESHGUI_RemoveElementsDlg.h"
#include "SMESHGUI_RemoveNodesDlg.h"
#include "SMESHGUI_RenumberingDlg.h"
#include "SMESHGUI_ReorientFacesDlg.h"
#include "SMESHGUI_RevolutionDlg.h"
#include "SMESHGUI_RotationDlg.h"
#include "SMESHGUI_ScaleDlg.h"
#include "SMESHGUI_Selection.h"
#include "SMESHGUI_SewingDlg.h"
#include "SMESHGUI_SingleEditDlg.h"
#include "SMESHGUI_SmoothingDlg.h"
#include "SMESHGUI_SymmetryDlg.h"
#include "SMESHGUI_TranslationDlg.h"
#include "SMESHGUI_TransparencyDlg.h"
#include "SMESHGUI_DisplayEntitiesDlg.h"
#include "SMESHGUI_SplitBiQuad.h"

#include "SMESHGUI_FilterUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_GroupUtils.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMESH_version.h"

#include "SMESH_ControlsDef.hxx"
#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_Client.hxx"
#include "SMESH_ScalarBarActor.h"
#include "SMESH_TypeFilter.hxx"

// SALOME GUI includes
#include <SalomeApp_Application.h>
#include <SalomeApp_CheckFileDlg.h>
#include <SalomeApp_DataObject.h>
#include <SalomeApp_Study.h>
#include <SalomeApp_Tools.h>

#include <LightApp_DataOwner.h>
#include <LightApp_NameDlg.h>
#include <LightApp_Preferences.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>

#include <SVTK_ViewManager.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>

#include <VTKViewer_Algorithm.h>

#include <SUIT_Desktop.h>
#include <SUIT_FileDlg.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>

#include <QtxPopupMgr.h>
#include <QtxFontEdit.h>

#include <SALOME_ListIO.hxx>

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
#include <QListView>
#include <QTreeView>
#include <QCheckBox>
#include <QLayout>
#include <QDialogButtonBox>

// BOOST includes
#include <boost/shared_ptr.hpp>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkLookupTable.h>
#include <vtkPlane.h>
#include <vtkRenderer.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_ClientFactory.hxx>
#include <SALOMEDSClient_IParameters.hxx>
#include <SALOMEDSClient_SComponent.hxx>
#include <SALOMEDSClient_StudyBuilder.hxx>
#include <SALOMEDS_Study.hxx>
#include <SALOMEDS_SObject.hxx>
#include "utilities.h"

// OCCT includes
#include <Standard_ErrorHandler.hxx>
#include <NCollection_DataMap.hxx>
#include <NCollection_DoubleMap.hxx>

#include <Basics_Utils.hxx>

// Below macro, when uncommented, switches on simplified (more performant) algorithm
// of auto-color picking up
#define SIMPLE_AUTOCOLOR

namespace
{
  // Declarations
  //=============================================================
  void ImportMeshesFromFile(SMESH::SMESH_Gen_ptr theComponentMesh,
                            int                  theCommandID);

  void ExportMeshToFile(int theCommandID);

  void SetDisplayMode(int theCommandID, SMESHGUI_StudyId2MarkerMap& theMarkerMap);

  void SetDisplayEntity(int theCommandID);

  int  ActionToControl( int theID, bool theReversed = false );

  void Control( int theCommandID );

  // Definitions
  //================================================================================
  /*!
   * \brief Reads meshes from file
   */
  //================================================================================

  void ImportMeshesFromFile( SMESH::SMESH_Gen_ptr theComponentMesh,
                             int                  theCommandID )
  {
    QStringList filter;
    std::string myExtension;

    if ( theCommandID == SMESHOp::OpImportMED ) {
      filter.append( QObject::tr( "MED_FILES_FILTER" ) + " (*.*med)" );
      filter.append( QObject::tr( "ALL_FILES_FILTER" ) + " (*)" );
    }
    else if ( theCommandID == SMESHOp::OpImportUNV ) {
      filter.append( QObject::tr( "IDEAS_FILES_FILTER" ) + " (*.unv)" );
    }
    else if ( theCommandID == SMESHOp::OpImportDAT ) {
      filter.append( QObject::tr( "DAT_FILES_FILTER" ) + " (*.dat)" );
    }
    else if ( theCommandID == SMESHOp::OpImportSTL ) {
      filter.append( QObject::tr( "STL_FILES_FILTER" ) + " (*.stl)" );
    }
  #ifdef WITH_CGNS
    else if ( theCommandID == SMESHOp::OpImportCGNS ) {
      filter.append( QObject::tr( "CGNS_FILES_FILTER" ) + " (*.cgns)" );
    }
  #endif
    else if ( theCommandID == SMESHOp::OpImportSAUV ) {
      filter.append( QObject::tr( "SAUV files (*.sauv*)" ) );
      filter.append( QObject::tr( "All files (*)" ) );
    }
    else if ( theCommandID == SMESHOp::OpImportGMF ) {
      filter.append( QObject::tr( "GMF_ASCII_FILES_FILTER" ) + " (*.mesh)"  );
      filter.append( QObject::tr( "GMF_BINARY_FILES_FILTER") + " (*.meshb)" );
    }

    QString anInitialPath = "";
    if ( SUIT_FileDlg::getLastVisitedPath().isEmpty() )
      anInitialPath = QDir::currentPath();

    QStringList filenames;
    bool toCreateGroups = true;

    // if ( theCommandID == SMESHOp::OpImportGMF ) { // GMF
    //   SalomeApp_CheckFileDlg* fd = new SalomeApp_CheckFileDlg
    //     ( SMESHGUI::desktop(), true, QObject::tr("SMESH_REQUIRED_GROUPS"), true, true );
    //   fd->setWindowTitle( QObject::tr( "SMESH_IMPORT_MESH" ) );
    //   fd->setNameFilters( filter );
    //   fd->SetChecked( true );
    //   if ( fd->exec() )
    //     filenames << fd->selectedFile();
    //   toCreateGroups = fd->IsChecked();

    //   delete fd;
    // }
    // else
    {
      filenames = SUIT_FileDlg::getOpenFileNames( SMESHGUI::desktop(),
                                                  anInitialPath,
                                                  filter,
                                                  QObject::tr( "SMESH_IMPORT_MESH" ) );
    }
    if ( filenames.count() > 0 )
    {
      SUIT_OverrideCursor wc;
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

      QStringList errors;
      QStringList anEntryList;
      bool isEmpty = false;
      for ( QStringList::ConstIterator it = filenames.begin(); it != filenames.end(); ++it )
      {
        QString filename = *it;
        SMESH::mesh_array_var aMeshes = new SMESH::mesh_array;
        try {
          switch ( theCommandID ) {
          case SMESHOp::OpImportDAT:
            {
              // DAT format (currently unsupported)
              errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                             arg( QObject::tr( "SMESH_ERR_NOT_SUPPORTED_FORMAT" ) ) );
              break;
            }
          case SMESHOp::OpImportUNV:
            {
              // UNV format
              aMeshes->length( 1 );
              aMeshes[0] = theComponentMesh->CreateMeshesFromUNV( filename.toUtf8().constData() );
              if ( aMeshes[0]->_is_nil() )
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( "SMESH_ERR_UNKNOWN_IMPORT_ERROR" ) ) );
              break;
            }
          case SMESHOp::OpImportMED:
            {
              // MED format
              SMESH::DriverMED_ReadStatus res;
              aMeshes = theComponentMesh->CreateMeshesFromMED( filename.toUtf8().constData(), res );
              if ( res != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
              }
              break;
            }
          case SMESHOp::OpImportSTL:
            {
              // STL format
              aMeshes->length( 1 );
              aMeshes[0] = theComponentMesh->CreateMeshesFromSTL( filename.toUtf8().constData() );
              if ( aMeshes[0]->_is_nil() ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( "SMESH_ERR_UNKNOWN_IMPORT_ERROR" ) ) );
              }
              break;
            }
        #ifdef WITH_CGNS
          case SMESHOp::OpImportCGNS:
            {
              // CGNS format
              SMESH::DriverMED_ReadStatus res;
              aMeshes = theComponentMesh->CreateMeshesFromCGNS( filename.toUtf8().constData(), res );
              if ( res != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
              }
              break;
            }
        #endif
          case SMESHOp::OpImportSAUV:
            {
              // SAUV format
              SMESH::DriverMED_ReadStatus res;
              aMeshes = theComponentMesh->CreateMeshesFromSAUV( filename.toUtf8().constData(), res );
              if ( res != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res ).toLatin1().data() ) ) );
              }
              break;
            }
          case SMESHOp::OpImportGMF:
            {
              // GMF format
              SMESH::ComputeError_var res;
              aMeshes->length( 1 );
              aMeshes[0] = theComponentMesh->CreateMeshesFromGMF( filename.toUtf8().constData(),
                                                                  toCreateGroups,
                                                                  res.out() );
              if ( res->code != SMESH::DRS_OK ) {
                errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                               arg( QObject::tr( QString( "SMESH_DRS_%1" ).arg( res->code ).toLatin1().data() ) ) );
                if ( strlen( res->comment.in() ) > 0 ) {
                  errors.back() += ": ";
                  errors.back() += res->comment.in();
                }
              }
              break;
            }
          }
        }
        catch ( const SALOME::SALOME_Exception& S_ex ) {
          errors.append( QString( "%1 :\n\t%2" ).arg( filename ).
                         arg( QObject::tr( "SMESH_ERR_UNKNOWN_IMPORT_ERROR" ) ) );
        }

        for ( int i = 0, iEnd = aMeshes->length(); i < iEnd; i++ )
        {
          _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshes[i] );
          if ( aMeshSO ) {
            _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
            _PTR(AttributePixMap) aPixmap = aBuilder->FindOrCreateAttribute( aMeshSO, "AttributePixMap" );
            aPixmap->SetPixMap( "ICON_SMESH_TREE_MESH_IMPORTED" );
            if ( theCommandID == SMESHOp::OpImportUNV ) // mesh names aren't taken from the file for UNV import
              SMESH::SetName( aMeshSO, QFileInfo(filename).fileName() );

            anEntryList.append( aMeshSO->GetID().c_str() );
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

    const bool isDAT = ( theCommandID == SMESHOp::OpExportDAT ||
                         theCommandID == SMESHOp::OpPopupExportDAT );
    const bool isMED = ( theCommandID == SMESHOp::OpExportMED ||
                         theCommandID == SMESHOp::OpPopupExportMED );
    const bool isUNV = ( theCommandID == SMESHOp::OpExportUNV ||
                         theCommandID == SMESHOp::OpPopupExportUNV );
    const bool isSTL = ( theCommandID == SMESHOp::OpExportSTL ||
                         theCommandID == SMESHOp::OpPopupExportSTL );
#ifdef WITH_CGNS
    const bool isCGNS= ( theCommandID == SMESHOp::OpExportCGNS ||
                         theCommandID == SMESHOp::OpPopupExportCGNS );
#else
    const bool isCGNS= false;
#endif
    const bool isSAUV= ( theCommandID == SMESHOp::OpExportSAUV ||
                         theCommandID == SMESHOp::OpPopupExportSAUV );
    const bool isGMF = ( theCommandID == SMESHOp::OpExportGMF ||
                         theCommandID == SMESHOp::OpPopupExportGMF );

    const bool multiMeshSupported = ( isMED || isCGNS ); // file can hold several meshes
    if ( selected.Extent() == 0 || ( selected.Extent() > 1 && !multiMeshSupported ))
      return;
    SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
    bool aCheckWarn = true;
    if ( resMgr )
      aCheckWarn = resMgr->booleanValue( "SMESH", "show_warning", false );
    // get mesh object from selection and check duplication of their names
    bool hasDuplicatedMeshNames = false;
    QList< QPair< SMESH::SMESH_IDSource_var, QString > >           aMeshList;
    QList< QPair< SMESH::SMESH_IDSource_var, QString > >::iterator aMeshIter;
    SALOME_ListIteratorOfListIO It( selected );
    for( ; It.More(); It.Next() )
    {
      Handle(SALOME_InteractiveObject) anIObject = It.Value();
      SMESH::SMESH_IDSource_var aMeshItem =
        SMESH::IObjectToInterface<SMESH::SMESH_IDSource>(anIObject);
      if ( aMeshItem->_is_nil() ) {
        SUIT_MessageBox::warning( SMESHGUI::desktop(),
                                  QObject::tr( "SMESH_WRN_WARNING" ),
                                  QObject::tr( "SMESH_BAD_MESH_SELECTION" ));
        return;
      }
      SMESH::SMESH_GroupBase_var aGroup   = SMESH::SMESH_GroupBase::_narrow( aMeshItem );
      if ( aCheckWarn && !aGroup->_is_nil() ) {
        QMessageBox msgBox(SUIT_MessageBox::Warning,QObject::tr("SMESH_WRN_WARNING"),
                            QObject::tr("SMESH_EXPORT_ONLY_GPOUP"),QMessageBox::StandardButton::NoButton, SMESHGUI::desktop());
        QCheckBox dontShowCheckBox(QObject::tr("SMESH_WRN_SHOW_DLG_CHECKBOX"));
        msgBox.addButton(QMessageBox::Ok);
        msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        QGridLayout* lt = qobject_cast<QGridLayout*>(msgBox.layout());
        QDialogButtonBox* btnbox = msgBox.findChild<QDialogButtonBox*>();
        lt->addWidget(&dontShowCheckBox, lt->rowCount(), lt->columnCount()-1, lt->rowCount(), lt->columnCount());
        lt->addWidget(btnbox, lt->rowCount(), 0, lt->rowCount(), lt->columnCount());
        if(msgBox.exec() == QMessageBox::Ok)
        {
            if(dontShowCheckBox.checkState() == Qt::Checked)
            {
              if ( resMgr )
                resMgr->setValue( "SMESH", "show_warning", false);
            }
            aCheckWarn = false;
        }
        else
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
    SMESH::SMESH_Mesh_var            aMesh = aMeshOrGroup->GetMesh();
    QString                      aMeshName = (*aMeshIter).second;

    if ( isMED || isCGNS || isSAUV ) // formats where group names must be unique
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
    
    // Warn the user about presence of not supported elements
    QString format;
    std::vector< SMESH::EntityType > notSupportedElemTypes, presentNotSupported;
    if ( isDAT )
    {
      format = "DAT";
      notSupportedElemTypes.push_back( SMESH::Entity_0D );
      notSupportedElemTypes.push_back( SMESH::Entity_Ball );
    }
    else if ( isUNV )
    {
      format = "UNV";
      notSupportedElemTypes.push_back( SMESH::Entity_Polygon );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Polygon );
      notSupportedElemTypes.push_back( SMESH::Entity_Polyhedra );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Polyhedra );
      notSupportedElemTypes.push_back( SMESH::Entity_Pyramid );
      notSupportedElemTypes.push_back( SMESH::Entity_Hexagonal_Prism );
      notSupportedElemTypes.push_back( SMESH::Entity_0D );
      notSupportedElemTypes.push_back( SMESH::Entity_Ball );
    }
    else if ( isSTL )
    {
      format = "STL";
      notSupportedElemTypes.push_back( SMESH::Entity_Edge );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Edge );
      notSupportedElemTypes.push_back( SMESH::Entity_0D );
      notSupportedElemTypes.push_back( SMESH::Entity_Ball );
    }
    else if ( isCGNS )
    {
      format = "CGNS";
      notSupportedElemTypes.push_back( SMESH::Entity_Ball );
    }
    else if ( isSAUV )
    {
      format = "SAUV";
      notSupportedElemTypes.push_back( SMESH::Entity_Ball );
      notSupportedElemTypes.push_back( SMESH::Entity_BiQuad_Triangle );
      notSupportedElemTypes.push_back( SMESH::Entity_BiQuad_Quadrangle );
      notSupportedElemTypes.push_back( SMESH::Entity_TriQuad_Hexa );
      notSupportedElemTypes.push_back( SMESH::Entity_Hexagonal_Prism );
      notSupportedElemTypes.push_back( SMESH::Entity_Polygon );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Polygon );
      notSupportedElemTypes.push_back( SMESH::Entity_Polyhedra );
    }
    else if ( isGMF )
    {
      format = "GMF";
      notSupportedElemTypes.push_back( SMESH::Entity_0D );
      notSupportedElemTypes.push_back( SMESH::Entity_Polygon );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Polygon );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Pyramid );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Penta );
      notSupportedElemTypes.push_back( SMESH::Entity_Hexagonal_Prism );
      notSupportedElemTypes.push_back( SMESH::Entity_Polyhedra );
      notSupportedElemTypes.push_back( SMESH::Entity_Quad_Polyhedra );
      notSupportedElemTypes.push_back( SMESH::Entity_Ball );
    }
    if ( ! notSupportedElemTypes.empty() )
    {
      SMESH::long_array_var nbElems = aMeshOrGroup->GetMeshInfo();
      for ( size_t iType = 0; iType < notSupportedElemTypes.size(); ++iType )
        if ( nbElems[ notSupportedElemTypes[ iType ]] > 0 )
          presentNotSupported.push_back( notSupportedElemTypes[ iType ]);
    }
    if ( !presentNotSupported.empty() )
    {
      QString typeNames;
      const char* typeMsg[] = {
        "SMESH_NODES", "SMESH_ELEMS0D","SMESH_EDGES","SMESH_QUADRATIC_EDGES",
        "SMESH_TRIANGLES", "SMESH_QUADRATIC_TRIANGLES", "SMESH_BIQUADRATIC_TRIANGLES",
        "SMESH_QUADRANGLES","SMESH_QUADRATIC_QUADRANGLES", "SMESH_BIQUADRATIC_QUADRANGLES",
        "SMESH_POLYGONS","SMESH_QUADRATIC_POLYGONS",
        "SMESH_TETRAHEDRA","SMESH_QUADRATIC_TETRAHEDRONS","SMESH_PYRAMIDS",
        "SMESH_QUADRATIC_PYRAMIDS","SMESH_HEXAHEDRA","SMESH_QUADRATIC_HEXAHEDRONS",
        "SMESH_TRIQUADRATIC_HEXAHEDRONS","SMESH_PENTAHEDRA","SMESH_QUADRATIC_PENTAHEDRONS",
        "SMESH_OCTAHEDRA","SMESH_POLYEDRONS","SMESH_QUADRATIC_POLYEDRONS","SMESH_BALLS"
      };
      // is typeMsg complete? (compilation failure mains that enum SMDSAbs_EntityType changed)
      const int nbTypes = sizeof( typeMsg ) / sizeof( const char* );
      int _assert[( nbTypes == SMESH::Entity_Last ) ? 2 : -1 ]; _assert[0]=_assert[1];

      QString andStr = " " + QObject::tr("SMESH_AND") + " ", comma(", ");
      for ( size_t iType = 0; iType < presentNotSupported.size(); ++iType ) {
        typeNames += QObject::tr( typeMsg[ presentNotSupported[ iType ]]);
        if ( iType != presentNotSupported.size() - 1 )
          typeNames += ( iType == presentNotSupported.size() - 2 ) ? andStr : comma;
      }
      int aRet = SUIT_MessageBox::warning
        (SMESHGUI::desktop(),
         QObject::tr("SMESH_WRN_WARNING"),
         QObject::tr("EXPORT_NOT_SUPPORTED").arg(aMeshName).arg(format).arg(typeNames),
         QObject::tr("SMESH_BUT_YES"),
         QObject::tr("SMESH_BUT_NO"), 0, 1);
      if (aRet != 0)
        return;
    }

    // Get parameters of export operation

    QString            aFilename;
    SMESH::MED_VERSION aFormat = SMESH::MED_V2_2;
    // Init the parameters with the default values
    bool aIsASCII_STL   = true;
    bool toCreateGroups = false;
    if ( resMgr )
      toCreateGroups = resMgr->booleanValue( "SMESH", "auto_groups", false );
    bool toOverwrite  = true;
    bool toFindOutDim = true;

    QString aFilter, aTitle = QObject::tr("SMESH_EXPORT_MESH");
    QString anInitialPath = "";
    if ( SUIT_FileDlg::getLastVisitedPath().isEmpty() )
      anInitialPath = QDir::currentPath();

    QList< QPair< GEOM::ListOfFields_var, QString > > aFieldList;

    // Get a file name to write in and additional otions
    if ( isUNV || isDAT || isGMF ) // Export w/o options
    {
      if ( isUNV )
        aFilter = QObject::tr( "IDEAS_FILES_FILTER" ) + " (*.unv)";
      else if ( isDAT )
        aFilter = QObject::tr( "DAT_FILES_FILTER" ) + " (*.dat)";
      else if ( isGMF )
        aFilter = QObject::tr( "GMF_ASCII_FILES_FILTER" ) + " (*.mesh)" +
          ";;" +  QObject::tr( "GMF_BINARY_FILES_FILTER" )  + " (*.meshb)";
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
      QStringList checkBoxes;
      checkBoxes << QObject::tr("SMESH_AUTO_GROUPS") << QObject::tr("SMESH_AUTO_DIM");

      SMESHGUI_FieldSelectorWdg* fieldSelWdg = new SMESHGUI_FieldSelectorWdg();
      QList< QWidget* > wdgList;
      if ( fieldSelWdg->GetAllFeilds( aMeshList, aFieldList ))
        wdgList.append( fieldSelWdg );

      SalomeApp_CheckFileDlg* fd =
        new SalomeApp_CheckFileDlg ( SMESHGUI::desktop(), false, checkBoxes, true, true, wdgList );
      fd->setWindowTitle( aTitle );
      fd->setNameFilters( filters );
      fd->selectNameFilter( aDefaultFilter );
      fd->SetChecked( toCreateGroups, 0 );
      fd->SetChecked( toFindOutDim,   1 );
      if ( !anInitialPath.isEmpty() )
        fd->setDirectory( anInitialPath );
      fd->selectFile(aMeshName);
      
      
      QListView *lview = fd->findChild<QListView*>("listView");
      if( lview ) {
        lview->setMinimumHeight(200);
      }
      QTreeView *tview = fd->findChild<QTreeView*>("treeView");
      if( tview ) {
        tview->setMinimumHeight(200);
      }

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
            bool isVersionOk = SMESHGUI::GetSMESHGen()->GetMEDVersion( aFilename.toUtf8().constData(), aVersion );
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
            SMESH::string_array_var aMeshNames = SMESHGUI::GetSMESHGen()->GetMeshNames( aFilename.toUtf8().constData() );
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
      toCreateGroups = fd->IsChecked(0);
      toFindOutDim   = fd->IsChecked(1);
      fieldSelWdg->GetSelectedFeilds();
      if ( !fieldSelWdg->parent() )
        delete fieldSelWdg;
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
//           Renumber= resMgr->booleanValue("renumbering");
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
            const GEOM::ListOfFields&       fields = aFieldList[ aMeshIndex ].first.in();
            const QString&            geoAssFields = aFieldList[ aMeshIndex ].second;
            const bool                   hasFields = ( fields.length() || !geoAssFields.isEmpty() );
            if ( !hasFields && aMeshOrGroup->_is_equivalent( aMeshItem ))
              aMeshItem->ExportToMEDX( aFilename.toUtf8().data(), toCreateGroups,
                                       aFormat, toOverwrite && aMeshIndex == 0, toFindOutDim );
            else
              aMeshItem->ExportPartToMED( aMeshOrGroup, aFilename.toUtf8().data(), toCreateGroups,
                                          aFormat, toOverwrite && aMeshIndex == 0, toFindOutDim,
                                          fields, geoAssFields.toLatin1().data() );
          }
        }
        else if ( isSAUV )
        {
          for( aMeshIter = aMeshList.begin(); aMeshIter != aMeshList.end(); aMeshIter++ )
          {
            SMESH::SMESH_Mesh_var aMeshItem = SMESH::SMESH_Mesh::_narrow( (*aMeshIter).first );
            if( !aMeshItem->_is_nil() )
              aMeshItem->ExportSAUV( aFilename.toUtf8().data(), toCreateGroups );
          }
        }
        else if ( isDAT )
        {
          if ( aMeshOrGroup->_is_equivalent( aMesh ))
            aMesh->ExportDAT( aFilename.toUtf8().data() );
          else
            aMesh->ExportPartToDAT( aMeshOrGroup, aFilename.toUtf8().data() );
        }
        else if ( isUNV )
        {
          if ( aMeshOrGroup->_is_equivalent( aMesh ))
            aMesh->ExportUNV( aFilename.toUtf8().data() );
          else
            aMesh->ExportPartToUNV( aMeshOrGroup, aFilename.toUtf8().data() );
        }
        else if ( isSTL )
        {
          if ( aMeshOrGroup->_is_equivalent( aMesh ))
            aMesh->ExportSTL( aFilename.toUtf8().data(), aIsASCII_STL );
          else
            aMesh->ExportPartToSTL( aMeshOrGroup, aFilename.toUtf8().data(), aIsASCII_STL );
        }
        else if ( isCGNS )
        {
          aMeshIter = aMeshList.begin();
          for( int aMeshIndex = 0; aMeshIter != aMeshList.end(); aMeshIter++, aMeshIndex++ )
          {
            SMESH::SMESH_IDSource_var aMeshOrGroup = (*aMeshIter).first;
            SMESH::SMESH_Mesh_var        aMeshItem = aMeshOrGroup->GetMesh();
            aMeshItem->ExportCGNS( aMeshOrGroup,
                                   aFilename.toUtf8().data(),
                                   toOverwrite && aMeshIndex == 0 );
          }
        }
        else if ( isGMF )
        {
          toCreateGroups = true;
          aMesh->ExportGMF( aMeshOrGroup, aFilename.toUtf8().data(), toCreateGroups );
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
                                unsigned int  theMode)
  {
    bool anIsNotPresent = ~theOutputMode & theMode;
    if(anIsNotPresent)
      theOutputMode |= theMode;
    else
      theOutputMode &= ~theMode;
  }

  void SetDisplayEntity(int theCommandID)
  {
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if ( aSel )
      aSel->selectedObjects( selected );

    if ( selected.Extent() >= 1 ) {
      SUIT_OverrideCursor wc;
      SALOME_ListIteratorOfListIO It( selected );
      for( ; It.More(); It.Next()){
        Handle(SALOME_InteractiveObject) IObject = It.Value();
        if(IObject->hasEntry()){
          if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
            unsigned int aMode = anActor->GetEntityMode();
            switch(theCommandID){
            case SMESHOp::OpDE0DElements: InverseEntityMode(aMode,SMESH_Actor::e0DElements); break;
            case SMESHOp::OpDEEdges:      InverseEntityMode(aMode,SMESH_Actor::eEdges); break;
            case SMESHOp::OpDEFaces:      InverseEntityMode(aMode,SMESH_Actor::eFaces); break;
            case SMESHOp::OpDEVolumes:    InverseEntityMode(aMode,SMESH_Actor::eVolumes); break;
            case SMESHOp::OpDEBalls:      InverseEntityMode(aMode,SMESH_Actor::eBallElem); break;
            case SMESHOp::OpDEAllEntity:  aMode = SMESH_Actor::eAllEntity; break;
            }
            if(aMode)
              anActor->SetEntityMode(aMode);
          }
        }
      }
    }
  }

  void AutoColor()
  {
    SalomeApp_Application* app =
      dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
    if ( !app )
      return;

    LightApp_SelectionMgr* aSel = app->selectionMgr();
    SalomeApp_Study* appStudy = dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
    if ( !aSel || !appStudy )
      return;

    SALOME_ListIO selected;
    aSel->selectedObjects( selected );
    if ( selected.IsEmpty() )
      return;

    Handle(SALOME_InteractiveObject) anIObject = selected.First();

    _PTR(Study)         aStudy = appStudy->studyDS();
    _PTR(SObject) aMainSObject = aStudy->FindObjectID( anIObject->getEntry() );
    SMESH::SMESH_Mesh_var aMainObject = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
    if ( aMainObject->_is_nil() )
      return;

    SUIT_OverrideCursor wc;

    aMainObject->SetAutoColor( true ); // mesh groups are re-colored here

    QList<SALOMEDS::Color> aReservedColors;

    SMESH::ListOfGroups aListOfGroups = *aMainObject->GetGroups();
    for ( int i = 0, n = aListOfGroups.length(); i < n; i++ )
    {
      SMESH::SMESH_GroupBase_var aGroupObject = aListOfGroups[i];

#ifdef SIMPLE_AUTOCOLOR   // simplified algorithm for auto-colors
      SALOMEDS::Color aColor = SMESHGUI::getPredefinedUniqueColor();
#else                     // old algorithm  for auto-colors
      SALOMEDS::Color aColor = SMESHGUI::getUniqueColor( aReservedColors );
      aReservedColors.append( aColor );
#endif                    // SIMPLE_AUTOCOLOR
      aGroupObject->SetColor( aColor );

      _PTR(SObject) aGroupSObject = SMESH::FindSObject(aGroupObject);
      if ( aGroupSObject ) {
        QColor c;
        int delta;
        if ( SMESH_Actor *anActor = SMESH::FindActorByEntry(aGroupSObject->GetID().c_str())) {
          switch ( aGroupObject->GetType ()) {
          case SMESH::NODE:
            anActor->SetNodeColor( aColor.R, aColor.G, aColor.B ); break;
          case SMESH::EDGE:
            anActor->SetEdgeColor( aColor.R, aColor.G, aColor.B ); break;
          case SMESH::ELEM0D:
            anActor->Set0DColor( aColor.R, aColor.G, aColor.B ); break;
          case SMESH::BALL:
            anActor->SetBallColor( aColor.R, aColor.G, aColor.B ); break;
          case SMESH::VOLUME:
            SMESH::GetColor("SMESH", "volume_color", c, delta, "255,0,170|-100");
            anActor->SetVolumeColor( aColor.R, aColor.G, aColor.B, delta ); break;
          case SMESH::FACE:
          default:
            SMESH::GetColor("SMESH", "fill_color", c, delta, "0,170,255|-100");
            anActor->SetSufaceColor( aColor.R, aColor.G, aColor.B, delta );
          }
        }
      }
    }

    SMESH::RepaintCurrentView();
  }

  void OverallMeshQuality()
  {
    SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if( aSel )
      aSel->selectedObjects( selected );

    if ( selected.IsEmpty() ) return;
    SALOME_ListIteratorOfListIO It( selected );
    for ( ; It.More(); It.Next() ) {
      SMESHGUI_CtrlInfoDlg* ctrlDlg = new SMESHGUI_CtrlInfoDlg( SMESHGUI::desktop() );
      ctrlDlg->showInfo( It.Value() );
      ctrlDlg->show();
    }
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
    else if ( dynamic_cast< SMESH::Controls::NodeConnectivityNumber* >( f.get() ) )
      type = QObject::tr( "NODE_CONNECTIVITY_NB" );
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
        if ( anActor &&
             anActor->GetScalarBarActor() &&
             anActor->GetControlMode() != SMESH_Actor::eNone )
        {
          SMESH_ScalarBarActor* aScalarBarActor = anActor->GetScalarBarActor();
          SMESH::Controls::FunctorPtr aFunctor = anActor->GetFunctor();
          if ( aScalarBarActor && aFunctor ) {
            SMESH::Controls::NumericalFunctor* aNumFun =
              dynamic_cast<SMESH::Controls::NumericalFunctor*>( aFunctor.get() );
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
              bool isLogarithmic = lookupTable->GetScale() == VTK_SCALE_LOG10;
              std::vector<int>    nbEvents;
              std::vector<double> funValues;
              aNumFun->GetHistogram( nbIntervals, nbEvents, funValues,
                                     elements, minmax, isLogarithmic );
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
                  for ( int i = 0; i < (int)qMin( nbEvents.size(), funValues.size()-1 ); i++ )
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

  void ShowElement( int theCommandID )
  {
    LightApp_SelectionMgr* aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if ( aSel )
      aSel->selectedObjects( selected );

    if ( selected.Extent() == 1 ) {
      Handle(SALOME_InteractiveObject) anIO = selected.First();
      if ( anIO->hasEntry() ) {
        SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() );
        if ( anActor &&
             anActor->GetScalarBarActor() &&
             anActor->GetControlMode() != SMESH_Actor::eNone )
        {
          SMESH_ScalarBarActor *aScalarBarActor = anActor->GetScalarBarActor();
          if ( theCommandID == SMESHOp::OpShowDistribution ) {
            aScalarBarActor->SetDistributionVisibility(!aScalarBarActor->GetDistributionVisibility());
          }
          else if ( theCommandID == SMESHOp::OpShowScalarBar ) {
            aScalarBarActor->SetVisibility( !aScalarBarActor->GetVisibility());
          }
        }
      }
    }
  }

#ifndef DISABLE_PLOT2DVIEWER
  void PlotDistribution()
  {
    SalomeApp_Application* app =
      dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
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

        SUIT_ViewManager* aViewManager =
          app->getViewManager( Plot2d_Viewer::Type(), true ); // create if necessary
        if( !aViewManager )
          return;

        SPlot2d_Viewer* aView = dynamic_cast<SPlot2d_Viewer*>(aViewManager->getViewModel());
        if ( !aView )
          return;

        Plot2d_ViewFrame* aPlot = aView->getActiveViewFrame();
        if ( !aPlot )
          return;

        if ( anActor && anActor->GetControlMode() != SMESH_Actor::eNone )
        {
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

  void DisableAutoColor()
  {
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if ( aSel )
      aSel->selectedObjects( selected );

    if ( selected.Extent() ) {
      Handle(SALOME_InteractiveObject) anIObject = selected.First();
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIObject);
      if ( !aMesh->_is_nil() ) {
        aMesh->SetAutoColor( false );
      }
    }
  }

  void sortChildren()
  {
    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SALOME_ListIO selected;
    if ( aSel ) {
      aSel->selectedObjects( selected );
      if ( selected.Extent() )
      {
        Handle(SALOME_InteractiveObject) anIObject = selected.First();
        _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
        _PTR(SObject) aSObj = aStudy->FindObjectID(anIObject->getEntry());
        if (aSObj) {
          if ( aStudy->GetUseCaseBuilder()->SortChildren( aSObj, true/*AscendingOrder*/ ) ) {
            SMESHGUI::GetSMESHGUI()->updateObjBrowser();
          }
        }
      }
    }
  }

  void SetDisplayMode(int theCommandID, SMESHGUI_StudyId2MarkerMap& theMarkerMap)
  {
    SALOME_ListIO selected;
    SalomeApp_Application* app =
      dynamic_cast< SalomeApp_Application* >( SUIT_Session::session()->activeApplication() );
    if ( !app )
      return;

    LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
    SalomeApp_Study*   appStudy = dynamic_cast<SalomeApp_Study*>( app->activeStudy() );
    if ( !aSel || !appStudy )
      return;

    if ( theCommandID == SMESHOp::OpClipping ) { // Clipping dialog can be activated without selection
      if ( SMESHGUI* aModule = SMESHGUI::GetSMESHGUI() ) {
        aModule->EmitSignalDeactivateDialog();
        if( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( aModule ) )
          (new SMESHGUI_ClippingDlg( aModule, aViewWindow ))->show();
      }
      return;
    }

    _PTR(Study) aStudy = appStudy->studyDS();

    aSel->selectedObjects( selected );

    if ( selected.Extent() >= 1 )
    {
      switch ( theCommandID ) {
      case SMESHOp::OpTransparency:
      {
        SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
        (new SMESHGUI_TransparencyDlg( SMESHGUI::GetSMESHGUI() ))->show();
        return;
      }
      case SMESHOp::OpProperties:
      {
        double color[3];
        QColor faceColor, edgeColor, nodeColor, elem0dColor, ballColor;
        QColor orientationColor, outlineColor, volumeColor;
        int deltaF = 0, deltaV = 0;
        int elem0dSize   = 1;
        //int ballSize     = 1;
        double ballScale = 1.0;
        int edgeWidth    = 1;
        int outlineWidth = 1;
        double shrinkCoef = 0.0;
        double orientationScale = 0.0;
        bool orientation3d = false;
        VTK::MarkerType markerType = VTK::MT_NONE;
        VTK::MarkerScale markerScale = VTK::MS_NONE;
        int markerId = 0;
        bool hasNodes = false;
        int presentEntities = 0;
        bool firstTime  = true;

        SALOME_ListIteratorOfListIO It( selected );
        for ( ; It.More(); It.Next() ) {
          Handle(SALOME_InteractiveObject) IObject = It.Value();
          if ( !IObject->hasEntry() ) continue;
          SMESH_Actor* anActor = SMESH::FindActorByEntry( IObject->getEntry() );
          if ( !anActor || !anActor->GetObject() ) continue;

          if ( firstTime ) {
            // nodes: color, marker
            anActor->GetNodeColor( color[0], color[1], color[2] );
            nodeColor.setRgbF( color[0], color[1], color[2] );
            markerType  = anActor->GetMarkerType();
            markerScale = anActor->GetMarkerScale();
            markerId    = anActor->GetMarkerTexture();
            // edges: color, width
            anActor->GetEdgeColor( color[0], color[1], color[2] );
            edgeColor.setRgbF( color[0], color[1], color[2] );
            edgeWidth = qMax( (int)anActor->GetLineWidth(), 1 ); // minimum allowed width is 1
            // faces: front color, back color (delta)
            anActor->GetSufaceColor( color[0], color[1], color[2], deltaF );
            faceColor.setRgbF( color[0], color[1], color[2] );
            // faces: front color, back color (delta)
            anActor->GetVolumeColor( color[0], color[1], color[2], deltaV );
            volumeColor.setRgbF( color[0], color[1], color[2] );
            // 0d elements: color, size
            anActor->Get0DColor( color[0], color[1], color[2] );
            elem0dColor.setRgbF( color[0], color[1], color[2] );
            elem0dSize = qMax( (int)anActor->Get0DSize(), 1 ); // minimum allowed size is 1
            // balls: color, size
            anActor->GetBallColor( color[0], color[1], color[2] );
            ballColor.setRgbF( color[0], color[1], color[2] );
            //ballSize = qMax( (int)anActor->GetBallSize(), 1 ); // minimum allowed size is 1
            ballScale = qMax( (double)anActor->GetBallScale(), 1e-2 ); // minimum allowed scale is 1e-2
            // outlines: color
            anActor->GetOutlineColor( color[0], color[1], color[2] );
            outlineColor.setRgbF( color[0], color[1], color[2] );
            outlineWidth = qMax( (int)anActor->GetOutlineWidth(), 1 ); // minimum allowed width is 1
            // orientation vectors: color, scale, 3d flag
            anActor->GetFacesOrientationColor( color[0], color[1], color[2] );
            orientationColor.setRgbF( color[0], color[1], color[2] );
            orientationScale = anActor->GetFacesOrientationScale();
            orientation3d = anActor->GetFacesOrientation3DVectors();
            // shrink factor
            shrinkCoef = anActor->GetShrinkFactor();
          }

          firstTime = false; // we only take properties from first object (for performance reasons)

          if ( !hasNodes )
            hasNodes = anActor->GetObject()->GetNbEntities( SMDSAbs_Node );
          if ( !(presentEntities & SMESH_Actor::eEdges) && anActor->GetObject()->GetNbEntities( SMDSAbs_Edge ) )
            presentEntities = presentEntities | SMESH_Actor::eEdges;
          if ( !(presentEntities & SMESH_Actor::eFaces) && anActor->GetObject()->GetNbEntities( SMDSAbs_Face ) )
            presentEntities = presentEntities | SMESH_Actor::eFaces;
          if ( !(presentEntities & SMESH_Actor::eVolumes) && anActor->GetObject()->GetNbEntities( SMDSAbs_Volume ) )
            presentEntities = presentEntities | SMESH_Actor::eVolumes;
          if ( !(presentEntities & SMESH_Actor::e0DElements) && anActor->GetObject()->GetNbEntities( SMDSAbs_0DElement ) )
            presentEntities = presentEntities | SMESH_Actor::e0DElements;
          if ( !(presentEntities & SMESH_Actor::eBallElem) && anActor->GetObject()->GetNbEntities( SMDSAbs_Ball ) )
            presentEntities = presentEntities | SMESH_Actor::eBallElem;
          
          // as we know that all types of elements are present, we can exit the loop
          if ( presentEntities == SMESH_Actor::eAllEntity )
            break;
        }

        SMESHGUI_PropertiesDlg dlg( theMarkerMap[ aStudy->StudyId() ], SMESHGUI::desktop() );
        // nodes: color, marker
        dlg.setNodeColor( nodeColor );
        if( markerType != VTK::MT_USER )
          dlg.setNodeMarker( markerType, markerScale );
        else
          dlg.setNodeCustomMarker( markerId );
        // edges: color, line width
        dlg.setEdgeColor( edgeColor );
        dlg.setEdgeWidth( edgeWidth );
        // faces: front color, back color
        dlg.setFaceColor( faceColor, deltaF );
        // volumes: normal color, reversed color
        dlg.setVolumeColor( volumeColor, deltaV );
        // outlines: color, line width
        dlg.setOutlineColor( outlineColor );
        dlg.setOutlineWidth( outlineWidth );
        // 0d elements: color, size
        dlg.setElem0dColor( elem0dColor );
        dlg.setElem0dSize( elem0dSize );
        // balls: color, size
        dlg.setBallColor( ballColor );
        //dlg.setBallSize( ballSize );
        dlg.setBallScale( ballScale );
        // orientation: color, scale, 3d flag
        dlg.setOrientationColor( orientationColor );
        dlg.setOrientationSize( int( orientationScale * 100. ) );
        dlg.setOrientation3d( orientation3d );
        // shrink: scale factor
        dlg.setShrinkCoef( int( shrinkCoef * 100. ) );
        // hide unused controls
        dlg.showControls( presentEntities, hasNodes );
        
        if ( dlg.exec() ) {
          nodeColor        = dlg.nodeColor();
          markerType       = dlg.nodeMarkerType();
          markerScale      = dlg.nodeMarkerScale();
          markerId         = dlg.nodeMarkerId();
          edgeColor        = dlg.edgeColor();
          edgeWidth        = dlg.edgeWidth();
          faceColor        = dlg.faceColor();
          deltaF           = dlg.faceColorDelta();
          volumeColor      = dlg.volumeColor();
          deltaV           = dlg.volumeColorDelta();
          outlineColor     = dlg.outlineColor();
          outlineWidth     = dlg.outlineWidth();
          elem0dColor      = dlg.elem0dColor();
          elem0dSize       = dlg.elem0dSize();
          ballColor        = dlg.ballColor();
         // ballSize         = dlg.ballSize();
          ballScale        = dlg.ballScale();
          orientationColor = dlg.orientationColor();
          orientationScale = dlg.orientationSize() / 100.;
          orientation3d    = dlg.orientation3d();
          shrinkCoef       = dlg.shrinkCoef() / 100.;

          // store point markers map that might be changed by the user
          theMarkerMap[ aStudy->StudyId() ] = dlg.customMarkers();

          // set properties from dialog box to the presentations
          SALOME_ListIteratorOfListIO It( selected );
          for ( ; It.More(); It.Next() ) {
            Handle(SALOME_InteractiveObject) IObject = It.Value();
            if ( !IObject->hasEntry() ) continue;
            SMESH_Actor* anActor = SMESH::FindActorByEntry( IObject->getEntry() );
            if ( !anActor ) continue;
            
            // nodes: color, marker
            anActor->SetNodeColor( nodeColor.redF(), nodeColor.greenF(), nodeColor.blueF() );
            if ( markerType != VTK::MT_USER ) {
              anActor->SetMarkerStd( markerType, markerScale );
            }
            else {
              const VTK::MarkerMap& markerMap = theMarkerMap[ aStudy->StudyId() ];
              VTK::MarkerMap::const_iterator iter = markerMap.find( markerId );
              if ( iter != markerMap.end() )
                anActor->SetMarkerTexture( markerId, iter->second.second );
            }
            // volumes: normal color, reversed color (delta)
            anActor->SetVolumeColor( volumeColor.redF(), volumeColor.greenF(), volumeColor.blueF(), deltaV );
            // faces: front color, back color (delta)
            anActor->SetSufaceColor( faceColor.redF(), faceColor.greenF(), faceColor.blueF(), deltaF );
            // edges: color, width
            anActor->SetEdgeColor( edgeColor.redF(), edgeColor.greenF(), edgeColor.blueF() );
            anActor->SetLineWidth( edgeWidth );
            // outlines: color
            anActor->SetOutlineColor( outlineColor.redF(), outlineColor.greenF(), outlineColor.blueF() );
            anActor->SetOutlineWidth( outlineWidth );
            // 0D elements: color, size
            anActor->Set0DColor( elem0dColor.redF(), elem0dColor.greenF(), elem0dColor.blueF() );
            anActor->Set0DSize( elem0dSize );
            // balls: color, size
            anActor->SetBallColor( ballColor.redF(), ballColor.greenF(), ballColor.blueF() );
            // anActor->SetBallSize( ballSize );
            anActor->SetBallScale( ballScale );
            // orientation: color, scale, 3d flag
            anActor->SetFacesOrientationColor( orientationColor.redF(), orientationColor.greenF(), orientationColor.blueF() );
            anActor->SetFacesOrientationScale( orientationScale );
            anActor->SetFacesOrientation3DVectors( orientation3d );
            // shrink factor
            anActor->SetShrinkFactor( shrinkCoef );

            // for groups, set also proper color
            SMESH::SMESH_GroupBase_var aGroupObject = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IObject);
            if ( !aGroupObject->_is_nil() ) {
              SMESH::ElementType anElementType = aGroupObject->GetType();
              QColor aColor;
              switch( anElementType ) {
              case SMESH::NODE:
                aColor = nodeColor; break;
              case SMESH::EDGE:
                aColor = edgeColor; break;
              case SMESH::FACE: 
                aColor = faceColor; break;
              case SMESH::VOLUME:
                aColor = volumeColor; break;
              case SMESH::ELEM0D: 
                aColor = elem0dColor; break;
              case SMESH::BALL: 
                aColor = ballColor; break;
              default: break;
              }
              
              if ( aColor.isValid() ) {
                SALOMEDS::Color aGroupColor;
                aGroupColor.R = aColor.redF();
                aGroupColor.G = aColor.greenF();
                aGroupColor.B = aColor.blueF();
                aGroupObject->SetColor( aGroupColor );
              }
            } // if ( !aGroupObject->_is_nil() )
          } // for ( ; It.More(); It.Next() )
          SMESH::RepaintCurrentView();
        } // if ( dlg.exec() )
        return;
      } // case SMESHOp::OpProperties:
      } // switch(theCommandID)
      SUIT_OverrideCursor wc;
      SALOME_ListIteratorOfListIO It( selected );
      for( ; It.More(); It.Next()){
        Handle(SALOME_InteractiveObject) IObject = It.Value();
        if(IObject->hasEntry()){
          if(SMESH_Actor *anActor = SMESH::FindActorByEntry(IObject->getEntry())){
            switch(theCommandID){
            case SMESHOp::OpDMWireframe:
              anActor->SetRepresentation(SMESH_Actor::eEdge);
              break;
            case SMESHOp::OpDMShading:
              anActor->SetRepresentation(SMESH_Actor::eSurface);
              break;
            case SMESHOp::OpDMShrink:
              if(anActor->IsShrunk())
                anActor->UnShrink();
              else
                anActor->SetShrink();
              break;
            case SMESHOp::OpDMNodes:
              anActor->SetRepresentation(SMESH_Actor::ePoint);
              break;
            case SMESHOp::OpRepresentationLines:
              if(anActor->GetQuadratic2DRepresentation() != SMESH_Actor::eLines)
                anActor->SetQuadratic2DRepresentation(SMESH_Actor::eLines);
              break;
            case SMESHOp::OpRepresentationArcs:
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

  int ActionToControl( int theID, bool theReversed )
  {
    NCollection_DoubleMap<int,int> ActionControl;
    ActionControl.Bind( 0,                                SMESH_Actor::eNone );
    ActionControl.Bind( SMESHOp::OpFreeNode,              SMESH_Actor::eFreeNodes );
    ActionControl.Bind( SMESHOp::OpEqualNode,             SMESH_Actor::eCoincidentNodes );
    ActionControl.Bind( SMESHOp::OpNodeConnectivityNb,    SMESH_Actor::eNodeConnectivityNb );
    ActionControl.Bind( SMESHOp::OpFreeEdge,              SMESH_Actor::eFreeEdges );
    ActionControl.Bind( SMESHOp::OpFreeBorder,            SMESH_Actor::eFreeBorders );
    ActionControl.Bind( SMESHOp::OpLength,                SMESH_Actor::eLength );
    ActionControl.Bind( SMESHOp::OpConnection,            SMESH_Actor::eMultiConnection );
    ActionControl.Bind( SMESHOp::OpEqualEdge,             SMESH_Actor::eCoincidentElems1D );
    ActionControl.Bind( SMESHOp::OpFreeFace,              SMESH_Actor::eFreeFaces );
    ActionControl.Bind( SMESHOp::OpBareBorderFace,        SMESH_Actor::eBareBorderFace );
    ActionControl.Bind( SMESHOp::OpOverConstrainedFace,   SMESH_Actor::eOverConstrainedFace );
    ActionControl.Bind( SMESHOp::OpLength2D,              SMESH_Actor::eLength2D );
    ActionControl.Bind( SMESHOp::OpConnection2D,          SMESH_Actor::eMultiConnection2D );
    ActionControl.Bind( SMESHOp::OpArea,                  SMESH_Actor::eArea );
    ActionControl.Bind( SMESHOp::OpTaper,                 SMESH_Actor::eTaper );
    ActionControl.Bind( SMESHOp::OpAspectRatio,           SMESH_Actor::eAspectRatio );
    ActionControl.Bind( SMESHOp::OpMinimumAngle,          SMESH_Actor::eMinimumAngle );
    ActionControl.Bind( SMESHOp::OpWarpingAngle,          SMESH_Actor::eWarping );
    ActionControl.Bind( SMESHOp::OpSkew,                  SMESH_Actor::eSkew );
    ActionControl.Bind( SMESHOp::OpMaxElementLength2D,    SMESH_Actor::eMaxElementLength2D );
    ActionControl.Bind( SMESHOp::OpEqualFace,             SMESH_Actor::eCoincidentElems2D );
    ActionControl.Bind( SMESHOp::OpAspectRatio3D,         SMESH_Actor::eAspectRatio3D );
    ActionControl.Bind( SMESHOp::OpVolume,                SMESH_Actor::eVolume3D );
    ActionControl.Bind( SMESHOp::OpMaxElementLength3D,    SMESH_Actor::eMaxElementLength3D );
    ActionControl.Bind( SMESHOp::OpBareBorderVolume,      SMESH_Actor::eBareBorderVolume );
    ActionControl.Bind( SMESHOp::OpOverConstrainedVolume, SMESH_Actor::eOverConstrainedVolume );
    ActionControl.Bind( SMESHOp::OpEqualVolume,           SMESH_Actor::eCoincidentElems3D );

    return theReversed ? ActionControl.Find2( theID ) : ActionControl.Find1( theID );
  }

  void Control( int theCommandID )
  {
    SMESH_Actor::eControl aControl = SMESH_Actor::eControl( ActionToControl( theCommandID ));
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();

    SALOME_ListIO selected;
    if ( LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr() )
      aSel->selectedObjects( selected );

    if ( !selected.IsEmpty() ) {
      SALOME_ListIteratorOfListIO It(selected);
      for ( ; It.More(); It.Next())
      {
        Handle(SALOME_InteractiveObject) anIO = It.Value();
        if ( !anIO.IsNull() ) {
          _PTR(SObject) SO = aStudy->FindObjectID( It.Value()->getEntry() );
          if ( SO ) {
            CORBA::Object_var         aObject = SMESH::SObjectToObject( SO );
            SMESH::SMESH_IDSource_var anIDSrc = SMESH::SMESH_IDSource::_narrow( aObject );
            if ( !anIDSrc->_is_nil() ) {
              SMESH_Actor *anActor = SMESH::FindActorByEntry( anIO->getEntry());
              if (( !anActor && selected.Extent() == 1 ) &&
                  ( anActor = SMESH::CreateActor( aStudy, anIO->getEntry() )))
              {
                anActor->SetControlMode( aControl );
                SMESH::DisplayActor( SMESH::GetCurrentVtkView(), anActor );
                SMESH::UpdateView  ( SMESH::eDisplay, anIO->getEntry() );
              }
              if ( anActor )
              {
                if ( anActor->GetControlMode() != aControl )
                  anActor->SetControlMode( aControl );
                QString functorName = functorToString( anActor->GetFunctor() );
                int anEntitiesCount = anActor->GetNumberControlEntities();
                if (anEntitiesCount >= 0)
                  functorName = functorName + ": " + QString::number(anEntitiesCount);
                anActor->GetScalarBarActor()->SetTitle( functorName.toLatin1().constData() );
                SMESH::RepaintCurrentView();
#ifndef DISABLE_PLOT2DVIEWER
                if ( anActor->GetPlot2Histogram() ) {
                  SPlot2d_Histogram* aHistogram = anActor->UpdatePlot2Histogram();
                  QString aHistogramName("%1 : %2");
                  aHistogramName = aHistogramName.arg( anIO->getName() ).arg( functorName );
                  aHistogram->setName( aHistogramName );
                  aHistogram->setHorTitle( functorName );
                  SMESH::ProcessIn2DViewers( anActor );
                }
#endif
              }
            }
          }
        }
      }
    }
  }


  bool CheckOIType(const Handle(SALOME_InteractiveObject) & theIO,
                   SMESH::MeshObjectType                    theType,
                   const QString                            theInTypeName,
                   QString &                                theOutTypeName)
  {
    SMESH_TypeFilter aTypeFilter( theType );
    QString entry;
    if ( !theIO.IsNull() )
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
    _PTR(Study)  aStudy = SMESH::GetActiveStudyDocument();
    _PTR(SObject) aSObj = aStudy->FindObjectID(theIO->getEntry());
    if (aSObj) {
      _PTR(SComponent) aSComp = aSObj->GetFatherComponent();
      CORBA::String_var  anID = aSComp->GetID().c_str();
      if ( !strcmp(anID.in(),theIO->getEntry()) )
        return "Component";
    }

    QString aTypeName;
    if (
        CheckOIType ( theIO, SMESH::HYPOTHESIS,    "Hypothesis", aTypeName ) ||
        CheckOIType ( theIO, SMESH::ALGORITHM,     "Algorithm",  aTypeName ) ||
        CheckOIType ( theIO, SMESH::MESH,          "Mesh",       aTypeName ) ||
        CheckOIType ( theIO, SMESH::SUBMESH,       "SubMesh",    aTypeName ) ||
        CheckOIType ( theIO, SMESH::GROUP,         "Group",      aTypeName )
        )
      return aTypeName;

    return "NoType";
  }


  // QString CheckHomogeneousSelection()
  // {
  //   LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
  //   SALOME_ListIO selected;
  //   if ( aSel )
  //     aSel->selectedObjects( selected );

  //   QString RefType = CheckTypeObject(selected.First());
  //   SALOME_ListIteratorOfListIO It(selected);
  //   for ( ; It.More(); It.Next())
  //   {
  //     Handle(SALOME_InteractiveObject) IObject = It.Value();
  //     QString Type = CheckTypeObject(IObject);
  //     if ( Type.compare(RefType) != 0 )
  //       return "Heterogeneous Selection";
  //   }

  //   return RefType;
  // }

  uint randomize( uint size )
  {
    static bool initialized = false;
    if ( !initialized ) {
      qsrand( QDateTime::currentDateTime().toTime_t() );
      initialized = true;
    }
    uint v = qrand();
    v = uint( (double)( v ) / RAND_MAX * size );
    v = qMax( uint(0), qMin ( v, size-1 ) );
    return v;
  }
  
} //namespace

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
      std::list< _PTR(SObject) >::iterator itSO = --listSO.end();
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

  // Call mesh->Clear() to prevent loading mesh from file caused by hypotheses removal
  for( It.Initialize( selected ); It.More(); It.Next()) // loop on selected IO's
  {
    Handle(SALOME_InteractiveObject) IObject = It.Value();
    SMESH::SMESH_Mesh_var mesh = SMESH::IObjectToInterface< SMESH::SMESH_Mesh >( IObject );
    if ( !mesh->_is_nil() )
      mesh->Clear();
  }

  // Treat SO's in the list starting from the back
  aStudyBuilder->NewCommand();  // There is a transaction
  for ( ritSO = listSO.rbegin(); ritSO != listSO.rend(); ++ritSO )
  {
    _PTR(SObject) SO = *ritSO;
    if ( !SO ) continue;
    std::string anEntry = SO->GetID();

    /** Erase graphical object and remove all its data **/
    if(SO->FindAttribute(anAttr, "AttributeIOR")) {
      SMESH::RemoveVisualObjectWithActors( anEntry.c_str(), true);
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
SalomeApp_Module( "SMESH" )
{
  if ( CORBA::is_nil( myComponentSMESH ) )
  {
    CORBA::Boolean anIsEmbeddedMode;
    myComponentSMESH = SMESH_Client::GetSMESHGen(getApp()->orb(),anIsEmbeddedMode);
    //MESSAGE("-------------------------------> anIsEmbeddedMode=" << anIsEmbeddedMode);

    //  0019923: EDF 765 SMESH : default values of hypothesis
    SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr(this);
    int nbSeg = aResourceMgr->integerValue( "SMESH", "segmentation", 10 );
    myComponentSMESH->SetBoundaryBoxSegmentation( nbSeg );
    nbSeg = aResourceMgr->integerValue( "SMESH", "nb_segments_per_edge", 15 );
    myComponentSMESH->SetDefaultNbSegments( nbSeg );

    const char* options[] = { "historical_python_dump", "forget_mesh_on_hyp_modif", "default_grp_color" };
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
bool SMESHGUI::automaticUpdate( SMESH::SMESH_IDSource_ptr theMesh,
                                int* entities, bool* limitExceeded, int* hidden, long* nbElements )
{
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
  if ( !resMgr )
    return false;

  bool autoUpdate  = resMgr->booleanValue( "SMESH", "auto_update", false );
  long updateLimit = resMgr->integerValue( "SMESH", "update_limit", 500000 );
  bool incrementalLimit = resMgr->booleanValue( "SMESH", "incremental_limit", false );

  SMESH::long_array_var info = theMesh->GetMeshInfo();
  long nbOdElems = info[SMDSEntity_0D];
  long nbEdges   = info[SMDSEntity_Edge] + info[SMDSEntity_Quad_Edge];
  long nbFaces   = info[SMDSEntity_Triangle]   + info[SMDSEntity_Quad_Triangle]   + info[SMDSEntity_BiQuad_Triangle] + 
                   info[SMDSEntity_Quadrangle] + info[SMDSEntity_Quad_Quadrangle] + info[SMDSEntity_BiQuad_Quadrangle] + 
                   info[SMDSEntity_Polygon] + info[SMDSEntity_Quad_Polygon];
  long nbVolumes = info[SMDSEntity_Tetra]   + info[SMDSEntity_Quad_Tetra] + 
                   info[SMDSEntity_Hexa]    + info[SMDSEntity_Quad_Hexa] + info[SMDSEntity_TriQuad_Hexa] + 
                   info[SMDSEntity_Pyramid] + info[SMDSEntity_Quad_Pyramid] + 
                   info[SMDSEntity_Penta]   + info[SMDSEntity_Quad_Penta] + 
                   info[SMDSEntity_Polyhedra] + 
                   info[SMDSEntity_Hexagonal_Prism];
  long nbBalls   = info[SMDSEntity_Ball];

  long requestedSize = nbOdElems + nbBalls + nbEdges + nbFaces + nbVolumes;
  *nbElements = requestedSize;
  
  *entities = SMESH_Actor::eAllEntity;
  *hidden   = 0;

  bool exceeded = updateLimit > 0 && requestedSize > updateLimit;

  if ( limitExceeded ) *limitExceeded = autoUpdate && exceeded;

  if ( incrementalLimit ) {
    long total     = 0;

    if ( nbOdElems > 0 ) {
      if ( total + nbOdElems > updateLimit ) {
        *entities = *entities & ~SMESH_Actor::e0DElements;
        *hidden = *hidden | SMESH_Actor::e0DElements;
      }
      else
        exceeded = false;
    }
    total += nbOdElems;

    if ( nbEdges > 0 ) {
      if ( total + nbEdges > updateLimit ) {
        *entities = *entities & ~SMESH_Actor::eEdges;
        *hidden = *hidden | SMESH_Actor::eEdges;
      }
      else
        exceeded = false;
    }
    total += nbEdges;

    if ( nbFaces > 0 ) {
      if ( total + nbFaces > updateLimit ) {
        *entities = *entities & ~SMESH_Actor::eFaces;
        *hidden = *hidden | SMESH_Actor::eFaces;
      }
      else
        exceeded = false;
    }
    total += nbFaces;

    if ( nbVolumes > 0 ) {
      if ( total + nbVolumes > updateLimit ) {
        *entities = *entities & ~SMESH_Actor::eVolumes;
        *hidden = *hidden | SMESH_Actor::eVolumes;
      }
      else
        exceeded = false;
    }
    total += nbVolumes;

    if ( nbBalls > 0 ) {
      if ( total + nbBalls > updateLimit ) {
        *entities = *entities & ~SMESH_Actor::eBallElem;
        *hidden = *hidden | SMESH_Actor::eBallElem;
      }
      else
        exceeded = false;
    }
    total += nbBalls;
  }

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
void SMESHGUI::EmitSignalCloseView()
{
  emit SignalCloseView();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESHGUI::EmitSignalActivatedViewManager()
{
  emit SignalActivatedViewManager();
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
  case SMESHOp::OpDelete:
    if(checkLock(aStudy)) break;
    OnEditDelete();
    break;
  case SMESHOp::OpImportDAT:
  case SMESHOp::OpImportUNV:
  case SMESHOp::OpImportMED:
  case SMESHOp::OpImportSTL:
#ifdef WITH_CGNS
  case SMESHOp::OpImportCGNS:
#endif
  case SMESHOp::OpImportSAUV:
  case SMESHOp::OpImportGMF:
    {
      if(checkLock(aStudy)) break;
      ::ImportMeshesFromFile(GetSMESHGen(),theCommandID);
      break;
    }

  case SMESHOp::OpFileInformation:
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
  case SMESHOp::OpExportDAT:
  case SMESHOp::OpExportMED:
  case SMESHOp::OpExportUNV:
  case SMESHOp::OpExportSTL:
#ifdef WITH_CGNS
  case SMESHOp::OpExportCGNS:
#endif
  case SMESHOp::OpExportSAUV:
  case SMESHOp::OpExportGMF:
  case SMESHOp::OpPopupExportDAT:
  case SMESHOp::OpPopupExportMED:
  case SMESHOp::OpPopupExportUNV:
  case SMESHOp::OpPopupExportSTL:
#ifdef WITH_CGNS
  case SMESHOp::OpPopupExportCGNS:
#endif
  case SMESHOp::OpPopupExportSAUV:
  case SMESHOp::OpPopupExportGMF:
    {
      ::ExportMeshToFile(theCommandID);
      break;
    }

  case SMESHOp::OpReset:                      // SCALAR BAR
    {
      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO selected;
      if( aSel )
        aSel->selectedObjects( selected );

      SALOME_ListIteratorOfListIO it(selected);
      for( ; it.More(); it.Next()) {
        Handle(SALOME_InteractiveObject) anIO = it.Value();
        if( anIO->hasEntry() ) {
          if( SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() ) ) {
            anActor->SetControlMode( SMESH_Actor::eNone );
#ifndef DISABLE_PLOT2DVIEWER
            SMESH::ProcessIn2DViewers(anActor,SMESH::RemoveFrom2dViewer);
#endif
          }
        }
      }
      SMESH::UpdateView();
      break;
    }
  case SMESHOp::OpScalarBarProperties:
    {
      SMESHGUI_Preferences_ScalarBarDlg::ScalarBarProperties( this );
      break;
    }
  case SMESHOp::OpShowScalarBar:
    {
      // show/hide scalar bar
      ::ShowElement(theCommandID);
      break;
    }
  case SMESHOp::OpSaveDistribution:
    {
      // dump control distribution data to the text file
      ::SaveDistribution();
      break;
    }

  case SMESHOp::OpShowDistribution:
    {
      // show/hide distribution
      ::ShowElement(theCommandID);
      break;
    }

#ifndef DISABLE_PLOT2DVIEWER
  case SMESHOp::OpPlotDistribution:
    {
      // plot distribution
      ::PlotDistribution();
      break;
    }
#endif

    // Auto-color
  case SMESHOp::OpAutoColor:
    ::AutoColor();
  break;

  case SMESHOp::OpDisableAutoColor:
    ::DisableAutoColor();
  break;

  case SMESHOp::OpClipping:
  case SMESHOp::OpTransparency:
  case SMESHOp::OpProperties: // Display preferences (colors, shrink size, line width, ...)

    // Display Mode
  case SMESHOp::OpDMWireframe:
  case SMESHOp::OpDMShading:
  case SMESHOp::OpDMNodes:
  case SMESHOp::OpDMShrink:
    ::SetDisplayMode(theCommandID, myMarkerMap);
  break;

  //2D quadratic representation
  case SMESHOp::OpRepresentationLines:
  case SMESHOp::OpRepresentationArcs:
    ::SetDisplayMode(theCommandID, myMarkerMap);
  break;

  // Display Entity
  case SMESHOp::OpDE0DElements:
  case SMESHOp::OpDEEdges:
  case SMESHOp::OpDEFaces:
  case SMESHOp::OpDEVolumes:
  case SMESHOp::OpDEBalls:
  case SMESHOp::OpDEAllEntity:
    ::SetDisplayEntity(theCommandID);
  break;

  // Choose entities to be displayed
  case SMESHOp::OpDEChoose:
    {
      ( new SMESHGUI_DisplayEntitiesDlg( SMESHGUI::desktop() ) )->exec();
      break;
    }

  case SMESHOp::OpOrientationOnFaces:
    {
      SUIT_OverrideCursor wc;
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

  case SMESHOp::OpUpdate:
    {
      if(checkLock(aStudy)) break;
      SUIT_OverrideCursor wc;
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

  case SMESHOp::OpHide:
  case SMESHOp::OpShow:
  case SMESHOp::OpShowOnly:
    {
      SUIT_OverrideCursor wc;
      SMESH::EDisplaing anAction;
      switch (theCommandID) {
      case SMESHOp::OpHide:     anAction = SMESH::eErase; break;
      case SMESHOp::OpShow:     anAction = SMESH::eDisplay; break;
      case SMESHOp::OpShowOnly: anAction = SMESH::eDisplayOnly; break;
      }

      LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr();
      SALOME_ListIO sel_objects, to_process;
      if (aSel)
        aSel->selectedObjects( sel_objects );

      if ( theCommandID==SMESHOp::OpShowOnly )
      {
        //MESSAGE("anAction = SMESH::eDisplayOnly");
        startOperation( myEraseAll );
      }

      extractContainers( sel_objects, to_process );

      try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
        OCC_CATCH_SIGNALS;
#endif
        if (vtkwnd) {
          SALOME_ListIteratorOfListIO It( to_process );
          for ( ; It.More(); It.Next())
          {
            Handle(SALOME_InteractiveObject) IOS = It.Value();
            if ( IOS->hasEntry() )
            {
              if ( !SMESH::UpdateView( anAction, IOS->getEntry() )) {
                SMESHGUI::GetSMESHGUI()->EmitSignalVisibilityChanged();
                break; // PAL16774 (Crash after display of many groups)
              }
              if (anAction == SMESH::eDisplayOnly)
                anAction = SMESH::eDisplay;
            }
          }
        }

        // PAL13338 + PAL15161 -->
        if ( ( theCommandID==SMESHOp::OpShow || theCommandID==SMESHOp::OpShowOnly ) && !checkLock(aStudy)) {
          SMESH::UpdateView();
          SMESHGUI::GetSMESHGUI()->EmitSignalVisibilityChanged();
        }
        // PAL13338 + PAL15161 <--
      }
      catch (...) { // PAL16774 (Crash after display of many groups)
        SMESH::OnVisuException();
      }

      if (anAction == SMESH::eErase) {
        SALOME_ListIO l1;
        aSel->setSelectedObjects( l1 );
      }
      else
        aSel->setSelectedObjects( to_process );

      break;
    }

  case SMESHOp::OpNode:
    {
      if(checkLock(aStudy)) break;

      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();

        ( new SMESHGUI_NodesDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"),tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }

  case SMESHOp::OpCreateMesh:
  case SMESHOp::OpCreateSubMesh:
  case SMESHOp::OpEditMeshOrSubMesh:
  case SMESHOp::OpEditMesh:
  case SMESHOp::OpEditSubMesh:
  case SMESHOp::OpCompute:
  case SMESHOp::OpComputeSubMesh:
  case SMESHOp::OpPreCompute:
  case SMESHOp::OpEvaluate:
  case SMESHOp::OpMeshOrder:
    startOperation( theCommandID );
    break;
  case SMESHOp::OpCopyMesh:
    {
      if (checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      ( new SMESHGUI_CopyMeshDlg( this ) )->show();
    }
    break;
  case SMESHOp::OpBuildCompoundMesh:
    {
      if (checkLock(aStudy)) break;
      EmitSignalDeactivateDialog();
      ( new SMESHGUI_BuildCompoundDlg( this ) )->show();
    }
    break;

  case SMESHOp::OpDiagonalInversion:
  case SMESHOp::OpUnionOfTwoTriangle:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ), tr( "NOT_A_VTK_VIEWER" ) );
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
      if ( theCommandID == SMESHOp::OpDiagonalInversion )
        ( new SMESHGUI_TrianglesInversionDlg( this ) )->show();
      else
        ( new SMESHGUI_UnionOfTwoTrianglesDlg( this ) )->show();
      break;
    }
  case SMESHOp::OpOrientation:
  case SMESHOp::OpUnionOfTriangles:
  case SMESHOp::OpCuttingOfQuadrangles:
  case SMESHOp::OpSplitVolumes:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ), tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();
      SMESHGUI_MultiEditDlg* aDlg = NULL;
      if ( theCommandID == SMESHOp::OpOrientation )
        aDlg = new SMESHGUI_ChangeOrientationDlg(this);
      else if ( theCommandID == SMESHOp::OpUnionOfTriangles )
        aDlg = new SMESHGUI_UnionOfTrianglesDlg(this);
      else if ( theCommandID == SMESHOp::OpSplitVolumes )
        aDlg = new SMESHGUI_SplitVolumesDlg(this);
      else
        aDlg = new SMESHGUI_CuttingOfQuadsDlg(this);

      aDlg->show();
      break;
    }
  case SMESHOp::OpSmoothing:
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_SmoothingDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(), tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpExtrusion:
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_ExtrusionDlg ( this ) )->show();
      } else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpExtrusionAlongAPath:
    {
      if (checkLock(aStudy)) break;
      if (vtkwnd) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_ExtrusionAlongPathDlg( this ) )->show();
      } else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpRevolution:
    {
      if(checkLock(aStudy)) break;
      if( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_RevolutionDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpPatternMapping:
    {
      if ( checkLock( aStudy ) )
        break;
      if ( vtkwnd )
      {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_MeshPatternDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpSplitBiQuadratic:
  case SMESHOp::OpConvertMeshToQuadratic:
  case SMESHOp::OpCreateBoundaryElements: // create 2D mesh from 3D
  case SMESHOp::OpReorientFaces:
  case SMESHOp::OpCreateGeometryGroup:
    {
      startOperation( theCommandID );
      break;
    }
  case SMESHOp::OpCreateGroup:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),tr( "NOT_A_VTK_VIEWER" ) );
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

  case SMESHOp::OpConstructGroup:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),tr( "NOT_A_VTK_VIEWER" ) );
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

  case SMESHOp::OpEditGroup:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),tr( "NOT_A_VTK_VIEWER" ) );
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

  case SMESHOp::OpAddElemGroupPopup:     // Add elements to group
    {
      if(checkLock(aStudy)) break;
      if (myState == 800) {
        SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) myActiveDialogBox;
        if (aDlg) aDlg->onAdd();
      }
      break;
    }

  case SMESHOp::OpRemoveElemGroupPopup:  // Remove elements from group
    {
      if(checkLock(aStudy)) break;
      if (myState == 800) {
        SMESHGUI_GroupDlg *aDlg = (SMESHGUI_GroupDlg*) myActiveDialogBox;
        if (aDlg) aDlg->onRemove();
      }
      break;
    }

  case SMESHOp::OpEditGeomGroupAsGroup:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),tr( "NOT_A_VTK_VIEWER" ) );
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

    case SMESHOp::OpUnionGroups:
    case SMESHOp::OpIntersectGroups:
    case SMESHOp::OpCutGroups:
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();

      SMESHGUI_GroupOpDlg* aDlg = 0;
      if ( theCommandID == SMESHOp::OpUnionGroups )
        aDlg = new SMESHGUI_UnionGroupsDlg( this );
      else if ( theCommandID == SMESHOp::OpIntersectGroups )
        aDlg = new SMESHGUI_IntersectGroupsDlg( this );
      else
        aDlg = new SMESHGUI_CutGroupsDlg( this );

      aDlg->show();

      break;
    }

    case SMESHOp::OpGroupUnderlyingElem: // Create groups of entities from existing groups of superior dimensions
    {
      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();
      SMESHGUI_GroupOpDlg* aDlg = new SMESHGUI_DimGroupDlg( this );
      aDlg->show();

      break;
    }

    case SMESHOp::OpDeleteGroup: // Delete groups with their contents
    {
      if ( !vtkwnd )
      {
        SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),tr( "NOT_A_VTK_VIEWER" ) );
        break;
      }

      if ( checkLock( aStudy ) )
        break;

      EmitSignalDeactivateDialog();

      ( new SMESHGUI_DeleteGroupDlg( this ) )->show();
      break;
    }

  case SMESHOp::OpMeshInformation:
  case SMESHOp::OpWhatIs:
    {
      int page = theCommandID == SMESHOp::OpMeshInformation ? SMESHGUI_MeshInfoDlg::BaseInfo : SMESHGUI_MeshInfoDlg::ElemInfo;
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

  case SMESHOp::OpFindElementByPoint:
    {
      startOperation( theCommandID );
      break;
    }

  case SMESHOp::OpEditHypothesis:
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

        if ( !aHypothesis->_is_nil() )
        {
          SMESHGUI_GenericHypothesisCreator* aCreator =
            SMESH::GetHypothesisCreator( SMESH::toQStr( aHypothesis->GetName() ));
          if (aCreator)
          {
            // set geometry of mesh and sub-mesh to aCreator
            aSel->selectedObjects( selected, "",  /*convertReferences=*/false);
            if ( selected.Extent() == 1 )
            {
              QString subGeomID, meshGeomID;
              Handle(SALOME_InteractiveObject) hypIO = selected.First();
              if ( SMESH::GetGeomEntries( hypIO, subGeomID, meshGeomID ))
              {
                if ( subGeomID.isEmpty() ) subGeomID = meshGeomID;
                aCreator->setShapeEntry( subGeomID );
                aCreator->setMainShapeEntry( meshGeomID );
              }
            }

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
  case SMESHOp::OpUnassign:                      // REMOVE HYPOTHESIS / ALGORITHMS
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

  case SMESHOp::OpElem0D:
  case SMESHOp::OpBall:
  case SMESHOp::OpEdge:
  case SMESHOp::OpTriangle:
  case SMESHOp::OpQuadrangle:
  case SMESHOp::OpPolygon:
  case SMESHOp::OpTetrahedron:
  case SMESHOp::OpHexahedron:
  case SMESHOp::OpPentahedron:
  case SMESHOp::OpPyramid:
  case SMESHOp::OpHexagonalPrism:
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        SMDSAbs_EntityType type = SMDSEntity_Edge;
        switch (theCommandID) {
        case SMESHOp::OpElem0D: type = SMDSEntity_0D;                      break;
        case SMESHOp::OpBall: type = SMDSEntity_Ball;                      break;
        case SMESHOp::OpTriangle: type = SMDSEntity_Triangle;              break;
        case SMESHOp::OpQuadrangle: type = SMDSEntity_Quadrangle;          break;
        case SMESHOp::OpTetrahedron: type = SMDSEntity_Tetra;              break;
        case SMESHOp::OpPolygon: type = SMDSEntity_Polygon;                break;
        case SMESHOp::OpHexahedron: type = SMDSEntity_Hexa;                break;
        case SMESHOp::OpPentahedron: type = SMDSEntity_Penta;              break;
        case SMESHOp::OpPyramid: type = SMDSEntity_Pyramid;                break;
        case SMESHOp::OpHexagonalPrism: type = SMDSEntity_Hexagonal_Prism; break;
        default:;
        }
        ( new SMESHGUI_AddMeshElementDlg( this, type ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpPolyhedron:
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        ( new SMESHGUI_CreatePolyhedralVolumeDlg( this ) )->show();
      }
      else {
        SUIT_MessageBox::warning(desktop(),tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpQuadraticEdge:
  case SMESHOp::OpQuadraticTriangle:
  case SMESHOp::OpBiQuadraticTriangle:
  case SMESHOp::OpQuadraticQuadrangle:
  case SMESHOp::OpBiQuadraticQuadrangle:
  case SMESHOp::OpQuadraticPolygon:
  case SMESHOp::OpQuadraticTetrahedron:
  case SMESHOp::OpQuadraticPyramid:
  case SMESHOp::OpQuadraticPentahedron:
  case SMESHOp::OpQuadraticHexahedron:
  case SMESHOp::OpTriQuadraticHexahedron:
    {
      if(checkLock(aStudy)) break;
      if ( vtkwnd ) {
        EmitSignalDeactivateDialog();
        SMDSAbs_EntityType type = SMDSEntity_Last;

        switch (theCommandID) {
        case SMESHOp::OpQuadraticEdge:          type = SMDSEntity_Quad_Edge; break;
        case SMESHOp::OpQuadraticTriangle:      type = SMDSEntity_Quad_Triangle; break;
        case SMESHOp::OpBiQuadraticTriangle:    type = SMDSEntity_BiQuad_Triangle; break;
        case SMESHOp::OpQuadraticQuadrangle:    type = SMDSEntity_Quad_Quadrangle; break;
        case SMESHOp::OpBiQuadraticQuadrangle:  type = SMDSEntity_BiQuad_Quadrangle; break;
        case SMESHOp::OpQuadraticPolygon:       type = SMDSEntity_Quad_Polygon; break;
        case SMESHOp::OpQuadraticTetrahedron:   type = SMDSEntity_Quad_Tetra; break;
        case SMESHOp::OpQuadraticPyramid:       type = SMDSEntity_Quad_Pyramid; break;
        case SMESHOp::OpQuadraticPentahedron:   type = SMDSEntity_Quad_Penta; break;
        case SMESHOp::OpQuadraticHexahedron:    type = SMDSEntity_Quad_Hexa; break;
        case SMESHOp::OpTriQuadraticHexahedron: type = SMDSEntity_TriQuad_Hexa; break;
        default: break;
        }
        if ( type != SMDSEntity_Last )
          ( new SMESHGUI_AddQuadraticElementDlg( this, type ) )->show();
      }
      else {
        SUIT_MessageBox::warning(SMESHGUI::desktop(),
                                 tr("SMESH_WRN_WARNING"), tr("SMESH_WRN_VIEWER_VTK"));
      }
      break;
    }
  case SMESHOp::OpRemoveNodes:
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
  case SMESHOp::OpRemoveElements:                                    // REMOVES ELEMENTS
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
  case SMESHOp::OpClearMesh: {

    if(checkLock(aStudy)) break;

    SALOME_ListIO selected;
    if( LightApp_SelectionMgr *aSel = SMESHGUI::selectionMgr() )
      aSel->selectedObjects( selected );

    SUIT_OverrideCursor wc;
    SALOME_ListIteratorOfListIO It (selected);
    for ( ; It.More(); It.Next() )
    {
      Handle(SALOME_InteractiveObject) IOS = It.Value();
      SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IOS);
      if ( aMesh->_is_nil()) continue;
      try {
        aMesh->Clear();
        if ( aMesh->NbNodes() == 0 ) // imported mesh is not empty
          SMESH::RemoveVisualObjectWithActors(IOS->getEntry(), true);
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
  case SMESHOp::OpRemoveOrphanNodes:
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
              SUIT_OverrideCursor wc;
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
  case SMESHOp::OpRenumberingNodes:
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
  case SMESHOp::OpRenumberingElements:
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
  case SMESHOp::OpTranslation:
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
  case SMESHOp::OpRotation:
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
  case SMESHOp::OpSymmetry:
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
  case SMESHOp::OpScale:
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

  case SMESHOp::OpSewing:
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
  case SMESHOp::OpMergeNodes:
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
  case SMESHOp::OpMergeElements:
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

  case SMESHOp::OpMoveNode: // MAKE MESH PASS THROUGH POINT
    startOperation( SMESHOp::OpMoveNode );
    break;

  case SMESHOp::OpDuplicateNodes:
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

  case SMESHOp::OpElem0DOnElemNodes: // 0D_ON_ALL_NODES
    startOperation( SMESHOp::OpElem0DOnElemNodes );
    break;

  case SMESHOp::OpSelectFiltersLibrary: // Library of selection filters
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
  // CONTROLS
  case SMESHOp::OpFreeNode:
  case SMESHOp::OpEqualNode:
  case SMESHOp::OpNodeConnectivityNb:
  case SMESHOp::OpFreeEdge:
  case SMESHOp::OpFreeBorder:
  case SMESHOp::OpLength:
  case SMESHOp::OpConnection:
  case SMESHOp::OpEqualEdge:
  case SMESHOp::OpFreeFace:
  case SMESHOp::OpBareBorderFace:
  case SMESHOp::OpOverConstrainedFace:
  case SMESHOp::OpLength2D:
  case SMESHOp::OpConnection2D:
  case SMESHOp::OpArea:
  case SMESHOp::OpTaper:
  case SMESHOp::OpAspectRatio:
  case SMESHOp::OpMinimumAngle:
  case SMESHOp::OpWarpingAngle:
  case SMESHOp::OpSkew:
  case SMESHOp::OpMaxElementLength2D:
  case SMESHOp::OpEqualFace:
  case SMESHOp::OpAspectRatio3D:
  case SMESHOp::OpVolume:
  case SMESHOp::OpMaxElementLength3D:
  case SMESHOp::OpBareBorderVolume:
  case SMESHOp::OpOverConstrainedVolume:
  case SMESHOp::OpEqualVolume:
    if ( vtkwnd ) {

      LightApp_SelectionMgr* mgr = selectionMgr();
      SALOME_ListIO selected; mgr->selectedObjects( selected );

      if( !selected.IsEmpty() ) {
        SUIT_OverrideCursor wc;
        ::Control( theCommandID );
        break;
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
  case SMESHOp::OpOverallMeshQuality:
    OverallMeshQuality();
    break;
  case SMESHOp::OpNumberingNodes:
    {
      SUIT_OverrideCursor wc;
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
  case SMESHOp::OpNumberingElements:
    {
      SUIT_OverrideCursor wc;
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
  case SMESHOp::OpPropertiesLength:
  case SMESHOp::OpPropertiesArea:
  case SMESHOp::OpPropertiesVolume:
  case SMESHOp::OpMinimumDistance:
  case SMESHOp::OpBoundingBox:
    {
      int page = SMESHGUI_MeasureDlg::MinDistance;
      if ( theCommandID == SMESHOp::OpBoundingBox )
        page = SMESHGUI_MeasureDlg::BoundingBox;
      else if ( theCommandID == SMESHOp::OpPropertiesLength )
        page = SMESHGUI_MeasureDlg::Length;
      else if ( theCommandID == SMESHOp::OpPropertiesArea )
        page = SMESHGUI_MeasureDlg::Area;
      else if ( theCommandID == SMESHOp::OpPropertiesVolume )
        page = SMESHGUI_MeasureDlg::Volume;

      EmitSignalDeactivateDialog();
      SMESHGUI_MeasureDlg* dlg = new SMESHGUI_MeasureDlg( SMESHGUI::desktop(), page );
      dlg->show();
      break;
    }
  case SMESHOp::OpSortChild:
    ::sortChildren();
    break;

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
  if( !popupMgr()->contains( popupMgr()->actionId( action( id ) ) ) )
    popupMgr()->insert( action( id ), pId, 0 );

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

  //createSMESHAction(  SMESHOp::OpImportDAT, "IMPORT_DAT", "", (Qt::CTRL+Qt::Key_B) );
  createSMESHAction( SMESHOp::OpImportUNV, "IMPORT_UNV", "", (Qt::CTRL+Qt::Key_I) );
  createSMESHAction( SMESHOp::OpImportMED, "IMPORT_MED", "", (Qt::CTRL+Qt::Key_M) );
  //createSMESHAction(  114, "NUM" );
  createSMESHAction( SMESHOp::OpImportSTL, "IMPORT_STL"  );
#ifdef WITH_CGNS
  createSMESHAction( SMESHOp::OpImportCGNS, "IMPORT_CGNS" );
#endif
  createSMESHAction( SMESHOp::OpImportSAUV, "IMPORT_SAUV" );
  createSMESHAction( SMESHOp::OpImportGMF,  "IMPORT_GMF"  );
  createSMESHAction( SMESHOp::OpExportDAT,  "DAT" );
  createSMESHAction( SMESHOp::OpExportMED,  "MED" );
  createSMESHAction( SMESHOp::OpExportUNV,  "UNV" );
  createSMESHAction( SMESHOp::OpExportSTL,  "STL" );
#ifdef WITH_CGNS
  createSMESHAction( SMESHOp::OpExportCGNS, "CGNS");
#endif
  createSMESHAction( SMESHOp::OpExportSAUV,     "SAUV");
  createSMESHAction( SMESHOp::OpExportGMF,      "GMF" );
  createSMESHAction( SMESHOp::OpPopupExportDAT, "DAT" );
  createSMESHAction( SMESHOp::OpPopupExportMED, "MED" );
  createSMESHAction( SMESHOp::OpPopupExportUNV, "UNV" );
  createSMESHAction( SMESHOp::OpPopupExportSTL, "STL" );
#ifdef WITH_CGNS
  createSMESHAction( SMESHOp::OpPopupExportCGNS, "CGNS");
#endif
  createSMESHAction( SMESHOp::OpPopupExportSAUV, "SAUV");
  createSMESHAction( SMESHOp::OpPopupExportGMF,  "GMF" );
  createSMESHAction( SMESHOp::OpFileInformation, "FILE_INFO" );
  createSMESHAction( SMESHOp::OpDelete,          "DELETE", "ICON_DELETE", Qt::Key_Delete );
  createSMESHAction( SMESHOp::OpSelectFiltersLibrary, "SEL_FILTER_LIB" );
  createSMESHAction( SMESHOp::OpCreateMesh,           "CREATE_MESH",             "ICON_DLG_INIT_MESH" );
  createSMESHAction( SMESHOp::OpCreateSubMesh,        "CREATE_SUBMESH",          "ICON_DLG_ADD_SUBMESH" );
  createSMESHAction( SMESHOp::OpEditMeshOrSubMesh,    "EDIT_MESHSUBMESH",        "ICON_DLG_EDIT_MESH" );
  createSMESHAction( SMESHOp::OpEditMesh,             "EDIT_MESH",               "ICON_DLG_EDIT_MESH" );
  createSMESHAction( SMESHOp::OpEditSubMesh,          "EDIT_SUBMESH",            "ICON_DLG_EDIT_MESH" );
  createSMESHAction( SMESHOp::OpBuildCompoundMesh,    "BUILD_COMPOUND",          "ICON_BUILD_COMPOUND" );
  createSMESHAction( SMESHOp::OpCopyMesh,             "COPY_MESH",               "ICON_COPY_MESH" );
  createSMESHAction( SMESHOp::OpCompute,              "COMPUTE",                 "ICON_COMPUTE" );
  createSMESHAction( SMESHOp::OpComputeSubMesh,        "COMPUTE_SUBMESH",         "ICON_COMPUTE" );
  createSMESHAction( SMESHOp::OpPreCompute,           "PRECOMPUTE",              "ICON_PRECOMPUTE" );
  createSMESHAction( SMESHOp::OpEvaluate,             "EVALUATE",                "ICON_EVALUATE" );
  createSMESHAction( SMESHOp::OpMeshOrder,            "MESH_ORDER",              "ICON_MESH_ORDER");
  createSMESHAction( SMESHOp::OpCreateGroup,          "CREATE_GROUP",            "ICON_CREATE_GROUP" );
  createSMESHAction( SMESHOp::OpCreateGeometryGroup,  "CREATE_GEO_GROUP",        "ICON_CREATE_GEO_GROUP" );
  createSMESHAction( SMESHOp::OpConstructGroup,       "CONSTRUCT_GROUP",         "ICON_CONSTRUCT_GROUP" );
  createSMESHAction( SMESHOp::OpEditGroup,            "EDIT_GROUP",              "ICON_EDIT_GROUP" );
  createSMESHAction( SMESHOp::OpEditGeomGroupAsGroup, "EDIT_GEOMGROUP_AS_GROUP", "ICON_EDIT_GROUP" );
  createSMESHAction( SMESHOp::OpUnionGroups,          "UN_GROUP",                "ICON_UNION" );
  createSMESHAction( SMESHOp::OpIntersectGroups,      "INT_GROUP",               "ICON_INTERSECT" );
  createSMESHAction( SMESHOp::OpCutGroups,            "CUT_GROUP",               "ICON_CUT" );
  createSMESHAction( SMESHOp::OpGroupUnderlyingElem,  "UNDERLYING_ELEMS",        "ICON_UNDERLYING_ELEMS" );
  createSMESHAction( SMESHOp::OpAddElemGroupPopup,    "ADD" );
  createSMESHAction( SMESHOp::OpRemoveElemGroupPopup, "REMOVE" );
  createSMESHAction( SMESHOp::OpDeleteGroup,          "DEL_GROUP",               "ICON_DEL_GROUP" );
  createSMESHAction( SMESHOp::OpMeshInformation ,     "ADV_INFO",                "ICON_ADV_INFO" );
  //createSMESHAction( SMESHOp::OpStdInfo, "STD_INFO",        "ICON_STD_INFO" );
  //createSMESHAction( SMESHOp::OpWhatIs, "WHAT_IS",         "ICON_WHAT_IS" ); // VSR: issue #0021242 (eliminate "Mesh Element Information" command)
  createSMESHAction( SMESHOp::OpFindElementByPoint,   "FIND_ELEM",               "ICON_FIND_ELEM" );
  //update
  createSMESHAction( SMESHOp::OpFreeNode,              "FREE_NODE",               "ICON_FREE_NODE",     0, true );
  createSMESHAction( SMESHOp::OpEqualNode,             "EQUAL_NODE",              "ICON_EQUAL_NODE",    0, true );
  createSMESHAction( SMESHOp::OpNodeConnectivityNb,    "NODE_CONNECTIVITY_NB",    "ICON_NODE_CONN_NB",    0, true );
  createSMESHAction( SMESHOp::OpFreeEdge,              "FREE_EDGE",               "ICON_FREE_EDGE",     0, true );
  createSMESHAction( SMESHOp::OpFreeBorder,            "FREE_BORDER",             "ICON_FREE_EDGE_2D",  0, true );
  createSMESHAction( SMESHOp::OpLength,                "LENGTH",                  "ICON_LENGTH",        0, true );
  createSMESHAction( SMESHOp::OpConnection,            "CONNECTION",              "ICON_CONNECTION",    0, true );
  createSMESHAction( SMESHOp::OpEqualEdge,             "EQUAL_EDGE",              "ICON_EQUAL_EDGE",    0, true );
  createSMESHAction( SMESHOp::OpFreeFace,              "FREE_FACES",              "ICON_FREE_FACES",    0, true );
  createSMESHAction( SMESHOp::OpBareBorderFace,        "BARE_BORDER_FACE",        "ICON_BARE_BORDER_FACE",        0, true );
  createSMESHAction( SMESHOp::OpOverConstrainedFace,   "OVER_CONSTRAINED_FACE",   "ICON_OVER_CONSTRAINED_FACE",   0, true );
  createSMESHAction( SMESHOp::OpLength2D,              "LENGTH_2D",               "ICON_LENGTH_2D",     0, true );
  createSMESHAction( SMESHOp::OpConnection2D,          "CONNECTION_2D",           "ICON_CONNECTION_2D", 0, true );
  createSMESHAction( SMESHOp::OpArea,                  "AREA",                    "ICON_AREA",          0, true );
  createSMESHAction( SMESHOp::OpTaper,                 "TAPER",                   "ICON_TAPER",         0, true );
  createSMESHAction( SMESHOp::OpAspectRatio,           "ASPECT",                  "ICON_ASPECT",        0, true );
  createSMESHAction( SMESHOp::OpMinimumAngle,          "MIN_ANG",                 "ICON_ANGLE",         0, true );
  createSMESHAction( SMESHOp::OpWarpingAngle,          "WARP",                    "ICON_WARP",          0, true );
  createSMESHAction( SMESHOp::OpSkew,                  "SKEW",                    "ICON_SKEW",          0, true );
  createSMESHAction( SMESHOp::OpMaxElementLength2D,    "MAX_ELEMENT_LENGTH_2D",   "ICON_MAX_ELEMENT_LENGTH_2D",   0, true );
  createSMESHAction( SMESHOp::OpEqualFace,             "EQUAL_FACE",              "ICON_EQUAL_FACE",    0, true );
  createSMESHAction( SMESHOp::OpAspectRatio3D,         "ASPECT_3D",               "ICON_ASPECT_3D",     0, true );
  createSMESHAction( SMESHOp::OpVolume,                "VOLUME_3D",               "ICON_VOLUME_3D",     0, true );
  createSMESHAction( SMESHOp::OpMaxElementLength3D,    "MAX_ELEMENT_LENGTH_3D",   "ICON_MAX_ELEMENT_LENGTH_3D",   0, true );
  createSMESHAction( SMESHOp::OpBareBorderVolume,      "BARE_BORDER_VOLUME",      "ICON_BARE_BORDER_VOLUME",      0, true );
  createSMESHAction( SMESHOp::OpOverConstrainedVolume, "OVER_CONSTRAINED_VOLUME", "ICON_OVER_CONSTRAINED_VOLUME", 0, true );
  createSMESHAction( SMESHOp::OpEqualVolume,           "EQUAL_VOLUME",            "ICON_EQUAL_VOLUME",  0, true );
  createSMESHAction( SMESHOp::OpOverallMeshQuality,    "OVERALL_MESH_QUALITY" );

  createSMESHAction( SMESHOp::OpNode,                   "NODE",            "ICON_DLG_NODE" );
  createSMESHAction( SMESHOp::OpElem0D,                 "ELEM0D",          "ICON_DLG_ELEM0D" );
  createSMESHAction( SMESHOp::OpElem0DOnElemNodes,      "0D_ON_ALL_NODES", "ICON_0D_ON_ALL_NODES" );
  createSMESHAction( SMESHOp::OpBall,                   "BALL",            "ICON_DLG_BALL" );
  createSMESHAction( SMESHOp::OpEdge,                   "EDGE",            "ICON_DLG_EDGE" );
  createSMESHAction( SMESHOp::OpTriangle,               "TRIANGLE",        "ICON_DLG_TRIANGLE" );
  createSMESHAction( SMESHOp::OpQuadrangle,             "QUAD",            "ICON_DLG_QUADRANGLE" );
  createSMESHAction( SMESHOp::OpPolygon,                "POLYGON",         "ICON_DLG_POLYGON" );
  createSMESHAction( SMESHOp::OpTetrahedron,            "TETRA",           "ICON_DLG_TETRAS" );
  createSMESHAction( SMESHOp::OpHexahedron,             "HEXA",            "ICON_DLG_HEXAS" );
  createSMESHAction( SMESHOp::OpPentahedron,            "PENTA",           "ICON_DLG_PENTA" );
  createSMESHAction( SMESHOp::OpPyramid ,               "PYRAMID",         "ICON_DLG_PYRAMID" );
  createSMESHAction( SMESHOp::OpHexagonalPrism,         "OCTA",            "ICON_DLG_OCTA" );
  createSMESHAction( SMESHOp::OpPolyhedron,             "POLYHEDRON",      "ICON_DLG_POLYHEDRON" );
  createSMESHAction( SMESHOp::OpQuadraticEdge,          "QUADRATIC_EDGE",          "ICON_DLG_QUADRATIC_EDGE" );
  createSMESHAction( SMESHOp::OpQuadraticTriangle,      "QUADRATIC_TRIANGLE",      "ICON_DLG_QUADRATIC_TRIANGLE" );
  createSMESHAction( SMESHOp::OpBiQuadraticTriangle,    "BIQUADRATIC_TRIANGLE",    "ICON_DLG_BIQUADRATIC_TRIANGLE" );
  createSMESHAction( SMESHOp::OpQuadraticQuadrangle,    "QUADRATIC_QUADRANGLE",    "ICON_DLG_QUADRATIC_QUADRANGLE" );
  createSMESHAction( SMESHOp::OpBiQuadraticQuadrangle,  "BIQUADRATIC_QUADRANGLE",  "ICON_DLG_BIQUADRATIC_QUADRANGLE" );
  createSMESHAction( SMESHOp::OpQuadraticPolygon,       "QUADRATIC_POLYGON",       "ICON_DLG_QUADRATIC_POLYGON" );
  createSMESHAction( SMESHOp::OpQuadraticTetrahedron,   "QUADRATIC_TETRAHEDRON",   "ICON_DLG_QUADRATIC_TETRAHEDRON" );
  createSMESHAction( SMESHOp::OpQuadraticPyramid,       "QUADRATIC_PYRAMID",       "ICON_DLG_QUADRATIC_PYRAMID" );
  createSMESHAction( SMESHOp::OpQuadraticPentahedron,   "QUADRATIC_PENTAHEDRON",   "ICON_DLG_QUADRATIC_PENTAHEDRON" );
  createSMESHAction( SMESHOp::OpQuadraticHexahedron,    "QUADRATIC_HEXAHEDRON",    "ICON_DLG_QUADRATIC_HEXAHEDRON" );
  createSMESHAction( SMESHOp::OpTriQuadraticHexahedron, "TRIQUADRATIC_HEXAHEDRON", "ICON_DLG_TRIQUADRATIC_HEXAHEDRON" );

  createSMESHAction( SMESHOp::OpRemoveNodes,       "REMOVE_NODES",          "ICON_DLG_REM_NODE" );
  createSMESHAction( SMESHOp::OpRemoveElements,    "REMOVE_ELEMENTS",       "ICON_DLG_REM_ELEMENT" );
  createSMESHAction( SMESHOp::OpRemoveOrphanNodes, "REMOVE_ORPHAN_NODES",   "ICON_DLG_REM_ORPHAN_NODES" );
  createSMESHAction( SMESHOp::OpClearMesh,         "CLEAR_MESH",            "ICON_CLEAR_MESH" );

  //createSMESHAction( SMESHOp::OpRenumberingNodes,    "RENUM_NODES",     "ICON_DLG_RENUMBERING_NODES" );
  //createSMESHAction( SMESHOp::OpRenumberingElements, "RENUM_ELEMENTS",  "ICON_DLG_RENUMBERING_ELEMENTS" );

  createSMESHAction( SMESHOp::OpTranslation,            "TRANS",           "ICON_SMESH_TRANSLATION_VECTOR" );
  createSMESHAction( SMESHOp::OpRotation,               "ROT",             "ICON_DLG_MESH_ROTATION" );
  createSMESHAction( SMESHOp::OpSymmetry,               "SYM",             "ICON_SMESH_SYMMETRY_PLANE" );
  createSMESHAction( SMESHOp::OpScale,                  "SCALE",           "ICON_DLG_MESH_SCALE" );
  createSMESHAction( SMESHOp::OpSewing,                 "SEW",             "ICON_SMESH_SEWING_FREEBORDERS" );
  createSMESHAction( SMESHOp::OpMergeNodes,             "MERGE",           "ICON_SMESH_MERGE_NODES" );
  createSMESHAction( SMESHOp::OpMergeElements,          "MERGE_ELEMENTS",  "ICON_DLG_MERGE_ELEMENTS" );
  createSMESHAction( SMESHOp::OpMoveNode,               "MESH_THROU_POINT","ICON_DLG_MOVE_NODE" );
  createSMESHAction( SMESHOp::OpDuplicateNodes,         "DUPLICATE_NODES", "ICON_SMESH_DUPLICATE_NODES" );
  createSMESHAction( SMESHOp::OpDiagonalInversion,      "INV",             "ICON_DLG_MESH_DIAGONAL" );
  createSMESHAction( SMESHOp::OpUnionOfTwoTriangle,     "UNION2",          "ICON_UNION2TRI" );
  createSMESHAction( SMESHOp::OpOrientation,            "ORIENT",          "ICON_DLG_MESH_ORIENTATION" );
  createSMESHAction( SMESHOp::OpReorientFaces,          "REORIENT_2D",     "ICON_REORIENT_2D" );
  createSMESHAction( SMESHOp::OpUnionOfTriangles,       "UNION",           "ICON_UNIONTRI" );
  createSMESHAction( SMESHOp::OpCuttingOfQuadrangles,   "CUT",             "ICON_CUTQUAD" );
  createSMESHAction( SMESHOp::OpSplitVolumes,           "SPLIT_TO_TETRA",  "ICON_SPLIT_TO_TETRA" );
  createSMESHAction( SMESHOp::OpSplitBiQuadratic,       "SPLIT_BIQUAD",    "ICON_SPLIT_BIQUAD" );
  createSMESHAction( SMESHOp::OpSmoothing,              "SMOOTH",          "ICON_DLG_SMOOTHING" );
  createSMESHAction( SMESHOp::OpExtrusion,              "EXTRUSION",       "ICON_EXTRUSION" );
  createSMESHAction( SMESHOp::OpExtrusionAlongAPath,    "EXTRUSION_ALONG", "ICON_EXTRUSION_ALONG" );
  createSMESHAction( SMESHOp::OpRevolution,             "REVOLUTION",      "ICON_REVOLUTION" );
  createSMESHAction( SMESHOp::OpPatternMapping,         "MAP",             "ICON_MAP" );
  createSMESHAction( SMESHOp::OpConvertMeshToQuadratic, "CONV_TO_QUAD",    "ICON_CONV_TO_QUAD" );
  createSMESHAction( SMESHOp::OpCreateBoundaryElements, "2D_FROM_3D",      "ICON_2D_FROM_3D" );

  createSMESHAction( SMESHOp::OpReset,               "RESET" );
  createSMESHAction( SMESHOp::OpScalarBarProperties, "SCALAR_BAR_PROP" );
  createSMESHAction( SMESHOp::OpShowScalarBar,       "SHOW_SCALAR_BAR","",0, true  );
  createSMESHAction( SMESHOp::OpSaveDistribution,    "SAVE_DISTRIBUTION" );
  createSMESHAction( SMESHOp::OpShowDistribution,    "SHOW_DISTRIBUTION","",0, true );
#ifndef DISABLE_PLOT2DVIEWER
  createSMESHAction( SMESHOp::OpPlotDistribution, "PLOT_DISTRIBUTION" );
#endif
  createSMESHAction( SMESHOp::OpDMWireframe,  "WIRE",    "ICON_WIRE", 0, true );
  createSMESHAction( SMESHOp::OpDMShading,    "SHADE",   "ICON_SHADE", 0, true );
  createSMESHAction( SMESHOp::OpDMNodes,      "NODES",   "ICON_POINTS", 0, true );
  createSMESHAction( SMESHOp::OpDMShrink,     "SHRINK",  "ICON_SHRINK", 0, true );
  createSMESHAction( SMESHOp::OpUpdate,       "UPDATE",  "ICON_UPDATE" );
  createSMESHAction( SMESHOp::OpDE0DElements, "ELEMS0D", "ICON_DLG_ELEM0D", 0, true );
  createSMESHAction( SMESHOp::OpDEEdges,      "EDGES",   "ICON_DLG_EDGE", 0, true );
  createSMESHAction( SMESHOp::OpDEFaces,      "FACES",   "ICON_DLG_TRIANGLE", 0, true );
  createSMESHAction( SMESHOp::OpDEVolumes,    "VOLUMES", "ICON_DLG_TETRAS", 0, true );
  createSMESHAction( SMESHOp::OpDEBalls,      "BALLS",   "ICON_DLG_BALL", 0, true );
  createSMESHAction( SMESHOp::OpDEChoose,     "CHOOSE",  "ICON_DLG_CHOOSE", 0, false );
  createSMESHAction( SMESHOp::OpDEAllEntity,  "ALL",     "ICON_DLG_CHOOSE_ALL", 0, false );
  createSMESHAction( SMESHOp::OpOrientationOnFaces, "FACE_ORIENTATION", "", 0, true );

  createSMESHAction( SMESHOp::OpRepresentationLines, "LINE_REPRESENTATION", "", 0, true );
  createSMESHAction( SMESHOp::OpRepresentationArcs,  "ARC_REPRESENTATION", "", 0, true );

  createSMESHAction( SMESHOp::OpEditHypothesis,    "EDIT_HYPO" );
  createSMESHAction( SMESHOp::OpUnassign,          "UNASSIGN" );
  createSMESHAction( SMESHOp::OpNumberingNodes,    "NUM_NODES", "", 0, true );
  createSMESHAction( SMESHOp::OpNumberingElements, "NUM_ELEMENTS", "", 0, true );
  createSMESHAction( SMESHOp::OpProperties,   "COLORS" );
  createSMESHAction( SMESHOp::OpTransparency, "TRANSP" );
  createSMESHAction( SMESHOp::OpClipping,     "CLIP" );
  createSMESHAction( SMESHOp::OpAutoColor,        "AUTO_COLOR" );
  createSMESHAction( SMESHOp::OpDisableAutoColor, "DISABLE_AUTO_COLOR" );

  createSMESHAction( SMESHOp::OpMinimumDistance,  "MEASURE_MIN_DIST", "ICON_MEASURE_MIN_DIST" );
  createSMESHAction( SMESHOp::OpBoundingBox,      "MEASURE_BND_BOX",  "ICON_MEASURE_BND_BOX" );
  createSMESHAction( SMESHOp::OpPropertiesLength, "MEASURE_LENGTH",   "ICON_MEASURE_LENGTH" );
  createSMESHAction( SMESHOp::OpPropertiesArea,   "MEASURE_AREA",     "ICON_MEASURE_AREA" );
  createSMESHAction( SMESHOp::OpPropertiesVolume, "MEASURE_VOLUME",   "ICON_MEASURE_VOLUME" );

  createSMESHAction( SMESHOp::OpHide,     "HIDE" );
  createSMESHAction( SMESHOp::OpShow,     "SHOW" );
  createSMESHAction( SMESHOp::OpShowOnly, "DISPLAY_ONLY" );

  createSMESHAction( SMESHOp::OpSortChild, "SORT_CHILD_ITEMS" );

  QList<int> aCtrlActions;
  aCtrlActions << SMESHOp::OpFreeNode << SMESHOp::OpEqualNode
               << SMESHOp::OpNodeConnectivityNb                                         // node controls
               << SMESHOp::OpFreeEdge << SMESHOp::OpFreeBorder
               << SMESHOp::OpLength << SMESHOp::OpConnection << SMESHOp::OpEqualEdge    // edge controls
               << SMESHOp::OpFreeFace << SMESHOp::OpLength2D << SMESHOp::OpConnection2D
               << SMESHOp::OpArea << SMESHOp::OpTaper << SMESHOp::OpAspectRatio
               << SMESHOp::OpMinimumAngle << SMESHOp::OpWarpingAngle << SMESHOp::OpSkew
               << SMESHOp::OpMaxElementLength2D << SMESHOp::OpBareBorderFace
               << SMESHOp::OpOverConstrainedFace << SMESHOp::OpEqualFace                // face controls
               << SMESHOp::OpAspectRatio3D << SMESHOp::OpVolume
               << SMESHOp::OpMaxElementLength3D << SMESHOp::OpBareBorderVolume
               << SMESHOp::OpOverConstrainedVolume << SMESHOp::OpEqualVolume;           // volume controls
  QActionGroup* aCtrlGroup = new QActionGroup( application()->desktop() );
  aCtrlGroup->setExclusive( true );
  for( int i = 0; i < aCtrlActions.size(); i++ )
    aCtrlGroup->addAction( action( aCtrlActions[i] ) );

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

  QMenu* nodeMenu = new QMenu(); QMenu* edgeMenu = new QMenu();
  QMenu* faceMenu = new QMenu(); QMenu* volumeMenu = new QMenu();
  int importId = createMenu( tr( "MEN_IMPORT" ), fileId, -1, 10 ),
      exportId = createMenu( tr( "MEN_EXPORT" ), fileId, -1, 10 ),
      nodeId   = createMenu( tr( "MEN_NODE_CTRL" ), ctrlId, -1, 10, -1, nodeMenu ),
      edgeId   = createMenu( tr( "MEN_EDGE_CTRL" ), ctrlId, -1, 10, -1, edgeMenu ),
      faceId   = createMenu( tr( "MEN_FACE_CTRL" ), ctrlId, -1, 10, -1, faceMenu ),
      volumeId = createMenu( tr( "MEN_VOLUME_CTRL" ), ctrlId, -1, 10, -1, volumeMenu ),
      addId    = createMenu( tr( "MEN_ADD" ),    modifyId, 402 ),
      removeId = createMenu( tr( "MEN_REMOVE" ), modifyId, 403 ),
    //renumId  = createMenu( tr( "MEN_RENUM" ),  modifyId, 404 ),
      transfId = createMenu( tr( "MEN_TRANSF" ), modifyId, 405 ),
      basicPropId = createMenu( tr( "MEN_BASIC_PROPERTIES" ), measureId, -1, 10 );

  //createMenu( SMESHOp::OpImportDAT, importId, -1 );
  createMenu( SMESHOp::OpImportUNV,  importId, -1 );
  createMenu( SMESHOp::OpImportMED,  importId, -1 );
  createMenu( SMESHOp::OpImportSTL,  importId, -1 );
#ifdef WITH_CGNS
  createMenu( SMESHOp::OpImportCGNS, importId, -1 );
#endif
  createMenu( SMESHOp::OpImportSAUV, importId, -1 );
  createMenu( SMESHOp::OpImportGMF,  importId, -1 );
  createMenu( SMESHOp::OpExportDAT,  exportId, -1 );
  createMenu( SMESHOp::OpExportMED,  exportId, -1 );
  createMenu( SMESHOp::OpExportUNV,  exportId, -1 );
  createMenu( SMESHOp::OpExportSTL,  exportId, -1 );
#ifdef WITH_CGNS
  createMenu( SMESHOp::OpExportCGNS, exportId, -1 );
#endif
  createMenu( SMESHOp::OpExportSAUV, exportId, -1 );
  createMenu( SMESHOp::OpExportGMF,  exportId, -1 );
  createMenu( separator(), fileId, 10 );

  createMenu( SMESHOp::OpDelete, editId, -1 );

  createMenu( SMESHOp::OpSelectFiltersLibrary, toolsId, -1 );

  createMenu( SMESHOp::OpCreateMesh,           meshId, -1 ); // "Mesh" menu
  createMenu( SMESHOp::OpCreateSubMesh,        meshId, -1 );
  createMenu( SMESHOp::OpEditMeshOrSubMesh,    meshId, -1 );
  createMenu( SMESHOp::OpBuildCompoundMesh,    meshId, -1 );
  createMenu( SMESHOp::OpCopyMesh,             meshId, -1 );
  createMenu( separator(),                     meshId, -1 );
  createMenu( SMESHOp::OpCompute,              meshId, -1 );
  createMenu( SMESHOp::OpPreCompute,           meshId, -1 );
  createMenu( SMESHOp::OpEvaluate,             meshId, -1 );
  createMenu( SMESHOp::OpMeshOrder,            meshId, -1 );
  createMenu( separator(),                     meshId, -1 );
  createMenu( SMESHOp::OpCreateGroup,          meshId, -1 );
  createMenu( SMESHOp::OpCreateGeometryGroup,  meshId, -1 );
  createMenu( SMESHOp::OpConstructGroup,       meshId, -1 );
  createMenu( SMESHOp::OpEditGroup,            meshId, -1 );
  createMenu( SMESHOp::OpEditGeomGroupAsGroup, meshId, -1 );
  createMenu( separator(),                     meshId, -1 );
  createMenu( SMESHOp::OpUnionGroups,          meshId, -1 );
  createMenu( SMESHOp::OpIntersectGroups,      meshId, -1 );
  createMenu( SMESHOp::OpCutGroups,            meshId, -1 );
  createMenu( separator(),                     meshId, -1 );
  createMenu( SMESHOp::OpGroupUnderlyingElem,  meshId, -1 );
  createMenu( separator(),                     meshId, -1 );
  createMenu( SMESHOp::OpMeshInformation,      meshId, -1 );
  //createMenu( SMESHOp::OpStdInfo, meshId, -1 );
  //createMenu( SMESHOp::OpWhatIs, meshId, -1 ); // VSR: issue #0021242 (eliminate "Mesh Element Information" command)
  createMenu( SMESHOp::OpFindElementByPoint,   meshId, -1 );
  createMenu( separator(),                     meshId, -1 );

  createMenu( SMESHOp::OpFreeNode,              nodeId,   -1 );
  createMenu( SMESHOp::OpEqualNode,             nodeId,   -1 );
  //createMenu( SMESHOp::OpNodeConnectivityNb,    nodeId,   -1 );
  createMenu( SMESHOp::OpFreeBorder,            edgeId,   -1 );
  createMenu( SMESHOp::OpLength,                edgeId,   -1 );
  createMenu( SMESHOp::OpConnection,            edgeId,   -1 );
  createMenu( SMESHOp::OpEqualEdge,             edgeId,   -1 );
  createMenu( SMESHOp::OpFreeEdge,              faceId,   -1 );
  createMenu( SMESHOp::OpFreeFace,              faceId,   -1 );
  createMenu( SMESHOp::OpBareBorderFace,        faceId,   -1 );
  createMenu( SMESHOp::OpOverConstrainedFace,   faceId,   -1 );
  createMenu( SMESHOp::OpLength2D,              faceId,   -1 );
  createMenu( SMESHOp::OpConnection2D,          faceId,   -1 );
  createMenu( SMESHOp::OpArea,                  faceId,   -1 );
  createMenu( SMESHOp::OpTaper,                 faceId,   -1 );
  createMenu( SMESHOp::OpAspectRatio,           faceId,   -1 );
  createMenu( SMESHOp::OpMinimumAngle,          faceId,   -1 );
  createMenu( SMESHOp::OpWarpingAngle,          faceId,   -1 );
  createMenu( SMESHOp::OpSkew,                  faceId,   -1 );
  createMenu( SMESHOp::OpMaxElementLength2D,    faceId,   -1 );
  createMenu( SMESHOp::OpEqualFace,             faceId,   -1 );
  createMenu( SMESHOp::OpAspectRatio3D,         volumeId, -1 );
  createMenu( SMESHOp::OpVolume,                volumeId, -1 );
  createMenu( SMESHOp::OpMaxElementLength3D,    volumeId, -1 );
  createMenu( SMESHOp::OpBareBorderVolume,      volumeId, -1 );
  createMenu( SMESHOp::OpOverConstrainedVolume, volumeId, -1 );
  createMenu( SMESHOp::OpEqualVolume,           volumeId, -1 );
  createMenu( separator(),                      ctrlId,   -1 );
  createMenu( SMESHOp::OpReset,                 ctrlId,   -1 );
  createMenu( separator(),                      ctrlId,   -1 );
  createMenu( SMESHOp::OpOverallMeshQuality,    ctrlId,   -1 );

  createMenu( SMESHOp::OpNode,                   addId, -1 );
  createMenu( SMESHOp::OpElem0D,                 addId, -1 );
  createMenu( SMESHOp::OpElem0DOnElemNodes,      addId, -1 );
  createMenu( SMESHOp::OpBall,                   addId, -1 );
  createMenu( SMESHOp::OpEdge,                   addId, -1 );
  createMenu( SMESHOp::OpTriangle,               addId, -1 );
  createMenu( SMESHOp::OpQuadrangle,             addId, -1 );
  createMenu( SMESHOp::OpPolygon,                addId, -1 );
  createMenu( SMESHOp::OpTetrahedron,            addId, -1 );
  createMenu( SMESHOp::OpHexahedron,             addId, -1 );
  createMenu( SMESHOp::OpPentahedron,            addId, -1 );
  createMenu( SMESHOp::OpPyramid,                addId, -1 );
  createMenu( SMESHOp::OpHexagonalPrism,         addId, -1 );
  createMenu( SMESHOp::OpPolyhedron,             addId, -1 );
  createMenu( separator(),                       addId, -1 );
  createMenu( SMESHOp::OpQuadraticEdge,          addId, -1 );
  createMenu( SMESHOp::OpQuadraticTriangle,      addId, -1 );
  createMenu( SMESHOp::OpBiQuadraticTriangle ,   addId, -1 );
  createMenu( SMESHOp::OpQuadraticQuadrangle,    addId, -1 );
  createMenu( SMESHOp::OpBiQuadraticQuadrangle,  addId, -1 );
  createMenu( SMESHOp::OpQuadraticPolygon,       addId, -1 );
  createMenu( SMESHOp::OpQuadraticTetrahedron,   addId, -1 );
  createMenu( SMESHOp::OpQuadraticPyramid,       addId, -1 );
  createMenu( SMESHOp::OpQuadraticPentahedron,   addId, -1 );
  createMenu( SMESHOp::OpQuadraticHexahedron,    addId, -1 );
  createMenu( SMESHOp::OpTriQuadraticHexahedron, addId, -1 );

  createMenu( SMESHOp::OpRemoveNodes,       removeId, -1 );
  createMenu( SMESHOp::OpRemoveElements,    removeId, -1 );
  createMenu( SMESHOp::OpRemoveOrphanNodes, removeId, -1 );
  createMenu( separator(),                  removeId, -1 );
  createMenu( SMESHOp::OpDeleteGroup,       removeId, -1 );
  createMenu( separator(),                  removeId, -1 );
  createMenu( SMESHOp::OpClearMesh,         removeId, -1 );

  //createMenu( SMESHOp::OpRenumberingNodes,    renumId, -1 );
  //createMenu( SMESHOp::OpRenumberingElements, renumId, -1 );

  createMenu( SMESHOp::OpTranslation,    transfId, -1 );
  createMenu( SMESHOp::OpRotation,       transfId, -1 );
  createMenu( SMESHOp::OpSymmetry,       transfId, -1 );
  createMenu( SMESHOp::OpScale,          transfId, -1 );
  createMenu( SMESHOp::OpSewing,         transfId, -1 );
  createMenu( SMESHOp::OpMergeNodes,     transfId, -1 );
  createMenu( SMESHOp::OpMergeElements,  transfId, -1 );
  createMenu( SMESHOp::OpDuplicateNodes, transfId, -1 );

  createMenu( SMESHOp::OpMoveNode,               modifyId, -1 );
  createMenu( SMESHOp::OpDiagonalInversion,      modifyId, -1 );
  createMenu( SMESHOp::OpUnionOfTwoTriangle,     modifyId, -1 );
  createMenu( SMESHOp::OpOrientation,            modifyId, -1 );
  createMenu( SMESHOp::OpReorientFaces,          modifyId, -1 );
  createMenu( SMESHOp::OpUnionOfTriangles,       modifyId, -1 );
  createMenu( SMESHOp::OpCuttingOfQuadrangles,   modifyId, -1 );
  createMenu( SMESHOp::OpSplitVolumes,           modifyId, -1 );
  createMenu( SMESHOp::OpSplitBiQuadratic,       modifyId, -1 );
  createMenu( SMESHOp::OpSmoothing,              modifyId, -1 );
  createMenu( SMESHOp::OpExtrusion,              modifyId, -1 );
  createMenu( SMESHOp::OpExtrusionAlongAPath ,   modifyId, -1 );
  createMenu( SMESHOp::OpRevolution,             modifyId, -1 );
  createMenu( SMESHOp::OpPatternMapping,         modifyId, -1 );
  createMenu( SMESHOp::OpConvertMeshToQuadratic, modifyId, -1 );
  createMenu( SMESHOp::OpCreateBoundaryElements, modifyId, -1 );

  createMenu( SMESHOp::OpMinimumDistance,  measureId,   -1 );
  createMenu( SMESHOp::OpBoundingBox,      measureId,   -1 );
  createMenu( SMESHOp::OpPropertiesLength, basicPropId, -1 );
  createMenu( SMESHOp::OpPropertiesArea,   basicPropId, -1 );
  createMenu( SMESHOp::OpPropertiesVolume, basicPropId, -1 );
  createMenu( SMESHOp::OpUpdate,           viewId,      -1 );

  connect( nodeMenu,   SIGNAL( aboutToShow() ), this, SLOT( onUpdateControlActions() ) );
  connect( edgeMenu,   SIGNAL( aboutToShow() ), this, SLOT( onUpdateControlActions() ) );
  connect( faceMenu,   SIGNAL( aboutToShow() ), this, SLOT( onUpdateControlActions() ) );
  connect( volumeMenu, SIGNAL( aboutToShow() ), this, SLOT( onUpdateControlActions() ) );

  // ----- create toolbars --------------
  int meshTb       = createTool( tr( "TB_MESH" ),      QString( "SMESHMeshToolbar" ) ),
      info         = createTool( tr( "TB_INFO" ),      QString( "SMESHInformationToolbar" ) ),
      groupTb      = createTool( tr( "TB_GROUP" ),     QString( "SMESHGroupToolbar" ) ),
      ctrl0dTb     = createTool( tr( "TB_CTRL0D" ),    QString( "SMESHNodeControlsToolbar" ) ),
      ctrl1dTb     = createTool( tr( "TB_CTRL1D" ),    QString( "SMESHEdgeControlsToolbar" ) ),
      ctrl2dTb     = createTool( tr( "TB_CTRL2D" ),    QString( "SMESHFaceControlsToolbar" ) ),
      ctrl3dTb     = createTool( tr( "TB_CTRL3D" ),    QString( "SMESHVolumeControlsToolbar" ) ),
      addElemTb    = createTool( tr( "TB_ADD" ),       QString( "SMESHAddElementToolbar" ) ),
      addNonElemTb = createTool( tr( "TB_ADDNON" ),    QString( "SMESHAddElementToolbar" ) ),
      remTb        = createTool( tr( "TB_REM" ),       QString( "SMESHRemoveToolbar" ) ),
    //renumbTb     = createTool( tr( "TB_RENUMBER" ),  QString( "SMESHRenumberingToolbar" ) ),  
      transformTb  = createTool( tr( "TB_TRANSFORM" ), QString( "SMESHTransformationToolbar" ) ),  
      modifyTb     = createTool( tr( "TB_MODIFY" ),    QString( "SMESHModificationToolbar" ) ),
      measuremTb   = createTool( tr( "TB_MEASUREM" ),  QString( "SMESHMeasurementsToolbar" ) ),
      dispModeTb   = createTool( tr( "TB_DISP_MODE" ), QString( "SMESHDisplayModeToolbar" ) );

  createTool( SMESHOp::OpCreateMesh,        meshTb );
  createTool( SMESHOp::OpCreateSubMesh,     meshTb );
  createTool( SMESHOp::OpEditMeshOrSubMesh, meshTb );
  createTool( SMESHOp::OpBuildCompoundMesh, meshTb );
  createTool( SMESHOp::OpCopyMesh,          meshTb );
  createTool( separator(),                  meshTb );
  createTool( SMESHOp::OpCompute,           meshTb );
  createTool( SMESHOp::OpPreCompute,        meshTb );
  createTool( SMESHOp::OpEvaluate,          meshTb );
  createTool( SMESHOp::OpMeshOrder,         meshTb );

  createTool( SMESHOp::OpCreateGroup,         groupTb );
  createTool( SMESHOp::OpCreateGeometryGroup, groupTb );
  createTool( SMESHOp::OpConstructGroup,      groupTb );
  createTool( SMESHOp::OpEditGroup,           groupTb );

  createTool( SMESHOp::OpMeshInformation,    info );
  //createTool( SMESHOp::OpStdInfo, meshTb );
  //createTool( SMESHOp::OpWhatIs, meshTb ); // VSR: issue #0021242 (eliminate "Mesh Element Information" command)
  createTool( SMESHOp::OpFindElementByPoint, info );

  createTool( SMESHOp::OpFreeNode,  ctrl0dTb );
  createTool( SMESHOp::OpEqualNode, ctrl0dTb );
  //createTool( SMESHOp::OpNodeConnectivityNb, ctrl0dTb );

  createTool( SMESHOp::OpFreeBorder, ctrl1dTb );
  createTool( SMESHOp::OpLength,     ctrl1dTb );
  createTool( SMESHOp::OpConnection, ctrl1dTb );
  createTool( SMESHOp::OpEqualEdge,  ctrl1dTb );

  createTool( SMESHOp::OpFreeEdge,            ctrl2dTb );
  createTool( SMESHOp::OpFreeFace,            ctrl2dTb );
  createTool( SMESHOp::OpBareBorderFace,      ctrl2dTb );
  createTool( SMESHOp::OpOverConstrainedFace, ctrl2dTb );
  createTool( SMESHOp::OpLength2D,            ctrl2dTb );
  createTool( SMESHOp::OpConnection2D,        ctrl2dTb );
  createTool( SMESHOp::OpArea,                ctrl2dTb );
  createTool( SMESHOp::OpTaper,               ctrl2dTb );
  createTool( SMESHOp::OpAspectRatio,         ctrl2dTb );
  createTool( SMESHOp::OpMinimumAngle,        ctrl2dTb );
  createTool( SMESHOp::OpWarpingAngle,        ctrl2dTb );
  createTool( SMESHOp::OpSkew,                ctrl2dTb );
  createTool( SMESHOp::OpMaxElementLength2D,  ctrl2dTb );
  createTool( SMESHOp::OpEqualFace,           ctrl2dTb );

  createTool( SMESHOp::OpAspectRatio3D,         ctrl3dTb );
  createTool( SMESHOp::OpVolume,                ctrl3dTb );
  createTool( SMESHOp::OpMaxElementLength3D,    ctrl3dTb );
  createTool( SMESHOp::OpBareBorderVolume,      ctrl3dTb );
  createTool( SMESHOp::OpOverConstrainedVolume, ctrl3dTb );
  createTool( SMESHOp::OpEqualVolume,           ctrl3dTb );

  createTool( SMESHOp::OpNode,              addElemTb );
  createTool( SMESHOp::OpElem0D,            addElemTb );
  createTool( SMESHOp::OpElem0DOnElemNodes, addElemTb );
  createTool( SMESHOp::OpBall,              addElemTb );
  createTool( SMESHOp::OpEdge,              addElemTb );
  createTool( SMESHOp::OpTriangle,          addElemTb );
  createTool( SMESHOp::OpQuadrangle,        addElemTb );
  createTool( SMESHOp::OpPolygon,           addElemTb );
  createTool( SMESHOp::OpTetrahedron,       addElemTb );
  createTool( SMESHOp::OpHexahedron,        addElemTb );
  createTool( SMESHOp::OpPentahedron,       addElemTb );
  createTool( SMESHOp::OpPyramid,           addElemTb );
  createTool( SMESHOp::OpHexagonalPrism,    addElemTb );
  createTool( SMESHOp::OpPolyhedron,        addElemTb );

  createTool( SMESHOp::OpQuadraticEdge,          addNonElemTb );
  createTool( SMESHOp::OpQuadraticTriangle,      addNonElemTb );
  createTool( SMESHOp::OpBiQuadraticTriangle,    addNonElemTb );
  createTool( SMESHOp::OpQuadraticQuadrangle,    addNonElemTb );
  createTool( SMESHOp::OpBiQuadraticQuadrangle,  addNonElemTb );
  createTool( SMESHOp::OpQuadraticPolygon,       addNonElemTb );
  createTool( SMESHOp::OpQuadraticTetrahedron,   addNonElemTb );
  createTool( SMESHOp::OpQuadraticPyramid,       addNonElemTb );
  createTool( SMESHOp::OpQuadraticPentahedron,   addNonElemTb );
  createTool( SMESHOp::OpQuadraticHexahedron,    addNonElemTb );
  createTool( SMESHOp::OpTriQuadraticHexahedron, addNonElemTb );

  createTool( SMESHOp::OpRemoveNodes,       remTb );
  createTool( SMESHOp::OpRemoveElements,    remTb );
  createTool( SMESHOp::OpRemoveOrphanNodes, remTb );
  createTool( SMESHOp::OpClearMesh,         remTb );

  //createTool( SMESHOp::OpRenumberingNodes,    renumbTb );
  //createTool( SMESHOp::OpRenumberingElements, renumbTb );

  createTool( SMESHOp::OpTranslation,    transformTb );
  createTool( SMESHOp::OpRotation,       transformTb );
  createTool( SMESHOp::OpSymmetry,       transformTb );
  createTool( SMESHOp::OpScale,          transformTb );
  createTool( SMESHOp::OpSewing,         transformTb );
  createTool( SMESHOp::OpMergeNodes,     transformTb );
  createTool( SMESHOp::OpMergeElements,  transformTb );
  createTool( SMESHOp::OpDuplicateNodes, transformTb );

  createTool( SMESHOp::OpMoveNode,               modifyTb );
  createTool( SMESHOp::OpDiagonalInversion,      modifyTb );
  createTool( SMESHOp::OpUnionOfTwoTriangle,     modifyTb );
  createTool( SMESHOp::OpOrientation,            modifyTb );
  createTool( SMESHOp::OpReorientFaces,          modifyTb );
  createTool( SMESHOp::OpUnionOfTriangles,       modifyTb );
  createTool( SMESHOp::OpCuttingOfQuadrangles,   modifyTb );
  createTool( SMESHOp::OpSplitVolumes,           modifyTb );
  createTool( SMESHOp::OpSplitBiQuadratic,       modifyTb );
  createTool( SMESHOp::OpSmoothing,              modifyTb );
  createTool( SMESHOp::OpExtrusion,              modifyTb );
  createTool( SMESHOp::OpExtrusionAlongAPath,    modifyTb );
  createTool( SMESHOp::OpRevolution,             modifyTb );
  createTool( SMESHOp::OpPatternMapping,         modifyTb );
  createTool( SMESHOp::OpConvertMeshToQuadratic, modifyTb );
  createTool( SMESHOp::OpCreateBoundaryElements, modifyTb );

  createTool( SMESHOp::OpMinimumDistance, measuremTb );

  createTool( SMESHOp::OpUpdate, dispModeTb );

  QString lc = "$";        // VSR : instead of QtxPopupSelection::defEquality();
  QString dc = "selcount"; // VSR : instead of QtxPopupSelection::defSelCountParam()

  myRules.clear();
  QString
    OB      = "'ObjectBrowser'",
    View    = "'" + SVTK_Viewer::Type() + "'",
    pat     = "'%1'",
    mesh    = pat.arg( SMESHGUI_Selection::typeName( SMESH::MESH ) ),
    group   = pat.arg( SMESHGUI_Selection::typeName( SMESH::GROUP ) ),
    hypo    = pat.arg( SMESHGUI_Selection::typeName( SMESH::HYPOTHESIS ) ),
    algo    = pat.arg( SMESHGUI_Selection::typeName( SMESH::ALGORITHM ) ),
    elems   = QString( "'%1' '%2' '%3' '%4' '%5' '%6'" ).
    arg( SMESHGUI_Selection::typeName( SMESH::SUBMESH_VERTEX ) ).
    arg( SMESHGUI_Selection::typeName( SMESH::SUBMESH_EDGE ) ).
    arg( SMESHGUI_Selection::typeName( SMESH::SUBMESH_FACE ) ).
    arg( SMESHGUI_Selection::typeName( SMESH::SUBMESH_SOLID ) ).
    arg( SMESHGUI_Selection::typeName( SMESH::SUBMESH_COMPOUND ) ).
    arg( SMESHGUI_Selection::typeName( SMESH::SUBMESH ) ),
    subMesh      = elems,
    mesh_part    = mesh + " " + subMesh + " " + group,
    mesh_group   = mesh + " " + group,
    mesh_submesh = mesh + " " + subMesh,
    hyp_alg      = hypo + " " + algo;

  // popup for object browser
  QString
    isInvisible("not( isVisible )"),
    isEmpty("numberOfNodes = 0"),
    isNotEmpty("numberOfNodes <> 0"),

    // has nodes, edges, etc in VISIBLE! actor
    hasNodes("(numberOfNodes > 0 ) && hasActor"),
    hasElems("(count( elemTypes ) > 0)"),
    hasDifferentElems("(count( elemTypes ) > 1)"),
    hasBalls("({'BallElem'} in elemTypes)"),
    hasElems0d("({'Elem0d'} in elemTypes)"),
    hasEdges("({'Edge'} in elemTypes)"),
    hasFaces("({'Face'} in elemTypes)"),
    hasVolumes("({'Volume'} in elemTypes)"),
    hasFacesOrVolumes("(({'Face'} in elemTypes) || ({'Volume'} in elemTypes)) ");

  createPopupItem( SMESHOp::OpFileInformation,   OB, mesh, "&& selcount=1 && isImported" );
  createPopupItem( SMESHOp::OpCreateSubMesh,     OB, mesh, "&& hasGeomReference");
  createPopupItem( SMESHOp::OpEditMesh,          OB, mesh, "&& selcount=1" );
  createPopupItem( SMESHOp::OpEditSubMesh,       OB, subMesh, "&& selcount=1 && hasGeomReference" );
  createPopupItem( SMESHOp::OpEditGroup,         OB, group );
  createPopupItem( SMESHOp::OpEditGeomGroupAsGroup, OB, group, "&& groupType != 'Group'" );

  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( SMESHOp::OpCompute,           OB, mesh, "&& selcount=1 && isComputable" );
  createPopupItem( SMESHOp::OpComputeSubMesh,    OB, subMesh, "&& selcount=1 && isComputable" );
  createPopupItem( SMESHOp::OpPreCompute,        OB, mesh, "&& selcount=1 && isPreComputable" );
  createPopupItem( SMESHOp::OpEvaluate,          OB, mesh, "&& selcount=1 && isComputable" );
  createPopupItem( SMESHOp::OpMeshOrder,         OB, mesh, "&& selcount=1 && isComputable && hasGeomReference" );
  createPopupItem( SMESHOp::OpUpdate,            OB, mesh_part );
  createPopupItem( SMESHOp::OpMeshInformation,   OB, mesh_part );
  createPopupItem( SMESHOp::OpFindElementByPoint,OB, mesh_group, "&& selcount=1" );
  createPopupItem( SMESHOp::OpOverallMeshQuality,OB, mesh_part );
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( SMESHOp::OpCreateGroup,       OB, mesh, "&& selcount=1" );
  createPopupItem( SMESHOp::OpCreateGeometryGroup, OB, mesh, "&& selcount=1 && hasGeomReference" );
  createPopupItem( SMESHOp::OpConstructGroup,    OB, subMesh );
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( SMESHOp::OpEditHypothesis,    OB, hypo, "&& isEditableHyp");
  createPopupItem( SMESHOp::OpUnassign,          OB, hyp_alg );
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( SMESHOp::OpConvertMeshToQuadratic, OB, mesh_submesh );
  createPopupItem( SMESHOp::OpCreateBoundaryElements, OB, mesh_group, "&& selcount=1 && dim>=2");
  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( SMESHOp::OpClearMesh,              OB, mesh );
  //popupMgr()->insert( separator(), -1, 0 );

  QString only_one_non_empty = QString( " && %1=1 && numberOfNodes>0" ).arg( dc );
  QString multiple_non_empty = QString( " && %1>0 && numberOfNodes>0" ).arg( dc );
  QString only_one_2D        = only_one_non_empty + " && dim>1";

  int anId = popupMgr()->insert( tr( "MEN_EXPORT" ), -1, -1 );        // EXPORT submenu
  createPopupItem( SMESHOp::OpPopupExportMED,  OB, mesh_group, multiple_non_empty, anId );
  createPopupItem( SMESHOp::OpPopupExportUNV,  OB, mesh_group, only_one_non_empty, anId );
  createPopupItem( SMESHOp::OpPopupExportSTL,  OB, mesh_group, only_one_2D, anId );
#ifdef WITH_CGNS
  createPopupItem( SMESHOp::OpPopupExportCGNS, OB, mesh_group, multiple_non_empty, anId );
#endif
  createPopupItem( SMESHOp::OpPopupExportSAUV, OB, mesh_group, only_one_non_empty, anId );
  createPopupItem( SMESHOp::OpPopupExportGMF,  OB, mesh_group, only_one_non_empty, anId );
  createPopupItem( SMESHOp::OpPopupExportDAT,  OB, mesh_group, only_one_non_empty, anId );
  createPopupItem( SMESHOp::OpDelete,          OB, mesh_part + " " + hyp_alg );
  createPopupItem( SMESHOp::OpDeleteGroup,     OB, group );
  popupMgr()->insert( separator(), -1, 0 );

  // popup for viewer
  createPopupItem( SMESHOp::OpEditGroup,            View, group );
  createPopupItem( SMESHOp::OpAddElemGroupPopup,    View, elems );
  createPopupItem( SMESHOp::OpRemoveElemGroupPopup, View, elems );

  popupMgr()->insert( separator(), -1, 0 );
  createPopupItem( SMESHOp::OpUpdate,             View, mesh_part );
  createPopupItem( SMESHOp::OpMeshInformation,    View, mesh_part );
  createPopupItem( SMESHOp::OpOverallMeshQuality, View, mesh_part );
  createPopupItem( SMESHOp::OpFindElementByPoint, View, mesh );
  popupMgr()->insert( separator(), -1, 0 );

  createPopupItem( SMESHOp::OpAutoColor,        OB + " " + View, mesh, "&& (not isAutoColor)" );
  createPopupItem( SMESHOp::OpDisableAutoColor, OB + " " + View, mesh, "&& isAutoColor" );
  popupMgr()->insert( separator(), -1, 0 );

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

  popupMgr()->insert( action( SMESHOp::OpNumberingNodes ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpNumberingNodes ), aMeshInVTK + "&& isVisible &&" + hasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpNumberingNodes ), "{'Point'} in labeledTypes", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpNumberingElements ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpNumberingElements ), aMeshInVTK + "&& isVisible &&" + hasElems, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpNumberingElements ), "{'Cell'} in labeledTypes", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Display Mode
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_DISPMODE" ), -1, -1 );

  popupMgr()->insert( action( SMESHOp::OpDMWireframe ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDMWireframe ), aMeshInVTK + "&&" + hasElems, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpDMWireframe ), "displayMode = 'eEdge'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpDMShading ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDMShading ),aMeshInVTK+ "&& (" + hasFaces + "||" + hasVolumes + ")", QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( SMESHOp::OpDMShading ), "displayMode = 'eSurface'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpDMNodes ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDMNodes ), aMeshInVTK + "&&" + hasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpDMNodes ), "displayMode = 'ePoint'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( SMESHOp::OpDMShrink ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDMShrink ), aMeshInVTK + "&& shrinkMode <> 'IsNotShrinkable' && displayMode <> 'ePoint'", QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( SMESHOp::OpDMShrink ), "shrinkMode = 'IsShrunk'", QtxPopupMgr::ToggleRule );

  //-------------------------------------------------
  // Display Entity
  //-------------------------------------------------
  QString aDiffElemsInVTK = aMeshInVTK + "&&" + hasDifferentElems;

  anId = popupMgr()->insert( tr( "MEN_DISP_ENT" ), -1, -1 );

  popupMgr()->insert( action( SMESHOp::OpDE0DElements ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDE0DElements ), aDiffElemsInVTK + "&& isVisible &&" + hasElems0d, QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( SMESHOp::OpDE0DElements ), "{'Elem0d'} in entityMode", QtxPopupMgr::ToggleRule);

  popupMgr()->insert( action( SMESHOp::OpDEEdges ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDEEdges ), aDiffElemsInVTK + "&& isVisible &&" + hasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpDEEdges ), "{'Edge'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpDEFaces ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDEFaces ), aDiffElemsInVTK + "&& isVisible &&" + hasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpDEFaces ), "{'Face'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpDEVolumes ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDEVolumes ), aDiffElemsInVTK + "&& isVisible &&" + hasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpDEVolumes ), "{'Volume'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpDEBalls ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDEBalls ), aDiffElemsInVTK + "&& isVisible &&" + hasBalls, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpDEBalls ), "{'BallElem'} in entityMode", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( SMESHOp::OpDEChoose ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDEChoose ), aClient + "&& $type in {" + mesh + "} &&" + isNotEmpty, QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( SMESHOp::OpDEAllEntity ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpDEAllEntity ), aDiffElemsInVTK + "&& isVisible && not( elemTypes in entityMode )", QtxPopupMgr::VisibleRule );


  //-------------------------------------------------
  // Representation of the 2D Quadratic elements
  //-------------------------------------------------
  anId = popupMgr()->insert( tr( "MEN_QUADRATIC_REPRESENT" ), -1, -1 );
  popupMgr()->insert( action( SMESHOp::OpRepresentationLines ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpRepresentationLines ), aMeshInVTK + "&& isVisible && isQuadratic",QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpRepresentationLines ), "quadratic2DMode = 'eLines'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpRepresentationArcs ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpRepresentationArcs ), aMeshInVTK + "&& isVisible && isQuadratic", QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpRepresentationArcs ), "quadratic2DMode = 'eArcs'", QtxPopupMgr::ToggleRule );

  //-------------------------------------------------
  // Orientation of faces
  //-------------------------------------------------
  popupMgr()->insert( action( SMESHOp::OpOrientationOnFaces ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpOrientationOnFaces ), aMeshInVTK + "&& isVisible", QtxPopupMgr::VisibleRule);
  popupMgr()->setRule( action( SMESHOp::OpOrientationOnFaces ), "facesOrientationMode = 'IsOriented'", QtxPopupMgr::ToggleRule );

  //-------------------------------------------------
  // Color / Size
  //-------------------------------------------------
  popupMgr()->insert( action( SMESHOp::OpProperties ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpProperties ), aMeshInVTK + "&& isVisible", QtxPopupMgr::VisibleRule );

  //-------------------------------------------------
  // Transparency
  //-------------------------------------------------
  popupMgr()->insert( action( SMESHOp::OpTransparency ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpTransparency ), aMeshInVTK + "&& isVisible", QtxPopupMgr::VisibleRule );

  //-------------------------------------------------
  // Controls
  //-------------------------------------------------
  QString
    aMeshInVtkHasNodes = aMeshInVTK + "&&" + hasNodes,
    aMeshInVtkHasEdges = aMeshInVTK + "&&" + hasEdges,
    aMeshInVtkHasFaces = aMeshInVTK + "&&" + hasFaces,
    aMeshInVtkHasVolumes = aMeshInVTK + "&&" + hasVolumes;

  anId = popupMgr()->insert( tr( "MEN_CTRL" ), -1, -1 );

  popupMgr()->insert( action( SMESHOp::OpReset ), anId, -1 ); // RESET
  popupMgr()->setRule( action( SMESHOp::OpReset ), aMeshInVTK + "&& controlMode <> 'eNone'", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), anId, -1 );

  int aSubId = popupMgr()->insert( tr( "MEN_NODE_CTRL" ), anId, -1 ); // NODE CONTROLS

  popupMgr()->insert( action( SMESHOp::OpFreeNode ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpFreeNode ), aMeshInVtkHasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpFreeNode ), "controlMode = 'eFreeNodes'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpEqualNode ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpEqualNode ), aMeshInVtkHasNodes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpEqualNode ), "controlMode = 'eCoincidentNodes'", QtxPopupMgr::ToggleRule);

  // popupMgr()->insert( action( SMESHOp::OpNodeConnectivityNb ), aSubId, -1 );
  // popupMgr()->setRule( action( SMESHOp::OpNodeConnectivityNb ), aMeshInVtkHasNodes, QtxPopupMgr::VisibleRule );
  // popupMgr()->setRule( action( SMESHOp::OpNodeConnectivityNb ), "controlMode = 'eNodeConnectivityNb'", QtxPopupMgr::ToggleRule );

  aSubId = popupMgr()->insert( tr( "MEN_EDGE_CTRL" ), anId, -1 ); // EDGE CONTROLS

  popupMgr()->insert( action( SMESHOp::OpFreeBorder ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpFreeBorder ), aMeshInVTK + "&&" + hasEdges + "&&" + hasFacesOrVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpFreeBorder ), "controlMode = 'eFreeBorders'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpLength ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpLength ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpLength ), "controlMode = 'eLength'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( action( SMESHOp::OpConnection ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpConnection ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpConnection ), "controlMode = 'eMultiConnection'", QtxPopupMgr::ToggleRule );
  popupMgr()->insert ( action( SMESHOp::OpEqualEdge ), aSubId, -1 ); // EQUAL_EDGE
  popupMgr()->setRule( action( SMESHOp::OpEqualEdge ), aMeshInVtkHasEdges, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpEqualEdge ), "controlMode = 'eCoincidentElems1D'", QtxPopupMgr::ToggleRule);

  aSubId = popupMgr()->insert( tr( "MEN_FACE_CTRL" ), anId, -1 ); // FACE CONTROLS

  popupMgr()->insert( action( SMESHOp::OpFreeEdge ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpFreeEdge ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpFreeEdge ), "controlMode = 'eFreeEdges'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpFreeFace ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpFreeFace ), aMeshInVtkHasFaces /*aMeshInVtkHasVolumes*/,
                                       QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpFreeFace ), "controlMode = 'eFreeFaces'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpLength2D ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpLength2D ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpLength2D ), "controlMode = 'eLength2D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpConnection2D ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpConnection2D ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpConnection2D ), "controlMode = 'eMultiConnection2D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpArea ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpArea ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpArea ), "controlMode = 'eArea'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpTaper ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpTaper ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpTaper ), "controlMode = 'eTaper'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpAspectRatio ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpAspectRatio ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpAspectRatio ), "controlMode = 'eAspectRatio'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpMinimumAngle ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpMinimumAngle ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpMinimumAngle ), "controlMode = 'eMinimumAngle'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpWarpingAngle ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpWarpingAngle ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpWarpingAngle ), "controlMode = 'eWarping'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpSkew ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpSkew ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpSkew ), "controlMode = 'eSkew'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpMaxElementLength2D ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpMaxElementLength2D ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpMaxElementLength2D ), "controlMode = 'eMaxElementLength2D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpBareBorderFace ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpBareBorderFace ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpBareBorderFace ), "controlMode = 'eBareBorderFace'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpOverConstrainedFace ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpOverConstrainedFace ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpOverConstrainedFace ), "controlMode = 'eOverConstrainedFace'", QtxPopupMgr::ToggleRule );
  popupMgr()->insert ( action( SMESHOp::OpEqualFace ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpEqualFace ), aMeshInVtkHasFaces, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpEqualFace ), "controlMode = 'eCoincidentElems2D'", QtxPopupMgr::ToggleRule );

  aSubId = popupMgr()->insert( tr( "MEN_VOLUME_CTRL" ), anId, -1 ); // VOLUME CONTROLS

  popupMgr()->insert ( action( SMESHOp::OpAspectRatio3D  ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpAspectRatio3D ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpAspectRatio3D ), "controlMode = 'eAspectRatio3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpVolume ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpVolume ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpVolume ), "controlMode = 'eVolume3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpMaxElementLength3D ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpMaxElementLength3D ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpMaxElementLength3D ), "controlMode = 'eMaxElementLength3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpBareBorderVolume ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpBareBorderVolume ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpBareBorderVolume ), "controlMode = 'eBareBorderVolume'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpOverConstrainedVolume ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpOverConstrainedVolume ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpOverConstrainedVolume ), "controlMode = 'eOverConstrainedVolume'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert ( action( SMESHOp::OpEqualVolume  ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpEqualVolume ), aMeshInVtkHasVolumes, QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpEqualVolume ), "controlMode = 'eCoincidentElems3D'", QtxPopupMgr::ToggleRule );

  popupMgr()->insert( separator(), anId, -1 );

  popupMgr()->insert( action( SMESHOp::OpShowScalarBar ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpShowScalarBar ), aMeshInVTK + "&& controlMode <> 'eNone'", QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpShowScalarBar ), aMeshInVTK + "&& controlMode <> 'eNone' && isScalarBarVisible", QtxPopupMgr::ToggleRule );
  popupMgr()->insert( action( SMESHOp::OpScalarBarProperties ), anId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpScalarBarProperties ), aMeshInVTK + "&& controlMode <> 'eNone'", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), anId, -1 );

  aSubId = popupMgr()->insert( tr( "MEN_DISTRIBUTION_CTRL" ), anId, -1 ); // NODE CONTROLS

  popupMgr()->insert( action( SMESHOp::OpSaveDistribution ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpSaveDistribution ), aMeshInVTK + "&& isNumFunctor", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( action( SMESHOp::OpShowDistribution ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpShowDistribution ), aMeshInVTK + "&& isNumFunctor", QtxPopupMgr::VisibleRule );
  popupMgr()->setRule( action( SMESHOp::OpShowDistribution ), aMeshInVTK + "&& isNumFunctor && isScalarBarVisible && isDistributionVisible", QtxPopupMgr::ToggleRule);

#ifndef DISABLE_PLOT2DVIEWER
  popupMgr()->insert( action( SMESHOp::OpPlotDistribution ), aSubId, -1 );
  popupMgr()->setRule( action( SMESHOp::OpPlotDistribution ), aMeshInVTK + "&& isNumFunctor", QtxPopupMgr::VisibleRule );
#endif

  //-------------------------------------------------
  // Show / Hide
  //-------------------------------------------------
  popupMgr()->insert( separator(), -1, -1 );
  QString aRule = "$component={'SMESH'} and ( type='Component' or (" + aClient + " and " +
    aType + " and " + aSelCount + " and " + anActiveVTK + " and " + isNotEmpty + " %1 ) )";
  popupMgr()->insert( action( SMESHOp::OpShow ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpShow ), aRule.arg( "and (not isVisible)" ), QtxPopupMgr::VisibleRule );

  popupMgr()->insert( action( SMESHOp::OpHide ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpHide ), aRule.arg( "and isVisible" ), QtxPopupMgr::VisibleRule );

  popupMgr()->insert( action( SMESHOp::OpShowOnly ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpShowOnly ), aRule.arg( "" ), QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), -1, -1 );

  //-------------------------------------------------
  // Clipping
  //-------------------------------------------------
  popupMgr()->insert( action( SMESHOp::OpClipping ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpClipping ), "client='VTKViewer'", QtxPopupMgr::VisibleRule );

  popupMgr()->insert( separator(), -1, -1 );

  popupMgr()->insert( action( SMESHOp::OpSortChild ), -1, -1 );
  popupMgr()->setRule( action( SMESHOp::OpSortChild ), "$component={'SMESH'} and client='ObjectBrowser' and isContainer and nbChildren>1", QtxPopupMgr::VisibleRule );
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
  return ( id == SMESHOp::OpCompute || id == SMESHOp::OpPreCompute || id == SMESHOp::OpEvaluate ) ? false : SalomeApp_Module::reusableOperation( id );
}

bool SMESHGUI::activateModule( SUIT_Study* study )
{
  bool res = SalomeApp_Module::activateModule( study );

  setMenuShown( true );
  setToolShown( true );

  // import Python module that manages SMESH plugins (need to be here because SalomePyQt API uses active module)
  PyGILState_STATE gstate = PyGILState_Ensure();
  PyObject* pluginsmanager = PyImport_ImportModuleNoBlock((char*)"salome_pluginsmanager");
  if ( !pluginsmanager ) {
    PyErr_Print();
  }
  else {
    PyObject* result = PyObject_CallMethod( pluginsmanager, (char*)"initialize", (char*)"isss",1,"smesh",tr("MEN_MESH").toUtf8().data(),tr("SMESH_PLUGINS_OTHER").toUtf8().data());
    if ( !result )
      PyErr_Print();
    Py_XDECREF(result);
  }
  PyGILState_Release(gstate);
  // end of SMESH plugins loading

  // Reset actions accelerator keys
  action(SMESHOp::OpDelete)->setEnabled(true); // Delete: Key_Delete

  //  0020210. Make SMESH_Gen update meshes at switching GEOM->SMESH
  GetSMESHGen()->SetCurrentStudy(SALOMEDS::Study::_nil());
  if ( SalomeApp_Study* s = dynamic_cast<SalomeApp_Study*>( study )) {
    if ( _PTR(Study) aStudy = s->studyDS() )
      GetSMESHGen()->SetCurrentStudy( _CAST(Study,aStudy)->GetStudy() );
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

  Py_XDECREF(pluginsmanager);
  return res;
}

bool SMESHGUI::deactivateModule( SUIT_Study* study )
{
  setMenuShown( false );
  setToolShown( false );

  EmitSignalCloseAllDialogs();

  // Unset actions accelerator keys
  action(SMESHOp::OpDelete)->setEnabled(false); // Delete: Key_Delete

  return SalomeApp_Module::deactivateModule( study );
}

void SMESHGUI::studyClosed( SUIT_Study* s )
{
  if( !s )
    return;
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
      QString aName = QString( SMESH::fromUtf8(obj->GetName()) );
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
  aMap.insert( SalomeApp_Application::WT_NoteBook, Qt::LeftDockWidgetArea );
#ifndef DISABLE_PYCONSOLE
  aMap.insert( SalomeApp_Application::WT_PyConsole, Qt::BottomDockWidgetArea );
#endif
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
    EmitSignalActivatedViewManager();
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
    if( theObject && (int) theEvent == SMESH::DeleteActorEvent ) {
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
  setPreferenceProperty( autoUpdate, "columns", 2 );
  int lim = addPreference( tr( "PREF_UPDATE_LIMIT" ), autoUpdate, LightApp_Preferences::IntSpin, "SMESH", "update_limit" );
  setPreferenceProperty( lim, "min",  0 );
  setPreferenceProperty( lim, "max",  100000000 );
  setPreferenceProperty( lim, "step", 1000 );
  setPreferenceProperty( lim, "special", tr( "PREF_UPDATE_LIMIT_NOLIMIT" ) );
  addPreference( tr( "PREF_INCREMENTAL_LIMIT" ), autoUpdate, LightApp_Preferences::Bool, "SMESH", "incremental_limit" );

  int dispgroup = addPreference( tr( "PREF_DISPLAY_MODE_GROUP" ), genTab );
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

  int arcgroup = addPreference( tr( "QUADRATIC_REPRESENT_MODE_GROUP" ), genTab );
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

  int qaGroup = addPreference( tr( "PREF_GROUP_QUALITY" ), genTab );
  setPreferenceProperty( qaGroup, "columns", 2 );
  addPreference( tr( "PREF_DISPLAY_ENTITY" ), qaGroup, LightApp_Preferences::Bool, "SMESH", "display_entity" );
  addPreference( tr( "PREF_PRECISION_USE" ), qaGroup, LightApp_Preferences::Bool, "SMESH", "use_precision" );
  int prec = addPreference( tr( "PREF_PRECISION_VALUE" ), qaGroup, LightApp_Preferences::IntSpin, "SMESH", "controls_precision" );
  setPreferenceProperty( prec, "min", 0 );
  setPreferenceProperty( prec, "max", 100 );
  int doubleNodesTol = addPreference( tr( "PREF_EQUAL_NODES_TOL" ), qaGroup, LightApp_Preferences::DblSpin, "SMESH", "equal_nodes_tolerance" );
  setPreferenceProperty( doubleNodesTol, "precision", 10 );
  setPreferenceProperty( doubleNodesTol, "min", 0.0000000001 );
  setPreferenceProperty( doubleNodesTol, "max", 1000000.0 );
  setPreferenceProperty( doubleNodesTol, "step", 0.0000001 );


  int exportgroup = addPreference( tr( "PREF_GROUP_EXPORT" ), genTab );
  setPreferenceProperty( exportgroup, "columns", 2 );
  addPreference( tr( "PREF_AUTO_GROUPS" ), exportgroup, LightApp_Preferences::Bool, "SMESH", "auto_groups" );
  addPreference( tr( "PREF_SHOW_WARN" ), exportgroup, LightApp_Preferences::Bool, "SMESH", "show_warning" );
  //addPreference( tr( "PREF_RENUMBER" ), exportgroup, LightApp_Preferences::Bool, "SMESH", "renumbering" );

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
  setPreferenceProperty( infoGroup, "columns", 2 );
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
  int ctrlLim = addPreference( tr( "PREF_CTRL_LIMIT" ), infoGroup, LightApp_Preferences::IntSpin, "SMESH", "info_controls_limit" );
  setPreferenceProperty( ctrlLim, "special", tr( "PREF_UPDATE_LIMIT_NOLIMIT" ) );
  setPreferenceProperty( ctrlLim, "min", 0 );
  setPreferenceProperty( ctrlLim, "max", 10000000 );
  setPreferenceProperty( ctrlLim, "step", 1000 );
  addPreference( tr( "PREF_ELEM_INFO_GRP_DETAILS" ), infoGroup, LightApp_Preferences::Bool, "SMESH", "elem_info_grp_details" );
  addPreference( tr( "PREF_DUMP_BASE_INFO" ), infoGroup, LightApp_Preferences::Bool, "SMESH", "info_dump_base" );
  addPreference( tr( "PREF_DUMP_ELEM_INFO" ), infoGroup, LightApp_Preferences::Bool, "SMESH", "info_dump_elem" );
  addPreference( tr( "PREF_DUMP_ADD_INFO"  ), infoGroup, LightApp_Preferences::Bool, "SMESH", "info_dump_add" );
  addPreference( tr( "PREF_DUMP_CTRL_INFO" ), infoGroup, LightApp_Preferences::Bool, "SMESH", "info_dump_ctrl" );

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
  setPreferenceProperty( chunkSize, "min",  1 );
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
    //aMarkerScaleValuesList  << QString::number( (i-(int)VTK::MS_10)*0.5 + 1.0 );
    aMarkerScaleValuesList  << QString::number( i );
  }
  setPreferenceProperty( markerScale, "strings", aMarkerScaleValuesList );
  setPreferenceProperty( markerScale, "indexes", aMarkerScaleIndicesList );

  int elemGroup = addPreference( tr( "PREF_GROUP_ELEMENTS" ), meshTab );
  //setPreferenceProperty( elemGroup, "columns", 2 );

  int ColorId = addPreference( tr( "PREF_FILL" ), elemGroup, LightApp_Preferences::BiColor, "SMESH", "fill_color" );
  setPreferenceProperty( ColorId, "text", tr("PREF_BACKFACE") );
  ColorId = addPreference( tr( "PREF_VOLUME" ), elemGroup, LightApp_Preferences::BiColor, "SMESH", "volume_color" );
  setPreferenceProperty( ColorId, "text", tr("PREF_REVERSEDVOLUME") );
  addPreference( tr( "PREF_COLOR_0D" ), elemGroup, LightApp_Preferences::Color, "SMESH", "elem0d_color" );
  addPreference( tr( "PREF_BALL_COLOR" ), elemGroup, LightApp_Preferences::Color, "SMESH", "ball_elem_color" );
  addPreference( tr( "PREF_OUTLINE"  ), elemGroup, LightApp_Preferences::Color, "SMESH", "outline_color" );
  addPreference( tr( "PREF_WIREFRAME"  ), elemGroup, LightApp_Preferences::Color, "SMESH", "wireframe_color" );
  addPreference( tr( "PREF_PREVIEW_COLOR"  ), elemGroup, LightApp_Preferences::BiColor, "SMESH", "preview_color" );


  int grpGroup = addPreference( tr( "PREF_GROUP_GROUPS" ), meshTab );
  setPreferenceProperty( grpGroup, "columns", 2 );

  addPreference( tr( "PREF_GRP_NAMES" ), grpGroup, LightApp_Preferences::Color, "SMESH", "group_name_color" );
  addPreference( tr( "PREF_GRP_DEF_COLOR" ), grpGroup, LightApp_Preferences::Color, "SMESH", "default_grp_color" );

  int size0d = addPreference(tr("PREF_SIZE_0D"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "elem0d_size");
  /* int ballSize = addPreference(tr("PREF_BALL_SIZE"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "ball_elem_size"); */
  double ballDiameter = addPreference(tr("PREF_BALL_DIAMETER"), elemGroup,
                             LightApp_Preferences::DblSpin, "SMESH", "ball_elem_diameter");
  double ballScale = addPreference(tr("PREF_BALL_SCALE"), elemGroup,
                             LightApp_Preferences::DblSpin, "SMESH", "ball_elem_scale");
  int elemW  = addPreference(tr("PREF_WIDTH"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "element_width");
  int outW  = addPreference(tr("PREF_OUTLINE_WIDTH"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "outline_width");
  int shrink = addPreference(tr("PREF_SHRINK_COEFF"), elemGroup,
                             LightApp_Preferences::IntSpin, "SMESH", "shrink_coeff");

  setPreferenceProperty( size0d, "min", 1 );
  setPreferenceProperty( size0d, "max", 10 );

 // setPreferenceProperty( ballSize, "min", 1 );
 // setPreferenceProperty( ballSize, "max", 10 );

  setPreferenceProperty( ballDiameter, "min", 1e-7 );
  setPreferenceProperty( ballDiameter, "max", 1e9 );
  setPreferenceProperty( ballDiameter, "step", 0.1 );

  setPreferenceProperty( ballScale, "min", 1e-2 );
  setPreferenceProperty( ballScale, "max", 1e7 );
  setPreferenceProperty( ballScale, "step", 0.5 );

  setPreferenceProperty( elemW, "min", 1 );
  setPreferenceProperty( elemW, "max", 5 );

  setPreferenceProperty( outW, "min", 1 );
  setPreferenceProperty( outW, "max", 5 );

  setPreferenceProperty( shrink, "min", 0 );
  setPreferenceProperty( shrink, "max", 100 );

  int numGroup = addPreference( tr( "PREF_GROUP_NUMBERING" ), meshTab );
  setPreferenceProperty( numGroup, "columns", 2 );
  
  addPreference( tr( "PREF_NUMBERING_NODE" ), numGroup, LightApp_Preferences::Color, "SMESH", "numbering_node_color" );
  addVtkFontPref( tr( "PREF_NUMBERING_FONT" ), numGroup, "numbering_node_font", true );

  addPreference( tr( "PREF_NUMBERING_ELEM" ), numGroup, LightApp_Preferences::Color, "SMESH", "numbering_elem_color" );
  addVtkFontPref( tr( "PREF_NUMBERING_FONT" ), numGroup, "numbering_elem_font", true );

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
  if ( sect=="SMESH" ) {
    float sbX1 = 0.01, sbY1 = 0.01, sbW = 0.08, sbH = 0.08;
    float aTol = 1.00000009999999;
    std::string aWarning;
    SUIT_ResourceMgr* aResourceMgr = SMESH::GetResourceMgr(this);

    if ( name== "selection_object_color" ||
         name=="selection_element_color" ||
         name==        "highlight_color" ||
         name=="selection_precision_node"    ||
         name=="selection_precision_element" ||
         name=="selection_precision_object"   )
    {
      SMESH::UpdateSelectionProp( this );
    }
    else if (name == "scalar_bar_vertical_x" || name == "scalar_bar_vertical_width")
    {
      sbX1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_x",     sbX1);
      sbW  = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_width", sbW);
      if ( sbX1+sbW > aTol ) {
        aWarning = "Origin and Size Vertical: X+Width > 1\n";
        sbX1 = 0.01;
        sbW  = 0.08;
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_x",     sbX1);
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_width", sbW);
      }
    }
    else if (name == "scalar_bar_vertical_y" || name == "scalar_bar_vertical_height" )
    {
      sbY1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_y",     sbY1);
      sbH  = aResourceMgr->doubleValue("SMESH", "scalar_bar_vertical_height",sbH);
      if ( sbY1 + sbH > aTol ) {
        aWarning = "Origin and Size Vertical: Y+Height > 1\n";
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_y",     sbY1);
        aResourceMgr->setValue("SMESH", "scalar_bar_vertical_height",sbH);
      }
    }
    else if (name == "scalar_bar_horizontal_x" || name ==  "scalar_bar_horizontal_width")
    {
      sbX1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_x",     sbX1);
      sbW  = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_width", sbW);
      if ( sbX1 + sbW > aTol ) {
        aWarning = "Origin and Size Horizontal: X+Width > 1\n";
        sbX1=0.1;
        sbW =0.08;
        aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_x", sbX1);
        aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_width", sbW);
      }
    }
    else if (name == "scalar_bar_horizontal_y" || name ==  "scalar_bar_horizontal_height")
    {
      sbY1 = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_y",     sbY1);
      sbH  = aResourceMgr->doubleValue("SMESH", "scalar_bar_horizontal_height",sbH);
      if ( sbY1 + sbH > aTol ) {
        aWarning = "Origin and Size Horizontal: Y+Height > 1\n";
        sbY1=0.01;
        sbH =0.08;
        aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_y", sbY1);
        aResourceMgr->setValue("SMESH", "scalar_bar_horizontal_height",sbH);
      }
    }
    else if ( name == "segmentation" )
    {
      int nbSeg = aResourceMgr->integerValue( "SMESH", "segmentation", 10 );
      myComponentSMESH->SetBoundaryBoxSegmentation( nbSeg );
    }
    else if ( name == "nb_segments_per_edge" )
    {
      int nbSeg = aResourceMgr->integerValue( "SMESH", "nb_segments_per_edge", 15 );
      myComponentSMESH->SetDefaultNbSegments( nbSeg );
    }
    else if ( name == "historical_python_dump" || name == "forget_mesh_on_hyp_modif" || name == "default_grp_color" )
    {
      QString val = aResourceMgr->stringValue( "SMESH", name );
      myComponentSMESH->SetOption( name.toLatin1().constData(), val.toLatin1().constData() );
    }
    else if ( name == "numbering_node_color" || name == "numbering_node_font" )
    {
      SMESH::UpdateFontProp( this );
    }
    else if ( name == "numbering_elem_color" || name == "numbering_elem_font" )
    {
      SMESH::UpdateFontProp( this );
    }

    if ( aWarning.size() != 0 ) {
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
    case SMESHOp::OpSplitBiQuadratic:
      op = new SMESHGUI_SplitBiQuadOp();
    break;
    case SMESHOp::OpConvertMeshToQuadratic:
      op = new SMESHGUI_ConvToQuadOp();
    break;
    case SMESHOp::OpCreateBoundaryElements: // create 2D mesh as boundary on 3D
      op = new SMESHGUI_Make2DFrom3DOp();
    break;
    case SMESHOp::OpReorientFaces:
      op = new SMESHGUI_ReorientFacesOp();
      break;
    case SMESHOp::OpCreateMesh:
      op = new SMESHGUI_MeshOp( true, true );
    break;
    case SMESHOp::OpCreateSubMesh:
      op = new SMESHGUI_MeshOp( true, false );
    break;
    case SMESHOp::OpEditMeshOrSubMesh:
    case SMESHOp::OpEditMesh:
    case SMESHOp::OpEditSubMesh:
      op = new SMESHGUI_MeshOp( false );
    break;
    case SMESHOp::OpCompute:
    case SMESHOp::OpComputeSubMesh:
      op = new SMESHGUI_ComputeOp();
    break;
    case SMESHOp::OpPreCompute:
      op = new SMESHGUI_PrecomputeOp();
    break;
    case SMESHOp::OpEvaluate:
      op = new SMESHGUI_EvaluateOp();
    break;
    case SMESHOp::OpMeshOrder:
      op = new SMESHGUI_MeshOrderOp();
    break;
    case SMESHOp::OpCreateGeometryGroup:
      op = new SMESHGUI_GroupOnShapeOp();
      break;
    case SMESHOp::OpFindElementByPoint:
      op = new SMESHGUI_FindElemByPointOp();
      break;
    case SMESHOp::OpMoveNode: // Make mesh pass through point
      op = new SMESHGUI_MakeNodeAtPointOp();
      break;
    case SMESHOp::OpElem0DOnElemNodes: // Create 0D elements on all nodes
      op = new SMESHGUI_Add0DElemsOnAllNodesOp();
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
  aSColor.R = aColor.redF();
  aSColor.G = aColor.greenF();
  aSColor.B = aColor.blueF();

  return aSColor;
}

const char* gSeparator = "_"; // character used to separate parameter names
const char* gDigitsSep = ":"; // character used to separate numeric parameter values (color = r:g:b)
const char* gPathSep   = "|"; // character used to separate paths

/*!
 * \brief Store visual parameters
 *
 * This method is called just before the study document is saved.
 * Store visual parameters in AttributeParameter attribue(s)
 */
void SMESHGUI::storeVisualParameters (int savePoint)
{
  // localizing
  Kernel_Utils::Localizer loc;

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

          QString aPropertyValue = QString::number( (int)aPlane->PlaneMode ).toLatin1().constData();
          aPropertyValue += gDigitsSep;
          aPropertyValue += QString::number( aPlane->IsOpenGLClipping ).toLatin1().constData();
          aPropertyValue += gDigitsSep;
          if ( aPlane->PlaneMode == SMESH::Absolute ) {
            aPropertyValue += QString::number( aPlane->myAbsoluteOrientation ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->X ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->Y ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->Z ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->Dx ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->Dy ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->Dz ).toLatin1().constData();
          }
          else if ( aPlane->PlaneMode == SMESH::Relative ) {
            aPropertyValue += QString::number( (int)aPlane->myRelativeOrientation ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->GetDistance() ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->myAngle[0] ).toLatin1().constData();
            aPropertyValue += gDigitsSep;
            aPropertyValue += QString::number( aPlane->myAngle[1] ).toLatin1().constData();
          }

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
                  bool isE  = aMode & SMESH_Actor::eEdges;
                  bool isF  = aMode & SMESH_Actor::eFaces;
                  bool isV  = aMode & SMESH_Actor::eVolumes;
                  bool is0d = aMode & SMESH_Actor::e0DElements;
                  bool isB  = aMode & SMESH_Actor::eBallElem;

                  QString modeStr ("e");
                  modeStr += gDigitsSep; modeStr += QString::number(isE);
                  modeStr += gDigitsSep; modeStr += "f";
                  modeStr += gDigitsSep; modeStr += QString::number(isF);
                  modeStr += gDigitsSep; modeStr += "v";
                  modeStr += gDigitsSep; modeStr += QString::number(isV);
                  modeStr += gDigitsSep; modeStr += "0d";
                  modeStr += gDigitsSep; modeStr += QString::number(is0d);
                  modeStr += gDigitsSep; modeStr += "b";
                  modeStr += gDigitsSep; modeStr += QString::number(isB);

                  param = vtkParam + "Entities";
                  ip->setParameter(entry, param, modeStr.toLatin1().data());

                  // Colors
                  double r, g, b;
                  int delta;

                  aSmeshActor->GetSufaceColor(r, g, b, delta);
                  QStringList colorStr;
                  colorStr << "surface";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  colorStr << "backsurface";
                  colorStr << QString::number(delta);

                  aSmeshActor->GetVolumeColor(r, g, b, delta);
                  colorStr << "volume";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);
                  colorStr << QString::number(delta);

                  aSmeshActor->GetEdgeColor(r, g, b);
                  colorStr << "edge";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  aSmeshActor->GetNodeColor(r, g, b);
                  colorStr << "node";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  aSmeshActor->GetOutlineColor(r, g, b);
                  colorStr << "outline";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  aSmeshActor->Get0DColor(r, g, b);
                  colorStr << "elem0d";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  aSmeshActor->GetBallColor(r, g, b);
                  colorStr << "ball";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  aSmeshActor->GetFacesOrientationColor(r, g, b);
                  colorStr << "orientation";
                  colorStr << QString::number(r);
                  colorStr << QString::number(g);
                  colorStr << QString::number(b);

                  param = vtkParam + "Colors";
                  ip->setParameter(entry, param, qPrintable(colorStr.join(gDigitsSep)));

                  // Sizes
                  QStringList sizeStr;
                  sizeStr << "line";
                  sizeStr << QString::number((int)aSmeshActor->GetLineWidth());
                  sizeStr << "outline";
                  sizeStr << QString::number((int)aSmeshActor->GetOutlineWidth());
                  sizeStr << "elem0d";
                  sizeStr << QString::number((int)aSmeshActor->Get0DSize());
                  sizeStr << "ball";
                  //sizeStr << QString::number((int)aSmeshActor->GetBallSize());
                  sizeStr << QString::number((double)aSmeshActor->GetBallSize());
                  sizeStr << QString::number((double)aSmeshActor->GetBallScale());
                  sizeStr << "shrink";
                  sizeStr << QString::number(aSmeshActor->GetShrinkFactor());
                  sizeStr << "orientation";
                  sizeStr << QString::number(aSmeshActor->GetFacesOrientationScale());
                  sizeStr << QString::number(aSmeshActor->GetFacesOrientation3DVectors());

                  param = vtkParam + "Sizes";
                  ip->setParameter(entry, param, qPrintable(sizeStr.join(gDigitsSep)));

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
  int Mode;
  bool isOpenGLClipping;
  vtkIdType RelativeOrientation;
  double Distance;
  double Angle[2];
  int AbsoluteOrientation;
  double X, Y, Z, Dx, Dy, Dz;
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
  // localizing
  Kernel_Utils::Localizer loc;

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
      if( aPropertyValueList.size() != 6 && aPropertyValueList.size() != 9 )
        continue;

      TPlaneData aPlaneData;
      aPlaneData.AbsoluteOrientation = false;
      aPlaneData.RelativeOrientation = 0;
      aPlaneData.Distance = aPlaneData.Angle[0] = aPlaneData.Angle[1] = 0;
      aPlaneData.X = aPlaneData.Y = aPlaneData.Z = 0;
      aPlaneData.Dx = aPlaneData.Dy = aPlaneData.Dz = 0;

      aPlaneData.Id = aClippingPlaneId;

      ok = false;
      aPlaneData.Mode = aPropertyValueList[0].toInt( &ok );
      if( !ok )
        continue;
      
      ok = false;
      aPlaneData.isOpenGLClipping = aPropertyValueList[1].toInt( &ok );
      if( !ok )
        continue;

      if ( (SMESH::Mode)aPlaneData.Mode == SMESH::Absolute )
      {
        ok = false;
        aPlaneData.AbsoluteOrientation = aPropertyValueList[2].toInt( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.X = aPropertyValueList[3].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Y = aPropertyValueList[4].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Z = aPropertyValueList[5].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Dx = aPropertyValueList[6].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Dy = aPropertyValueList[7].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Dz = aPropertyValueList[8].toDouble( &ok );
        if( !ok )
          continue;
      }
      else if ( (SMESH::Mode)aPlaneData.Mode == SMESH::Relative ) {
        ok = false;
        aPlaneData.RelativeOrientation = aPropertyValueList[2].toInt( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Distance = aPropertyValueList[3].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Angle[0] = aPropertyValueList[4].toDouble( &ok );
        if( !ok )
          continue;

        ok = false;
        aPlaneData.Angle[1] = aPropertyValueList[5].toDouble( &ok );
        if( !ok )
          continue;
      }

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
                  Handle(SALOME_InteractiveObject) io = aGeomAc->getIO();
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
              int aEntityMode = SMESH_Actor::eAllEntity;
              for ( int i = 0; i < mode.count(); i+=2 ) {
                if ( i < mode.count()-1 ) {
                  QString type = mode[i];
                  bool val = mode[i+1].toInt();
                  if      ( type == "e" && !val )
                    aEntityMode = aEntityMode & ~SMESH_Actor::eEdges;
                  else if ( type == "f" && !val )
                    aEntityMode = aEntityMode & ~SMESH_Actor::eFaces;
                  else if ( type == "v" && !val )
                    aEntityMode = aEntityMode & ~SMESH_Actor::eVolumes;
                  else if ( type == "0d" && !val )
                    aEntityMode = aEntityMode & ~SMESH_Actor::e0DElements;
                  else if ( type == "b" && !val )
                    aEntityMode = aEntityMode & ~SMESH_Actor::eBallElem;
                }
              }
              aSmeshActor->SetEntityMode( aEntityMode );
            }
            // Colors
            else if (paramNameStr == "Colors") {
              QStringList colors = val.split(gDigitsSep, QString::SkipEmptyParts);
              QColor nodeColor;
              QColor edgeColor;
              QColor faceColor;
              QColor volumeColor;
              QColor elem0dColor;
              QColor ballColor;
              QColor outlineColor;
              QColor orientationColor;
              int deltaF;
              int deltaV;
              QColor c;
              double r, g, b;
              bool bOk;
              // below lines are required to get default values for delta coefficients
              // of backface color for faces and color of reversed volumes
              SMESH::GetColor( "SMESH", "fill_color",   c, deltaF, "0,170,255|-100" );
              SMESH::GetColor( "SMESH", "volume_color", c, deltaV, "255,0,170|-100" );
              for ( int i = 0; i < colors.count(); i++ ) {
                QString type = colors[i];
                if ( type == "surface" ) {
                  // face color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  faceColor.setRgbF( r, g, b );
                  i += 3;
                }
                else if ( type == "backsurface" ) {
                  // backface color can be defined in several ways
                  // - in old versions, it is set as rgb triple r:g:b - this was is unsupported now
                  // - in latest versions, it is set as delta coefficient
                  bool rgbOk = false, deltaOk;
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  int delta = colors[i+1].toInt( &deltaOk );
                  i++;                                 // shift index
                  if ( i+1 < colors.count() )          // index is shifted to 1
                    g = colors[i+1].toDouble( &rgbOk );
                  if ( rgbOk ) i++;                    // shift index
                  if ( rgbOk && i+1 < colors.count() ) // index is shifted to 2
                    b = colors[i+1].toDouble( &rgbOk );
                  if ( rgbOk ) i++;
                  // - as currently there's no way to set directly backsurface color as it was before,
                  // we ignore old dump where r,g,b triple was set
                  // - also we check that delta parameter is set properly
                  if ( !rgbOk && deltaOk )
                    deltaF = delta;
                }
                else if ( type == "volume" ) {
                  // volume color is set by 4 values r:g:b:delta, where
                  // - r,g,b - is a normal volume rgb color components
                  // - delta - is a reversed volume color delta coefficient
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+4 >= colors.count() ) break;                  // format error
                  int delta = colors[i+4].toInt( &bOk );
                  if ( !bOk ) break;                                   // format error
                  volumeColor.setRgbF( r, g, b );
                  deltaV = delta;
                  i += 4;
                }
                else if ( type == "edge" ) {
                  // edge color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  edgeColor.setRgbF( r, g, b );
                  i += 3;
                }
                else if ( type == "node" ) {
                  // node color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  nodeColor.setRgbF( r, g, b );
                  i += 3;
                }
                else if ( type == "elem0d" ) {
                  // 0d element color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  elem0dColor.setRgbF( r, g, b );
                  i += 3;
                }
                else if ( type == "ball" ) {
                  // ball color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  ballColor.setRgbF( r, g, b );
                  i += 3;
                }
                else if ( type == "outline" ) {
                  // outline color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  outlineColor.setRgbF( r, g, b );
                  i += 3;
                }
                else if ( type == "orientation" ) {
                  // orientation color is set by 3 values r:g:b, where
                  // - r,g,b - is rgb color components
                  if ( i+1 >= colors.count() ) break;                  // format error
                  r = colors[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= colors.count() ) break;                  // format error
                  g = colors[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+3 >= colors.count() ) break;                  // format error
                  b = colors[i+3].toDouble( &bOk ); if ( !bOk ) break; // format error
                  orientationColor.setRgbF( r, g, b );
                  i += 3;
                }
              }
              // node color
              if ( nodeColor.isValid() )
                aSmeshActor->SetNodeColor( nodeColor.redF(), nodeColor.greenF(), nodeColor.blueF() );
              // edge color
              if ( edgeColor.isValid() )
                aSmeshActor->SetEdgeColor( edgeColor.redF(), edgeColor.greenF(), edgeColor.blueF() );
              // face color
              if ( faceColor.isValid() )
                aSmeshActor->SetSufaceColor( faceColor.redF(), faceColor.greenF(), faceColor.blueF(), deltaF );
              // volume color
              if ( volumeColor.isValid() )
                aSmeshActor->SetVolumeColor( volumeColor.redF(), volumeColor.greenF(), volumeColor.blueF(), deltaV );
              else if ( faceColor.isValid() ) // backward compatibility (no separate color for volumes)
                aSmeshActor->SetVolumeColor( faceColor.redF(), faceColor.greenF(), faceColor.blueF(), deltaF );
              // 0d element color
              if ( elem0dColor.isValid() )
                aSmeshActor->Set0DColor( elem0dColor.redF(), elem0dColor.greenF(), elem0dColor.blueF() );
              // ball color
              if ( ballColor.isValid() )
                aSmeshActor->SetBallColor( ballColor.redF(), ballColor.greenF(), ballColor.blueF() );
              // outline color
              if ( outlineColor.isValid() )
                aSmeshActor->SetOutlineColor( outlineColor.redF(), outlineColor.greenF(), outlineColor.blueF() );
              // orientation color
              if ( orientationColor.isValid() )
                aSmeshActor->SetFacesOrientationColor( orientationColor.redF(), orientationColor.greenF(), orientationColor.blueF() );
            }
            // Sizes
            else if (paramNameStr == "Sizes") {
              QStringList sizes = val.split(gDigitsSep, QString::SkipEmptyParts);
              bool bOk;
              int lineWidth = -1;
              int outlineWidth = -1;
              int elem0dSize = -1;
              //int ballSize = -1;
              double ballDiameter = -1.0;
              double ballScale = -1.0;
              double shrinkSize = -1;
              double orientationSize = -1;
              bool orientation3d = false;
              for ( int i = 0; i < sizes.count(); i++ ) {
                QString type = sizes[i];
                if ( type == "line" ) {
                  // line (wireframe) width is given as single integer value
                  if ( i+1 >= sizes.count() ) break;                    // format error
                  int v = sizes[i+1].toInt( &bOk ); if ( !bOk ) break;  // format error
                  lineWidth = v;
                  i++;
                }
                if ( type == "outline" ) {
                  // outline width is given as single integer value
                  if ( i+1 >= sizes.count() ) break;                    // format error
                  int v = sizes[i+1].toInt( &bOk ); if ( !bOk ) break;  // format error
                  outlineWidth = v;
                  i++;
                }
                else if ( type == "elem0d" ) {
                  // 0d element size is given as single integer value
                  if ( i+1 >= sizes.count() ) break;                    // format error
                  int v = sizes[i+1].toInt( &bOk ); if ( !bOk ) break;  // format error
                  elem0dSize = v;
                  i++;
                }
                else if ( type == "ball" ) {
                  // balls are specified by two values: size:scale, where
                  // - size - is a integer value specifying size
                  // - scale - is a double value specifying scale factor
                  if ( i+1 >= sizes.count() ) break;                       // format error
                  //int v1 = sizes[i+1].toInt( &bOk ); if ( !bOk ) break;    // format error
                  double v1 = sizes[i+1].toInt( &bOk ); if ( !bOk ) break;    // format error
                  if ( i+2 >= sizes.count() ) break;                       // format error
                  double v2 = sizes[i+2].toDouble( &bOk ); if ( !bOk ) break; // format error
                  //ballSize = v1;
                  ballDiameter = v1;
                  ballScale = v2;
                  i += 2;
                }
                else if ( type == "shrink" ) {
                  // shrink factor is given as single floating point value
                  if ( i+1 >= sizes.count() ) break;                          // format error
                  double v = sizes[i+1].toDouble( &bOk ); if ( !bOk ) break;  // format error
                  shrinkSize = v;
                  i++;
                }
                else if ( type == "orientation" ) {
                  // orientation vectors are specified by two values size:3d, where
                  // - size - is a floating point value specifying scale factor
                  // - 3d - is a boolean
                  if ( i+1 >= sizes.count() ) break;                          // format error
                  double v1 = sizes[i+1].toDouble( &bOk ); if ( !bOk ) break; // format error
                  if ( i+2 >= sizes.count() ) break;                          // format error
                  int v2 = sizes[i+2].toInt( &bOk ); if ( !bOk ) break;       // format error
                  orientationSize = v1;
                  orientation3d = (bool)v2;
                  i += 2;
                }
              }
              // line (wireframe) width
              if ( lineWidth > 0 )
                aSmeshActor->SetLineWidth( lineWidth );
              // outline width
              if ( outlineWidth > 0 )
                aSmeshActor->SetOutlineWidth( outlineWidth );
              else if ( lineWidth > 0 ) // backward compatibility (no separate width for outlines)
                aSmeshActor->SetOutlineWidth( lineWidth );
              // 0d element size
              if ( elem0dSize > 0 )
                aSmeshActor->Set0DSize( elem0dSize );
              // ball size
              /*if ( ballSize > 0 )
                aSmeshActor->SetBallSize( ballSize );*/
              // ball diameter
              if ( ballDiameter > 0 )
                aSmeshActor->SetBallSize( ballDiameter );
              // ball scale
              if ( ballScale > 0.0 )
                aSmeshActor->SetBallScale( ballScale );
              // shrink factor
              if ( shrinkSize > 0 )
                aSmeshActor->SetShrinkFactor( shrinkSize );
              // orientation vectors
              if ( orientationSize > 0 ) {
                aSmeshActor->SetFacesOrientationScale( orientationSize );
                aSmeshActor->SetFacesOrientation3DVectors( orientation3d );
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
              // old format - val looks like "Off" or "1:0:0:0.5:0:0" 
              // (mode(relative), is OpenGL clipping plane, orientation, distance, two angles)
              // or "0:1:1:10.5:1.0:1.0:15.0:10.0:10.0" 
              // (mode(absolute), is OpenGL clipping plane, orientation, base point(x, y, z), direction (dx, dy, dz))
              // new format - val looks like "Off" or "0" (plane id)
              // (note: in new format "Off" value is used only for consistency,
              //  so it is processed together with values in old format)
              bool anIsOldFormat = ( vals.count() == 6 || vals.count() == 9 || val == "Off" );
              if( anIsOldFormat ) {
                if (paramNameStr == "ClippingPlane1" || val == "Off")
                  aSmeshActor->RemoveAllClippingPlanes();
                if (val != "Off") {
                  QList<SUIT_ViewManager*> lst;
                  getApp()->viewManagers(viewerTypStr, lst);
                  // SVTK ViewManager always has 1 ViewWindow, so view index is index of view manager
                  if (viewIndex >= 0 && viewIndex < lst.count()) {
                    SUIT_ViewManager* vman = lst.at(viewIndex);
                    SVTK_ViewWindow* vtkView = (SVTK_ViewWindow*) vman->getActiveView();

                    SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = myClippingPlaneInfoMap[ vman ];

                    SMESH::TActorList anActorList;
                    anActorList.push_back( aSmeshActor );
                    SMESH::OrientedPlane* aPlane = SMESH::OrientedPlane::New( vtkView );
                    aPlane->myViewWindow = vtkView;
                    SMESH::Mode aMode = ( SMESH::Mode )vals[0].toInt();
                    aPlane->PlaneMode = aMode;
                    bool isOpenGLClipping = ( bool )vals[1].toInt();
                    aPlane->IsOpenGLClipping = isOpenGLClipping;
                    if ( aMode == SMESH::Absolute ) {
                      aPlane->myAbsoluteOrientation = vals[2].toInt();
                      aPlane->X = vals[3].toFloat();
                      aPlane->Y = vals[4].toFloat();
                      aPlane->Z = vals[5].toFloat();
                      aPlane->Dx = vals[6].toFloat();
                      aPlane->Dy = vals[7].toFloat();
                      aPlane->Dz = vals[8].toFloat();
                    }
                    else if ( aMode == SMESH::Relative ) {
                      aPlane->myRelativeOrientation = (SMESH::Orientation)vals[2].toInt();
                      aPlane->myDistance = vals[3].toFloat();
                      aPlane->myAngle[0] = vals[4].toFloat();
                      aPlane->myAngle[1] = vals[5].toFloat();
                    }

                    if( aPlane ) {
                      if ( SMESHGUI_ClippingDlg::AddPlane( anActorList, aPlane ) ) {
                        SMESH::ClippingPlaneInfo aClippingPlaneInfo;
                        aClippingPlaneInfo.Plane = aPlane;
                        aClippingPlaneInfo.ActorList = anActorList;
                        aClippingPlaneInfoList.push_back( aClippingPlaneInfo );
                      }
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
          SMESH::OrientedPlane* aPlane = SMESH::OrientedPlane::New( aViewWindow );
          aPlane->myViewWindow = aViewWindow;
          aPlane->PlaneMode = (SMESH::Mode)aPlaneData.Mode;
          aPlane->IsOpenGLClipping = aPlaneData.isOpenGLClipping;
          if ( aPlane->PlaneMode == SMESH::Absolute ) {
            aPlane->myAbsoluteOrientation = aPlaneData.AbsoluteOrientation;
            aPlane->X = aPlaneData.X;
            aPlane->Y = aPlaneData.Y;
            aPlane->Z = aPlaneData.Z;
            aPlane->Dx = aPlaneData.Dx;
            aPlane->Dy = aPlaneData.Dy;
            aPlane->Dz = aPlaneData.Dz;
          }
          else if ( aPlane->PlaneMode == SMESH::Relative ) {
            aPlane->myRelativeOrientation = (SMESH::Orientation)aPlaneData.RelativeOrientation;
            aPlane->myDistance = aPlaneData.Distance;
            aPlane->myAngle[0] = aPlaneData.Angle[0];
            aPlane->myAngle[1] = aPlaneData.Angle[1];
          }
          if( aPlane ) {
            if ( SMESHGUI_ClippingDlg::AddPlane( anActorList, aPlane ) ) {
              SMESH::ClippingPlaneInfo aClippingPlaneInfo;
              aClippingPlaneInfo.Plane = aPlane;
              aClippingPlaneInfo.ActorList = anActorList;
              aClippingPlaneInfoList.push_back( aClippingPlaneInfo );
            }
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
      // set OpenGL clipping planes
      VTK::ActorCollectionCopy aCopy( vtkView->getRenderer()->GetActors() );
      vtkActorCollection* anAllActors = aCopy.GetActors();
      anAllActors->InitTraversal();
      while( vtkActor* aVTKActor = anAllActors->GetNextActor() )
        if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
          anActor->SetOpenGLClippingPlane();
      
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
int SMESHGUI::addVtkFontPref( const QString& label, const int pId, const QString& param, const bool needSize )
{
  int tfont = addPreference( label, pId, LightApp_Preferences::Font, "SMESH", param );

  setPreferenceProperty( tfont, "mode", QtxFontEdit::Custom );

  QStringList fam;
  fam.append( tr( "SMESH_FONT_ARIAL" ) );
  fam.append( tr( "SMESH_FONT_COURIER" ) );
  fam.append( tr( "SMESH_FONT_TIMES" ) );

  setPreferenceProperty( tfont, "fonts", fam );

  int f = QtxFontEdit::Family | QtxFontEdit::Bold | QtxFontEdit::Italic | QtxFontEdit::Shadow;
  if ( needSize ) f = f | QtxFontEdit::Size;
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
  \brief Actions after choosing menu of control modes
  Updates control mode actions according to current selection
*/
void SMESHGUI::onUpdateControlActions()
{
  SALOME_ListIO selected;
  if ( LightApp_SelectionMgr* aSel = SMESHGUI::selectionMgr() )
    aSel->selectedObjects( selected );

  SMESH_Actor::eControl aControl = SMESH_Actor::eNone;
  if ( selected.Extent() ) {
    if ( selected.First()->hasEntry() ) {
      if ( SMESH_Actor* anActor = SMESH::FindActorByEntry( selected.First()->getEntry() )) {
        aControl = anActor->GetControlMode();
        SALOME_ListIteratorOfListIO it(selected);
        for ( it.Next(); it.More(); it.Next() ) {
          Handle(SALOME_InteractiveObject) anIO = it.Value();
          if ( anIO->hasEntry() ) {
            if ( SMESH_Actor* anActor = SMESH::FindActorByEntry( anIO->getEntry() ) ) {
              if ( aControl != anActor->GetControlMode() ) {
                aControl = SMESH_Actor::eNone;
                break;
              }
            }
          }
        }
      }
    }
  }

  int anAction = ActionToControl( aControl, true );
  if ( anAction)
    action( anAction )->setChecked( true );
  else {
    QMenu* send = (QMenu*)sender();
    QList<QAction*> actions = send->actions();
    for ( int i = 0; i < actions.size(); i++ )
      actions[i]->setChecked( false );
  }
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
  EmitSignalCloseView();
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
        name = SMESH::fromUtf8(obj->GetName());
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
  if (aType == SMESH::MESH || aType == SMESH::GROUP ||
      aType == SMESH::SUBMESH || aType == SMESH::SUBMESH_COMPOUND ||
      aType == SMESH::SUBMESH_SOLID || aType == SMESH::SUBMESH_FACE ||
      aType == SMESH::SUBMESH_EDGE || aType == SMESH::SUBMESH_VERTEX ||
      aType == SMESH::HYPOTHESIS || aType == SMESH::ALGORITHM)
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
      if (aType == SMESH::MESH || aType == SMESH::GROUP ||
          aType == SMESH::SUBMESH || aType == SMESH::SUBMESH_COMPOUND ||
          aType == SMESH::SUBMESH_SOLID || aType == SMESH::SUBMESH_FACE ||
          aType == SMESH::SUBMESH_EDGE || aType == SMESH::SUBMESH_VERTEX ||
          aType == SMESH::HYPOTHESIS || aType == SMESH::ALGORITHM) {
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

SALOMEDS::Color SMESHGUI::getPredefinedUniqueColor()
{
  static QList<QColor> colors;

  if ( colors.isEmpty() ) {

    for (int s = 0; s < 2 ; s++)
    {
      for (int v = 100; v >= 40; v = v - 20)
      {
        for (int h = 0; h < 359 ; h = h + 60)
        {
          colors.append(QColor::fromHsv(h, 255 - s * 127, v * 255 / 100));
        }
      }
    }
  }
  static int currentColor = randomize( colors.size() );

  SALOMEDS::Color color;
  color.R = (double)colors[currentColor].red()   / 255.0;
  color.G = (double)colors[currentColor].green() / 255.0;
  color.B = (double)colors[currentColor].blue()  / 255.0;

  currentColor = (currentColor+1) % colors.count();

  return color;
}
