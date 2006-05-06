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
// File:      SMESH_MesherHelper.cxx
// Created:   15.02.06 15:22:41
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2006


#include "SMESH_MesherHelper.hxx"

#include "SMDS_FacePosition.hxx" 
#include "SMDS_EdgePosition.hxx"
#include "SMESH_MeshEditor.hxx"

#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <gp_Pnt2d.hxx>
#include <ShapeAnalysis.hxx>

//=======================================================================
//function : CheckShape
//purpose  : 
//=======================================================================

bool SMESH_MesherHelper::IsQuadraticSubMesh(const TopoDS_Shape& aSh)
{
  SMESHDS_Mesh* meshDS = GetMesh()->GetMeshDS();
  // we can create quadratic elements only if all elements
  // created on subshapes of given shape are quadratic
  // also we have to fill myNLinkNodeMap
  myCreateQuadratic = true;
  mySeamShapeIds.clear();
  TopAbs_ShapeEnum subType( aSh.ShapeType()==TopAbs_FACE ? TopAbs_EDGE : TopAbs_FACE );
  SMDSAbs_ElementType elemType( subType==TopAbs_FACE ? SMDSAbs_Face : SMDSAbs_Edge );

  TopExp_Explorer exp( aSh, subType );
  for (; exp.More() && myCreateQuadratic; exp.Next()) {
    if ( SMESHDS_SubMesh * subMesh = meshDS->MeshElements( exp.Current() )) {
      if ( SMDS_ElemIteratorPtr it = subMesh->GetElements() ) {
        while(it->more()) {
          const SMDS_MeshElement* e = it->next();
          if ( e->GetType() != elemType || !e->IsQuadratic() ) {
            myCreateQuadratic = false;
            break;
          }
          else {
            // fill NLinkNodeMap
            switch ( e->NbNodes() ) {
            case 3:
              AddNLinkNode(e->GetNode(0),e->GetNode(1),e->GetNode(2)); break;
            case 6:
              AddNLinkNode(e->GetNode(0),e->GetNode(1),e->GetNode(3));
              AddNLinkNode(e->GetNode(1),e->GetNode(2),e->GetNode(4));
              AddNLinkNode(e->GetNode(2),e->GetNode(0),e->GetNode(5)); break;
            case 8:
              AddNLinkNode(e->GetNode(0),e->GetNode(1),e->GetNode(4));
              AddNLinkNode(e->GetNode(1),e->GetNode(2),e->GetNode(5));
              AddNLinkNode(e->GetNode(2),e->GetNode(3),e->GetNode(6));
              AddNLinkNode(e->GetNode(3),e->GetNode(0),e->GetNode(7));
              break;
            default:
              myCreateQuadratic = false;
              break;
            }
          }
        }
      }
    }
  }

  if(!myCreateQuadratic) {
    myNLinkNodeMap.clear();
  }
  else {
    SetSubShape( aSh );
  }
  return myCreateQuadratic;
}

//================================================================================
/*!
 * \brief Set geomerty to make elements on
  * \param aSh - geomertic shape
 */
//================================================================================

void SMESH_MesherHelper::SetSubShape(const int aShID)
{
  if ( aShID == myShapeID )
    return;
  if ( aShID > 1 )
    SetSubShape( GetMesh()->GetMeshDS()->IndexToShape( aShID ));
  else
    SetSubShape( TopoDS_Shape() );
}

//================================================================================
/*!
 * \brief Set geomerty to make elements on
  * \param aSh - geomertic shape
 */
//================================================================================

