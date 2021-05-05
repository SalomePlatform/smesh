// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
#include "SMESHGUI_GroupOnShapeDlg.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_MeshDlg.h"
#include "SMESHGUI_Operations.h"
#include "SMESHGUI_ShapeByMeshDlg.h"
#include "SMESHGUI_Utils.h"
#include "SMESH_NumberFilter.hxx"
#include "SMESH_TypeDefs.hxx"
#include "SMESH_TypeFilter.hxx"

#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

// SALOME GEOM includes
#include <GEOMBase.h>
#include <GEOMImpl_Types.hxx>
#include <GEOM_Client.hxx>
#include <GEOM_SelectionFilter.h>
#include <GEOM_wrap.hxx>
#include <GeometryGUI.h>

// SALOME GUI includes
#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>
#include <LightApp_UpdateFlags.h>
#include <SALOME_InteractiveObject.hxx>
#include <SALOME_ListIO.hxx>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SalomeApp_Application.h>
#include <SalomeApp_Tools.h>

// SALOME KERNEL includes
#include <SALOMEDS_SComponent.hxx>
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDS_Study.hxx>
#include <SALOMEDS_wrap.hxx>

// Qt includes
#include <QApplication>
#include <QLineEdit>
#include <QStringList>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>

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
  myIsInvalidSubMesh( false )
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
  if (SMESHGUI::isStudyLocked())
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
    if (( myToCreate && !myIsMesh ) || myIsInvalidSubMesh )
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

  myHypoSet = 0;

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
      if ( myIsMesh ) myHelpFileName = "constructing_meshes.html";
      else            myHelpFileName = "constructing_submeshes.html";
    }
    else {
      myHelpFileName = "editing_meshes.html";
    }
  }
  SMESHGUI_SelectionOp::startOperation();

  // clear available hypotheses
  QStringList hypList;
  for ( int i = SMESH::DIM_0D; i <= SMESH::DIM_3D; i++ )
  {
    SMESHGUI_MeshTab* aTab = myDlg->tab( i );
    // set algos
    aTab->setAvailableHyps( Algo, hypList );
    aTab->setAvailableHyps( MainHyp, hypList );
    aTab->setAvailableHyps( AddHyp, hypList );
    aTab->setExistingHyps( MainHyp, hypList );
    aTab->setExistingHyps( AddHyp, hypList );
    myExistingHyps[ i ][ MainHyp ].clear();
    myExistingHyps[ i ][ AddHyp ].clear();
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

  updateMeshTypeList();
  myDlg->setCurrentMeshType( MT_ANY );

  myDlg->setCurrentTab( SMESH::DIM_3D );
  myDlg->setGeomPopupEnabled(false);
  myDlg->show();

  selectionDone();

  myHypoSet = 0;
  myHasConcurrentSubBefore = false;
  myObjectToSelect.clear();
}

//=================================================================================
/*!
 * \brief Selects a recently created mesh or sub-mesh if necessary
 *
 * Virtual method redefined from base class called when operation is committed
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
 *  \param theId - identifier of current selection widget
 *  \retval SUIT_SelectionFilter* - pointer to the created filter or null
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
 * \brief Return type of shape contained in a group
 */
//================================================================================

TopAbs_ShapeEnum getGroupType(const TopoDS_Shape& group)
{
  if ( group.ShapeType() != TopAbs_COMPOUND )
    return group.ShapeType();

  // iterate on a compound
  TopoDS_Iterator it( group );
  if ( it.More() )
    return getGroupType( it.Value() );

  return TopAbs_SHAPE;
}

//================================================================================
/*!
 * \brief check if selected shape is a sub-shape of the shape to mesh
 *  \retval bool - check result
 */
