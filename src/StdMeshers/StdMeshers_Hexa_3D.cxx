//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  SMESH SMESH : implementaion of SMESH idl descriptions
//  File   : StdMeshers_Hexa_3D.cxx
//           Moved here from SMESH_Hexa_3D.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#include "StdMeshers_Hexa_3D.hxx"

#include "StdMeshers_CompositeHexa_3D.hxx"
#include "StdMeshers_FaceSide.hxx"
#include "StdMeshers_HexaFromSkin_3D.hxx"
#include "StdMeshers_Penta_3D.hxx"
#include "StdMeshers_Prism_3D.hxx"
#include "StdMeshers_Quadrangle_2D.hxx"
#include "StdMeshers_ViscousLayers.hxx"

#include "SMESH_Comment.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMDS_VolumeOfNodes.hxx"

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt2d.hxx>

#include "utilities.h"
#include "Utils_ExceptHandlers.hxx"

typedef SMESH_Comment TComm;

using namespace std;

static SMESH_ComputeErrorPtr ComputePentahedralMesh(SMESH_Mesh &,
                                                    const TopoDS_Shape &,
                                                    SMESH_ProxyMesh* proxyMesh=0);

static bool EvaluatePentahedralMesh(SMESH_Mesh &, const TopoDS_Shape &,
                                    MapShapeNbElems &);

//=============================================================================
/*!
 * Constructor
 */
//=============================================================================

StdMeshers_Hexa_3D::StdMeshers_Hexa_3D(int hypId, int studyId, SMESH_Gen * gen)
  :SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("StdMeshers_Hexa_3D::StdMeshers_Hexa_3D");
  _name = "Hexa_3D";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);       // 1 bit /shape type
  _requireShape = false;
  _compatibleHypothesis.push_back("ViscousLayers");
}

//=============================================================================
/*!
 * Destructor
 */
//=============================================================================

StdMeshers_Hexa_3D::~StdMeshers_Hexa_3D()
{
  MESSAGE("StdMeshers_Hexa_3D::~StdMeshers_Hexa_3D");
}

//=============================================================================
/*!
 * Retrieves defined hypotheses
 */
//=============================================================================

bool StdMeshers_Hexa_3D::CheckHypothesis
                         (SMESH_Mesh&                          aMesh,
                          const TopoDS_Shape&                  aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  // check nb of faces in the shape
/*  PAL16229
  aStatus = SMESH_Hypothesis::HYP_BAD_GEOMETRY;
  int nbFaces = 0;
  for (TopExp_Explorer exp(aShape, TopAbs_FACE); exp.More(); exp.Next())
    if ( ++nbFaces > 6 )
      break;
  if ( nbFaces != 6 )
    return false;
*/

  _viscousLayersHyp = NULL;

  const list<const SMESHDS_Hypothesis*>& hyps =
    GetUsedHypothesis(aMesh, aShape, /*ignoreAuxiliary=*/false);
  list <const SMESHDS_Hypothesis* >::const_iterator h = hyps.begin();
  if ( h == hyps.end())
  {
    aStatus = SMESH_Hypothesis::HYP_OK;
    return true;
  }

  aStatus = HYP_OK;
  for ( ; h != hyps.end(); ++h )
  {
    string hypName = (*h)->GetName();
    if ( find( _compatibleHypothesis.begin(),_compatibleHypothesis.end(),hypName )
         != _compatibleHypothesis.end() )
    {
      _viscousLayersHyp = dynamic_cast< const StdMeshers_ViscousLayers*> ( *h );
    }
    else
    {
      aStatus = HYP_INCOMPATIBLE;
    }
  }

  if ( !_viscousLayersHyp )
    aStatus = HYP_INCOMPATIBLE;

  return aStatus == HYP_OK;
}

namespace
{
  //=============================================================================

  // symbolic names of box sides
  enum EBoxSides{ B_BOTTOM=0, B_RIGHT, B_TOP, B_LEFT, B_FRONT, B_BACK, B_NB_SIDES };

