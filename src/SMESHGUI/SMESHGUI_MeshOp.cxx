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
//  File   : SMESHGUI_MeshOp.cxx
//  Author : Sergey LITONIN, Open CASCADE S.A.S.

// SMESH includes
#include "SMESHGUI_MeshOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_MeshDlg.h"
#include "SMESHGUI_Operations.h"
#include "SMESHGUI_ShapeByMeshDlg.h"
#include "SMESHGUI_Utils.h"
#include "SMESH_NumberFilter.hxx"
#include "SMESH_TypeFilter.hxx"

// SALOME GEOM includes
#include <GEOM_SelectionFilter.h>
#include <GEOMBase.h>
#include <GeometryGUI.h>
#include <GEOM_wrap.hxx>
#include <GEOMImpl_Types.hxx>

// SALOME GUI includes
#include <SalomeApp_Tools.h>
#include <SalomeApp_Application.h>
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>
#include <SALOME_InteractiveObject.hxx>
#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDS_SComponent.hxx>
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDS_Study.hxx>
#include <SALOMEDS_wrap.hxx>

// Qt includes
#include <QStringList>
#include <QLineEdit>
#include <QApplication>

// OCCT includes
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Gen)

//================================================================================
/*!
 * \brief Constructor
  * \param theToCreate - if this parameter is true then operation is used for creation,
  * for editing otherwise
 *
 * Initialize operation
*/
//================================================================================
SMESHGUI_MeshOp::SMESHGUI_MeshOp( const bool theToCreate, const bool theIsMesh )
: SMESHGUI_SelectionOp(),
  myDlg( 0 ),
  myShapeByMeshOp( 0 ),
  myToCreate( theToCreate ),
  myIsMesh( theIsMesh ),
  myHypoSet( 0 )
{
  if ( GeometryGUI::GetGeomGen()->_is_nil() )// check that GEOM_Gen exists
    GeometryGUI::InitGeomGen();
  myIsOnGeometry = true;
  myMaxShapeDim = -1;
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================
SMESHGUI_MeshOp::~SMESHGUI_MeshOp()
{
  if ( myDlg )
    delete myDlg;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================
LightApp_Dialog* SMESHGUI_MeshOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief Creates or edits mesh
  * \retval bool - TRUE if operation is performed successfully, FALSE otherwise
 *
 * Virtual slot redefined from the base class called when "Apply" button is clicked
 * creates or edits mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::onApply()
{
  if (isStudyLocked())
    return false;

  QString aMess;
  if ( !isValid( aMess ) )
  {
    dlg()->show();
    if ( aMess != "" )
      SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), aMess );
    return false;
  }

  bool aResult = false;
  aMess = "";
  try
  {
    QStringList anEntryList;
    if ( myToCreate && myIsMesh )
      aResult = createMesh( aMess, anEntryList );
    if ( myToCreate && !myIsMesh )
      aResult = createSubMesh( aMess, anEntryList );
    else if ( !myToCreate )
      aResult = editMeshOrSubMesh( aMess );
    if ( aResult )
    {
      SMESHGUI::Modified();
      update( UF_ObjBrowser | UF_Model );
      if( LightApp_Application* anApp =
          dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
        myObjectToSelect = anApp->browseObjects( anEntryList, isApplyAndClose() );
    }
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
    aResult = false;
  }
  catch ( ... )
  {
    aResult = false;
  }

  if ( aResult )
  {
    if ( myToCreate )
      setDefaultName();
  }
  else
  {
    if ( aMess == "" )
      aMess = tr( "SMESH_OPERATION_FAILED" );
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ), aMess );
  }

  return aResult;
}

//================================================================================
/*!
 * \brief Creates dialog if necessary and shows it
 *
 * Virtual method redefined from base class called when operation is started creates
 * dialog if necessary and shows it, activates selection
 */
//================================================================================
void SMESHGUI_MeshOp::startOperation()
{
  myIgnoreAlgoSelection = false;

  if (!myDlg)
  {
    myDlg = new SMESHGUI_MeshDlg( myToCreate, myIsMesh );
    for ( int i = SMESH::DIM_0D; i <= SMESH::DIM_3D; i++ )
    {
      connect( myDlg->tab( i ), SIGNAL( createHyp( const int, const int ) ),
               this, SLOT( onCreateHyp( const int, const int ) ) );
      connect( myDlg->tab( i ), SIGNAL( editHyp( const int, const int ) ),
               this, SLOT( onEditHyp( const int, const int ) ) );
      connect( myDlg->tab( i ), SIGNAL( selectAlgo( const int ) ),
               this, SLOT( onAlgoSelected( const int ) ) );
    }
    connect( myDlg, SIGNAL( hypoSet( const QString& )), SLOT( onHypoSet( const QString& )));
    connect( myDlg, SIGNAL( geomSelectionByMesh( bool )), SLOT( onGeomSelectionByMesh( bool )));
    connect( myDlg, SIGNAL( selectMeshType( const int, const int ) ), SLOT( onAlgoSetByMeshType( const int, const int)));
    if ( myToCreate ) {
      if ( myIsMesh ) myHelpFileName = "constructing_meshes_page.html";
      else            myHelpFileName = "constructing_submeshes_page.html";
    }
    else {
      myHelpFileName = "editing_meshes_page.html";
    }
  }
  SMESHGUI_SelectionOp::startOperation();
  // iterate through dimensions and get available algorithms, set them to the dialog
  _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
  for ( int i = SMESH::DIM_0D; i <= SMESH::DIM_3D; i++ )
  {
    SMESHGUI_MeshTab* aTab = myDlg->tab( i );
    QStringList hypList;
    // clear available hypotheses
    aTab->setAvailableHyps( MainHyp, hypList );
    aTab->setAvailableHyps( AddHyp, hypList );
    aTab->setExistingHyps( MainHyp, hypList );
    aTab->setExistingHyps( AddHyp, hypList );
    myExistingHyps[ i ][ MainHyp ].clear();
    myExistingHyps[ i ][ AddHyp ].clear();
    // set algos
    availableHyps( i, Algo, hypList, myAvailableHypData[i][Algo] );
    aTab->setAvailableHyps( Algo, hypList );
  }
  if ( myToCreate )
  {
    setDefaultName();
    myDlg->activateObject( myIsMesh ? SMESHGUI_MeshDlg::Geom : SMESHGUI_MeshDlg::Mesh );
  }
  else
  {
    myDlg->activateObject( SMESHGUI_MeshDlg::Obj );
  }
  myDlg->setCurrentTab( SMESH::DIM_3D );

  QStringList TypeMeshList;
  createMeshTypeList( TypeMeshList );
  setAvailableMeshType( TypeMeshList );

  myDlg->show();
  myDlg->setGeomPopupEnabled(false);
  selectionDone();

  myHasConcurrentSubBefore = false;

  myObjectToSelect.clear();
}

//=================================================================================
/*!
 * \brief Selects a recently created mesh or sub-mesh if necessary
 *
 * Virtual method redefined from base class called when operation is commited
 * selects a recently created mesh or sub-mesh if necessary. Allows to perform
 * selection when the custom selection filters are removed.
 */
//=================================================================================
void SMESHGUI_MeshOp::commitOperation()
{
  SMESHGUI_SelectionOp::commitOperation();

  if ( !myObjectToSelect.isEmpty() ) {
    if ( LightApp_SelectionMgr* aSelectionMgr = selectionMgr() ) {
      SUIT_DataOwnerPtrList aList;
      aList.append( new LightApp_DataOwner( myObjectToSelect ) );
      aSelectionMgr->setSelected( aList );
    }
  }
}

//================================================================================
/*!
 * \brief Creates selection filter
  * \param theId - identifier of current selection widget
  * \retval SUIT_SelectionFilter* - pointer to the created filter or null
 *
 * Creates selection filter in accordance with identifier of current selection widget
 */
//================================================================================
SUIT_SelectionFilter* SMESHGUI_MeshOp::createFilter( const int theId ) const
{
  if ( theId == SMESHGUI_MeshDlg::Geom )
  {
//     TColStd_MapOfInteger allTypesMap;
//     for ( int i = 0; i < 10; i++ )
//       allTypesMap.Add( i );
//     return new SMESH_NumberFilter( "GEOM", TopAbs_SHAPE, 0, allTypesMap );
    return new GEOM_SelectionFilter( (SalomeApp_Study*)study(), true );
  }
  else if ( theId == SMESHGUI_MeshDlg::Obj && !myToCreate )
    return new SMESH_TypeFilter( SMESH::MESHorSUBMESH );
  else if ( theId == SMESHGUI_MeshDlg::Mesh )
    return new SMESH_TypeFilter( SMESH::MESH );
  else
    return 0;
}

//================================================================================
/*!
 * \brief check if selected shape is a sub-shape of the shape to mesh
  * \retval bool - check result
 */
//================================================================================
bool SMESHGUI_MeshOp::isSubshapeOk() const
{
  if ( !myToCreate || myIsMesh ) // not submesh creation
    return false;

  // mesh
  QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() );
  if (!pMesh) return false;

  SMESH::SMESH_Mesh_var mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );
  if (mesh->_is_nil()) return false;

  // main shape of the mesh
  GEOM::GEOM_Object_var mainGeom = mesh->GetShapeToMesh();
  if (mainGeom->_is_nil()) return false;

  // geometry
  QStringList aGEOMs;
  myDlg->selectedObject(SMESHGUI_MeshDlg::Geom, aGEOMs);

  if (aGEOMs.count() > 0) {
    GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (geomGen->_is_nil() || !aStudy) return false;

    GEOM::GEOM_IGroupOperations_wrap op = geomGen->GetIGroupOperations(aStudy->StudyId());
    if (op->_is_nil()) return false;

    // check all selected shapes
    QStringList::const_iterator aSubShapesIter = aGEOMs.begin();
    for ( ; aSubShapesIter != aGEOMs.end(); aSubShapesIter++) {
      QString aSubGeomEntry = (*aSubShapesIter);
      _PTR(SObject) pSubGeom = studyDS()->FindObjectID(aSubGeomEntry.toLatin1().data());
      if (!pSubGeom) return false;

      GEOM::GEOM_Object_var aSubGeomVar =
        GEOM::GEOM_Object::_narrow(_CAST(SObject,pSubGeom)->GetObject());
      if (aSubGeomVar->_is_nil()) return false;

      // skl for NPAL14695 - implementation of searching of mainObj
      GEOM::GEOM_Object_var mainObj = op->GetMainShape(aSubGeomVar); /* _var not _wrap as
                                                                        mainObj already exists! */
      while( !mainObj->_is_nil()) {
        CORBA::String_var entry1 = mainObj->GetEntry();
        CORBA::String_var entry2 = mainGeom->GetEntry();
        if (std::string( entry1.in() ) == entry2.in() )
          return true;
        mainObj = op->GetMainShape(mainObj);
      }
      if ( aSubGeomVar->GetShapeType() == GEOM::COMPOUND )
      {
        // is aSubGeomVar a compound of sub-shapes?
        GEOM::GEOM_IShapesOperations_wrap sop = geomGen->GetIShapesOperations(aStudy->StudyId());
        if (sop->_is_nil()) return false;
        GEOM::ListOfLong_var ids = sop->GetAllSubShapesIDs( aSubGeomVar,
                                                            GEOM::SHAPE,/*sorted=*/false);
        if ( ids->length() > 0 )
        {
          ids->length( 1 );
          GEOM::GEOM_Object_var compSub = geomGen->AddSubShape( aSubGeomVar, ids );
          if ( !compSub->_is_nil() )
          {
            GEOM::ListOfGO_var shared = sop->GetSharedShapes( mainGeom,
                                                              compSub,
                                                              compSub->GetShapeType() );
            geomGen->RemoveObject( compSub );
            compSub->UnRegister();
            if ( shared->length() > 0 ) {
              geomGen->RemoveObject( shared[0] );
              shared[0]->UnRegister();
            }
            return ( shared->length() > 0 );
          }
        }
      }
    }
  }

  return false;
}

//================================================================================
/*!
 * \brief Return name of the algorithm that does not support sub-meshes and makes
 * sub-mesh creation useless
 *  \retval char* - string is to be deleted!!!
 */
//================================================================================
char* SMESHGUI_MeshOp::isSubmeshIgnored() const
{
  if ( myToCreate && !myIsMesh ) {

    QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
    QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
    _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() );
    if ( pMesh ) {

      QStringList algoNames;
      THypList    algoList;
      existingHyps(3, Algo, pMesh, algoNames, algoList);
      if (!algoList.empty()) {
        HypothesisData* algo =
          SMESH::GetHypothesisData( SMESH::toQStr( algoList[0].first->GetName() ));
        if ( algo &&
             algo->InputTypes.empty() && // builds all dimensions it-self
             !algo->IsSupportSubmeshes )
          return CORBA::string_dup( algoNames[0].toLatin1().data() );
      }

//       GEOM::GEOM_Object_var geom;
//       if (_PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.toLatin1().data() ))
//         geom = SMESH::SObjectToInterface<GEOM::GEOM_Object>( pGeom );

//       if ( !geom->_is_nil() && geom->GetShapeType() >= GEOM::FACE ) { // WIRE, EDGE as well
        existingHyps(2, Algo, pMesh, algoNames, algoList);
        if (!algoList.empty()) {
          HypothesisData* algo =
            SMESH::GetHypothesisData( SMESH::toQStr( algoList[0].first->GetName() ));
          if ( algo &&
               algo->InputTypes.empty() && // builds all dimensions it-self
               !algo->IsSupportSubmeshes )
            return CORBA::string_dup( algoNames[0].toLatin1().data() );
        }
//       }
    }
  }
  return 0;
}