void SMESH_MesherHelper::SetSubShape(const TopoDS_Shape& aSh)
{
  if ( !myShape.IsNull() && !aSh.IsNull() && myShape.IsSame( aSh ))
    return;

  myShape = aSh;
  mySeamShapeIds.clear();

  if ( myShape.IsNull() ) {
    myShapeID  = -1;
    return;
  }
  SMESHDS_Mesh* meshDS = GetMesh()->GetMeshDS();
  myShapeID = meshDS->ShapeToIndex(aSh);

  // treatment of periodic faces
  if ( aSh.ShapeType() == TopAbs_FACE )
  {
    const TopoDS_Face& face = TopoDS::Face( aSh );
    BRepAdaptor_Surface surface( face );
    if ( surface.IsUPeriodic() || surface.IsVPeriodic() )
    {
      // look for a seam edge
      for ( TopExp_Explorer exp( face, TopAbs_EDGE ); exp.More(); exp.Next()) {
        const TopoDS_Edge& edge = TopoDS::Edge( exp.Current() );
        if ( BRep_Tool::IsClosed( edge, face )) {
          // initialize myPar1, myPar2 and myParIndex
          if ( mySeamShapeIds.empty() ) {
            gp_Pnt2d uv1, uv2;
            BRep_Tool::UVPoints( edge, face, uv1, uv2 );
            if ( Abs( uv1.Coord(1) - uv2.Coord(1) ) < Abs( uv1.Coord(2) - uv2.Coord(2) ))
            {
              myParIndex = 1; // U periodic
              myPar1 = surface.FirstUParameter();
              myPar2 = surface.LastUParameter();
            }
            else {
              myParIndex = 2;  // V periodic
              myPar1 = surface.FirstVParameter();
              myPar2 = surface.LastVParameter();
            }
          }
          // store shapes indices
          mySeamShapeIds.insert( meshDS->ShapeToIndex( exp.Current() ));
          for ( TopExp_Explorer v( exp.Current(), TopAbs_VERTEX ); v.More(); v.Next() )
            mySeamShapeIds.insert( meshDS->ShapeToIndex( v.Current() ));
        }
      }
    }
  }
}

//================================================================================
  /*!
   * \brief Check if inFaceNode argument is necessary for call GetNodeUV(F,..)
    * \param F - the face
    * \retval bool - return true if the face is periodic
   */
//================================================================================

bool SMESH_MesherHelper::GetNodeUVneedInFaceNode(const TopoDS_Face& F) const
{
  if ( F.IsNull() ) return !mySeamShapeIds.empty();

  if ( !F.IsNull() && !myShape.IsNull() && myShape.IsSame( F ))
    return !mySeamShapeIds.empty();

  Handle(Geom_Surface) aSurface = BRep_Tool::Surface( F );
  if ( !aSurface.IsNull() )
    return ( aSurface->IsUPeriodic() || aSurface->IsVPeriodic() );

  return false;
}

//=======================================================================
//function : IsMedium
//purpose  : 
//=======================================================================

bool SMESH_MesherHelper::IsMedium(const SMDS_MeshNode*      node,
                                 const SMDSAbs_ElementType typeToCheck)
{
  return SMESH_MeshEditor::IsMedium( node, typeToCheck );
}

//=======================================================================
//function : AddNLinkNode
//purpose  : 
//=======================================================================
/*!
 * Auxilary function for filling myNLinkNodeMap
 */
void SMESH_MesherHelper::AddNLinkNode(const SMDS_MeshNode* n1,
                                     const SMDS_MeshNode* n2,
                                     const SMDS_MeshNode* n12)
{
  NLink link( n1, n2 );
  if ( n1 > n2 ) link = NLink( n2, n1 );
  // add new record to map
  myNLinkNodeMap.insert( make_pair(link,n12));
}

//=======================================================================
/*!
 * \brief Select UV on either of 2 pcurves of a seam edge, closest to the given UV
 * \param uv1 - UV on the seam
 * \param uv2 - UV within a face
 * \retval gp_Pnt2d - selected UV
 */
//=======================================================================

gp_Pnt2d SMESH_MesherHelper::GetUVOnSeam( const gp_Pnt2d& uv1, const gp_Pnt2d& uv2 ) const
{
  double p1 = uv1.Coord( myParIndex );
  double p2 = uv2.Coord( myParIndex );
  double p3 = ( Abs( p1 - myPar1 ) < Abs( p1 - myPar2 )) ? myPar2 : myPar1;
  if ( Abs( p2 - p1 ) > Abs( p2 - p3 ))
    p1 = p3;
  gp_Pnt2d result = uv1;
  result.SetCoord( myParIndex, p1 );
  return result;
}

//=======================================================================
/*!
 * \brief Return node UV on face
 * \param F - the face
 * \param n - the node
 * \param n2 - a node of element being created located inside a face
 * \retval gp_XY - resulting UV
 * 
 * Auxilary function called form GetMediumNode()
 */
//=======================================================================

