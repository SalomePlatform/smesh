
#include "SMESHGUI_Selection.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SMESH_Type.h"
#include "SMESH_Actor.h"

#include "SalomeApp_SelectionMgr.h"
#include "SalomeApp_Study.h"
#include "SalomeApp_VTKSelector.h"

#include "SUIT_Session.h"

#include "SVTK_RenderWindowInteractor.h"
#include "SVTK_ViewWindow.h"

#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

//=======================================================================
//function : SMESHGUI_Selection
//purpose  : 
//=======================================================================
SMESHGUI_Selection::SMESHGUI_Selection()
: SalomeApp_Selection()
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
void SMESHGUI_Selection::init( const QString& client, SalomeApp_SelectionMgr* mgr )
{
  SalomeApp_Selection::init( client, mgr );

  if( mgr && study() )
  {
    _PTR(Study) aStudy = study()->studyDS();

    SUIT_DataOwnerPtrList sel;
    mgr->selected( sel, client );
    myDataOwners = sel;
    SUIT_DataOwnerPtrList::const_iterator anIt = sel.begin(),
                                          aLast = sel.end();
    for( ; anIt!=aLast; anIt++ )
    {
      SUIT_DataOwner* owner = ( SUIT_DataOwner* )( (*anIt ).get() );
      SalomeApp_DataOwner* sowner = dynamic_cast<SalomeApp_DataOwner*>( owner );
      if( sowner )
        myTypes.append( typeName( type( sowner, aStudy ) ) );
      else
        myTypes.append( "Unknown" );
    }
  }
}

//=======================================================================
//function : param
//purpose  : 
//=======================================================================
QtxValue SMESHGUI_Selection::param( const int ind, const QString& p ) const
{
  QtxValue val;
       if ( p=="client" )        val = QtxValue( globalParam( p ) );
  else if ( p=="type" )          val = QtxValue( myTypes[ind] );
  else if ( p=="elemTypes" )     val = QtxValue( elemTypes( ind ) );
  else if ( p=="numberOfNodes" ) val = QtxValue( numberOfNodes( ind ) );
  else if ( p=="labeledTypes" )  val = QtxValue( labeledTypes( ind ) );
  else if ( p=="shrinkMode" )    val = QtxValue( shrinkMode( ind ) );
  else if ( p=="entityMode" )    val = QtxValue( entityMode( ind ) );
  else if ( p=="controlMode" )   val = QtxValue( controlMode( ind ) );
  else if ( p=="displayMode" )   val = QtxValue( displayMode( ind ) );
  else if ( p=="isComputable" )  val = QtxValue( isComputable( ind ) );
  else if ( p=="hasReference" )  val = QtxValue( hasReference( ind ) );
  else if ( p=="isVisible" )     val = QtxValue( isVisible( ind ) );

  // printf( "--> param() : [%s] = %s (%s)\n", p.latin1(), val.toString().latin1(), val.typeName() );
  //if ( val.type() == QVariant::List )
  //cout << "size: " << val.toList().count() << endl;
  return val;
}

//=======================================================================
//function : getVtkOwner
//purpose  : 
//=======================================================================

SMESH_Actor* SMESHGUI_Selection::getActor( int ind ) const
{
  if ( ind >= 0 && ind < myDataOwners.count() ) {
    const SalomeApp_SVTKDataOwner* owner = 
      dynamic_cast<const SalomeApp_SVTKDataOwner*> ( myDataOwners[ ind ].get() );
    if ( owner )    
      return dynamic_cast<SMESH_Actor*>( owner->GetActor() );
  }
  return 0;
}

//=======================================================================
//function : elemTypes
//purpose  : may return {'Edge' 'Face' 'Volume'} at most
//=======================================================================

QValueList<QVariant> SMESHGUI_Selection::elemTypes( int ind ) const
{
  QValueList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    TVisualObjPtr object = actor->GetObject();
    if ( object ) {
      if ( object->GetNbEntities( SMDSAbs_Edge )) types.append( "Edge" );
      if ( object->GetNbEntities( SMDSAbs_Face )) types.append( "Face" );
      if ( object->GetNbEntities( SMDSAbs_Volume )) types.append( "Volume" );
    }
  }
  return types;
}