//================================================================================
/*!
 * \brief find an existing submesh by the selected shape
 * \retval _PTR(SObject) - the found submesh SObject
 */
//================================================================================
_PTR(SObject) SMESHGUI_MeshOp::getSubmeshByGeom() const
{
  QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
  _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() );
  _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.toLatin1().data() );
  if ( pMesh && pGeom ) {
    GEOM::GEOM_Object_var geom = SMESH::SObjectToInterface<GEOM::GEOM_Object>( pGeom );
    if ( !geom->_is_nil() ) {
      int tag = -1;
      switch ( geom->GetShapeType() ) {
      case GEOM::VERTEX:   tag = SMESH::Tag_SubMeshOnVertex;   break;
      case GEOM::EDGE:     tag = SMESH::Tag_SubMeshOnEdge;     break;
      case GEOM::WIRE:     tag = SMESH::Tag_SubMeshOnWire;     break;
      case GEOM::FACE:     tag = SMESH::Tag_SubMeshOnFace;     break;
      case GEOM::SHELL:    tag = SMESH::Tag_SubMeshOnShell;    break;
      case GEOM::SOLID:    tag = SMESH::Tag_SubMeshOnSolid;    break;
      case GEOM::COMPOUND: tag = SMESH::Tag_SubMeshOnCompound; break;
      default:;
      }
      _PTR(GenericAttribute) anAttr;
      _PTR(SObject) aSubmeshRoot;
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
      if ( pMesh->FindSubObject( tag, aSubmeshRoot ) )
      {
        _PTR(ChildIterator) smIter = aStudy->NewChildIterator( aSubmeshRoot );
        for ( ; smIter->More(); smIter->Next() )
        {
          _PTR(SObject) aSmObj = smIter->Value();
          if ( ! aSmObj->FindAttribute( anAttr, "AttributeIOR" ))
            continue;
          _PTR(ChildIterator) anIter1 = aStudy->NewChildIterator(aSmObj);
          for ( ; anIter1->More(); anIter1->Next()) {
            _PTR(SObject) pGeom2 = anIter1->Value();
            if ( pGeom2->ReferencedObject( pGeom2 ) &&
                 pGeom2->GetID() == pGeom->GetID() )
              return aSmObj;
          }
        }
      }
    }
  }
  return _PTR(SObject)();
}

//================================================================================
/*!
 * \brief Updates dialog's look and feel
 *
 * Virtual method redefined from the base class updates dialog's look and feel
 */
//================================================================================
void SMESHGUI_MeshOp::selectionDone()
{
  if (!dlg()->isVisible() || !myDlg->isEnabled())
    return;

  SMESHGUI_SelectionOp::selectionDone();

  try
  {
    myIsOnGeometry = true;

    //Check geometry for mesh
    QString anObjEntry = myDlg->selectedObject(SMESHGUI_MeshDlg::Obj);
    _PTR(SObject) pObj = studyDS()->FindObjectID(anObjEntry.toLatin1().data());
    if (pObj)
    {
      SMESH::SMESH_Mesh_var aMeshVar =
        SMESH::SMESH_Mesh::_narrow(_CAST(SObject,pObj)->GetObject());
      if (!aMeshVar->_is_nil()) {
        if (!myToCreate && !aMeshVar->HasShapeToMesh())
          myIsOnGeometry = false;
      }
    }

    if (myIsOnGeometry)
    {
      // Enable tabs according to shape dimension

      int shapeDim = 3;

      QStringList aGEOMs;
      myDlg->selectedObject(SMESHGUI_MeshDlg::Geom, aGEOMs);
      GEOM::ListOfGO_var aSeq = new GEOM::ListOfGO;

      if (aGEOMs.count() > 0) {
        // one or more GEOM shape selected
        aSeq->length(aGEOMs.count());
        QStringList::const_iterator aSubShapesIter = aGEOMs.begin();
        int iSubSh = 0;
        for ( ; aSubShapesIter != aGEOMs.end(); aSubShapesIter++, iSubSh++) {
          QString aSubGeomEntry = (*aSubShapesIter);
          _PTR(SObject) pSubGeom = studyDS()->FindObjectID(aSubGeomEntry.toLatin1().data());
         
          if( pSubGeom ) { 
            SALOMEDS_SObject* sobj = _CAST(SObject,pSubGeom);
            if( sobj ) {
              GEOM::GEOM_Object_var aSubGeomVar =
                GEOM::GEOM_Object::_narrow(sobj->GetObject());
              if( !aSubGeomVar->_is_nil() ){
                aSeq[iSubSh] = aSubGeomVar;
              }
            }
          }
        }
      } else {
        // get geometry by selected sub-mesh
        QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
        _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
        GEOM::GEOM_Object_var aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pObj );
        if (!aGeomVar->_is_nil()) {
          aSeq->length(1);
          aSeq[0] = aGeomVar;
        }
      }

      if (aSeq->length() > 0) {
        shapeDim = -1;
        for ( CORBA::ULong iss = 0; iss < aSeq->length() && shapeDim < 3; iss++) {
          GEOM::GEOM_Object_var aGeomVar = aSeq[iss];
          switch ( aGeomVar->GetShapeType() ) {
          case GEOM::SOLID:  shapeDim = 3; break;
          case GEOM::SHELL:
            // Bug 0016155: EDF PAL 447: If the shape is a Shell, disable 3D tab
            // {
            //   TopoDS_Shape aShape;
            //   bool isClosed = GEOMBase::GetShape(aGeomVar, aShape) && /*aShape.Closed()*/BRep_Tool::IsClosed(aShape);
            //   shapeDim = qMax(isClosed ? 3 : 2, shapeDim);
            // }
            // break;
          case GEOM::FACE:   shapeDim = qMax(2, shapeDim); break;
          case GEOM::WIRE:
          case GEOM::EDGE:   shapeDim = qMax(1, shapeDim); break;
          case GEOM::VERTEX: shapeDim = qMax(0, shapeDim); break;
          default:
            {
              TopoDS_Shape aShape;
              if (GEOMBase::GetShape(aGeomVar, aShape))
              {
                TopExp_Explorer exp (aShape, TopAbs_SOLID);
                if (exp.More()) {
                  shapeDim = 3;
                }
                // Bug 0016155: EDF PAL 447: If the shape is a Shell, disable 3D tab
                // else if ( exp.Init( aShape, TopAbs_SHELL ), exp.More() )
                // {
                //   shapeDim = 2;
                //   for (; exp.More() && shapeDim == 2; exp.Next()) {
                //     if (/*exp.Current().Closed()*/BRep_Tool::IsClosed(exp.Current()))
                //       shapeDim = 3;
                //   }
                // }
                else if ( exp.Init( aShape, TopAbs_FACE ), exp.More() )
                  shapeDim = qMax(2, shapeDim);
                else if ( exp.Init( aShape, TopAbs_EDGE ), exp.More() )
                  shapeDim = qMax(1, shapeDim);
                else if ( exp.Init( aShape, TopAbs_VERTEX ), exp.More() )
                  shapeDim = qMax(0, shapeDim);
              }
            }
          }
          if ( shapeDim == 3 )
            break;
        }
      }
      for (int i = SMESH::DIM_3D; i > shapeDim; i--) {
        // reset algos before disabling tabs (0020138)
        onAlgoSelected(-1, i);
      }
      myDlg->setMaxHypoDim( shapeDim );
      myMaxShapeDim = shapeDim;
      myDlg->setHypoSets( SMESH::GetHypothesesSets( shapeDim ));

      if (!myToCreate) // edition: read hypotheses
      {
        if (pObj != 0)
        {
          SMESH::SMESH_subMesh_var submeshVar =
            SMESH::SMESH_subMesh::_narrow( _CAST( SObject,pObj )->GetObject() );
          myIsMesh = submeshVar->_is_nil();
          myDlg->setTitile( myToCreate, myIsMesh );
          myDlg->setObjectShown( SMESHGUI_MeshDlg::Mesh, !submeshVar->_is_nil() );
          myDlg->setObjectShown( SMESHGUI_MeshDlg::Geom, true );
          myDlg->objectWg( SMESHGUI_MeshDlg::Mesh, SMESHGUI_MeshDlg::Btn )->hide();
          myDlg->objectWg( SMESHGUI_MeshDlg::Geom, SMESHGUI_MeshDlg::Btn )->hide();
          myDlg->updateGeometry();
          myDlg->adjustSize();
          readMesh();
        }
        else
          myDlg->reset();
      }
      else if ( !myIsMesh ) // submesh creation
      {
        // if a submesh on the selected shape already exist, pass to submesh edition mode
        if ( _PTR(SObject) pSubmesh = getSubmeshByGeom() ) {
          SMESH::SMESH_subMesh_var sm =
            SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( pSubmesh );
          bool editSubmesh = ( !sm->_is_nil() &&
                               SUIT_MessageBox::question( myDlg, tr( "SMESH_WARNING" ),
                                                          tr( "EDIT_SUBMESH_QUESTION"),
                                                          SUIT_MessageBox::Yes |
                                                          SUIT_MessageBox::No,
                                                          SUIT_MessageBox::No )
                               == SUIT_MessageBox::Yes );
          if ( editSubmesh )
          {
            selectionMgr()->clearFilters();
            selectObject( pSubmesh );
            SMESHGUI::GetSMESHGUI()->switchToOperation( SMESHOp::OpEditMeshOrSubMesh );
            return;
          }
          else
          {
            myDlg->selectObject( "", SMESHGUI_MeshDlg::Geom, "" );
            selectObject( _PTR(SObject)() );
            selectionDone();
            return;
          }
        }
        // discard selected mesh if submesh creation not allowed because of
        // a global algorithm that does not support submeshes
        if ( char* algoName = isSubmeshIgnored() ) {
          SUIT_MessageBox::warning( myDlg, tr( "SMESH_ERROR" ),
                                    tr("SUBMESH_NOT_ALLOWED").arg(algoName));
          CORBA::string_free( algoName );
          myDlg->selectObject( "", SMESHGUI_MeshDlg::Mesh, "" );
          selectObject( _PTR(SObject)() );
          selectionDone();
          return;
        }

        // enable/disable popup for choice of geom selection way
        bool enable = false;
        QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
        if ( _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() )) {
          SMESH::SMESH_Mesh_var mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );
          if ( !mesh->_is_nil() ) {
            //rnv: issue 21056: EDF 1608 SMESH: Dialog Box "Create Sub Mesh": focus should automatically switch to geometry
            QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
            _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.toLatin1().data() );
            if ( !pGeom || GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() )->_is_nil() )
              myDlg->activateObject(SMESHGUI_MeshDlg::Geom);
            enable = ( shapeDim > 1 ) && ( mesh->NbEdges() > 0 );
          }
        }
        myDlg->setGeomPopupEnabled( enable );
      }
    }
    else { // no geometry defined
      myDlg->enableTab( SMESH::DIM_3D );
      QStringList hypList;
      availableHyps( SMESH::DIM_3D, Algo, hypList,
                     myAvailableHypData[SMESH::DIM_3D][Algo]);

      SMESHGUI_MeshTab* aTab = myDlg->tab( SMESH::DIM_3D );
      aTab->setAvailableHyps( Algo, hypList );
      for (int i = SMESH::DIM_0D;i < SMESH::DIM_3D; ++i) {
        myDlg->disableTab(i);
      }
      myMaxShapeDim = -1;
      //Hide labels and fields (Mesh and Geometry)
      myDlg->setObjectShown( SMESHGUI_MeshDlg::Mesh, false );
      myDlg->setObjectShown( SMESHGUI_MeshDlg::Geom, false );
      myDlg->adjustSize();
      readMesh();
    }
    int curIndex = myDlg->currentMeshType( );
    QStringList TypeMeshList;
    createMeshTypeList( TypeMeshList );
    setAvailableMeshType( TypeMeshList );
    curIndex =( curIndex >= TypeMeshList.count() ) ? 0 : curIndex;
    myDlg->setCurrentMeshType( curIndex );
    setFilteredAlgoData( myMaxShapeDim, curIndex);
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... )
  {
  }
}

//================================================================================
/*!
 * \brief Verifies validity of input data
  * \param theMess - Output parameter intended for returning error message
  * \retval bool  - TRUE if input data is valid, false otherwise
 *
 * Verifies validity of input data. This method is called when "Apply" or "OK" button
 * is pressed before mesh creation or editing.
 */