gp_XY SMESH_MesherHelper::GetNodeUV(const TopoDS_Face&   F,
                                    const SMDS_MeshNode* n,
                                    const SMDS_MeshNode* n2)
{
  gp_Pnt2d uv;
  const SMDS_PositionPtr Pos = n->GetPosition();
  if(Pos->GetTypeOfPosition()==SMDS_TOP_FACE) {
    // node has position on face
    const SMDS_FacePosition* fpos =
      static_cast<const SMDS_FacePosition*>(n->GetPosition().get());
    uv = gp_Pnt2d(fpos->GetUParameter(),fpos->GetVParameter());
  }
  else if(Pos->GetTypeOfPosition()==SMDS_TOP_EDGE) {
    // node has position on edge => it is needed to find
    // corresponding edge from face, get pcurve for this
    // edge and recieve value from this pcurve
    const SMDS_EdgePosition* epos =
      static_cast<const SMDS_EdgePosition*>(n->GetPosition().get());
    SMESHDS_Mesh* meshDS = GetMesh()->GetMeshDS();
    int edgeID = Pos->GetShapeId();
    TopoDS_Edge E = TopoDS::Edge(meshDS->IndexToShape(edgeID));
    double f, l;
    TopLoc_Location loc;
    Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface(E, F, f, l);
    uv = C2d->Value( epos->GetUParameter() );
    // for a node on a seam edge select one of UVs on 2 pcurves
    if ( n2 && mySeamShapeIds.find( edgeID ) != mySeamShapeIds.end() )
      uv = GetUVOnSeam( uv, GetNodeUV( F, n2, 0 ));
  }
  else if(Pos->GetTypeOfPosition()==SMDS_TOP_VERTEX) {
    SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
    int vertexID = n->GetPosition()->GetShapeId();
    const TopoDS_Vertex& V = TopoDS::Vertex(meshDS->IndexToShape(vertexID));
    uv = BRep_Tool::Parameters( V, F );
    if ( n2 && mySeamShapeIds.find( vertexID ) != mySeamShapeIds.end() )
      uv = GetUVOnSeam( uv, GetNodeUV( F, n2, 0 ));
  }
  return uv.XY();
}

//=======================================================================
/*!
 * \brief Return node U on edge
 * \param E - the Edge
 * \param n - the node
 * \retval double - resulting U
 * 
 * Auxilary function called form GetMediumNode()
 */
//=======================================================================

double SMESH_MesherHelper::GetNodeU(const TopoDS_Edge&   E,
                                    const SMDS_MeshNode* n)
{
  double param = 0;
  const SMDS_PositionPtr Pos = n->GetPosition();
  if(Pos->GetTypeOfPosition()==SMDS_TOP_EDGE) {
    const SMDS_EdgePosition* epos =
      static_cast<const SMDS_EdgePosition*>(n->GetPosition().get());
    param =  epos->GetUParameter();
  }
  else if(Pos->GetTypeOfPosition()==SMDS_TOP_VERTEX) {
    SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
    int vertexID = n->GetPosition()->GetShapeId();
    const TopoDS_Vertex& V = TopoDS::Vertex(meshDS->IndexToShape(vertexID));
    param =  BRep_Tool::Parameter( V, E );
  }
  return param;
}

//=======================================================================
//function : GetMediumNode
//purpose  : 
//=======================================================================
/*!
 * Special function for search or creation medium node
 */