  // indices of FACE's of box sides in terms of SMESH_Block::TShapeID enum
  enum ESideIndex{ I_BOTTOM    = SMESH_Block::ID_Fxy0,
                   I_RIGHT     = SMESH_Block::ID_F1yz,
                   I_TOP       = SMESH_Block::ID_Fxy1,
                   I_LEFT      = SMESH_Block::ID_F0yz,
                   I_FRONT     = SMESH_Block::ID_Fx0z,
                   I_BACK      = SMESH_Block::ID_Fx1z,
                   I_UNDEFINED = SMESH_Block::ID_NONE
  };
  //=============================================================================
  /*!
   * \brief Container of nodes of structured mesh on a qudrangular geom FACE
   */
  struct _FaceGrid
  {
    // map of (node parameter on EDGE) to (column (vector) of nodes)
    TParam2ColumnMap _u2nodesMap;

    // node column's taken form _u2nodesMap taking into account sub-shape orientation
    vector<TNodeColumn> _columns;

    // geometry of a cube side
    TopoDS_Face _sideF;
    TopoDS_Edge _baseE;

    const SMDS_MeshNode* GetNode(int iCol, int iRow) const
    {
      return _columns[iCol][iRow];
    }
    gp_XYZ GetXYZ(int iCol, int iRow) const
    {
      return SMESH_MeshEditor::TNodeXYZ( GetNode( iCol, iRow ));
    }
  };

  //================================================================================
  /*!
   * \brief Convertor of a pair of integers to a sole index
   */
  struct _Indexer
  {
    int _xSize, _ySize;
    _Indexer( int xSize, int ySize ): _xSize(xSize), _ySize(ySize) {}
    int size() const { return _xSize * _ySize; }
    int operator()(const int x, const int y) const { return y * _xSize + x; }
  };
}

//=============================================================================
/*!
 * Generates hexahedron mesh on hexaedron like form using algorithm from
 * "Application de l'interpolation transfinie à la création de maillages
 *  C0 ou G1 continus sur des triangles, quadrangles, tetraedres, pentaedres
 *  et hexaedres déformés."
 * Alain PERONNET - 8 janvier 1999
 */
//=============================================================================

