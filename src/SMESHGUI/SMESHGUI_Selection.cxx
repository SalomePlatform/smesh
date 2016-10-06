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

// SMESH SMESHGUI_Selection
// File   : SMESHGUI_Selection.cxx
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//

// SMESH includes
#include "SMESHGUI_Selection.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_ComputeDlg.h"
#include "SMESHGUI_ConvToQuadOp.h"

#include <SMESH_Type.h>
#include <SMESH_Actor.h>
#include <SMESH_ScalarBarActor.h>

// SALOME GUI includes
#include <SalomeApp_Study.h>
#include <LightApp_VTKSelector.h>
#include <SVTK_ViewWindow.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Gen)
#include CORBA_CLIENT_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(SMESH_Group)

//=======================================================================
//function : SMESHGUI_Selection
//purpose  : 
//=======================================================================
SMESHGUI_Selection::SMESHGUI_Selection()
: LightApp_Selection()
{
}

//=======================================================================
//function : ~SMESHGUI_Selection
//purpose  : 
//=======================================================================
SMESHGUI_Selection::~SMESHGUI_Selection()
{
}

//=======================================================================
//function : init
//purpose  : 
//=======================================================================
void SMESHGUI_Selection::init( const QString& client, LightApp_SelectionMgr* mgr )
{
  LightApp_Selection::init( client, mgr );

  if( mgr && study() )
  {
    SalomeApp_Study* aSStudy = dynamic_cast<SalomeApp_Study*>(study());
    if (!aSStudy)
      return;
    _PTR(Study) aStudy = aSStudy->studyDS();

    for( int i=0, n=count(); i<n; i++ ) {
      myTypes.append( typeName( type( entry( i ), aStudy ) ) );
      myControls.append( controlMode( i ) );
    }
  }
}

//=======================================================================
//function : processOwner
//purpose  : 
//=======================================================================
bool SMESHGUI_Selection::processOwner( const LightApp_DataOwner* ow )
{
  const LightApp_SVTKDataOwner* owner =
    dynamic_cast<const LightApp_SVTKDataOwner*> ( ow );
  if( owner ) {
    myActors.append( dynamic_cast<SMESH_Actor*>( owner->GetActor() ) );
  }
  else if ( ow ) { // SVTK selection disabled
    QString entry = ow->entry();
    myActors.append( SMESH::FindActorByEntry( entry.toStdString().c_str() ));
  }
  else {
    myActors.append( 0 );
  }
  return true;
}

//=======================================================================
//function : parameter
//purpose  : 
//=======================================================================
QVariant SMESHGUI_Selection::parameter( const int ind, const QString& p ) const
{
  QVariant val;
  if      ( p=="client" )               val = QVariant( LightApp_Selection::parameter( p ) );
  else if ( p=="type" )                 val = QVariant( myTypes[ind] );
  else if ( p=="hasActor" )             val = QVariant( getActor( ind ) != 0 );
  else if ( p=="elemTypes" )            val = QVariant( elemTypes( ind ) );
  else if ( p=="isAutoColor" )          val = QVariant( isAutoColor( ind ) );
  else if ( p=="numberOfNodes" )        val = QVariant( numberOfNodes( ind ) );
  else if ( p=="dim" )                  val = QVariant( dim( ind ) );
  else if ( p=="labeledTypes" )         val = QVariant( labeledTypes( ind ) );
  else if ( p=="shrinkMode" )           val = QVariant( shrinkMode( ind ) );
  else if ( p=="entityMode" )           val = QVariant( entityMode( ind ) );
  else if ( p=="isNumFunctor" )         val = QVariant( isNumFunctor( ind ) );
  else if ( p=="displayMode" )          val = QVariant( displayMode( ind ) );
  else if ( p=="isComputable" )         val = QVariant( isComputable( ind ) );
  else if ( p=="isPreComputable" )      val = QVariant( isPreComputable( ind ) );
  else if ( p=="hasGeomReference" )     val = QVariant( hasGeomReference( ind ) );
  else if ( p=="isEditableHyp" )        val = QVariant( isEditableHyp( ind ) );
  else if ( p=="isImported" )           val = QVariant( isImported( ind ) );
  else if ( p=="facesOrientationMode" ) val = QVariant( facesOrientationMode( ind ) );
  else if ( p=="groupType" )            val = QVariant( groupType( ind ) );
  else if ( p=="isQuadratic" )          val = QVariant( isQuadratic( ind ) );
  else if ( p=="quadratic2DMode")       val = QVariant( quadratic2DMode( ind ) );
  else if ( p=="isDistributionVisible") val = QVariant( isDistributionVisible( ind ) );
  else if ( p=="isScalarBarVisible")    val = QVariant( isScalarBarVisible( ind ) );
  else if ( p=="hasChildren")           val = QVariant( hasChildren( ind ) );
  else if ( p=="nbChildren")            val = QVariant( nbChildren( ind ) );
  else if ( p=="isContainer")           val = QVariant( isContainer( ind ) );

  if ( val.isValid() )
    return val;
  else
    return LightApp_Selection::parameter( ind, p );
}

