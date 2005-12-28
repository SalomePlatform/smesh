// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
/**
*  SMESH SMESHGUI
*
*  Copyright (C) 2005  CEA/DEN, EDF R&D
*
*
*
*  File   : SMESHGUI_MeshOp.h
*  Author : Sergey LITONIN
*  Module : SMESHGUI
*/

#include "SMESHGUI_MeshOp.h"
#include "SMESHGUI_MeshDlg.h"
#include "SMESH_TypeFilter.hxx"
#include "SMESHGUI.h"

#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SMESH_TypeFilter.hxx"
#include "SMESH_NumberFilter.hxx"

#include "GEOM_SelectionFilter.h"

#include "SalomeApp_Tools.h"
#include "SALOMEDSClient_Study.hxx"
#include "SALOMEDSClient_AttributeIOR.hxx"
#include "SALOMEDSClient_AttributeName.hxx"
#include "SALOMEDS_SComponent.hxx"
#include "SALOMEDS_SObject.hxx"


#include "LightApp_SelectionMgr.h"
#include "LightApp_UpdateFlags.h"
#include "SUIT_MessageBox.h"
#include "SUIT_Desktop.h"
#include "SUIT_OverrideCursor.h"

#include "GEOMBase.h"

#include "utilities.h"

#include <qstringlist.h>
#include <qlineedit.h>