//================================================================================
bool SMESHGUI_MeshOp::isValid( QString& theMess ) const
{
  // Selected object to be  edited
  if ( !myToCreate && myDlg->selectedObject( SMESHGUI_MeshDlg::Obj ) == "" )
  {
    theMess = tr( "THERE_IS_NO_OBJECT_FOR_EDITING" );
    return false;
  }

  // Name
  QString aMeshName = myDlg->objectText( SMESHGUI_MeshDlg::Obj ).trimmed();
  if ( aMeshName.isEmpty() )
  {
    theMess = myIsMesh ? tr( "NAME_OF_MESH_IS_EMPTY" ) : tr( "NAME_OF_SUBMESH_IS_EMPTY" );
    return false;
  }

/*  // Imported mesh, if create sub-mesh or edit mesh
  if ( !myToCreate || ( myToCreate && !myIsMesh ))
  {
    QString aMeshEntry = myDlg->selectedObject
      ( myToCreate ? SMESHGUI_MeshDlg::Mesh : SMESHGUI_MeshDlg::Obj );
    if ( _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() )) {
      SMESH::SMESH_Mesh_var mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );
      if ( !mesh->_is_nil() && CORBA::is_nil( mesh->GetShapeToMesh() )) {
        theMess = tr( "IMPORTED_MESH" );
        return false;
      }
    }
  }*/

  // Geom
  if ( myToCreate )
  {
    QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
    if ( aGeomEntry.isEmpty() )
    {
      theMess = tr( myIsMesh ?
                    "GEOMETRY_OBJECT_IS_NOT_DEFINED_MESH" :
                    "GEOMETRY_OBJECT_IS_NOT_DEFINED_SUBMESH");
      if ( !myIsMesh )
        return false;
      dlg()->show();
      if ( SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), theMess,
           SUIT_MessageBox::Yes, SUIT_MessageBox::No ) == SUIT_MessageBox::No )
      {
        theMess = "";
        return false;
      }
      return true;
    }
    _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.toLatin1().data() );
    if ( !pGeom || GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() )->_is_nil() )
    {
      theMess = tr( "GEOMETRY_OBJECT_IS_NULL" );
      return false;
    }

    // Mesh
    if ( !myIsMesh ) // i.e sub-mesh creation,
    {
      QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
      if ( aMeshEntry == "" )
      {
        theMess = tr( "MESH_IS_NOT_DEFINED" );
        return false;
      }
      _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() );
      if ( !pMesh || SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() )->_is_nil() )
      {
        theMess = tr( "MESH_IS_NULL" );
        return false;
      }
      if ( !isSubshapeOk() )
      {
        theMess = tr( "INVALID_SUBSHAPE" );
        return false;
      }
    }
  }

  return true;
}

//================================================================================
/*!
 * \brief check compatibility of the algorithm and another algorithm or hypothesis
  * \param theAlgoData - algorithm data
  * \param theHypData - hypothesis data
  * \param theHypType - hypothesis type
  * \param theHypTypeName - hypothesis type name, must be provided if 2-nd arg is not algo
  * \retval bool - check result
 */
//================================================================================
static bool isCompatible(const HypothesisData* theAlgoData,
                         const HypothesisData* theHypData,
                         const int             theHypType)
{
  if ( !theAlgoData )
    return true;

  if ( theHypType == SMESHGUI_MeshOp::Algo )
    return SMESH::IsCompatibleAlgorithm( theAlgoData, theHypData );

  bool isOptional;
  return ( SMESH::IsAvailableHypothesis( theAlgoData, theHypData->TypeName, isOptional ));
}

//================================================================================
/*!
 * \brief check compatibility of the geometry
  * \param theAlgoData - to select hypos able to be used by this algo
  * \param theCurrentGeomToSelect - the current name of the selected geometry
  * \param theGeomVar - currently selected geometry
  * \retval bool - check result
 */
//================================================================================
bool SMESHGUI_MeshOp::isCompatibleToGeometry(HypothesisData* theAlgoData,
                                             QString theCurrentGeomToSelect,
                                             GEOM::GEOM_Object_var theGeomVar)
{
  if ( theGeomVar->_is_nil() )
    return true;

  bool isApplicable = false;
  if ( theCurrentGeomToSelect == myLastGeomToSelect && !theCurrentGeomToSelect.isEmpty() ) {
    THypLabelIsAppMap::const_iterator iter = myHypMapIsApplicable.find( theAlgoData->Label );
    if ( iter != myHypMapIsApplicable.end() && iter.key() == theAlgoData->Label ) {
      isApplicable = iter.value();
      return isApplicable;
    }
  }
  bool toCheckIsApplicableToAll = !myIsMesh;
  if ( toCheckIsApplicableToAll )
    toCheckIsApplicableToAll = ( theGeomVar->GetType() == GEOM_GROUP );
  isApplicable = SMESH::IsApplicable( theAlgoData->TypeName, theGeomVar, toCheckIsApplicableToAll );
  myHypMapIsApplicable.insert( theAlgoData->Label, isApplicable );
  return isApplicable;
}

//================================================================================
/*!
 * \brief check compatibility of the mesh type
  * \param theAlgoData - to select hypos able to be used by this algo
  * \param theMeshType - type of mesh for filtering algorithms
  * \retval bool - check result
 */
//================================================================================
bool SMESHGUI_MeshOp::isCompatibleToMeshType(HypothesisData* theAlgoData,
                                             QString theMeshType)
{
  bool isAvailableAlgo = ( theAlgoData->OutputTypes.count() == 0 );
  QStringList::const_iterator inElemType = theAlgoData->OutputTypes.begin();
  for ( ; inElemType != theAlgoData->OutputTypes.end(); inElemType++ ) {
    if ( *inElemType == theMeshType ) {
      isAvailableAlgo = true;
      break;
    }
  }
  return isAvailableAlgo;
}

//================================================================================
/*!
 * \brief Gets available hypotheses or algorithms
  * \param theDim - specifies dimension of returned hypotheses/algorifms
  * \param theHypType - specifies whether algorims or hypotheses or additional ones
  * are retrieved (possible values are in HypType enumeration)
  * \param theHyps - Output list of hypotheses' names
  * \param thePrevAlgoData - to select hypos able to be used by previously algo (optional)
  * \param theNextAlgoData - to select hypos able to be used by next algo (optional)
  * \param theMeshType - type of mesh for filtering algorithms (optional)
 *
 * Gets available hypotheses or algorithm in accordance with input parameters
 */
//================================================================================
void SMESHGUI_MeshOp::availableHyps( const int       theDim,
                                     const int       theHypType,
                                     QStringList&    theHyps,
                                     THypDataList&   theDataList,
                                     HypothesisData* thePrevAlgoData,
                                     HypothesisData* theNextAlgoData,
                                     const QString&  theMeshType)
{
  theDataList.clear();
  theHyps.clear();
  bool isAlgo = ( theHypType == Algo );
  bool isAux  = ( theHypType >= AddHyp );
  QStringList aHypTypeNameList = SMESH::GetAvailableHypotheses( isAlgo, theDim, isAux, myIsOnGeometry, !myIsMesh );

  GEOM::GEOM_Object_var aGeomVar;
  QString aCurrentGeomToSelect;
  if ( !theMeshType.isEmpty() ) {
    aCurrentGeomToSelect = myDlg->selectedObject( myToCreate ? SMESHGUI_MeshDlg::Geom : SMESHGUI_MeshDlg::Obj );
    if ( _PTR(SObject) so = studyDS()->FindObjectID( aCurrentGeomToSelect.toLatin1().data() )) {
      aGeomVar = SMESH::GetGeom( so );
    }
   if ( aCurrentGeomToSelect != myLastGeomToSelect )
     myHypMapIsApplicable.clear();
  }

  std::multimap< double, HypothesisData* > sortedHyps;
  QStringList::const_iterator anIter;
  for ( anIter = aHypTypeNameList.begin(); anIter != aHypTypeNameList.end(); ++anIter )
  {
    HypothesisData* aData = SMESH::GetHypothesisData( *anIter );
    if ( ( isCompatible ( thePrevAlgoData, aData, theHypType ) &&
           isCompatible ( theNextAlgoData, aData, theHypType ) ) ||
           ( theMeshType == "ANY" && aData->InputTypes.isEmpty()))
    {
      if ( ( !theMeshType.isEmpty() )  &&
           ( theDim >= SMESH::DIM_2D ) &&
           ( ( theMeshType != "ANY" && !isCompatibleToMeshType( aData, theMeshType )) ||
             !isCompatibleToGeometry( aData, aCurrentGeomToSelect, aGeomVar )))
        continue;

      int  groupID = aData->GroupID;
      int priority = aData->Priority;
      if ( groupID  < 0 || groupID > 9    ) groupID  = 9;
      if ( priority < 0 || priority > 999 ) priority = 999;

      sortedHyps.insert( std::make_pair( groupID + priority * 1e-3, aData ));
    }
  }

  if ( !sortedHyps.empty() )
  {
    HypothesisData* aPrevGroup = SMESH::GetGroupTitle( sortedHyps.rbegin()->second, isAlgo );
    std::multimap< double, HypothesisData* >::iterator key_hyp = sortedHyps.begin();
    for ( ; key_hyp != sortedHyps.end(); ++key_hyp )
    {
      HypothesisData*  aData = key_hyp->second;
      HypothesisData* aGroup = SMESH::GetGroupTitle( aData, isAlgo );
      if ( aPrevGroup != aGroup )
      {
        theDataList.append( aGroup );
        theHyps.append( aGroup->Label );
        aPrevGroup = aGroup;
      }
      theDataList.append( aData );
      theHyps.append( aData->Label );
    }
  }

  if ( !theMeshType.isEmpty() && !aCurrentGeomToSelect.isEmpty() )
    myLastGeomToSelect = aCurrentGeomToSelect;
}

//================================================================================
/*!
 * \brief Gets existing hypotheses or algorithms
 *  \param theDim - specifies dimension of returned hypotheses/algorifms
 *  \param theHypType - specifies whether algorims or hypotheses or additional ones
 *  are retrieved (possible values are in HypType enumeration)
 *  \param theFather - start object for finding ( may be component, mesh, or sub-mesh )
 *  \param theHyps - output list of names.
 *  \param theHypVars - output list of variables.
 *  \param theAlgoData - to select hypos able to be used by this algo (optional)
 *
 * Gets existing (i.e. already created) hypotheses or algorithm in accordance with
 * input parameters.
 *
 * WARNING: when using this method to get hyps existing in Mesh component,
 *          call availableHyps() before in order to get only hyps of available types
 *          that was filtered by availableHyps()
 */