//=======================================================================
//function : parameter
//purpose  :
//=======================================================================
QVariant SMESHGUI_Selection::parameter( const QString& p ) const
{
  QVariant val;
  if ( p=="controlMode" ) val = QVariant( controlMode() );

  if ( val.isValid() )
    return val;
  else
    return LightApp_Selection::parameter( p );
}

//=======================================================================
//function : getVtkOwner
//purpose  : 
//=======================================================================

SMESH_Actor* SMESHGUI_Selection::getActor( int ind ) const
{
  if( ind >= 0 && ind < count() )
    return myActors.isEmpty() ? 0 : myActors.at( ind );
  else
    return 0;
}

//=======================================================================
//function : elemTypes
//purpose  : may return {'Elem0d' 'Edge' 'Face' 'Volume' 'BallElem'} at most
//=======================================================================

QList<QVariant> SMESHGUI_Selection::elemTypes( int ind ) const
{
  QList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    TVisualObjPtr object = actor->GetObject();
    if ( object ) {
      if ( object->GetNbEntities( SMDSAbs_0DElement )) types.append( "Elem0d" );
      if ( object->GetNbEntities( SMDSAbs_Ball ))      types.append( "BallElem" );
      if ( object->GetNbEntities( SMDSAbs_Edge ))      types.append( "Edge" );
      if ( object->GetNbEntities( SMDSAbs_Face ))      types.append( "Face" );
      if ( object->GetNbEntities( SMDSAbs_Volume ))    types.append( "Volume" );
    }
  }
  return types;
}

//=======================================================================
//function : labeledTypes
//purpose  : may return {'Point' 'Cell'} at most
//=======================================================================

QList<QVariant> SMESHGUI_Selection::labeledTypes( int ind ) const
{
  QList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    if ( actor->GetPointsLabeled()) types.append( "Point" );
    if ( actor->GetCellsLabeled())  types.append( "Cell" );
  }
  return types;
}

//=======================================================================
//function : displayMode
//purpose  : return SMESH_Actor::EReperesent
//=======================================================================

QString SMESHGUI_Selection::displayMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    switch( actor->GetRepresentation() ) {
    case SMESH_Actor::eEdge:    return "eEdge";
    case SMESH_Actor::eSurface: return "eSurface";
    case SMESH_Actor::ePoint:   return "ePoint";
    default: break;
    }
  }
  return "Unknown";
}

//=======================================================================
//function : isQuadratic
//purpose  : return true if the mesh has quadratic/bi-quadratic type
//=======================================================================

bool SMESHGUI_Selection::isQuadratic( int ind ) const
{
  _PTR(SObject) so = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
  if ( !so )
    return false;
  SMESH::SMESH_IDSource_var idSource =  SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( so );
  if ( idSource->_is_nil() )
    return false;
  SMESHGUI_ConvToQuadOp::MeshDestinationType meshTgtType = SMESHGUI_ConvToQuadOp::DestinationMesh( idSource );
  if ( meshTgtType & SMESHGUI_ConvToQuadOp::Linear )
    return true;
  return false;
}

//=======================================================================
//function : quadratic2DMode
//purpose  : return SMESH_Actor::EQuadratic2DRepresentation
//=======================================================================
QString SMESHGUI_Selection::quadratic2DMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    switch( actor->GetQuadratic2DRepresentation() ) {
    case SMESH_Actor::eLines: return "eLines";
    case SMESH_Actor::eArcs:  return "eArcs";
    default: break;
    }
  }
  return "Unknown";
}

//=======================================================================
//function : isDistributionVisible
//purpose  : Visible/Invisible distribution of the ScalarBar Actor
//=======================================================================

