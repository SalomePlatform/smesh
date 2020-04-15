// Copyright (C) 2007-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "DriverSTL_W_SMDS_Mesh.h"

#ifdef WIN32
#define NOMINMAX
#endif

#include <Basics_Utils.hxx>

#include "SMDS_FaceOfNodes.hxx"
#include "SMDS_IteratorOnIterators.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_PolygonalFaceOfNodes.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESH_File.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_TypeDefs.hxx"

#include <limits>


// definition des constantes 
static const int LABEL_SIZE = 80;

DriverSTL_W_SMDS_Mesh::DriverSTL_W_SMDS_Mesh()
{
  myIsAscii = false;
}

void DriverSTL_W_SMDS_Mesh::SetIsAscii( const bool theIsAscii )
{
  myIsAscii = theIsAscii;
}

Driver_Mesh::Status DriverSTL_W_SMDS_Mesh::Perform()
{
  Kernel_Utils::Localizer loc;

  Status aResult = DRS_OK;

  if ( !myMesh ) {
    fprintf(stderr, ">> ERROR : Mesh is null \n");
    return DRS_FAIL;
  }
  findVolumeTriangles();
  if ( myIsAscii )
    aResult = writeAscii();
  else
    aResult = writeBinary();

  return aResult;
}

//================================================================================
/*!
 * \brief Destructor deletes temporary faces
 */
//================================================================================

DriverSTL_W_SMDS_Mesh::~DriverSTL_W_SMDS_Mesh()
{
  for ( unsigned i = 0; i < myVolumeFacets.size(); ++i )
    delete myVolumeFacets[i];
}

//================================================================================
/*!
 * \brief Finds free facets of volumes for which faces are missing in the mesh
 */
//================================================================================

void DriverSTL_W_SMDS_Mesh::findVolumeTriangles()
{
  myNbVolumeTrias = 0;

  SMDS_VolumeTool theVolume;
  SMDS_VolumeIteratorPtr vIt = myMesh->volumesIterator();
  std::vector< const SMDS_MeshNode*> nodes;
  while ( vIt->more() )
  {
    theVolume.Set( vIt->next(), /*ignoreCentralNodes=*/false );
    for ( int iF = 0; iF < theVolume.NbFaces(); ++iF )
      if ( theVolume.IsFreeFace( iF ))
      {
        const SMDS_MeshNode** n = theVolume.GetFaceNodes(iF);
        int                 nbN = theVolume.NbFaceNodes(iF);
        nodes.assign( n, n+nbN );
        if ( !myMesh->FindElement( nodes, SMDSAbs_Face, /*noMedium=*/false))
        {
          if (( nbN == 9 || nbN == 7 ) &&
              ( !theVolume.IsPoly() )) // facet is bi-quaratic
          {
            int nbTria = nbN - 1;
            for ( int iT = 0; iT < nbTria; ++iT )
              myVolumeFacets.push_back( new SMDS_FaceOfNodes( n[8], n[0+iT], n[1+iT] ));
            myNbVolumeTrias += nbTria;
          }
          else
          {
            myVolumeFacets.push_back( new SMDS_PolygonalFaceOfNodes( nodes ));
            myNbVolumeTrias += nbN - 2;
          }
        }
      }
  }
}

//================================================================================
/*!
 * \brief Return iterator on both faces in the mesh and on temporary faces
 */
//================================================================================

SMDS_ElemIteratorPtr DriverSTL_W_SMDS_Mesh::getFaces() const
{
  SMDS_ElemIteratorPtr facesIter = myMesh->elementsIterator(SMDSAbs_Face);
  SMDS_ElemIteratorPtr tmpTriaIter( new SMDS_ElementVectorIterator( myVolumeFacets.begin(),
                                                                    myVolumeFacets.end()));
  typedef std::vector< SMDS_ElemIteratorPtr > TElemIterVector;
  TElemIterVector iters(2);
  iters[0] = facesIter;
  iters[1] = tmpTriaIter;
  
  typedef SMDS_IteratorOnIterators<const SMDS_MeshElement *, TElemIterVector> TItersIter;
  return SMDS_ElemIteratorPtr( new TItersIter( iters ));
}

// static methods

static void writeInteger( const Standard_Integer& theVal, SMESH_File& ofile )
{
  union {
    Standard_Integer i;
    char c[4];
  } u;

  u.i = theVal;

  Standard_Integer entier;
  entier  =  u.c[0] & 0xFF;
  entier |= (u.c[1] & 0xFF) << 0x08;
  entier |= (u.c[2] & 0xFF) << 0x10;
  entier |= (u.c[3] & 0xFF) << 0x18;

  ofile.write( entier );
}

static void writeFloat( const Standard_ShortReal& theVal, SMESH_File& ofile)
{
  union {
    Standard_ShortReal f;
    char c[4];
  } u;

  u.f = theVal;

  Standard_Integer entier;

  entier  =  u.c[0] & 0xFF;
  entier |= (u.c[1] & 0xFF) << 0x08;
  entier |= (u.c[2] & 0xFF) << 0x10;
  entier |= (u.c[3] & 0xFF) << 0x18;

  ofile.write( entier );
}