//================================================================================
bool SMESHGUI_MeshOp::isSubshapeOk() const
{
  if ( !myToCreate || myIsMesh ) // not submesh creation
    return false;

  // mesh
  QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() );
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
    GEOM::GEOM_Gen_var geomGen = mainGeom->GetGen();
    if (geomGen->_is_nil()) return false;

    GEOM::GEOM_IGroupOperations_wrap op = geomGen->GetIGroupOperations();
    if (op->_is_nil()) return false;

    // check all selected shapes
    QStringList::const_iterator aSubShapesIter = aGEOMs.begin();
    for ( ; aSubShapesIter != aGEOMs.end(); aSubShapesIter++)
    {
      QString aSubGeomEntry = (*aSubShapesIter);
      _PTR(SObject) pSubGeom = SMESH::getStudy()->FindObjectID(aSubGeomEntry.toUtf8().data());
      if (!pSubGeom) return false;

      GEOM::GEOM_Object_var aSubGeomVar =
        GEOM::GEOM_Object::_narrow(_CAST(SObject,pSubGeom)->GetObject());
      if (aSubGeomVar->_is_nil()) return false;

      // skl for NPAL14695 - implementation of searching of mainObj
      GEOM::GEOM_Object_var mainObj = op->GetMainShape(aSubGeomVar); /* _var not _wrap as
                                                                        mainObj already exists! */
      while( !mainObj->_is_nil())
      {
        CORBA::String_var entry1 = mainObj->GetEntry();
        CORBA::String_var entry2 = mainGeom->GetEntry();
        if (std::string( entry1.in() ) == entry2.in() )
          return true;
        mainObj = op->GetMainShape(mainObj);
      }
      if ( aSubGeomVar->GetShapeType() == GEOM::COMPOUND )
      {
        // is aSubGeomVar a compound of sub-shapes?
        GEOM::GEOM_IShapesOperations_wrap sop = geomGen->GetIShapesOperations();
        if (sop->_is_nil()) return false;
        GEOM::ListOfLong_var ids = sop->GetAllSubShapesIDs( aSubGeomVar,
                                                            GEOM::SHAPE,/*sorted=*/false);
        if ( ids->length() > 0 )
        {
          GEOM_Client geomClient;
          TopoDS_Shape  subShape = geomClient.GetShape( geomGen, aSubGeomVar );
          TopoDS_Shape mainShape = geomClient.GetShape( geomGen, mainGeom );
          if ( subShape.IsNull() || mainShape.IsNull() )
            return false;

          TopAbs_ShapeEnum subType = getGroupType( subShape );
          TopTools_IndexedMapOfShape subMap;
          TopExp::MapShapes( subShape, subType, subMap );
          for ( TopExp_Explorer exp( mainShape, subType ); exp.More(); exp.Next() )
            if ( subMap.Contains( exp.Current() ))
              return true;
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
    _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() );
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
          return CORBA::string_dup( algoNames[0].toUtf8().data() );
      }

//       GEOM::GEOM_Object_var geom;
//       if (_PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( aGeomEntry.toUtf8().data() ))
//         geom = SMESH::SObjectToInterface<GEOM::GEOM_Object>( pGeom );

//       if ( !geom->_is_nil() && geom->GetShapeType() >= GEOM::FACE ) { // WIRE, EDGE as well
        existingHyps(2, Algo, pMesh, algoNames, algoList);
        if (!algoList.empty()) {
          HypothesisData* algo =
            SMESH::GetHypothesisData( SMESH::toQStr( algoList[0].first->GetName() ));
          if ( algo &&
               algo->InputTypes.empty() && // builds all dimensions it-self
               !algo->IsSupportSubmeshes )
            return CORBA::string_dup( algoNames[0].toUtf8().data() );
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
  QString  aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() );
  if ( pMesh && !myGeom->_is_nil() ) {
    int tag = -1;
    switch ( myGeom->GetShapeType() ) {
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
    _PTR(Study) aStudy = SMESH::getStudy();
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
               myGeomEntry == pGeom2->GetID().c_str() )
            return aSmObj;
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

  myGeomEntry.clear();
  myGeom = GEOM::GEOM_Object::_nil();
  myIsInvalidSubMesh = false;
  myIsOnGeometry = true;

  try
  {
    // allow selecting multiple geom objects only for sub-mesh creation
    QStringList aGEOMs;
    myDlg->selectedObject( SMESHGUI_MeshDlg::Geom, aGEOMs );
    int nbGeoms = aGEOMs.count();
    if ( nbGeoms > 0 )
    {
      myGeomEntry = aGEOMs[0];
      _PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( myGeomEntry.toUtf8().data() );
      if ( nbGeoms > 1 && myToCreate && myIsMesh )
      {
        selectObject( pGeom );
        selectionDone();
        return;
      }
      myGeom = SMESH::SObjectToInterface< GEOM::GEOM_Object >( pGeom ); // creation case
    }

    QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
    _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );

    SMESH::SMESH_Mesh_var    mesh = SMESH::SObjectToInterface< SMESH::SMESH_Mesh >( pObj );
    SMESH::SMESH_subMesh_var subMesh;

    if ( !myToCreate ) // edition: read hypotheses
    {
      if ( pObj )
      {
        subMesh  = SMESH::SObjectToInterface< SMESH::SMESH_subMesh >( pObj );
        myIsMesh = subMesh->_is_nil();
        myGeom   = SMESH::GetShapeOnMeshOrSubMesh( pObj );
        myIsInvalidSubMesh = ( !myIsMesh && subMesh->GetId() < 1 );
        if ( !mesh->_is_nil() && !mesh->HasShapeToMesh() )
        {
          myIsOnGeometry = false;
          myGeom = GEOM::GEOM_Object::_nil();
        }
        if ( !myGeom->_is_nil() )
          myGeomEntry = myGeom->GetStudyEntry();

        myDlg->setTitile( myToCreate, myIsMesh );
        myDlg->setObjectShown( SMESHGUI_MeshDlg::Mesh, !subMesh->_is_nil() );
        myDlg->setObjectShown( SMESHGUI_MeshDlg::Geom, !myGeom->_is_nil() );
        myDlg->objectWg( SMESHGUI_MeshDlg::Mesh, SMESHGUI_MeshDlg::Btn )->hide();
        if ( myIsInvalidSubMesh )
        {
          // it is necessary to select a new geometry
          myDlg->objectWg( SMESHGUI_MeshDlg::Geom, SMESHGUI_MeshDlg::Btn )->show();
          myDlg->activateObject( SMESHGUI_MeshDlg::Geom );
        }
        else
        {
          myDlg->objectWg( SMESHGUI_MeshDlg::Geom, SMESHGUI_MeshDlg::Btn )->hide();
        }
        myDlg->updateGeometry();
        myDlg->adjustSize();
        readMesh();
      }
      else
        myDlg->reset();
    }
    else if ( !myIsMesh ) // sub-mesh creation
    {
      // if a submesh on the selected shape already exist, pass to submesh edition mode
      if ( _PTR(SObject) pSubmesh = getSubmeshByGeom() ) {
        subMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( pSubmesh );
        bool editSubmesh = ( !subMesh->_is_nil() &&
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
      if ( _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toLatin1().data() ))
      {
        mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );
        if ( !mesh->_is_nil() )
          enable = /*( shapeDim > 1 ) && */( mesh->NbEdges() > 0 );
      }
      myDlg->setGeomPopupEnabled( enable );

      //rnv: issue 21056: "Create Sub Mesh": focus should automatically switch to geometry
      if ( pObj && myGeomEntry.isEmpty() )
        myDlg->activateObject( SMESHGUI_MeshDlg::Geom );
    }

    setFilteredAlgoData();

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
    if ( _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() )) {
      SMESH::SMESH_Mesh_var mesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pMesh );
      if ( !mesh->_is_nil() && CORBA::is_nil( mesh->GetShapeToMesh() )) {
        theMess = tr( "IMPORTED_MESH" );
        return false;
      }
    }
  }*/

  // Geom
  if ( myToCreate || myIsInvalidSubMesh )
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
    _PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( aGeomEntry.toUtf8().data() );
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
      _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() );
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
 *  \param theAlgoData - algorithm data
 *  \param theHypData - hypothesis data
 *  \param theHypType - hypothesis type
 *  \retval bool - check result
 */
//================================================================================
static bool isCompatible(const HypothesisData* theAlgoData,
                         const HypothesisData* theHypData,
                         const int             theHypType)
{
  if ( !theAlgoData )
    return true;

  if ( theHypType == SMESHGUI_MeshOp::Algo ) // compatibility of 2 algos
    return SMESH::IsCompatibleAlgorithm( theAlgoData, theHypData );

  bool isOptional; // compatibility of algo and hyp
  return ( SMESH::IsAvailableHypothesis( theAlgoData, theHypData->TypeName, isOptional ));
}

//================================================================================
/*!
 * \brief check compatibility of the geometry
 * \param theAlgoData - to select hypos able to be used by this algo
 * \param theDim - algo dimension
 * \retval bool - check result
 */
//================================================================================
bool SMESHGUI_MeshOp::isCompatibleToGeometry(HypothesisData* theAlgoData,
                                             const int       theDim)
{
  if ( theDim < 1 || myGeom->_is_nil() )
    return true;

  bool isApplicable = false;
  if ( myGeomEntry == myLastGeomEntry && !myGeomEntry.isEmpty() ) {
    THypLabelIsAppMap::const_iterator lab2isApp = myHypMapIsApplicable.find( theAlgoData->TypeName );
    if ( lab2isApp != myHypMapIsApplicable.end() ) {
      isApplicable = lab2isApp.value();
      return isApplicable;
    }
  }
  bool toCheckIsApplicableToAll = !myIsMesh;
  if ( toCheckIsApplicableToAll )
    toCheckIsApplicableToAll = ( myGeom->GetType() == GEOM_GROUP );

  isApplicable = SMESH::IsApplicable( theAlgoData->TypeName, myGeom, toCheckIsApplicableToAll );
  myHypMapIsApplicable.insert( theAlgoData->TypeName, isApplicable );
  return isApplicable;
}

//================================================================================
/*!
 * \brief check compatibility of the mesh type
 *  \param theAlgoData - to select hypos able to be used by this algo
 *  \param theDim - algo dimension
 *  \retval bool - check result
 */
//================================================================================
bool SMESHGUI_MeshOp::isCompatibleToMeshType(HypothesisData* theAlgoData,
                                             const int       theDim)
{
  if ( theDim < 2 )
    return true;

  QString elemType;
  int curMeshType = myDlg->currentMeshType();
  switch ( curMeshType ) {
  case MT_ANY:
    return true;

  case MT_TRIANGULAR:
    if ( theDim > 2 )
      return false;
    elemType = "TRIA";
    break;

  case MT_QUADRILATERAL:
    if ( theDim > 2 )
      return false;
    elemType = "QUAD";
    break;

  case MT_TETRAHEDRAL:
    if ( theDim < 3 )
      elemType = "TRIA";
    else
      elemType = "TETRA";
    break;

  case MT_HEXAHEDRAL:
    if ( theDim < 3 )
      elemType = "QUAD";
    else
      elemType = "HEXA";
    break;

  default:
    return true;
  }

  bool isAvailableAlgo = theAlgoData->OutputTypes.isEmpty();
  QStringList::const_iterator inElemType = theAlgoData->OutputTypes.begin();
  for ( ; inElemType != theAlgoData->OutputTypes.end(); inElemType++ )
  {
    if ( *inElemType == elemType )
    {
      isAvailableAlgo = true;
      break;
    }
  }
  return isAvailableAlgo;
}

