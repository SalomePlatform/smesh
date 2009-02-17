//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI_Selection
// File   : SMESHGUI_Selection.cxx
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Selection.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include <SMESH_Type.h>
#include <SMESH_Actor.h>

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

    for( int i=0, n=count(); i<n; i++ )
      myTypes.append( typeName( type( entry( i ), aStudy ) ) );
  }
}

//=======================================================================
//function : processOwner
//purpose  : 
//=======================================================================
void SMESHGUI_Selection::processOwner( const LightApp_DataOwner* ow )
{
  const LightApp_SVTKDataOwner* owner =
    dynamic_cast<const LightApp_SVTKDataOwner*> ( ow );
  if( owner )
    myActors.append( dynamic_cast<SMESH_Actor*>( owner->GetActor() ) );
  else
    myActors.append( 0 );
}

//=======================================================================
//function : parameter
//purpose  : 
//=======================================================================
QVariant SMESHGUI_Selection::parameter( const int ind, const QString& p ) const
{
  QVariant val;
  if      ( p=="client" )        val = QVariant( LightApp_Selection::parameter( p ) );
  else if ( p=="type" )          val = QVariant( myTypes[ind] );
  else if ( p=="elemTypes" )     val = QVariant( elemTypes( ind ) );
  else if ( p=="isAutoColor" )   val = QVariant( isAutoColor( ind ) );
  else if ( p=="numberOfNodes" ) val = QVariant( numberOfNodes( ind ) );
  else if ( p=="labeledTypes" )  val = QVariant( labeledTypes( ind ) );
  else if ( p=="shrinkMode" )    val = QVariant( shrinkMode( ind ) );
  else if ( p=="entityMode" )    val = QVariant( entityMode( ind ) );
  else if ( p=="controlMode" )   val = QVariant( controlMode( ind ) );
  else if ( p=="displayMode" )   val = QVariant( displayMode( ind ) );
  else if ( p=="isComputable" )  val = QVariant( isComputable( ind ) );
  else if ( p=="hasReference" )  val = QVariant( hasReference( ind ) );
  else if ( p=="isImported" )    val = QVariant( isImported( ind ) );
  else if ( p=="facesOrientationMode" ) val = QVariant( facesOrientationMode( ind ) );
  else if ( p=="groupType" )     val = QVariant( groupType( ind ) );

  if( val.isValid() )
    return val;
  else
    return LightApp_Selection::parameter( ind, p );
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
//purpose  : may return {'Edge' 'Face' 'Volume'} at most
//=======================================================================

QList<QVariant> SMESHGUI_Selection::elemTypes( int ind ) const
{
  QList<QVariant> types;
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

QList<QVariant> SMESHGUI_Selection::labeledTypes( int ind ) const
{
  QList<QVariant> types;
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
    default: break;
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

QList<QVariant> SMESHGUI_Selection::entityMode( int ind ) const
{
  QList<QVariant> types;
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
    case SMESH_Actor::eFreeFaces:         return "eFreeFaces";
    case SMESH_Actor::eMultiConnection:   return "eMultiConnection";
    case SMESH_Actor::eMultiConnection2D: return "eMultiConnection2D";
    case SMESH_Actor::eArea:              return "eArea";
    case SMESH_Actor::eVolume3D:          return "eVolume3D";
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
//function : facesOrientationMode
//purpose  : 
//=======================================================================

QString SMESHGUI_Selection::facesOrientationMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    if ( actor->GetFacesOriented() )
      return "IsOriented";
    return "IsNotOriented";
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

    if ( ! CORBA::is_nil( obj )) {
      SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( obj );
      if ( ! mesh->_is_nil() )
        return mesh->GetAutoColor();
    }
  }
  return false;
}

//=======================================================================
//function : numberOfNodes
//purpose  : 
//=======================================================================

int SMESHGUI_Selection::numberOfNodes( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    _PTR(SObject) sobj = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    CORBA::Object_var obj = SMESH::SObjectToObject( sobj, SMESH::GetActiveStudyDocument() );

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
/*    Handle(SALOME_InteractiveObject) io =
      static_cast<LightApp_DataOwner*>( myDataOwners[ ind ].get() )->IO();
    if ( !io.IsNull() ) {
      SMESH::SMESH_Mesh_var mesh = SMESH::GetMeshByIO(io); // m,sm,gr->m
      if ( !mesh->_is_nil() ) {*/
        _PTR(SObject) so = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
	//FindSObject( mesh );
        if ( so ) {
          CORBA::Object_var obj = SMESH::SObjectToObject(so, SMESH::GetActiveStudyDocument());
          if(!CORBA::is_nil(obj)){
            SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( obj );
            if (!mesh->_is_nil()){
              if(mesh->HasShapeToMesh()) {
                GEOM::GEOM_Object_var shape = SMESH::GetShapeOnMeshOrSubMesh( so );
                return QVariant( !shape->_is_nil() );
              }
              else
              {
                return QVariant(!mesh->NbFaces()==0);
              }
            }
            else
            {
              GEOM::GEOM_Object_var shape = SMESH::GetShapeOnMeshOrSubMesh( so );
              return QVariant( !shape->_is_nil() );
            }
          }
        }
//      }
//    }
  }
  return QVariant( false );
}

//=======================================================================
//function : hasReference
//purpose  : 
//=======================================================================

QVariant SMESHGUI_Selection::hasReference( int ind ) const
{
  return QVariant( isReference( ind ) );
}

//=======================================================================
//function : isVisible
//purpose  : 
//=======================================================================

QVariant SMESHGUI_Selection::isVisible( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    QString ent = entry( ind );
    SMESH_Actor* actor = SMESH::FindActorByEntry( ent.toLatin1().data() );
    if ( actor && actor->hasIO() ) {
      if(SVTK_ViewWindow* aViewWindow = SMESH::GetCurrentVtkView())
	return QVariant( aViewWindow->isVisible( actor->getIO() ) );
    }
  }
  return QVariant( false );
}