bool StdMeshers_Hexa_3D::Compute(SMESH_Mesh &         aMesh,
                                 const TopoDS_Shape & aShape)// throw(SALOME_Exception)
{
  // PAL14921. Enable catching std::bad_alloc and Standard_OutOfMemory outside
  //Unexpect aCatch(SalomeException);
  MESSAGE("StdMeshers_Hexa_3D::Compute");
  SMESHDS_Mesh * meshDS = aMesh.GetMeshDS();

  // 1) Shape verification
  // ----------------------

  // shape must be a solid (or a shell) with 6 faces
  TopoDS_Shell shell;
  TopExp_Explorer exp(aShape,TopAbs_SHELL);
  if ( !exp.More() )
    return error(COMPERR_BAD_SHAPE, "No SHELL in the geometry");
  shell = TopoDS::Shell( exp.Current());
  if ( exp.Next(), exp.More() )
    return error(COMPERR_BAD_SHAPE, "More than one SHELL in the geometry");

  TopTools_IndexedMapOfShape FF;
  TopExp::MapShapes( shell, TopAbs_FACE, FF);
  if ( FF.Extent() != 6)
  {
    static StdMeshers_CompositeHexa_3D compositeHexa(_gen->GetANewId(), 0, _gen);
    if ( !compositeHexa.Compute( aMesh, aShape ))
      return error( compositeHexa.GetComputeError() );
    return true;
  }

  // Find sub-shapes of a cube
  TopTools_IndexedMapOfOrientedShape cubeSubShapes;
  TopoDS_Vertex V;
  if ( !SMESH_Block::FindBlockShapes( shell, V,V, cubeSubShapes ))
    return error(COMPERR_BAD_SHAPE, "Not a 6 sides cube");

  // 2) make viscous layers

  SMESH_ProxyMesh::Ptr proxymesh;
  if ( _viscousLayersHyp )
  {
    proxymesh = _viscousLayersHyp->Compute( aMesh, aShape, /*makeN2NMap=*/ true );
    if ( !proxymesh )
      return false;
  }

  // 3) Check presence of regular grid mesh on FACEs of the cube
  // ------------------------------------------------------------

  // indices of FACEs of cube sides within cubeSubShapes
  const int sideIndex[6] = { I_BOTTOM, I_RIGHT, I_TOP, I_LEFT, I_FRONT, I_BACK };
  // indices of base EDGEs of cube sides within cubeSubShapes, corresponding to sideIndex
  const int baseEdgeIndex[6] = {
    SMESH_Block::ID_Ex00, // bottom side
    SMESH_Block::ID_E1y0, // right side
    SMESH_Block::ID_Ex01, // top side
    SMESH_Block::ID_E0y0, // left side
    SMESH_Block::ID_Ex00, // front side
    SMESH_Block::ID_Ex10  // back side
  };

  // Load mesh of cube sides

  _FaceGrid aCubeSide[ 6 ] ;

  // tool creating quadratic elements if needed
  SMESH_MesherHelper helper (aMesh);
  _quadraticMesh = helper.IsQuadraticSubMesh(aShape);

  for ( int i = 0; i < 6; ++i )
  {
    aCubeSide[i]._sideF = TopoDS::Face( cubeSubShapes( sideIndex[i] ));
    aCubeSide[i]._baseE = TopoDS::Edge( cubeSubShapes( baseEdgeIndex[i] ));

    // assure correctness of node positions on _baseE
    if ( SMESHDS_SubMesh* smDS = meshDS->MeshElements( aCubeSide[i]._baseE ))
    {
      bool ok;
      helper.SetSubShape( aCubeSide[i]._baseE );
      SMDS_ElemIteratorPtr eIt = smDS->GetElements();
      while ( eIt->more() )
      {
        const SMDS_MeshElement* e = eIt->next();
        helper.GetNodeU( aCubeSide[i]._baseE, e->GetNode(0), e->GetNode(1), &ok);
        helper.GetNodeU( aCubeSide[i]._baseE, e->GetNode(1), e->GetNode(0), &ok);
      }
    }

    // load grid
    if ( !helper.LoadNodeColumns( aCubeSide[i]._u2nodesMap,
                                  aCubeSide[i]._sideF,
                                  aCubeSide[i]._baseE, meshDS, proxymesh.get() ))
    {
      SMESH_ComputeErrorPtr err = ComputePentahedralMesh(aMesh, aShape, proxymesh.get());
      return error( err );
    }

    // check if there are triangles on aCubeSide[i]._sideF
    if ( aMesh.NbTriangles() > 0 )
    {
      if ( SMESHDS_SubMesh* smDS = meshDS->MeshElements( aCubeSide[i]._sideF ))
      {
        bool isAllQuad = true;
        SMDS_ElemIteratorPtr fIt = smDS->GetElements();
        while ( fIt->more() && isAllQuad )
        {
          const SMDS_MeshElement* f = fIt->next();
          isAllQuad = ( f->NbCornerNodes() == 4 );
        }
        if ( !isAllQuad )
        {
          SMESH_ComputeErrorPtr err = ComputePentahedralMesh(aMesh, aShape, proxymesh.get());
          return error( err );
        }
      }
    }
  }

  // Orient loaded grids of cube sides along axis of the unitary cube coord system
  for ( int i = 0; i < 6; ++i )
  {
    bool reverse = false;
    if ( helper.GetSubShapeOri( shell.Oriented( TopAbs_FORWARD ),
                                aCubeSide[i]._sideF ) == TopAbs_REVERSED )
      reverse = !reverse;

    if ( helper.GetSubShapeOri( aCubeSide[i]._sideF.Oriented( TopAbs_FORWARD ),
                                aCubeSide[i]._baseE ) == TopAbs_REVERSED )
      reverse = !reverse;

    if ( sideIndex[i] == I_BOTTOM ||
         sideIndex[i] == I_LEFT   ||
         sideIndex[i] == I_BACK )
      reverse = !reverse;

    aCubeSide[i]._columns.resize( aCubeSide[i]._u2nodesMap.size() );

    int iFwd = 0, iRev = aCubeSide[i]._columns.size()-1;
    int* pi = reverse ? &iRev : &iFwd;
    TParam2ColumnMap::iterator u2nn = aCubeSide[i]._u2nodesMap.begin();
    for ( ; iFwd < aCubeSide[i]._columns.size(); --iRev, ++iFwd, ++u2nn )
      aCubeSide[i]._columns[ *pi ].swap( u2nn->second );

    aCubeSide[i]._u2nodesMap.clear();
  }
  
  if ( proxymesh )
    for ( int i = 0; i < 6; ++i )
      for ( unsigned j = 0; j < aCubeSide[i]._columns.size(); ++j)
        for ( unsigned k = 0; k < aCubeSide[i]._columns[j].size(); ++k)
        {
          const SMDS_MeshNode* & n = aCubeSide[i]._columns[j][k];
          n = proxymesh->GetProxyNode( n );
        }

  // 4) Create internal nodes of the cube
  // -------------------------------------

  helper.SetSubShape( aShape );
  helper.SetElementsOnShape(true);

  // shortcuts to sides
  _FaceGrid* fBottom = & aCubeSide[ B_BOTTOM ];
  _FaceGrid* fRight  = & aCubeSide[ B_RIGHT  ];
  _FaceGrid* fTop    = & aCubeSide[ B_TOP    ];
  _FaceGrid* fLeft   = & aCubeSide[ B_LEFT   ];
  _FaceGrid* fFront  = & aCubeSide[ B_FRONT  ];
  _FaceGrid* fBack   = & aCubeSide[ B_BACK   ];

  // cube size measured in nb of nodes
  int x, xSize = fBottom->_columns.size() , X = xSize - 1;
  int y, ySize = fLeft->_columns.size()   , Y = ySize - 1;
  int z, zSize = fLeft->_columns[0].size(), Z = zSize - 1;

  // columns of internal nodes "rising" from nodes of fBottom
  _Indexer colIndex( xSize, ySize );
  vector< vector< const SMDS_MeshNode* > > columns( colIndex.size() );

  // fill node columns by front and back box sides
  for ( x = 0; x < xSize; ++x ) {
    vector< const SMDS_MeshNode* >& column0 = columns[ colIndex( x, 0 )];
    vector< const SMDS_MeshNode* >& column1 = columns[ colIndex( x, Y )];
    column0.resize( zSize );
    column1.resize( zSize );
    for ( z = 0; z < zSize; ++z ) {
      column0[ z ] = fFront->GetNode( x, z );
      column1[ z ] = fBack ->GetNode( x, z );
    }
  }
  // fill node columns by left and right box sides
  for ( y = 1; y < ySize-1; ++y ) {
    vector< const SMDS_MeshNode* >& column0 = columns[ colIndex( 0, y )];
    vector< const SMDS_MeshNode* >& column1 = columns[ colIndex( X, y )];
    column0.resize( zSize );
    column1.resize( zSize );
    for ( z = 0; z < zSize; ++z ) {
      column0[ z ] = fLeft ->GetNode( y, z );
      column1[ z ] = fRight->GetNode( y, z );
    }
  }
  // get nodes from top and bottom box sides
  for ( x = 1; x < xSize-1; ++x ) {
    for ( y = 1; y < ySize-1; ++y ) {
      vector< const SMDS_MeshNode* >& column = columns[ colIndex( x, y )];
      column.resize( zSize );
      column.front() = fBottom->GetNode( x, y );
      column.back()  = fTop   ->GetNode( x, y );
    }
  }

  // projection points of the internal node on cube sub-shapes by which
  // coordinates of the internal node are computed
  vector<gp_XYZ> pointsOnShapes( SMESH_Block::ID_Shell );

  // projections on vertices are constant
  pointsOnShapes[ SMESH_Block::ID_V000 ] = fBottom->GetXYZ( 0, 0 );
  pointsOnShapes[ SMESH_Block::ID_V100 ] = fBottom->GetXYZ( X, 0 );
  pointsOnShapes[ SMESH_Block::ID_V010 ] = fBottom->GetXYZ( 0, Y );
  pointsOnShapes[ SMESH_Block::ID_V110 ] = fBottom->GetXYZ( X, Y );
  pointsOnShapes[ SMESH_Block::ID_V001 ] = fTop->GetXYZ( 0, 0 );
  pointsOnShapes[ SMESH_Block::ID_V101 ] = fTop->GetXYZ( X, 0 );
  pointsOnShapes[ SMESH_Block::ID_V011 ] = fTop->GetXYZ( 0, Y );
  pointsOnShapes[ SMESH_Block::ID_V111 ] = fTop->GetXYZ( X, Y );

  for ( x = 1; x < xSize-1; ++x )
  {
    gp_XYZ params; // normalized parameters of internal node within a unit box
    params.SetCoord( 1, x / double(X) );
    for ( y = 1; y < ySize-1; ++y )
    {
      params.SetCoord( 2, y / double(Y) );
      // a column to fill in during z loop
      vector< const SMDS_MeshNode* >& column = columns[ colIndex( x, y )];
      // projection points on horizontal edges
      pointsOnShapes[ SMESH_Block::ID_Ex00 ] = fBottom->GetXYZ( x, 0 );
      pointsOnShapes[ SMESH_Block::ID_Ex10 ] = fBottom->GetXYZ( x, Y );
      pointsOnShapes[ SMESH_Block::ID_E0y0 ] = fBottom->GetXYZ( 0, y );
      pointsOnShapes[ SMESH_Block::ID_E1y0 ] = fBottom->GetXYZ( X, y );
      pointsOnShapes[ SMESH_Block::ID_Ex01 ] = fTop->GetXYZ( x, 0 );
      pointsOnShapes[ SMESH_Block::ID_Ex11 ] = fTop->GetXYZ( x, Y );
      pointsOnShapes[ SMESH_Block::ID_E0y1 ] = fTop->GetXYZ( 0, y );
      pointsOnShapes[ SMESH_Block::ID_E1y1 ] = fTop->GetXYZ( X, y );
      // projection points on horizontal faces
      pointsOnShapes[ SMESH_Block::ID_Fxy0 ] = fBottom->GetXYZ( x, y );
      pointsOnShapes[ SMESH_Block::ID_Fxy1 ] = fTop   ->GetXYZ( x, y );
      for ( z = 1; z < zSize-1; ++z ) // z loop
      {
        params.SetCoord( 3, z / double(Z) );
        // projection points on vertical edges
        pointsOnShapes[ SMESH_Block::ID_E00z ] = fFront->GetXYZ( 0, z );    
        pointsOnShapes[ SMESH_Block::ID_E10z ] = fFront->GetXYZ( X, z );    
        pointsOnShapes[ SMESH_Block::ID_E01z ] = fBack->GetXYZ( 0, z );    
        pointsOnShapes[ SMESH_Block::ID_E11z ] = fBack->GetXYZ( X, z );
        // projection points on vertical faces
        pointsOnShapes[ SMESH_Block::ID_Fx0z ] = fFront->GetXYZ( x, z );    
        pointsOnShapes[ SMESH_Block::ID_Fx1z ] = fBack ->GetXYZ( x, z );    
        pointsOnShapes[ SMESH_Block::ID_F0yz ] = fLeft ->GetXYZ( y, z );    
        pointsOnShapes[ SMESH_Block::ID_F1yz ] = fRight->GetXYZ( y, z );

        // compute internal node coordinates
        gp_XYZ coords;
        SMESH_Block::ShellPoint( params, pointsOnShapes, coords );
        column[ z ] = helper.AddNode( coords.X(), coords.Y(), coords.Z() );

      }
    }
  }

  // side data no more needed, free memory
  for ( int i = 0; i < 6; ++i )
    aCubeSide[i]._columns.clear();

  // 5) Create hexahedrons
  // ---------------------

  for ( x = 0; x < xSize-1; ++x ) {
    for ( y = 0; y < ySize-1; ++y ) {
      vector< const SMDS_MeshNode* >& col00 = columns[ colIndex( x, y )];
      vector< const SMDS_MeshNode* >& col10 = columns[ colIndex( x+1, y )];
      vector< const SMDS_MeshNode* >& col01 = columns[ colIndex( x, y+1 )];
      vector< const SMDS_MeshNode* >& col11 = columns[ colIndex( x+1, y+1 )];
      for ( z = 0; z < zSize-1; ++z )
      {
        // bottom face normal of a hexa mush point outside the volume
        helper.AddVolume(col00[z],   col01[z],   col11[z],   col10[z],
                         col00[z+1], col01[z+1], col11[z+1], col10[z+1]);
      }
    }
  }
  return true;
}