const SMDS_MeshNode* SMESH_MesherHelper::GetMediumNode(const SMDS_MeshNode* n1,
                                                       const SMDS_MeshNode* n2,
                                                       bool force3d)
{
  TopAbs_ShapeEnum shapeType = myShape.IsNull() ? TopAbs_SHAPE : myShape.ShapeType();

  NLink link(( n1 < n2 ? n1 : n2 ), ( n1 < n2 ? n2 : n1 ));
  ItNLinkNode itLN = myNLinkNodeMap.find( link );
  if ( itLN != myNLinkNodeMap.end() ) {
    return (*itLN).second;
  }
  else {
    // create medium node
    SMDS_MeshNode* n12;
    SMESHDS_Mesh* meshDS = GetMesh()->GetMeshDS();
    int faceID = -1, edgeID = -1;
    const SMDS_PositionPtr Pos1 = n1->GetPosition();
    const SMDS_PositionPtr Pos2 = n2->GetPosition();
  
    if( myShape.IsNull() )
    {
      if( Pos1->GetTypeOfPosition()==SMDS_TOP_FACE ) {
        faceID = Pos1->GetShapeId();
      }
      else if( Pos2->GetTypeOfPosition()==SMDS_TOP_FACE ) {
        faceID = Pos2->GetShapeId();
      }

      if( Pos1->GetTypeOfPosition()==SMDS_TOP_EDGE ) {
        edgeID = Pos1->GetShapeId();
      }
      if( Pos2->GetTypeOfPosition()==SMDS_TOP_EDGE ) {
        edgeID = Pos2->GetShapeId();
      }
    }

    if(!force3d) {
      // we try to create medium node using UV parameters of
      // nodes, else - medium between corresponding 3d points
      if(faceID>-1 || shapeType == TopAbs_FACE) {
	// obtaining a face and 2d points for nodes
	TopoDS_Face F;
	if( myShape.IsNull() )
          F = TopoDS::Face(meshDS->IndexToShape(faceID));
	else {
          F = TopoDS::Face(myShape);
          faceID = myShapeID;
        }

	gp_XY p1 = GetNodeUV(F,n1,n2);
        gp_XY p2 = GetNodeUV(F,n2,n1);

	//checking if surface is periodic
	Handle(Geom_Surface) S = BRep_Tool::Surface(F);
	Standard_Real UF,UL,VF,VL;
	S->Bounds(UF,UL,VF,VL);

	Standard_Real u,v;
	Standard_Boolean isUPeriodic = S->IsUPeriodic();
	if(isUPeriodic) {
	  Standard_Real UPeriod = S->UPeriod();
	  Standard_Real p2x = p2.X()+ShapeAnalysis::AdjustByPeriod(p2.X(),p1.X(),UPeriod);
	  Standard_Real pmid = (p1.X()+p2x)/2.;
	  u = pmid+ShapeAnalysis::AdjustToPeriod(pmid,UF,UL);
	}
	else 
	  u= (p1.X()+p2.X())/2.;

	Standard_Boolean isVPeriodic = S->IsVPeriodic();
	if(isVPeriodic) {
	  Standard_Real VPeriod = S->VPeriod();
	  Standard_Real p2y = p2.Y()+ShapeAnalysis::AdjustByPeriod(p2.Y(),p1.Y(),VPeriod);
	  Standard_Real pmid = (p1.Y()+p2y)/2.;
	  v = pmid+ShapeAnalysis::AdjustToPeriod(pmid,VF,VL);
	}
	else
	  v = (p1.Y()+p2.Y())/2.;

        gp_Pnt P = S->Value(u, v);
        n12 = meshDS->AddNode(P.X(), P.Y(), P.Z());
        meshDS->SetNodeOnFace(n12, faceID, u, v);
        myNLinkNodeMap.insert(NLinkNodeMap::value_type(link,n12));
        return n12;
      }
      if (edgeID>-1 || shapeType == TopAbs_EDGE) {

	TopoDS_Edge E;
	if( myShape.IsNull() )
          E = TopoDS::Edge(meshDS->IndexToShape(edgeID));
	else {
          E = TopoDS::Edge(myShape);
          edgeID = myShapeID;
        }

	double p1 = GetNodeU(E,n1);
	double p2 = GetNodeU(E,n2);

	double f,l;
	Handle(Geom_Curve) C = BRep_Tool::Curve(E, f, l);
	if(!C.IsNull()) {

	  Standard_Boolean isPeriodic = C->IsPeriodic();
	  double u;
	  if(isPeriodic) {
	    Standard_Real Period = C->Period();
	    Standard_Real p = p2+ShapeAnalysis::AdjustByPeriod(p2,p1,Period);
	    Standard_Real pmid = (p1+p)/2.;
	    u = pmid+ShapeAnalysis::AdjustToPeriod(pmid,C->FirstParameter(),C->LastParameter());
	  }
	  else
	    u = (p1+p2)/2.;

          gp_Pnt P = C->Value( u );
          n12 = meshDS->AddNode(P.X(), P.Y(), P.Z());
          meshDS->SetNodeOnEdge(n12, edgeID, u);
          myNLinkNodeMap.insert(NLinkNodeMap::value_type(link,n12));
          return n12;
	}
      }
    }
    // 3d variant
    double x = ( n1->X() + n2->X() )/2.;
    double y = ( n1->Y() + n2->Y() )/2.;
    double z = ( n1->Z() + n2->Z() )/2.;
    n12 = meshDS->AddNode(x,y,z);
    if(edgeID>-1)
        meshDS->SetNodeOnEdge(n12, edgeID);
    else if(faceID>-1)
        meshDS->SetNodeOnFace(n12, faceID);
    else
      meshDS->SetNodeInVolume(n12, myShapeID);
    myNLinkNodeMap.insert(NLinkNodeMap::value_type(link,n12));
    return n12;
  }
}