//=======================================================================
//function : labeledTypes
//purpose  : may return {'Point' 'Cell'} at most
//=======================================================================

QValueList<QVariant> SMESHGUI_Selection::labeledTypes( int ind ) const
{
  QValueList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    if ( actor->GetPointsLabeled()) types.append( "Point" );
    if ( actor->GetCellsLabeled()) types.append( "Cell" );
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
    default:;
    }
  }
  return "Unknown";
}

//=======================================================================
//function : shrinkMode
//purpose  : return either 'IsSrunk', 'IsNotShrunk' or 'IsNotShrinkable'
//=======================================================================

QString SMESHGUI_Selection::shrinkMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor && actor->IsShrunkable() ) {
    if ( actor->IsShrunk() )
      return "IsShrunk";
    return "IsNotShrunk";
  }
  return "IsNotShrinkable";
}

//=======================================================================
//function : entityMode
//purpose  : may return {'Edge' 'Face' 'Volume'} at most
//=======================================================================

QValueList<QVariant> SMESHGUI_Selection::entityMode( int ind ) const
{
  QValueList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    unsigned int aMode = actor->GetEntityMode();
    if ( aMode & SMESH_Actor::eVolumes) types.append( "Volume");
    if ( aMode & SMESH_Actor::eFaces  ) types.append( "Face"  );
    if ( aMode & SMESH_Actor::eEdges  ) types.append( "Edge"  );
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
  if ( actor ) {
    switch( actor->GetControlMode() ) {
    case SMESH_Actor::eLength:            return "eLength";
    case SMESH_Actor::eLength2D:          return "eLength2D";
    case SMESH_Actor::eFreeEdges:         return "eFreeEdges";
    case SMESH_Actor::eFreeBorders:       return "eFreeBorders";
    case SMESH_Actor::eMultiConnection:   return "eMultiConnection";
    case SMESH_Actor::eMultiConnection2D: return "eMultiConnection2D";
    case SMESH_Actor::eArea:              return "eArea";
    case SMESH_Actor::eTaper:             return "eTaper";
    case SMESH_Actor::eAspectRatio:       return "eAspectRatio";
    case SMESH_Actor::eAspectRatio3D:     return "eAspectRatio3D";
    case SMESH_Actor::eMinimumAngle:      return "eMinimumAngle";
    case SMESH_Actor::eWarping:           return "eWarping";
    case SMESH_Actor::eSkew:              return "eSkew";
    default:;
    }
  }
  return "eNone";
}

//=======================================================================
//function : numberOfNodes
//purpose  : 
//=======================================================================

int SMESHGUI_Selection::numberOfNodes( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    CORBA::Object_var obj =
      SMESH::DataOwnerToObject( static_cast<SalomeApp_DataOwner*>( myDataOwners[ ind ].get() ));
    if ( ! CORBA::is_nil( obj )) {
      SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( obj );
      if ( ! mesh->_is_nil() )
        return mesh->NbNodes();
      SMESH::SMESH_subMesh_var aSubMeshObj = SMESH::SMESH_subMesh::_narrow( obj );
      if ( !aSubMeshObj->_is_nil() )
        return aSubMeshObj->GetNumberOfNodes(true);
      SMESH::SMESH_GroupBase_var aGroupObj = SMESH::SMESH_GroupBase::_narrow( obj );
      if ( !aGroupObj->_is_nil() )
        return aGroupObj->Size();
    }
  }
  return 0;
}

//=======================================================================
//function : isComputable
//purpose  : 
//=======================================================================

QVariant SMESHGUI_Selection::isComputable( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    Handle(SALOME_InteractiveObject) io =
      static_cast<SalomeApp_DataOwner*>( myDataOwners[ ind ].get() )->IO();
    if ( !io.IsNull() ) {
      SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO(io) ; // m,sm,gr->m
      if ( !mesh->_is_nil() ) {
        _PTR(SObject) so = SMESH::FindSObject( mesh );
        if ( so ) {
          GEOM::GEOM_Object_var shape = SMESH::GetShapeOnMeshOrSubMesh( so );
          return QVariant( !shape->_is_nil(), 0 );
        }
      }
    }
  }
  return QVariant( false, 0 );
}

