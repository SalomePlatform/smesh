// Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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
// File      : FrontTrack.cxx
// Created   : Tue Apr 25 17:20:28 2017
// Author    : Edward AGAPOV (eap)

#include "FrontTrack.hxx"
#include "FrontTrack_NodeGroups.hxx"
#include "FrontTrack_Utils.hxx"

#include <MCAuto.hxx>
#include <MEDCouplingMemArray.hxx>
#include <MEDFileMesh.hxx>

#include <XAO_Xao.hxx>
#include <XAO_BrepGeometry.hxx>

#include <stdexcept>

#include <OSD_Parallel.hxx>

/*!
 * \brief Relocate nodes to lie on geometry
 *  \param [in] theInputMedFile - a MED file holding a mesh including nodes that will be
 *         moved onto the geometry
 *  \param [in] theOutputMedFile - a MED file to create, that will hold a modified mesh
 *  \param [in] theInputNodeFiles - an array of names of files describing groups of nodes that
 *         will be moved onto the geometry
 *  \param [in] theXaoFileName - a path to a file in XAO format holding the geometry and
 *         the geometrical groups.
 *  \param [in] theIsParallel - if \c true, all processors are used to treat boundary shapes
 *          in parallel.
 */
void FrontTrack::track( const std::string&                 theInputMedFile,
                        const std::string&                 theOutputMedFile,
                        const std::vector< std::string > & theInputNodeFiles,
                        const std::string&                 theXaoFileName,
                        bool                               theIsParallel )
{
  // check arguments
#ifdef _DEBUG_
  std::cout << "FrontTrack::track" << std::endl;
#endif

  if ( theInputNodeFiles.empty() )
    return;

#ifdef _DEBUG_
  std::cout << "Input MED file: " << theInputMedFile << std::endl;
#endif
  if ( !FT_Utils::fileExists( theInputMedFile ))
    throw std::invalid_argument( "Input MED file does not exist: " + theInputMedFile );

#ifdef _DEBUG_
  std::cout << "Output MED file: " << theOutputMedFile << std::endl;
#endif
  if ( !FT_Utils::canWrite( theOutputMedFile ))
    throw std::invalid_argument( "Can't create the output MED file: " + theOutputMedFile );

  std::vector< std::string > theNodeFiles ;
  for ( size_t i = 0; i < theInputNodeFiles.size(); ++i )
  {
#ifdef _DEBUG_
    std::cout << "Initial input node file #"<<i<<": " << theInputNodeFiles[i] << std::endl;
#endif
    if ( !FT_Utils::fileExists( theInputNodeFiles[i] ))
      throw std::invalid_argument( "Input node file does not exist: " + theInputNodeFiles[i] );
    // the name of the groupe on line #1, then the numbers of nodes on line #>1
    // keep only files with more than 1 line:
    std::ifstream fichier(theInputNodeFiles[i].c_str());
    std::string s;
    unsigned int nb_lines = 0;
    while(std::getline(fichier,s)) ++nb_lines;
//     std::cout << ". nb_lines: " << nb_lines << std::endl;
    if ( nb_lines >= 2 ) { theNodeFiles.push_back( theInputNodeFiles[i] ); }
  }
#ifdef _DEBUG_
  for ( size_t i = 0; i < theNodeFiles.size(); ++i )
  { std::cout << "Valid input node file #"<<i<<": " << theNodeFiles[i] << std::endl; }
#endif

#ifdef _DEBUG_
  std::cout << "XAO file: " << theXaoFileName << std::endl;
#endif
  if ( !FT_Utils::fileExists( theXaoFileName ))
    throw std::invalid_argument( "Input XAO file does not exist: " + theXaoFileName );

  // read a mesh

#ifdef _DEBUG_
  std::cout << "Lecture du maillage" << std::endl;
#endif
  MEDCoupling::MCAuto< MEDCoupling::MEDFileUMesh >
    mfMesh( MEDCoupling::MEDFileUMesh::New( theInputMedFile ));
  if ( mfMesh.isNull() )
    throw std::invalid_argument( "Failed to read the input MED file: " + theInputMedFile );

  MEDCoupling::DataArrayDouble * nodeCoords = mfMesh->getCoords();
  if ( !nodeCoords || nodeCoords->empty() )
    throw std::invalid_argument( "No nodes in the input mesh" );


  // read a geometry

#ifdef _DEBUG_
  std::cout << "Lecture de la geometrie" << std::endl;
#endif
  XAO::Xao xao;
  if ( !xao.importXAO( theXaoFileName ) || !xao.getGeometry() )
    throw std::invalid_argument( "Failed to read the XAO input file: " + theXaoFileName );

#ifdef _DEBUG_
  std::cout << "Conversion en BREP" << std::endl;
#endif
  XAO::BrepGeometry* xaoGeom = dynamic_cast<XAO::BrepGeometry*>( xao.getGeometry() );
  if ( !xaoGeom || xaoGeom->getTopoDS_Shape().IsNull() )
    throw std::invalid_argument( "Failed to get a BREP shape from the XAO input file" );


  // read groups of nodes and associate them with boundary shapes using names (no projection so far)

#ifdef _DEBUG_
  std::cout << "Lecture des groupes" << std::endl;
#endif
  FT_NodeGroups nodeGroups;
  nodeGroups.read( theNodeFiles, &xao, nodeCoords );
#ifdef _DEBUG_
  std::cout << "Nombre de groupes : " << nodeGroups.nbOfGroups() << std::endl;
#endif

  // project nodes to the boundary shapes and change their coordinates

#ifdef _DEBUG_
  std::cout << "Projection des noeuds, theIsParallel=" << theIsParallel << std::endl;
#endif
  OSD_Parallel::For( 0, nodeGroups.nbOfGroups(), nodeGroups, !theIsParallel );

  // save the modified mesh

#ifdef _DEBUG_
  std::cout << "Ecriture du maillage" << std::endl;
#endif
  const int erase = 2;
  mfMesh->write( theOutputMedFile, /*mode=*/erase );

  if ( !nodeGroups.isOK() )
    throw std::runtime_error("Unable to project some nodes");
}