//================================================================================
void SMESHGUI_MeshOp::existingHyps( const int       theDim,
                                    const int       theHypType,
                                    _PTR(SObject)   theFather,
                                    QStringList&    theHyps,
                                    THypList&       theHypList,
                                    HypothesisData* theAlgoData) const
{
  // Clear hypoheses list
  theHyps.clear();
  theHypList.clear();

  if ( !theFather )
    return;

  _PTR(SObject)          aHypRoot;
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName)    aName;
  _PTR(AttributeIOR)     anIOR;

  const bool isMesh = !_CAST( SComponent, theFather );
  int aPart = -1;
  if ( isMesh )
    aPart = theHypType == Algo ? SMESH::Tag_RefOnAppliedAlgorithms : SMESH::Tag_RefOnAppliedHypothesis;
  else
    aPart = theHypType == Algo ? SMESH::Tag_AlgorithmsRoot : SMESH::Tag_HypothesisRoot;

  const bool isAux   = ( theHypType >= AddHyp );
  const bool allHyps = ( !isMesh && theHypType != Algo && theDim > -1);

  if ( theFather->FindSubObject( aPart, aHypRoot ) )
  {
    _PTR(ChildIterator) anIter =
      SMESH::GetActiveStudyDocument()->NewChildIterator( aHypRoot );
    for ( ; anIter->More(); anIter->Next() )
    {
      _PTR(SObject) anObj = anIter->Value();
      if ( isMesh ) // i.e. mesh or submesh
      {
        _PTR(SObject) aRefObj;
        if ( anObj->ReferencedObject( aRefObj ) )
          anObj = aRefObj;
        else
          continue;
      }
      if ( anObj->FindAttribute( anAttr, "AttributeName" ) )
      {
        aName = anAttr;
        CORBA::Object_var aVar = _CAST(SObject,anObj)->GetObject();
        if ( !CORBA::is_nil( aVar ) )
        {
          SMESH::SMESH_Hypothesis_var aHypVar = SMESH::SMESH_Hypothesis::_narrow( aVar );
          if ( !aHypVar->_is_nil() )
          {
            CORBA::String_var hypType = aHypVar->GetName();
            HypothesisData* aData = SMESH::GetHypothesisData( hypType.in() );
            if ( !aData) continue;
            if (( theDim == -1 || aData->Dim.contains( theDim ) ) &&
                ( isCompatible ( theAlgoData, aData, theHypType )) &&
                ( theHypType == Algo || isAux == aData->IsAuxOrNeedHyp ) &&
                ( !allHyps || myAvailableHypData[theDim][theHypType].count(aData) ))
            {
              std::string aHypName = aName->Value();
              theHyps.append( aHypName.c_str() );
              theHypList.append( THypItem( aHypVar, aHypName.c_str() ) );
            }
          }
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief If create or edit a submesh, return a hypothesis holding parameters used
 *        to mesh a sub-shape
  * \param aHypType - The hypothesis type name
  * \param aServerLib - Server library name
  * \param hypData - The structure holding the hypothesis type etc.
  * \retval SMESH::SMESH_Hypothesis_var - the hypothesis holding parameter values
 */
//================================================================================
SMESH::SMESH_Hypothesis_var
SMESHGUI_MeshOp::getInitParamsHypothesis( const QString& aHypType,
                                          const QString& aServerLib ) const
{
  if ( aHypType.isEmpty() || aServerLib.isEmpty() )
    return SMESH::SMESH_Hypothesis::_nil();

  const int nbColonsInMeshEntry = 3;
  bool isSubMesh = myToCreate ?
    !myIsMesh :
    myDlg->selectedObject( SMESHGUI_MeshDlg::Obj ).count(':') > nbColonsInMeshEntry;

  // get mesh and geom object
  SMESH::SMESH_Mesh_var aMeshVar = SMESH::SMESH_Mesh::_nil();
  GEOM::GEOM_Object_var aGeomVar = GEOM::GEOM_Object::_nil();

  QString anEntry;
  if ( isSubMesh )
  {
    anEntry = myDlg->selectedObject
      ( myToCreate ? SMESHGUI_MeshDlg::Mesh : SMESHGUI_MeshDlg::Obj );
    if ( _PTR(SObject) pObj = studyDS()->FindObjectID( anEntry.toLatin1().data() ))
    {
      CORBA::Object_ptr Obj = _CAST( SObject,pObj )->GetObject();
      if ( myToCreate ) // mesh and geom may be selected
      {
        aMeshVar = SMESH::SMESH_Mesh::_narrow( Obj );
        anEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
        if ( _PTR(SObject) pGeom = studyDS()->FindObjectID( anEntry.toLatin1().data() ))
          aGeomVar= GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
      }
      else // edition: sub-mesh may be selected
      {
        SMESH::SMESH_subMesh_var sm = SMESH::SMESH_subMesh::_narrow( Obj );
        if ( !sm->_is_nil() ) {
          aMeshVar = sm->GetFather();
          aGeomVar = sm->GetSubShape();
        }
      }
    }
  }
  else // mesh
  {
    if ( !myToCreate ) // mesh to edit can be selected
    {
      anEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
      if ( _PTR(SObject) pMesh = studyDS()->FindObjectID( anEntry.toLatin1().data() ))
      {
        aMeshVar = SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() );
        if ( !aMeshVar->_is_nil() )
          aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pMesh );
      }
    }
    if ( aGeomVar->_is_nil() ) {
      anEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
      if ( _PTR(SObject) pGeom = studyDS()->FindObjectID( anEntry.toLatin1().data() ))
      {
        aGeomVar= GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
      }
    }
  }

  SMESH::SMESH_Hypothesis_var hyp =
    SMESHGUI::GetSMESHGen()->GetHypothesisParameterValues( aHypType.toLatin1().data(),
                                                           aServerLib.toLatin1().data(),
                                                           aMeshVar,
                                                           aGeomVar,
                                                           /*byMesh = */isSubMesh);
  if ( hyp->_is_nil() && isSubMesh )
    hyp = SMESHGUI::GetSMESHGen()->GetHypothesisParameterValues( aHypType.toLatin1().data(),
                                                                 aServerLib.toLatin1().data(),
                                                                 aMeshVar,
                                                                 aGeomVar,
                                                                 /*byMesh = */false);
  return hyp;
}

//================================================================================
/*!
 * \brief initialize a hypothesis creator
 */
//================================================================================

void SMESHGUI_MeshOp::initHypCreator( SMESHGUI_GenericHypothesisCreator* theCreator )
{
  if ( !theCreator ) return;

  // Set shapes, of mesh and sub-mesh if any

  // get Entry of the Geom object
  QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
  QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj ); 

  if ( myToCreate && myIsMesh )
    aMeshEntry = aGeomEntry;

  if ( aMeshEntry != aGeomEntry ) { // Get Geom object from Mesh of a sub-mesh being edited
    _PTR(SObject) pObj = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() );
    GEOM::GEOM_Object_var aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pObj );
    aMeshEntry = ( aGeomVar->_is_nil() ) ? QString() : SMESH::toQStr( aGeomVar->GetStudyEntry() );
  }

  if ( aMeshEntry == "" && aGeomEntry == "" ) { // get geom of an object being edited
    _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
    bool isMesh;
    GEOM::GEOM_Object_var aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pObj, &isMesh );
    if ( !aGeomVar->_is_nil() )
    {
      aGeomEntry = SMESH::toQStr( aGeomVar->GetStudyEntry() );
      if ( isMesh )
        aMeshEntry = aGeomEntry;
    }
  }

  if ( anObjEntry != "" && aGeomEntry != "" && aMeshEntry == "" ) {
    // take geometry from submesh being created
    _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
    if ( pObj ) {
      // if current object is sub-mesh
      SMESH::SMESH_subMesh_var aSubMeshVar =
        SMESH::SMESH_subMesh::_narrow( _CAST( SObject,pObj )->GetObject() );
      if ( !aSubMeshVar->_is_nil() ) {
        SMESH::SMESH_Mesh_var aMeshVar =  aSubMeshVar->GetFather();
        if ( !aMeshVar->_is_nil() ) {
          _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshVar );
          GEOM::GEOM_Object_var aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( aMeshSO );
          if ( !aGeomVar->_is_nil() )
            aMeshEntry = SMESH::toQStr( aGeomVar->GetStudyEntry() );
        }
      }
    }
  }

  theCreator->setShapeEntry( aGeomEntry );
  if ( aMeshEntry != "" )
    theCreator->setMainShapeEntry( aMeshEntry );

  theCreator->setNoGeomMesh( !myIsOnGeometry && myIsMesh && !myToCreate );
}

//================================================================================
/*!
 * \Brief Returns tab dimention
  * \param tab - the tab in the dlg
  * \param dlg - my dialogue
  * \retval int - dimention
 */
//================================================================================
static int getTabDim (const QObject* tab, SMESHGUI_MeshDlg* dlg )
{
  int aDim = -1;
  for (int i = SMESH::DIM_0D; i <= SMESH::DIM_3D; i++)
    if (tab == dlg->tab(i))
      aDim = i;
  return aDim;
}

//================================================================================
/*!
 * \brief Create hypothesis
  * \param theHypType - hypothesis category (main or additional)
  * \param theIndex - index of type of hypothesis to be cerated
 *
 * Specifies dimension of hypothesis to be created (using sender() method),
 * specifies its type and calls method for hypothesis creation
 */
//================================================================================
void SMESHGUI_MeshOp::onCreateHyp( const int theHypType, const int theIndex )
{
  // Specifies dimension of hypothesis to be created
  int aDim = getTabDim( sender(), myDlg );
  if (aDim == -1)
    return;

  // Specifies type of hypothesis to be created
  THypDataList& dataList = myAvailableHypData[ aDim ][ theHypType ];
  if (theIndex < 0 || theIndex >= dataList.count())
    return;
  QString aHypTypeName = dataList[ theIndex ]->TypeName;

  // Create hypothesis
  createHypothesis(aDim, theHypType, aHypTypeName);
}

namespace
{
  QString GetUniqueName (const QStringList& theHypNames,
                         const QString& theName,
                         size_t theIteration = 1)
  {
    QString aName = theName + "_" + QString::number( theIteration );
    if ( theHypNames.contains( aName ) )
      return GetUniqueName( theHypNames, theName, ++theIteration );
    return aName;
  }
}

//================================================================================
/*!
 *  Create hypothesis and update dialog.
 *  \param theDim - dimension of hypothesis to be created
 *  \param theType - hypothesis category (algorithm, hypothesis, additional hypothesis)
 *  \param theTypeName - specifies hypothesis to be created
 */
//================================================================================
void SMESHGUI_MeshOp::createHypothesis(const int theDim,
                                       const int theType,
                                       const QString& theTypeName)
{
  HypothesisData* aData = SMESH::GetHypothesisData(theTypeName);
  if (!aData)
    return;

  myDim = theDim;
  myType = theType;

  // get a unique hyp name
  QStringList aHypNames;
  TDim2Type2HypList::const_iterator aDimIter = myExistingHyps.begin();
  for ( ; aDimIter != myExistingHyps.end(); aDimIter++) {
    const TType2HypList& aType2HypList = aDimIter.value();
    TType2HypList::const_iterator aTypeIter = aType2HypList.begin();
    for ( ; aTypeIter != aType2HypList.end(); aTypeIter++) {
      const THypList& aHypList = aTypeIter.value();
      THypList::const_iterator anIter = aHypList.begin();
      for ( ; anIter != aHypList.end(); anIter++) {
        const THypItem& aHypItem = *anIter;
        const QString& aHypName = aHypItem.second;
        aHypNames.append(aHypName);
      }
    }
  }
  QString aHypName = GetUniqueName( aHypNames, aData->Label);

  // existing hypos
  bool dialog = false;

  QString aClientLibName = aData->ClientLibName;
  if (aClientLibName == "") {
    // Call hypothesis creation server method (without GUI)
    SMESH::SMESH_Hypothesis_var aHyp =
      SMESH::CreateHypothesis(theTypeName, aHypName, false);
    aHyp.out();
  }
  else {
    // Get hypotheses creator client (GUI)
    SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator(theTypeName);

    // Create hypothesis
    if (aCreator)
    {
      // Get parameters appropriate to initialize a new hypothesis
      SMESH::SMESH_Hypothesis_var initParamHyp =
        getInitParamsHypothesis(theTypeName, aData->ServerLibName);

      removeCustomFilters(); // Issue 0020170

      // set shapes, of mesh and sub-mesh if any
      initHypCreator( aCreator );

      myDlg->setEnabled( false );
      aCreator->create(initParamHyp, aHypName, myDlg, this, SLOT( onHypoCreated( int ) ) );
      dialog = true;
    }
    else
    {
     SMESH::SMESH_Hypothesis_var aHyp =
       SMESH::CreateHypothesis(theTypeName, aHypName, false);
     aHyp.out();
    }
  }

  if( !dialog )
    onHypoCreated(2);
}

//================================================================================
/*!
 *  Necessary steps after hypothesis creation
 *  \param result - creation result:
 *   0 = rejected
 *   1 = accepted
 *   2 = additional value meaning that slot is called not from dialog box
 */
//================================================================================
void SMESHGUI_MeshOp::onHypoCreated( int result )
{
  if( result != 2 )
  {
    int obj = myDlg->getActiveObject();
    onActivateObject( obj ); // Issue 0020170. Restore filters
    myDlg->setEnabled( true );
  }

  _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent("SMESH");

  int nbHyp = myExistingHyps[myDim][myType].count();
  HypothesisData* algoData = hypData( myDim, Algo, currentHyp( myDim, Algo ));
  QStringList aNewHyps;
  existingHyps(myDim, myType, aFather, aNewHyps, myExistingHyps[myDim][myType], algoData);
  if (aNewHyps.count() > nbHyp)
  {
    for (int i = nbHyp; i < aNewHyps.count(); i++)
      myDlg->tab(myDim)->addHyp(myType, aNewHyps[i]);
  }

  if( result!=2 && myHypoSet )
    processSet();
}

//================================================================================
/*!
 * \brief Calls plugin methods for hypothesis editing
  * \param theHypType - specifies whether main hypothesis or additional one
  * is edited
  * \param theIndex - index of existing hypothesis
 *
 * Calls plugin methods for hypothesis editing
 */
//================================================================================
void SMESHGUI_MeshOp::onEditHyp( const int theHypType, const int theIndex )
{
  // Speicfies dimension of hypothesis to be created
  int aDim = getTabDim( sender(), myDlg );
  if (aDim == -1)
    return;

  const THypList& aList = myExistingHyps[ aDim ][ theHypType ];
  if ( theIndex < 0 || theIndex >= aList.count() )
    return;
  const THypItem& aHypItem = aList[ theIndex ];
  SMESH::SMESH_Hypothesis_var aHyp = aHypItem.first;
  if ( aHyp->_is_nil() )
    return;

  SMESHGUI_GenericHypothesisCreator* aCreator =
    SMESH::GetHypothesisCreator( SMESH::toQStr( aHyp->GetName() ));
  if ( aCreator )
  {
    // set initial parameters
    SMESH::SMESH_Hypothesis_var initParamHyp =
      getInitParamsHypothesis( SMESH::toQStr( aHyp->GetName() ),
                               SMESH::toQStr( aHyp->GetLibName() ));
    aCreator->setInitParamsHypothesis( initParamHyp );

    // set shapes, of mesh and sub-mesh if any
    initHypCreator( aCreator );

    removeCustomFilters(); // Issue 0020170
    myDlg->setEnabled( false );

    aCreator->edit( aHyp.in(), aHypItem.second, dlg(), this, SLOT( onHypoEdited( int ) ) );
  }
}

//================================================================================
/*!
 *  Necessary steps after hypothesis edition
 *  \param result - creation result:
 *   0 = rejected
 *   1 = accepted
 */
//================================================================================
void SMESHGUI_MeshOp::onHypoEdited( int result )
{
  int obj = myDlg->getActiveObject();
  onActivateObject( obj ); // Issue 0020170. Restore filters
  myDlg->setEnabled( true );
}

//================================================================================
/*!
 * \brief access to hypothesis data
  * \param theDim - hyp dimension
  * \param theHypType - hyp type (Algo,MainHyp or AddHyp)
  * \param theIndex - index in the list
  * \retval HypothesisData* - result data, may be 0
 */
//================================================================================
HypothesisData* SMESHGUI_MeshOp::hypData( const int theDim,
                                          const int theHypType,
                                          const int theIndex)
{
  if ( theDim     > -1 && theDim    <= SMESH::DIM_3D &&
       theHypType > -1 && theHypType < NbHypTypes &&
       theIndex   > -1 && theIndex   < myAvailableHypData[ theDim ][ theHypType ].count() )
    return myAvailableHypData[ theDim ][ theHypType ][ theIndex ];
  return 0;
}

//================================================================================
/*!
 * \brief Set available algos and hypos according to the selected algorithm
  * \param theIndex - algorithm index
 */
