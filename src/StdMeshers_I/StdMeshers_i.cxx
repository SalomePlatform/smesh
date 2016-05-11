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

//  SMESH StdMeshers : implementaion of SMESH idl descriptions
//  File   : StdMeshers_i.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//
#include "SMESH_StdMeshers_I.hxx"

#include "SMESH_Gen_i.hxx"

#include "utilities.h"

#include "StdMeshers_Adaptive1D_i.hxx"
#include "StdMeshers_Arithmetic1D_i.hxx"
#include "StdMeshers_AutomaticLength_i.hxx"
#include "StdMeshers_CartesianParameters3D_i.hxx"
#include "StdMeshers_Cartesian_3D_i.hxx"
#include "StdMeshers_CompositeSegment_1D_i.hxx"
#include "StdMeshers_Deflection1D_i.hxx"
#include "StdMeshers_FixedPoints1D_i.hxx"
#include "StdMeshers_Geometric1D_i.hxx"
#include "StdMeshers_Hexa_3D_i.hxx"
#include "StdMeshers_ImportSource1D_i.hxx"
#include "StdMeshers_ImportSource2D_i.hxx"
#include "StdMeshers_Import_1D2D_i.hxx"
#include "StdMeshers_Import_1D_i.hxx"
#include "StdMeshers_LayerDistribution2D_i.hxx"
#include "StdMeshers_LayerDistribution_i.hxx"
#include "StdMeshers_LengthFromEdges_i.hxx"
#include "StdMeshers_LocalLength_i.hxx"
#include "StdMeshers_MaxElementArea_i.hxx"
#include "StdMeshers_MaxElementVolume_i.hxx"
#include "StdMeshers_MaxLength_i.hxx"
#include "StdMeshers_NotConformAllowed_i.hxx"
#include "StdMeshers_NumberOfLayers2D_i.hxx"
#include "StdMeshers_NumberOfLayers_i.hxx"
#include "StdMeshers_NumberOfSegments_i.hxx"
#include "StdMeshers_PolygonPerFace_2D_i.hxx"
#include "StdMeshers_Prism_3D_i.hxx"
#include "StdMeshers_ProjectionSource1D_i.hxx"
#include "StdMeshers_ProjectionSource2D_i.hxx"
#include "StdMeshers_ProjectionSource3D_i.hxx"
#include "StdMeshers_Projection_1D_2D_3D_i.hxx"
#include "StdMeshers_Propagation_i.hxx"
#include "StdMeshers_QuadrangleParams_i.hxx"
#include "StdMeshers_QuadranglePreference_i.hxx"
#include "StdMeshers_Quadrangle_2D_i.hxx"
#include "StdMeshers_QuadraticMesh_i.hxx"
#include "StdMeshers_RadialQuadrangle_1D2D_i.hxx"
#include "StdMeshers_Regular_1D_i.hxx"
#include "StdMeshers_SegmentAroundVertex_0D_i.hxx"
#include "StdMeshers_SegmentLengthAroundVertex_i.hxx"
#include "StdMeshers_StartEndLength_i.hxx"
#include "StdMeshers_UseExisting_1D2D_i.hxx"
#include "StdMeshers_ViscousLayers2D_i.hxx"
#include "StdMeshers_ViscousLayers_i.hxx"
#ifdef ENABLE_MEFISTO
 #include "StdMeshers_MEFISTO_2D_i.hxx"
#endif

namespace SMESH {
  class ApplicableToAny
  {
  public:
    static CORBA::Boolean IsApplicable( const TopoDS_Shape &S, CORBA::Boolean toCheckAll )
    {
      return true;
    }
  };
};
template <class T, class TIsApplicable = SMESH::ApplicableToAny>
class StdHypothesisCreator_i : public HypothesisCreator_i< T >
{
public:
  // as we have 'module StdMeshers' in SMESH_BasicHypothesis.idl
  virtual std::string GetModuleName() { return "StdMeshers"; }
  virtual CORBA::Boolean IsApplicable( const TopoDS_Shape & S, CORBA::Boolean toCheckAll )
  {
    return TIsApplicable::IsApplicable( S, toCheckAll );
  }
};

//=============================================================================
/*!
 * \brief Creates StdMeshers_QuadrangleParams_i instead of
 * StdMeshers_TrianglePreference_i and StdMeshers_QuadranglePreference_i
 */
//=============================================================================

template <StdMeshers::QuadType TYPE>
class QuadrangleParamsCreator : public StdHypothesisCreator_i<StdMeshers_QuadrangleParams_i>
{
public:
  virtual SMESH_Hypothesis_i* Create (PortableServer::POA_ptr thePOA,
                                      int                     theStudyId,
                                      ::SMESH_Gen*            theGenImpl)
  {
    StdMeshers_QuadrangleParams_i* h =
      new StdMeshers_QuadrangleParams_i( thePOA, theStudyId, theGenImpl);
    h->SetQuadType( TYPE );
    return h;
  }
};

//=============================================================================
/*!
 *
 */
//=============================================================================

