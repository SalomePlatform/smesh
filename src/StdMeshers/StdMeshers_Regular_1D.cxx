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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
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
#include "StdMeshers_Distribution.hxx"

#include "StdMeshers_LocalLength.hxx"
#include "StdMeshers_NumberOfSegments.hxx"
#include "StdMeshers_Arithmetic1D.hxx"
#include "StdMeshers_StartEndLength.hxx"
#include "StdMeshers_Deflection1D.hxx"
#include "StdMeshers_AutomaticLength.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_HypoFilter.hxx"
#include "SMESH_subMesh.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_EdgePosition.hxx"

#include "Utils_SALOME_Exception.hxx"
#include "utilities.h"

#include <BRep_Tool.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <Precision.hxx>
#include <Expr_GeneralExpression.hxx>
#include <Expr_NamedUnknown.hxx>
#include <Expr_Array1OfNamedUnknown.hxx>
#include <ExprIntrp_GenExp.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <OSD.hxx>

#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>

#include <string>
#include <math.h>

using namespace std;

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
	_compatibleHypothesis.push_back("AutomaticLength");

	_compatibleHypothesis.push_back("QuadraticMesh"); // auxiliary !!!
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
                         (SMESH_Mesh&                          aMesh,
                          const TopoDS_Shape&                  aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  _hypType = NONE;
  _quadraticMesh = false;

  const bool ignoreAuxiliaryHyps = false;
  const list <const SMESHDS_Hypothesis * > & hyps =
    GetUsedHypothesis(aMesh, aShape, ignoreAuxiliaryHyps);

  // find non-auxiliary hypothesis
  const SMESHDS_Hypothesis *theHyp = 0;
  list <const SMESHDS_Hypothesis * >::const_iterator h = hyps.begin();
  for ( ; h != hyps.end(); ++h ) {
    if ( static_cast<const SMESH_Hypothesis*>(*h)->IsAuxiliary() ) {
      if ( strcmp( "QuadraticMesh", (*h)->GetName() ) == 0 )
        _quadraticMesh = true;
    }
    else {
      if ( !theHyp )
        theHyp = *h; // use only the first non-auxiliary hypothesis
    }
  }

  if ( !theHyp )
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work without a hypothesis
  }

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
    _ivalue[ NB_SEGMENTS_IND  ] = hyp->GetNumberOfSegments();
    ASSERT( _ivalue[ NB_SEGMENTS_IND ] > 0 );
    _ivalue[ DISTR_TYPE_IND ] = (int) hyp->GetDistrType();
    switch (_ivalue[ DISTR_TYPE_IND ])
    {
    case StdMeshers_NumberOfSegments::DT_Scale:
      _value[ SCALE_FACTOR_IND ] = hyp->GetScaleFactor();
      break;
    case StdMeshers_NumberOfSegments::DT_TabFunc:
      _vvalue[ TAB_FUNC_IND ] = hyp->GetTableFunction();
      break;
    case StdMeshers_NumberOfSegments::DT_ExprFunc:
      _svalue[ EXPR_FUNC_IND ] = hyp->GetExpressionFunction();
      break;
    case StdMeshers_NumberOfSegments::DT_Regular:
      break;
    default:
      ASSERT(0);
      break;
    }
    if (_ivalue[ DISTR_TYPE_IND ] == StdMeshers_NumberOfSegments::DT_TabFunc ||
        _ivalue[ DISTR_TYPE_IND ] == StdMeshers_NumberOfSegments::DT_ExprFunc)
        _ivalue[ CONV_MODE_IND ] = hyp->ConversionMode();
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

  else if (hypName == "AutomaticLength")
  {
    StdMeshers_AutomaticLength * hyp = const_cast<StdMeshers_AutomaticLength *>
      (dynamic_cast <const StdMeshers_AutomaticLength * >(theHyp));
    ASSERT(hyp);
    _value[ BEG_LENGTH_IND ] = _value[ END_LENGTH_IND ] = hyp->GetLength( &aMesh, aShape );
    ASSERT( _value[ BEG_LENGTH_IND ] > 0 );
    _hypType = LOCAL_LENGTH;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

  return ( _hypType != NONE );
}

//=======================================================================
//function : compensateError
//purpose  : adjust theParams so that the last segment length == an
//=======================================================================