//=============================================================================
/*!
 *  Evaluate
 */
//=============================================================================

bool StdMeshers_Hexa_3D::Evaluate(SMESH_Mesh & aMesh,
                                  const TopoDS_Shape & aShape,
                                  MapShapeNbElems& aResMap)
{
  vector < SMESH_subMesh * >meshFaces;
  TopTools_SequenceOfShape aFaces;
  for (TopExp_Explorer exp(aShape, TopAbs_FACE); exp.More(); exp.Next()) {
    aFaces.Append(exp.Current());
    SMESH_subMesh *aSubMesh = aMesh.GetSubMeshContaining(exp.Current());
    ASSERT(aSubMesh);
    meshFaces.push_back(aSubMesh);
  }
  if (meshFaces.size() != 6) {
    //return error(COMPERR_BAD_SHAPE, TComm(meshFaces.size())<<" instead of 6 faces in a block");
    static StdMeshers_CompositeHexa_3D compositeHexa(-10, 0, aMesh.GetGen());
    return compositeHexa.Evaluate(aMesh, aShape, aResMap);
  }
  
  int i = 0;
  for(; i<6; i++) {
    //TopoDS_Shape aFace = meshFaces[i]->GetSubShape();
    TopoDS_Shape aFace = aFaces.Value(i+1);
    SMESH_Algo *algo = _gen->GetAlgo(aMesh, aFace);
    if( !algo ) {
      std::vector<int> aResVec(SMDSEntity_Last);
      for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aResVec[i] = 0;
      SMESH_subMesh * sm = aMesh.GetSubMesh(aShape);
      aResMap.insert(std::make_pair(sm,aResVec));
      SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
      smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,"Submesh can not be evaluated",this));
      return false;
    }
    string algoName = algo->GetName();
    bool isAllQuad = false;
    if (algoName == "Quadrangle_2D") {
      MapShapeNbElemsItr anIt = aResMap.find(meshFaces[i]);
      if( anIt == aResMap.end() ) continue;
      std::vector<int> aVec = (*anIt).second;
      int nbtri = Max(aVec[SMDSEntity_Triangle],aVec[SMDSEntity_Quad_Triangle]);
      if( nbtri == 0 )
        isAllQuad = true;
    }
    if ( ! isAllQuad ) {
      return EvaluatePentahedralMesh(aMesh, aShape, aResMap);
    }
  }
  
  // find number of 1d elems for 1 face
  int nb1d = 0;
  TopTools_MapOfShape Edges1;
  bool IsQuadratic = false;
  bool IsFirst = true;
  for (TopExp_Explorer exp(aFaces.Value(1), TopAbs_EDGE); exp.More(); exp.Next()) {
    Edges1.Add(exp.Current());
    SMESH_subMesh *sm = aMesh.GetSubMesh(exp.Current());
    if( sm ) {
      MapShapeNbElemsItr anIt = aResMap.find(sm);
      if( anIt == aResMap.end() ) continue;
      std::vector<int> aVec = (*anIt).second;
      nb1d += Max(aVec[SMDSEntity_Edge],aVec[SMDSEntity_Quad_Edge]);
      if(IsFirst) {
        IsQuadratic = (aVec[SMDSEntity_Quad_Edge] > aVec[SMDSEntity_Edge]);
        IsFirst = false;
      }
    }
  }
  // find face opposite to 1 face
  int OppNum = 0;
  for(i=2; i<=6; i++) {
    bool IsOpposite = true;
    for(TopExp_Explorer exp(aFaces.Value(i), TopAbs_EDGE); exp.More(); exp.Next()) {
      if( Edges1.Contains(exp.Current()) ) {
        IsOpposite = false;
        break;
      }
    }
    if(IsOpposite) {
      OppNum = i;
      break;
    }
  }
  // find number of 2d elems on side faces
  int nb2d = 0;
  for(i=2; i<=6; i++) {
    if( i == OppNum ) continue;
    MapShapeNbElemsItr anIt = aResMap.find( meshFaces[i-1] );
    if( anIt == aResMap.end() ) continue;
    std::vector<int> aVec = (*anIt).second;
    nb2d += Max(aVec[SMDSEntity_Quadrangle],aVec[SMDSEntity_Quad_Quadrangle]);
  }
  
  MapShapeNbElemsItr anIt = aResMap.find( meshFaces[0] );
  std::vector<int> aVec = (*anIt).second;
  int nb2d_face0 = Max(aVec[SMDSEntity_Quadrangle],aVec[SMDSEntity_Quad_Quadrangle]);
  int nb0d_face0 = aVec[SMDSEntity_Node];

  std::vector<int> aResVec(SMDSEntity_Last);
  for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aResVec[i] = 0;
  if(IsQuadratic) {
    aResVec[SMDSEntity_Quad_Hexa] = nb2d_face0 * ( nb2d/nb1d );
    int nb1d_face0_int = ( nb2d_face0*4 - nb1d ) / 2;
    aResVec[SMDSEntity_Node] = nb0d_face0 * ( 2*nb2d/nb1d - 1 ) - nb1d_face0_int * nb2d/nb1d;
  }
  else {
    aResVec[SMDSEntity_Node] = nb0d_face0 * ( nb2d/nb1d - 1 );
    aResVec[SMDSEntity_Hexa] = nb2d_face0 * ( nb2d/nb1d );
  }
  SMESH_subMesh * sm = aMesh.GetSubMesh(aShape);
  aResMap.insert(std::make_pair(sm,aResVec));

  return true;
}