//=======================================================================
//function : type
//purpose  : 
//=======================================================================

int SMESHGUI_Selection::type( const QString& entry, _PTR(Study) study )
{
  _PTR(SObject) obj (study->FindObjectID(entry.toLatin1().data()));
  if( !obj )
    return -1;

  _PTR(SObject) ref;
  if( obj->ReferencedObject( ref ) )
    obj = ref;

  _PTR(SObject) objFather = obj->GetFather();
  _PTR(SComponent) objComponent = obj->GetFatherComponent();

  if( objComponent->ComponentDataType()!="SMESH" )
    return -1;

  if( objComponent->GetIOR()==obj->GetIOR() )
    return COMPONENT;

  int aLevel = obj->Depth() - objComponent->Depth(),
      aFTag = objFather->Tag(),
      anOTag = obj->Tag(),
      res = -1;

  switch (aLevel)
  {
  case 1:
    if (anOTag >= SMESH::Tag_FirstMeshRoot)
      res = MESH;
    break;
  case 2:
    switch (aFTag)
    {
    case SMESH::Tag_HypothesisRoot:
      res = HYPOTHESIS;
      break;
    case SMESH::Tag_AlgorithmsRoot:
      res = ALGORITHM;
      break;
    }
    break;
  case 3:
    switch (aFTag)
    {
    case SMESH::Tag_SubMeshOnVertex:
      res = SUBMESH_VERTEX;
      break;
    case SMESH::Tag_SubMeshOnEdge:
      res = SUBMESH_EDGE;
      break;
    case SMESH::Tag_SubMeshOnFace:
      res = SUBMESH_FACE;
      break;
    case SMESH::Tag_SubMeshOnSolid:
      res = SUBMESH_SOLID;
      break;
    case SMESH::Tag_SubMeshOnCompound:
      res = SUBMESH_COMPOUND;
      break;
    default:
      if (aFTag >= SMESH::Tag_FirstGroup)
        res = GROUP;
      else
        res = SUBMESH;
    }
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
  case COMPONENT:
    return "Component";
  default:
    return "Unknown";
  }
}

bool SMESHGUI_Selection::isImported( const int ind ) const
{
  QString e = entry( ind );
  _PTR(SObject) SO = SMESH::GetActiveStudyDocument()->FindObjectID( e.toLatin1().constData() );
  bool res = false;
  /*
  if( SO )
  {
    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( SMESH::SObjectToObject( SO ) );
    if( !aMesh->_is_nil() )
    {
      SALOME_MED::MedFileInfo* inf = aMesh->GetMEDFileInfo();
      res = strlen( (char*)inf->fileName ) > 0;
    }
  }
  */
  return res;
}

//=======================================================================
//function : groupType
//purpose  : 
//=======================================================================

QString SMESHGUI_Selection::groupType( int ind ) const
{
  QString e = entry( ind );
  _PTR(SObject) SO = SMESH::GetActiveStudyDocument()->FindObjectID( e.toLatin1().constData() );
  QString type;
  if( SO )
  {
    CORBA::Object_var obj = SMESH::SObjectToObject( SO );
  
    SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_narrow( obj );
    SMESH::SMESH_GroupOnGeom_var aGroupOnGeom = SMESH::SMESH_GroupOnGeom::_narrow( obj );
    if( !aGroup->_is_nil() )
      type = QString( "Group" );
    else if ( !aGroupOnGeom->_is_nil() )
      type = QString( "GroupOnGeom" );
  }
  return type;
}