//=======================================================================
//function : hasReference
//purpose  : 
//=======================================================================

QVariant SMESHGUI_Selection::hasReference( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    SalomeApp_DataOwner* owner = dynamic_cast<SalomeApp_DataOwner*>( myDataOwners[ ind ].operator->() );
    if( owner )
    {
      _PTR(SObject) obj ( study()->studyDS()->FindObjectID( owner->entry().latin1() ) ), ref;
      return QVariant( obj->ReferencedObject( ref ), 0 );
    }
  }
  return QVariant( false, 0 );
}

//=======================================================================
//function : isVisible
//purpose  : 
//=======================================================================

QVariant SMESHGUI_Selection::isVisible( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    QString entry = static_cast<SalomeApp_DataOwner*>( myDataOwners[ ind ].get() )->entry();
    SMESH_Actor* actor = SMESH::FindActorByEntry( entry.latin1() );
    if ( actor && actor->hasIO() ) {
      SVTK_RenderWindowInteractor* renderInter = SMESH::GetCurrentVtkView()->getRWInteractor();
      return QVariant( renderInter->isVisible( actor->getIO() ), 0 );
    }
  }
  return QVariant( false, 0 );
}


//=======================================================================
//function : type
//purpose  :
//=======================================================================
int SMESHGUI_Selection::type( SalomeApp_DataOwner* owner, _PTR(Study) study )
{
  return type( owner->entry(), study );
}

//=======================================================================
//function : type
//purpose  : 
//=======================================================================

int SMESHGUI_Selection::type( const QString& entry, _PTR(Study) study )
{
  _PTR(SObject) obj (study->FindObjectID(entry.latin1()));
  if( !obj )
    return -1;

  _PTR(SObject) ref;
  if( obj->ReferencedObject( ref ) )
    obj = ref;

  _PTR(SObject) objFather = obj->GetFather();
  _PTR(SComponent) objComponent = obj->GetFatherComponent();

  if( objComponent->ComponentDataType()!="SMESH" )
    return -1;

  int aLevel = obj->Depth() - objComponent->Depth(),
      aFTag = objFather->Tag(),
      anOTag = obj->Tag(),
      res = -1;

  switch( aLevel )
  {
  case 1:
    if( anOTag>=3 )
      res = MESH;
    break;
  case 2:
    switch( aFTag )
    {
    case 1:
      res = HYPOTHESIS;
      break;
    case 2:
      res = ALGORITHM;
      break;
    }
    break;
  case 3:
    switch( aFTag )
    {
    case 4:
      res = SUBMESH_VERTEX;
      break;
    case 5:
      res = SUBMESH_EDGE;
      break;
    case 7:
      res = SUBMESH_FACE;
      break;
    case 9:
      res = SUBMESH_SOLID;
      break;
    case 10:
      res = SUBMESH_COMPOUND;
      break;
    }
    if( aFTag>10 )
      res = GROUP;

    break;
  }

  return res;
}

//=======================================================================
//function : typeName
//purpose  : 
//=======================================================================

QString SMESHGUI_Selection::typeName( const int t )
{
  switch( t )
  {
  case HYPOTHESIS:
    return "Hypothesis";
  case ALGORITHM:
    return "Algorithm";
  case MESH:
    return "Mesh";
  case SUBMESH:
    return "SubMesh";
  case MESHorSUBMESH:
    return "Mesh or submesh";
  case SUBMESH_VERTEX:
    return "Mesh vertex";
  case SUBMESH_EDGE:
    return "Mesh edge";
  case SUBMESH_FACE:
    return "Mesh face";
  case SUBMESH_SOLID:
    return "Mesh solid";
  case SUBMESH_COMPOUND:
    return "Mesh compound";
  case GROUP:
    return "Group";
  default:
    return "Unknown";
  }
}