//================================================================================
void SMESHGUI_MeshOp::onAlgoSelected( const int theIndex,
                                      const int theDim )
{
  if ( myIgnoreAlgoSelection )
    return;

  int aDim = theDim < 0 ? getTabDim( sender(), myDlg ): theDim;
  if (aDim == -1)
    return;

  const bool isSubmesh = ( myToCreate ? !myIsMesh : myDlg->isObjectShown( SMESHGUI_MeshDlg::Mesh ));

  HypothesisData* algoData = hypData( aDim, Algo, theIndex );
  HypothesisData* algoByDim[4];
  algoByDim[ aDim ] = algoData;

  QStringList anAvailable;

  // enable / disable tabs
  if ( myIsOnGeometry ) {
    for (int i = SMESH::DIM_3D; i >= SMESH::DIM_0D; i--) {
      if ( i > aDim ) {
        if ( i > myMaxShapeDim ) myDlg->disableTab( i );
        else                     myDlg->enableTab( i );
      }
      else if ( i == aDim ) {
        continue;
      }
      else {//( i < aDim )
        if ( algoData && algoData->InputTypes.isEmpty() ) {
          myDlg->disableTab( i );
          for ( int type = Algo, nbTypes = nbDlgHypTypes(i); type < nbTypes; type++ )
            setCurrentHyp(i, type, -1);
        }
        else {
          myDlg->enableTab( i );
        }
      }
    }
  }

  int algoDim = aDim;
  HypothesisData* a3DAlgo = 0;
  // 2 loops: backward and forward from algo dimension
  for ( int forward = 0; forward <= 1; ++forward )
  {
    int dim = algoDim + 1, lastDim = SMESH::DIM_3D, dir = 1;
    if ( !forward ) {
      dim = algoDim - 1; lastDim = SMESH::DIM_0D; dir = -1;
    }
    HypothesisData* prevAlgo = algoData;
    bool noCompatible = false;
    for ( ; dim * dir <= lastDim * dir; dim += dir)
    {
      if ( !isAccessibleDim( dim ))
        continue;
      if ( noCompatible ) { // the selected algo has no compatible ones
        anAvailable.clear();
        myDlg->tab( dim )->setAvailableHyps( Algo, anAvailable );
        myAvailableHypData[dim][Algo].clear();
        algoByDim[ dim ] = 0;
        continue;
      }
      HypothesisData* nextAlgo = 0;
      if ( myMaxShapeDim == SMESH::DIM_3D && a3DAlgo && dim == SMESH::DIM_2D ) {
        nextAlgo = a3DAlgo;
      }
      // get currently selected algo
      int algoIndex = currentHyp( dim, Algo );
      HypothesisData* curAlgo = hypData( dim, Algo, algoIndex );

      QString anCompareType = currentMeshTypeName(myDlg->currentMeshType());
      QString anCurrentCompareType = "";
      if ( dim == SMESH::DIM_3D || anCompareType == "ANY" )
        anCurrentCompareType = anCompareType;
      else if ( dim == SMESH::DIM_2D ) {
        anCurrentCompareType = (anCompareType == "HEXA" || anCompareType == "QUAD") ? "QUAD" : "TRIA";
        nextAlgo = 0;
      }

      // set new available algorithms
      availableHyps( dim, Algo, anAvailable, myAvailableHypData[dim][Algo], prevAlgo, nextAlgo, anCurrentCompareType);
      HypothesisData* soleCompatible = 0;
      if ( anAvailable.count() == 1 )
        soleCompatible = myAvailableHypData[dim][Algo][0];
      myDlg->tab( dim )->setAvailableHyps( Algo, anAvailable );
      noCompatible = anAvailable.isEmpty();
      algoIndex = myAvailableHypData[dim][Algo].indexOf( curAlgo );
      if ( !isSubmesh && algoIndex < 0 && soleCompatible && !forward && dim != SMESH::DIM_0D) {
        // select the sole compatible algo
        algoIndex = 0;
      }
      setCurrentHyp( dim, Algo, algoIndex );

      // remember current algo
      prevAlgo = algoByDim[ dim ] = hypData( dim, Algo, algoIndex );

    } // loop on dims

    if ( myMaxShapeDim == SMESH::DIM_3D && forward && algoDim == SMESH::DIM_1D )
    {
      algoDim = SMESH::DIM_3D;
      forward = -1;
      a3DAlgo = prevAlgo;
      continue;
    }
  } // loops backward and forward


  // set hypotheses corresponding to the found algorithms

  _PTR(SObject) pObj = SMESH::GetActiveStudyDocument()->FindComponent("SMESH");

  for ( int dim = SMESH::DIM_0D; dim <= SMESH::DIM_3D; dim++ )
  {
    if ( !isAccessibleDim( dim ))
      continue;

    // get indices of selected hyps
    const int nbTypes = nbDlgHypTypes(dim);
    std::vector<int> hypIndexByType( nbTypes, -1 );
    for ( int dlgType = MainHyp; dlgType < nbTypes; dlgType++ )
    {
      hypIndexByType[ dlgType ] = currentHyp( dim, dlgType );
    }

    // update hyps
    for ( int dlgType = MainHyp; dlgType < nbTypes; dlgType++ )
    {
      const int type = Min( dlgType, AddHyp );
      myAvailableHypData[ dim ][ type ].clear();
      QStringList anAvailable, anExisting;

      HypothesisData* curAlgo = algoByDim[ dim ];
      int hypIndex = hypIndexByType[ dlgType ];

      SMESH::SMESH_Hypothesis_var curHyp;
      if ( hypIndex >= 0 && hypIndex < myExistingHyps[ dim ][ type ].count() )
        curHyp = myExistingHyps[ dim ][ type ][ hypIndex ].first;

      if ( !myToCreate && !curAlgo && !curHyp->_is_nil() ) { // edition, algo not selected
        // try to find algo by selected hypothesis in order to keep it selected
        bool algoDeselectedByUser = ( theDim < 0 && aDim == dim );
        QString curHypType = SMESH::toQStr( curHyp->GetName() );
        if ( !algoDeselectedByUser &&
             myObjHyps[ dim ][ type ].count() > 0 &&
             curHypType == SMESH::toQStr( myObjHyps[ dim ][ type ].first().first->GetName()) )
        {
          HypothesisData* hypData = SMESH::GetHypothesisData( SMESH::toQStr( curHyp->GetName() ));
          for (int i = 0; i < myAvailableHypData[ dim ][ Algo ].count(); ++i) {
            curAlgo = myAvailableHypData[ dim ][ Algo ][ i ];
            if (curAlgo && hypData && isCompatible(curAlgo, hypData, type))
              break;
            else
              curAlgo = 0;
          }
        }
      }
      // get hyps compatible with curAlgo
      bool defaulHypAvlbl = false;
      if ( curAlgo )
      {
        // check if a selected hyp is compatible with the curAlgo
        if ( !curHyp->_is_nil() ) {
          HypothesisData* hypData = SMESH::GetHypothesisData( SMESH::toQStr( curHyp->GetName() ));
          if ( !isCompatible( curAlgo, hypData, type ))
            curHyp = SMESH::SMESH_Hypothesis::_nil();
        }
        availableHyps( dim, type, anAvailable, myAvailableHypData[ dim ][ type ], curAlgo);
        existingHyps( dim, type, pObj, anExisting, myExistingHyps[ dim ][ type ], curAlgo);
        defaulHypAvlbl = (type == MainHyp && !curAlgo->IsAuxOrNeedHyp );
      }
      // set list of hypotheses
      if ( dlgType <= AddHyp )
      {
        myDlg->tab( dim )->setAvailableHyps( type, anAvailable );
        myDlg->tab( dim )->setExistingHyps( type, anExisting, defaulHypAvlbl );
      }
      // set current existing hypothesis
      if ( !curHyp->_is_nil() && !anExisting.isEmpty() )
        hypIndex = this->find( curHyp, myExistingHyps[ dim ][ type ]);
      else
        hypIndex = -1;
      if ( !isSubmesh && myToCreate && hypIndex < 0 && anExisting.count() == 1 ) {
        // none is yet selected => select the sole existing if it is not optional
        CORBA::String_var hypTypeName = myExistingHyps[ dim ][ type ].first().first->GetName();
        bool isOptional = true;
        if ( algoByDim[ dim ] &&
             SMESH::IsAvailableHypothesis( algoByDim[ dim ], hypTypeName.in(), isOptional ) &&
             !isOptional )
          hypIndex = 0;
      }
      setCurrentHyp( dim, dlgType, hypIndex );
    }
  }
}

//================================================================================
/*!
 * \brief Creates and selects hypothesis of hypotheses set
 * \param theSetName - The name of hypotheses set
 */
//================================================================================
void SMESHGUI_MeshOp::onHypoSet( const QString& theSetName )
{
  myHypoSet = SMESH::GetHypothesesSet(theSetName);
  if (!myHypoSet)
    return;

  // clear all hyps
  for (int dim = SMESH::DIM_0D; dim <= SMESH::DIM_3D; dim++) {
    setCurrentHyp(dim, Algo, -1);
    setCurrentHyp(dim, AddHyp, -1);
    setCurrentHyp(dim, MainHyp, -1);
  }

  myHypoSet->init(true); //algorithms
  processSet();
  myHypoSet->init(false); //hypotheses
  processSet();
  myHypoSet = 0;
}

//================================================================================
/*!
 * \brief One step of hypothesis/algorithm list creation
 *
 * Creates a hypothesis or an algorithm for current item of internal list of names myHypoSet
 */
//================================================================================
void SMESHGUI_MeshOp::processSet()
{
  myHypoSet->next();
  if( !myHypoSet->more() )
    return;

  bool isAlgo = myHypoSet->isAlgo();
  QString aHypoTypeName = myHypoSet->current();
  HypothesisData* aHypData = SMESH::GetHypothesisData(aHypoTypeName);
  if (!aHypData)
  {
    processSet();
    return;
  }

  int aDim = aHypData->Dim[0];
  // create or/and set
  if (isAlgo)
  {
    int index = myAvailableHypData[aDim][Algo].indexOf( aHypData );
    if ( index < 0 )
    {
      QStringList anAvailable;
      availableHyps( aDim, Algo, anAvailable, myAvailableHypData[aDim][Algo] );
      myDlg->tab( aDim )->setAvailableHyps( Algo, anAvailable );
      index = myAvailableHypData[aDim][Algo].indexOf( aHypData );
    }
    setCurrentHyp( aDim, Algo, index );
    onAlgoSelected( index, aDim );
    processSet();
  }
  else
  {
    bool mainHyp = true;
    QStringList anAvailable;
    availableHyps( aDim, MainHyp, anAvailable, myAvailableHypData[aDim][MainHyp] );
    myDlg->tab( aDim )->setAvailableHyps( MainHyp, anAvailable );
    int index = myAvailableHypData[aDim][MainHyp].indexOf( aHypData );
    if ( index < 0 )
    {
      mainHyp = false;
      index = myAvailableHypData[aDim][AddHyp].indexOf( aHypData );
    }
    if (index >= 0)
      createHypothesis(aDim, mainHyp ? MainHyp : AddHyp, aHypoTypeName);
    else
      processSet();
  }
}