//================================================================================
/*!
 * \brief Get available hypotheses or algorithms
 *  \param theDim - specifies dimension of returned hypotheses/algorifms
 *  \param theHypType - specifies whether algorims or hypotheses or additional ones
 *         are retrieved (possible values are in HypType enumeration)
 *  \param theHyps - output list of hypotheses' names
 *  \param thePrevAlgoData - to select hypos able to be used by previously algo (optional)
 *  \param theNextAlgoData - to select hypos able to be used by next algo (optional)
 *  \param theMeshType - type of mesh for filtering algorithms (optional)
 *
 * Get available hypotheses or algorithm in accordance with input parameters.
 * Result (\a theHyps) optionally depends on the following:
 * - selected algorithm(s)
 * - selected geometry
 * - selected mesh type
 */
//================================================================================
void SMESHGUI_MeshOp::availableHyps( const int       theDim,
                                     const int       theHypType,
                                     QStringList&    theHyps,
                                     THypDataList&   theDataList,
                                     HypothesisData* thePrevAlgoData,
                                     HypothesisData* theNextAlgoData,
                                     const QString&  /*theMeshType*/)
{
  theDataList.clear();
  theHyps.clear();
  bool isAlgo = ( theHypType == Algo );
  bool isAux  = ( theHypType >= AddHyp );
  QStringList aHypTypeNameList = SMESH::GetAvailableHypotheses( isAlgo, theDim, isAux, myIsOnGeometry, !myIsMesh );

  std::multimap< double, HypothesisData* > sortedHyps;
  QStringList::const_iterator anIter;
  for ( anIter = aHypTypeNameList.begin(); anIter != aHypTypeNameList.end(); ++anIter )
  {
    HypothesisData* aData = SMESH::GetHypothesisData( *anIter );
    if ( isCompatible( thePrevAlgoData, aData, theHypType ) &&
         isCompatible( theNextAlgoData, aData, theHypType ) &&
         isCompatibleToGeometry( aData, theDim )            &&
         isCompatibleToMeshType( aData, theDim ))
    {
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
}

//================================================================================
/*!
 * \brief Gets existing hypotheses or algorithms
 *  \param theDim - specifies dimension of returned hypotheses/algorifms
 *  \param theHypType - specifies whether algorims or hypotheses or additional ones
 *         are retrieved (possible values are in HypType enumeration)
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
    _PTR(ChildIterator) anIter = SMESH::getStudy()->NewChildIterator( aHypRoot );
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
SMESHGUI_MeshOp::getInitParamsHypothesis( const QString&              aHypType,
                                          const QString&              aServerLib,
                                          const SMESH::HypInitParams* aParams ) const
{
  if ( aHypType.isEmpty() || aServerLib.isEmpty() )
    return SMESH::SMESH_Hypothesis::_nil();

  const int nbColonsInMeshEntry = 3;
  bool isSubMesh = myToCreate ?
    !myIsMesh :
    myDlg->selectedObject( SMESHGUI_MeshDlg::Obj ).count(':') > nbColonsInMeshEntry;

  // get mesh and geom object
  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_nil();
  GEOM::GEOM_Object_var aGeomVar = GEOM::GEOM_Object::_nil();

  QString anEntry;
  if ( isSubMesh )
  {
    anEntry = myDlg->selectedObject
      ( myToCreate ? SMESHGUI_MeshDlg::Mesh : SMESHGUI_MeshDlg::Obj );
    if ( _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anEntry.toUtf8().data() ))
    {
      CORBA::Object_ptr Obj = _CAST( SObject,pObj )->GetObject();
      if ( myToCreate ) // mesh and geom may be selected
      {
        aMesh = SMESH::SMESH_Mesh::_narrow( Obj );
        anEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
        if ( _PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( anEntry.toUtf8().data() ))
          aGeomVar= GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
      }
      else // edition: sub-mesh may be selected
      {
        SMESH::SMESH_subMesh_var sm = SMESH::SMESH_subMesh::_narrow( Obj );
        if ( !sm->_is_nil() ) {
          aMesh = sm->GetFather();
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
      if ( _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( anEntry.toUtf8().data() ))
      {
        aMesh = SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() );
        if ( !aMesh->_is_nil() )
          aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pMesh );
      }
    }
    if ( aGeomVar->_is_nil() ) {
      anEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
      if ( _PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( anEntry.toUtf8().data() ))
      {
        aGeomVar= GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
      }
    }
  }

  SMESH::HypInitParams initParams;
  enum { BY_MESH, BY_GEOM, BY_AVERAGE_LENGTH }; // same as ::SMESH_Hypothesis::InitWay
  if ( aParams )
  {
    initParams = *aParams;
  }
  else
  {
    initParams.way = isSubMesh ? BY_MESH : BY_GEOM;
  }

  SMESH::SMESH_Hypothesis_var hyp;
  if ( initParams.way == BY_AVERAGE_LENGTH )
    hyp = SMESHGUI::GetSMESHGen()->CreateHypothesisByAverageLength( aHypType.toUtf8().data(),
                                                                    aServerLib.toUtf8().data(),
                                                                    initParams.averageLength,
                                                                    initParams.quadDominated );
  else
    hyp = SMESHGUI::GetSMESHGen()->GetHypothesisParameterValues( aHypType.toUtf8().data(),
                                                                 aServerLib.toUtf8().data(),
                                                                 aMesh,
                                                                 aGeomVar,
                                                                 initParams );
  if ( hyp->_is_nil() && initParams.way == BY_MESH )
  {
    initParams.way = BY_GEOM;
    hyp = SMESHGUI::GetSMESHGen()->GetHypothesisParameterValues( aHypType.toLatin1().data(),
                                                                 aServerLib.toUtf8().data(),
                                                                 aMesh,
                                                                 aGeomVar,
                                                                 initParams );
  }
  if ( hyp->_is_nil() && initParams.way == BY_GEOM )
  {
    initParams.way = BY_AVERAGE_LENGTH;
    initParams.averageLength = 1.;
    hyp = SMESHGUI::GetSMESHGen()->GetHypothesisParameterValues( aHypType.toLatin1().data(),
                                                                 aServerLib.toUtf8().data(),
                                                                 aMesh,
                                                                 aGeomVar,
                                                                 initParams );
  }
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
    _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() );
    GEOM::GEOM_Object_var aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pObj );
    aMeshEntry = ( aGeomVar->_is_nil() ) ? QString() : SMESH::toQStr( aGeomVar->GetStudyEntry() );
  }

  if ( aMeshEntry == "" && aGeomEntry == "" ) { // get geom of an object being edited
    _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );
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
    _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );
    if ( pObj ) {
      // if current object is sub-mesh
      SMESH::SMESH_subMesh_var aSubMeshVar =
        SMESH::SMESH_subMesh::_narrow( _CAST( SObject,pObj )->GetObject() );
      if ( !aSubMeshVar->_is_nil() ) {
        SMESH::SMESH_Mesh_var aMesh =  aSubMeshVar->GetFather();
        if ( !aMesh->_is_nil() ) {
          _PTR(SObject) aMeshSO = SMESH::FindSObject( aMesh );
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
 * \Brief Returns tab dimension
  * \param tab - the tab in the dlg
  * \param dlg - my dialogue
  * \retval int - dimension
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
  * \param theIndex - index of type of hypothesis to be created
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
 * \brief Return names of all existing hypotheses
 */
//================================================================================

QStringList SMESHGUI_MeshOp::getHypoNames()
{
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
  return aHypNames;
}

//================================================================================
/*!
 *  Create hypothesis and update dialog.
 *  \param theDim - dimension of hypothesis to be created
 *  \param theType - hypothesis category (algorithm, hypothesis, additional hypothesis)
 *  \param theTypeName - specifies hypothesis to be created
 */
//================================================================================
void SMESHGUI_MeshOp::createHypothesis(const int      theDim,
                                       const int      theType,
                                       const QString& theTypeName)
{
  HypothesisData* aData = SMESH::GetHypothesisData(theTypeName);
  if (!aData)
    return;

  myDim = theDim;
  myType = theType;

  // get a unique hyp name
  QString aHypName = GetUniqueName( getHypoNames(), aData->Label);

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

  _PTR(SComponent) aFather = SMESH::getStudy()->FindComponent("SMESH");

  int nbHyp = myExistingHyps[myDim][myType].count();
  HypothesisData* algoData = hypData( myDim, Algo, currentHyp( myDim, Algo ));
  QStringList aNewHyps;
  existingHyps(myDim, myType, aFather, aNewHyps, myExistingHyps[myDim][myType], algoData);
  if (aNewHyps.count() > nbHyp)
  {
    for (int i = nbHyp; i < aNewHyps.count(); i++)
      myDlg->tab(myDim)->addHyp(myType, aNewHyps[i]);
  }
  return;
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
void SMESHGUI_MeshOp::onHypoEdited( int /*result*/ )
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
 * \brief Update list of algorithms and hypotheses according to currently selected
 *        algorithm, geometry and mesh type
 */
//================================================================================
void SMESHGUI_MeshOp::setFilteredAlgoData()
{
  // keep current algorithms
  HypothesisData* curAlgos[ 4 ];
  for ( int dim = 0; dim <= 3; ++dim )
    curAlgos[ dim ] = hypData( dim, Algo, currentHyp( dim, Algo ));

  // find out myMaxShapeDim by checking algorithm applicability to geometry

  int curMeshType = myDlg->currentMeshType();
  myDlg->setCurrentMeshType( MT_ANY );
  QStringList algosAvailable;
  // get available algorithms taking into account geometry only
  if ( myGeom->_is_nil() )
  {
    myMaxShapeDim = 3;

    if ( myToCreate ) // readMesh() has done it if !myToCreate
      for ( int dim = 0; dim <= 3; ++dim )
        availableHyps( dim, Algo, algosAvailable, myAvailableHypData[ dim ][ Algo ] );
  }
  else
  {
    if ( myGeomEntry != myLastGeomEntry )
      myHypMapIsApplicable.clear();

    for ( int dim = 0; dim <= 3; ++dim )
    {
      availableHyps( dim, Algo, algosAvailable, myAvailableHypData[ dim ][ Algo ] );
      if ( algosAvailable.count() )
        myMaxShapeDim = dim;
    }
    if ( !myGeomEntry.isEmpty() )
      myLastGeomEntry = myGeomEntry;
  }
  myDlg->setMaxHypoDim( myMaxShapeDim );

  // set mesh types according to myMaxShapeDim
  updateMeshTypeList();

  // update available hypo-sets that depend on geometry and mesh type
  myDlg->setCurrentMeshType( Max( MT_ANY, curMeshType ));
  updateHypoSets(); // it sets myAvailableHypData by curMeshType

  // restore current algorithms according to changed myAvailableHypData
  algosAvailable.clear();
  for ( int dim = 0; dim <= 3; ++dim )
  {
    int algoIndex = myAvailableHypData[ dim ][ Algo ].indexOf( curAlgos[ dim ]);
    while ( algosAvailable.count() <= algoIndex )
      algosAvailable << "";
    myDlg->tab( dim )->setAvailableHyps( Algo, algosAvailable );
    setCurrentHyp( dim, Algo, algoIndex );
    if ( algoIndex < 0 )
      curAlgos[ dim ] = 0;
  }

  // find a selected algo, current or of highest dimension
  int algoDim = myDlg->currentTab();
  if ( !curAlgos[ algoDim ])
    for ( algoDim = SMESH::DIM_3D; algoDim >= SMESH::DIM_0D; algoDim-- )
      if ( curAlgos[ algoDim ] )
        break;
  if ( algoDim < SMESH::DIM_0D )
    algoDim = myMaxShapeDim;

  // set algorithms and hypotheses according to all parameters (mesh type etc)
  onAlgoSelected( currentHyp( algoDim, Algo ), algoDim );
}

//================================================================================
/*!
 * \brief Set available algos and hypos according to the selected algorithm
 * \param theIndex - algorithm index
 * \param theDim - algorithm dimension
 */
//================================================================================
void SMESHGUI_MeshOp::onAlgoSelected( const int theIndex,
                                      const int theDim )
{
  if ( myIgnoreAlgoSelection )
    return;

  int curDim = getTabDim( sender(), myDlg );
  int aDim = theDim < 0 ? curDim : theDim;
  if (aDim == -1)
    return;

  const bool isSubmesh = ( myToCreate ? !myIsMesh : myDlg->isObjectShown( SMESHGUI_MeshDlg::Mesh ));

  HypothesisData* algoData = hypData( aDim, Algo, theIndex );
  HypothesisData* algoByDim[4];
  algoByDim[ aDim ] = algoData;

  QStringList anAvailable;
  const int algoDim = aDim;

  // 2 loops: backward and forward from algo dimension (forward == to higher dimension)
  for ( int forward = 1; forward >= 0; --forward )
  {
    int dim = algoDim, lastDim = SMESH::DIM_3D, dir = 1;
    if ( !forward ) {
      dim = algoDim - 1; lastDim = SMESH::DIM_0D; dir = -1;
    }
    //bool noCompatible = false;
    for ( ; dim * dir <= lastDim * dir; dim += dir)
    {
      // if ( noCompatible ) // the selected algo has no compatible ones (like 1D2D3D algo)
      // {
      //   anAvailable.clear();
      //   myDlg->tab( dim )->setAvailableHyps( Algo, anAvailable );
      //   myAvailableHypData[dim][Algo].clear();
      //   algoByDim[ dim ] = 0;
      // }
      // else
      {
        int           algoIndex = currentHyp( dim, Algo );
        HypothesisData *curAlgo = hypData( dim, Algo, algoIndex );

        // set new available algorithms
        HypothesisData *prevAlgo = 0;
        if ( dim != algoDim )
          for ( int prevDim = dim + 1; prevDim <=3 && !prevAlgo; ++prevDim )
            prevAlgo = hypData( prevDim, Algo, currentHyp( prevDim, Algo ));
        availableHyps( dim, Algo, anAvailable, myAvailableHypData[dim][Algo], prevAlgo );
        myDlg->tab( dim )->setAvailableHyps( Algo, anAvailable );

        //noCompatible = ( prevAlgo && !forward && anAvailable.isEmpty() );

        // set current algo
        algoIndex = myAvailableHypData[dim][Algo].indexOf( curAlgo );
        setCurrentHyp( dim, Algo, algoIndex, /*updateHyps=*/true );

        algoByDim[ dim ] = hypData( dim, Algo, algoIndex );
      }

      // activate tabs according to algorithms availability
      if ( anAvailable.isEmpty() ) myDlg->disableTab( dim );
      else                         myDlg->enableTab( dim );

    } // loop on dims
  } // loops backward and forward


  // set hypotheses corresponding to the found algorithms

  _PTR(SObject) pComp = SMESH::getStudy()->FindComponent("SMESH");

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
      QString                     curHypType;
      HypothesisData*             hypData = 0;
      if ( hypIndex >= 0 && hypIndex < myExistingHyps[ dim ][ type ].count() )
      {
        curHyp = myExistingHyps[ dim ][ type ][ hypIndex ].first;
        if ( !curHyp->_is_nil() )
        {
          curHypType = SMESH::toQStr( curHyp->GetName() );
          hypData = SMESH::GetHypothesisData( curHypType );
        }
      }
      if ( !myToCreate && !curAlgo && !curHyp->_is_nil() ) // edition, algo not selected
      {
        // try to find algo by selected hypothesis in order to keep it selected
        bool algoDeselectedByUser = ( theDim < 0 && aDim == dim );
        if ( !algoDeselectedByUser &&
             myObjHyps[ dim ][ type ].count() > 0 &&
             curHypType == SMESH::toQStr( myObjHyps[ dim ][ type ].first().first->GetName()) )
        {
          for ( int i = 0; i < myAvailableHypData[ dim ][ Algo ].count(); ++i )
          {
            curAlgo = myAvailableHypData[ dim ][ Algo ][ i ];
            if ( curAlgo && hypData && isCompatible( curAlgo, hypData, type ))
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
        if ( hypData && !isCompatible( curAlgo, hypData, type ))
          curHyp = SMESH::SMESH_Hypothesis::_nil();
        availableHyps( dim, type, anAvailable, myAvailableHypData[ dim ][ type ], curAlgo );
        existingHyps ( dim, type, pComp, anExisting, myExistingHyps[ dim ][ type ], curAlgo );
        defaulHypAvlbl = ( type == MainHyp && !curAlgo->IsAuxOrNeedHyp );
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
      if ( !isSubmesh && myToCreate && hypIndex < 0 && anExisting.count() == 1 && dim == curDim )
      {
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
  return;
}

//================================================================================
/*!
 * \brief Create a sub-mesh on internal edges in the case where the global algorithm
 *        is of type 1D-2D[-3D] so that the internal edges would remain not meshed.
 */
//================================================================================

void SMESHGUI_MeshOp::createSubMeshOnInternalEdges( SMESH::SMESH_Mesh_ptr theMesh,
                                                    GEOM::GEOM_Object_ptr theMainShape )
{
  if ( theMesh->_is_nil() || theMainShape->_is_nil() )
    return;

  if ( isAccessibleDim( 1 ))
    return; // global 1D algorithm is/can be assigned

  const HypothesesSet::SetType internSet = HypothesesSet::INTERN;
  bool toCreate = true;
  bool toCreateMandatory = ( myHypoSet &&
                             myAverageSize > 0 &&
                             ( myHypoSet->init( /*algo=*/ true, internSet ), myHypoSet->more()) &&
                             ( myHypoSet->init( /*algo=*/false, internSet ), myHypoSet->more()));
  if ( !toCreateMandatory ) // ask the user
    toCreate = false; // can't pass both mesh and geometry to Create Submesh operation (so far?)
    // toCreate = SUIT_MessageBox::warning( SMESHGUI::desktop(),
    //                                      QObject::tr("SMESH_WRN_WARNING"),
    //                                      QObject::tr("SMESH_CREATE_SUBMESH_ON_INTERNAL_EDGES"),
    //                                      QObject::tr("SMESH_BUT_YES"),
    //                                      QObject::tr("SMESH_BUT_NO"), 1, 0);
  if ( !toCreate )
    return;

  TopoDS_Shape shape;
  if ( !GEOMBase::GetShape( theMainShape, shape ))
    return;

  std::vector< TopoDS_Shape > internalEdges;
  for ( TopExp_Explorer edge( shape, TopAbs_EDGE, TopAbs_WIRE ); edge.More(); edge.Next() )
    internalEdges.push_back( edge.Current() );

  if ( internalEdges.empty() )
    return;

  TopTools_IndexedMapOfShape shapeIDs;
  TopExp::MapShapes( shape, shapeIDs );

  std::set< int > intIDSet;
  for ( size_t i = 0; i < internalEdges.size(); ++i )
    intIDSet.insert( shapeIDs.FindIndex( internalEdges[ i ]));

  GEOM::GEOM_Gen_var geomGen = theMainShape->GetGen();
  if (geomGen->_is_nil()) return;

  GEOM::GEOM_Object_var edgeGroup;
  GEOM::GEOM_IShapesOperations_wrap sOp = geomGen->GetIShapesOperations();
  GEOM::GEOM_IGroupOperations_wrap  gOp = geomGen->GetIGroupOperations();
  GEOM::ListOfGO_var         geomGroups = sOp->GetExistingSubObjects( theMainShape,
                                                                      /*groupsOnly=*/true );
  for ( CORBA::ULong i = 0; i < geomGroups->length(); ++i )
  {
    GEOM::ListOfLong_var ids = gOp->GetObjects( geomGroups[ i ]);
    std::set< int > idSet( & ids[0], & ids[0] + ids->length() );
    if ( idSet == intIDSet )
    {
      edgeGroup = geomGroups[ i ];
      break;
    }
  }

  if ( edgeGroup->_is_nil() )
  {
    GEOM::GEOM_Object_var edgeGroup = gOp->CreateGroup( theMainShape, TopAbs_EDGE );

    GEOM::ListOfLong_var edgeIDs = new GEOM::ListOfLong;
    edgeIDs->length( internalEdges.size() );
    std::set< int >::iterator id = intIDSet.begin();
    for ( size_t i = 0; i < intIDSet.size(); ++i, ++id )
      edgeIDs[ i ] = *id;
    gOp->UnionIDs( edgeGroup, edgeIDs );

    SALOMEDS::SObject_wrap so = geomGen->AddInStudy( edgeGroup, "Internal edges", theMainShape );
  }

  if ( !toCreateMandatory )
  {
    // show Create Sub-mesh dislog
    // _PTR(SObject) aMeshSO = SMESH::FindSObject( theMesh );
    // selectionMgr()->clearFilters();
    // selectObject( pSubmesh );
    // SMESHGUI::GetSMESHGUI()->switchToOperation( SMESHOp::OpEditMeshOrSubMesh );
    return;
  }

  // create a sub-mesh using myAverageSize w/o GUI

  SMESH::SMESH_subMesh_var subMesh = theMesh->GetSubMesh( edgeGroup, "" );

  for ( int isAlgo = 1; isAlgo >= 0; --isAlgo )
    for ( myHypoSet->init( isAlgo, internSet ); myHypoSet->more(); myHypoSet->next() )
    {
      QString    aHypoTypeName = myHypoSet->current();
      HypothesisData* aHypData = SMESH::GetHypothesisData( aHypoTypeName );
      if ( !aHypData )
        continue;

      myDim = aHypData->Dim[0];
      if ( myDim != 1 )
        continue;

      // create or/and set
      SMESH::SMESH_Hypothesis_var newHypo;
      if ( isAlgo )
      {
        myAvailableHypData[ myDim ][ Algo ].clear();
        myAvailableHypData[ myDim ][ Algo ] << aHypData;
        QStringList hypList; hypList << aHypoTypeName;
        myDlg->tab( myDim )->setAvailableHyps( Algo, hypList );
        setCurrentHyp( myDim, Algo, 0 );
        newHypo = getAlgo( myDim );
      }
      else
      {
        SMESH::HypInitParams params = { 2, myAverageSize, false };
        newHypo = getInitParamsHypothesis( aHypData->TypeName, aHypData->ServerLibName, & params );
        QString hypName = GetUniqueName( getHypoNames(), aHypData->Label );
        SALOMEDS::SObject_wrap so =
          SMESHGUI::GetSMESHGen()->PublishInStudy( SALOMEDS::SObject::_nil(), newHypo,
                                                   hypName.toUtf8().data() );
      }
      SMESH::AddHypothesisOnSubMesh( subMesh, newHypo );
    }

  return;
}

//================================================================================
/*!
 * \brief Ask the user to enter an average size which will be used to create
 *        hypotheses of a hypo-set basing on this size
 *  \param [out] averageSize - average element size
 *  \return bool - false if the user canceled the dialog
 */
//================================================================================

bool SMESHGUI_MeshOp::getAverageSize( double & averageSize )
{
  HypothesisData* hypData = SMESH::GetHypothesisData( "MaxLength" );
  if ( !hypData )
    return false;

  SMESH::SMESH_Hypothesis_var hyp = getInitParamsHypothesis( hypData->TypeName,
                                                             hypData->ServerLibName );
  if ( hyp->_is_nil() )
    return false;

  SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator( hypData->TypeName );
  if ( !aCreator )
    return false;
  
  initHypCreator( aCreator );
  myDlg->setEnabled( false );

  aCreator->edit( hyp.in(), HypothesesSet::getCommonHypoSetHypoType(),
                  dlg(), this, SLOT( onHypoEdited( int )));

  StdMeshers::StdMeshers_MaxLength_var lenHyp = StdMeshers::StdMeshers_MaxLength::_narrow( hyp );
  if ( lenHyp->_is_nil() )
    return false;

  averageSize = lenHyp->GetLength();
  return true;
}

//================================================================================
/*!
 * \brief Creates and selects hypothesis of hypotheses set
 * \param theSetName - The name of hypotheses set
 */
//================================================================================
void SMESHGUI_MeshOp::onHypoSet( const QString& theSetName )
{
  HypothesesSet* aHypoSet = SMESH::GetHypothesesSet(theSetName);
  myHypoSet = aHypoSet;
  if (!aHypoSet)
    return;

  // clear all hyps
  for (int dim = SMESH::DIM_0D; dim <= SMESH::DIM_3D; dim++)
  {
    setCurrentHyp(dim, Algo, -1);
    setCurrentHyp(dim, AddHyp, -1);
    setCurrentHyp(dim, MainHyp, -1);
    onAlgoSelected( -1, dim );
  }

  HypothesesSet::SetType setType = aHypoSet->getPreferredHypType();
  if ( !aHypoSet->getAlgoAvailable( setType ))
  {
    setType = ( setType == HypothesesSet::ALT ) ? HypothesesSet::MAIN : HypothesesSet::ALT;
    if ( !aHypoSet->getAlgoAvailable( setType ))
      return;
  }

  myAverageSize = -1;
  if ( aHypoSet->toUseCommonSize() && !getAverageSize( myAverageSize ))
    return;

  int maxDim = -1;
  for ( int isAlgo = 1; isAlgo >= 0; --isAlgo )
    for ( aHypoSet->init( isAlgo, setType ); aHypoSet->more(); aHypoSet->next() )
    {
      QString    aHypoTypeName = aHypoSet->current();
      HypothesisData* aHypData = SMESH::GetHypothesisData( aHypoTypeName );
      if (!aHypData)
        continue;

      myDim = aHypData->Dim[0];
      // create or/and set
      if ( isAlgo )
      {
        int index = myAvailableHypData[myDim][Algo].indexOf( aHypData );
        if ( index >= 0 )
        {
          setCurrentHyp( myDim, Algo, index );
          onAlgoSelected( index, myDim );
          maxDim = Max( maxDim, myDim );
        }
      }
      else
      {
        myType = MainHyp;
        int index = myAvailableHypData[myDim][MainHyp].indexOf( aHypData );
        if ( index < 0 )
        {
          myType = AddHyp;
          index = myAvailableHypData[myDim][AddHyp].indexOf( aHypData );
        }
        if ( index >= 0 )
        {
          if ( myAverageSize > 0 )
          {
            SMESH::HypInitParams params = { 2, myAverageSize, aHypoSet->isQuadDominated() };
            SMESH::SMESH_Hypothesis_var hyp =
              getInitParamsHypothesis( aHypData->TypeName, aHypData->ServerLibName, & params );

            QString hypName = GetUniqueName( getHypoNames(), aHypData->Label );
            SALOMEDS::SObject_wrap so =
              SMESHGUI::GetSMESHGen()->PublishInStudy( SALOMEDS::SObject::_nil(), hyp,
                                                       hypName.toUtf8().data() );
            onHypoCreated(2);
          }
          else
          {
            createHypothesis( myDim, myType, aHypoTypeName );
          }
        }
      }
    }

  if ( maxDim > 0 )
    myDlg->setCurrentTab( maxDim );

  return;
}

//================================================================================
/*!
 * \brief Creates mesh
 *  \param theMess - Output parameter intended for returning error message
 *  \param theEntryList - List of entries of published objects
 *  \retval bool  - TRUE if mesh is created, FALSE otherwise
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

    SMESH::SMESH_Mesh_var aMesh= aSMESHGen->CreateEmptyMesh();
    if ( aMesh->_is_nil() )
      return false;

    _PTR(SObject) aMeshSO = SMESH::FindSObject( aMesh.in() );
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
  for ( int i = 0; it != aList.end(); it++, ++i )
  {
    QString aGeomEntry = *it;
    _PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( aGeomEntry.toUtf8().data() );
    GEOM::GEOM_Object_var aGeomVar =
      GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );

    SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
    if ( aSMESHGen->_is_nil() )
      return false;

    SUIT_OverrideCursor aWaitCursor;

    // create mesh
    SMESH::SMESH_Mesh_var aMesh = aSMESHGen->CreateMesh( aGeomVar );
    if ( aMesh->_is_nil() )
      return false;
    _PTR(SObject) aMeshSO = SMESH::FindSObject( aMesh.in() );
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
            SMESH::AddHypothesisOnMesh( aMesh, aHypVar );
        }
      }
      // find or create algorithm
      SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( aDim );
      if ( !anAlgoVar->_is_nil() )
        SMESH::AddHypothesisOnMesh( aMesh, anAlgoVar );
    }


    if ( myDlg->toCreateAllGroups() && !aGeomVar->_is_nil() )
    {
      // Create groups on all geom groups

      GEOM::GEOM_Gen_var           geomGen = aGeomVar->GetGen();
      GEOM::GEOM_IShapesOperations_wrap op = geomGen->GetIShapesOperations();
      GEOM::ListOfGO_var        geomGroups = op->GetExistingSubObjects( aGeomVar,
                                                                        /*groupsOnly=*/false );
      SMESH::SMESH_GroupOnGeom_var meshGroup;
      for ( CORBA::ULong iG = 0; iG < geomGroups->length(); ++iG )
      {
        SMESH::ElementType elemType = SMESHGUI_GroupOnShapeOp::ElementType( geomGroups[ iG ] );
        if ( elemType == SMESH::ALL )
          continue;
        if ( elemType == SMESH::ELEM0D )
          elemType = SMESH::NODE;

        CORBA::String_var name = geomGroups[ iG ]->GetName();
        meshGroup = aMesh->CreateGroupFromGEOM( elemType, name, geomGroups[ iG ]);
        // if ( elemType != SMESH::NODE )
        //   meshGroup = aMesh->CreateGroupFromGEOM( SMESH::NODE, name, geomGroups[ iG ]);
      }
    }

    createSubMeshOnInternalEdges( aMesh, aGeomVar );

  }

  return true;
}

//================================================================================
/*!
 * \brief Create sub-mesh
 * \param theMess - Output parameter intended for returning error message
 * \param theEntryList - List of entries of published objects
 * \retval bool  - TRUE if sub-mesh is created, FALSE otherwise
 */
//================================================================================
bool SMESHGUI_MeshOp::createSubMesh( QString& theMess, QStringList& theEntryList )
{
  theMess = "";

  SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
  if ( aSMESHGen->_is_nil() )
    return false;

  // get mesh object
  SMESH::SMESH_Mesh_var aMesh =
    SMESH::EntryToInterface<SMESH::SMESH_Mesh>( myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh ));
  if ( aMesh->_is_nil() && myIsInvalidSubMesh )
  {
    SMESH::SMESH_subMesh_var aSMVar =
      SMESH::EntryToInterface<SMESH::SMESH_subMesh>( myDlg->selectedObject( SMESHGUI_MeshDlg::Obj ));
    if ( !aSMVar->_is_nil() )
      aMesh = aSMVar->GetMesh();
  }
  if ( aMesh->_is_nil() )
    return false;

  // GEOM shape of the main mesh
  GEOM::GEOM_Object_var mainGeom = aMesh->GetShapeToMesh();

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
    _PTR(SObject) pGeom = SMESH::getStudy()->FindObjectID( aGeomEntry.toUtf8().data() );
    aGeomVar = GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
  }
  else if (aGEOMs.count() > 1)
  {
    // create a GEOM group
    GEOM::GEOM_Gen_var geomGen = mainGeom->GetGen();
    if ( !geomGen->_is_nil() ) {
      GEOM::GEOM_IGroupOperations_wrap op = geomGen->GetIGroupOperations();
      if ( !op->_is_nil() )
      {
        // check and add all selected GEOM objects: they must be
        // a sub-shapes of the main GEOM and must be of one type
        int iSubSh = 0;
        TopAbs_ShapeEnum aGroupType = TopAbs_SHAPE;
        GEOM::ListOfGO_var aSeq = new GEOM::ListOfGO;
        aSeq->length(aGEOMs.count());
        QStringList::const_iterator aSubShapesIter = aGEOMs.begin();
        for ( ; aSubShapesIter != aGEOMs.end(); aSubShapesIter++, iSubSh++) {
          QString aSubGeomEntry = (*aSubShapesIter);
          _PTR(SObject) pSubGeom = SMESH::getStudy()->FindObjectID(aSubGeomEntry.toUtf8().data());
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
          SALOMEDS::SObject_wrap aNewGroupSO =
            geomGen->AddInStudy( aGeomVar,
                                 aNewGeomGroupName.toUtf8().data(), mainGeom);
        }
      }
    }
  }
  else {
  }
  if (aGeomVar->_is_nil())
    return false;

  SUIT_OverrideCursor aWaitCursor;

  QString aNameOrID = aName;
  if ( myIsInvalidSubMesh )
    // pass a sub-mesh entry to mesh->GetSubMesh() to replace the invalid sub-mesh
    // by a valid one in an existing SO
    aNameOrID = myDlg->selectedObject(SMESHGUI_MeshDlg::Obj);

  // create sub-mesh
  SMESH::SMESH_subMesh_var aSubMeshVar = aMesh->GetSubMesh( aGeomVar, aNameOrID.toUtf8().data() );
  _PTR(SObject) aSubMeshSO = SMESH::FindSObject( aSubMeshVar.in() );
  if ( aSubMeshSO ) {
    SMESH::SetName( aSubMeshSO, aName.toUtf8().data() );
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

  checkSubMeshConcurrency( aMesh, aSubMeshVar, /*askUser=*/true );

  return true;
}

//================================================================================
/*!
 * \brief Return index of current hypothesis or algorithm
  * \param theDim - dimension of hypothesis or algorithm
  * \param theHypType - Type of hypothesis (Algo, MainHyp, AddHyp)
  * \retval int - index of current hypothesis or algorithm, -1 if None selected
 */
//================================================================================
int SMESHGUI_MeshOp::currentHyp( const int theDim, const int theHypType ) const
{
  if ( 0 <= theDim && theDim <= 3 )
    return myDlg->tab( theDim )->currentHyp( theHypType ) - 1;
  else
    return -1;
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

  _PTR(Study) aStudy = SMESH::getStudy();
  int i = 1;

  QString aPrefix = thePrefix;
  if ( aPrefix.isEmpty() )
    aPrefix = tr( myIsMesh ? "SMESH_OBJECT_MESH" : "SMESH_SUBMESH" ) + "_";

  _PTR(SObject) anObj;
  do
  {
    aResName = aPrefix + QString::number( i++ );
    anObj = aStudy->FindObject( aResName.toUtf8().data() );
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
  _PTR(SObject) pComp = SMESH::getStudy()->FindComponent("SMESH");
  QStringList tmp;
  existingHyps( theDim, Algo, pComp, tmp, myExistingHyps[ theDim ][ Algo ]);

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
          SMESH::CreateHypothesis(aHypName, aHypData->Label, true);
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
            SMESH::CreateHypothesis(aHypName, aHypData->Label, true);
          aHyp.out();
        }
        delete aCreator;
      }
      QStringList tmpList;
      _PTR(SComponent) aFather = SMESH::getStudy()->FindComponent( "SMESH" );
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
  _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );
  if ( !pObj )
    return;

  if ( myIsOnGeometry )
  {
    // Set name of mesh if current object is sub-mesh
    SMESH::SMESH_subMesh_var subMesh = SMESH::SObjectToInterface< SMESH::SMESH_subMesh >( pObj );
    if ( !subMesh->_is_nil() )
    {
      SMESH::SMESH_Mesh_var aMesh =  subMesh->GetFather();
      if ( !aMesh->_is_nil() )
      {
        _PTR(SObject) aMeshSO = SMESH::FindSObject( aMesh );
        QString     aMeshName = name( aMeshSO );
        myDlg->setObjectText( SMESHGUI_MeshDlg::Mesh, aMeshName );
      }
      myHasConcurrentSubBefore = checkSubMeshConcurrency( aMesh, subMesh );
    }

    if ( !myIsInvalidSubMesh )
    {
      // Set name of geometry object
      CORBA::String_var name = SMESH::GetGeomName( pObj );
      if ( name.in() )
        myDlg->setObjectText( SMESHGUI_MeshDlg::Geom, name.in() );
    }
  }

  // Get hypotheses and algorithms assigned to the mesh/sub-mesh
  QStringList anExisting, anAvailable;
  bool algoFound = false;
  const int dim1st = ( myIsOnGeometry ) ? SMESH::DIM_0D : SMESH::DIM_2D;
  for ( int dim = dim1st; dim <= SMESH::DIM_3D; ++dim )
  {
    // get algorithm
    existingHyps( dim, Algo, pObj, anExisting, myObjHyps[ dim ][ Algo ] );
    // find algo index among available ones
    int aHypIndex = -1;
    anAvailable.clear();
    if ( myObjHyps[ dim ][ Algo ].count() > 0 )
    {
      SMESH::SMESH_Hypothesis_var hyp = myObjHyps[ dim ][ Algo ].first().first;
      HypothesisData* algoData = SMESH::GetHypothesisData( SMESH::toQStr( hyp->GetName() ));
      availableHyps( dim, Algo, anAvailable, myAvailableHypData[ dim ][ Algo ] );
      aHypIndex = myAvailableHypData[ dim ][ Algo ].indexOf( algoData );
      //       if ( aHypIndex < 0 && algoData ) {
      //         // assigned algo is incompatible with other algorithms
      //         myAvailableHypData[ dim ][ Algo ].push_back( algoData );
      //         aHypIndex = myAvailableHypData[ dim ][ hypType ].count() - 1;
      //       }
      algoFound = ( aHypIndex > -1 );
    }
    myDlg->tab( dim )->setAvailableHyps( Algo, anAvailable );
    setCurrentHyp( dim, Algo, aHypIndex );
    // set existing and available hypothesis according to the selected algo
    if ( aHypIndex > -1 || !algoFound )
      onAlgoSelected( aHypIndex, dim );
  }

  // get hypotheses
  bool hypWithoutAlgo = false;
  for ( int dim = dim1st; dim <= SMESH::DIM_3D; ++dim )
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
    onAlgoSelected( currentHyp( /*dim=*/0, Algo ), /*dim=*/0 );
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
  * \retval bool  - TRUE if mesh is edited successfully, FALSE otherwise
 *
 * Assigns new name hypotheses and algorithms to the mesh or sub-mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::editMeshOrSubMesh( QString& theMess )
{
  theMess = "";

  SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
  if ( aSMESHGen->_is_nil() )
    return false;

  QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
  _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );
  if ( !pObj )
    return false;

  SUIT_OverrideCursor aWaitCursor;

  // Set new name
  QString aName = myDlg->objectText( SMESHGUI_MeshDlg::Obj );
  SMESH::SetName( pObj, aName );
  int aDim = ( myIsOnGeometry ) ? SMESH::DIM_0D : SMESH::DIM_2D;

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
  SMESH::SMESH_Mesh_var          aMesh = SMESH::SMESH_Mesh::_narrow( anObject );
  SMESH::SMESH_subMesh_var aSubMeshVar = SMESH::SMESH_subMesh::_narrow( anObject );
  bool isMesh = !aMesh->_is_nil();
  if ( !isMesh && !aSubMeshVar->_is_nil() )
    aMesh = aSubMeshVar->GetFather();

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
        SMESH::AddHypothesisOnMesh( aMesh, anAlgoVar );
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
            SMESH::AddHypothesisOnMesh (aMesh, hyp );
          else if ( !aSubMeshVar->_is_nil() )
            SMESH::AddHypothesisOnSubMesh ( aSubMeshVar, hyp );
        }
      }
      // reread all hypotheses of mesh
      QStringList anExisting;
      existingHyps( dim, hypType, pObj, anExisting, myObjHyps[ dim ][ hypType ] );
    }
  }

  if ( aSubMeshVar->_is_nil() )
  {
    GEOM::GEOM_Object_var mainGeom = aMesh->GetShapeToMesh();
    createSubMeshOnInternalEdges( aMesh, mainGeom );
  }

  myHasConcurrentSubBefore =
    checkSubMeshConcurrency( aMesh, aSubMeshVar, /*askUser=*/!myHasConcurrentSubBefore );

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
 * returns false if theOtherOp operation is not intended for deleting objects or mesh
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
    if ( _PTR(SObject) pMesh = SMESH::getStudy()->FindObjectID( aMeshEntry.toUtf8().data() )) {
      SMESH::SMESH_Mesh_var aMesh =
        SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() );
      if ( !aMesh->_is_nil() ) {
        myDlg->hide(); // stop processing selection
        myShapeByMeshOp->setModule( getSMESHGUI() );
        myShapeByMeshOp->setStudy( 0 ); // it's really necessary
        myShapeByMeshOp->SetMesh( aMesh );
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
      if ( _PTR(SObject) aGeomSO = SMESH::getStudy()->FindObjectID( ID.toUtf8().data() )) {
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
 * \brief Update available list of mesh types
 */
//================================================================================
void SMESHGUI_MeshOp::updateMeshTypeList()
{
  QStringList meshTypes;
  meshTypes.append( tr( "MT_ANY" ) );
  if ( myMaxShapeDim >= 2 || myMaxShapeDim == -1 )
  {
    meshTypes.append( tr( "MT_TRIANGULAR" ) );
    meshTypes.append( tr( "MT_QUADRILATERAL" ) );
  }
  if ( myMaxShapeDim == 3 || myMaxShapeDim == -1 )
  {
    meshTypes.append( tr( "MT_TETRAHEDRAL" ) );
    meshTypes.append( tr( "MT_HEXAHEDRAL" ) );
  }

  myDlg->setAvailableMeshType( meshTypes );
}

//================================================================================
/*!
 * \brief Update available list of hypothesis sets
 */
//================================================================================
void SMESHGUI_MeshOp::updateHypoSets()
{
  // get available algorithms taking into account geometry and mesh type
  QStringList algosAvailable;
  if ( myDlg->currentMeshType() != MT_ANY )
    for ( int dim = 0; dim <= 3; ++dim )
      availableHyps( dim, Algo, algosAvailable, myAvailableHypData[ dim ][ Algo ] );

  THypDataList anAvailableAlgsData;
  QStringList aHypothesesSetsList = SMESH::GetHypothesesSets( myMaxShapeDim );
  QStringList aFilteredHypothesesSetsList;
  QStringList::const_iterator inHypoSetName = aHypothesesSetsList.begin();
  for ( ; inHypoSetName != aHypothesesSetsList.end(); ++inHypoSetName )
  {
    HypothesesSet* currentHypoSet = SMESH::GetHypothesesSet( *inHypoSetName );
    HypothesesSet::SetType sType;
    for ( sType = HypothesesSet::MAIN; sType <= HypothesesSet::ALT; SMESHUtils::Increment( sType ))
    {
      bool isAvailable = false;
      for ( currentHypoSet->init( true, sType ); currentHypoSet->more(); currentHypoSet->next() )
      {
        isAvailable = false;
        if ( HypothesisData* algoDataIn = SMESH::GetHypothesisData( currentHypoSet->current() )) {
          for ( int dim = SMESH::DIM_0D; dim <= myMaxShapeDim; dim++) {
            int aCurrentAvailableAlgo = myAvailableHypData[dim][Algo].indexOf( algoDataIn );
            if ( aCurrentAvailableAlgo > -1 ) {
              isAvailable = true;
              break;
            }
          }
          if ( !isAvailable )
            break;
        }
      }
      currentHypoSet->setAlgoAvailable( sType, isAvailable );
    }
    if ( currentHypoSet->hasAlgo( HypothesesSet::MAIN ) &&
         currentHypoSet->hasAlgo( HypothesesSet::ALT ))
    {
      HypothesesSet::SetType setType = HypothesesSet::getPreferredHypType();
      if ( !currentHypoSet->getAlgoAvailable( setType ))
        continue; // not add if a preferred type not available currently
    }

    if ( currentHypoSet->getAlgoAvailable( HypothesesSet::MAIN ) ||
         currentHypoSet->getAlgoAvailable( HypothesesSet::ALT ))
    {
      aFilteredHypothesesSetsList.append( *inHypoSetName );
    }
  }
  myDlg->setHypoSets( aFilteredHypothesesSetsList );
}

//================================================================================
/*!
 * \brief SLOT. Is called when the user select type of mesh
 * \param theTabIndex - Index of current active tab
 * \param theIndex - Index of current type of mesh
 */
//================================================================================
void SMESHGUI_MeshOp::onAlgoSetByMeshType( const int /*theTabIndex*/, const int /*theIndex*/ )
{
  setFilteredAlgoData();
}