//================================================================================
/*!
 * \brief Computes hexahedral mesh from 2D mesh of block
 */
//================================================================================

bool StdMeshers_Hexa_3D::Compute(SMESH_Mesh & aMesh, SMESH_MesherHelper* aHelper)
{
  static StdMeshers_HexaFromSkin_3D * algo = 0;
  if ( !algo ) {
    SMESH_Gen* gen = aMesh.GetGen();
    algo = new StdMeshers_HexaFromSkin_3D( gen->GetANewId(), 0, gen );
  }
  algo->InitComputeError();
  algo->Compute( aMesh, aHelper );
  return error( algo->GetComputeError());
}

//=======================================================================
//function : ComputePentahedralMesh
//purpose  : 
//=======================================================================

SMESH_ComputeErrorPtr ComputePentahedralMesh(SMESH_Mesh &          aMesh,
                                             const TopoDS_Shape &  aShape,
                                             SMESH_ProxyMesh*      proxyMesh)
{
  SMESH_ComputeErrorPtr err = SMESH_ComputeError::New();
  if ( proxyMesh )
  {
    err->myName = COMPERR_BAD_INPUT_MESH;
    err->myComment = "Can't build pentahedral mesh on viscous layers";
    return err;
  }
  bool bOK;
  StdMeshers_Penta_3D anAlgo;
  //
  bOK=anAlgo.Compute(aMesh, aShape);
  //
  err = anAlgo.GetComputeError();
  //
  if ( !bOK && anAlgo.ErrorStatus() == 5 )
  {
    static StdMeshers_Prism_3D * aPrism3D = 0;
    if ( !aPrism3D ) {
      SMESH_Gen* gen = aMesh.GetGen();
      aPrism3D = new StdMeshers_Prism_3D( gen->GetANewId(), 0, gen );
    }
    SMESH_Hypothesis::Hypothesis_Status aStatus;
    if ( aPrism3D->CheckHypothesis( aMesh, aShape, aStatus ) ) {
      aPrism3D->InitComputeError();
      bOK = aPrism3D->Compute( aMesh, aShape );
      err = aPrism3D->GetComputeError();
    }
  }
  return err;
}


//=======================================================================
//function : EvaluatePentahedralMesh
//purpose  : 
//=======================================================================

bool EvaluatePentahedralMesh(SMESH_Mesh & aMesh,
                             const TopoDS_Shape & aShape,
                             MapShapeNbElems& aResMap)
{
  StdMeshers_Penta_3D anAlgo;
  bool bOK = anAlgo.Evaluate(aMesh, aShape, aResMap);

  //err = anAlgo.GetComputeError();
  //if ( !bOK && anAlgo.ErrorStatus() == 5 )
  if( !bOK ) {
    static StdMeshers_Prism_3D * aPrism3D = 0;
    if ( !aPrism3D ) {
      SMESH_Gen* gen = aMesh.GetGen();
      aPrism3D = new StdMeshers_Prism_3D( gen->GetANewId(), 0, gen );
    }
    SMESH_Hypothesis::Hypothesis_Status aStatus;
    if ( aPrism3D->CheckHypothesis( aMesh, aShape, aStatus ) ) {
      return aPrism3D->Evaluate(aMesh, aShape, aResMap);
    }
  }

  return bOK;
}
