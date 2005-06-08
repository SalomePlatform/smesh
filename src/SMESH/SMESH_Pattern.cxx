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

// File      : SMESH_Pattern.hxx
// Created   : Mon Aug  2 10:30:00 2004
// Author    : Edward AGAPOV (eap)

#include "SMESH_Pattern.hxx"

#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_Box2d.hxx>
#include <ElSLib.hxx>
#include <Extrema_GenExtPS.hxx>
#include <Extrema_POnSurf.hxx>
#include <Geom2d_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax2.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_XY.hxx>
#include <gp_XYZ.hxx>

#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Block.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_subMesh.hxx"

#include "utilities.h"

using namespace std;

typedef map< const SMDS_MeshElement*, int > TNodePointIDMap;

//=======================================================================
//function : SMESH_Pattern
//purpose  : 
//=======================================================================

SMESH_Pattern::SMESH_Pattern ()
{
}
//=======================================================================
//function : getInt
//purpose  : 
//=======================================================================

static inline int getInt( const char * theSring )
{
  if ( *theSring < '0' || *theSring > '9' )
    return -1;

  char *ptr;
  int val = strtol( theSring, &ptr, 10 );
  if ( ptr == theSring ||
      // there must not be neither '.' nor ',' nor 'E' ...
      (*ptr != ' ' && *ptr != '\n' && *ptr != '\0'))
    return -1;

  return val;
}

//=======================================================================
//function : getDouble
//purpose  : 
//=======================================================================

static inline double getDouble( const char * theSring )
{
  char *ptr;
  return strtod( theSring, &ptr );
}

//=======================================================================
//function : readLine
//purpose  : Put token starting positions in theFields until '\n' or '\0'
//           Return the number of the found tokens
//=======================================================================

static int readLine (list <const char*> & theFields,
                     const char*        & theLineBeg,
                     const bool           theClearFields )
{
  if ( theClearFields )
    theFields.clear();

  //  algo:
  /*  loop                                                       */
  /*    switch ( symbol ) {                                      */
  /*    case white-space:                                        */
  /*      look for a non-space symbol;                           */
  /*    case string-end:                                         */
  /*    case line-end:                                           */
  /*      exit;                                                  */
  /*    case comment beginning:                                  */
  /*      skip all till a line-end;                              */
  /*    case a number                                            */
  /*      put its position in theFields, skip till a white-space;*/
  /*    default:                                                 */
  /*      abort;                                                 */
  /*  till line-end                                              */

  int nbRead = 0;
  bool stopReading = false;
  do {
    bool goOn = true;
    bool isNumber = false;
    switch ( *theLineBeg )
    {
    case ' ':  // white space
    case '\t': // tab
    case 13:   // ^M
      break;

    case '\n': // a line ends
      stopReading = ( nbRead > 0 );
      break;

    case '!':  // comment
      do theLineBeg++;
      while ( *theLineBeg != '\n' && *theLineBeg != '\0' );
      goOn = false;
      break;

    case '\0': // file ends
      return nbRead;

    case '-': // real number
    case '+':
    case '.':
      isNumber = true;
    default: // data
      isNumber = isNumber || ( *theLineBeg >= '0' && *theLineBeg <= '9' );
      if ( isNumber ) {
        theFields.push_back( theLineBeg );
        nbRead++;
        do theLineBeg++;
        while (*theLineBeg != ' ' &&
               *theLineBeg != '\n' &&
               *theLineBeg != '\0');
        goOn = false;
      }
      else
        return 0; // incorrect file format
    }

    if ( goOn )
      theLineBeg++;

  } while ( !stopReading );

  return nbRead;
}

//=======================================================================
//function : Load
//purpose  : Load a pattern from <theFile>
//=======================================================================