static void compensateError(double a1, double an,
                            double U1, double Un,
                            double             length,
                            GeomAdaptor_Curve& C3d,
                            list<double> &     theParams)
{
  int i, nPar = theParams.size();
  if ( a1 + an < length && nPar > 1 )
  {
    list<double>::reverse_iterator itU = theParams.rbegin();
    double Ul = *itU++;
    // dist from the last point to the edge end <Un>, it should be equal <an>
    double Ln = GCPnts_AbscissaPoint::Length( C3d, Ul, Un );
    double dLn = an - Ln; // error of <an>
    if ( Abs( dLn ) <= Precision::Confusion() )
      return;
    double dU = Abs( Ul - *itU ); // parametric length of the last but one segment
    double dUn = dLn * Abs( Un - U1 ) / length; // parametric error of <an>
    if ( dUn < 0.5 * dU ) { // last segment is a bit shorter than it should
      dUn = -dUn; // move the last parameter to the edge beginning
    }
    else {  // last segment is much shorter than it should -> remove the last param and
      theParams.pop_back(); nPar--; // move the rest points toward the edge end
      Ln = GCPnts_AbscissaPoint::Length( C3d, theParams.back(), Un );
      dUn = ( an - Ln ) * Abs( Un - U1 ) / length;
      if ( dUn < 0.5 * dU )
        dUn = -dUn;
    }
    if ( U1 > Un )
      dUn = -dUn;
    double q  = dUn / ( nPar - 1 );
    for ( itU = theParams.rbegin(), i = 1; i < nPar; itU++, i++ ) {
      (*itU) += dUn;
      dUn -= q;
    }
  }
}