#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>

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
  myToCreate( theToCreate ),
  myIsMesh( theIsMesh ),
  myDlg( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================
SMESHGUI_MeshOp::~SMESHGUI_MeshOp()
{
  if( myDlg )
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
  if( isStudyLocked() )
    return false;

  QString aMess;
  if ( !isValid( aMess ) )
  {
    if ( aMess != "" )
      SUIT_MessageBox::warn1( myDlg,
        tr( "SMESH_WRN_WARNING" ), aMess, tr( "SMESH_BUT_OK" ) );
    return false;
  }

  bool aResult = false;
  aMess = "";
  try
  {
    if ( myToCreate && myIsMesh )
      aResult = createMesh( aMess );
    if ( myToCreate && !myIsMesh )
      aResult = createSubMesh( aMess );
    else if ( !myToCreate )
      aResult = editMeshOrSubMesh( aMess );
    if ( aResult )
      update( UF_ObjBrowser | UF_Model );
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
    SUIT_MessageBox::warn1( myDlg,
      tr( "SMESH_ERROR" ), aMess, tr( "SMESH_BUT_OK" ) );
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
  if( !myDlg )
  {
    myDlg = new SMESHGUI_MeshDlg( myToCreate, myIsMesh );
    for ( int i = SMESH::DIM_1D; i <= SMESH::DIM_3D; i++ )
    {
      connect( myDlg->tab( i ), SIGNAL( createHyp( const int, const int ) ),
              this, SLOT( onCreateHyp( const int, const int) ) );
      connect( myDlg->tab( i ), SIGNAL( editHyp( const int, const int ) ),
              this, SLOT( onEditHyp( const int, const int) ) );
    }
    connect( myDlg, SIGNAL( hypoSet( const QString& )), SLOT( onHypoSet( const QString& )));
  }
  SMESHGUI_SelectionOp::startOperation();

  // iterate through dimensions and get available and existing algoritms and hypotheses,
  // set them to the dialog
  int i, j;
  _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
  for ( i = SMESH::DIM_1D; i <= SMESH::DIM_3D; i++ )
  {
    SMESHGUI_MeshTab* aTab = myDlg->tab( i );
    QStringList anAvailable, anExisting;
    for ( j = Algo; j <= AddHyp; j++ )
    {
      availableHyps( i, j, anAvailable );
      existingHyps( i, j, aFather, anExisting, myExistingHyps[ i ][ j ] );

      aTab->setAvailableHyps( j, anAvailable );
      aTab->setExistingHyps( j, anExisting );
    }
  }
  if ( myToCreate )
  {
    setDefaultName();
    myDlg->activateObject( myIsMesh ? SMESHGUI_MeshDlg::Geom : SMESHGUI_MeshDlg::Mesh );
  }
  else
    myDlg->activateObject( SMESHGUI_MeshDlg::Obj );

  myDlg->setHypoSets( SMESH::GetHypothesesSets() );

  selectionDone();

  myDlg->setCurrentTab( SMESH::DIM_1D );
  myDlg->show();
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
    return new SMESH_TypeFilter( MESHorSUBMESH );
  else if ( theId == SMESHGUI_MeshDlg::Mesh )
    return new SMESH_TypeFilter( MESH );
  else
    return 0;
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
  SMESHGUI_SelectionOp::selectionDone();

  if ( !myToCreate )
  {
    try
    {
      QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
      _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.latin1() );
      if ( pObj != 0 )
      {
        SMESH::SMESH_subMesh_var aVar =
          SMESH::SMESH_subMesh::_narrow( _CAST( SObject,pObj )->GetObject() );
        myDlg->setObjectShown( SMESHGUI_MeshDlg::Mesh, !aVar->_is_nil() );
        myDlg->objectWg( SMESHGUI_MeshDlg::Mesh, SMESHGUI_MeshDlg::Btn )->hide();
        myDlg->updateGeometry();
        myDlg->adjustSize();
        readMesh();
      }
      else
        myDlg->reset();
    }
    catch ( const SALOME::SALOME_Exception& S_ex )
    {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
    catch ( ... )
    {
    }
  }

  // Enable tabs according to shape dimension

  int shapeDim = 3;
  try
  {
    GEOM::GEOM_Object_var aGeomVar;
    QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
    _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.latin1() );
    if ( pGeom ) {
      aGeomVar = GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );
    }
    else {
      QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
      _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.latin1() );
      aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pObj );
    }
    if ( !aGeomVar->_is_nil() ) {
      shapeDim = 0;
      switch ( aGeomVar->GetShapeType() ) {
      case GEOM::SOLID:
      case GEOM::SHELL:  shapeDim = 3; break;
      case GEOM::FACE:   shapeDim = 2; break;
      case GEOM::WIRE:   
      case GEOM::EDGE:   shapeDim = 1; break;
      case GEOM::VERTEX: shapeDim = 0; break;
      default:
        TopoDS_Shape aShape;
        if ( GEOMBase::GetShape(aGeomVar, aShape)) {
          TopExp_Explorer exp( aShape, TopAbs_SHELL );
          if ( exp.More() )
            shapeDim = 3;
          else if ( exp.Init( aShape, TopAbs_FACE ), exp.More() )
            shapeDim = 2;
          else if ( exp.Init( aShape, TopAbs_EDGE ), exp.More() )
            shapeDim = 1;
          else
            shapeDim = 0;
        }
      }
    }
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... )
  {
  }
  myDlg->setMaxHypoDim( shapeDim );
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
  QString aMeshName = myDlg->objectText( SMESHGUI_MeshDlg::Obj );
  aMeshName = aMeshName.stripWhiteSpace();
  if ( aMeshName == "" )
  {
    theMess = myIsMesh ? tr( "NAME_OF_MESH_IS_EMPTY" ) : tr( "NAME_OF_SUBMESH_IS_EMPTY" );
    return false;
  }

  // Geom
  if ( myToCreate )
  {
    QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
    if ( aGeomEntry == "" )
    {
      theMess = tr( "GEOMETRY_OBJECT_IS_NOT_DEFINED" );
      return false;
    }
    _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.latin1() );
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
      _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.latin1() );
      if ( !pMesh || SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() )->_is_nil() )
      {
        theMess = tr( "MESH_IS_NULL" );
        return false;
      }
    }
  }
  return true;
}

//================================================================================
/*!
 * \brief Gets available hypotheses or algorithms
  * \param theDim - specifies dimension of returned hypotheses/algorifms
  * \param theHypType - specifies whether algorims or hypotheses or additional ones
  * are retrieved (possible values are in HypType enumeration)
  * \param theHyps - Output list of hypotheses' names
 *
 * Gets available hypotheses or algorithm in accordance with input parameters
 */