bool SMESHGUI_Selection::isDistributionVisible(int ind) const {
  SMESH_Actor* actor = getActor( ind );
  return (actor && actor->GetScalarBarActor() && actor->GetScalarBarActor()->GetDistributionVisibility());
} 

//=======================================================================
//function : isScalarBarVisible
//purpose  : Visible/Invisible Scalar Bar
//=======================================================================

bool SMESHGUI_Selection::isScalarBarVisible(int ind) const {
  SMESH_Actor* actor = getActor( ind );
  return (actor && actor->GetScalarBarActor() && actor->GetScalarBarActor()->GetVisibility());
}

//=======================================================================
//function : shrinkMode
//purpose  : return either 'IsSrunk', 'IsNotShrunk' or 'IsNotShrinkable'
//=======================================================================

QString SMESHGUI_Selection::shrinkMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor && actor->IsShrunkable() ) {
    return actor->IsShrunk() ? "IsShrunk" : "IsNotShrunk";
  }
  return "IsNotShrinkable";
}

//=======================================================================
//function : entityMode
//purpose  : may return {'Elem0d' 'Edge' 'Face' 'Volume' 'BallElem' } at most
//=======================================================================

QList<QVariant> SMESHGUI_Selection::entityMode( int ind ) const
{
  QList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    unsigned int aMode = actor->GetEntityMode();
    if ( aMode & SMESH_Actor::eVolumes    ) types.append( "Volume" );
    if ( aMode & SMESH_Actor::eFaces      ) types.append( "Face"   );
    if ( aMode & SMESH_Actor::eEdges      ) types.append( "Edge"   );
    if ( aMode & SMESH_Actor::e0DElements ) types.append( "Elem0d" );
    if ( aMode & SMESH_Actor::eBallElem )   types.append( "BallElem" );
  }
  return types;
}

//=======================================================================
//function : controlMode
//purpose  : return SMESH_Actor::eControl
//=======================================================================

QString SMESHGUI_Selection::controlMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  QString mode = "eNone";
  if ( actor ) {
    switch( actor->GetControlMode() ) {
    case SMESH_Actor::eLength:                mode = "eLength";                break;
    case SMESH_Actor::eLength2D:              mode = "eLength2D";              break;
    case SMESH_Actor::eFreeEdges:             mode = "eFreeEdges";             break;
    case SMESH_Actor::eFreeNodes:             mode = "eFreeNodes";             break;
    case SMESH_Actor::eFreeBorders:           mode = "eFreeBorders";           break;
    case SMESH_Actor::eFreeFaces:             mode = "eFreeFaces";             break;
    case SMESH_Actor::eMultiConnection:       mode = "eMultiConnection";       break;
    case SMESH_Actor::eMultiConnection2D:     mode = "eMultiConnection2D";     break;
    case SMESH_Actor::eArea:                  mode = "eArea";                  break;
    case SMESH_Actor::eVolume3D:              mode = "eVolume3D";              break;
    case SMESH_Actor::eMaxElementLength2D:    mode = "eMaxElementLength2D";    break;
    case SMESH_Actor::eMaxElementLength3D:    mode = "eMaxElementLength3D";    break;
    case SMESH_Actor::eTaper:                 mode = "eTaper";                 break;
    case SMESH_Actor::eAspectRatio:           mode = "eAspectRatio";           break;
    case SMESH_Actor::eAspectRatio3D:         mode = "eAspectRatio3D";         break;
    case SMESH_Actor::eMinimumAngle:          mode = "eMinimumAngle";          break;
    case SMESH_Actor::eWarping:               mode = "eWarping";               break;
    case SMESH_Actor::eSkew:                  mode = "eSkew";                  break;
    case SMESH_Actor::eBareBorderFace:        mode = "eBareBorderFace";        break;
    case SMESH_Actor::eBareBorderVolume:      mode = "eBareBorderVolume";      break;
    case SMESH_Actor::eOverConstrainedFace:   mode = "eOverConstrainedFace";   break;
    case SMESH_Actor::eOverConstrainedVolume: mode = "eOverConstrainedVolume"; break;
    case SMESH_Actor::eCoincidentNodes:       mode = "eCoincidentNodes";       break;
    case SMESH_Actor::eCoincidentElems1D:     mode = "eCoincidentElems1D";     break;
    case SMESH_Actor::eCoincidentElems2D:     mode = "eCoincidentElems2D";     break;
    case SMESH_Actor::eCoincidentElems3D:     mode = "eCoincidentElems3D";     break;
    default:break;
    }
  }
  return mode;
}