//================================================================================
/*!
 * \brief Creates mesh
  * \param theMess - Output parameter intended for returning error message
  * \param theEntryList - List of entries of published objects
  * \retval bool  - TRUE if mesh is created, FALSE otherwise
 *
 * Creates mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::createMesh( QString& theMess, QStringList& theEntryList )
{
  theMess = "";

  QStringList aList;
  myDlg->selectedObject( SMESHGUI_MeshDlg::Geom, aList );
  if ( aList.isEmpty() )
  {
    SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
    if ( aSMESHGen->_is_nil() )
      return false;

    SMESH::SMESH_Mesh_var aMeshVar= aSMESHGen->CreateEmptyMesh();
    if ( aMeshVar->_is_nil() )
      return false;

    _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshVar.in() );
    if ( aMeshSO ) {
      SMESH::SetName( aMeshSO, myDlg->objectText( SMESHGUI_MeshDlg::Obj ) );
      theEntryList.append( aMeshSO->GetID().c_str() );
    }
    return true;
  }
  QString namePrefix;
  if ( aList.count() > 1 )
  {
    namePrefix = myDlg->objectText( SMESHGUI_MeshDlg::Obj );
    int i = namePrefix.length() - 1;
    while ( i > 0 && namePrefix[i].isDigit() )
      --i;
    if ( i < namePrefix.length() - 1 )
      namePrefix.chop( namePrefix.length() - 1 - i );
    else
      namePrefix += "_";
  }
  QStringList::Iterator it = aList.begin();
  for ( int i = 0; it!=aList.end(); it++, ++i )
  {
    QString aGeomEntry = *it;
    _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.toLatin1().data() );
    GEOM::GEOM_Object_var aGeomVar =
      GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );

    SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
    if ( aSMESHGen->_is_nil() )
      return false;

    SUIT_OverrideCursor aWaitCursor;

    // create mesh
    SMESH::SMESH_Mesh_var aMeshVar = aSMESHGen->CreateMesh( aGeomVar );
    if ( aMeshVar->_is_nil() )
      return false;
    _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshVar.in() );
    if ( aMeshSO ) {
      theEntryList.append( aMeshSO->GetID().c_str() );
      if ( i > 0 ) setDefaultName( namePrefix );
      SMESH::SetName( aMeshSO, myDlg->objectText( SMESHGUI_MeshDlg::Obj ) );
    }

    for ( int aDim = SMESH::DIM_0D; aDim <= SMESH::DIM_3D; aDim++ )
    {
      if ( !isAccessibleDim( aDim )) continue;

      // assign hypotheses
      for ( int dlgType = MainHyp; dlgType < nbDlgHypTypes(aDim); dlgType++ )
      {
        const int aHypIndex = currentHyp( aDim, dlgType );
        const int  aHypType = Min( dlgType, AddHyp );
        if ( aHypIndex >= 0 && aHypIndex < myExistingHyps[ aDim ][ aHypType ].count() )
        {
          SMESH::SMESH_Hypothesis_var aHypVar =
            myExistingHyps[ aDim ][ aHypType ][ aHypIndex ].first;
          if ( !aHypVar->_is_nil() )
            SMESH::AddHypothesisOnMesh( aMeshVar, aHypVar );
        }
      }
      // find or create algorithm
      SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( aDim );
      if ( !anAlgoVar->_is_nil() )
        SMESH::AddHypothesisOnMesh( aMeshVar, anAlgoVar );
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Creates sub-mesh
  * \param theMess - Output parameter intended for returning error message
  * \param theEntryList - List of entries of published objects
  * \retval bool  - TRUE if sub-mesh is created, FALSE otherwise
 *
 * Creates sub-mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::createSubMesh( QString& theMess, QStringList& theEntryList )
{
  theMess = "";

  SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
  if ( aSMESHGen->_is_nil() )
    return false;

  // get mesh object
  QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() );
  SMESH::SMESH_Mesh_var aMeshVar =
    SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() );
  if (aMeshVar->_is_nil())
    return false;

  // GEOM shape of the main mesh
  GEOM::GEOM_Object_var mainGeom = aMeshVar->GetShapeToMesh();

  // Name for the new sub-mesh
  QString aName = myDlg->objectText(SMESHGUI_MeshDlg::Obj);

  // get geom object
  GEOM::GEOM_Object_var aGeomVar;
  QStringList aGEOMs;
  myDlg->selectedObject(SMESHGUI_MeshDlg::Geom, aGEOMs);
  if (aGEOMs.count() == 1)
  {
    //QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
    QString aGeomEntry = aGEOMs.first();
    _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.toLatin1().data() );
    aGeomVar = GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
  }
  else if (aGEOMs.count() > 1)
  {
    // create a GEOM group
    GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (!geomGen->_is_nil() && aStudy) {
      GEOM::GEOM_IGroupOperations_wrap op =
        geomGen->GetIGroupOperations(aStudy->StudyId());
      if (!op->_is_nil()) {
        // check and add all selected GEOM objects: they must be
        // a sub-shapes of the main GEOM and must be of one type
        int iSubSh = 0;
        TopAbs_ShapeEnum aGroupType = TopAbs_SHAPE;
        GEOM::ListOfGO_var aSeq = new GEOM::ListOfGO;
        aSeq->length(aGEOMs.count());
        QStringList::const_iterator aSubShapesIter = aGEOMs.begin();
        for ( ; aSubShapesIter != aGEOMs.end(); aSubShapesIter++, iSubSh++) {
          QString aSubGeomEntry = (*aSubShapesIter);
          _PTR(SObject) pSubGeom = studyDS()->FindObjectID(aSubGeomEntry.toLatin1().data());
          GEOM::GEOM_Object_var aSubGeomVar =
            GEOM::GEOM_Object::_narrow(_CAST(SObject,pSubGeom)->GetObject());
          TopAbs_ShapeEnum aSubShapeType = (TopAbs_ShapeEnum)aSubGeomVar->GetShapeType();
          if (iSubSh == 0) {
            aGroupType = aSubShapeType;
          } else {
            if (aSubShapeType != aGroupType)
              aGroupType = TopAbs_SHAPE;
          }
          aSeq[iSubSh] = aSubGeomVar;
        }
        // create a group
        GEOM::GEOM_Object_wrap aGroupVar = op->CreateGroup(mainGeom, aGroupType);
        op->UnionList(aGroupVar, aSeq);

        if (op->IsDone())
        {
          aGeomVar = GEOM::GEOM_Object::_duplicate( aGroupVar.in() );

          // publish the GEOM group in study
          QString aNewGeomGroupName ("Auto_group_for_");
          aNewGeomGroupName += aName;
          SALOMEDS::Study_var aStudyVar = _CAST(Study, aStudy)->GetStudy();
          SALOMEDS::SObject_wrap aNewGroupSO =
            geomGen->AddInStudy( aStudyVar, aGeomVar,
                                 aNewGeomGroupName.toLatin1().data(), mainGeom);
        }
      }
    }
  }
  else {
  }
  if (aGeomVar->_is_nil())
    return false;

  SUIT_OverrideCursor aWaitCursor;

  // create sub-mesh
  SMESH::SMESH_subMesh_var aSubMeshVar = aMeshVar->GetSubMesh( aGeomVar, aName.toLatin1().data() );
  _PTR(SObject) aSubMeshSO = SMESH::FindSObject( aSubMeshVar.in() );
  if ( aSubMeshSO ) {
    SMESH::SetName( aSubMeshSO, aName.toLatin1().data() );
    theEntryList.append( aSubMeshSO->GetID().c_str() );
  }

  for ( int aDim = SMESH::DIM_0D; aDim <= SMESH::DIM_3D; aDim++ )
  {
    if ( !isAccessibleDim( aDim )) continue;

    // find or create algorithm
    SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( aDim );
    if ( !anAlgoVar->_is_nil() )
      SMESH::AddHypothesisOnSubMesh( aSubMeshVar, anAlgoVar );
    // assign hypotheses
    for ( int dlgType = MainHyp; dlgType < nbDlgHypTypes(aDim); dlgType++ )
    {
      const int aHypIndex = currentHyp( aDim, dlgType );
      const int  aHypType = Min( dlgType, AddHyp );
      if ( aHypIndex >= 0 && aHypIndex < myExistingHyps[ aDim ][ aHypType ].count() )
      {
        SMESH::SMESH_Hypothesis_var aHypVar =
          myExistingHyps[ aDim ][ aHypType ][ aHypIndex ].first;
        if ( !aHypVar->_is_nil() )
          SMESH::AddHypothesisOnSubMesh( aSubMeshVar, aHypVar );
      }
    }
  }

  // deselect geometry: next submesh should be created on other sub-shape
  myDlg->clearSelection( SMESHGUI_MeshDlg::Geom );
  selectObject( _PTR(SObject)() );
  selectionDone();

  checkSubMeshConcurrency( aMeshVar, aSubMeshVar, /*askUser=*/true );

  return true;
}

//================================================================================
/*!
 * \brief Gets current hypothesis or algorithms
  * \param theDim - dimension of hypothesis or algorithm
  * \param theHypType - Type of hypothesis (Algo, MainHyp, AddHyp)
  * \retval int - current hypothesis or algorithms
 *
 * Gets current hypothesis or algorithms
 */
//================================================================================
int SMESHGUI_MeshOp::currentHyp( const int theDim, const int theHypType ) const
{
  return myDlg->tab( theDim )->currentHyp( theHypType ) - 1;
}

//================================================================================
/*!
 * \brief Checks if a hypothesis is selected
 */
//================================================================================

bool SMESHGUI_MeshOp::isSelectedHyp( int theDim, int theHypType, int theIndex) const
{
  if ( theIndex < 0 )
    return false;

  if ( theHypType < AddHyp ) // only one hyp can be selected
    return currentHyp( theDim, theHypType ) == theIndex;

  for ( int dlgHypType = AddHyp; dlgHypType < nbDlgHypTypes( theDim ); ++dlgHypType )
    if ( currentHyp( theDim, dlgHypType ) == theIndex )
      return true;

  return false;
}

//================================================================================
/*!
 * \brief Returns nb of HypType's taking into account possible several
 *        selected additional hypotheses which are coded as additional HypType's.
 */
//================================================================================

int SMESHGUI_MeshOp::nbDlgHypTypes( const int dim ) const
{
  return NbHypTypes + myDlg->tab( dim )->nbAddHypTypes();
}

//================================================================================
/*!
 * \brief Returns true if hypotheses of given dim can be assigned
  * \param theDim - hypotheses dimension
  * \retval bool - result
 */
//================================================================================
bool SMESHGUI_MeshOp::isAccessibleDim( const int theDim ) const
{
  return myDlg->isTabEnabled( theDim );
}

//================================================================================
/*!
 * \brief Sets current hypothesis or algorithms
  * \param theDim - dimension of hypothesis or algorithm
  * \param theHypType - Type of hypothesis (Algo, MainHyp, AddHyp)
  * \param theIndex - Index of hypothesis
  * \param updateHypsOnAlgoDeselection - to clear and disable hyps if algo deselected
 *
 * Gets current hypothesis or algorithms
 */
//================================================================================
void SMESHGUI_MeshOp::setCurrentHyp( const int  theDim,
                                     const int  theHypType,
                                     const int  theIndex,
                                     const bool updateHypsOnAlgoDeselection)
{
  myIgnoreAlgoSelection = true;
  myDlg->tab( theDim )->setCurrentHyp( theHypType, theIndex + 1 );
  myIgnoreAlgoSelection = false;

  if ( updateHypsOnAlgoDeselection && theHypType == Algo && theIndex < 0 )
  {
    const QStringList noHyps;
    myDlg->tab( theDim )->setAvailableHyps( MainHyp, noHyps );
    myDlg->tab( theDim )->setExistingHyps ( MainHyp, noHyps );
    myDlg->tab( theDim )->setAvailableHyps( AddHyp,  noHyps );
    myDlg->tab( theDim )->setExistingHyps ( AddHyp,  noHyps );
  }
}

//================================================================================
/*!
 * \brief Generates default and sets mesh/submesh name
 *
 * Generates and sets default mesh/submesh name(Mesh_1, Mesh_2, etc.)
 */
//================================================================================
void SMESHGUI_MeshOp::setDefaultName( const QString& thePrefix ) const
{
  QString aResName;

  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
  int i = 1;

  QString aPrefix = thePrefix;
  if ( aPrefix.isEmpty() )
    aPrefix = tr( myIsMesh ? "SMESH_OBJECT_MESH" : "SMESH_SUBMESH" ) + "_";

  _PTR(SObject) anObj;
  do
  {
    aResName = aPrefix + QString::number( i++ );
    anObj = aStudy->FindObject( aResName.toLatin1().data() );
  }
  while ( anObj );

  QLineEdit* aControl = ( QLineEdit* )myDlg->objectWg(
    SMESHGUI_MeshDlg::Obj, SMESHGUI_MeshDlg::Control );
  aControl->setText( aResName );
}

//================================================================================
/*!
 * \brief Gets algorithm or creates it if necessary
  * \param theDim - specifies dimension of returned hypotheses/algorifms
  * \retval SMESH::SMESH_Hypothesis_var - algorithm
 *
 * Gets algorithm or creates it if necessary
 */
//================================================================================
SMESH::SMESH_Hypothesis_var SMESHGUI_MeshOp::getAlgo( const int theDim )
{
  SMESH::SMESH_Hypothesis_var anAlgoVar;

  // get type of the selected algo
  int aHypIndex = currentHyp( theDim, Algo );
  THypDataList& dataList = myAvailableHypData[ theDim ][ Algo ];
  if ( aHypIndex < 0 || aHypIndex >= dataList.count())
    return anAlgoVar;
  QString aHypName = dataList[ aHypIndex ]->TypeName;

  // get existing algorithms
  _PTR(SObject) pObj = SMESH::GetActiveStudyDocument()->FindComponent("SMESH");
  QStringList tmp;
  existingHyps( theDim, Algo, pObj, tmp, myExistingHyps[ theDim ][ Algo ]);

  // look for an existing algo of such a type
  THypList& aHypVarList = myExistingHyps[ theDim ][ Algo ];
  THypList::iterator anIter = aHypVarList.begin();
  for ( ; anIter != aHypVarList.end(); anIter++)
  {
    SMESH::SMESH_Hypothesis_var aHypVar = (*anIter).first;
    if ( !aHypVar->_is_nil() && aHypName == SMESH::toQStr( aHypVar->GetName() ))
    {
      anAlgoVar = aHypVar;
      break;
    }
  }

  if (anAlgoVar->_is_nil())
  {
    HypothesisData* aHypData = SMESH::GetHypothesisData( aHypName );
    if (aHypData)
    {
      QString aClientLibName = aHypData->ClientLibName;
      if ( aClientLibName.isEmpty() )
      {
        // Call hypothesis creation server method (without GUI)
        SMESH::SMESH_Hypothesis_var aHyp =
          SMESH::CreateHypothesis(aHypName, aHypName, true);
        aHyp.out();
      }
      else
      {
        // Get hypotheses creator client (GUI)
        SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator(aHypName);

        // Create algorithm
        if (aCreator)
          aCreator->create( true, aHypName, myDlg, 0, QString::null );
        else {
          SMESH::SMESH_Hypothesis_var aHyp =
            SMESH::CreateHypothesis(aHypName, aHypName, true);
          aHyp.out();
        }
        delete aCreator;
      }
      QStringList tmpList;
      _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
      existingHyps( theDim, Algo, aFather, tmpList, myExistingHyps[ theDim ][ Algo ] );
    }

    THypList& aNewHypVarList = myExistingHyps[ theDim ][ Algo ];
    for ( anIter = aNewHypVarList.begin(); anIter != aNewHypVarList.end(); ++anIter )
    {
      SMESH::SMESH_Hypothesis_var aHypVar = (*anIter).first;
      if ( !aHypVar->_is_nil() && aHypName == SMESH::toQStr( aHypVar->GetName() ))
      {
        anAlgoVar = aHypVar;
        break;
      }
    }
  }

  return anAlgoVar._retn();
}