//================================================================================
void SMESHGUI_MeshOp::availableHyps( const int theDim,
                                     const int theHypType,
                                     QStringList& theHyps ) const
{
  theHyps.clear();
  QStringList aHypTypeNameList = SMESH::GetAvailableHypotheses(
    theHypType == Algo , theDim, theHypType == AddHyp );
  QStringList::const_iterator anIter;
  for ( anIter = aHypTypeNameList.begin(); anIter != aHypTypeNameList.end(); ++anIter )
  {
    HypothesisData* aData = SMESH::GetHypothesisData( *anIter );
    theHyps.append( aData->Label );
  }
}

//================================================================================
/*!
 * \brief Gets existing hypotheses or algorithms
  * \param theDim - specifies dimension of returned hypotheses/algorifms
  * \param theHypType - specifies whether algorims or hypotheses or additional ones
  * are retrieved (possible values are in HypType enumeration)
  * \param theFather - start object for finding ( may be component, mesh, or sub-mesh )
  * \param theHyps - output list of names.
  * \param theHypVars - output list of variables.
 *
 * Gets existing (i.e. already created) hypotheses or algorithm in accordance with
 * input parameters
 */
//================================================================================
void SMESHGUI_MeshOp::existingHyps( const int theDim,
                                    const int theHypType,
                                    _PTR(SObject) theFather,
                                    QStringList& theHyps,
                                    QValueList<SMESH::SMESH_Hypothesis_var>& theHypVars )
{
  // Clear hypoheses list
  theHyps.clear();
  theHypVars.clear();

  if ( !theFather )
    return;

  _PTR(SObject)          aHypRoot;
  _PTR(GenericAttribute) anAttr;
  _PTR(AttributeName)    aName;
  _PTR(AttributeIOR)     anIOR;

  bool isMesh = !_CAST( SComponent, theFather );
  int aPart = -1;
  if ( isMesh )
    aPart = theHypType == Algo ? 3 : 2;
  else
    aPart = theHypType == Algo ? 2 : 1;

  if ( theFather->FindSubObject( aPart, aHypRoot ) )
  {
    _PTR(ChildIterator) anIter =
      SMESH::GetActiveStudyDocument()->NewChildIterator( aHypRoot );
    for (; anIter->More(); anIter->Next() )
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
            QString aHypType( aHypVar->GetName() );
            HypothesisData* aData = SMESH::GetHypothesisData( aHypType );
            if ( ( theDim == -1 || aData->Dim.contains( theDim ) ) &&
                 ( theHypType == AddHyp ) == aData->IsAux )
            {
              theHyps.append( aName->Value().c_str() );
              theHypVars.append( aHypVar );
            }
          }
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief Calls plugin methods for hypothesis creation
  * \param theHypType - specifies whether main hypotheses or additional ones
  * are created
  * \param theIndex - index of type of hypothesis to be cerated
 *
 * Speicfies dimension of hypothesis to be created (using sender() method), specifies
 * its type and calls plugin methods for hypothesis creation
 */
//================================================================================
void SMESHGUI_MeshOp::onCreateHyp( const int theHypType, const int theIndex )
{
  // Speicfies dimension of hypothesis to be created
  const QObject* aSender = sender();
  int aDim = -1;
  for ( int i = SMESH::DIM_1D; i <= SMESH::DIM_3D; i++ )
    if ( aSender == myDlg->tab( i ) )
      aDim = i;
  if ( aDim == -1 )
    return;

  // Speicfies type of hypothesis to be created
  QStringList aHypTypeNames = SMESH::GetAvailableHypotheses( false , aDim, theHypType == AddHyp );
  if ( theIndex < 0 || theIndex >= aHypTypeNames.count() )
    return;

  QString aHypTypeName = aHypTypeNames[ theIndex ];
  HypothesisData* aData = SMESH::GetHypothesisData( aHypTypeName.latin1() );
  if ( aData == 0 )
    return;

  QString aClientLibName = aData->ClientLibName;
  QStringList anOldHyps;
  _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
  existingHyps( aDim, theHypType, aFather, anOldHyps, myExistingHyps[ aDim ][ theHypType ] );

  if ( aClientLibName == "" )
  {
    // Call hypothesis creation server method (without GUI)
    QString aHypName = aData->Label;
    SMESH::CreateHypothesis( aHypTypeName, aHypName, false );
  }
  else
  {
    // Get hypotheses creator client (GUI)
    SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator( aHypTypeName );

    // Create hypothesis
    if( aCreator )
      aCreator->create( false, myDlg );
  }

  QStringList aNewHyps;
  aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
  existingHyps( aDim, theHypType, aFather, aNewHyps, myExistingHyps[ aDim ][ theHypType ] );
  if ( aNewHyps.count() > anOldHyps.count() )
  {
    for ( int i = anOldHyps.count(); i < aNewHyps.count(); i++ )
      myDlg->tab( aDim )->addHyp( theHypType, aNewHyps[ i ] );
  }
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
  const QObject* aSender = sender();
  int aDim = -1;
  for ( int i = SMESH::DIM_1D; i <= SMESH::DIM_3D; i++ )
    if ( aSender == myDlg->tab( i ) )
      aDim = i;
  if ( aDim == -1 )
    return;

  QValueList<SMESH::SMESH_Hypothesis_var> aList = myExistingHyps[ aDim ][ theHypType ];
  SMESH::SMESH_Hypothesis_var aHyp = aList[ theIndex - 1 ];
  if ( aHyp->_is_nil() )
    return;

  char* aTypeName = aHyp->GetName();
  SMESHGUI_GenericHypothesisCreator* aCreator = SMESH::GetHypothesisCreator( aTypeName );
  if ( aCreator )
    aCreator->edit( aHyp.in(), dlg() );
}

//================================================================================
/*!
 * \brief Creates and selects hypothesis of hypotheses set 
  * \param theSetName - The name of hypotheses set
 */
//================================================================================

void SMESHGUI_MeshOp::onHypoSet( const QString& theSetName )
{
  HypothesesSet* aHypoSet = SMESH::GetHypothesesSet( theSetName );
  if ( !aHypoSet ) return;

  for ( int aHypType = Algo; aHypType < AddHyp; aHypType++ )
  {
    bool isAlgo = (aHypType == Algo);

    // clear all hyps
    for ( int dim = SMESH::DIM_1D; dim <= SMESH::DIM_3D; dim++ )
      setCurrentHyp( dim, aHypType, -1 );

    // set hyps from the set
    
    QStringList* aHypoList = isAlgo ? & aHypoSet->AlgoList : & aHypoSet->HypoList;
    for ( int i = 0, n = aHypoList->count(); i < n; i++ )
    {
      const QString& aHypoTypeName = (*aHypoList)[ i ];
      HypothesisData* aHypData = SMESH::GetHypothesisData( aHypoTypeName );
      if ( !aHypData )
	continue;

      int aDim = aHypData->Dim[0];
      // create or/and set
      int index = -1;
      if ( isAlgo )
      {
        QStringList aHypTypeNameList = SMESH::GetAvailableHypotheses( isAlgo, aDim );
        index = aHypTypeNameList.findIndex( aHypoTypeName );
        if ( index < 0 ) continue;
        setCurrentHyp ( aDim, aHypType, index );
      }
      else
      {
        // try to find an existing hypo
        QValueList<SMESH::SMESH_Hypothesis_var> & aList = myExistingHyps[ aDim ][ aHypType ];
        int iHyp = 0, nbHyp = aList.count();
        for ( ; iHyp < nbHyp; ++iHyp )
        {
          SMESH::SMESH_Hypothesis_var aHyp = aList[ iHyp ];
          if ( !aHyp->_is_nil() && aHypoTypeName == aHyp->GetName() ) {
            index = iHyp;
            break;
          }
        }
        if ( index >= 0 ) // found
        {
          // select an algorithm
          setCurrentHyp ( aDim, aHypType, index );
        }
        else
        {
          // create a hypothesis
          QString aClientLibName = aHypData->ClientLibName;
          if ( aClientLibName == "" ) {
            // Call hypothesis creation server method (without GUI)
            SMESH::CreateHypothesis( aHypoTypeName, aHypData->Label, isAlgo );
          }
          else {
            // Get hypotheses creator client (GUI)
            SMESHGUI_GenericHypothesisCreator* aCreator =
              SMESH::GetHypothesisCreator( aHypoTypeName );
            aCreator->create( false, myDlg );
          }
          QStringList aNewHyps;
          _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
          existingHyps( aDim, aHypType, aFather, aNewHyps, aList );
          if ( aList.count() > nbHyp )
          {
            for ( int i = nbHyp; i < aNewHyps.count(); i++ )
              myDlg->tab( aDim )->addHyp( aHypType, aNewHyps[ i ] );
          }
        }
      }
    } // loop on hypos in the set
  } // loop on algo/hypo
}

//================================================================================
/*!
 * \brief Creates mesh
  * \param theMess - Output parameter intended for returning error message
  * \retval bool  - TRUE if mesh is created, FALSE otherwise
 *
 * Creates mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::createMesh( QString& theMess )
{
  theMess = "";

  QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
  _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.latin1() );
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
  if ( aMeshSO )
    SMESH::SetName( aMeshSO, myDlg->objectText( SMESHGUI_MeshDlg::Obj ).latin1() );

  for ( int aDim = SMESH::DIM_1D; aDim <= SMESH::DIM_3D; aDim++ )
  {
    if ( !isAccessibleDim( aDim )) continue;

    // assign hypotheses
    for ( int aHypType = MainHyp; aHypType <= AddHyp; aHypType++ )
    {
      int aHypIndex = currentHyp( aDim, aHypType );
      if ( aHypIndex >= 0 && aHypIndex < myExistingHyps[ aDim ][ aHypType ].count() )
      {
        SMESH::SMESH_Hypothesis_var aHypVar = myExistingHyps[ aDim ][ aHypType ][ aHypIndex ];
        if ( !aHypVar->_is_nil() )
          SMESH::AddHypothesisOnMesh( aMeshVar, aHypVar );
      }
    }
    // find or create algorithm
    SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( aDim );
    if ( !anAlgoVar->_is_nil() )
      SMESH::AddHypothesisOnMesh( aMeshVar, anAlgoVar );
  }
  return true;
}

//================================================================================
/*!
 * \brief Creates sub-mesh
  * \param theMess - Output parameter intended for returning error message
  * \retval bool  - TRUE if sub-mesh is created, FALSE otherwise
 *
 * Creates sub-mesh
 */
//================================================================================
bool SMESHGUI_MeshOp::createSubMesh( QString& theMess )
{
  theMess = "";

  SMESH::SMESH_Gen_var aSMESHGen = SMESHGUI::GetSMESHGen();
  if ( aSMESHGen->_is_nil() )
    return false;

  // get mesh object
  QString aMeshEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Mesh );
  _PTR(SObject) pMesh = studyDS()->FindObjectID( aMeshEntry.latin1() );
  SMESH::SMESH_Mesh_var aMeshVar =
    SMESH::SMESH_Mesh::_narrow( _CAST( SObject,pMesh )->GetObject() );

  // get geom object
  QString aGeomEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Geom );
  _PTR(SObject) pGeom = studyDS()->FindObjectID( aGeomEntry.latin1() );
  GEOM::GEOM_Object_var aGeomVar =
    GEOM::GEOM_Object::_narrow( _CAST( SObject,pGeom )->GetObject() );

  SUIT_OverrideCursor aWaitCursor;

  // create sub-mesh
  QString aName = myDlg->objectText( SMESHGUI_MeshDlg::Obj );
  SMESH::SMESH_subMesh_var aSubMeshVar = aMeshVar->GetSubMesh( aGeomVar, aName.latin1() );

  for ( int aDim = SMESH::DIM_1D; aDim <= SMESH::DIM_3D; aDim++ )
  {
    if ( !isAccessibleDim( aDim )) continue;

    // find or create algorithm
    SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( aDim );
    if ( !anAlgoVar->_is_nil() )
      SMESH::AddHypothesisOnSubMesh( aSubMeshVar, anAlgoVar );
    // assign hypotheses
    for ( int aHypType = MainHyp; aHypType <= AddHyp; aHypType++ )
    {
      int aHypIndex = currentHyp( aDim, aHypType );
      if ( aHypIndex >= 0 && aHypIndex < myExistingHyps[ aDim ][ aHypType ].count() )
      {
        SMESH::SMESH_Hypothesis_var aHypVar =
          myExistingHyps[ aDim ][ aHypType ][ aHypIndex ];
        if ( !aHypVar->_is_nil() )
          SMESH::AddHypothesisOnSubMesh( aSubMeshVar, aHypVar );
      }
    }
  }
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
 * \brief Returns true if hypotheses of given dim can be assigned
  * \param theDim - hypotheses dimension
  * \retval bool - result
 */