//=======================================================================
//function : controlMode
//purpose  : gets global control mode; return SMESH_Actor::eControl
//=======================================================================
QString SMESHGUI_Selection::controlMode() const
{
  if( myControls.count() > 0 ) {
    QString mode = myControls[0];
    for( int ind = 1; ind < myControls.count(); ind++ ) {
      if( mode != myControls[ind] )
        return "eMixed"; // different controls used for different actors
    }
    return mode;
  }
  return "eNone";
}

bool SMESHGUI_Selection::isNumFunctor( int ind ) const
{
  bool result = false;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    switch( actor->GetControlMode() ) {
    case SMESH_Actor::eLength:
    case SMESH_Actor::eLength2D:
    case SMESH_Actor::eMultiConnection:
    case SMESH_Actor::eMultiConnection2D:
    case SMESH_Actor::eArea:
    case SMESH_Actor::eVolume3D:
    case SMESH_Actor::eMaxElementLength2D:
    case SMESH_Actor::eMaxElementLength3D:
    case SMESH_Actor::eTaper:
    case SMESH_Actor::eAspectRatio:
    case SMESH_Actor::eAspectRatio3D:
    case SMESH_Actor::eMinimumAngle:
    case SMESH_Actor::eWarping:
    case SMESH_Actor::eSkew:
      result = true;
      break;
    default:
      break;
    }
  }
  return result;
}

//=======================================================================
//function : facesOrientationMode
//purpose  : 
//=======================================================================

QString SMESHGUI_Selection::facesOrientationMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    return actor->GetFacesOriented() ? "IsOriented" : "IsNotOriented";
  }
  return "Unknown";
}

//=======================================================================
//function : isAutoColor
//purpose  : 
//=======================================================================

bool SMESHGUI_Selection::isAutoColor( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    CORBA::Object_var obj = SMESH::SObjectToObject( sobj, SMESH::GetActiveStudyDocument() );

    if ( !CORBA::is_nil( obj ) ) {
      SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( obj );
      if ( !CORBA::is_nil( mesh ) )
        return mesh->GetAutoColor();
    }
  }
  return false;
}

//=======================================================================
//function : numberOfNodes
//purpose  : this method is actually used to check if an object is empty or not
//=======================================================================

int SMESHGUI_Selection::numberOfNodes( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    CORBA::Object_var obj = SMESH::SObjectToObject( sobj, SMESH::GetActiveStudyDocument() );

    if ( !CORBA::is_nil( obj ) ) {
      SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( obj );
      if ( !CORBA::is_nil( mesh ) )
        return mesh->NbNodes();
      SMESH::SMESH_subMesh_var aSubMeshObj = SMESH::SMESH_subMesh::_narrow( obj );
      if ( !CORBA::is_nil( aSubMeshObj ) )
        return aSubMeshObj->GetNumberOfNodes(true);
      SMESH::SMESH_GroupBase_var aGroupObj = SMESH::SMESH_GroupBase::_narrow( obj );
      if ( !CORBA::is_nil( aGroupObj ) )
        return aGroupObj->IsEmpty() ? 0 : 1; // aGroupObj->Size();
    }
  }
  return 0;
}

//================================================================================
/*!
 * \brief return dimension of elements of the selected object
 *
 *  \retval int - 0 for 0D elements, -1 for an empty object (the rest as usual)
 */
//================================================================================

int SMESHGUI_Selection::dim( int ind ) const
{
  int dim = -1;
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    CORBA::Object_var obj = SMESH::SObjectToObject( sobj, SMESH::GetActiveStudyDocument() );

    if ( !CORBA::is_nil( obj ) ) {
      SMESH::SMESH_IDSource_var idSrc = SMESH::SMESH_IDSource::_narrow( obj );
      if ( !CORBA::is_nil( idSrc ) )
      {
        SMESH::array_of_ElementType_var types = idSrc->GetTypes();
        for ( size_t i = 0; i < types->length(); ++ i) {
          switch ( types[i] ) {
          case SMESH::EDGE  : dim = std::max( dim, 1 ); break;
          case SMESH::FACE  : dim = std::max( dim, 2 ); break;
          case SMESH::VOLUME: dim = std::max( dim, 3 ); break;
          case SMESH::ELEM0D: dim = std::max( dim, 0 ); break;
          case SMESH::BALL  : dim = std::max( dim, 0 ); break;
          default: break;
          }
        }
      }
    }
  }
  return dim;
}