//================================================================================
/*!
 * \brief Reads parameters of an edited mesh/sub-mesh and assigns them to the dialog
 *
 * Called when mesh is edited only.
 */
//================================================================================
void SMESHGUI_MeshOp::readMesh()
{
  QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
  _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
  if ( !pObj )
    return;

  if (myIsOnGeometry) {
    // Get name of mesh if current object is sub-mesh
    SMESH::SMESH_subMesh_var aSubMeshVar =
      SMESH::SMESH_subMesh::_narrow( _CAST( SObject,pObj )->GetObject() );
    if ( !aSubMeshVar->_is_nil() )
    {
      SMESH::SMESH_Mesh_var aMeshVar =  aSubMeshVar->GetFather();
      if ( !aMeshVar->_is_nil() )
      {
        _PTR(SObject) aMeshSO = SMESH::FindSObject( aMeshVar );
        QString aMeshName = name( aMeshSO );
        myDlg->setObjectText( SMESHGUI_MeshDlg::Mesh, aMeshName );
      }
      myHasConcurrentSubBefore = checkSubMeshConcurrency( aMeshVar, aSubMeshVar );
    }

    // Get name of geometry object
    CORBA::String_var name = SMESH::GetGeomName( pObj );
    if ( name.in() )
      myDlg->setObjectText( SMESHGUI_MeshDlg::Geom, name.in() );
  }

  // Get hypotheses and algorithms assigned to the mesh/sub-mesh
  QStringList anExisting;
  const int lastDim = ( myIsOnGeometry ) ? SMESH::DIM_0D : SMESH::DIM_3D;
  bool algoFound = false;
  for ( int dim = SMESH::DIM_3D; dim >= lastDim; --dim )
  {
    // get algorithm
    existingHyps( dim, Algo, pObj, anExisting, myObjHyps[ dim ][ Algo ] );
    // find algo index among available ones
    int aHypIndex = -1;
    if ( myObjHyps[ dim ][ Algo ].count() > 0 )
    {
      SMESH::SMESH_Hypothesis_var aVar = myObjHyps[ dim ][ Algo ].first().first;
      HypothesisData* algoData = SMESH::GetHypothesisData( SMESH::toQStr( aVar->GetName() ));
      aHypIndex = myAvailableHypData[ dim ][ Algo ].indexOf ( algoData );
      //       if ( aHypIndex < 0 && algoData ) {
      //         // assigned algo is incompatible with other algorithms
      //         myAvailableHypData[ dim ][ Algo ].push_back( algoData );
      //         aHypIndex = myAvailableHypData[ dim ][ hypType ].count() - 1;
      //       }
      algoFound = ( aHypIndex > -1 );
    }
    setCurrentHyp( dim, Algo, aHypIndex );
    // set existing and available hypothesis according to the selected algo
    if ( aHypIndex > -1 || !algoFound )
      onAlgoSelected( aHypIndex, dim );
  }

  // get hypotheses
  bool hypWithoutAlgo = false;
  for ( int dim = SMESH::DIM_3D; dim >= lastDim; --dim )
  {
    for ( int hypType = MainHyp; hypType <= AddHyp; hypType++ )
    {
      // get hypotheses
      existingHyps( dim, hypType, pObj, anExisting, myObjHyps[ dim ][ hypType ] );
      if ( myObjHyps[ dim ][ hypType ].count() == 0 ) {
        setCurrentHyp( dim, hypType, -1 );
      }
      for ( int i = 0, nb = myObjHyps[ dim ][ hypType ].count(); i < nb; ++i )
      {
        // find index of required hypothesis among existing ones for this dimension and type
        int aHypIndex = find( myObjHyps[ dim ][ hypType ][ i ].first,
                              myExistingHyps[ dim ][ hypType ] );
        if ( aHypIndex < 0 ) {
          // assigned hypothesis is incompatible with the algorithm
          if ( currentHyp( dim, Algo ) < 0 )
          { // none algo selected; it is edition for sure, of submesh maybe
            hypWithoutAlgo = true;
            myExistingHyps[ dim ][ hypType ].push_back( myObjHyps[ dim ][ hypType ][ i ] );
            anExisting.push_back( myObjHyps[ dim ][ hypType ][ i ].second );
            aHypIndex = myExistingHyps[ dim ][ hypType ].count() - 1;
            myDlg->tab( dim )->setExistingHyps( hypType, anExisting );
          }
        }
        setCurrentHyp( dim, hypType + i, aHypIndex );

        if ( hypType == MainHyp ) break; // only one main hyp allowed
      }
    }
  }
  // make available other hyps of same type as one without algo
  if ( hypWithoutAlgo )
    onAlgoSelected( currentHyp( 0, Algo ), 0 );
}

//================================================================================
/*!
 * \brief Gets name of object
 * \param theSO - SObject
 * \retval QString - name of object
 *
 * Gets name of object
 */
//================================================================================
QString SMESHGUI_MeshOp::name( _PTR(SObject) theSO ) const
{
  QString aResName;
  if ( theSO )
  {
    _PTR(GenericAttribute) anAttr;
    _PTR(AttributeName)    aNameAttr;
    if ( theSO->FindAttribute( anAttr, "AttributeName" ) )
    {
      aNameAttr = anAttr;
      aResName = aNameAttr->Value().c_str();
    }
  }
  return aResName;
}

//================================================================================
/*!
 * \brief Finds hypothesis in input list
  * \param theHyp - hypothesis to be found
  * \param theHypList - input list of hypotheses
  * \retval int - index of hypothesis or -1 if it is not found
 *
 * Finds position of hypothesis in input list
 */
//================================================================================
int SMESHGUI_MeshOp::find( const SMESH::SMESH_Hypothesis_var& theHyp,
                           const THypList& theHypList ) const
{
  int aRes = -1;
  if ( !theHyp->_is_nil() )
  {
    int i = 0;
    THypList::const_iterator anIter = theHypList.begin();
    for ( ; anIter != theHypList.end(); ++ anIter)
    {
      if ( theHyp->_is_equivalent( (*anIter).first ) )
      {
        aRes = i;
        break;
      }
      i++;
    }
  }
  return aRes;
}

//================================================================================
/*!
 * \brief Edits mesh or sub-mesh
  * \param theMess - Output parameter intended for returning error message
  * \retval bool  - TRUE if mesh is edited succesfully, FALSE otherwise
 *
 * Assigns new name hypotheses and algoriths to the mesh or sub-mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::editMeshOrSubMesh( QString& theMess )
{
  theMess = "";

  SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
  if ( aSMESHGen->_is_nil() )
    return false;

  QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
  _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
  if ( !pObj )
    return false;

  SUIT_OverrideCursor aWaitCursor;

  // Set new name
  QString aName = myDlg->objectText( SMESHGUI_MeshDlg::Obj );
  SMESH::SetName( pObj, aName );
  int aDim = ( myIsOnGeometry ) ? SMESH::DIM_0D : SMESH::DIM_3D;

  // First, remove old algos in order to avoid messages on algorithm hiding
  for ( int dim = aDim; dim <= SMESH::DIM_3D; dim++ )
  {
    if ( /*isAccessibleDim( dim ) &&*/ myObjHyps[ dim ][ Algo ].count() > 0 )
    {
      SMESH::SMESH_Hypothesis_var anOldAlgo = myObjHyps[ dim ][ Algo ].first().first;
      SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( dim );
      if ( anAlgoVar->_is_nil() || // no new algo selected or
           SMESH::toQStr(anOldAlgo->GetName()) != SMESH::toQStr(anAlgoVar->GetName())) // algo change
      {
        // remove old algorithm
        SMESH::RemoveHypothesisOrAlgorithmOnMesh ( pObj, myObjHyps[ dim ][ Algo ].first().first );
        myObjHyps[ dim ][ Algo ].clear();
      }
    }
  }

  SALOMEDS_SObject* aSObject = _CAST(SObject, pObj);
  CORBA::Object_var anObject = aSObject->GetObject();
  SMESH::SMESH_Mesh_var       aMeshVar = SMESH::SMESH_Mesh::_narrow( anObject );
  SMESH::SMESH_subMesh_var aSubMeshVar = SMESH::SMESH_subMesh::_narrow( anObject );
  bool isMesh = !aMeshVar->_is_nil();
  if ( !isMesh && !aSubMeshVar->_is_nil() )
    aMeshVar = aSubMeshVar->GetFather();

  // Assign new algorithms and hypotheses
  for ( int dim = aDim; dim <= SMESH::DIM_3D; dim++ )
  {
    //if ( !isAccessibleDim( dim )) continue;

    // find or create algorithm
    SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( dim );

    // assign new algorithm
    if ( !anAlgoVar->_is_nil() && // some algo selected and
         myObjHyps[ dim ][ Algo ].count() == 0 ) // no algo assigned
    {
      if ( isMesh )
        SMESH::AddHypothesisOnMesh( aMeshVar, anAlgoVar );
      else if ( !aSubMeshVar->_is_nil() )
        SMESH::AddHypothesisOnSubMesh( aSubMeshVar, anAlgoVar );

      myObjHyps[ dim ][ Algo ].append( THypItem( anAlgoVar, aName) );
    }

    // remove deselected hypotheses
    for ( int hypType = MainHyp; hypType <= AddHyp; hypType++ )
    {
      for ( int i = 0, nb = myObjHyps[ dim ][ hypType ].count(); i < nb; ++i )
      {
        SMESH::SMESH_Hypothesis_var hyp = myObjHyps[ dim ][ hypType ][ i ].first;
        int hypIndex = this->find( hyp, myExistingHyps[ dim ][ hypType ]);
        if ( !isSelectedHyp( dim, hypType, hypIndex ) && !hyp->_is_nil() )
        {
          SMESH::RemoveHypothesisOrAlgorithmOnMesh( pObj, hyp );
        }
      }
    }
    // assign newly selected hypotheses
    for ( int dlgType = MainHyp; dlgType < nbDlgHypTypes(dim); dlgType++ )
    {
      const int curIndex = currentHyp( dim, dlgType );
      const int  hypType = Min( dlgType, AddHyp );
      if ( curIndex >= 0 && curIndex < myExistingHyps[ dim ][ hypType ].count() )
      {
        SMESH::SMESH_Hypothesis_var hyp = myExistingHyps[ dim ][ hypType ][ curIndex ].first;

        bool isAssigned = ( this->find( hyp, myObjHyps[ dim ][ hypType ]) >= 0 );
        if ( !isAssigned )
        {
          if ( isMesh )
            SMESH::AddHypothesisOnMesh (aMeshVar, hyp );
          else if ( !aSubMeshVar->_is_nil() )
            SMESH::AddHypothesisOnSubMesh ( aSubMeshVar, hyp );
        }
      }
      // reread all hypotheses of mesh
      QStringList anExisting;
      existingHyps( dim, hypType, pObj, anExisting, myObjHyps[ dim ][ hypType ] );
    }
  }

  myHasConcurrentSubBefore =
    checkSubMeshConcurrency( aMeshVar, aSubMeshVar, /*askUser=*/!myHasConcurrentSubBefore );

  return true;
}

//================================================================================
/*!
 * \brief Checks if a concurrent sub-meshes appear as result of sub-mesh
 *        creation/edition and, if (askUser) , proposes the uses to set up a desired
 *        order of sub-mesh computation.
 *        Returns \c true if a sub-mesh concurrency detected.
 */
//================================================================================

bool SMESHGUI_MeshOp::checkSubMeshConcurrency(SMESH::SMESH_Mesh_ptr    mesh,
                                              SMESH::SMESH_subMesh_ptr submesh,
                                              bool                     askUser)
{
  if ( CORBA::is_nil( mesh ) || CORBA::is_nil( submesh ))
    return false;

  bool isNewConcurrent = mesh->IsUnorderedSubMesh( submesh->GetId() );
  if ( isNewConcurrent && askUser )
  {
    int butID = SUIT_MessageBox::warning( myDlg->parentWidget(), tr( "SMESH_WARNING" ),
                                          tr("CONCURRENT_SUBMESH_APPEARS"),
                                          tr("SMESH_BUT_YES"), tr("SMESH_BUT_NO"));
    if ( butID == 0 )
    {
      _PTR(SObject) meshSO = SMESH::FindSObject( mesh );
      LightApp_SelectionMgr* aSelectionMgr = selectionMgr();
      if ( meshSO && aSelectionMgr )
      {
        myDlg->setEnabled( false ); // disactivate selection
        selectionMgr()->clearFilters();
        selectObject( meshSO );
        SMESHGUI::GetSMESHGUI()->OnGUIEvent( SMESHOp::OpMeshOrder ); // MESH_ORDER
        qApp->processEvents();

        myDlg->setEnabled( true );
        int obj = myDlg->getActiveObject();
        onActivateObject( obj ); // restore filter
        if ( !myToCreate )
        {
          selectObject( SMESH::FindSObject( submesh ));
          selectionDone();
        }
      }
    }
  }

  return isNewConcurrent;
}

//================================================================================
/*!
 * \brief Verifies whether given operator is valid for this one
 * \param theOtherOp - other operation
 * \return Returns TRUE if the given operator is valid for this one, FALSE otherwise
 *
 * method redefined from base class verifies whether given operator is valid for
 * this one (i.e. can be started "above" this operator). In current implementation method
 * retuns false if theOtherOp operation is not intended for deleting objects or mesh
 * elements.
 */