extern "C"
{
STDMESHERS_I_EXPORT
  GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName)
  {
    GenericHypothesisCreator_i* aCreator = 0;

    // Hypotheses
    if      (strcmp(aHypName, "LocalLength") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_LocalLength_i>;
    else if (strcmp(aHypName, "MaxLength") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_MaxLength_i>;
    else if (strcmp(aHypName, "NumberOfSegments") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_NumberOfSegments_i>;
    else if (strcmp(aHypName, "LengthFromEdges") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_LengthFromEdges_i>;
    else if (strcmp(aHypName, "NotConformAllowed") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_NotConformAllowed_i>;
    else if (strcmp(aHypName, "Propagation") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Propagation_i>;
    else if (strcmp(aHypName, "PropagOfDistribution") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_PropagOfDistribution_i>;
    else if (strcmp(aHypName, "MaxElementArea") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_MaxElementArea_i>;
    else if (strcmp(aHypName, "MaxElementVolume") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_MaxElementVolume_i>;
    else if (strcmp(aHypName, "StartEndLength") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_StartEndLength_i>;
    else if (strcmp(aHypName, "Deflection1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Deflection1D_i>;
    else if (strcmp(aHypName, "Adaptive1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Adaptive1D_i>;
    else if (strcmp(aHypName, "FixedPoints1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_FixedPoints1D_i>;
    else if (strcmp(aHypName, "Arithmetic1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Arithmetic1D_i>;
    else if (strcmp(aHypName, "GeometricProgression") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Geometric1D_i>;
    else if (strcmp(aHypName, "AutomaticLength") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_AutomaticLength_i>;
    else if (strcmp(aHypName, "QuadranglePreference") == 0)
      // do not convert to StdMeshers_QuadrangleParams_i as it is used by NETGEN 2D
      aCreator = new StdHypothesisCreator_i<StdMeshers_QuadranglePreference_i>;
      //aCreator = new QuadrangleParamsCreator< StdMeshers::QUAD_QUADRANGLE_PREF >();
    else if (strcmp(aHypName, "TrianglePreference") == 0)
      //aCreator = new StdHypothesisCreator_i<StdMeshers_TrianglePreference_i>;
      aCreator = new QuadrangleParamsCreator< StdMeshers::QUAD_TRIANGLE_PREF >();
    else if (strcmp(aHypName, "QuadraticMesh") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_QuadraticMesh_i>;
    else if (strcmp(aHypName, "ProjectionSource3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ProjectionSource3D_i>;
    else if (strcmp(aHypName, "ProjectionSource2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ProjectionSource2D_i>;
    else if (strcmp(aHypName, "ProjectionSource1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ProjectionSource1D_i>;
    else if (strcmp(aHypName, "NumberOfLayers") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_NumberOfLayers_i>;
    else if (strcmp(aHypName, "LayerDistribution") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_LayerDistribution_i>;
    else if (strcmp(aHypName, "NumberOfLayers2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_NumberOfLayers2D_i>;
    else if (strcmp(aHypName, "LayerDistribution2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_LayerDistribution2D_i>;
    else if (strcmp(aHypName, "SegmentLengthAroundVertex") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_SegmentLengthAroundVertex_i>;
    else if (strcmp(aHypName, "QuadrangleParams") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_QuadrangleParams_i>;
    else if (strcmp(aHypName, "ImportSource1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ImportSource1D_i>;
    else if (strcmp(aHypName, "ImportSource2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ImportSource2D_i>;
    else if (strcmp(aHypName, "ViscousLayers") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ViscousLayers_i>;
    else if (strcmp(aHypName, "ViscousLayers2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_ViscousLayers2D_i>;
    else if (strcmp(aHypName, "CartesianParameters3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_CartesianParameters3D_i>;

    // Algorithms
    else if (strcmp(aHypName, "Regular_1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Regular_1D_i>;
#ifdef ENABLE_MEFISTO
    else if (strcmp(aHypName, "MEFISTO_2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_MEFISTO_2D_i>;
#endif
    else if (strcmp(aHypName, "Quadrangle_2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Quadrangle_2D_i, StdMeshers_Quadrangle_2D_i>;
    else if (strcmp(aHypName, "QuadFromMedialAxis_1D2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_QuadFromMedialAxis_1D2D_i, StdMeshers_QuadFromMedialAxis_1D2D_i>;
    else if (strcmp(aHypName, "Hexa_3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Hexa_3D_i, StdMeshers_Hexa_3D_i>;
    else if (strcmp(aHypName, "Projection_1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Projection_1D_i>;
    else if (strcmp(aHypName, "Projection_1D2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Projection_1D2D_i>;
    else if (strcmp(aHypName, "Projection_2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Projection_2D_i>;
    else if (strcmp(aHypName, "Projection_3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Projection_3D_i, StdMeshers_Projection_3D_i>;
    else if (strcmp(aHypName, "Prism_3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Prism_3D_i, StdMeshers_Prism_3D_i>;
    else if (strcmp(aHypName, "RadialPrism_3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_RadialPrism_3D_i, StdMeshers_RadialPrism_3D_i>;
    else if (strcmp(aHypName, "SegmentAroundVertex_0D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_SegmentAroundVertex_0D_i>;
    else if (strcmp(aHypName, "CompositeSegment_1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_CompositeSegment_1D_i>;
    else if (strcmp(aHypName, "UseExisting_1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_UseExisting_1D_i>;
    else if (strcmp(aHypName, "UseExisting_2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_UseExisting_2D_i>;
    else if (strcmp(aHypName, "RadialQuadrangle_1D2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_RadialQuadrangle_1D2D_i, StdMeshers_RadialQuadrangle_1D2D_i>;
    else if (strcmp(aHypName, "Import_1D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Import_1D_i>;
    else if (strcmp(aHypName, "Import_1D2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Import_1D2D_i>;
    else if (strcmp(aHypName, "Cartesian_3D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_Cartesian_3D_i>;
    else if (strcmp(aHypName, "PolygonPerFace_2D") == 0)
      aCreator = new StdHypothesisCreator_i<StdMeshers_PolygonPerFace_2D_i>;
    else ;

    return aCreator;
  }
}