//=======================================================================
//function : isComputable
//purpose  : return true for a ready-to-compute mesh
//=======================================================================

bool SMESHGUI_Selection::isComputable( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && ( myTypes[ind] == "Mesh" ||
                                              myTypes[ind].startsWith("Mesh " )))
  {
    QMap<int,int> modeMap;
    _PTR(SObject) meshSO = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );

    _PTR(SComponent) component = meshSO->GetFatherComponent();
    if ( meshSO->Depth() - component->Depth() > 1 ) // sub-mesh, get a mesh
      while ( meshSO->Depth() - component->Depth() > 1 )
        meshSO = meshSO->GetFather();

    SMESHGUI_PrecomputeOp::getAssignedAlgos( meshSO, modeMap );
    return modeMap.size() > 0;
  }
  return false;
}

//=======================================================================
//function : isPreComputable
//purpose  : returns true for a mesh with algorithms
//=======================================================================

bool SMESHGUI_Selection::isPreComputable( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] == "Mesh" )
  {
    int maxDim = dim( ind );
    if ( maxDim < 2 ) // we can preview 1D or 2D
    {
      QMap<int,int> modeMap;
      _PTR(SObject) pMesh = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
      SMESHGUI_PrecomputeOp::getAssignedAlgos( pMesh, modeMap );
      if ( modeMap.size() > 1 )
        return (( modeMap.contains( SMESH::DIM_3D )) ||
                ( modeMap.contains( SMESH::DIM_2D ) && maxDim < 1 ));
    }
  }
  return false;
}

//=======================================================================
//function : hasGeomReference
//purpose  : returns true for a mesh or sub-mesh on geometry
//=======================================================================

bool SMESHGUI_Selection::hasGeomReference( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    _PTR(SObject) so = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    GEOM::GEOM_Object_var shape = SMESH::GetShapeOnMeshOrSubMesh( so );
    return !shape->_is_nil();
  }
  return false;
}

//=======================================================================
//function : isEditableHyp
//purpose  : 
//=======================================================================

bool SMESHGUI_Selection::isEditableHyp( int ind ) const
{
  bool isEditable = true;
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] == "Hypothesis" )
  {
    _PTR(SObject) so = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    SMESH::SMESH_Hypothesis_var hyp = SMESH::SObjectToInterface<SMESH::SMESH_Hypothesis>( so );
    if ( !hyp->_is_nil() )
    {
      isEditable = hyp->HasParameters();
    }
  }
  return isEditable;
}

//=======================================================================
//function : isVisible
//purpose  : 
//=======================================================================

bool SMESHGUI_Selection::isVisible( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    SMESH_Actor* actor = SMESH::FindActorByEntry( entry( ind ).toLatin1().data() );
    if ( actor && actor->hasIO() ) {
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView() )
        return aViewWindow->isVisible( actor->getIO() );
    }
  }
  return false;
}

//=======================================================================
//function : hasChildren
//purpose  : 
//=======================================================================

bool SMESHGUI_Selection::hasChildren( int ind ) const
{
  if ( ind >= 0 )
  {
    _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    if ( sobj ) 
      return SMESH::GetActiveStudyDocument()->GetUseCaseBuilder()->HasChildren( sobj );
  }
  return false;
}

//=======================================================================
//function : hasChildren
//purpose  : 
//=======================================================================

int SMESHGUI_Selection::nbChildren( int ind ) const
{
  int nb = 0;
  if ( ind >= 0 )
  {
    _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    if ( sobj && sobj->GetStudy()->GetUseCaseBuilder()->IsUseCaseNode( sobj ) ) {
      _PTR(UseCaseIterator) it = sobj->GetStudy()->GetUseCaseBuilder()->GetUseCaseIterator( sobj ); 
      for ( it->Init( false ); it->More(); it->Next() ) nb++;
    }
  }
  return nb;
}

//=======================================================================
//function : isContainer
//purpose  : 
//=======================================================================

bool SMESHGUI_Selection::isContainer( int ind ) const
{
  return ind >= 0 && ind < myTypes.count() && myTypes[ind] == "Unknown";
}

//=======================================================================
//function : type
//purpose  : 
//=======================================================================