//================================================================================
bool SMESHGUI_MeshOp::isValid( SUIT_Operation* theOp ) const
{
  return SMESHGUI_Operation::isValid( theOp ) && !theOp->inherits( "SMESHGUI_MeshOp" );
}

//================================================================================
/*!
 * \brief SLOT. Is called when the user selects a way of geometry selection
 * \param theByMesh - true if the user wants to find geometry by mesh element
 */
//================================================================================
void SMESHGUI_MeshOp::onGeomSelectionByMesh( bool theByMesh )
{
  if ( theByMesh ) {
    if ( !myShapeByMeshOp ) {
      myShapeByMeshOp = new SMESHGUI_ShapeByMeshOp();
      connect(myShapeByMeshOp, SIGNAL(committed(SUIT_Operation*)),
              SLOT(onPublishShapeByMeshDlg(SUIT_Operation*)));
      connect(myShapeByMeshOp, SIGNAL(aborted(SUIT_Operation*)),
              SLOT(onCloseShapeByMeshDlg(SUIT_Operation*)));
    }
    // set mesh object to SMESHGUI_ShapeByMeshOp and start it
    QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
    if ( _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.toLatin1().data() )) {
      SMESH::SMESH_Mesh_var aMeshVar =
        SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() );
      if ( !aMeshVar->_is_nil() ) {
        myDlg->hide(); // stop processing selection
        myShapeByMeshOp->setModule( getSMESHGUI() );
        myShapeByMeshOp->setStudy( 0 ); // it's really necessary
        myShapeByMeshOp->SetMesh( aMeshVar );
        myShapeByMeshOp->start();
      }
    }
  }
}

//================================================================================
/*!
 * \brief SLOT. Is called when Ok is pressed in SMESHGUI_ShapeByMeshDlg
 */
//================================================================================
void SMESHGUI_MeshOp::onPublishShapeByMeshDlg(SUIT_Operation* op)
{
  if ( myShapeByMeshOp == op ) {
    SMESHGUI::GetSMESHGUI()->getApp()->updateObjectBrowser(); //MZN: 24.11.2006  IPAL13980 - Object Browser update added
    myDlg->show();
    // Select a found geometry object
    GEOM::GEOM_Object_var aGeomVar = myShapeByMeshOp->GetShape();
    if ( !aGeomVar->_is_nil() )
    {
      QString ID = SMESH::toQStr( aGeomVar->GetStudyEntry() );
      if ( _PTR(SObject) aGeomSO = studyDS()->FindObjectID( ID.toLatin1().data() )) {
        selectObject( aGeomSO );
        selectionDone();
      }
    }
  }
}

//================================================================================
/*!
 * \brief SLOT. Is called when Close is pressed in SMESHGUI_ShapeByMeshDlg
 */
//================================================================================
void SMESHGUI_MeshOp::onCloseShapeByMeshDlg(SUIT_Operation* op)
{
  if ( myShapeByMeshOp == op && myDlg ) {
    myDlg->show();
  }
}

//================================================================================
/*!
 * \brief Selects a SObject
 * \param theSObj - the SObject to select
 */
//================================================================================
void SMESHGUI_MeshOp::selectObject( _PTR(SObject) theSObj ) const
{
  if ( LightApp_SelectionMgr* sm = selectionMgr() ) {
    SALOME_ListIO anIOList;
    if ( theSObj ) {
      Handle(SALOME_InteractiveObject) anIO = new SALOME_InteractiveObject
        ( theSObj->GetID().c_str(), "SMESH", theSObj->GetName().c_str() );
      anIOList.Append( anIO );
    }
    sm->setSelectedObjects( anIOList, false );
  }
}
//================================================================================
/*!
 * \brief Create available list types of mesh
  * \param theTypeMesh - Output list of available types of mesh
 */
//================================================================================
void SMESHGUI_MeshOp::createMeshTypeList( QStringList& theTypeMesh)
{
  theTypeMesh.clear();
  theTypeMesh.append( tr( "MT_ANY" ) );
  if ( myMaxShapeDim >= 2 || myMaxShapeDim == -1 )
  {
    theTypeMesh.append( tr( "MT_TRIANGULAR" ) );
    theTypeMesh.append( tr( "MT_QUADRILATERAL" ) );
  }
  if ( myMaxShapeDim == 3 || myMaxShapeDim == -1 )
  {
    theTypeMesh.append( tr( "MT_TETRAHEDRAL" ) );
    theTypeMesh.append( tr( "MT_HEXAHEDRAL" ) );
  }

}
//================================================================================
/*!
 * \brief Set available types of mesh
  * \param theTypeMesh - List of available types of mesh
 */
//================================================================================
void SMESHGUI_MeshOp::setAvailableMeshType( const QStringList& theTypeMesh )
{
  myDlg->setAvailableMeshType( theTypeMesh );
}

//================================================================================
/*!
 * \brief SLOT. Is called when the user select type of mesh
  * \param theTabIndex - Index of current active tab
  * \param theIndex - Index of current type of mesh
 */
//================================================================================
void SMESHGUI_MeshOp::onAlgoSetByMeshType( const int theTabIndex, const int theIndex )
{
  setFilteredAlgoData( theTabIndex, theIndex);
}

//================================================================================
/*!
 * \brief Set a filtered list of available algorithms by mesh type
  * \param theTabIndex - Index of current active tab
  * \param theIndex - Index of current type of mesh
 */
//================================================================================
void SMESHGUI_MeshOp::setFilteredAlgoData( const int theTabIndex, const int theIndex )
{
  QStringList anAvailableAlgs;
  QString anCompareType = currentMeshTypeName( theIndex );
  int anCurrentAvailableAlgo = -1;
  bool isNone = true;
  int aDim = SMESH::DIM_3D;
  if ( theIndex == MT_TRIANGULAR || theIndex == MT_QUADRILATERAL)
    aDim = SMESH::DIM_2D;
  if ( anCompareType == "ANY" )
  {
    bool isReqDisBound = false;
    int aReqDim = SMESH::DIM_3D;
    for ( int dim = SMESH::DIM_3D; dim >= SMESH::DIM_2D; dim-- )
    {
      anCurrentAvailableAlgo = -1;
      isNone = currentHyp( dim, Algo ) < 0;
      //return current algo in current tab and set new algorithm list
      HypothesisData* algoCur = 0;
      if ( !isNone && !myAvailableHypData[dim][Algo].empty() ) {
        algoCur = myAvailableHypData[dim][Algo].at( currentHyp( dim, Algo ) );
      }
      HypothesisData* prevAlgo = 0;
      HypothesisData* nextAlgo = 0;
      if ( dim == SMESH::DIM_2D ) {
        prevAlgo = hypData( SMESH::DIM_1D, Algo, currentHyp( SMESH::DIM_1D, Algo ) );
        if ( aDim == SMESH::DIM_3D )
          nextAlgo = hypData( SMESH::DIM_3D, Algo, currentHyp( SMESH::DIM_3D, Algo ) );
      }
      // retrieves a list of available algorithms from resources
      availableHyps( dim, Algo, anAvailableAlgs, myAvailableHypData[dim][Algo], prevAlgo, nextAlgo, anCompareType);
      anCurrentAvailableAlgo = myAvailableHypData[dim][Algo].indexOf( algoCur );
      myDlg->tab( dim )->setAvailableHyps( Algo, anAvailableAlgs );
      setCurrentHyp( dim, Algo, anCurrentAvailableAlgo );
      if ( anCurrentAvailableAlgo > -1 )
        isReqDisBound = algoCur->InputTypes.isEmpty();
      if ( isReqDisBound ) {
        aReqDim = dim;
        break;
      }
    }
    if ( !myIsOnGeometry )
      for ( int i = SMESH::DIM_0D; i <= SMESH::DIM_3D; i++ ) {
        if ( i < SMESH::DIM_3D ) myDlg->disableTab( i );
        else                     myDlg->enableTab( i );
      }
    else
      for ( int i = SMESH::DIM_0D; i <= SMESH::DIM_3D; i++ ) {
        if ( i > myMaxShapeDim || ( isReqDisBound && i < aReqDim ) ) myDlg->disableTab( i );
        else                                                         myDlg->enableTab( i );
      }
    myDlg->setCurrentTab( theTabIndex );
  }
  else
  {
    HypothesisData* anCurrentAlgo = 0;
    bool isReqDisBound = true;
    QString anCurrentCompareType = anCompareType;
    isNone = currentHyp( aDim, Algo ) < 0;
    if ( !isNone && !myAvailableHypData[aDim][Algo].empty() )
      isReqDisBound = myAvailableHypData[aDim][Algo].at( currentHyp( aDim, Algo ) )->InputTypes.isEmpty();
    for ( int dim = aDim; dim >= SMESH::DIM_2D; dim-- )
    {
      bool isNoneAlg = currentHyp( dim, Algo ) < 0;
      anCurrentAvailableAlgo = -1;
      HypothesisData* prevAlgo = 0;
      HypothesisData* nextAlgo = 0;
      if ( dim == SMESH::DIM_2D ) {
        prevAlgo = hypData( SMESH::DIM_1D, Algo, currentHyp( SMESH::DIM_1D, Algo ) );
        if ( aDim == SMESH::DIM_3D )
          nextAlgo = hypData( SMESH::DIM_3D, Algo, currentHyp( SMESH::DIM_3D, Algo ) );
      }
      // finding algorithm which is selected
      if ( !isNoneAlg ) {
        anCurrentAlgo = myAvailableHypData[dim][Algo].at( currentHyp( dim, Algo ) );
      }
      // retrieves a list of available algorithms from resources
      availableHyps( dim, Algo, anAvailableAlgs, myAvailableHypData[dim][Algo], prevAlgo, nextAlgo, anCurrentCompareType );
      // finding and adding algorithm depending on the type mesh
      anCurrentAvailableAlgo = myAvailableHypData[dim][Algo].indexOf( anCurrentAlgo );
      //set new algorithm list and select the current algorithm
      myDlg->tab( dim )->setAvailableHyps( Algo, anAvailableAlgs );
      anCurrentCompareType = ( anCompareType == "HEXA" || anCompareType == "QUAD" ) ? "QUAD" : "TRIA";
      setCurrentHyp( dim, Algo, anCurrentAvailableAlgo, /*updateHyps=*/true );
    }

    for ( int i = myMaxShapeDim; i >= SMESH::DIM_0D; i-- ) {
      bool isNoneAlg = currentHyp( i, Algo ) < 0;
      if ( !isNoneAlg )
        isReqDisBound = myAvailableHypData[i][Algo].at( currentHyp( i, Algo ) )->InputTypes.isEmpty();
      else
        isReqDisBound = true;
      if ( isReqDisBound && !isNoneAlg && i <= aDim) {
        for (int j = myMaxShapeDim; j >= SMESH::DIM_0D; j--) {
          if ( currentHyp( j, Algo ) < 0 ) {
            myDlg->disableTab( j );
            setCurrentHyp( j , Algo, -1, /*updateHyps=*/true );
          }
        }
        break;
      }
      else {
        myDlg->enableTab( i );
      }
    }
    if ( aDim == SMESH::DIM_2D) {
      setCurrentHyp( SMESH::DIM_3D, Algo, -1, /*updateHyps=*/true );
      myDlg->disableTab( SMESH::DIM_3D );
    }

    int currentTab = ( theTabIndex <= aDim ) ? theTabIndex : aDim;
    myDlg->setCurrentTab( currentTab );
  }
  THypDataList anAvailableAlgsData;
  QStringList aHypothesesSetsList = SMESH::GetHypothesesSets( aDim );
  QStringList aFilteredHypothesesSetsList;
  aFilteredHypothesesSetsList.clear();
  QStringList::const_iterator inHypoSetName = aHypothesesSetsList.begin();
  for ( ; inHypoSetName != aHypothesesSetsList.end(); ++inHypoSetName ) {
    HypothesesSet* currentHypoSet = SMESH::GetHypothesesSet( *inHypoSetName );
    bool isAvailable = false;
    currentHypoSet->init( true );
    while ( currentHypoSet->next(), currentHypoSet->more() ) {
      isAvailable = false;
      if ( HypothesisData* algoDataIn = SMESH::GetHypothesisData( currentHypoSet->current() )) {
        for (int i = SMESH::DIM_0D; i <= myMaxShapeDim; i++) {
          int anCurrentAvailableAlgo = myAvailableHypData[i][Algo].indexOf( algoDataIn );
          if ( anCurrentAvailableAlgo > -1 ) {
            isAvailable = true;
            break;
          }
        }
        if ( !isAvailable )
          break;
      }
    }
    if ( isAvailable )
      aFilteredHypothesesSetsList.append( *inHypoSetName );
  }
  myDlg->setHypoSets( aFilteredHypothesesSetsList );
}

//================================================================================
/*!
 * \brief Get current name types of mesh
  * \param theIndex - current index types of mesh
  * \retval QString - result
 */
//================================================================================
QString SMESHGUI_MeshOp::currentMeshTypeName( const int theIndex ) const
{
  QString aMeshType = "";
  switch ( theIndex ) {
  case MT_ANY:
    aMeshType = "ANY";
    break;
  case MT_TRIANGULAR:
    aMeshType = "TRIA";
    break;
  case MT_QUADRILATERAL:
    aMeshType = "QUAD";
    break;
  case MT_TETRAHEDRAL:
    aMeshType = "TETRA";
    break;
  case MT_HEXAHEDRAL:
    aMeshType = "HEXA";
    break;
  default:;
  }
  return aMeshType;
}

