//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshers_Regular_1D.cxx
//           Moved here from SMESH_Regular_1D.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;

#include "StdMeshers_Regular_1D.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

#include "StdMeshers_LocalLength.hxx"
#include "StdMeshers_NumberOfSegments.hxx"
#include "StdMeshers_Arithmetic1D.hxx"
#include "StdMeshers_StartEndLength.hxx"
#include "StdMeshers_Deflection1D.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMESH_subMesh.hxx"

#include "utilities.h"

#include <BRep_Tool.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Precision.hxx>

#include <string>
//#include <algorithm>

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Regular_1D::StdMeshers_Regular_1D(int hypId, int studyId,
	SMESH_Gen * gen):SMESH_1D_Algo(hypId, studyId, gen)
{
	MESSAGE("StdMeshers_Regular_1D::StdMeshers_Regular_1D");
	_name = "Regular_1D";
	_shapeType = (1 << TopAbs_EDGE);

	_compatibleHypothesis.push_back("LocalLength");
	_compatibleHypothesis.push_back("NumberOfSegments");
	_compatibleHypothesis.push_back("StartEndLength");
	_compatibleHypothesis.push_back("Deflection1D");
	_compatibleHypothesis.push_back("Arithmetic1D");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_Regular_1D::~StdMeshers_Regular_1D()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Regular_1D::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  _hypType = NONE;

  const list <const SMESHDS_Hypothesis * >&hyps = GetUsedHypothesis(aMesh, aShape);
  if (hyps.size() == 0)
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work without a hypothesis
  }

  // use only the first hypothesis
  const SMESHDS_Hypothesis *theHyp = hyps.front();

  string hypName = theHyp->GetName();

  if (hypName == "LocalLength")
  {
    const StdMeshers_LocalLength * hyp =
      dynamic_cast <const StdMeshers_LocalLength * >(theHyp);
    ASSERT(hyp);
    _value[ BEG_LENGTH_IND ] = _value[ END_LENGTH_IND ] = hyp->GetLength();
    ASSERT( _value[ BEG_LENGTH_IND ] > 0 );
    _hypType = LOCAL_LENGTH;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }

  else if (hypName == "NumberOfSegments")
  {
    const StdMeshers_NumberOfSegments * hyp =
      dynamic_cast <const StdMeshers_NumberOfSegments * >(theHyp);
    ASSERT(hyp);
    _value[ NB_SEGMENTS_IND  ] = hyp->GetNumberOfSegments();
    _value[ SCALE_FACTOR_IND ] = hyp->GetScaleFactor();
    ASSERT( _value[ NB_SEGMENTS_IND ] > 0 );
    _hypType = NB_SEGMENTS;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }

  else if (hypName == "Arithmetic1D")
  {
    const StdMeshers_Arithmetic1D * hyp =
      dynamic_cast <const StdMeshers_Arithmetic1D * >(theHyp);
    ASSERT(hyp);
    _value[ BEG_LENGTH_IND ] = hyp->GetLength( true );
    _value[ END_LENGTH_IND ] = hyp->GetLength( false );
    ASSERT( _value[ BEG_LENGTH_IND ] > 0 && _value[ END_LENGTH_IND ] > 0 );
    _hypType = ARITHMETIC_1D;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }

  else if (hypName == "StartEndLength")
  {
    const StdMeshers_StartEndLength * hyp =
      dynamic_cast <const StdMeshers_StartEndLength * >(theHyp);
    ASSERT(hyp);
    _value[ BEG_LENGTH_IND ] = hyp->GetLength( true );
    _value[ END_LENGTH_IND ] = hyp->GetLength( false );
    ASSERT( _value[ BEG_LENGTH_IND ] > 0 && _value[ END_LENGTH_IND ] > 0 );
    _hypType = BEG_END_LENGTH;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }

  else if (hypName == "Deflection1D")
  {
    const StdMeshers_Deflection1D * hyp =
      dynamic_cast <const StdMeshers_Deflection1D * >(theHyp);
    ASSERT(hyp);
    _value[ DEFLECTION_IND ] = hyp->GetDeflection();
    ASSERT( _value[ DEFLECTION_IND ] > 0 );
    _hypType = DEFLECTION;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

  return ( _hypType != NONE );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
bool StdMeshers_Regular_1D::computeInternalParameters(const TopoDS_Edge& theEdge,
                                                      list<double> &     theParams) const
{
  theParams.clear();

  double f, l;
  Handle(Geom_Curve) Curve = BRep_Tool::Curve(theEdge, f, l);
  GeomAdaptor_Curve C3d(Curve);

  double length = EdgeLength(theEdge);
  //SCRUTE(length);

  switch( _hypType )
  {
  case LOCAL_LENGTH:
  case NB_SEGMENTS: {

    double eltSize = 1;
    if ( _hypType == LOCAL_LENGTH )
    {
      double nbseg = ceil(length / _value[ BEG_LENGTH_IND ]); // integer sup
      if (nbseg <= 0)
        nbseg = 1;                        // degenerated edge
      eltSize = length / nbseg;
    }
    else
    {
      double epsilon = 0.001;
      if (fabs(_value[ SCALE_FACTOR_IND ] - 1.0) > epsilon)
      {
        double alpha =
          pow( _value[ SCALE_FACTOR_IND ], 1.0 / (_value[ NB_SEGMENTS_IND ] - 1));
        double factor =
          length / (1 - pow( alpha,_value[ NB_SEGMENTS_IND ]));

        int i, NbPoints = (int) _value[ NB_SEGMENTS_IND ];
        for ( i = 2; i < NbPoints; i++ )
        {
          double param = factor * (1 - pow(alpha, i - 1));
          theParams.push_back( param );
        }
        return true;
      }
      else
      {
        eltSize = length / _value[ NB_SEGMENTS_IND ];
      }
    }

    GCPnts_UniformAbscissa Discret(C3d, eltSize, f, l);
    if ( !Discret.IsDone() )
      return false;

    int NbPoints = Discret.NbPoints();
    for ( int i = 2; i < NbPoints; i++ )
    {
      double param = Discret.Parameter(i);
      theParams.push_back( param );
    }
    return true;
  }

  case BEG_END_LENGTH: {

    // geometric progression: SUM(n) = ( a1 - an * q ) / ( 1 - q ) = length

    double a1 = _value[ BEG_LENGTH_IND ];
    double an = _value[ END_LENGTH_IND ];
    double q  = ( length - a1 ) / ( length - an );

    double U1 = Min ( f, l );
    double Un = Max ( f, l );
    double param = U1;
    double eltSize = a1;
    while ( 1 ) {
      // computes a point on a curve <C3d> at the distance <eltSize>
      // from the point of parameter <param>.
      GCPnts_AbscissaPoint Discret( C3d, eltSize, param );
      if ( !Discret.IsDone() ) break;
      param = Discret.Parameter();
      if ( param < Un )
        theParams.push_back( param );
      else
        break;
      eltSize *= q;
    }
    if ( a1 + an < length ) {
      // compensate error
      double Ln = GCPnts_AbscissaPoint::Length( C3d, theParams.back(), Un );
      double dLn = an - Ln;
      if ( dLn < 0.5 * an )
        dLn = -dLn;
      else {
        theParams.pop_back();
        Ln = GCPnts_AbscissaPoint::Length( C3d, theParams.back(), Un );
        dLn = an - Ln;
        if ( dLn < 0.5 * an )
          dLn = -dLn;
      }
      double dUn = dLn * ( Un - U1 ) / length;
//       SCRUTE( Ln );
//       SCRUTE( dLn );
//       SCRUTE( dUn );
      list<double>::reverse_iterator itU = theParams.rbegin();
      int i, n = theParams.size();
      for ( i = 1 ; i < n; itU++, i++ ) {
        (*itU) += dUn;
        dUn /= q;
      }
    }

    return true;
  }

  case DEFLECTION: {

    GCPnts_UniformDeflection Discret(C3d, _value[ DEFLECTION_IND ], true);
    if ( !Discret.IsDone() )
      return false;

    int NbPoints = Discret.NbPoints();
    for ( int i = 2; i < NbPoints; i++ )
    {
      double param = Discret.Parameter(i);
      theParams.push_back( param );
    }
    return true;
    
  }

  case ARITHMETIC_1D: {
        // arithmetic progression: SUM(n) = ( an - a1 + q ) * ( a1 + an ) / ( 2 * q ) = length

    double a1 = _value[ BEG_LENGTH_IND ];
    double an = _value[ END_LENGTH_IND ];

    double nd = (2 * length) / (an + a1) - 1;
    int n = int(nd);
    if(n != nd)
      n++;

    double q = ((2 * length) / (n + 1) - 2 * a1) / n;
    double U1 = Min ( f, l );
    double Un = Max ( f, l );
    double param = U1;
    double eltSize = a1;

    double L=0;
    while ( 1 ) {
      L+=eltSize;
      // computes a point on a curve <C3d> at the distance <eltSize>
      // from the point of parameter <param>.
      GCPnts_AbscissaPoint Discret( C3d, eltSize, param );
      if ( !Discret.IsDone() ) break;
      param = Discret.Parameter();
      if ( fabs(param - Un) > Precision::Confusion() && param < Un) {
        theParams.push_back( param );
      }
      else
        break;
      eltSize += q;
    }

    return true;
  }

  default:;
  }

  return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool StdMeshers_Regular_1D::Compute(SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
  MESSAGE("StdMeshers_Regular_1D::Compute");

  if ( _hypType == NONE )
    return false;

  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();
  aMesh.GetSubMesh(aShape);

  const TopoDS_Edge & EE = TopoDS::Edge(aShape);
  TopoDS_Edge E = TopoDS::Edge(EE.Oriented(TopAbs_FORWARD));

  double f, l;
  Handle(Geom_Curve) Curve = BRep_Tool::Curve(E, f, l);

  TopoDS_Vertex VFirst, VLast;
  TopExp::Vertices(E, VFirst, VLast);   // Vfirst corresponds to f and Vlast to l

  ASSERT(!VFirst.IsNull());
  SMDS_NodeIteratorPtr lid= aMesh.GetSubMesh(VFirst)->GetSubMeshDS()->GetNodes();
  if (!lid->more())
  {
    MESSAGE (" NO NODE BUILT ON VERTEX ");
    return false;
  }
  const SMDS_MeshNode * idFirst = lid->next();

  ASSERT(!VLast.IsNull());
  lid=aMesh.GetSubMesh(VLast)->GetSubMeshDS()->GetNodes();
  if (!lid->more())
  {
    MESSAGE (" NO NODE BUILT ON VERTEX ");
    return false;
  }
  const SMDS_MeshNode * idLast = lid->next();

  if (!Curve.IsNull())
  {
    list< double > params;
    try {
      if ( ! computeInternalParameters( E, params ))
        return false;
    }
    catch ( Standard_Failure ) {
      return false;
    }

    // edge extrema (indexes : 1 & NbPoints) already in SMDS (TopoDS_Vertex)
    // only internal nodes receive an edge position with param on curve

    const SMDS_MeshNode * idPrev = idFirst;
    
    for (list<double>::iterator itU = params.begin(); itU != params.end(); itU++)
    {
      double param = *itU;
      gp_Pnt P = Curve->Value(param);

      //Add the Node in the DataStructure
      SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnEdge(node, E);

      // **** edgePosition associe au point = param. 
      SMDS_EdgePosition* epos =
        dynamic_cast<SMDS_EdgePosition *>(node->GetPosition().get());
      epos->SetUParameter(param);

      SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node);
      meshDS->SetMeshElementOnShape(edge, E);
      idPrev = node;
    }
    SMDS_MeshEdge* edge = meshDS->AddEdge(idPrev, idLast);
    meshDS->SetMeshElementOnShape(edge, E);
  }
  else
  {
    // Edge is a degenerated Edge : We put n = 5 points on the edge.
    int NbPoints = 5;
    BRep_Tool::Range(E, f, l);
    double du = (l - f) / (NbPoints - 1);
    //MESSAGE("************* Degenerated edge! *****************");

    TopoDS_Vertex V1, V2;
    TopExp::Vertices(E, V1, V2);
    gp_Pnt P = BRep_Tool::Pnt(V1);

    const SMDS_MeshNode * idPrev = idFirst;
    for (int i = 2; i < NbPoints; i++)
    {
      double param = f + (i - 1) * du;
      SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnEdge(node, E);

      SMDS_EdgePosition* epos =
        dynamic_cast<SMDS_EdgePosition*>(node->GetPosition().get());
      epos->SetUParameter(param);

      SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node);
      meshDS->SetMeshElementOnShape(edge, E);
      idPrev = node;
    }
    SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, idLast);
    meshDS->SetMeshElementOnShape(edge, E);
  }
  return true;
}