int SMESHGUI_Selection::type( const QString& entry, _PTR(Study) study )
{
  int res = -1;
  _PTR(SObject) obj = study->FindObjectID( entry.toLatin1().data() );
  if ( obj ) {
    _PTR(SObject) ref;
    if ( obj->ReferencedObject( ref ) )
      obj = ref;

    _PTR(SObject) objFather = obj->GetFather();
    _PTR(SComponent) objComponent = obj->GetFatherComponent();

    if ( objComponent->ComponentDataType() == "SMESH" ) {
      if ( objComponent->GetIOR() == obj->GetIOR() ) {
        res = SMESH::COMPONENT;
      }
      else {
        int aLevel = obj->Depth() - objComponent->Depth(),
          aFTag = objFather->Tag(),
          anOTag = obj->Tag();

        switch ( aLevel )
        {
        case 1:
          if ( anOTag >= SMESH::Tag_FirstMeshRoot )
            res = SMESH::MESH;
          break;
        case 2:
          switch ( aFTag )
          {
          case SMESH::Tag_HypothesisRoot: res = SMESH::HYPOTHESIS; break;
          case SMESH::Tag_AlgorithmsRoot: res = SMESH::ALGORITHM;  break;
          default: break;
          }
          break;
        case 3:
          switch ( aFTag )
          {
          case SMESH::Tag_SubMeshOnVertex:   res = SMESH::SUBMESH_VERTEX;   break;
          case SMESH::Tag_SubMeshOnEdge:     res = SMESH::SUBMESH_EDGE;     break;
          case SMESH::Tag_SubMeshOnFace:     res = SMESH::SUBMESH_FACE;     break;
          case SMESH::Tag_SubMeshOnSolid:    res = SMESH::SUBMESH_SOLID;    break;
          case SMESH::Tag_SubMeshOnCompound: res = SMESH::SUBMESH_COMPOUND; break;
          default:
            if ( aFTag >= SMESH::Tag_FirstGroup) res = SMESH::GROUP;
            else                                 res = SMESH::SUBMESH;
            break;
          }
          break;
        }
      }
    }
  }
  return res;
}

//=======================================================================
//function : typeName
//purpose  : 
//=======================================================================

QString SMESHGUI_Selection::typeName( const int t )
{
  QString res = "Unknown";
  switch( t )
  {
  case SMESH::HYPOTHESIS:
    res = "Hypothesis"; break;
  case SMESH::ALGORITHM:
    res = "Algorithm"; break;
  case SMESH::MESH:
    res = "Mesh"; break;
  case SMESH::SUBMESH:
    res = "SubMesh"; break;
  case SMESH::MESHorSUBMESH:
    res = "Mesh or submesh"; break;
  case SMESH::SUBMESH_VERTEX:
    res = "Mesh vertex"; break;
  case SMESH::SUBMESH_EDGE:
    res = "Mesh edge"; break;
  case SMESH::SUBMESH_FACE:
    res = "Mesh face"; break;
  case SMESH::SUBMESH_SOLID:
    res = "Mesh solid"; break;
  case SMESH::SUBMESH_COMPOUND:
    res = "Mesh compound"; break;
  case SMESH::GROUP:
    res = "Group"; break;
  case SMESH::COMPONENT:
    res = "Component"; break;
  default:
     break;
  }
  return res;
}

bool SMESHGUI_Selection::isImported( const int ind ) const
{
  bool res = false;
  _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().constData() );
  if ( sobj )
  {
    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( SMESH::SObjectToObject( sobj ) );
    if( !aMesh->_is_nil() )
    {
      SMESH::MedFileInfo_var inf = aMesh->GetMEDFileInfo();
      res = strlen( (char*)inf->fileName ) > 0;
    }
  }
  return res;
}

//=======================================================================
//function : groupType
//purpose  : 
//=======================================================================

QString SMESHGUI_Selection::groupType( int ind ) const
{
  _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().constData() );
  if ( sobj )
  {
    SMESH::SMESH_Group_var g = SMESH::SObjectToInterface<SMESH::SMESH_Group>( sobj );
    if ( !CORBA::is_nil( g ) )
      return "Group";
    SMESH::SMESH_GroupOnGeom_var gog = SMESH::SObjectToInterface<SMESH::SMESH_GroupOnGeom>( sobj );
    if( !CORBA::is_nil( gog ) )
      return "GroupOnGeom";
    SMESH::SMESH_GroupOnFilter_var gof = SMESH::SObjectToInterface<SMESH::SMESH_GroupOnFilter>( sobj );
    if ( !CORBA::is_nil( gof ) )
      return "GroupOnFilter";
  }
  return "";
}