//=======================================================================
//function : AddQuadraticEdge
//purpose  : 
//=======================================================================
/**
 * Special function for creation quadratic edge
 */
SMDS_QuadraticEdge* SMESH_MesherHelper::AddQuadraticEdge(const SMDS_MeshNode* n1,
                                                         const SMDS_MeshNode* n2,
                                                         const int id,
							 const bool force3d)
{
  SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
  
  const SMDS_MeshNode* n12 = GetMediumNode(n1,n2,force3d);
  
  myCreateQuadratic = true;

  if(id)
    return  (SMDS_QuadraticEdge*)(meshDS->AddEdgeWithID(n1, n2, n12, id));
  else
    return  (SMDS_QuadraticEdge*)(meshDS->AddEdge(n1, n2, n12));
}

//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
/*!
 * Special function for creation quadratic triangle
 */
SMDS_MeshFace* SMESH_MesherHelper::AddFace(const SMDS_MeshNode* n1,
                                           const SMDS_MeshNode* n2,
                                           const SMDS_MeshNode* n3,
                                           const int id,
					   const bool force3d)
{
  SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
  if(!myCreateQuadratic) {
    if(id)
      return  meshDS->AddFaceWithID(n1, n2, n3, id);
    else
      return  meshDS->AddFace(n1, n2, n3);
  }

  const SMDS_MeshNode* n12 = GetMediumNode(n1,n2,force3d);
  const SMDS_MeshNode* n23 = GetMediumNode(n2,n3,force3d);
  const SMDS_MeshNode* n31 = GetMediumNode(n3,n1,force3d);

  if(id)
    return  meshDS->AddFaceWithID(n1, n2, n3, n12, n23, n31, id);
  else
    return  meshDS->AddFace(n1, n2, n3, n12, n23, n31);
}


//=======================================================================
//function : AddFace
//purpose  : 
//=======================================================================
/*!
 * Special function for creation quadratic quadrangle
 */
SMDS_MeshFace* SMESH_MesherHelper::AddFace(const SMDS_MeshNode* n1,
                                           const SMDS_MeshNode* n2,
                                           const SMDS_MeshNode* n3,
                                           const SMDS_MeshNode* n4,
                                           const int id,
					   const bool force3d)
{
  SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
  if(!myCreateQuadratic) {
    if(id)
      return  meshDS->AddFaceWithID(n1, n2, n3, n4, id);
    else
      return  meshDS->AddFace(n1, n2, n3, n4);
  }

  const SMDS_MeshNode* n12 = GetMediumNode(n1,n2,force3d);
  const SMDS_MeshNode* n23 = GetMediumNode(n2,n3,force3d);
  const SMDS_MeshNode* n34 = GetMediumNode(n3,n4,force3d);
  const SMDS_MeshNode* n41 = GetMediumNode(n4,n1,force3d);

  if(id)
    return  meshDS->AddFaceWithID(n1, n2, n3, n4, n12, n23, n34, n41, id);
  else
    return  meshDS->AddFace(n1, n2, n3, n4, n12, n23, n34, n41);
}


//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
/*!
 * Special function for creation quadratic volume
 */
SMDS_MeshVolume* SMESH_MesherHelper::AddVolume(const SMDS_MeshNode* n1,
                                               const SMDS_MeshNode* n2,
                                               const SMDS_MeshNode* n3,
                                               const SMDS_MeshNode* n4,
                                               const SMDS_MeshNode* n5,
                                               const SMDS_MeshNode* n6,
                                               const int id,
					       const bool force3d)
{
  SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
  if(!myCreateQuadratic) {
    if(id)
      return meshDS->AddVolumeWithID(n1, n2, n3, n4, n5, n6, id);
    else
      return meshDS->AddVolume(n1, n2, n3, n4, n5, n6);
  }

  const SMDS_MeshNode* n12 = GetMediumNode(n1,n2,force3d);
  const SMDS_MeshNode* n23 = GetMediumNode(n2,n3,force3d);
  const SMDS_MeshNode* n31 = GetMediumNode(n3,n1,force3d);

  const SMDS_MeshNode* n45 = GetMediumNode(n4,n5,force3d);
  const SMDS_MeshNode* n56 = GetMediumNode(n5,n6,force3d);
  const SMDS_MeshNode* n64 = GetMediumNode(n6,n4,force3d);

  const SMDS_MeshNode* n14 = GetMediumNode(n1,n4,force3d);
  const SMDS_MeshNode* n25 = GetMediumNode(n2,n5,force3d);
  const SMDS_MeshNode* n36 = GetMediumNode(n3,n6,force3d);

  if(id)
    return meshDS->AddVolumeWithID(n1, n2, n3, n4, n5, n6, 
                                   n12, n23, n31, n45, n56, n64, n14, n25, n36, id);
  else
    return meshDS->AddVolume(n1, n2, n3, n4, n5, n6,
                             n12, n23, n31, n45, n56, n64, n14, n25, n36);
}