static bool computeParamByFunc(Adaptor3d_Curve& C3d, double first, double last,
                               double length, bool theReverse, 
                               int nbSeg, Function& func,
                               list<double>& theParams)
{
  // never do this way
  //OSD::SetSignal( true );

  if( nbSeg<=0 )
    return false;

  MESSAGE( "computeParamByFunc" );

  int nbPnt = 1 + nbSeg;
  vector<double> x(nbPnt, 0.);

  if( !buildDistribution( func, 0.0, 1.0, nbSeg, x, 1E-4 ) )
     return false;

  MESSAGE( "Points:\n" );
  char buf[1024];
  for( int i=0; i<=nbSeg; i++ )
  {
    sprintf(  buf, "%f\n", float(x[i] ) );
    MESSAGE( buf );
  }
    


  // apply parameters in range [0,1] to the space of the curve
  double prevU = first;
  double sign = 1.;
  if (theReverse)
  {
    prevU = last;
    sign = -1.;
  }
  for( int i = 1; i < nbSeg; i++ )
  {
    double curvLength = length * (x[i] - x[i-1]) * sign;
    GCPnts_AbscissaPoint Discret( C3d, curvLength, prevU );
    if ( !Discret.IsDone() )
      return false;
    double U = Discret.Parameter();
    if ( U > first && U < last )
      theParams.push_back( U );
    else
      return false;
    prevU = U;
  }
  return true;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
bool StdMeshers_Regular_1D::computeInternalParameters(const TopoDS_Edge& theEdge,
                                                      list<double> &     theParams,
                                                      const bool         theReverse) const
{
  theParams.clear();

  double f, l;
  Handle(Geom_Curve) Curve = BRep_Tool::Curve(theEdge, f, l);
  GeomAdaptor_Curve C3d (Curve, f, l);

  double length = EdgeLength(theEdge);

  switch( _hypType )
  {
  case LOCAL_LENGTH:
  case NB_SEGMENTS: {

    double eltSize = 1;
    if ( _hypType == LOCAL_LENGTH )
    {
      // Local Length hypothesis
      double nbseg = ceil(length / _value[ BEG_LENGTH_IND ]); // integer sup
      if (nbseg <= 0)
        nbseg = 1;                        // degenerated edge
      eltSize = length / nbseg;
    }
    else
    {
      // Number Of Segments hypothesis
      int NbSegm = _ivalue[ NB_SEGMENTS_IND ];
      if ( NbSegm < 1 )  return false;
      if ( NbSegm == 1 ) return true;

      switch (_ivalue[ DISTR_TYPE_IND ])
      {
      case StdMeshers_NumberOfSegments::DT_Scale:
        {
          double scale = _value[ SCALE_FACTOR_IND ];

          if (fabs(scale - 1.0) < Precision::Confusion()) {
            // special case to avoid division on zero
            for (int i = 1; i < NbSegm; i++) {
              double param = f + (l - f) * i / NbSegm;
              theParams.push_back( param );
            }
          } else {
            // general case of scale distribution
            if ( theReverse )
              scale = 1.0 / scale;

            double alpha = pow(scale, 1.0 / (NbSegm - 1));
            double factor = (l - f) / (1.0 - pow(alpha, NbSegm));

            for (int i = 1; i < NbSegm; i++) {
              double param = f + factor * (1.0 - pow(alpha, i));
              theParams.push_back( param );
            }
          }
          return true;
        }
        break;
      case StdMeshers_NumberOfSegments::DT_TabFunc:
        {
          FunctionTable func(_vvalue[ TAB_FUNC_IND ], _ivalue[ CONV_MODE_IND ]);
          return computeParamByFunc(C3d, f, l, length, theReverse,
                                    _ivalue[ NB_SEGMENTS_IND ], func,
                                    theParams);
        }
        break;
      case StdMeshers_NumberOfSegments::DT_ExprFunc:
        {
          FunctionExpr func(_svalue[ EXPR_FUNC_IND ].c_str(), _ivalue[ CONV_MODE_IND ]);
          return computeParamByFunc(C3d, f, l, length, theReverse,
                                    _ivalue[ NB_SEGMENTS_IND ], func,
                                    theParams);
        }
        break;
      case StdMeshers_NumberOfSegments::DT_Regular:
        eltSize = length / _ivalue[ NB_SEGMENTS_IND ];
        break;
      default:
        return false;
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
    compensateError( eltSize, eltSize, f, l, length, C3d, theParams ); // for PAL9899
    return true;
  }

  case BEG_END_LENGTH: {

    // geometric progression: SUM(n) = ( a1 - an * q ) / ( 1 - q ) = length

    double a1 = _value[ BEG_LENGTH_IND ];
    double an = _value[ END_LENGTH_IND ];
    double q  = ( length - a1 ) / ( length - an );

    double U1 = theReverse ? l : f;
    double Un = theReverse ? f : l;
    double param = U1;
    double eltSize = theReverse ? -a1 : a1;
    while ( 1 ) {
      // computes a point on a curve <C3d> at the distance <eltSize>
      // from the point of parameter <param>.
      GCPnts_AbscissaPoint Discret( C3d, eltSize, param );
      if ( !Discret.IsDone() ) break;
      param = Discret.Parameter();
      if ( param > f && param < l )
        theParams.push_back( param );
      else
        break;
      eltSize *= q;
    }
    compensateError( a1, an, U1, Un, length, C3d, theParams );
    return true;
  }

  case ARITHMETIC_1D: {

    // arithmetic progression: SUM(n) = ( an - a1 + q ) * ( a1 + an ) / ( 2 * q ) = length

    double a1 = _value[ BEG_LENGTH_IND ];
    double an = _value[ END_LENGTH_IND ];

    double  q = ( an - a1 ) / ( 2 *length/( a1 + an ) - 1 );
    int     n = int( 1 + ( an - a1 ) / q );

    double U1 = theReverse ? l : f;
    double Un = theReverse ? f : l;
    double param = U1;
    double eltSize = a1;
    if ( theReverse ) {
      eltSize = -eltSize;
      q = -q;
    }
    while ( n-- > 0 && eltSize * ( Un - U1 ) > 0 ) {
      // computes a point on a curve <C3d> at the distance <eltSize>
      // from the point of parameter <param>.
      GCPnts_AbscissaPoint Discret( C3d, eltSize, param );
      if ( !Discret.IsDone() ) break;
      param = Discret.Parameter();
      if ( param > f && param < l )
        theParams.push_back( param );
      else
        break;
      eltSize += q;
    }
    compensateError( a1, an, U1, Un, length, C3d, theParams );

    return true;
  }

  case DEFLECTION: {

    GCPnts_UniformDeflection Discret(C3d, _value[ DEFLECTION_IND ], f, l, true);
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
  int shapeID = meshDS->ShapeToIndex( E );

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
  if (!lid->more()) {
    MESSAGE (" NO NODE BUILT ON VERTEX ");
    return false;
  }
  const SMDS_MeshNode * idLast = lid->next();

  if (!Curve.IsNull()) {
    list< double > params;
    bool reversed = false;
    if ( !_mainEdge.IsNull() )
      reversed = aMesh.IsReversedInChain( EE, _mainEdge );
    try {
#if (OCC_VERSION_MAJOR << 16 | OCC_VERSION_MINOR << 8 | OCC_VERSION_MAINTENANCE) > 0x060100
      OCC_CATCH_SIGNALS;
#endif
      if ( ! computeInternalParameters( E, params, reversed )) {
        //cout << "computeInternalParameters() failed" <<endl;
        return false;
      }
    }
    catch ( Standard_Failure ) {
      //cout << "computeInternalParameters() failed, Standard_Failure" <<endl;
      return false;
    }

    // edge extrema (indexes : 1 & NbPoints) already in SMDS (TopoDS_Vertex)
    // only internal nodes receive an edge position with param on curve

    const SMDS_MeshNode * idPrev = idFirst;
    double parPrev = f;
    double parLast = l;
//     if(reversed) {
//       parPrev = l;
//       parLast = f;
//     }
    
    for (list<double>::iterator itU = params.begin(); itU != params.end(); itU++) {
      double param = *itU;
      gp_Pnt P = Curve->Value(param);

      //Add the Node in the DataStructure
      SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnEdge(node, shapeID, param);

      if(_quadraticMesh) {
        // create medium node
        double prm = ( parPrev + param )/2;
        gp_Pnt PM = Curve->Value(prm);
        SMDS_MeshNode * NM = meshDS->AddNode(PM.X(), PM.Y(), PM.Z());
        meshDS->SetNodeOnEdge(NM, shapeID, prm);
        SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node, NM);
        meshDS->SetMeshElementOnShape(edge, shapeID);
      }
      else {
        SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node);
        meshDS->SetMeshElementOnShape(edge, shapeID);
      }

      idPrev = node;
      parPrev = param;
    }
    if(_quadraticMesh) {
      double prm = ( parPrev + parLast )/2;
      gp_Pnt PM = Curve->Value(prm);
      SMDS_MeshNode * NM = meshDS->AddNode(PM.X(), PM.Y(), PM.Z());
      meshDS->SetNodeOnEdge(NM, shapeID, prm);
      SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, idLast, NM);
      meshDS->SetMeshElementOnShape(edge, shapeID);
    }
    else {
      SMDS_MeshEdge* edge = meshDS->AddEdge(idPrev, idLast);
      meshDS->SetMeshElementOnShape(edge, shapeID);
    }
  }
  else {
    // Edge is a degenerated Edge : We put n = 5 points on the edge.
    const int NbPoints = 5;
    BRep_Tool::Range(E, f, l);
    double du = (l - f) / (NbPoints - 1);
    //MESSAGE("************* Degenerated edge! *****************");

    TopoDS_Vertex V1, V2;
    TopExp::Vertices(E, V1, V2);
    gp_Pnt P = BRep_Tool::Pnt(V1);

    const SMDS_MeshNode * idPrev = idFirst;
    for (int i = 2; i < NbPoints; i++) {
      double param = f + (i - 1) * du;
      SMDS_MeshNode * node = meshDS->AddNode(P.X(), P.Y(), P.Z());
      if(_quadraticMesh) {
        // create medium node
        double prm = param - du/2.;
        SMDS_MeshNode * NM = meshDS->AddNode(P.X(), P.Y(), P.Z());
        meshDS->SetNodeOnEdge(NM, shapeID, prm);
        SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node, NM);
        meshDS->SetMeshElementOnShape(edge, shapeID);
      }
      else {
        SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, node);
        meshDS->SetMeshElementOnShape(edge, shapeID);
      }
      meshDS->SetNodeOnEdge(node, shapeID, param);
      idPrev = node;
    }
    if(_quadraticMesh) {
      // create medium node
      double prm = l - du/2.;
      SMDS_MeshNode * NM = meshDS->AddNode(P.X(), P.Y(), P.Z());
      meshDS->SetNodeOnEdge(NM, shapeID, prm);
      SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, idLast, NM);
      meshDS->SetMeshElementOnShape(edge, shapeID);
    }
    else {
      SMDS_MeshEdge * edge = meshDS->AddEdge(idPrev, idLast);
      meshDS->SetMeshElementOnShape(edge, shapeID);
    }
  }
  return true;
}