static gp_XYZ getNormale( const SMDS_MeshNode* n1,
                          const SMDS_MeshNode* n2,
                          const SMDS_MeshNode* n3)
{
  SMESH_TNodeXYZ xyz1( n1 );
  SMESH_TNodeXYZ xyz2( n2 );
  SMESH_TNodeXYZ xyz3( n3 );
  gp_XYZ q1 = xyz2 - xyz1;
  gp_XYZ q2 = xyz3 - xyz1;
  gp_XYZ n  = q1 ^ q2;
  double len = n.Modulus();
  if ( len > std::numeric_limits<double>::min() )
    n /= len;

  return n;
}

// private methods

Driver_Mesh::Status DriverSTL_W_SMDS_Mesh::writeAscii() const
{
  Status aResult = DRS_OK;
  if ( myFile.empty() ) {
    fprintf(stderr, ">> ERREOR : invalid file name \n");
    return DRS_FAIL;
  }

  SMESH_File aFile( myFile, /*openForReading=*/false );
  aFile.openForWriting();

  std::string buf("solid ");
  buf += myName + "\n";
  aFile.writeRaw( buf.c_str(), buf.size() );

  char sval[128];
  std::vector< const SMDS_MeshNode* > triaNodes;

  SMESH_MeshAlgos::Triangulate triangulator;

  SMDS_ElemIteratorPtr itFaces = getFaces();
  while ( itFaces->more() )
  {
    const SMDS_MeshElement* aFace = itFaces->next();
    int nbTria = triangulator.GetTriangles( aFace, triaNodes );

    for ( int iT = 0, iN = 0; iT < nbTria; ++iT )
    {
      gp_XYZ normale = getNormale( triaNodes[iN],
                                   triaNodes[iN+1],
                                   triaNodes[iN+2] );
      sprintf (sval,
               " facet normal % 12e % 12e % 12e\n"
               "   outer loop\n" ,
               normale.X(), normale.Y(), normale.Z());
      aFile.writeRaw ( sval, 70 + strlen( sval + 70 )); // at least 70 but can be more (WIN)

      for ( int jN = 0; jN < 3; ++jN, ++iN )
      {
        SMESH_TNodeXYZ node = triaNodes[iN];
        sprintf (sval,
                 "     vertex % 12e % 12e % 12e\n",
                 node.X(), node.Y(), node.Z() );
        aFile.writeRaw ( sval, 54 + strlen( sval + 54 ));
      }
      aFile.writeRaw ("   endloop\n"
                      " endfacet\n", 21 );
    }
  }
  buf = "endsolid " + myName + "\n";
  aFile.writeRaw( buf.c_str(), buf.size() );

  return aResult;
}

//================================================================================
/*!
 * \brief Writes all triangles in binary format
 *  \return Driver_Mesh::Status - DRS_FAIL if no file name is provided
 */
//================================================================================

Driver_Mesh::Status DriverSTL_W_SMDS_Mesh::writeBinary() const
{
  Status aResult = DRS_OK;

  if ( myFile.empty() ) {
    fprintf(stderr, ">> ERREOR : invalid filename \n");
    return DRS_FAIL;
  }

  SMESH_File aFile( myFile );
  aFile.openForWriting();

  SMESH_MeshAlgos::Triangulate triangulator;

  // we first count the number of triangles
  int nbTri = myNbVolumeTrias;
  {
    SMDS_FaceIteratorPtr itFaces = myMesh->facesIterator();
    while ( itFaces->more() ) {
      const SMDS_MeshElement* aFace = itFaces->next();
      nbTri += triangulator.GetNbTriangles( aFace );
    }
  }
  std::string sval( LABEL_SIZE, ' ' );
  if ( !myName.empty() )
  {
    sval = "name: " + myName;
    sval.resize( LABEL_SIZE, ' ' );
  }
  aFile.write( sval.c_str(), LABEL_SIZE );

  // write number of triangles
  writeInteger( nbTri, aFile );  

  // Loop writing nodes

  int dum=0;

  std::vector< const SMDS_MeshNode* > triaNodes;

  SMDS_ElemIteratorPtr itFaces = getFaces();
  while ( itFaces->more() )
  {
    const SMDS_MeshElement* aFace = itFaces->next();
    int nbTria = triangulator.GetTriangles( aFace, triaNodes );
    
    for ( int iT = 0, iN = 0; iT < nbTria; ++iT )
    {
      gp_XYZ normale = getNormale( triaNodes[iN],
                                   triaNodes[iN+1],
                                   triaNodes[iN+2] );
      writeFloat(normale.X(),aFile);
      writeFloat(normale.Y(),aFile);
      writeFloat(normale.Z(),aFile);

      for ( int jN = 0; jN < 3; ++jN, ++iN )
      {
        const SMDS_MeshNode* node = triaNodes[iN];
        writeFloat(node->X(),aFile);
        writeFloat(node->Y(),aFile);
        writeFloat(node->Z(),aFile);
      }
      aFile.writeRaw ( &dum, 2 );
    }
  }

  return aResult;
}