//=============================================================================
/*!
 *  See comments in SMESH_Algo.cxx
 */
//=============================================================================

const list <const SMESHDS_Hypothesis *> & StdMeshers_Regular_1D::GetUsedHypothesis(
	SMESH_Mesh & aMesh, const TopoDS_Shape & aShape)
{
  _usedHypList.clear();
  _usedHypList = GetAppliedHypothesis(aMesh, aShape);	// copy
  int nbHyp = _usedHypList.size();
  if (nbHyp == 0)
  {
    // Check, if propagated from some other edge
    TopoDS_Shape aMainEdge;
    if (aShape.ShapeType() == TopAbs_EDGE &&
        aMesh.IsPropagatedHypothesis(aShape, aMainEdge))
    {
      // Propagation of 1D hypothesis from <aMainEdge> on this edge
      _usedHypList = GetAppliedHypothesis(aMesh, aMainEdge);	// copy
      nbHyp = _usedHypList.size();
    }
  }
  if (nbHyp == 0)
  {
    TopTools_ListIteratorOfListOfShape ancIt( aMesh.GetAncestors( aShape ));
    for (; ancIt.More(); ancIt.Next())
    {
      const TopoDS_Shape& ancestor = ancIt.Value();
      _usedHypList = GetAppliedHypothesis(aMesh, ancestor);	// copy
      nbHyp = _usedHypList.size();
      if (nbHyp == 1)
        break;
    }
  }
  if (nbHyp > 1)
    _usedHypList.clear();	//only one compatible hypothesis allowed
  return _usedHypList;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_Regular_1D::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_Regular_1D::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_Regular_1D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_Regular_1D & hyp)
{
  return hyp.LoadFrom( load );
}