//=============================================================================
/*!
 *  See comments in SMESH_Algo.cxx
 */
//=============================================================================

const list <const SMESHDS_Hypothesis *> &
StdMeshers_Regular_1D::GetUsedHypothesis(SMESH_Mesh &         aMesh,
                                         const TopoDS_Shape & aShape,
                                         const bool           ignoreAuxiliary)
{
  _usedHypList.clear();
  _mainEdge.Nullify();

  SMESH_HypoFilter auxiliaryFilter, compatibleFilter;
  auxiliaryFilter.Init( SMESH_HypoFilter::IsAuxiliary() );
  const bool ignoreAux = true;
  InitCompatibleHypoFilter( compatibleFilter, ignoreAux );

  // get non-auxiliary assigned to aShape
  int nbHyp = aMesh.GetHypotheses( aShape, compatibleFilter, _usedHypList, false );

  if (nbHyp == 0)
  {
    // Check, if propagated from some other edge
    if (aShape.ShapeType() == TopAbs_EDGE &&
        aMesh.IsPropagatedHypothesis(aShape, _mainEdge))
    {
      // Propagation of 1D hypothesis from <aMainEdge> on this edge;
      // get non-auxiliary assigned to _mainEdge
      nbHyp = aMesh.GetHypotheses( _mainEdge, compatibleFilter, _usedHypList, true );
    }
  }

  if (nbHyp == 0) // nothing propagated nor assigned to aShape
  {
    SMESH_Algo::GetUsedHypothesis( aMesh, aShape, ignoreAuxiliary );
    nbHyp = _usedHypList.size();
  }
  else
  {
    // get auxiliary hyps from aShape
    aMesh.GetHypotheses( aShape, auxiliaryFilter, _usedHypList, true );
  }
  if ( nbHyp > 1 && ignoreAuxiliary )
    _usedHypList.clear(); //only one compatible non-auxiliary hypothesis allowed

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