//================================================================================
bool SMESHGUI_MeshOp::isAccessibleDim( const int theDim) const
{
  return myDlg->tab( theDim )->isEnabled();
}

//================================================================================
/*!
 * \brief Sets current hypothesis or algorithms
  * \param theDim - dimension of hypothesis or algorithm
  * \param theHypType - Type of hypothesis (Algo, MainHyp, AddHyp)
  * \param theIndex - Index of hypothesis
 *
 * Gets current hypothesis or algorithms
 */
//================================================================================
void SMESHGUI_MeshOp::setCurrentHyp( const int theDim,
                                     const int theHypType,
                                     const int theIndex )
{
  myDlg->tab( theDim )->setCurrentHyp( theHypType, theIndex + 1 );
}

//================================================================================
/*!
 * \brief Generates default and sets mesh/submesh name
 *
 * Generates and sets default mesh/submesh name(Mesh_1, Mesh_2, etc.)
 */
//================================================================================
void SMESHGUI_MeshOp::setDefaultName() const
{
  QString aResName;

  _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
  int i = 1;
  QString aPrefix = tr( myIsMesh ? "SMESH_OBJECT_MESH" : "SMESH_SUBMESH" ) + "_";
  _PTR(SObject) anObj;
  do
  {
    aResName = aPrefix + QString::number( i++ );
    anObj = aStudy->FindObject( aResName.latin1() );
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
  int aHypIndex = currentHyp( theDim, Algo );
  QStringList aHypTypeNameList = SMESH::GetAvailableHypotheses( true, theDim, false );
  if ( aHypIndex < 0 || aHypIndex >= aHypTypeNameList.count() )
    return anAlgoVar;
  QString aHypName = aHypTypeNameList[ aHypIndex ];
  QValueList<SMESH::SMESH_Hypothesis_var>& aHypVarList = myExistingHyps[ theDim ][ Algo ];
  QValueList<SMESH::SMESH_Hypothesis_var>::iterator anIter;
  for ( anIter = aHypVarList.begin(); anIter != aHypVarList.end(); anIter++ )
  {
    SMESH::SMESH_Hypothesis_var aHypVar = *anIter;
    if ( !aHypVar->_is_nil() && aHypName == aHypVar->GetName() )
    {
      anAlgoVar = aHypVar;
      break;
    }
  }
  if ( anAlgoVar->_is_nil() )
  {
    HypothesisData* aHypData = SMESH::GetHypothesisData( aHypName );
    if ( aHypData )
    {
      QString aClientLibName = aHypData->ClientLibName;
      if ( aClientLibName == "" )
        SMESH::CreateHypothesis( aHypName, aHypData->Label, true );
      else
      {
        SMESHGUI_GenericHypothesisCreator* aCreator =
          SMESH::GetHypothesisCreator( aHypName );
        if ( aCreator )
          aCreator->create( true, myDlg );
      }
      QStringList tmpList;
      _PTR(SComponent) aFather = SMESH::GetActiveStudyDocument()->FindComponent( "SMESH" );
      existingHyps( theDim, Algo, aFather, tmpList, myExistingHyps[ theDim ][ Algo ] );
    }

    QValueList<SMESH::SMESH_Hypothesis_var>& aNewHypVarList = myExistingHyps[ theDim ][ Algo ];
    for ( anIter = aNewHypVarList.begin(); anIter != aNewHypVarList.end(); ++anIter )
    {
      SMESH::SMESH_Hypothesis_var aHypVar = *anIter;
      if ( !aHypVar->_is_nil() && aHypName == aHypVar->GetName() )
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
 * \brief Reads parameters of edited mesh and assigns them to the dialog
 *
 * Reads parameters of edited mesh and assigns them to the dialog (called when
 * mesh is edited only)
 */
//================================================================================
void SMESHGUI_MeshOp::readMesh()
{
  QString anObjEntry = myDlg->selectedObject( SMESHGUI_MeshDlg::Obj );
  _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.latin1() );
  if ( !pObj )
    return;

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
  }

  // Get name of geometry object
  GEOM::GEOM_Object_var aGeomVar = SMESH::GetShapeOnMeshOrSubMesh( pObj );
  if ( !aGeomVar->_is_nil() )
  {
    _PTR(SObject) aGeomSO = studyDS()->FindObjectID( aGeomVar->GetStudyEntry() );
    QString aShapeName = name( aGeomSO );
    myDlg->setObjectText( SMESHGUI_MeshDlg::Geom, aShapeName );
  }

  // Get hypotheses and algorithms assigned to the mesh/sub-mesh
  for ( int dim = SMESH::DIM_1D; dim <= SMESH::DIM_3D; dim++ )
  {
    // get algorithm
    QStringList anExisting;
    int aHypIndex = -1;
    existingHyps( dim, Algo, pObj, anExisting, myObjHyps[ dim ][ Algo ] );
    if ( myObjHyps[ dim ][ Algo ].count() > 0 )
    {
      SMESH::SMESH_Hypothesis_var aVar = myObjHyps[ dim ][ Algo ].first();
      QString aHypTypeName = aVar->GetName();

      QStringList aHypTypeNameList = SMESH::GetAvailableHypotheses( true , dim, false );
      for ( int i = 0, n = aHypTypeNameList.count(); i < n; i++ )
        if ( aHypTypeName == aHypTypeNameList[ i ] )
        {
          aHypIndex = i;
          break;
        }
    }
    setCurrentHyp( dim, Algo, aHypIndex );

    // get hypotheses
    for ( int hypType = MainHyp; hypType <= AddHyp; hypType++ )
    {
      // get hypotheses
      existingHyps( dim, hypType, pObj, anExisting, myObjHyps[ dim ][ hypType ] );
      // find index of requered hypothesis among existing ones for this dimension
      // and hyp types
      int aHypIndex = -1;
      if ( myObjHyps[ dim ][ hypType ].count() > 0 )
        aHypIndex = find( myObjHyps[ dim ][ hypType ].first(),
                          myExistingHyps[ dim ][ hypType ] );
      setCurrentHyp( dim, hypType, aHypIndex );
    }
  }
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
                           const QValueList<SMESH::SMESH_Hypothesis_var>& theHypList ) const
{
  int aRes = -1;
  if ( !theHyp->_is_nil() )
  {
    int i = 0;
    QValueList<SMESH::SMESH_Hypothesis_var>::const_iterator anIter;
    for ( anIter = theHypList.begin(); anIter != theHypList.end(); ++ anIter )
    {
      if ( theHyp->_is_equivalent( *anIter ) )
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
  _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.latin1() );
  if ( !pObj )
    return false;

  SUIT_OverrideCursor aWaitCursor;

  // Set new name
  SMESH::SetName( pObj, myDlg->objectText( SMESHGUI_MeshDlg::Obj ).latin1() );

  // Assign new hypotheses and algorithms
  for ( int dim = SMESH::DIM_1D; dim <= SMESH::DIM_3D; dim++ )
  {
    if ( !isAccessibleDim( dim )) continue;

    // find or create algorithm
    bool toDelete = false, toAdd = true;
    SMESH::SMESH_Hypothesis_var anAlgoVar = getAlgo( dim );
    if ( anAlgoVar->_is_nil() ) {
      toAdd = false;
    }
    if ( myObjHyps[ dim ][ Algo ].count() > 0 ) {
      SMESH::SMESH_Hypothesis_var anOldAlgo = myObjHyps[ dim ][ Algo ].first();
      if ( toAdd ) {
        if ( strcmp(anOldAlgo->GetName(), anAlgoVar->GetName()) == 0 ) {
          toAdd = false;
        } else {
          toDelete = true;
        }
      } else {
        toDelete = true;
      }
    }
    // remove old algorithm
    if ( toDelete )
      SMESH::RemoveHypothesisOrAlgorithmOnMesh
        ( pObj, myObjHyps[ dim ][ Algo ].first() );

    // assign new algorithm
    if ( toAdd ) {
      SMESH::SMESH_Mesh_var aMeshVar =
        SMESH::SMESH_Mesh::_narrow( _CAST(SObject,pObj)->GetObject() );
      bool isMesh = !aMeshVar->_is_nil();
      if ( isMesh ) {
        SMESH::AddHypothesisOnMesh( aMeshVar, anAlgoVar );
      } else {
        SMESH::SMESH_subMesh_var aVar =
          SMESH::SMESH_subMesh::_narrow( _CAST(SObject,pObj)->GetObject() );
        if ( !aVar->_is_nil() )
          SMESH::AddHypothesisOnSubMesh( aVar, anAlgoVar );
      }
    }

    // assign hypotheses
    for ( int hypType = MainHyp; hypType <= AddHyp; hypType++ )
    {
      int aNewHypIndex = currentHyp( dim, hypType );
      int anOldHypIndex = -1;
      if ( myObjHyps[ dim ][ hypType ].count() > 0 )
        anOldHypIndex = find( myObjHyps[ dim ][ hypType ].first(),
                              myExistingHyps[ dim ][ hypType ] );
      if ( aNewHypIndex != anOldHypIndex )
      {
        // remove old hypotheses
        if ( anOldHypIndex >= 0 )
          SMESH::RemoveHypothesisOrAlgorithmOnMesh(
            pObj, myExistingHyps[ dim ][ hypType ][ anOldHypIndex ] );

        // assign new hypotheses
        if ( aNewHypIndex != -1 )
        {
          SMESH::SMESH_Mesh_var aMeshVar =
              SMESH::SMESH_Mesh::_narrow( _CAST(SObject,pObj)->GetObject() );
          bool isMesh = !aMeshVar->_is_nil();
          if ( isMesh )
          {
            SMESH::AddHypothesisOnMesh(
              aMeshVar, myExistingHyps[ dim ][ hypType ][ aNewHypIndex ] );
          }
          else
          {
            SMESH::SMESH_subMesh_var aVar =
              SMESH::SMESH_subMesh::_narrow( _CAST(SObject,pObj)->GetObject() );
            if ( !aVar->_is_nil() )
              SMESH::AddHypothesisOnSubMesh(
                aVar, myExistingHyps[ dim ][ hypType ][ aNewHypIndex ] );
          }
        }
        // reread all hypotheses of mesh if necessary
        QStringList anExisting;
        existingHyps( dim, hypType, pObj, anExisting, myObjHyps[ dim ][ hypType ] );
      }
    }
  }

  return true;
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
