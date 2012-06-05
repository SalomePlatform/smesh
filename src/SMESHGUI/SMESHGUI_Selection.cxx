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

// SMESH SMESHGUI_Selection
// File   : SMESHGUI_Selection.cxx
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Selection.h"

#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_ComputeDlg.h"

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

    for( int i=0, n=count(); i<n; i++ )
      myTypes.append( typeName( type( entry( i ), aStudy ) ) );
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
  if( owner )
    myActors.append( dynamic_cast<SMESH_Actor*>( owner->GetActor() ) );
  else
    myActors.append( 0 );
  return true;
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
  else if ( p=="dim" )           val = QVariant( dim( ind ) );
  else if ( p=="labeledTypes" )  val = QVariant( labeledTypes( ind ) );
  else if ( p=="shrinkMode" )    val = QVariant( shrinkMode( ind ) );
  else if ( p=="entityMode" )    val = QVariant( entityMode( ind ) );
  else if ( p=="controlMode" )   val = QVariant( controlMode( ind ) );
  else if ( p=="isNumFunctor" )  val = QVariant( isNumFunctor( ind ) );
  else if ( p=="displayMode" )   val = QVariant( displayMode( ind ) );
  else if ( p=="isComputable" )  val = QVariant( isComputable( ind ) );
  else if ( p=="isPreComputable" )  val = QVariant( isPreComputable( ind ) );
  else if ( p=="hasReference" )  val = QVariant( hasReference( ind ) );
  else if ( p=="isImported" )    val = QVariant( isImported( ind ) );
  else if ( p=="facesOrientationMode" ) val = QVariant( facesOrientationMode( ind ) );
  else if ( p=="groupType" )     val = QVariant( groupType( ind ) );
  else if ( p=="quadratic2DMode") val =  QVariant(quadratic2DMode(ind));
  else if ( p=="isDistributionVisible") val = QVariant(isDistributionVisible(ind));

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
//purpose  : may return {'Elem0d' 'Edge' 'Face' 'Volume'} at most
//=======================================================================

QList<QVariant> SMESHGUI_Selection::elemTypes( int ind ) const
{
  QList<QVariant> types;
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    TVisualObjPtr object = actor->GetObject();
    if ( object ) {
      if ( object->GetNbEntities( SMDSAbs_0DElement )) types.append( "Elem0d" );
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
//function : quadratic2DMode
//purpose  : return SMESH_Actor::EQuadratic2DRepresentation
//=======================================================================
QString SMESHGUI_Selection::quadratic2DMode( int ind ) const
{
  SMESH_Actor* actor = getActor( ind );
  if ( actor ) {
    switch( actor->GetQuadratic2DRepresentation() ) {
    case SMESH_Actor::eLines:    return "eLines";
    case SMESH_Actor::eArcs: return "eArcs";
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
//purpose  : may return {'Elem0d' 'Edge' 'Face' 'Volume'} at most
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

    if ( ! CORBA::is_nil( obj )) {
      SMESH::SMESH_IDSource_var idSrc = SMESH::SMESH_IDSource::_narrow( obj );
      if ( ! idSrc->_is_nil() )
      {
        SMESH::array_of_ElementType_var types = idSrc->GetTypes();
        for ( int i = 0; i < types->length(); ++ i)
          switch ( types[i] ) {
          case SMESH::EDGE  : dim = std::max( dim, 1 ); break;
          case SMESH::FACE  : dim = std::max( dim, 2 ); break;
          case SMESH::VOLUME: dim = std::max( dim, 3 ); break;
          case SMESH::ELEM0D: dim = std::max( dim, 0 ); break;
          default:;
          }
      }
    }
  }
  return dim;
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
//function : isPreComputable
//purpose  : 
//=======================================================================

QVariant SMESHGUI_Selection::isPreComputable( int ind ) const
{
  if ( ind >= 0 && ind < myTypes.count() && myTypes[ind] != "Unknown" )
  {
    QMap<int,int> modeMap;
    _PTR(SObject) pMesh = SMESH::GetActiveStudyDocument()->FindObjectID( entry( ind ).toLatin1().data() );
    SMESHGUI_PrecomputeOp::getAssignedAlgos( pMesh, modeMap );
    return QVariant( modeMap.size() > 1 );
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
  if( SO )
  {
    SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( SMESH::SObjectToObject( SO ) );
    if( !aMesh->_is_nil() )
    {
      SALOME_MED::MedFileInfo* inf = aMesh->GetMEDFileInfo();
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
  QString e = entry( ind );
  _PTR(SObject) SO = SMESH::GetActiveStudyDocument()->FindObjectID( e.toLatin1().constData() );
  if( SO )
  {
    SMESH::SMESH_Group_var g = SMESH::SObjectToInterface<SMESH::SMESH_Group>( SO );
    if( !g->_is_nil() )
      return "Group";

    SMESH::SMESH_GroupOnGeom_var gog = SMESH::SObjectToInterface<SMESH::SMESH_GroupOnGeom>( SO );
    if( !gog->_is_nil() )
      return "GroupOnGeom";

    SMESH::SMESH_GroupOnFilter_var gof = SMESH::SObjectToInterface<SMESH::SMESH_GroupOnFilter>(SO);
    if ( !gof->_is_nil() )
      return "GroupOnFilter";
  }
  return "";
}