//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
/*!
 * Special function for creation quadratic volume
 */
SMDS_MeshVolume* SMESH_MesherHelper::AddVolume(const SMDS_MeshNode* n1,
                                               const SMDS_MeshNode* n2,
                                               const SMDS_MeshNode* n3,
                                               const SMDS_MeshNode* n4,
                                               const int id, 
					       const bool force3d)
{
  SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
  if(!myCreateQuadratic) {
    if(id)
      return meshDS->AddVolumeWithID(n1, n2, n3, n4, id);
    else
      return meshDS->AddVolume(n1, n2, n3, n4);
  }

  const SMDS_MeshNode* n12 = GetMediumNode(n1,n2,force3d);
  const SMDS_MeshNode* n23 = GetMediumNode(n2,n3,force3d);
  const SMDS_MeshNode* n31 = GetMediumNode(n3,n1,force3d);

  const SMDS_MeshNode* n14 = GetMediumNode(n1,n4,force3d);
  const SMDS_MeshNode* n24 = GetMediumNode(n2,n4,force3d);
  const SMDS_MeshNode* n34 = GetMediumNode(n3,n4,force3d);

  if(id)
    return meshDS->AddVolumeWithID(n1, n2, n3, n4, n12, n23, n31, n14, n24, n34, id);
  else
    return meshDS->AddVolume(n1, n2, n3, n4, n12, n23, n31, n14, n24, n34);
}


//=======================================================================
//function : AddVolume
//purpose  : 
//=======================================================================
/*!
 * Special function for creation quadratic volume
 */
SMDS_MeshVolume* SMESH_MesherHelper::AddVolume(const SMDS_MeshNode* n1,
                                               const SMDS_MeshNode* n2,
                                               const SMDS_MeshNode* n3,
                                               const SMDS_MeshNode* n4,
                                               const SMDS_MeshNode* n5,
                                               const SMDS_MeshNode* n6,
                                               const SMDS_MeshNode* n7,
                                               const SMDS_MeshNode* n8,
                                               const int id,
					       const bool force3d)
{
  SMESHDS_Mesh * meshDS = GetMesh()->GetMeshDS();
  if(!myCreateQuadratic) {
    if(id)
      return meshDS->AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, id);
    else
      return meshDS->AddVolume(n1, n2, n3, n4, n5, n6, n7, n8);
  }

  const SMDS_MeshNode* n12 = GetMediumNode(n1,n2,force3d);
  const SMDS_MeshNode* n23 = GetMediumNode(n2,n3,force3d);
  const SMDS_MeshNode* n34 = GetMediumNode(n3,n4,force3d);
  const SMDS_MeshNode* n41 = GetMediumNode(n4,n1,force3d);

  const SMDS_MeshNode* n56 = GetMediumNode(n5,n6,force3d);
  const SMDS_MeshNode* n67 = GetMediumNode(n6,n7,force3d);
  const SMDS_MeshNode* n78 = GetMediumNode(n7,n8,force3d);
  const SMDS_MeshNode* n85 = GetMediumNode(n8,n5,force3d);

  const SMDS_MeshNode* n15 = GetMediumNode(n1,n5,force3d);
  const SMDS_MeshNode* n26 = GetMediumNode(n2,n6,force3d);
  const SMDS_MeshNode* n37 = GetMediumNode(n3,n7,force3d);
  const SMDS_MeshNode* n48 = GetMediumNode(n4,n8,force3d);

  if(id)
    return meshDS->AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8,
                                   n12, n23, n34, n41, n56, n67,
                                   n78, n85, n15, n26, n37, n48, id);
  else
    return meshDS->AddVolume(n1, n2, n3, n4, n5, n6, n7, n8,
                             n12, n23, n34, n41, n56, n67,
                             n78, n85, n15, n26, n37, n48);
}