bool SMESH_Pattern::Load (const char* theFileContents)
{
  MESSAGE("Load( file ) ");

  // file structure:

  // ! This is a comment
  // NB_POINTS               ! 1 integer - the number of points in the pattern.
  //   X1 Y1 [Z1]            ! 2 or 3 reals - nodes coordinates within 2D or 3D domain:
  //   X2 Y2 [Z2]            ! the pattern dimention is defined by the number of coordinates
  //   ...
  // [ ID1 ID2 ... IDn ]     ! Indices of key-points for a 2D pattern (only).
  // ! elements description goes after all
  // ID1 ID2 ... IDn         ! 2-4 or 4-8 integers - nodal connectivity of a 2D or 3D element.
  // ...

  Clear();

  const char* lineBeg = theFileContents;
  list <const char*> fields;
  const bool clearFields = true;

  // NB_POINTS               ! 1 integer - the number of points in the pattern.

  if ( readLine( fields, lineBeg, clearFields ) != 1 ) {
    MESSAGE("Error reading NB_POINTS");
    return setErrorCode( ERR_READ_NB_POINTS );
  }
  int nbPoints = getInt( fields.front() );

  //   X1 Y1 [Z1]            ! 2 or 3 reals - nodes coordinates within 2D or 3D domain:

  // read the first point coordinates to define pattern dimention
  int dim = readLine( fields, lineBeg, clearFields );
  if ( dim == 2 )
    myIs2D = true;
  else if ( dim == 3 )
    myIs2D = false;
  else {
    MESSAGE("Error reading points: wrong nb of coordinates");
    return setErrorCode( ERR_READ_POINT_COORDS );
  }
  if ( nbPoints <= dim ) {
    MESSAGE(" Too few points ");
    return setErrorCode( ERR_READ_TOO_FEW_POINTS );
  }
    
  // read the rest points
  int iPoint;
  for ( iPoint = 1; iPoint < nbPoints; iPoint++ )
    if ( readLine( fields, lineBeg, !clearFields ) != dim ) {
      MESSAGE("Error reading  points : wrong nb of coordinates ");
      return setErrorCode( ERR_READ_POINT_COORDS );
    }
  // store point coordinates
  myPoints.resize( nbPoints );
  list <const char*>::iterator fIt = fields.begin();
  for ( iPoint = 0; iPoint < nbPoints; iPoint++ )
  {
    TPoint & p = myPoints[ iPoint ];
    for ( int iCoord = 1; iCoord <= dim; iCoord++, fIt++ )
    {
      double coord = getDouble( *fIt );
      if ( !myIs2D && ( coord < 0.0 || coord > 1.0 )) {
        MESSAGE("Error reading 3D points, value should be in [0,1]: " << coord);
        Clear();
        return setErrorCode( ERR_READ_3D_COORD );
      }
      p.myInitXYZ.SetCoord( iCoord, coord );
      if ( myIs2D )
        p.myInitUV.SetCoord( iCoord, coord );
    }
  }

  // [ ID1 ID2 ... IDn ]     ! Indices of key-points for a 2D pattern (only).
  if ( myIs2D )
  {
    if ( readLine( fields, lineBeg, clearFields ) == 0 ) {
      MESSAGE("Error: missing key-points");
      Clear();
      return setErrorCode( ERR_READ_NO_KEYPOINT );
    }
    set<int> idSet;
    for ( fIt = fields.begin(); fIt != fields.end(); fIt++ )
    {
      int pointIndex = getInt( *fIt );
      if ( pointIndex >= nbPoints || pointIndex < 0 ) {
        MESSAGE("Error: invalid point index " << pointIndex );
        Clear();
        return setErrorCode( ERR_READ_BAD_INDEX );
      }
      if ( idSet.insert( pointIndex ).second ) // unique?
        myKeyPointIDs.push_back( pointIndex );
    }
  }

  // ID1 ID2 ... IDn         ! 2-4 or 4-8 integers - nodal connectivity of a 2D or 3D element.

  while ( readLine( fields, lineBeg, clearFields ))
  {
    myElemPointIDs.push_back( list< int >() );
    list< int >& elemPoints = myElemPointIDs.back();
    for ( fIt = fields.begin(); fIt != fields.end(); fIt++ )
    {
      int pointIndex = getInt( *fIt );
      if ( pointIndex >= nbPoints || pointIndex < 0 ) {
        MESSAGE("Error: invalid point index " << pointIndex );
        Clear();
        return setErrorCode( ERR_READ_BAD_INDEX );
      }
      elemPoints.push_back( pointIndex );
    }
    // check the nb of nodes in element
    bool Ok = true;
    switch ( elemPoints.size() ) {
    case 3: if ( !myIs2D ) Ok = false; break;
    case 4: break;
    case 5:
    case 6:
    case 8: if ( myIs2D ) Ok = false; break;
    default: Ok = false;
    }
    if ( !Ok ) {
      MESSAGE("Error: wrong nb of nodes in element " << elemPoints.size() );
      Clear();
      return setErrorCode( ERR_READ_ELEM_POINTS );
    }
  }
  if ( myElemPointIDs.empty() ) {
    MESSAGE("Error: no elements");
    Clear();
    return setErrorCode( ERR_READ_NO_ELEMS );
  }

  findBoundaryPoints(); // sort key-points

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : Save
//purpose  : Save the loaded pattern into the file <theFileName>
//=======================================================================

bool SMESH_Pattern::Save (ostream& theFile)
{
  MESSAGE(" ::Save(file) " );
  if ( !IsLoaded() ) {
    MESSAGE(" Pattern not loaded ");
    return setErrorCode( ERR_SAVE_NOT_LOADED );
  }

  theFile << "!!! SALOME Mesh Pattern file" << endl;
  theFile << "!!!" << endl;
  theFile << "!!! Nb of points:" << endl;
  theFile << myPoints.size() << endl;

  // point coordinates
  const int width = 8;
//  theFile.width( 8 );
//  theFile.setf(ios::fixed);// use 123.45 floating notation
//  theFile.setf(ios::right);
//  theFile.flags( theFile.flags() & ~ios::showpoint); // do not show trailing zeros
//   theFile.setf(ios::showpoint); // do not show trailing zeros
  vector< TPoint >::const_iterator pVecIt = myPoints.begin();
  for ( int i = 0; pVecIt != myPoints.end(); pVecIt++, i++ ) {
    const gp_XYZ & xyz = (*pVecIt).myInitXYZ;
    theFile << " " << setw( width ) << xyz.X() << " " << setw( width ) << xyz.Y();
    if ( !myIs2D ) theFile  << " " << setw( width ) << xyz.Z();
    theFile  << "  !- " << i << endl; // point id to ease reading by a human being
  }
  // key-points
  if ( myIs2D ) {
    theFile << "!!! Indices of " << myKeyPointIDs.size() << " key-points:" << endl;
    list< int >::const_iterator kpIt = myKeyPointIDs.begin();
    for ( ; kpIt != myKeyPointIDs.end(); kpIt++ )
      theFile << " " << *kpIt;
    if ( !myKeyPointIDs.empty() )
      theFile << endl;
  }
  // elements
  theFile << "!!! Indices of points of " << myElemPointIDs.size() << " elements:" << endl;
  list<list< int > >::const_iterator epIt = myElemPointIDs.begin();
  for ( ; epIt != myElemPointIDs.end(); epIt++ )
  {
    const list< int > & elemPoints = *epIt;
    list< int >::const_iterator iIt = elemPoints.begin();
    for ( ; iIt != elemPoints.end(); iIt++ )
      theFile << " " << *iIt;
    theFile << endl;
  }

  theFile << endl;
  
  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : sortBySize
//purpose  : sort theListOfList by size
//=======================================================================

template<typename T> struct TSizeCmp {
  bool operator ()( const list < T > & l1, const list < T > & l2 )
    const { return l1.size() < l2.size(); }
};

template<typename T> void sortBySize( list< list < T > > & theListOfList )
{
  if ( theListOfList.size() > 2 ) {
    TSizeCmp< T > SizeCmp;
    theListOfList.sort( SizeCmp );
  }
}

//=======================================================================
//function : getOrderedEdges
//purpose  : return nb wires and a list of oredered edges
//=======================================================================

static int getOrderedEdges (const TopoDS_Face&   theFace,
                            const TopoDS_Vertex& theFirstVertex,
                            list< TopoDS_Edge >& theEdges,
                            list< int >  &       theNbVertexInWires)
{
  // put wires in a list, so that an outer wire comes first
  list<TopoDS_Wire> aWireList;
  TopoDS_Wire anOuterWire = BRepTools::OuterWire( theFace );
  aWireList.push_back( anOuterWire );
  for ( TopoDS_Iterator wIt (theFace); wIt.More(); wIt.Next() )
    if ( !anOuterWire.IsSame( wIt.Value() ))
      aWireList.push_back( TopoDS::Wire( wIt.Value() ));

  // loop on edges of wires
  theNbVertexInWires.clear();
  list<TopoDS_Wire>::iterator wlIt = aWireList.begin();
  for ( ; wlIt != aWireList.end(); wlIt++ )
  {
    int iE;
    BRepTools_WireExplorer wExp( *wlIt, theFace );
    for ( iE = 0; wExp.More(); wExp.Next(), iE++ )
    {
      TopoDS_Edge edge = wExp.Current();
      edge = TopoDS::Edge( edge.Oriented( wExp.Orientation() ));
      theEdges.push_back( edge );
    }
    theNbVertexInWires.push_back( iE );
    iE = 0;
    if ( wlIt == aWireList.begin() && theEdges.size() > 1 ) { // the outer wire
      // orient closed edges
      list< TopoDS_Edge >::iterator eIt, eIt2;
      for ( eIt = theEdges.begin(); eIt != theEdges.end(); eIt++ )
      {
        TopoDS_Edge& edge = *eIt;
        if ( TopExp::FirstVertex( edge ).IsSame( TopExp::LastVertex( edge ) ))
        {
          eIt2 = eIt;
          bool isNext = ( eIt2 == theEdges.begin() );
          TopoDS_Edge edge2 = isNext ? *(++eIt2) : *(--eIt2);
          double f1,l1,f2,l2;
          Handle(Geom2d_Curve) c1 = BRep_Tool::CurveOnSurface( edge, theFace, f1,l1 );
          Handle(Geom2d_Curve) c2 = BRep_Tool::CurveOnSurface( edge2, theFace, f2,l2 );
          gp_Pnt2d pf = c1->Value( edge.Orientation() == TopAbs_FORWARD ? f1 : l1 );
          gp_Pnt2d pl = c1->Value( edge.Orientation() == TopAbs_FORWARD ? l1 : f1 );
          bool isFirst = ( edge2.Orientation() == TopAbs_FORWARD ? isNext : !isNext );
          gp_Pnt2d p2 = c2->Value( isFirst ? f2 : l2 );
          isFirst = ( p2.SquareDistance( pf ) < p2.SquareDistance( pl ));
          if ( isNext ? isFirst : !isFirst )
            edge.Reverse();
        }
      }
      // rotate theEdges until it begins from theFirstVertex
      if ( ! theFirstVertex.IsNull() )
        while ( !theFirstVertex.IsSame( TopExp::FirstVertex( theEdges.front(), true )))
        {
          theEdges.splice(theEdges.end(), theEdges,
                          theEdges.begin(), ++ theEdges.begin());
          if ( iE++ > theNbVertexInWires.back() ) 
            break; // break infinite loop
        }
    }
  }

  return aWireList.size();
}

//=======================================================================
//function : project
//purpose  : 
//=======================================================================

static gp_XY project (const SMDS_MeshNode* theNode,
                      Extrema_GenExtPS &   theProjectorPS)
{
  gp_Pnt P( theNode->X(), theNode->Y(), theNode->Z() );
  theProjectorPS.Perform( P );
  if ( !theProjectorPS.IsDone() ) {
    MESSAGE( "SMESH_Pattern: point projection FAILED");
    return gp_XY(0.,0.);
  }
  double u, v, minVal = DBL_MAX;
  for ( int i = theProjectorPS.NbExt(); i > 0; i-- )
    if ( theProjectorPS.Value( i ) < minVal ) {
      minVal = theProjectorPS.Value( i );
      theProjectorPS.Point( i ).Parameter( u, v );
    }
  return gp_XY( u, v );
}

//=======================================================================
//function : isMeshBoundToShape
//purpose  : return true if all 2d elements are bound to shape
//=======================================================================

static bool isMeshBoundToShape(SMESH_Mesh* theMesh)
{
  // check faces binding
  SMESHDS_Mesh * aMeshDS = theMesh->GetMeshDS();
  SMESHDS_SubMesh * aMainSubMesh = aMeshDS->MeshElements( aMeshDS->ShapeToMesh() );
  if ( aMeshDS->NbFaces() != aMainSubMesh->NbElements() )
    return false;

  // check face nodes binding
  SMDS_FaceIteratorPtr fIt = aMeshDS->facesIterator();
  while ( fIt->more() )
  {
    SMDS_ElemIteratorPtr nIt = fIt->next()->nodesIterator();
    while ( nIt->more() )
    {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nIt->next() );
      SMDS_PositionPtr pos = node->GetPosition();
      if ( !pos || !pos->GetShapeId() )
        return false;
    }
  }
  return true;
}

//=======================================================================
//function : Load
//purpose  : Create a pattern from the mesh built on <theFace>.
//           <theProject>==true makes override nodes positions
//           on <theFace> computed by mesher
//=======================================================================

bool SMESH_Pattern::Load (SMESH_Mesh*        theMesh,
                          const TopoDS_Face& theFace,
                          bool               theProject)
{
  MESSAGE(" ::Load(face) " );
  Clear();
  myIs2D = true;

  SMESHDS_Mesh * aMeshDS = theMesh->GetMeshDS();
  SMESHDS_SubMesh * fSubMesh = aMeshDS->MeshElements( theFace );

  int nbNodes = ( !fSubMesh ? 0 : fSubMesh->NbNodes() );
  int nbElems = ( !fSubMesh ? 0 : fSubMesh->NbElements() );
  if ( nbElems == 0 && aMeshDS->NbFaces() == 0 )
  {
    MESSAGE( "No elements bound to the face");
    return setErrorCode( ERR_LOAD_EMPTY_SUBMESH );
  }

  TopoDS_Face face = TopoDS::Face( theFace.Oriented( TopAbs_FORWARD ));

  // check that face is not closed
  TopoDS_Vertex bidon;
  list<TopoDS_Edge> eList;
  getOrderedEdges( face, bidon, eList, myNbKeyPntInBoundary );
  list<TopoDS_Edge>::iterator elIt = eList.begin();
  for ( ; elIt != eList.end() ; elIt++ )
    if ( BRep_Tool::IsClosed( *elIt , face ))
      return setErrorCode( ERR_LOADF_CLOSED_FACE );
  

  Extrema_GenExtPS projector;
  GeomAdaptor_Surface aSurface( BRep_Tool::Surface( face ));
  if ( theProject || nbElems == 0 )
    projector.Initialize( aSurface, 20,20, 1e-5,1e-5 );

  int iPoint = 0;
  TNodePointIDMap nodePointIDMap;

  if ( nbElems == 0 || (theProject &&
                        theMesh->IsMainShape( face ) &&
                        !isMeshBoundToShape( theMesh )))
  {
    MESSAGE("Project the whole mesh");
    // ---------------------------------------------------------------
    // The case where the whole mesh is projected to theFace
    // ---------------------------------------------------------------

    // put nodes of all faces in the nodePointIDMap and fill myElemPointIDs
    SMDS_FaceIteratorPtr fIt = aMeshDS->facesIterator();
    while ( fIt->more() )
    {
      myElemPointIDs.push_back( list< int >() );
      list< int >& elemPoints = myElemPointIDs.back();
      SMDS_ElemIteratorPtr nIt = fIt->next()->nodesIterator();
      while ( nIt->more() )
      {
        const SMDS_MeshElement* node = nIt->next();
        TNodePointIDMap::iterator nIdIt = nodePointIDMap.find( node );
        if ( nIdIt == nodePointIDMap.end() )
        {
          elemPoints.push_back( iPoint );
          nodePointIDMap.insert( make_pair( node, iPoint++ ));
        }
        else
          elemPoints.push_back( (*nIdIt).second );
      }
    }
    myPoints.resize( iPoint );

    // project all nodes of 2d elements to theFace
    TNodePointIDMap::iterator nIdIt = nodePointIDMap.begin();
    for ( ; nIdIt != nodePointIDMap.end(); nIdIt++ )
    {
      const SMDS_MeshNode* node = 
        static_cast<const SMDS_MeshNode*>( (*nIdIt).first );
      TPoint * p = & myPoints[ (*nIdIt).second ];
      p->myInitUV = project( node, projector );
      p->myInitXYZ.SetCoord( p->myInitUV.X(), p->myInitUV.Y(), 0 );
    }
    // find key-points: the points most close to UV of vertices
    TopExp_Explorer vExp( face, TopAbs_VERTEX );
    set<int> foundIndices;
    for ( ; vExp.More(); vExp.Next() ) {
      const TopoDS_Vertex v = TopoDS::Vertex( vExp.Current() );
      gp_Pnt2d uv = BRep_Tool::Parameters( v, face );
      double minDist = DBL_MAX;
      int index;
      vector< TPoint >::const_iterator pVecIt = myPoints.begin();
      for ( iPoint = 0; pVecIt != myPoints.end(); pVecIt++, iPoint++ ) {
        double dist = uv.SquareDistance( (*pVecIt).myInitUV );
        if ( dist < minDist ) {
          minDist = dist;
          index = iPoint;
        }
      }
      if ( foundIndices.insert( index ).second ) // unique?
        myKeyPointIDs.push_back( index );
    }
    myIsBoundaryPointsFound = false;

  }
  else
  {
    // ---------------------------------------------------------------------
    // The case where a pattern is being made from the mesh built by mesher
    // ---------------------------------------------------------------------

    // Load shapes in the consequent order and count nb of points

    // vertices
    for ( elIt = eList.begin(); elIt != eList.end(); elIt++ ) {
      myShapeIDMap.Add( TopExp::FirstVertex( *elIt, true ));
      SMESHDS_SubMesh * eSubMesh = aMeshDS->MeshElements( *elIt );
      if ( eSubMesh )
        nbNodes += eSubMesh->NbNodes() + 1;
    }
    // edges
    for ( elIt = eList.begin(); elIt != eList.end(); elIt++ )
      myShapeIDMap.Add( *elIt );
    // the face
    myShapeIDMap.Add( face );

    myPoints.resize( nbNodes );

    // Load U of points on edges

    for ( elIt = eList.begin(); elIt != eList.end(); elIt++ )
    {
      TopoDS_Edge & edge = *elIt;
      list< TPoint* > & ePoints = getShapePoints( edge );
      double f, l;
      Handle(Geom2d_Curve) C2d;
      if ( !theProject )
        C2d = BRep_Tool::CurveOnSurface( edge, face, f, l );
      bool isForward = ( edge.Orientation() == TopAbs_FORWARD );

      // the forward key-point
      TopoDS_Shape v = TopExp::FirstVertex( edge, true );
      list< TPoint* > & vPoint = getShapePoints( v );
      if ( vPoint.empty() )
      {
        SMESHDS_SubMesh * vSubMesh = aMeshDS->MeshElements( v );
        if ( vSubMesh && vSubMesh->NbNodes() ) {
          myKeyPointIDs.push_back( iPoint );
          SMDS_NodeIteratorPtr nIt = vSubMesh->GetNodes();
          const SMDS_MeshNode* node = nIt->next();
          nodePointIDMap.insert( make_pair( node, iPoint ));

          TPoint* keyPoint = &myPoints[ iPoint++ ];
          vPoint.push_back( keyPoint );
          if ( theProject )
            keyPoint->myInitUV = project( node, projector );
          else
            keyPoint->myInitUV = C2d->Value( isForward ? f : l ).XY();
          keyPoint->myInitXYZ.SetCoord (keyPoint->myInitUV.X(), keyPoint->myInitUV.Y(), 0);
        }
      }
      if ( !vPoint.empty() )
        ePoints.push_back( vPoint.front() );

      // on-edge points
      SMESHDS_SubMesh * eSubMesh = aMeshDS->MeshElements( edge );
      if ( eSubMesh && eSubMesh->NbNodes() )
      {
        // loop on nodes of an edge: sort them by param on edge
        typedef map < double, const SMDS_MeshNode* > TParamNodeMap;
        TParamNodeMap paramNodeMap;
        SMDS_NodeIteratorPtr nIt = eSubMesh->GetNodes();
        while ( nIt->more() )
        {
          const SMDS_MeshNode* node = 
            static_cast<const SMDS_MeshNode*>( nIt->next() );
          const SMDS_EdgePosition* epos =
            static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
          double u = epos->GetUParameter();
          paramNodeMap.insert( TParamNodeMap::value_type( u, node ));
        }
        // put U in [0,1] so that the first key-point has U==0
        double du = l - f;
        TParamNodeMap::iterator         unIt  = paramNodeMap.begin();
        TParamNodeMap::reverse_iterator unRIt = paramNodeMap.rbegin();
        while ( unIt != paramNodeMap.end() )
        {
          TPoint* p = & myPoints[ iPoint ];
          ePoints.push_back( p );
          const SMDS_MeshNode* node = isForward ? (*unIt).second : (*unRIt).second;
          nodePointIDMap.insert ( make_pair( node, iPoint ));

          if ( theProject )
            p->myInitUV = project( node, projector );
          else {
            double u = isForward ? (*unIt).first : (*unRIt).first;
            p->myInitU = isForward ? (( u - f ) / du ) : ( 1.0 - ( u - f ) / du );
            p->myInitUV = C2d->Value( u ).XY();
          }
          p->myInitXYZ.SetCoord( p->myInitUV.X(), p->myInitUV.Y(), 0 );
          unIt++; unRIt++;
          iPoint++;
        }
      }
      // the reverse key-point
      v = TopExp::LastVertex( edge, true ).Reversed();
      list< TPoint* > & vPoint2 = getShapePoints( v );
      if ( vPoint2.empty() )
      {
        SMESHDS_SubMesh * vSubMesh = aMeshDS->MeshElements( v );
        if ( vSubMesh && vSubMesh->NbNodes() ) {
          myKeyPointIDs.push_back( iPoint );
          SMDS_NodeIteratorPtr nIt = vSubMesh->GetNodes();
          const SMDS_MeshNode* node = nIt->next();
          nodePointIDMap.insert( make_pair( node, iPoint ));

          TPoint* keyPoint = &myPoints[ iPoint++ ];
          vPoint2.push_back( keyPoint );
          if ( theProject )
            keyPoint->myInitUV = project( node, projector );
          else
            keyPoint->myInitUV = C2d->Value( isForward ? l : f ).XY();
          keyPoint->myInitXYZ.SetCoord( keyPoint->myInitUV.X(), keyPoint->myInitUV.Y(), 0 );
        }
      }
      if ( !vPoint2.empty() )
        ePoints.push_back( vPoint2.front() );

      // compute U of edge-points
      if ( theProject )
      {
        double totalDist = 0;
        list< TPoint* >::iterator pIt = ePoints.begin();
        TPoint* prevP = *pIt;
        prevP->myInitU = totalDist;
        for ( pIt++; pIt != ePoints.end(); pIt++ ) {
          TPoint* p = *pIt;
          totalDist += ( p->myInitUV - prevP->myInitUV ).Modulus();
          p->myInitU = totalDist;
          prevP = p;
        }
        if ( totalDist > DBL_MIN)
          for ( pIt = ePoints.begin(); pIt != ePoints.end(); pIt++ ) {
            TPoint* p = *pIt;
            p->myInitU /= totalDist;
          }
      }
    } // loop on edges of a wire

    // Load in-face points and elements

    if ( fSubMesh && fSubMesh->NbElements() )
    {
      list< TPoint* > & fPoints = getShapePoints( face );
      SMDS_NodeIteratorPtr nIt = fSubMesh->GetNodes();
      while ( nIt->more() )
      {
        const SMDS_MeshNode* node = 
          static_cast<const SMDS_MeshNode*>( nIt->next() );
        nodePointIDMap.insert( make_pair( node, iPoint ));
        TPoint* p = &myPoints[ iPoint++ ];
        fPoints.push_back( p );
        if ( theProject )
          p->myInitUV = project( node, projector );
        else {
          const SMDS_FacePosition* pos =
            static_cast<const SMDS_FacePosition*>(node->GetPosition().get());
          p->myInitUV.SetCoord( pos->GetUParameter(), pos->GetVParameter() );
        }
        p->myInitXYZ.SetCoord( p->myInitUV.X(), p->myInitUV.Y(), 0 );
      }
      // load elements
      SMDS_ElemIteratorPtr elemIt = fSubMesh->GetElements();
      while ( elemIt->more() ) {
        SMDS_ElemIteratorPtr nIt = elemIt->next()->nodesIterator();
        myElemPointIDs.push_back( list< int >() );
        list< int >& elemPoints = myElemPointIDs.back();
        while ( nIt->more() )
          elemPoints.push_back( nodePointIDMap[ nIt->next() ]);
      }
    }

    myIsBoundaryPointsFound = true;
  }

  // Assure that U range is proportional to V range

  Bnd_Box2d bndBox;
  vector< TPoint >::iterator pVecIt = myPoints.begin();
  for ( ; pVecIt != myPoints.end(); pVecIt++ )
    bndBox.Add( gp_Pnt2d( (*pVecIt).myInitUV ));
  double minU, minV, maxU, maxV;
  bndBox.Get( minU, minV, maxU, maxV );
  double dU = maxU - minU, dV = maxV - minV;
  if ( dU <= DBL_MIN || dV <= DBL_MIN ) {
    Clear();
    return setErrorCode( ERR_LOADF_NARROW_FACE );
  }
  double ratio = dU / dV, maxratio = 3, scale;
  int iCoord = 0;
  if ( ratio > maxratio ) {
    scale = ratio / maxratio;
    iCoord = 2;
  }
  else if ( ratio < 1./maxratio ) {
    scale = maxratio / ratio;
    iCoord = 1;
  }
  if ( iCoord ) {
    SCRUTE( scale );
    for ( pVecIt = myPoints.begin(); pVecIt != myPoints.end(); pVecIt++ ) {
      TPoint & p = *pVecIt;
      p.myInitUV.SetCoord( iCoord, p.myInitUV.Coord( iCoord ) * scale );
      p.myInitXYZ.SetCoord( p.myInitUV.X(), p.myInitUV.Y(), 0 );
    }
  }
  if ( myElemPointIDs.empty() ) {
    MESSAGE( "No elements bound to the face");
    return setErrorCode( ERR_LOAD_EMPTY_SUBMESH );
  }

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : computeUVOnEdge
//purpose  : compute coordinates of points on theEdge
//=======================================================================

void SMESH_Pattern::computeUVOnEdge (const TopoDS_Edge&      theEdge,
                                     const list< TPoint* > & ePoints )
{
  bool isForward = ( theEdge.Orientation() == TopAbs_FORWARD );
  double f, l;
  Handle(Geom2d_Curve) C2d =
    BRep_Tool::CurveOnSurface( theEdge, TopoDS::Face( myShape ), f, l );

  ePoints.back()->myInitU = 1.0;
  list< TPoint* >::const_iterator pIt = ePoints.begin();
  for ( pIt++; pIt != ePoints.end(); pIt++ )
  {
    TPoint* point = *pIt;
    // U
    double du = ( isForward ? point->myInitU : 1 - point->myInitU );
    point->myU = ( f * ( 1 - du ) + l * du );
    // UV
    point->myUV = C2d->Value( point->myU ).XY();
  }
}

//=======================================================================
//function : intersectIsolines
//purpose  : 
//=======================================================================

static bool intersectIsolines(const gp_XY& uv11, const gp_XY& uv12, const double r1,
                              const gp_XY& uv21, const gp_XY& uv22, const double r2,
                              gp_XY& resUV,
                              bool& isDeformed)
{
  gp_XY loc1 = uv11 * ( 1 - r1 ) + uv12 * r1;
  gp_XY loc2 = uv21 * ( 1 - r2 ) + uv22 * r2;
  resUV = 0.5 * ( loc1 + loc2 );
  isDeformed = ( loc1 - loc2 ).SquareModulus() > 1e-8;
//   double len1 = ( uv11 - uv12 ).Modulus();
//   double len2 = ( uv21 - uv22 ).Modulus();
//   resUV = loc1 * len2 / ( len1 + len2 ) + loc2 * len1 / ( len1 + len2 );
//  return true;

  
//   gp_Lin2d line1( uv11, uv12 - uv11 );
//   gp_Lin2d line2( uv21, uv22 - uv21 );
//   double angle = Abs( line1.Angle( line2 ) );

//     IntAna2d_AnaIntersection inter;
//     inter.Perform( line1.Normal( loc1 ), line2.Normal( loc2 ) );
//     if ( inter.IsDone() && inter.NbPoints() == 1 )
//     {
//       gp_Pnt2d interUV = inter.Point(1).Value();
//       resUV += interUV.XY();
//   inter.Perform( line1, line2 );
//   interUV = inter.Point(1).Value();
//   resUV += interUV.XY();
  
//   resUV /= 2.;
//     }
  return true;
}

//=======================================================================
//function : compUVByIsoIntersection
//purpose  : 
//=======================================================================

bool SMESH_Pattern::compUVByIsoIntersection (const list< list< TPoint* > >& theBndPoints,
                                             const gp_XY&                   theInitUV,
                                             gp_XY&                         theUV,
                                             bool &                         theIsDeformed )
{
  // compute UV by intersection of 2 iso lines
  //gp_Lin2d isoLine[2];
  gp_XY uv1[2], uv2[2];
  double ratio[2];
  const double zero = DBL_MIN;
  for ( int iIso = 0; iIso < 2; iIso++ )
  {
    // to build an iso line:
    // find 2 pairs of consequent edge-points such that the range of their
    // initial parameters encloses the in-face point initial parameter
    gp_XY UV[2], initUV[2];
    int nbUV = 0, iCoord = iIso + 1;
    double initParam = theInitUV.Coord( iCoord );

    list< list< TPoint* > >::const_iterator bndIt = theBndPoints.begin();
    for ( ; bndIt != theBndPoints.end(); bndIt++ )
    {
      const list< TPoint* > & bndPoints = * bndIt;
      TPoint* prevP = bndPoints.back(); // this is the first point
      list< TPoint* >::const_iterator pIt = bndPoints.begin();
      bool coincPrev = false; 
      // loop on the edge-points
      for ( ; pIt != bndPoints.end(); pIt++ )
      {
        double paramDiff     = initParam - (*pIt)->myInitUV.Coord( iCoord );
        double prevParamDiff = initParam - prevP->myInitUV.Coord( iCoord );
        double sumOfDiff = Abs(prevParamDiff) + Abs(paramDiff);
        if (!coincPrev && // ignore if initParam coincides with prev point param
            sumOfDiff > zero && // ignore if both points coincide with initParam
            prevParamDiff * paramDiff <= zero )
        {
          // find UV in parametric space of theFace
          double r = Abs(prevParamDiff) / sumOfDiff;
          gp_XY uvInit = (*pIt)->myInitUV * r + prevP->myInitUV * ( 1 - r );
          int i = nbUV++;
          if ( i >= 2 ) {
            // throw away uv most distant from <theInitUV>
            gp_XY vec0 = initUV[0] - theInitUV;
            gp_XY vec1 = initUV[1] - theInitUV;
            gp_XY vec  = uvInit    - theInitUV;
            bool isBetween = ( vec0 * vec1 < 0 ); // is theInitUV between initUV[0] and initUV[1]
            double dist0 = vec0.SquareModulus();
            double dist1 = vec1.SquareModulus();
            double dist  = vec .SquareModulus();
            if ( !isBetween || dist < dist0 || dist < dist1 ) {
              i = ( dist0 < dist1 ? 1 : 0 );
              if ( isBetween && vec.Dot( i ? vec1 : vec0 ) < 0 )
                i = 3; // theInitUV must remain between
            }
          }
          if ( i < 2 ) {
            initUV[ i ] = uvInit;
            UV[ i ]     = (*pIt)->myUV * r + prevP->myUV * ( 1 - r );
          }
          coincPrev = ( Abs(paramDiff) <= zero );
        }
        else
          coincPrev = false;
        prevP = *pIt;
      }
    }
    if ( nbUV < 2 || (UV[0]-UV[1]).SquareModulus() <= DBL_MIN*DBL_MIN ) {
      MESSAGE(" consequent edge-points not found, nb UV found: " << nbUV <<
              ", for point: " << theInitUV.X() <<" " << theInitUV.Y() );
      return setErrorCode( ERR_APPLF_BAD_TOPOLOGY );
    }
    // an iso line should be normal to UV[0] - UV[1] direction
    // and be located at the same relative distance as from initial ends
    //gp_Lin2d iso( UV[0], UV[0] - UV[1] );
    double r =
      (initUV[0]-theInitUV).Modulus() / (initUV[0]-initUV[1]).Modulus();
    //gp_Pnt2d isoLoc = UV[0] * ( 1 - r ) + UV[1] * r;
    //isoLine[ iIso ] = iso.Normal( isoLoc );
    uv1[ iIso ] = UV[0];
    uv2[ iIso ] = UV[1];
    ratio[ iIso ] = r;
  }
  if ( !intersectIsolines( uv1[0], uv2[0], ratio[0],
                          uv1[1], uv2[1], ratio[1], theUV, theIsDeformed )) {
    MESSAGE(" Cant intersect isolines for a point "<<theInitUV.X()<<", "<<theInitUV.Y());
    return setErrorCode( ERR_APPLF_BAD_TOPOLOGY );
  }

  return true;
}


// ==========================================================
// structure representing a node of a grid of iso-poly-lines
// ==========================================================

struct TIsoNode {
  bool   myIsMovable;
  gp_XY  myInitUV;
  gp_XY  myUV;
  double myRatio[2];
  gp_Dir2d  myDir[2]; // boundary tangent dir for boundary nodes, iso dir for internal ones
  TIsoNode* myNext[4]; // order: (iDir=0,isForward=0), (1,0), (0,1), (1,1)
  TIsoNode* myBndNodes[4];     // order: (iDir=0,i=0), (1,0), (0,1), (1,1)
  TIsoNode(double initU, double initV):
    myInitUV( initU, initV ), myUV( 1e100, 1e100 ), myIsMovable(true)
  { myNext[0] = myNext[1] = myNext[2] = myNext[3] = 0; }
  bool IsUVComputed() const
  { return myUV.X() != 1e100; }
  bool IsMovable() const
  { return myIsMovable && myNext[0] && myNext[1] && myNext[2] && myNext[3]; }
  void SetNotMovable()
  { myIsMovable = false; }
  void SetBoundaryNode(TIsoNode* node, int iDir, int i)
  { myBndNodes[ iDir + i * 2 ] = node; }
  TIsoNode* GetBoundaryNode(int iDir, int i)
  { return myBndNodes[ iDir + i * 2 ]; }
  void SetNext(TIsoNode* node, int iDir, int isForward)
  { myNext[ iDir + isForward  * 2 ] = node; }
  TIsoNode* GetNext(int iDir, int isForward)
  { return myNext[ iDir + isForward * 2 ]; }
};

//=======================================================================
//function : getNextNode
//purpose  : 
//=======================================================================

static inline TIsoNode* getNextNode(const TIsoNode* node, int dir )
{
  TIsoNode* n = node->myNext[ dir ];
  if ( n && !n->IsUVComputed()/* && node->IsMovable()*/ ) {
    n = 0;//node->myBndNodes[ dir ];
//     MESSAGE("getNextNode: use bnd for node "<<
//             node->myInitUV.X()<<" "<<node->myInitUV.Y());
  }
  return n;
}
//=======================================================================
//function : checkQuads
//purpose  : check if newUV destortes quadrangles around node,
//           and if ( crit == FIX_OLD ) fix newUV in this case
//=======================================================================

enum { CHECK_NEW_IN, CHECK_NEW_OK, FIX_OLD };

static bool checkQuads (const TIsoNode* node,
                        gp_XY&          newUV,
                        const bool      reversed,
                        const int       crit = FIX_OLD,
                        double          fixSize = 0.)
{
  gp_XY oldUV = node->myUV, oldUVFixed[4], oldUVImpr[4];
  int nbOldFix = 0, nbOldImpr = 0;
  double newBadRate = 0, oldBadRate = 0;
  bool newIsOk = true, newIsIn = true, oldIsIn = true, oldIsOk = true;
  int i, dir1 = 0, dir2 = 3;
  for ( ; dir1 < 4; dir1++, dir2++ )  // loop on 4 quadrangles around <node>
  {
    if ( dir2 > 3 ) dir2 = 0;
    TIsoNode* n[3];
    // walking counterclockwise around a quad,
    // nodes are in the order: node, n[0], n[1], n[2]
    n[0] = getNextNode( node, dir1 );
    n[2] = getNextNode( node, dir2 );
    if ( !n[0] || !n[2] ) continue;
    n[1] = getNextNode( n[0], dir2 );
    if ( !n[1] ) n[1] = getNextNode( n[2], dir1 );
    bool isTriangle = ( !n[1] );
    if ( reversed ) {
      TIsoNode* tmp = n[0]; n[0] = n[2]; n[2] = tmp;
    }
//     if ( fixSize != 0 ) {
// cout<<"NODE: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<" UV: "<<node->myUV.X()<<" "<<node->myUV.Y()<<endl;
// cout<<"\t0: "<<n[0]->myInitUV.X()<<" "<<n[0]->myInitUV.Y()<<" UV: "<<n[0]->myUV.X()<<" "<<n[0]->myUV.Y()<<endl;
// cout<<"\t1: "<<n[1]->myInitUV.X()<<" "<<n[1]->myInitUV.Y()<<" UV: "<<n[1]->myUV.X()<<" "<<n[1]->myUV.Y()<<endl;
// cout<<"\t2: "<<n[2]->myInitUV.X()<<" "<<n[2]->myInitUV.Y()<<" UV: "<<n[2]->myUV.X()<<" "<<n[2]->myUV.Y()<<endl;
// }
    // check if a quadrangle is degenerated
    if ( !isTriangle &&
        ((( n[0]->myUV - n[1]->myUV ).SquareModulus() <= DBL_MIN ) ||
         (( n[2]->myUV - n[1]->myUV ).SquareModulus() <= DBL_MIN )))
      isTriangle = true;
    if ( isTriangle &&
        ( n[0]->myUV - n[2]->myUV ).SquareModulus() <= DBL_MIN )
      continue;

    // find min size of the diagonal node-n[1]
    double minDiag = fixSize;
    if ( minDiag == 0. ) {
      double maxLen2 = ( node->myUV - n[0]->myUV ).SquareModulus();
      if ( !isTriangle ) {
        maxLen2 = Max( maxLen2, ( n[0]->myUV - n[1]->myUV ).SquareModulus() );
        maxLen2 = Max( maxLen2, ( n[1]->myUV - n[2]->myUV ).SquareModulus() );
      }
      maxLen2 = Max( maxLen2, ( n[2]->myUV - node->myUV ).SquareModulus() );
      minDiag = sqrt( maxLen2 ) * PI / 60.; // ~ maxLen * Sin( 3 deg )
    }

    // check if newUV is behind 3 dirs: n[0]-n[1], n[1]-n[2] and n[0]-n[2]
    // ( behind means "to the right of")
    // it is OK if
    // 1. newUV is not behind 01 and 12 dirs
    // 2. or newUV is not behind 02 dir and n[2] is convex
    bool newIn[3] = { true, true, true }, newOk[3] = { true, true, true };
    bool wasIn[3] = { true, true, true }, wasOk[3] = { true, true, true };
    gp_Vec2d moveVec[3], outVec[3];
    for ( i = isTriangle ? 2 : 0; i < 3; i++ )
    {
      bool isDiag = ( i == 2 );
      if ( isDiag && newOk[0] && newOk[1] && !isTriangle )
        break;
      gp_Vec2d sideDir;
      if ( isDiag )
        sideDir = gp_Vec2d( n[0]->myUV, n[2]->myUV );
      else
        sideDir = gp_Vec2d( n[i]->myUV, n[i+1]->myUV );

      gp_Vec2d outDir( sideDir.Y(), -sideDir.X() ); // to the right
      outDir.Normalize();
      gp_Vec2d newDir( n[i]->myUV, newUV );
      gp_Vec2d oldDir( n[i]->myUV, oldUV );
      outVec[i] = outDir;
      if ( newIsOk ) newOk[i] = ( outDir * newDir < -minDiag );
      if ( newIsIn ) newIn[i] = ( outDir * newDir < 0 );
      if ( crit == FIX_OLD ) {
        wasIn[i] = ( outDir * oldDir < 0 );
        wasOk[i] = ( outDir * oldDir < -minDiag );
        if ( !newOk[i] )
          newBadRate += outDir * newDir;
        if ( !wasOk[i] )
          oldBadRate += outDir * oldDir;
        // push node inside
        if ( !wasOk[i] ) {
          double oldDist = - outDir * oldDir;//, l2 = outDir * newDir;
          //               double r = ( l1 - minDiag ) / ( l1 + l2 );
          //               moveVec[i] = r * gp_Vec2d( node->myUV, newUV );
          moveVec[i] = ( oldDist - minDiag ) * outDir;
        }
      }
    }

    // check if n[2] is convex
    bool convex = true;
    if ( !isTriangle )
      convex = ( outVec[0] * gp_Vec2d( n[1]->myUV, n[2]->myUV ) < 0 );

    bool isNewOk = ( newOk[0] && newOk[1] ) || ( newOk[2] && convex );
    bool isNewIn = ( newIn[0] && newIn[1] ) || ( newIn[2] && convex );
    newIsOk = ( newIsOk && isNewOk );
    newIsIn = ( newIsIn && isNewIn );

    if ( crit != FIX_OLD ) {
      if ( crit == CHECK_NEW_OK && !newIsOk ) break;
      if ( crit == CHECK_NEW_IN && !newIsIn ) break;
      continue;
    }

    bool isOldIn = ( wasIn[0] && wasIn[1] ) || ( wasIn[2] && convex );
    bool isOldOk = ( wasOk[0] && wasOk[1] ) || ( wasOk[2] && convex );
    oldIsIn = ( oldIsIn && isOldIn );
    oldIsOk = ( oldIsOk && isOldIn );


    if ( !isOldIn ) { // node is outside a quadrangle
      // move newUV inside a quadrangle
//MESSAGE("Quad "<< dir1 << "  WAS IN " << wasIn[0]<<" "<<wasIn[1]<<" "<<wasIn[2]);
      // node and newUV are outside: push newUV inside
      gp_XY uv;
      if ( convex || isTriangle ) {
        uv = 0.5 * ( n[0]->myUV + n[2]->myUV ) - minDiag * outVec[2].XY();
      }
      else {
        gp_Vec2d out = outVec[0].Normalized() + outVec[1].Normalized();
        double outSize = out.Magnitude();
        if ( outSize > DBL_MIN )
          out /= outSize;
        else
          out.SetCoord( -outVec[1].Y(), outVec[1].X() );
        uv = n[1]->myUV - minDiag * out.XY();
      }
      oldUVFixed[ nbOldFix++ ] = uv;
      //node->myUV = newUV;
    }
    else if ( !isOldOk )  {
      // try to fix old UV: move node inside as less as possible
//MESSAGE("Quad "<< dir1 << "  old is BAD, try to fix old, minDiag: "<< minDiag);
      gp_XY uv1, uv2 = node->myUV;
      for ( i = isTriangle ? 2 : 0; i < 3; i++ ) // mark not computed vectors
        if ( wasOk[i] )
          moveVec[ i ].SetCoord( 1, 2e100); // not use this vector 
      while ( !isOldOk ) {
        // find the least moveVec
        int i, iMin = 4;
        double minMove2 = 1e100;
        for ( i = isTriangle ? 2 : 0; i < 3; i++ )
        {
          if ( moveVec[i].Coord(1) < 1e100 ) {
            double move2 = moveVec[i].SquareMagnitude();
            if ( move2 < minMove2 ) {
              minMove2 = move2;
              iMin = i;
            }
          }
        }
        if ( iMin == 4 ) {
          break;
        }
        // move node to newUV
        uv1 = node->myUV + moveVec[ iMin ].XY();
        uv2 += moveVec[ iMin ].XY();
        moveVec[ iMin ].SetCoord( 1, 2e100); // not use this vector more
        // check if uv1 is ok
        for ( i = isTriangle ? 2 : 0; i < 3; i++ )
          wasOk[i] = ( outVec[i] * gp_Vec2d( n[i]->myUV, uv1 ) < -minDiag );
        isOldOk = ( wasOk[0] && wasOk[1] ) || ( wasOk[2] && convex );
        if ( isOldOk )
          oldUVImpr[ nbOldImpr++ ] = uv1;
        else {
          // check if uv2 is ok
          for ( i = isTriangle ? 2 : 0; i < 3; i++ )
            wasOk[i] = ( outVec[i] * gp_Vec2d( n[i]->myUV, uv2 ) < -minDiag );
          isOldOk = ( wasOk[0] && wasOk[1] ) || ( wasOk[2] && convex );
          if ( isOldOk )
            oldUVImpr[ nbOldImpr++ ] = uv2;
        }
      }
    }

  } // loop on 4 quadrangles around <node>

  if ( crit == CHECK_NEW_OK  )
    return newIsOk;
  if ( crit == CHECK_NEW_IN  )
    return newIsIn;

  if ( newIsOk )
    return true;

  if ( oldIsOk )
    newUV = oldUV;
  else {
    if ( oldIsIn && nbOldImpr ) {
//       MESSAGE(" Try to improve UV, init: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<
//               " uv: "<<oldUV.X()<<" "<<oldUV.Y() );
      gp_XY uv = oldUVImpr[ 0 ];
      for ( int i = 1; i < nbOldImpr; i++ )
        uv += oldUVImpr[ i ];
      uv /= nbOldImpr;
      if ( checkQuads( node, uv, reversed, CHECK_NEW_OK )) {
        newUV = uv;
        return false;
      }
      else {
        //MESSAGE(" Cant improve UV, uv: "<<uv.X()<<" "<<uv.Y());
      }
    }
    if ( !oldIsIn && nbOldFix ) {
//       MESSAGE(" Try to fix UV, init: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<
//               " uv: "<<oldUV.X()<<" "<<oldUV.Y() );
      gp_XY uv = oldUVFixed[ 0 ];
      for ( int i = 1; i < nbOldFix; i++ )
        uv += oldUVFixed[ i ];
      uv /= nbOldFix;
      if ( checkQuads( node, uv, reversed, CHECK_NEW_IN )) {
        newUV = uv;
        return false;
      }
      else {
        //MESSAGE(" Cant fix UV, uv: "<<uv.X()<<" "<<uv.Y());
      }
    }
    if ( newIsIn && oldIsIn )
      newUV = ( newBadRate < oldBadRate ) ? newUV : oldUV;
    else if ( !newIsIn )
      newUV = oldUV;
  }

  return false;
}

//=======================================================================
//function : compUVByElasticIsolines
//purpose  : compute UV as nodes of iso-poly-lines consisting of
//           segments keeping relative size as in the pattern
//=======================================================================
//#define DEB_COMPUVBYELASTICISOLINES
bool SMESH_Pattern::
  compUVByElasticIsolines(const list< list< TPoint* > >& theBndPoints,
                          const list< TPoint* >&         thePntToCompute)
{
//cout << "============================== KEY POINTS =============================="<<endl;
//   list< int >::iterator kpIt = myKeyPointIDs.begin();
//   for ( ; kpIt != myKeyPointIDs.end(); kpIt++ ) {
//     TPoint& p = myPoints[ *kpIt ];
//     cout << "INIT: " << p.myInitUV.X() << " " << p.myInitUV.Y() <<
//       " UV: " << p.myUV.X() << " " << p.myUV.Y() << endl;
//  }
//cout << "=============================="<<endl;

  // Define parameters of iso-grid nodes in U and V dir

  set< double > paramSet[ 2 ];
  list< list< TPoint* > >::const_iterator pListIt;
  list< TPoint* >::const_iterator pIt;
  for ( pListIt = theBndPoints.begin(); pListIt != theBndPoints.end(); pListIt++ ) {
    const list< TPoint* > & pList = * pListIt;
    for ( pIt = pList.begin(); pIt != pList.end(); pIt++ ) {
      paramSet[0].insert( (*pIt)->myInitUV.X() );
      paramSet[1].insert( (*pIt)->myInitUV.Y() );
    }
  }
  for ( pIt = thePntToCompute.begin(); pIt != thePntToCompute.end(); pIt++ ) {
    paramSet[0].insert( (*pIt)->myInitUV.X() );
    paramSet[1].insert( (*pIt)->myInitUV.Y() );
  }
  // unite close parameters and split too long segments
  int iDir;
  double tol[ 2 ];
  for ( iDir = 0; iDir < 2; iDir++ )
  {
    set< double > & params = paramSet[ iDir ];
    double range = ( *params.rbegin() - *params.begin() );
    double toler = range / 1e6;
    tol[ iDir ] = toler;
//    double maxSegment = range / params.size() / 2.;
//
//     set< double >::iterator parIt = params.begin();
//     double prevPar = *parIt;
//     for ( parIt++; parIt != params.end(); parIt++ )
//     {
//       double segLen = (*parIt) - prevPar;
//       if ( segLen < toler )
//         ;//params.erase( prevPar ); // unite
//       else if ( segLen > maxSegment )
//         params.insert( prevPar + 0.5 * segLen ); // split
//       prevPar = (*parIt);
//     }
  }

  // Make nodes of a grid of iso-poly-lines

  list < TIsoNode > nodes;
  typedef list < TIsoNode *> TIsoLine;
  map < double, TIsoLine > isoMap[ 2 ];

  set< double > & params0 = paramSet[ 0 ];
  set< double >::iterator par0It = params0.begin();
  for ( ; par0It != params0.end(); par0It++ )
  {
    TIsoLine & isoLine0 = isoMap[0][ *par0It ]; // vertical isoline with const U
    set< double > & params1 = paramSet[ 1 ];
    set< double >::iterator par1It = params1.begin();
    for ( ; par1It != params1.end(); par1It++ )
    {
      nodes.push_back( TIsoNode( *par0It, *par1It ) );
      isoLine0.push_back( & nodes.back() );
      isoMap[1][ *par1It ].push_back( & nodes.back() );
    }
  }

  // Compute intersections of boundaries with iso-lines:
  // only boundary nodes will have computed UV so far

  Bnd_Box2d uvBnd;
  list< list< TPoint* > >::const_iterator bndIt = theBndPoints.begin();
  list< TIsoNode* > bndNodes; // nodes corresponding to outer theBndPoints
  for ( ; bndIt != theBndPoints.end(); bndIt++ )
  {
    const list< TPoint* > & bndPoints = * bndIt;
    TPoint* prevP = bndPoints.back(); // this is the first point
    list< TPoint* >::const_iterator pIt = bndPoints.begin();
    // loop on the edge-points
    for ( ; pIt != bndPoints.end(); pIt++ )
    {
      TPoint* point = *pIt;
      for ( iDir = 0; iDir < 2; iDir++ )
      {
        const int iCoord = iDir + 1;
        const int iOtherCoord = 2 - iDir;
        double par1 = prevP->myInitUV.Coord( iCoord );
        double par2 = point->myInitUV.Coord( iCoord );
        double parDif = par2 - par1;
        if ( Abs( parDif ) <= DBL_MIN )
          continue;
        // find iso-lines intersecting a bounadry
        double toler = tol[ 1 - iDir ];
        double minPar = Min ( par1, par2 );
        double maxPar = Max ( par1, par2 );
        map < double, TIsoLine >& isos = isoMap[ iDir ];
        map < double, TIsoLine >::iterator isoIt = isos.begin();
        for ( ; isoIt != isos.end(); isoIt++ )
        {
          double isoParam = (*isoIt).first;
          if ( isoParam < minPar || isoParam > maxPar )
            continue;
          double r = ( isoParam - par1 ) / parDif;
          gp_XY uv = ( 1 - r ) * prevP->myUV + r * point->myUV;
          gp_XY initUV = ( 1 - r ) * prevP->myInitUV + r * point->myInitUV;
          double otherPar = initUV.Coord( iOtherCoord ); // along isoline
          // find existing node with otherPar or insert a new one
          TIsoLine & isoLine = (*isoIt).second;
          double nodePar;
          TIsoLine::iterator nIt = isoLine.begin();
          for ( ; nIt != isoLine.end(); nIt++ ) {
            nodePar = (*nIt)->myInitUV.Coord( iOtherCoord );
            if ( nodePar >= otherPar )
              break;
          }
          TIsoNode * node;
          if ( Abs( nodePar - otherPar ) <= toler )
            node = ( nIt == isoLine.end() ) ? isoLine.back() : (*nIt);
          else {
            nodes.push_back( TIsoNode( initUV.X(), initUV.Y() ) );
            node = & nodes.back();
            isoLine.insert( nIt, node );
          }
          node->SetNotMovable();
          node->myUV = uv;
          uvBnd.Add( gp_Pnt2d( uv ));
//  cout << "bnd: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<" UV: "<<node->myUV.X()<<" "<<node->myUV.Y()<<endl;
          // tangent dir
          gp_XY tgt( point->myUV - prevP->myUV );
          if ( ::IsEqual( r, 1. ))
            node->myDir[ 0 ] = tgt;
          else if ( ::IsEqual( r, 0. ))
            node->myDir[ 1 ] = tgt;
          else
            node->myDir[ 1 ] = node->myDir[ 0 ] = tgt;
          // keep boundary nodes corresponding to boundary points
          if ( bndIt == theBndPoints.begin() && ::IsEqual( r, 1. ))
            if ( bndNodes.empty() || bndNodes.back() != node )
              bndNodes.push_back( node );
        } // loop on isolines
      } // loop on 2 directions
      prevP = point;
    } // loop on boundary points
  } // loop on boundaries

  // Define orientation

  // find the point with the least X
  double leastX = DBL_MAX;
  TIsoNode * leftNode;
  list < TIsoNode >::iterator nodeIt = nodes.begin();
  for ( ; nodeIt != nodes.end(); nodeIt++  ) {
    TIsoNode & node = *nodeIt;
    if ( node.IsUVComputed() && node.myUV.X() < leastX ) {
      leastX = node.myUV.X();
      leftNode = &node;
    }
// if ( node.IsUVComputed() ) {
// cout << "bndNode INIT: " << node.myInitUV.X()<<" "<<node.myInitUV.Y()<<" UV: "<<
//   node.myUV.X()<<" "<<node.myUV.Y()<<endl<<
//    " dir0: "<<node.myDir[0].X()<<" "<<node.myDir[0].Y() <<
//      " dir1: "<<node.myDir[1].X()<<" "<<node.myDir[1].Y() << endl;
// }
  }
  bool reversed = ( leftNode->myDir[0].Y() + leftNode->myDir[1].Y() > 0 );
  //SCRUTE( reversed );

  // Prepare internal nodes:
  // 1. connect nodes
  // 2. compute ratios
  // 3. find boundary nodes for each node
  // 4. remove nodes out of the boundary
  for ( iDir = 0; iDir < 2; iDir++ )
  {
    const int iCoord = 2 - iDir; // coord changing along an isoline
    map < double, TIsoLine >& isos = isoMap[ iDir ];
    map < double, TIsoLine >::iterator isoIt = isos.begin();
    for ( ; isoIt != isos.end(); isoIt++ )
    {
      TIsoLine & isoLine = (*isoIt).second;
      bool firstCompNodeFound = false;
      TIsoLine::iterator lastCompNodePos, nPrevIt, nIt, nNextIt, nIt2;
      nPrevIt = nIt = nNextIt = isoLine.begin();
      nIt++;
      nNextIt++; nNextIt++;
      while ( nIt != isoLine.end() )
      {
        // 1. connect prev - cur
        TIsoNode* node = *nIt, * prevNode = *nPrevIt;
        if ( !firstCompNodeFound && prevNode->IsUVComputed() ) {
          firstCompNodeFound = true;
          lastCompNodePos = nPrevIt;
        }
        if ( firstCompNodeFound ) {
          node->SetNext( prevNode, iDir, 0 );
          prevNode->SetNext( node, iDir, 1 );
        }
        // 2. compute ratio
        if ( nNextIt != isoLine.end() ) {
          double par1 = prevNode->myInitUV.Coord( iCoord );
          double par2 = node->myInitUV.Coord( iCoord );
          double par3 = (*nNextIt)->myInitUV.Coord( iCoord );
          node->myRatio[ iDir ] = ( par2 - par1 ) / ( par3 - par1 );
        }
        // 3. find boundary nodes
        if ( node->IsUVComputed() )
          lastCompNodePos = nIt;
        else if ( firstCompNodeFound && nNextIt != isoLine.end() ) {
          TIsoNode* bndNode1 = *lastCompNodePos, *bndNode2 = 0;
          for ( nIt2 = nNextIt; nIt2 != isoLine.end(); nIt2++ )
            if ( (*nIt2)->IsUVComputed() )
              break;
          if ( nIt2 != isoLine.end() ) {
            bndNode2 = *nIt2;
            node->SetBoundaryNode( bndNode1, iDir, 0 );
            node->SetBoundaryNode( bndNode2, iDir, 1 );
// cout << "--------------------------------------------------"<<endl;
//  cout << "bndNode1: " << bndNode1->myUV.X()<<" "<<bndNode1->myUV.Y()<<endl<<
//   " dir0: "<<bndNode1->myDir[0].X()<<" "<<bndNode1->myDir[0].Y() <<
//     " dir1: "<<bndNode1->myDir[1].X()<<" "<<bndNode1->myDir[1].Y() << endl;
//  cout << "bndNode2: " << bndNode2->myUV.X()<<" "<<bndNode2->myUV.Y()<<endl<<
//   " dir0: "<<bndNode2->myDir[0].X()<<" "<<bndNode2->myDir[0].Y() <<
//     " dir1: "<<bndNode2->myDir[1].X()<<" "<<bndNode2->myDir[1].Y() << endl;
          }
        }
        nIt++; nPrevIt++;
        if ( nNextIt != isoLine.end() ) nNextIt++;
        // 4. remove nodes out of the boundary
        if ( !firstCompNodeFound )
          isoLine.pop_front();
      } // loop on isoLine nodes

      // remove nodes after the boundary
//       for ( nIt = ++lastCompNodePos; nIt != isoLine.end(); nIt++ )
//         (*nIt)->SetNotMovable();
      isoLine.erase( ++lastCompNodePos, isoLine.end() );
    } // loop on isolines
  } // loop on 2 directions

  // Compute local isoline direction for internal nodes

  /*
  map < double, TIsoLine >& isos = isoMap[ 0 ]; // vertical isolines with const U
  map < double, TIsoLine >::iterator isoIt = isos.begin();
  for ( ; isoIt != isos.end(); isoIt++ )
  {
    TIsoLine & isoLine = (*isoIt).second;
    TIsoLine::iterator nIt = isoLine.begin();
    for ( ; nIt != isoLine.end(); nIt++ )
    {
      TIsoNode* node = *nIt;
      if ( node->IsUVComputed() || !node->IsMovable() )
        continue;
      gp_Vec2d aTgt[2], aNorm[2];
      double ratio[2];
      bool OK = true;
      for ( iDir = 0; iDir < 2; iDir++ )
      {
        TIsoNode* bndNode1 = node->GetBoundaryNode( iDir, 0 );
        TIsoNode* bndNode2 = node->GetBoundaryNode( iDir, 1 );
        if ( !bndNode1 || !bndNode2 ) {
          OK = false;
          break;
        }
        const int iCoord = 2 - iDir; // coord changing along an isoline
        double par1 = bndNode1->myInitUV.Coord( iCoord );
        double par2 = node->myInitUV.Coord( iCoord );
        double par3 = bndNode2->myInitUV.Coord( iCoord );
        ratio[ iDir ] = ( par2 - par1 ) / ( par3 - par1 );

        gp_Vec2d tgt1( bndNode1->myDir[0].XY() + bndNode1->myDir[1].XY() );
        gp_Vec2d tgt2( bndNode2->myDir[0].XY() + bndNode2->myDir[1].XY() );
        if ( bool( iDir ) == reversed ) tgt2.Reverse(); // along perpend. isoline
        else                            tgt1.Reverse();
//cout<<" tgt: " << tgt1.X()<<" "<<tgt1.Y()<<" | "<< tgt2.X()<<" "<<tgt2.Y()<<endl;

        if ( ratio[ iDir ] < 0.5 )
          aNorm[ iDir ] = gp_Vec2d( -tgt1.Y(), tgt1.X() ); // rotate tgt to the left
        else
          aNorm[ iDir ] = gp_Vec2d( -tgt2.Y(), tgt2.X() );
        if ( iDir == 1 )
          aNorm[ iDir ].Reverse();  // along iDir isoline

        double angle = tgt1.Angle( tgt2 ); //  [-PI, PI]
        // maybe angle is more than |PI|
        if ( Abs( angle ) > PI / 2. ) {
          // check direction of the last but one perpendicular isoline
          TIsoNode* prevNode = bndNode2->GetNext( iDir, 0 );
          bndNode1 = prevNode->GetBoundaryNode( 1 - iDir, 0 );
          bndNode2 = prevNode->GetBoundaryNode( 1 - iDir, 1 );
          gp_Vec2d isoDir( bndNode1->myUV, bndNode2->myUV );
          if ( isoDir * tgt2 < 0 )
            isoDir.Reverse();
          double angle2 = tgt1.Angle( isoDir );
          //cout << " isoDir: "<< isoDir.X() <<" "<<isoDir.Y() << " ANGLE: "<< angle << " "<<angle2<<endl;
          if (angle2 * angle < 0 && // check the sign of an angle close to PI
              Abs ( Abs ( angle ) - PI ) <= PI / 180. ) {
            //MESSAGE("REVERSE ANGLE");
            angle = -angle;
          }
          if ( Abs( angle2 ) > Abs( angle ) ||
              ( angle2 * angle < 0 && Abs( angle2 ) > Abs( angle - angle2 ))) {
            //MESSAGE("Add PI");
            // cout << "NODE: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<endl;
            // cout <<"ISO: " << isoParam << " " << (*iso2It).first << endl;
            // cout << "bndNode1: " << bndNode1->myUV.X()<<" "<<bndNode1->myUV.Y()<< endl;
            // cout << "bndNode2: " << bndNode2->myUV.X()<<" "<<bndNode2->myUV.Y()<<endl;
            // cout <<" tgt: " << tgt1.X()<<" "<<tgt1.Y()<<"  "<< tgt2.X()<<" "<<tgt2.Y()<<endl;
            angle += ( angle < 0 ) ? 2. * PI : -2. * PI;
          }
        }
        aTgt[ iDir ] = tgt1.Rotated( angle * ratio[ iDir ] ).XY();
      } // loop on 2 dir

      if ( OK ) {
        for ( iDir = 0; iDir < 2; iDir++ )
        {
          aTgt[iDir].Normalize();
          aNorm[1-iDir].Normalize();
          double r = Abs ( ratio[iDir] - 0.5 ) * 2.0; // [0,1] - distance from the middle
          r *= r;
          
          node->myDir[iDir] = //aTgt[iDir];
            aNorm[1-iDir] * r + aTgt[iDir] * ( 1. - r );
        }
// cout << "NODE: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<endl;
// cout <<" tgt: " << tgt1.X()<<" "<<tgt1.Y()<<" - "<< tgt2.X()<<" "<<tgt2.Y()<<endl;
//  cout << " isoDir: "<< node->myDir[0].X() <<" "<<node->myDir[0].Y()<<"  |  "
//    << node->myDir[1].X() <<" "<<node->myDir[1].Y()<<endl;
      }
    } // loop on iso nodes
  } // loop on isolines
*/
  // Find nodes to start computing UV from

  list< TIsoNode* > startNodes;
  list< TIsoNode* >::iterator nIt = bndNodes.end();
  TIsoNode* node = *(--nIt);
  TIsoNode* prevNode = *(--nIt);
  for ( nIt = bndNodes.begin(); nIt != bndNodes.end(); nIt++ )
  {
    TIsoNode* nextNode = *nIt;
    gp_Vec2d initTgt1( prevNode->myInitUV, node->myInitUV );
    gp_Vec2d initTgt2( node->myInitUV, nextNode->myInitUV );
    double initAngle = initTgt1.Angle( initTgt2 );
    double angle = node->myDir[0].Angle( node->myDir[1] );
    if ( reversed ) angle = -angle;
    if ( initAngle > angle && initAngle - angle > PI / 2.1 ) {
      // find a close internal node
      TIsoNode* nClose = 0;
      list< TIsoNode* > testNodes;
      testNodes.push_back( node );
      list< TIsoNode* >::iterator it = testNodes.begin();
      for ( ; !nClose && it != testNodes.end(); it++ )
      {
        for (int i = 0; i < 4; i++ )
        {
          nClose = (*it)->myNext[ i ];
          if ( nClose ) {
            if ( !nClose->IsUVComputed() )
              break;
            else {
              testNodes.push_back( nClose );
              nClose = 0;
            }
          }
        }
      }
      startNodes.push_back( nClose );
// cout << "START: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<" UV: "<<
//   node->myUV.X()<<" "<<node->myUV.Y()<<endl<<
//   "initAngle: " << initAngle << " angle: " << angle << endl;
// cout <<" init tgt: " << initTgt1.X()<<" "<<initTgt1.Y()<<" | "<< initTgt2.X()<<" "<<initTgt2.Y()<<endl;
// cout << " tgt: "<< node->myDir[ 0 ].X() <<" "<<node->myDir[ 0 ].Y()<<" | "<<
//    node->myDir[ 1 ].X() <<" "<<node->myDir[ 1 ].Y()<<endl;
// cout << "CLOSE: "<<nClose->myInitUV.X()<<" "<<nClose->myInitUV.Y()<<endl;
    }
    prevNode = node;
    node = nextNode;
  }

  // Compute starting UV of internal nodes

  list < TIsoNode* > internNodes;
  bool needIteration = true;
  if ( startNodes.empty() ) {
    MESSAGE( " Starting UV by compUVByIsoIntersection()");
    needIteration = false;
    map < double, TIsoLine >& isos = isoMap[ 0 ];
    map < double, TIsoLine >::iterator isoIt = isos.begin();
    for ( ; isoIt != isos.end(); isoIt++ )
    {
      TIsoLine & isoLine = (*isoIt).second;
      TIsoLine::iterator nIt = isoLine.begin();
      for ( ; !needIteration && nIt != isoLine.end(); nIt++ )
      {
        TIsoNode* node = *nIt;
        if ( !node->IsUVComputed() && node->IsMovable() ) {
          internNodes.push_back( node );
          //bool isDeformed;
          if ( !compUVByIsoIntersection(theBndPoints, node->myInitUV,
                                        node->myUV, needIteration ))
            node->myUV = node->myInitUV;
        }
      }
    }
    if ( needIteration )
      for ( nIt = bndNodes.begin(); nIt != bndNodes.end(); nIt++ )
      {
        TIsoNode* node = *nIt, *nClose = 0;
        list< TIsoNode* > testNodes;
        testNodes.push_back( node );
        list< TIsoNode* >::iterator it = testNodes.begin();
        for ( ; !nClose && it != testNodes.end(); it++ )
        {
          for (int i = 0; i < 4; i++ )
          {
            nClose = (*it)->myNext[ i ];
            if ( nClose ) {
              if ( !nClose->IsUVComputed() && nClose->IsMovable() )
                break;
              else {
                testNodes.push_back( nClose );
                nClose = 0;
              }
            }
          }
        }
        startNodes.push_back( nClose );
      }
  }

  double aMin[2], aMax[2], step[2];
  uvBnd.Get( aMin[0], aMin[1], aMax[0], aMax[1] );
  double minUvSize = Min ( aMax[0]-aMin[0], aMax[1]-aMin[1] );
  step[0] = minUvSize / paramSet[ 0 ].size() / 10;
  step[1] = minUvSize / paramSet[ 1 ].size() / 10;
//cout << "STEPS: " << step[0] << " " << step[1]<< endl;

  for ( nIt = startNodes.begin(); nIt != startNodes.end(); nIt++ )
  {
    TIsoNode* prevN[2], *node = *nIt;
    if ( node->IsUVComputed() || !node->IsMovable() )
      continue;
    gp_XY newUV( 0, 0 ), sumDir( 0, 0 );
    int nbComp = 0, nbPrev = 0;
    for ( iDir = 0; iDir < 2; iDir++ )
    {
      TIsoNode* prevNode1 = 0, *prevNode2 = 0;
      TIsoNode* n = node->GetNext( iDir, 0 );
      if ( n->IsUVComputed() )
        prevNode1 = n;
      else
        startNodes.push_back( n );
      n = node->GetNext( iDir, 1 );
      if ( n->IsUVComputed() )
        prevNode2 = n;
      else
        startNodes.push_back( n );
      if ( !prevNode1 ) {
        prevNode1 = prevNode2;
        prevNode2 = 0;
      }
      if ( prevNode1 ) nbPrev++;
      if ( prevNode2 ) nbPrev++;
      if ( prevNode1 ) {
        gp_XY dir;
          double prevPar = prevNode1->myInitUV.Coord( 2 - iDir );
          double par = node->myInitUV.Coord( 2 - iDir );
          bool isEnd = ( prevPar > par );
//          dir = node->myDir[ 1 - iDir ].XY() * ( isEnd ? -1. : 1. );
        //cout << "__________"<<endl<< "NODE: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<endl;
          TIsoNode* bndNode = node->GetBoundaryNode( iDir, isEnd );
          gp_XY tgt( bndNode->myDir[0].XY() + bndNode->myDir[1].XY() );
          dir.SetCoord( 1, tgt.Y() * ( reversed ? 1 : -1 ));
          dir.SetCoord( 2, tgt.X() * ( reversed ? -1 : 1 ));
        //cout << "bndNode UV: " << bndNode->myUV.X()<<" "<<bndNode->myUV.Y()<< endl;
          //  cout << " tgt: "<< bndNode->myDir[ 0 ].X() <<" "<<bndNode->myDir[ 0 ].Y()<<" | "<<
          //     bndNode->myDir[ 1 ].X() <<" "<<bndNode->myDir[ 1 ].Y()<<endl;
          //cout << "prevNode UV: " << prevNode1->myUV.X()<<" "<<prevNode1->myUV.Y()<<
            //" par: " << prevPar << endl;
          //           cout <<" tgt: " << tgt.X()<<" "<<tgt.Y()<<endl;
        //cout << " DIR: "<< dir.X() <<" "<<dir.Y()<<endl;
        if ( prevNode2 ) {
          //cout << "____2next______"<<endl<< "NODE: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<endl;
          gp_XY & uv1 = prevNode1->myUV;
          gp_XY & uv2 = prevNode2->myUV;
//           dir = ( uv2 - uv1 );
//           double len = dir.Modulus();
//           if ( len > DBL_MIN )
//             dir /= len * 0.5;
          double r = node->myRatio[ iDir ];
          newUV += uv1 * ( 1 - r ) + uv2 * r;
        }
        else {
          newUV += prevNode1->myUV + dir * step[ iDir ];
        }
        sumDir += dir;
        prevN[ iDir ] = prevNode1;
        nbComp++;
      }
    }
    newUV /= nbComp;
    node->myUV = newUV;
    //cout << "NODE: "<<node->myInitUV.X()<<" "<<node->myInitUV.Y()<<endl;

    // check if a quadrangle is not distorted
    if ( nbPrev > 1 ) {
      //int crit = ( nbPrev == 4 ) ? FIX_OLD : CHECK_NEW_IN;
      if ( !checkQuads( node, newUV, reversed, FIX_OLD, step[0] + step[1] )) {
      //cout <<" newUV: " << node->myUV.X() << " "<<node->myUV.Y() << " nbPrev: "<<nbPrev<< endl;
      //  cout << "_FIX_INIT_ fixedUV: " << newUV.X() << " "<<newUV.Y() << endl;
        node->myUV = newUV;
      }
    }
    internNodes.push_back( node );
  }
  
  // Move nodes

  static int maxNbIter = 100;
#ifdef DEB_COMPUVBYELASTICISOLINES
//   maxNbIter++;
  bool useNbMoveNode = 0;
  static int maxNbNodeMove = 100;
  maxNbNodeMove++;
  int nbNodeMove = 0;
  if ( !useNbMoveNode )
    maxNbIter = ( maxNbIter < 0 ) ? 100 : -1;
#endif    
  double maxMove;
  int nbIter = 0;
  do {
    if ( !needIteration) break;
#ifdef DEB_COMPUVBYELASTICISOLINES
    if ( nbIter >= maxNbIter ) break;
#endif
    maxMove = 0.0;
    list < TIsoNode* >::iterator nIt = internNodes.begin();
    for ( ; nIt != internNodes.end(); nIt++  ) {
#ifdef DEB_COMPUVBYELASTICISOLINES
      if (useNbMoveNode )
        cout << nbNodeMove <<" =================================================="<<endl;
#endif
      TIsoNode * node = *nIt;
      // make lines
      //gp_Lin2d line[2];
      gp_XY loc[2];
      for ( iDir = 0; iDir < 2; iDir++ )
      {
        gp_XY & uv1 = node->GetNext( iDir, 0 )->myUV;
        gp_XY & uv2 = node->GetNext( iDir, 1 )->myUV;
        double r = node->myRatio[ iDir ];
        loc[ iDir ] = uv1 * ( 1 - r ) + uv2 * r;
//         line[ iDir ].SetLocation( loc[ iDir ] );
//         line[ iDir ].SetDirection( node->myDir[ iDir ] );
      }
      // define ratio
      double locR[2] = { 0, 0 };
      for ( iDir = 0; iDir < 2; iDir++ )
      {
        const int iCoord = 2 - iDir; // coord changing along an isoline
        TIsoNode* bndNode1 = node->GetBoundaryNode( iDir, 0 );
        TIsoNode* bndNode2 = node->GetBoundaryNode( iDir, 1 );
        double par1 = bndNode1->myInitUV.Coord( iCoord );
        double par2 = node->myInitUV.Coord( iCoord );
        double par3 = bndNode2->myInitUV.Coord( iCoord );
        double r = ( par2 - par1 ) / ( par3 - par1 );
        r = Abs ( r - 0.5 ) * 2.0;  // [0,1] - distance from the middle
        locR[ iDir ] = ( 1 - r * r ) * 0.25;
      }
      //locR[0] = locR[1] = 0.25;
      // intersect the 2 lines and move a node
      //IntAna2d_AnaIntersection inter( line[0], line[1] );
      if ( /*inter.IsDone() && inter.NbPoints() ==*/ 1 )
      {
//         double intR = 1 - locR[0] - locR[1];
//         gp_XY newUV = inter.Point(1).Value().XY();
//         if ( !checkQuads( node, newUV, reversed, CHECK_NEW_IN ))
//           newUV = ( locR[0] * loc[0] + locR[1] * loc[1] ) / ( 1 - intR );
//         else
//           newUV = intR * newUV + locR[0] * loc[0] + locR[1] * loc[1];
        gp_XY newUV = 0.5 * ( loc[0] +  loc[1] );
        // avoid parallel isolines intersection
        checkQuads( node, newUV, reversed );

        maxMove = Max( maxMove, ( newUV - node->myUV ).SquareModulus());
        node->myUV = newUV;
      } // intersection found
#ifdef DEB_COMPUVBYELASTICISOLINES
      if (useNbMoveNode && ++nbNodeMove >= maxNbNodeMove ) break;
#endif
    } // loop on internal nodes
#ifdef DEB_COMPUVBYELASTICISOLINES
    if (useNbMoveNode && nbNodeMove >= maxNbNodeMove ) break;
#endif
  } while ( maxMove > 1e-8 && nbIter++ < maxNbIter );

  MESSAGE( "compUVByElasticIsolines(): Nb iterations " << nbIter << " dist: " << sqrt( maxMove ));

  if ( nbIter >= maxNbIter && sqrt(maxMove) > minUvSize * 0.05 ) {
    MESSAGE( "compUVByElasticIsolines() failed: "<<sqrt(maxMove)<<">"<<minUvSize * 0.05);
#ifndef DEB_COMPUVBYELASTICISOLINES
    return false;
#endif
  }

  // Set computed UV to points

  for ( pIt = thePntToCompute.begin(); pIt != thePntToCompute.end(); pIt++ ) {
    TPoint* point = *pIt;
    //gp_XY oldUV = point->myUV;
    double minDist = DBL_MAX;
    list < TIsoNode >::iterator nIt = nodes.begin();
    for ( ; nIt != nodes.end(); nIt++ ) {
      double dist = ( (*nIt).myInitUV - point->myInitUV ).SquareModulus();
      if ( dist < minDist ) {
        minDist = dist;
        point->myUV = (*nIt).myUV;
      }
    }
  }
      
    
  return true;
}


//=======================================================================
//function : setFirstEdge
//purpose  : choose the best first edge of theWire; return the summary distance
//           between point UV computed by isolines intersection and
//           eventual UV got from edge p-curves
//=======================================================================

//#define DBG_SETFIRSTEDGE
double SMESH_Pattern::setFirstEdge (list< TopoDS_Edge > & theWire, int theFirstEdgeID)
{
  int iE, nbEdges = theWire.size();
  if ( nbEdges == 1 )
    return 0;

  // Transform UVs computed by iso to fit bnd box of a wire

  // max nb of points on an edge
  int maxNbPnt = 0;
  int eID = theFirstEdgeID;
  for ( iE = 0; iE < nbEdges; iE++ )
    maxNbPnt = Max ( maxNbPnt, getShapePoints( eID++ ).size() );
  
  // compute bnd boxes
  TopoDS_Face face = TopoDS::Face( myShape );
  Bnd_Box2d bndBox, eBndBox;
  eID = theFirstEdgeID;
  list< TopoDS_Edge >::iterator eIt;
  list< TPoint* >::iterator pIt;
  for ( eIt = theWire.begin(); eIt != theWire.end(); eIt++ )
  {
    // UV by isos stored in TPoint.myXYZ
    list< TPoint* > & ePoints = getShapePoints( eID++ );
    for ( pIt = ePoints.begin(); pIt != ePoints.end(); pIt++ ) {
      TPoint* p = (*pIt);
      bndBox.Add( gp_Pnt2d( p->myXYZ.X(), p->myXYZ.Y() ));
    }
    // UV by an edge p-curve
    double f, l;
    Handle(Geom2d_Curve) C2d = BRep_Tool::CurveOnSurface( *eIt, face, f, l );
    double dU = ( l - f ) / ( maxNbPnt - 1 );
    for ( int i = 0; i < maxNbPnt; i++ )
      eBndBox.Add( C2d->Value( f + i * dU ));
  }

  // transform UVs by isos
  double minPar[2], maxPar[2], eMinPar[2], eMaxPar[2];
  bndBox.Get( minPar[0], minPar[1], maxPar[0], maxPar[1] );
  eBndBox.Get( eMinPar[0], eMinPar[1], eMaxPar[0], eMaxPar[1] );
#ifdef DBG_SETFIRSTEDGE
  cout << "EDGES: X: " << eMinPar[0] << " - " << eMaxPar[0] << " Y: "
    << eMinPar[1] << " - " << eMaxPar[1] << endl;
#endif
  for ( int iC = 1, i = 0; i < 2; iC++, i++ ) // loop on 2 coordinates
  {
    double dMin = eMinPar[i] - minPar[i];
    double dMax = eMaxPar[i] - maxPar[i];
    double dPar = maxPar[i] - minPar[i];
    eID = theFirstEdgeID;
    for ( iE = 0; iE < nbEdges; iE++ ) // loop on edges of a boundary
    {
      list< TPoint* > & ePoints = getShapePoints( eID++ );
      for ( pIt = ++ePoints.begin(); pIt != ePoints.end(); pIt++ ) // loop on edge points
      {
        double par = (*pIt)->myXYZ.Coord( iC );
        double r = ( par - minPar[i] ) / dPar;
        par += ( 1 - r ) * dMin + r * dMax;
        (*pIt)->myXYZ.SetCoord( iC, par );
      }
    }
  }

  TopoDS_Edge eBest;
  double minDist = DBL_MAX;
  for ( iE = 0 ; iE < nbEdges; iE++ )
  {
#ifdef DBG_SETFIRSTEDGE
    cout << " VARIANT " << iE << endl;
#endif
    // evaluate the distance between UV computed by the 2 methods:
    // by isos intersection ( myXYZ ) and by edge p-curves ( myUV )
    double dist = 0;
    int eID = theFirstEdgeID;
    for ( eIt = theWire.begin(); eIt != theWire.end(); eIt++ )
    {
      list< TPoint* > & ePoints = getShapePoints( eID++ );
      computeUVOnEdge( *eIt, ePoints );
      for ( pIt = ++ePoints.begin(); pIt != ePoints.end(); pIt++ ) {
        TPoint* p = (*pIt);
        dist += ( p->myUV - gp_XY( p->myXYZ.X(), p->myXYZ.Y() )).SquareModulus();
#ifdef DBG_SETFIRSTEDGE
        cout << " ISO : ( " << p->myXYZ.X() << ", "<< p->myXYZ.Y() << " ) PCURVE : ( " <<
          p->myUV.X() << ", " << p->myUV.Y() << ") " << endl;
#endif
      }
    }
#ifdef DBG_SETFIRSTEDGE
    cout << "dist -- " << dist << endl;
#endif
    if ( dist < minDist ) {
      minDist = dist;
      eBest = theWire.front();
    }
    // check variant with another first edge
    theWire.splice( theWire.begin(), theWire, --theWire.end(), theWire.end() );
  }
  // put the best first edge to the theWire front
  if ( eBest != theWire.front() ) {
    eIt = find ( theWire.begin(), theWire.end(), eBest );
    theWire.splice( theWire.begin(), theWire, eIt, theWire.end() );
  }

  return minDist;
}

//=======================================================================
//function : sortSameSizeWires
//purpose  : sort wires in theWireList from theFromWire until theToWire,
//           the wires are set in the order to correspond to the order
//           of boundaries; after sorting, edges in the wires are put
//           in a good order, point UVs on edges are computed and points
//           are appended to theEdgesPointsList
//=======================================================================

bool SMESH_Pattern::sortSameSizeWires (TListOfEdgesList &                theWireList,
                                       const TListOfEdgesList::iterator& theFromWire,
                                       const TListOfEdgesList::iterator& theToWire,
                                       const int                         theFirstEdgeID,
                                       list< list< TPoint* > >&          theEdgesPointsList )
{
  TopoDS_Face F = TopoDS::Face( myShape );
  int iW, nbWires = 0;
  TListOfEdgesList::iterator wlIt = theFromWire;
  while ( wlIt++ != theToWire )
    nbWires++;

  // Recompute key-point UVs by isolines intersection,
  // compute CG of key-points for each wire and bnd boxes of GCs

  bool aBool;
  gp_XY orig( gp::Origin2d().XY() );
  vector< gp_XY > vGcVec( nbWires, orig ), gcVec( nbWires, orig );
  Bnd_Box2d bndBox, vBndBox;
  int eID = theFirstEdgeID;
  list< TopoDS_Edge >::iterator eIt;
  for ( iW = 0, wlIt = theFromWire; wlIt != theToWire; wlIt++, iW++ )
  {
    list< TopoDS_Edge > & wire = *wlIt;
    for ( eIt = wire.begin(); eIt != wire.end(); eIt++ )
    {
      list< TPoint* > & ePoints = getShapePoints( eID++ );
      TPoint* p = ePoints.front();
      if ( !compUVByIsoIntersection( theEdgesPointsList, p->myInitUV, p->myUV, aBool )) {
        MESSAGE("cant sortSameSizeWires()");
        return false;
      }
      gcVec[iW] += p->myUV;
      bndBox.Add( gp_Pnt2d( p->myUV ));
      TopoDS_Vertex V = TopExp::FirstVertex( *eIt, true );
      gp_Pnt2d vXY = BRep_Tool::Parameters( V, F );
      vGcVec[iW] += vXY.XY();
      vBndBox.Add( vXY );
      // keep the computed UV to compare against by setFirstEdge()
      p->myXYZ.SetCoord( p->myUV.X(), p->myUV.Y(), 0. );
    }
    gcVec[iW] /= nbWires;
    vGcVec[iW] /= nbWires;
// cout << " Wire " << iW << " iso: " << gcVec[iW].X() << " " << gcVec[iW].Y() << endl <<
//   " \t vertex: " << vGcVec[iW].X() << " " << vGcVec[iW].Y() << endl;
  }

  // Transform GCs computed by isos to fit in bnd box of GCs by vertices

  double minPar[2], maxPar[2], vMinPar[2], vMaxPar[2];
  bndBox.Get( minPar[0], minPar[1], maxPar[0], maxPar[1] );
  vBndBox.Get( vMinPar[0], vMinPar[1], vMaxPar[0], vMaxPar[1] );
  for ( int iC = 1, i = 0; i < 2; iC++, i++ ) // loop on 2 coordinates
  {
    double dMin = vMinPar[i] - minPar[i];
    double dMax = vMaxPar[i] - maxPar[i];
    double dPar = maxPar[i] - minPar[i];
    if ( Abs( dPar ) <= DBL_MIN )
      continue;
    for ( iW = 0; iW < nbWires; iW++ ) { // loop on GCs of wires
      double par = gcVec[iW].Coord( iC );
      double r = ( par - minPar[i] ) / dPar;
      par += ( 1 - r ) * dMin + r * dMax;
      gcVec[iW].SetCoord( iC, par );
    }
  }

  // Define boundary - wire correspondence by GC closeness

  TListOfEdgesList tmpWList;
  tmpWList.splice( tmpWList.end(), theWireList, theFromWire, theToWire );
  typedef map< int, TListOfEdgesList::iterator > TIntWirePosMap;
  TIntWirePosMap bndIndWirePosMap;
  vector< bool > bndFound( nbWires, false );
  for ( iW = 0, wlIt = tmpWList.begin(); iW < nbWires; iW++, wlIt++ )
  {
// cout << " TRSF Wire " << iW << " iso: " << gcVec[iW].X() << " " << gcVec[iW].Y() << endl <<
//   " \t vertex: " << vGcVec[iW].X() << " " << vGcVec[iW].Y() << endl;
    double minDist = DBL_MAX;
    gp_XY & wGc = vGcVec[ iW ];
    int bIndex;
    for ( int iB = 0; iB < nbWires; iB++ ) {
      if ( bndFound[ iB ] ) continue;
      double dist = ( wGc - gcVec[ iB ] ).SquareModulus();
      if ( dist < minDist ) {
        minDist = dist;
        bIndex = iB;
      }
    }
    bndFound[ bIndex ] = true;
    bndIndWirePosMap.insert( TIntWirePosMap::value_type( bIndex, wlIt ));
  }

  // Treat each wire  

  TIntWirePosMap::iterator bIndWPosIt = bndIndWirePosMap.begin();
  eID = theFirstEdgeID;
  for ( ; bIndWPosIt != bndIndWirePosMap.end(); bIndWPosIt++ )
  {
    TListOfEdgesList::iterator wirePos = (*bIndWPosIt).second;
    list < TopoDS_Edge > & wire = ( *wirePos );

    // choose the best first edge of a wire
    setFirstEdge( wire, eID );
    
    // compute eventual UV and fill theEdgesPointsList
    theEdgesPointsList.push_back( list< TPoint* >() );
    list< TPoint* > & edgesPoints = theEdgesPointsList.back();
    for ( eIt = wire.begin(); eIt != wire.end(); eIt++ )
    {
      list< TPoint* > & ePoints = getShapePoints( eID++ );
      computeUVOnEdge( *eIt, ePoints );
      edgesPoints.insert( edgesPoints.end(), ePoints.begin(), (--ePoints.end()));
    }
    // put wire back to theWireList
    wlIt = wirePos++;
    theWireList.splice( theToWire, tmpWList, wlIt, wirePos );
  }

  return true;
}

//=======================================================================
//function : Apply
//purpose  : Compute  nodes coordinates applying
//           the loaded pattern to <theFace>. The first key-point
//           will be mapped into <theVertexOnKeyPoint1>
//=======================================================================

bool SMESH_Pattern::Apply (const TopoDS_Face&   theFace,
                           const TopoDS_Vertex& theVertexOnKeyPoint1,
                           const bool           theReverse)
{
  MESSAGE(" ::Apply(face) " );
  TopoDS_Face face  = theReverse ? TopoDS::Face( theFace.Reversed() ) : theFace;
  if ( !setShapeToMesh( face ))
    return false;

  // find points on edges, it fills myNbKeyPntInBoundary
  if ( !findBoundaryPoints() )
    return false;

  // Define the edges order so that the first edge starts at
  // theVertexOnKeyPoint1

  list< TopoDS_Edge > eList;
  list< int >         nbVertexInWires;
  int nbWires = getOrderedEdges( face, theVertexOnKeyPoint1, eList, nbVertexInWires);
  if ( !theVertexOnKeyPoint1.IsSame( TopExp::FirstVertex( eList.front(), true )))
  {
    MESSAGE( " theVertexOnKeyPoint1 not found in the outer wire ");
    return setErrorCode( ERR_APPLF_BAD_VERTEX );
  }
  // check nb wires and edges
  list< int > l1 = myNbKeyPntInBoundary, l2 = nbVertexInWires;
  l1.sort(); l2.sort();
  if ( l1 != l2 )
  {
    MESSAGE( "Wrong nb vertices in wires" );
    return setErrorCode( ERR_APPL_BAD_NB_VERTICES );
  }

  // here shapes get IDs, for the outer wire IDs are OK
  list<TopoDS_Edge>::iterator elIt = eList.begin();
  for ( ; elIt != eList.end(); elIt++ ) {
    myShapeIDMap.Add( TopExp::FirstVertex( *elIt, true ));
    if ( BRep_Tool::IsClosed( *elIt, theFace ) )
      myShapeIDMap.Add( TopExp::LastVertex( *elIt, true ));
  }
  int nbVertices = myShapeIDMap.Extent();

  for ( elIt = eList.begin(); elIt != eList.end(); elIt++ )
    myShapeIDMap.Add( *elIt );

  myShapeIDMap.Add( face );

  if ( myShapeIDToPointsMap.size() != myShapeIDMap.Extent()/* + nbSeamShapes*/ ) {
    MESSAGE( myShapeIDToPointsMap.size() <<" != " << myShapeIDMap.Extent());
    return setErrorCode( ERR_APPLF_INTERNAL_EEROR );
  }

  // points on edges to be used for UV computation of in-face points
  list< list< TPoint* > > edgesPointsList;
  edgesPointsList.push_back( list< TPoint* >() );
  list< TPoint* > * edgesPoints = & edgesPointsList.back();
  list< TPoint* >::iterator pIt;

  // compute UV of points on the outer wire
  int iE, nbEdgesInOuterWire = nbVertexInWires.front();
  for (iE = 0, elIt = eList.begin();
       iE < nbEdgesInOuterWire && elIt != eList.end();
       iE++, elIt++ )
  {
    list< TPoint* > & ePoints = getShapePoints( *elIt );
    // compute UV
    computeUVOnEdge( *elIt, ePoints );
    // collect on-edge points (excluding the last one)
    edgesPoints->insert( edgesPoints->end(), ePoints.begin(), --ePoints.end());
  }

  // If there are several wires, define the order of edges of inner wires:
  // compute UV of inner edge-points using 2 methods: the one for in-face points
  // and the one for on-edge points and then choose the best edge order
  // by the best correspondance of the 2 results
  if ( nbWires > 1 )
  {
    // compute UV of inner edge-points using the method for in-face points
    // and devide eList into a list of separate wires
    bool aBool;
    list< list< TopoDS_Edge > > wireList;
    list<TopoDS_Edge>::iterator eIt = elIt;
    list<int>::iterator nbEIt = nbVertexInWires.begin();
    for ( nbEIt++; nbEIt != nbVertexInWires.end(); nbEIt++ )
    {
      int nbEdges = *nbEIt;
      wireList.push_back( list< TopoDS_Edge >() );
      list< TopoDS_Edge > & wire = wireList.back();
      for ( iE = 0 ; iE < nbEdges; eIt++, iE++ )
      {
        list< TPoint* > & ePoints = getShapePoints( *eIt );
        pIt = ePoints.begin();
        for (  pIt++; pIt != ePoints.end(); pIt++ ) {
          TPoint* p = (*pIt);
          if ( !compUVByIsoIntersection( edgesPointsList, p->myInitUV, p->myUV, aBool )) {
            MESSAGE("cant Apply(face)");
            return false;
          }
          // keep the computed UV to compare against by setFirstEdge()
          p->myXYZ.SetCoord( p->myUV.X(), p->myUV.Y(), 0. );
        }
        wire.push_back( *eIt );
      }
    }
    // remove inner edges from eList
    eList.erase( elIt, eList.end() );

    // sort wireList by nb edges in a wire
    sortBySize< TopoDS_Edge > ( wireList );

    // an ID of the first edge of a boundary
    int id1 = nbVertices + nbEdgesInOuterWire + 1;
//     if ( nbSeamShapes > 0 )
//       id1 += 2; // 2 vertices more

    // find points - edge correspondence for wires of unique size,
    // edge order within a wire should be defined only

    list< list< TopoDS_Edge > >::iterator wlIt = wireList.begin();
    while ( wlIt != wireList.end() )
    {
      list< TopoDS_Edge >& wire = (*wlIt);
      int nbEdges = wire.size();
      wlIt++;
      if ( wlIt == wireList.end() || (*wlIt).size() != nbEdges ) // a unique size wire
      {
        // choose the best first edge of a wire
        setFirstEdge( wire, id1 );

        // compute eventual UV and collect on-edge points
        edgesPointsList.push_back( list< TPoint* >() );
        edgesPoints = & edgesPointsList.back();
        int eID = id1;
        for ( eIt = wire.begin(); eIt != wire.end(); eIt++ )
        {
          list< TPoint* > & ePoints = getShapePoints( eID++ );
          computeUVOnEdge( *eIt, ePoints );
          edgesPoints->insert( edgesPoints->end(), ePoints.begin(), (--ePoints.end()));
        }
      }
      id1 += nbEdges;
    }

    // find boundary - wire correspondence for several wires of same size
    
    id1 = nbVertices + nbEdgesInOuterWire + 1;
    wlIt = wireList.begin();
    while ( wlIt != wireList.end() )
    {
      int nbSameSize = 0, nbEdges = (*wlIt).size();
      list< list< TopoDS_Edge > >::iterator wlIt2 = wlIt;
      wlIt2++;
      while ( wlIt2 != wireList.end() && (*wlIt2).size() == nbEdges ) { // a same size wire
        nbSameSize++;
        wlIt2++;
      }
      if ( nbSameSize > 0 )
        if (!sortSameSizeWires(wireList, wlIt, wlIt2, id1, edgesPointsList))
          return false;
      wlIt = wlIt2;
      id1 += nbEdges * ( nbSameSize + 1 );
    }

    // add well-ordered edges to eList
    
    for ( wlIt = wireList.begin(); wlIt != wireList.end(); wlIt++ )
    {
      list< TopoDS_Edge >& wire = (*wlIt);
      eList.splice( eList.end(), wire, wire.begin(), wire.end() );
    }

    // re-fill myShapeIDMap - all shapes get good IDs

    myShapeIDMap.Clear();
    for ( elIt = eList.begin(); elIt != eList.end(); elIt++ )
      myShapeIDMap.Add( TopExp::FirstVertex( *elIt, true ));
    for ( elIt = eList.begin(); elIt != eList.end(); elIt++ )
      myShapeIDMap.Add( *elIt );
    myShapeIDMap.Add( face );
    
  } // there are inner wires

  // Compute XYZ of on-edge points

  TopLoc_Location loc;
  for ( iE = nbVertices + 1, elIt = eList.begin(); elIt != eList.end(); elIt++ )
  {
    double f,l;
    Handle(Geom_Curve) C3d = BRep_Tool::Curve( *elIt, loc, f, l );
    const gp_Trsf & aTrsf = loc.Transformation();
    list< TPoint* > & ePoints = getShapePoints( iE++ );
    pIt = ePoints.begin();
    for ( pIt++; pIt != ePoints.end(); pIt++ )
    {
      TPoint* point = *pIt;
      point->myXYZ = C3d->Value( point->myU );
      if ( !loc.IsIdentity() )
        aTrsf.Transforms( point->myXYZ.ChangeCoord() );
    }
  }

  // Compute UV and XYZ of in-face points

  // try to use a simple algo
  list< TPoint* > & fPoints = getShapePoints( face );
  bool isDeformed = false;
  for ( pIt = fPoints.begin(); !isDeformed && pIt != fPoints.end(); pIt++ )
    if ( !compUVByIsoIntersection( edgesPointsList, (*pIt)->myInitUV,
                                  (*pIt)->myUV, isDeformed )) {
      MESSAGE("cant Apply(face)");
      return false;
    }
  // try to use a complex algo if it is a difficult case
  if ( isDeformed && !compUVByElasticIsolines( edgesPointsList, fPoints ))
  {
    for ( ; pIt != fPoints.end(); pIt++ ) // continue with the simple algo
      if ( !compUVByIsoIntersection( edgesPointsList, (*pIt)->myInitUV,
                                    (*pIt)->myUV, isDeformed )) {
        MESSAGE("cant Apply(face)");
        return false;
      }
  }

  Handle(Geom_Surface) aSurface = BRep_Tool::Surface( face, loc );
  const gp_Trsf & aTrsf = loc.Transformation();
  for ( pIt = fPoints.begin(); pIt != fPoints.end(); pIt++ )
  {
    TPoint * point = *pIt;
    point->myXYZ = aSurface->Value( point->myUV.X(), point->myUV.Y() );
    if ( !loc.IsIdentity() )
      aTrsf.Transforms( point->myXYZ.ChangeCoord() );
  }

  myIsComputed = true;

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : Apply
//purpose  : Compute nodes coordinates applying
//           the loaded pattern to <theFace>. The first key-point
//           will be mapped into <theNodeIndexOnKeyPoint1>-th node
//=======================================================================

bool SMESH_Pattern::Apply (const SMDS_MeshFace* theFace,
                           const int            theNodeIndexOnKeyPoint1,
                           const bool           theReverse)
{
  MESSAGE(" ::Apply(MeshFace) " );

  if ( !IsLoaded() ) {
    MESSAGE( "Pattern not loaded" );
    return setErrorCode( ERR_APPL_NOT_LOADED );
  }

  // check nb of nodes
  if (theFace->NbNodes() != myNbKeyPntInBoundary.front() ) {
    MESSAGE( myKeyPointIDs.size() << " != " << theFace->NbNodes() );
    return setErrorCode( ERR_APPL_BAD_NB_VERTICES );
  }

  // find points on edges, it fills myNbKeyPntInBoundary
  if ( !findBoundaryPoints() )
    return false;

  // check that there are no holes in a pattern
  if (myNbKeyPntInBoundary.size() > 1 ) {
    return setErrorCode( ERR_APPL_BAD_NB_VERTICES );
  }

  // Define the nodes order

  list< const SMDS_MeshNode* > nodes;
  list< const SMDS_MeshNode* >::iterator n = nodes.end();
  SMDS_ElemIteratorPtr noIt = theFace->nodesIterator();
  int iSub = 0;
  while ( noIt->more() ) {
    const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( noIt->next() );
    nodes.push_back( node );
    if ( iSub++ == theNodeIndexOnKeyPoint1 )
      n = --nodes.end();
  }
  if ( n != nodes.end() ) {
    if ( theReverse ) {
      if ( n != --nodes.end() )
        nodes.splice( nodes.begin(), nodes, ++n, nodes.end() );
      nodes.reverse();
    }
    else if ( n != nodes.begin() )
      nodes.splice( nodes.end(), nodes, nodes.begin(), n );
  }
  list< gp_XYZ > xyzList;
  myOrderedNodes.resize( theFace->NbNodes() );
  for ( iSub = 0, n = nodes.begin(); n != nodes.end(); ++n ) {
    xyzList.push_back( gp_XYZ( (*n)->X(), (*n)->Y(), (*n)->Z() ));
    myOrderedNodes[ iSub++] = *n;
  }

  // Define a face plane

  list< gp_XYZ >::iterator xyzIt = xyzList.begin();
  gp_Pnt P ( *xyzIt++ );
  gp_Vec Vx( P, *xyzIt++ ), N;
  do {
    N = Vx ^ gp_Vec( P, *xyzIt++ );
  } while ( N.SquareMagnitude() <= DBL_MIN && xyzIt != xyzList.end() );
  if ( N.SquareMagnitude() <= DBL_MIN )
    return setErrorCode( ERR_APPLF_BAD_FACE_GEOM );
  gp_Ax2 pos( P, N, Vx );

  // Compute UV of key-points on a plane
  for ( xyzIt = xyzList.begin(), iSub = 1; xyzIt != xyzList.end(); xyzIt++, iSub++ )
  {
    gp_Vec vec ( pos.Location(), *xyzIt );
    TPoint* p = getShapePoints( iSub ).front();
    p->myUV.SetX( vec * pos.XDirection() );
    p->myUV.SetY( vec * pos.YDirection() );
    p->myXYZ = *xyzIt;
  }

  // points on edges to be used for UV computation of in-face points
  list< list< TPoint* > > edgesPointsList;
  edgesPointsList.push_back( list< TPoint* >() );
  list< TPoint* > * edgesPoints = & edgesPointsList.back();
  list< TPoint* >::iterator pIt;

  // compute UV and XYZ of points on edges

  for ( xyzIt = xyzList.begin(); xyzIt != xyzList.end(); iSub++ )
  {
    gp_XYZ& xyz1 = *xyzIt++;
    gp_XYZ& xyz2 = ( xyzIt != xyzList.end() ) ? *xyzIt : xyzList.front();
    
    list< TPoint* > & ePoints = getShapePoints( iSub );
    ePoints.back()->myInitU = 1.0;
    list< TPoint* >::const_iterator pIt = ++ePoints.begin();
    while ( *pIt != ePoints.back() )
    {
      TPoint* p = *pIt++;
      p->myXYZ = xyz1 * ( 1 - p->myInitU ) + xyz2 * p->myInitU;
      gp_Vec vec ( pos.Location(), p->myXYZ );
      p->myUV.SetX( vec * pos.XDirection() );
      p->myUV.SetY( vec * pos.YDirection() );
    }
    // collect on-edge points (excluding the last one)
    edgesPoints->insert( edgesPoints->end(), ePoints.begin(), --ePoints.end());
  }

  // Compute UV and XYZ of in-face points

  // try to use a simple algo to compute UV
  list< TPoint* > & fPoints = getShapePoints( iSub );
  bool isDeformed = false;
  for ( pIt = fPoints.begin(); !isDeformed && pIt != fPoints.end(); pIt++ )
    if ( !compUVByIsoIntersection( edgesPointsList, (*pIt)->myInitUV,
                                  (*pIt)->myUV, isDeformed )) {
      MESSAGE("cant Apply(face)");
      return false;
    }
  // try to use a complex algo if it is a difficult case
  if ( isDeformed && !compUVByElasticIsolines( edgesPointsList, fPoints ))
  {
    for ( ; pIt != fPoints.end(); pIt++ ) // continue with the simple algo
      if ( !compUVByIsoIntersection( edgesPointsList, (*pIt)->myInitUV,
                                    (*pIt)->myUV, isDeformed )) {
        MESSAGE("cant Apply(face)");
        return false;
      }
  }

  for ( pIt = fPoints.begin(); pIt != fPoints.end(); pIt++ )
  {
    (*pIt)->myXYZ = ElSLib::PlaneValue( (*pIt)->myUV.X(), (*pIt)->myUV.Y(), pos );
  }

  myIsComputed = true;

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : undefinedXYZ
//purpose  : 
//=======================================================================

static const gp_XYZ& undefinedXYZ()
{
  static gp_XYZ xyz( 1.e100, 0., 0. );
  return xyz;
}

//=======================================================================
//function : isDefined
//purpose  : 
//=======================================================================

inline static bool isDefined(const gp_XYZ& theXYZ)
{
  return theXYZ.X() < 1.e100;
}

//=======================================================================
//function : mergePoints
//purpose  : Look for coincident points between myXYZs indexed with
//           list<int> of each element of xyzIndGroups. Coincident indices
//           are merged in myElemXYZIDs.
//=======================================================================

void SMESH_Pattern::mergePoints (map<TNodeSet, list<list<int> > >&  indGroups,
                                 map< int, list< list< int >* > > & reverseConnectivity)
{
  map< TNodeSet, list< list< int > > >::iterator indListIt;
  for ( indListIt = indGroups.begin(); indListIt != indGroups.end(); indListIt++ )
  {
    list<list< int > > groups = indListIt->second;
    if ( groups.size() < 2 )
      continue;

//     const TNodeSet & nodes = indListIt->first;
//     TNodeSet::const_iterator n = nodes.begin();
//     for ( ; n != nodes.end(); n++ )
//       cout << *n ;

    // find tolerance
    Bnd_Box box;
    list< int >& indices = groups.front();
    list< int >::iterator ind, ind1, ind2;
    for ( ind = indices.begin(); ind != indices.end(); ind++ )
      box.Add( gp_Pnt( myXYZ[ *ind ]));
    double x, y, z, X, Y, Z;
    box.Get( x, y, z, X, Y, Z );
    gp_Pnt p( x, y, z ), P( X, Y, Z );
    double tol2 = 1.e-4 * p.SquareDistance( P );

    // compare points, replace indices

    list< list< int > >::iterator grpIt1, grpIt2;
    for ( grpIt1 = groups.begin(); grpIt1 != groups.end(); grpIt1++ )
    {
      list< int >& indices1 = *grpIt1;
      grpIt2 = grpIt1;
      for ( grpIt2++; grpIt2 != groups.end(); grpIt2++ )
      {
        list< int >& indices2 = *grpIt2;
        for ( ind1 = indices1.begin(); ind1 != indices1.end(); ind1++ )
        {
          gp_XYZ& p1 = myXYZ[ *ind1 ];
          ind2 = indices2.begin();
          while ( ind2 != indices2.end() )
          {
            gp_XYZ& p2 = myXYZ[ *ind2 ];
            //MESSAGE("COMP: " << *ind1 << " " << *ind2 << " X: " << p2.X() << " tol2: " << tol2);
            if ( ( p1 - p2 ).SquareModulus() <= tol2 )
            {
              ASSERT( reverseConnectivity.find( *ind2 ) != reverseConnectivity.end() );
              list< list< int >* > & elemXYZIDsList = reverseConnectivity[ *ind2 ];
              list< list< int >* >::iterator elemXYZIDs = elemXYZIDsList.begin();
              for ( ; elemXYZIDs != elemXYZIDsList.end(); elemXYZIDs++ )
              {
                ind = find( (*elemXYZIDs)->begin(), (*elemXYZIDs)->end(), *ind2 );
                //MESSAGE( " Replace " << *ind << " with " << *ind1 );
                myXYZ[ *ind ] = undefinedXYZ();
                *ind = *ind1;
              }
              ind2 = indices2.erase( ind2 );
            }
            else
              ind2++;
          }
        }
      }
    }
  }
}

//=======================================================================
//function : Apply
//purpose  : Compute nodes coordinates applying
//           the loaded pattern to <theFaces>. The first key-point
//           will be mapped into <theNodeIndexOnKeyPoint1>-th node
//=======================================================================

bool SMESH_Pattern::Apply (std::set<const SMDS_MeshFace*> theFaces,
                           const int                      theNodeIndexOnKeyPoint1,
                           const bool                     theReverse)
{
  MESSAGE(" ::Apply(set<MeshFace>) " );

  if ( !IsLoaded() ) {
    MESSAGE( "Pattern not loaded" );
    return setErrorCode( ERR_APPL_NOT_LOADED );
  }

  // find points on edges, it fills myNbKeyPntInBoundary
  if ( !findBoundaryPoints() )
    return false;

  // check that there are no holes in a pattern
  if (myNbKeyPntInBoundary.size() > 1 ) {
    return setErrorCode( ERR_APPL_BAD_NB_VERTICES );
  }

  myXYZ.clear();
  myElemXYZIDs.clear();
  myXYZIdToNodeMap.clear();
  myElements.clear();

  myXYZ.resize( myPoints.size() * theFaces.size(), undefinedXYZ() );
  myElements.reserve( theFaces.size() );

  // to find point index
  map< TPoint*, int > pointIndex;
  for ( int i = 0; i < myPoints.size(); i++ )
    pointIndex.insert( make_pair( & myPoints[ i ], i ));

  // to merge nodes on edges of the elements being refined
  typedef set<const SMDS_MeshNode*> TLink;
  map< TLink, list< list< int > > > linkPointIndListMap;
  map< int, list< list< int >* > >  reverseConnectivity;

  int ind1 = 0; // lowest point index for a face

  // apply to each face in theFaces set
  set<const SMDS_MeshFace*>::iterator face = theFaces.begin();
  for ( ; face != theFaces.end(); ++face )
  {
    if ( !Apply( *face, theNodeIndexOnKeyPoint1, theReverse )) {
      MESSAGE( "Failed on " << *face );
      continue;
    }
    myElements.push_back( *face );

    // store computed points belonging to elements
    list< list< int > >::iterator ll = myElemPointIDs.begin();
    for ( ; ll != myElemPointIDs.end(); ++ll )
    {
      myElemXYZIDs.push_back(std::list<int>()); // mpv: in 3.4.1 compiler there is no method with zero args
      list< int >& xyzIds = myElemXYZIDs.back();
      list< int >& pIds = *ll;
      for ( list<int>::iterator id = pIds.begin(); id != pIds.end(); id++ ) {
        int pIndex = *id + ind1;
        xyzIds.push_back( pIndex );
        myXYZ[ pIndex ] = myPoints[ *id ].myXYZ.XYZ();
        reverseConnectivity[ pIndex ].push_back( & xyzIds );
      }
    }
    // put points on links to linkPointIndListMap
    int nbNodes = (*face)->NbNodes(), eID = nbNodes + 1;
    for ( int i = 0; i < nbNodes; i++ )
    {
      const SMDS_MeshNode* n1 = myOrderedNodes[ i ];
      const SMDS_MeshNode* n2 = myOrderedNodes[ i + 1 == nbNodes ? 0 : i + 1 ];
      // make a link of node pointers
      TLink link;
      link.insert( n1 );
      link.insert( n2 );
      // add the link to the map
      list< list< int > >& groups = linkPointIndListMap[ link ];
      groups.push_back(list<int>()); // mpv: in 3.4.1 compiler there is no method with zero args
      list< int >& indList = groups.back();
      list< TPoint* > & linkPoints = getShapePoints( eID++ );
      list< TPoint* >::iterator p = linkPoints.begin();
      // map the first link point to n1
      myXYZIdToNodeMap[ pointIndex[ *p ] + ind1 ] = n1;
      // add points to the map excluding the end points
      for ( p++; *p != linkPoints.back(); p++ )
        indList.push_back( pointIndex[ *p ] + ind1 );
    }
    ind1 += myPoints.size();
  }

  mergePoints( linkPointIndListMap, reverseConnectivity );

  return !myElemXYZIDs.empty();
}

//=======================================================================
//function : Apply
//purpose  : Compute nodes coordinates applying
//           the loaded pattern to <theVolumes>. The (0,0,0) key-point
//           will be mapped into <theNode000Index>-th node. The
//           (0,0,1) key-point will be mapped into <theNode000Index>-th
//           node.
//=======================================================================

bool SMESH_Pattern::Apply (std::set<const SMDS_MeshVolume*> theVolumes,
                           const int                        theNode000Index,
                           const int                        theNode001Index)
{
  MESSAGE(" ::Apply(set<MeshVolumes>) " );

  if ( !IsLoaded() ) {
    MESSAGE( "Pattern not loaded" );
    return setErrorCode( ERR_APPL_NOT_LOADED );
  }

   // bind ID to points
  if ( !findBoundaryPoints() )
    return false;

  // check that there are no holes in a pattern
  if (myNbKeyPntInBoundary.size() > 1 ) {
    return setErrorCode( ERR_APPL_BAD_NB_VERTICES );
  }

  myXYZ.clear();
  myElemXYZIDs.clear();
  myXYZIdToNodeMap.clear();
  myElements.clear();

  myXYZ.resize( myPoints.size() * theVolumes.size(), undefinedXYZ() );
  myElements.reserve( theVolumes.size() );

  // to find point index
  map< TPoint*, int > pointIndex;
  for ( int i = 0; i < myPoints.size(); i++ )
    pointIndex.insert( make_pair( & myPoints[ i ], i ));

  // to merge nodes on edges and faces of the elements being refined
  map< TNodeSet, list< list< int > > > subPointIndListMap;
  map< int, list< list< int >* > >  reverseConnectivity;

  int ind1 = 0; // lowest point index for an element

  // apply to each element in theVolumes set
  set<const SMDS_MeshVolume*>::iterator vol = theVolumes.begin();
  for ( ; vol != theVolumes.end(); ++vol )
  {
    if ( !Apply( *vol, theNode000Index, theNode001Index )) {
      MESSAGE( "Failed on " << *vol );
      continue;
    }
    myElements.push_back( *vol );

    // store computed points belonging to elements
    list< list< int > >::iterator ll = myElemPointIDs.begin();
    for ( ; ll != myElemPointIDs.end(); ++ll )
    {
      myElemXYZIDs.push_back(std::list<int>()); // mpv: in 3.4.1 compiler there is no method with zero args
      list< int >& xyzIds = myElemXYZIDs.back();
      list< int >& pIds = *ll;
      for ( list<int>::iterator id = pIds.begin(); id != pIds.end(); id++ ) {
        int pIndex = *id + ind1;
        xyzIds.push_back( pIndex );
        myXYZ[ pIndex ] = myPoints[ *id ].myXYZ.XYZ();
        reverseConnectivity[ pIndex ].push_back( & xyzIds );
      }
    }
    // put points on edges and faces to subPointIndListMap
    for ( int Id = SMESH_Block::ID_V000; Id <= SMESH_Block::ID_F1yz; Id++ )
    {
      // make a set of sub-points
      TNodeSet subNodes;
      vector< int > subIDs;
      if ( SMESH_Block::IsVertexID( Id )) {
        // use nodes of refined volumes for merge
      }
      else if ( SMESH_Block::IsEdgeID( Id )) {
        SMESH_Block::GetEdgeVertexIDs( Id, subIDs );
        subNodes.insert( myOrderedNodes[ subIDs.front() - 1 ]);
        subNodes.insert( myOrderedNodes[ subIDs.back() - 1 ]);
      }
      else {
        SMESH_Block::GetFaceEdgesIDs( Id, subIDs );
        int e1 = subIDs[ 0 ], e2 = subIDs[ 1 ];
        SMESH_Block::GetEdgeVertexIDs( e1, subIDs );
        subNodes.insert( myOrderedNodes[ subIDs.front() - 1 ]);
        subNodes.insert( myOrderedNodes[ subIDs.back() - 1 ]);
        SMESH_Block::GetEdgeVertexIDs( e2, subIDs );
        subNodes.insert( myOrderedNodes[ subIDs.front() - 1 ]);
        subNodes.insert( myOrderedNodes[ subIDs.back() - 1 ]);
      }
      list< list< int > >& groups = subPointIndListMap[ subNodes ];
      groups.push_back(list<int>()); // mpv: in 3.4.1 compiler there is no method with zero args
      list< int >& indList = groups.back();
      // add points
      list< TPoint* > & points = getShapePoints( Id );
      list< TPoint* >::iterator p = points.begin();
      if ( subNodes.empty() ) // vertex case
        myXYZIdToNodeMap[ pointIndex[ *p ] + ind1 ] = myOrderedNodes[ Id - 1 ];
      else
        for ( ; p != points.end(); p++ )
          indList.push_back( pointIndex[ *p ] + ind1 );
    }
    ind1 += myPoints.size();
  }

  mergePoints( subPointIndListMap, reverseConnectivity );

  return !myElemXYZIDs.empty();
}

//=======================================================================
//function : Load
//purpose  : Create a pattern from the mesh built on <theBlock>
//=======================================================================

bool SMESH_Pattern::Load (SMESH_Mesh*         theMesh,
                          const TopoDS_Shell& theBlock)
{
  MESSAGE(" ::Load(volume) " );
  Clear();
  myIs2D = false;
  SMESHDS_Mesh * aMeshDS = theMesh->GetMeshDS();

  // load shapes in myShapeIDMap
  SMESH_Block block;
  TopoDS_Vertex v1, v2;
  if ( !block.LoadBlockShapes( theBlock, v1, v2, myShapeIDMap ))
    return setErrorCode( ERR_LOADV_BAD_SHAPE );

  // count nodes
  int nbNodes = 0, shapeID;
  for ( shapeID = 1; shapeID <= myShapeIDMap.Extent(); shapeID++ )
  {
    const TopoDS_Shape& S = myShapeIDMap( shapeID );
    SMESHDS_SubMesh * aSubMesh = aMeshDS->MeshElements( S );
    if ( aSubMesh )
      nbNodes += aSubMesh->NbNodes();
  }
  myPoints.resize( nbNodes );

  // load U of points on edges
  TNodePointIDMap nodePointIDMap;
  int iPoint = 0;
  for ( shapeID = 1; shapeID <= myShapeIDMap.Extent(); shapeID++ )
  {
    const TopoDS_Shape& S = myShapeIDMap( shapeID );
    list< TPoint* > & shapePoints = getShapePoints( shapeID );
    SMESHDS_SubMesh * aSubMesh = aMeshDS->MeshElements( S );
    if ( ! aSubMesh ) continue;
    SMDS_NodeIteratorPtr nIt = aSubMesh->GetNodes();
    if ( !nIt->more() ) continue;

      // store a node and a point
    while ( nIt->more() ) {
      const SMDS_MeshNode* node = static_cast<const SMDS_MeshNode*>( nIt->next() );
      nodePointIDMap.insert( make_pair( node, iPoint ));
      if ( block.IsVertexID( shapeID ))
        myKeyPointIDs.push_back( iPoint );
      TPoint* p = & myPoints[ iPoint++ ];
      shapePoints.push_back( p );
      p->myXYZ.SetCoord( node->X(), node->Y(), node->Z() );
      p->myInitXYZ.SetCoord( 0,0,0 );
    }
    list< TPoint* >::iterator pIt = shapePoints.begin();

    // compute init XYZ
    switch ( S.ShapeType() )
    {
    case TopAbs_VERTEX:
    case TopAbs_EDGE: {

      for ( ; pIt != shapePoints.end(); pIt++ ) {
        double * coef = block.GetShapeCoef( shapeID );
        for ( int iCoord = 1; iCoord <= 3; iCoord++ )
          if ( coef[ iCoord - 1] > 0 )
            (*pIt)->myInitXYZ.SetCoord( iCoord, 1. );
      }
      if ( S.ShapeType() == TopAbs_VERTEX )
        break;

      const TopoDS_Edge& edge = TopoDS::Edge( S );
      double f,l;
      BRep_Tool::Range( edge, f, l );
      int iCoord     = SMESH_Block::GetCoordIndOnEdge( shapeID );
      bool isForward = SMESH_Block::IsForwardEdge( edge, myShapeIDMap );
      pIt = shapePoints.begin();
      nIt = aSubMesh->GetNodes();
      for ( ; nIt->more(); pIt++ )
      {
        const SMDS_MeshNode* node = 
          static_cast<const SMDS_MeshNode*>( nIt->next() );
        const SMDS_EdgePosition* epos =
          static_cast<const SMDS_EdgePosition*>(node->GetPosition().get());
        double u = ( epos->GetUParameter() - f ) / ( l - f );
        (*pIt)->myInitXYZ.SetCoord( iCoord, isForward ? u : 1 - u );
      }
      break;
    }
    default:
      for ( ; pIt != shapePoints.end(); pIt++ )
      {
        if ( !block.ComputeParameters( (*pIt)->myXYZ, (*pIt)->myInitXYZ, shapeID )) {
          MESSAGE( "!block.ComputeParameters()" );
          return setErrorCode( ERR_LOADV_COMPUTE_PARAMS );
        }
      }
    }
  } // loop on block sub-shapes

  // load elements

  SMESHDS_SubMesh * aSubMesh = aMeshDS->MeshElements( theBlock );
  if ( aSubMesh )
  {
    SMDS_ElemIteratorPtr elemIt = aSubMesh->GetElements();
    while ( elemIt->more() ) {
      SMDS_ElemIteratorPtr nIt = elemIt->next()->nodesIterator();
      myElemPointIDs.push_back( list< int >() );
      list< int >& elemPoints = myElemPointIDs.back();
      while ( nIt->more() )
        elemPoints.push_back( nodePointIDMap[ nIt->next() ]);
    }
  }

  myIsBoundaryPointsFound = true;

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : Apply
//purpose  : Compute nodes coordinates applying
//           the loaded pattern to <theBlock>. The (0,0,0) key-point
//           will be mapped into <theVertex000>. The (0,0,1)
//           fifth key-point will be mapped into <theVertex001>.
//=======================================================================

bool SMESH_Pattern::Apply (const TopoDS_Shell&  theBlock,
                           const TopoDS_Vertex& theVertex000,
                           const TopoDS_Vertex& theVertex001)
{
  MESSAGE(" ::Apply(volume) " );

  if (!findBoundaryPoints()     || // bind ID to points
      !setShapeToMesh( theBlock )) // check theBlock is a suitable shape
    return false;

  SMESH_Block block;  // bind ID to shape
  if (!block.LoadBlockShapes( theBlock, theVertex000, theVertex001, myShapeIDMap ))
    return setErrorCode( ERR_APPLV_BAD_SHAPE );

  // compute XYZ of points on shapes

  for ( int shapeID = 1; shapeID <= myShapeIDMap.Extent(); shapeID++ )
  {
    list< TPoint* > & shapePoints = getShapePoints( shapeID );
    list< TPoint* >::iterator pIt = shapePoints.begin();
    const TopoDS_Shape& S = myShapeIDMap( shapeID );
    switch ( S.ShapeType() )
    {
    case TopAbs_VERTEX: {

      for ( ; pIt != shapePoints.end(); pIt++ )
        block.VertexPoint( shapeID, (*pIt)->myXYZ.ChangeCoord() );
      break;
    }
    case TopAbs_EDGE: {

      for ( ; pIt != shapePoints.end(); pIt++ )
        block.EdgePoint( shapeID, (*pIt)->myInitXYZ, (*pIt)->myXYZ.ChangeCoord() );
      break;
    }
    case TopAbs_FACE: {

      for ( ; pIt != shapePoints.end(); pIt++ )
        block.FacePoint( shapeID, (*pIt)->myInitXYZ, (*pIt)->myXYZ.ChangeCoord() );
      break;
    }
    default:
      for ( ; pIt != shapePoints.end(); pIt++ )
        block.ShellPoint( (*pIt)->myInitXYZ, (*pIt)->myXYZ.ChangeCoord() );
    }
  } // loop on block sub-shapes

  myIsComputed = true;

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : Apply
//purpose  : Compute nodes coordinates applying
//           the loaded pattern to <theVolume>. The (0,0,0) key-point
//           will be mapped into <theNode000Index>-th node. The
//           (0,0,1) key-point will be mapped into <theNode000Index>-th
//           node.
//=======================================================================

bool SMESH_Pattern::Apply (const SMDS_MeshVolume* theVolume,
                           const int              theNode000Index,
                           const int              theNode001Index)
{
  MESSAGE(" ::Apply(MeshVolume) " );

  if (!findBoundaryPoints()) // bind ID to points
    return false;

  SMESH_Block block;  // bind ID to shape
  if (!block.LoadMeshBlock( theVolume, theNode000Index, theNode001Index, myOrderedNodes ))
    return setErrorCode( ERR_APPLV_BAD_SHAPE );
  // compute XYZ of points on shapes

  for ( int ID = SMESH_Block::ID_V000; ID <= SMESH_Block::ID_Shell; ID++ )
  {
    list< TPoint* > & shapePoints = getShapePoints( ID );
    list< TPoint* >::iterator pIt = shapePoints.begin();

    if ( block.IsVertexID( ID ))
      for ( ; pIt != shapePoints.end(); pIt++ ) {
        block.VertexPoint( ID, (*pIt)->myXYZ.ChangeCoord() );
      }
    else if ( block.IsEdgeID( ID ))
      for ( ; pIt != shapePoints.end(); pIt++ ) {
        block.EdgePoint( ID, (*pIt)->myInitXYZ, (*pIt)->myXYZ.ChangeCoord() );
      }
    else if ( block.IsFaceID( ID ))
      for ( ; pIt != shapePoints.end(); pIt++ ) {
        block.FacePoint( ID, (*pIt)->myInitXYZ, (*pIt)->myXYZ.ChangeCoord() );
      }
    else
      for ( ; pIt != shapePoints.end(); pIt++ )
        block.ShellPoint( (*pIt)->myInitXYZ, (*pIt)->myXYZ.ChangeCoord() );
  } // loop on block sub-shapes

  myIsComputed = true;

  return setErrorCode( ERR_OK );
}

//=======================================================================
//function : MakeMesh
//purpose  : Create nodes and elements in <theMesh> using nodes
//           coordinates computed by either of Apply...() methods
//=======================================================================

bool SMESH_Pattern::MakeMesh(SMESH_Mesh* theMesh)
{
  MESSAGE(" ::MakeMesh() " );
  if ( !myIsComputed )
    return setErrorCode( ERR_MAKEM_NOT_COMPUTED );

  SMESHDS_Mesh* aMeshDS = theMesh->GetMeshDS();
  SMESH_MeshEditor editor( theMesh ); 

  // clear elements and nodes existing on myShape

  if ( !myShape.IsNull() )
  {
    SMESH_subMesh * aSubMesh = theMesh->GetSubMeshContaining( myShape );
    SMESHDS_SubMesh * aSubMeshDS = aMeshDS->MeshElements( myShape );
    if ( aSubMesh )
      aSubMesh->ComputeStateEngine( SMESH_subMesh::CLEAN );
    else if ( aSubMeshDS )
    {
      SMDS_ElemIteratorPtr eIt = aSubMeshDS->GetElements();
      while ( eIt->more() )
        aMeshDS->RemoveElement( eIt->next() );
      SMDS_NodeIteratorPtr nIt = aSubMeshDS->GetNodes();
      while ( nIt->more() )
        aMeshDS->RemoveNode( static_cast<const SMDS_MeshNode*>( nIt->next() ));
    }
  }

  bool onMeshElements = ( !myElements.empty() );

  // loop on sub-shapes of myShape: create nodes and build point-node map

  vector< const SMDS_MeshNode* >       nodesVector;
  map< TPoint*, const SMDS_MeshNode* > pointNodeMap;
  if ( onMeshElements )
  {
    nodesVector.resize( myXYZ.size() );
    for ( int i = 0; i < myXYZ.size(); ++i ) {
      map< int, const SMDS_MeshNode*>::iterator idNode = myXYZIdToNodeMap.find( i );
      if ( idNode != myXYZIdToNodeMap.end() )
        nodesVector[ i ] = idNode->second;
      else if ( isDefined( myXYZ[ i ] ))
        nodesVector[ i ] = aMeshDS->AddNode (myXYZ[ i ].X(),
                                             myXYZ[ i ].Y(),
                                             myXYZ[ i ].Z());
    }
  }
  else
  {
    map< int, list< TPoint* > >::iterator idPointIt = myShapeIDToPointsMap.begin();
    for ( ; idPointIt != myShapeIDToPointsMap.end(); idPointIt++ )
    {
      TopoDS_Shape S;
      SMESHDS_SubMesh * subMeshDS = 0;
      if ( !myShapeIDMap.IsEmpty() ) {
        S = myShapeIDMap( idPointIt->first );
        subMeshDS = aMeshDS->MeshElements( S );
      }
      list< TPoint* > & points = idPointIt->second;
      list< TPoint* >::iterator pIt = points.begin();
      for ( ; pIt != points.end(); pIt++ )
      {
        TPoint* point = *pIt;
        if ( pointNodeMap.find( point ) != pointNodeMap.end() )
          continue;
        SMDS_MeshNode* node = aMeshDS->AddNode (point->myXYZ.X(),
                                                point->myXYZ.Y(),
                                                point->myXYZ.Z());
        pointNodeMap.insert( make_pair( point, node ));
        if ( subMeshDS ) {
          switch ( S.ShapeType() ) {
          case TopAbs_VERTEX: {
            aMeshDS->SetNodeOnVertex( node, TopoDS::Vertex( S ));
            break;
          }
          case TopAbs_EDGE: {
            aMeshDS->SetNodeOnEdge( node, TopoDS::Edge( S ));
            SMDS_EdgePosition* epos =
              dynamic_cast<SMDS_EdgePosition *>(node->GetPosition().get());
            epos->SetUParameter( point->myU );
            break;
          }
          case TopAbs_FACE: {
            aMeshDS->SetNodeOnFace( node, TopoDS::Face( S ));
            SMDS_FacePosition* pos =
              dynamic_cast<SMDS_FacePosition *>(node->GetPosition().get());
            pos->SetUParameter( point->myUV.X() );
            pos->SetVParameter( point->myUV.Y() );
            break;
          }
          default:
            aMeshDS->SetNodeInVolume( node, TopoDS::Shell( S ));
          }
        }
      }
    }
  }
  
  // create elements

  // shapes and groups myElements are on
  vector< int > shapeIDs;
  vector< list< SMESHDS_Group* > > groups;
  if ( onMeshElements )
  {
    shapeIDs.resize( myElements.size() );
    groups.resize( myElements.size() );
    const set<SMESHDS_GroupBase*>& allGroups = aMeshDS->GetGroups();
    set<SMESHDS_GroupBase*>::const_iterator grIt;
    for ( int i = 0; i < myElements.size(); i++ )
    {
      shapeIDs[ i ] = editor.FindShape( myElements[ i ] );
      for ( grIt = allGroups.begin(); grIt != allGroups.end(); grIt++ ) {
        SMESHDS_Group* group = dynamic_cast<SMESHDS_Group*>( *grIt );
        if ( group && group->SMDSGroup().Contains( myElements[ i ] ))
          groups[ i ].push_back( group );
      }
    }
  }
  int nbElems = myElemPointIDs.size(); // nb elements in a pattern

  list<list< int > >::iterator epIt, epEnd;
  if ( onMeshElements ) {
    epIt  = myElemXYZIDs.begin();
    epEnd = myElemXYZIDs.end();
  }
  else {
    epIt  = myElemPointIDs.begin();
    epEnd = myElemPointIDs.end();
  }
  for ( int iElem = 0; epIt != epEnd; epIt++, iElem++ )
  {
    list< int > & elemPoints = *epIt;
    // retrieve nodes
    const SMDS_MeshNode* nodes[ 8 ];
    list< int >::iterator iIt = elemPoints.begin();
    int nbNodes;
    for ( nbNodes = 0; iIt != elemPoints.end(); iIt++ ) {
      if ( onMeshElements )
        nodes[ nbNodes++ ] = nodesVector[ *iIt ];
      else
        nodes[ nbNodes++ ] = pointNodeMap[ & myPoints[ *iIt ]];
    }
    // add an element
    const SMDS_MeshElement* elem = 0;
    if ( myIs2D ) {
      switch ( nbNodes ) {
      case 3:
        elem = aMeshDS->AddFace( nodes[0], nodes[1], nodes[2] ); break;
      case 4:
        elem = aMeshDS->AddFace( nodes[0], nodes[1], nodes[2], nodes[3] ); break;
      default:
        ASSERT( nbNodes < 8 );
      }
    }
    else {
      switch ( nbNodes ) {
      case 4:
        elem = aMeshDS->AddVolume (nodes[0], nodes[1], nodes[2], nodes[3] ); break;
      case 5:
        elem = aMeshDS->AddVolume (nodes[0], nodes[1], nodes[2], nodes[3],
                                   nodes[4] ); break;
      case 6:
        elem = aMeshDS->AddVolume (nodes[0], nodes[1], nodes[2], nodes[3],
                                   nodes[4], nodes[5] ); break;
      case 8:
        elem = aMeshDS->AddVolume (nodes[0], nodes[1], nodes[2], nodes[3],
                                   nodes[4], nodes[5], nodes[6], nodes[7] ); break;
      default:
        ASSERT( nbNodes < 8 );
      }
    }
    // set element on a shape
    if ( elem && onMeshElements ) // applied to mesh elements
    {
      int elemIndex = iElem / nbElems;
      int shapeID = shapeIDs[ elemIndex ];
      if ( shapeID > 0 ) {
        aMeshDS->SetMeshElementOnShape( elem, shapeID );
        // set nodes on a shape
        TopoDS_Shape S = aMeshDS->IndexToShape( shapeID );
        if ( S.ShapeType() == TopAbs_SOLID ) {
          TopoDS_Iterator shellIt( S );
          if ( shellIt.More() )
            shapeID = aMeshDS->ShapeToIndex( shellIt.Value() );
        }
        SMDS_ElemIteratorPtr noIt = elem->nodesIterator();
        while ( noIt->more() ) {
          SMDS_MeshNode* node = const_cast<SMDS_MeshNode*>
            ( static_cast<const SMDS_MeshNode*>( noIt->next() ));
          if ( !node->GetPosition() || !node->GetPosition()->GetShapeId() ) {
            if ( S.ShapeType() == TopAbs_FACE )
              aMeshDS->SetNodeOnFace( node, shapeID );
            else
              aMeshDS->SetNodeInVolume( node, shapeID );
          }
        }
      }
      // add elem in groups
      list< SMESHDS_Group* >::iterator g = groups[ elemIndex ].begin();
      for ( ; g != groups[ elemIndex ].end(); ++g )
        (*g)->SMDSGroup().Add( elem );
    }
    if ( elem && !myShape.IsNull() ) // applied to shape
      aMeshDS->SetMeshElementOnShape( elem, myShape );
  }

  // make that SMESH_subMesh::_computeState = COMPUTE_OK
  // so that operations with hypotheses will erase the mesh being built

  SMESH_subMesh * subMesh;
  if ( !myShape.IsNull() ) {
    subMesh = theMesh->GetSubMeshContaining( myShape );
    if ( subMesh )
      subMesh->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
  }
  if ( onMeshElements ) {
    list< int > elemIDs;
    for ( int i = 0; i < myElements.size(); i++ )
    {
      int shapeID = shapeIDs[ i ];
      if ( shapeID > 0 ) {
        TopoDS_Shape S = aMeshDS->IndexToShape( shapeID );
        subMesh = theMesh->GetSubMeshContaining( S );
        if ( subMesh )
          subMesh->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
      }
      elemIDs.push_back( myElements[ i ]->GetID() );
    }
    // remove refined elements
    editor.Remove( elemIDs, false );
  }

  return setErrorCode( ERR_OK );
}


//=======================================================================
//function : arrangeBoundaries
//purpose  : if there are several wires, arrange boundaryPoints so that
//           the outer wire goes first and fix inner wires orientation
//           update myKeyPointIDs to correspond to the order of key-points
//           in boundaries; sort internal boundaries by the nb of key-points
//=======================================================================

void SMESH_Pattern::arrangeBoundaries (list< list< TPoint* > >& boundaryList)
{
  typedef list< list< TPoint* > >::iterator TListOfListIt;
  TListOfListIt bndIt;
  list< TPoint* >::iterator pIt;

  int nbBoundaries = boundaryList.size();
  if ( nbBoundaries > 1 )
  {
    // sort boundaries by nb of key-points
    if ( nbBoundaries > 2 )
    {
      // move boundaries in tmp list
      list< list< TPoint* > > tmpList; 
      tmpList.splice( tmpList.begin(), boundaryList, boundaryList.begin(), boundaryList.end());
      // make a map nb-key-points to boundary-position-in-tmpList,
      // boundary-positions get ordered in it
      typedef map< int, TListOfListIt > TNbKpBndPosMap;
      TNbKpBndPosMap nbKpBndPosMap;
      bndIt = tmpList.begin();
      list< int >::iterator nbKpIt = myNbKeyPntInBoundary.begin();
      for ( ; nbKpIt != myNbKeyPntInBoundary.end(); nbKpIt++, bndIt++ ) {
        int nb = *nbKpIt * nbBoundaries;
        while ( nbKpBndPosMap.find ( nb ) != nbKpBndPosMap.end() )
          nb++;
        nbKpBndPosMap.insert( TNbKpBndPosMap::value_type( nb, bndIt ));
      }
      // move boundaries back to boundaryList
      TNbKpBndPosMap::iterator nbKpBndPosIt = nbKpBndPosMap.begin();
      for ( ; nbKpBndPosIt != nbKpBndPosMap.end(); nbKpBndPosIt++ ) {
        TListOfListIt & bndPos2 = (*nbKpBndPosIt).second;
        TListOfListIt bndPos1 = bndPos2++;
        boundaryList.splice( boundaryList.end(), tmpList, bndPos1, bndPos2 );
      }
    }

    // Look for the outer boundary: the one with the point with the least X
    double leastX = DBL_MAX;
    TListOfListIt outerBndPos;
    for ( bndIt = boundaryList.begin(); bndIt != boundaryList.end(); bndIt++ )
    {
      list< TPoint* >& boundary = (*bndIt);
      for ( pIt = boundary.begin(); pIt != boundary.end(); pIt++)
      {
        TPoint* point = *pIt;
        if ( point->myInitXYZ.X() < leastX ) {
          leastX = point->myInitXYZ.X();
          outerBndPos = bndIt;
        }
      }
    }

    if ( outerBndPos != boundaryList.begin() )
      boundaryList.splice( boundaryList.begin(), boundaryList, outerBndPos, ++outerBndPos );

  } // if nbBoundaries > 1
                 
  // Check boundaries orientation and re-fill myKeyPointIDs

  set< TPoint* > keyPointSet;
  list< int >::iterator kpIt = myKeyPointIDs.begin();
  for ( ; kpIt != myKeyPointIDs.end(); kpIt++ )
    keyPointSet.insert( & myPoints[ *kpIt ]);
  myKeyPointIDs.clear();

  // update myNbKeyPntInBoundary also
  list< int >::iterator nbKpIt = myNbKeyPntInBoundary.begin();

  for ( bndIt = boundaryList.begin(); bndIt != boundaryList.end(); bndIt++, nbKpIt++ )
  {
    // find the point with the least X
    double leastX = DBL_MAX;
    list< TPoint* >::iterator xpIt;
    list< TPoint* >& boundary = (*bndIt);
    for ( pIt = boundary.begin(); pIt != boundary.end(); pIt++)
    {
      TPoint* point = *pIt;
      if ( point->myInitXYZ.X() < leastX ) {
        leastX = point->myInitXYZ.X();
        xpIt = pIt;
      }
    }
    // find points next to the point with the least X
    TPoint* p = *xpIt, *pPrev, *pNext;
    if ( p == boundary.front() )
      pPrev = *(++boundary.rbegin());
    else {
      xpIt--;
      pPrev = *xpIt;
      xpIt++;
    }
    if ( p == boundary.back() )
      pNext = *(++boundary.begin());
    else {
      xpIt++;
      pNext = *xpIt;
    }
    // vectors of boundary direction near <p>
    gp_Vec2d v1( pPrev->myInitUV, p->myInitUV ), v2( p->myInitUV, pNext->myInitUV );
    double sqMag1 = v1.SquareMagnitude(), sqMag2 = v2.SquareMagnitude();
    if ( sqMag1 > DBL_MIN && sqMag2 > DBL_MIN ) {
      double yPrev = v1.Y() / sqrt( sqMag1 );
      double yNext = v2.Y() / sqrt( sqMag2 );
      double sumY = yPrev + yNext;
      bool reverse;
      if ( bndIt == boundaryList.begin() ) // outer boundary
        reverse = sumY > 0;
      else
        reverse = sumY < 0;
      if ( reverse )
        boundary.reverse();
    }

    // Put key-point IDs of a well-oriented boundary in myKeyPointIDs
    (*nbKpIt) = 0; // count nb of key-points again
    pIt = boundary.begin();
    for ( ; pIt != boundary.end(); pIt++)
    {
      TPoint* point = *pIt;
      if ( keyPointSet.find( point ) == keyPointSet.end() )
        continue;
      // find an index of a keypoint
      int index = 0;
      vector< TPoint >::const_iterator pVecIt = myPoints.begin();
      for ( ; pVecIt != myPoints.end(); pVecIt++, index++ )
        if ( &(*pVecIt) == point )
          break;
      myKeyPointIDs.push_back( index );
      (*nbKpIt)++;
    }
    myKeyPointIDs.pop_back(); // remove the first key-point from the back
    (*nbKpIt)--;

  } // loop on a list of boundaries

  ASSERT( myKeyPointIDs.size() == keyPointSet.size() );
}

//=======================================================================
//function : findBoundaryPoints
//purpose  : if loaded from file, find points to map on edges and faces and
//           compute their parameters
//=======================================================================

bool SMESH_Pattern::findBoundaryPoints()
{
  if ( myIsBoundaryPointsFound ) return true;

  MESSAGE(" findBoundaryPoints() ");

  if ( myIs2D )
  {
    set< TPoint* > pointsInElems;

    // Find free links of elements:
    // put links of all elements in a set and remove links encountered twice

    typedef pair< TPoint*, TPoint*> TLink;
    set< TLink > linkSet;
    list<list< int > >::iterator epIt = myElemPointIDs.begin();
    for ( ; epIt != myElemPointIDs.end(); epIt++ )
    {
      list< int > & elemPoints = *epIt;
      list< int >::iterator pIt = elemPoints.begin();
      int prevP = elemPoints.back();
      for ( ; pIt != elemPoints.end(); pIt++ ) {
        TPoint* p1 = & myPoints[ prevP ];
        TPoint* p2 = & myPoints[ *pIt ];
        TLink link(( p1 < p2 ? p1 : p2 ), ( p1 < p2 ? p2 : p1 ));
        ASSERT( link.first != link.second );
        pair<set< TLink >::iterator,bool> itUniq = linkSet.insert( link );
        if ( !itUniq.second )
          linkSet.erase( itUniq.first );
        prevP = *pIt;

        pointsInElems.insert( p1 );
      }
    }
    // Now linkSet contains only free links,
    // find the points order that they have in boundaries

    // 1. make a map of key-points
    set< TPoint* > keyPointSet;
    list< int >::iterator kpIt = myKeyPointIDs.begin();
    for ( ; kpIt != myKeyPointIDs.end(); kpIt++ )
      keyPointSet.insert( & myPoints[ *kpIt ]);

    // 2. chain up boundary points
    list< list< TPoint* > > boundaryList;
    boundaryList.push_back( list< TPoint* >() );
    list< TPoint* > * boundary = & boundaryList.back();

    TPoint *point1, *point2, *keypoint1;
    kpIt = myKeyPointIDs.begin();
    point1 = keypoint1 = & myPoints[ *kpIt++ ];
    // loop on free links: look for the next point
    int iKeyPoint = 0;
    set< TLink >::iterator lIt = linkSet.begin();
    while ( lIt != linkSet.end() )
    {
      if ( (*lIt).first == point1 )
        point2 = (*lIt).second;
      else if ( (*lIt).second == point1 )
        point2 = (*lIt).first;
      else {
        lIt++;
        continue;
      }
      linkSet.erase( lIt );
      lIt = linkSet.begin();

      if ( keyPointSet.find( point2 ) == keyPointSet.end() ) // not a key-point
      {
        boundary->push_back( point2 );
      }
      else // a key-point found
      {
        keyPointSet.erase( point2 ); // keyPointSet contains not found key-points only
        iKeyPoint++;
        if ( point2 != keypoint1 ) // its not the boundary end
        {
          boundary->push_back( point2 );
        }
        else  // the boundary end reached
        {
          boundary->push_front( keypoint1 );
          boundary->push_back( keypoint1 );
          myNbKeyPntInBoundary.push_back( iKeyPoint );
          if ( keyPointSet.empty() )
            break; // all boundaries containing key-points are found

          // prepare to search for the next boundary
          boundaryList.push_back( list< TPoint* >() );
          boundary = & boundaryList.back();
          point2 = keypoint1 = (*keyPointSet.begin());
        }
      }
      point1 = point2;
    } // loop on the free links set

    if ( boundary->empty() ) {
      MESSAGE(" a separate key-point");
      return setErrorCode( ERR_READ_BAD_KEY_POINT );
    }

    // if there are several wires, arrange boundaryPoints so that
    // the outer wire goes first and fix inner wires orientation;
    // sort myKeyPointIDs to correspond to the order of key-points
    // in boundaries
    arrangeBoundaries( boundaryList );

    // Find correspondence shape ID - points,
    // compute points parameter on edge

    keyPointSet.clear();
    for ( kpIt = myKeyPointIDs.begin(); kpIt != myKeyPointIDs.end(); kpIt++ )
      keyPointSet.insert( & myPoints[ *kpIt ]);

    set< TPoint* > edgePointSet; // to find in-face points
    int vertexID = 1; // the first index in TopTools_IndexedMapOfShape
    int edgeID = myKeyPointIDs.size() + 1;

    list< list< TPoint* > >::iterator bndIt = boundaryList.begin();
    for ( ; bndIt != boundaryList.end(); bndIt++ )
    {
      boundary = & (*bndIt);
      double edgeLength = 0;
      list< TPoint* >::iterator pIt = boundary->begin();
      getShapePoints( edgeID ).push_back( *pIt );
      getShapePoints( vertexID++ ).push_back( *pIt );
      for ( pIt++; pIt != boundary->end(); pIt++)
      {
        list< TPoint* > & edgePoints = getShapePoints( edgeID );
        TPoint* prevP = edgePoints.empty() ? 0 : edgePoints.back();
        TPoint* point = *pIt;
        edgePointSet.insert( point );
        if ( keyPointSet.find( point ) == keyPointSet.end() ) // inside-edge point
        {
          edgePoints.push_back( point );
          edgeLength += ( point->myInitUV - prevP->myInitUV ).Modulus();
          point->myInitU = edgeLength;
        }
        else // a key-point
        {
          // treat points on the edge which ends up: compute U [0,1]
          edgePoints.push_back( point );
          if ( edgePoints.size() > 2 ) {
            edgeLength += ( point->myInitUV - prevP->myInitUV ).Modulus();
            list< TPoint* >::iterator epIt = edgePoints.begin();
            for ( ; epIt != edgePoints.end(); epIt++ )
              (*epIt)->myInitU /= edgeLength;
          }
          // begin the next edge treatment
          edgeLength = 0;
          edgeID++;
          if ( point != boundary->front() ) { // not the first key-point again
            getShapePoints( edgeID ).push_back( point );
            getShapePoints( vertexID++ ).push_back( point );
          }
        }
      }
    }

    // find in-face points
    list< TPoint* > & facePoints = getShapePoints( edgeID );
    vector< TPoint >::iterator pVecIt = myPoints.begin();
    for ( ; pVecIt != myPoints.end(); pVecIt++ ) {
      TPoint* point = &(*pVecIt);
      if ( edgePointSet.find( point ) == edgePointSet.end() &&
          pointsInElems.find( point ) != pointsInElems.end())
        facePoints.push_back( point );
    }

  } // 2D case

  else // 3D case
  {
    // bind points to shapes according to point parameters
    vector< TPoint >::iterator pVecIt = myPoints.begin();
    for ( int i = 0; pVecIt != myPoints.end(); pVecIt++, i++ ) {
      TPoint* point = &(*pVecIt);
      int shapeID = SMESH_Block::GetShapeIDByParams( point->myInitXYZ );
      getShapePoints( shapeID ).push_back( point );
      // detect key-points
      if ( SMESH_Block::IsVertexID( shapeID ))
        myKeyPointIDs.push_back( i );        
    }
  }

  myIsBoundaryPointsFound = true;
  return myIsBoundaryPointsFound;
}

//=======================================================================
//function : Clear
//purpose  : clear fields
//=======================================================================

void SMESH_Pattern::Clear()
{
  myIsComputed = myIsBoundaryPointsFound = false;

  myPoints.clear();
  myKeyPointIDs.clear();
  myElemPointIDs.clear();
  myShapeIDToPointsMap.clear();
  myShapeIDMap.Clear();
  myShape.Nullify();
  myNbKeyPntInBoundary.clear();
}

//=======================================================================
//function : setShapeToMesh
//purpose  : set a shape to be meshed. Return True if meshing is possible
//=======================================================================

bool SMESH_Pattern::setShapeToMesh(const TopoDS_Shape& theShape)
{
  if ( !IsLoaded() ) {
    MESSAGE( "Pattern not loaded" );
    return setErrorCode( ERR_APPL_NOT_LOADED );
  }

  TopAbs_ShapeEnum aType = theShape.ShapeType();
  bool dimOk = ( myIs2D ? aType == TopAbs_FACE : aType == TopAbs_SHELL );
  if ( !dimOk ) {
    MESSAGE( "Pattern dimention mismatch" );
    return setErrorCode( ERR_APPL_BAD_DIMENTION );
  }

  // check if a face is closed
  int nbNodeOnSeamEdge = 0;
  if ( myIs2D ) {
    TopoDS_Face face = TopoDS::Face( theShape );
    TopExp_Explorer eExp( theShape, TopAbs_EDGE );
    for ( ; eExp.More() && nbNodeOnSeamEdge == 0; eExp.Next() )
      if ( BRep_Tool::IsClosed( TopoDS::Edge( eExp.Current() ), face ))
        nbNodeOnSeamEdge = 2;
  }
    
  // check nb of vertices
  TopTools_IndexedMapOfShape vMap;
  TopExp::MapShapes( theShape, TopAbs_VERTEX, vMap );
  if ( vMap.Extent() + nbNodeOnSeamEdge != myKeyPointIDs.size() ) {
    MESSAGE( myKeyPointIDs.size() << " != " << vMap.Extent() );
    return setErrorCode( ERR_APPL_BAD_NB_VERTICES );
  }

  myShapeIDMap.Clear();
  myShape = theShape;
  return true;
}

//=======================================================================
//function : GetMappedPoints
//purpose  : Return nodes coordinates computed by Apply() method
//=======================================================================

bool SMESH_Pattern::GetMappedPoints ( list< const gp_XYZ * > & thePoints ) const
{
  thePoints.clear();
  if ( !myIsComputed )
    return false;

  if ( myElements.empty() ) { // applied to shape
    vector< TPoint >::const_iterator pVecIt = myPoints.begin();
    for ( ; pVecIt != myPoints.end(); pVecIt++ )
      thePoints.push_back( & (*pVecIt).myXYZ.XYZ() );
  }
  else { // applied to mesh elements
    const gp_XYZ * definedXYZ = & myPoints[ myKeyPointIDs.front() ].myXYZ.XYZ();
    vector<gp_XYZ>::const_iterator xyz = myXYZ.begin();
    for ( ; xyz != myXYZ.end(); ++xyz )
      if ( !isDefined( *xyz ))
        thePoints.push_back( definedXYZ );
      else
        thePoints.push_back( & (*xyz) );
  }
  return !thePoints.empty();
}


//=======================================================================
//function : GetPoints
//purpose  : Return nodes coordinates of the pattern
//=======================================================================

bool SMESH_Pattern::GetPoints ( list< const gp_XYZ * > & thePoints ) const
{
  thePoints.clear();

  if ( !IsLoaded() )
    return false;

  vector< TPoint >::const_iterator pVecIt = myPoints.begin();
  for ( ; pVecIt != myPoints.end(); pVecIt++ )
    thePoints.push_back( & (*pVecIt).myInitXYZ );

  return ( thePoints.size() > 0 );
}

//=======================================================================
//function : getShapePoints
//purpose  : return list of points located on theShape
//=======================================================================

list< SMESH_Pattern::TPoint* > &
  SMESH_Pattern::getShapePoints(const TopoDS_Shape& theShape)
{
  int aShapeID;
  if ( !myShapeIDMap.Contains( theShape ))
    aShapeID = myShapeIDMap.Add( theShape );
  else
    aShapeID = myShapeIDMap.FindIndex( theShape );

  return myShapeIDToPointsMap[ aShapeID ];
}

//=======================================================================
//function : getShapePoints
//purpose  : return list of points located on the shape
//=======================================================================

list< SMESH_Pattern::TPoint* > & SMESH_Pattern::getShapePoints(const int theShapeID)
{
  return myShapeIDToPointsMap[ theShapeID ];
}

//=======================================================================
//function : DumpPoints
//purpose  : Debug
//=======================================================================

void SMESH_Pattern::DumpPoints() const
{
#ifdef _DEBUG_
  vector< TPoint >::const_iterator pVecIt = myPoints.begin();
  for ( int i = 0; pVecIt != myPoints.end(); pVecIt++, i++ )
    cout << i << ": " << *pVecIt;
#endif
}

//=======================================================================
//function : TPoint()
//purpose  : 
//=======================================================================

SMESH_Pattern::TPoint::TPoint()
{
#ifdef _DEBUG_
  myInitXYZ.SetCoord(0,0,0);
  myInitUV.SetCoord(0.,0.);
  myInitU = 0;
  myXYZ.SetCoord(0,0,0);
  myUV.SetCoord(0.,0.);
  myU = 0;
#endif
}

//=======================================================================
//function : operator <<
//purpose  : 
//=======================================================================

ostream & operator <<(ostream & OS, const SMESH_Pattern::TPoint& p)
{
  gp_XYZ xyz = p.myInitXYZ;
  OS << "\tinit( xyz( " << xyz.X() << " " << xyz.Y() << " " << xyz.Z() << " )";
  gp_XY xy = p.myInitUV;
  OS << " uv( " <<  xy.X() << " " << xy.Y() << " )";
  double u = p.myInitU;
  OS << " u( " <<  u << " )) " << &p << endl;
  xyz = p.myXYZ.XYZ();
  OS << "\t    ( xyz( " << xyz.X() << " " << xyz.Y() << " " << xyz.Z() << " )";
  xy = p.myUV;
  OS << " uv( " <<  xy.X() << " " << xy.Y() << " )";
  u = p.myU;
  OS << " u( " <<  u << " ))" << endl;
  
  return OS;
}
