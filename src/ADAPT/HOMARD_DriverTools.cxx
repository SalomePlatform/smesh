// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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
//  File   : HOMARD_DriverTools.cxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)
//
// ----------------------------------------------------------------------------

#include "HOMARD_DriverTools.hxx"
#include "HOMARD_Boundary.hxx"
#include "HOMARD_Cas.hxx"
#include "HOMARD_Hypothesis.hxx"
#include "HOMARD_Iteration.hxx"
#include "HOMARD_Zone.hxx"
#include "HOMARD_YACS.hxx"
#include <sstream>
#include <cstdlib>
#include "utilities.h"

namespace HOMARD
{

  std::string SEPARATOR = "|" ;

  /*!
    \brief Read next chunk of data from the string
    \internal

    The function tries to read next chunk of the data from the input string \a str.
    The parameter \a start specifies the start position of next chunk. If the operation
    read the chunk successfully, after its completion this parameter will refer to the
    start position of the next chunk. The function returns resulting chunk as a string.
    The status of the operation is returned via \a ok parameter.

    \param str source data stream string
    \param start start position to get next chunk
    \param ok in this variable the status of the chunk reading operation is returned
    \return next chunk read from the string
  */
  static std::string getNextChunk( const std::string& str, std::string::size_type& start, bool& ok )
  {
    std::string chunk = "";
    ok = false;
    if ( start <= str.size() ) {
      std::string::size_type end = str.find( separator(), start );
      chunk = str.substr( start, end == std::string::npos ? std::string::npos : end-start );
      start = end == std::string::npos ? str.size()+1 : end + separator().size();
      ok = true;
    }
    return chunk;
  }

  /*!
    \brief Get persistence signature
    \param type persistence entity type
    \return persistence signature
  */
  std::string GetSignature( SignatureType type )
  {
    std::string signature = "";
    switch ( type ) {
    case Case:       signature = "CASE"; break;
    case Zone:       signature = "ZONE"; break;
    case Hypothesis: signature = "HYPO"; break;
    case Iteration:  signature = "ITER"; break;
    case Boundary:   signature = "BOUNDARY"; break;
    case YACS:       signature = "YACS"; break;
    default: break;
    }
    signature += separator();
    return signature;
  }

  /*!
    \brief Get data separator
    \return string that is used to separate data entities in the stream
  */
  std::string separator()
  {
    return SEPARATOR ;
  }

// =======================
// 1.1. Case
// =======================
  /*!
    \brief Dump case to the string
    \param cas case being dumped
    \return string representation of the case
  */
  std::string Dump( const HOMARD_Cas& cas )
  {
    std::stringstream os;
    std::string saux ;
    // ...
    MESSAGE( ". Sauvegarde du cas "<<cas.GetName());
    os << cas.GetName();
    os << separator() << cas.GetDirName();
    os << separator() << cas.GetConfType();
    os << separator() << cas.GetExtType();

    std::vector<double> coor = cas.GetBoundingBox();
    os << separator() << coor.size();
    for ( int i = 0; i < coor.size(); i++ )
          os << separator() << coor[i];

    std::list<std::string> ListString = cas.GetIterations();
    os << separator() << ListString.size();
    std::list<std::string>::const_iterator it;
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    ListString = cas.GetGroups();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;
    ListString = cas.GetBoundaryGroup();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;

    os << separator() << cas.GetPyram();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// ==============
// 1.2. Iteration
// ==============
//
  /*!
    \brief Dump iteration to the string
    \param iteration iteration being dumped
    \return string representation of the iteration
  */
  std::string Dump( const HOMARD_Iteration& iteration )
  {
    std::stringstream os;
    std::string saux ;
    // ...
    MESSAGE( ". Sauvegarde de l'iteration "<<iteration.GetName());
    os << iteration.GetName();
    os << separator() << iteration.GetState();
    os << separator() << iteration.GetNumber();
    os << separator() << iteration.GetMeshFile();
    os << separator() << iteration.GetLogFile();
    os << separator() << iteration.GetMeshName();
    os << separator() << iteration.GetFieldFile();
    os << separator() << iteration.GetTimeStep();
    os << separator() << iteration.GetRank();
    os << separator() << iteration.GetIterParentName();
    //
    std::list<std::string> ListString = iteration.GetIterations();
    os << separator() << ListString.size();
    std::list<std::string>::const_iterator it;
    for ( it = ListString.begin(); it != ListString.end(); ++it )
      os << separator() << *it;

    os << separator() << iteration.GetHypoName();
    os << separator() << iteration.GetCaseName();
    os << separator() << iteration.GetDirNameLoc();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// ==============
// 1.3. hypothese
// ==============
  /*!
    \brief Dump hypothesis to the string
    \param hypothesis hypothesis being dumped
    \return string representation of the hypothesis
  */
  std::string Dump( const HOMARD_Hypothesis& hypothesis )
  {
    std::stringstream os;
    std::string saux ;
    // ...
    MESSAGE( ". Sauvegarde de l'hypothese "<<hypothesis.GetName());
    os << hypothesis.GetName();
    os << separator() << hypothesis.GetCaseCreation();
    os << separator() << hypothesis.GetAdapType();
    os << separator() << hypothesis.GetRefinType();
    os << separator() << hypothesis.GetUnRefType();
    os << separator() << hypothesis.GetFieldName();
    os << separator() << hypothesis.GetRefinThrType();
    os << separator() << hypothesis.GetThreshR();
    os << separator() << hypothesis.GetUnRefThrType();
    os << separator() << hypothesis.GetThreshC();
    os << separator() << hypothesis.GetUseField();
    os << separator() << hypothesis.GetUseComp();
    os << separator() << hypothesis.GetTypeFieldInterp();

    std::list<std::string> ListString = hypothesis.GetIterations();
    std::list<std::string>::const_iterator it;
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;

    ListString = hypothesis.GetZones();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    ListString = hypothesis.GetComps();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
         os << separator() << *it;

    ListString = hypothesis.GetGroups();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    ListString = hypothesis.GetFieldInterps();
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    os << separator() << hypothesis.GetNivMax();
    os << separator() << hypothesis.GetDiamMin();
    os << separator() << hypothesis.GetAdapInit();
    os << separator() << hypothesis.GetExtraOutput();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// =========
// 1.4. Zone
// =========

  /*!
    \brief Dump zone to the string
    \param zone zone being dumped
    \return string representation of the zone
  */
  std::string Dump( const HOMARD_Zone& zone )
  {
    std::stringstream os;
    std::string saux ;
    MESSAGE( ". Sauvegarde de la zone "<<zone.GetName());
    os << zone.GetName();
    os << separator() << zone.GetType();

    std::vector<double> coords = zone.GetCoords();
    for ( int i = 0; i < coords.size(); i++ )
      os << separator() << ( i < coords.size() ? coords[i] : 0. );

    std::vector<double> limit = zone.GetLimit();
    for ( int i = 0; i < 3; i++ )
      os << separator() << ( i < limit.size() ? limit[i] : 0. );

    std::list<std::string> hypos = zone.GetHypo();
    os << separator() << hypos.size();
    std::list<std::string>::const_iterator it;
    for ( it = hypos.begin(); it != hypos.end(); ++it )
      os << separator() << *it;

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// ==============================
// 1.5. Archivage d'une frontiere
// ==============================

  /*!
    \brief Dump boundary to the string
    \param boundary boundary being dumped
    \return string representation of the boundary
  */
  std::string Dump( const HOMARD_Boundary& boundary )
  {
    std::stringstream os;
    std::string saux ;
    MESSAGE( ". Sauvegarde de la frontiere "<<boundary.GetName());

    int BoundaryType = boundary.GetType() ;

    os << boundary.GetName() ;
    os << separator() << BoundaryType ;
    os << separator() << boundary.GetCaseCreation() ;

    if ( BoundaryType == -1 )
    {
      os << separator() << boundary.GetDataFile();
    }
    else if ( BoundaryType == 0 )
    {
      os << separator() << boundary.GetMeshName();
      os << separator() << boundary.GetDataFile();
    }
    else {
      std::vector<double> coor = boundary.GetCoords() ;
      for ( int i = 0; i < coor.size(); i++ )
            os << separator() << coor[i];
      std::vector<double> limit = boundary.GetLimit();
      for ( int i = 0; i < limit.size(); i++ )
            os << separator() << limit[i];
    }

    std::list<std::string> ListString = boundary.GetGroups();
    std::list<std::string>::const_iterator it;
    os << separator() << ListString.size();
    for ( it = ListString.begin(); it != ListString.end(); ++it )
          os << separator() << *it;

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }

//
// =========
// 1.6. YACS
// =========

  /*!
    \brief Dump YACS to the string
    \param yacs yacs being dumped
    \return string representation of the zone
  */
  std::string Dump( const HOMARD_YACS& yacs )
  {
    std::stringstream os;
    std::string saux ;
    MESSAGE( ". Sauvegarde du schema YACS "<<yacs.GetName());
    os << yacs.GetName();
    os << separator() << yacs.GetType();

    saux = os.str();
//     MESSAGE( ". Fin avec "<<saux);
    return saux ;
  }
//
// 2. Restauration des objets
// ==========================
// 2.1. Case
// ==========================
//
  /*!
    \brief Restore case from the string
    \param cas case being restored
    \param stream string representation of the case
    \return \c true if case is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Cas& cas, const std::string& stream )
  {
    MESSAGE( ". Restoration du cas ");
    std::string::size_type start = 0;
    std::string chunk, chunkNext;
    bool ok;
    // ...
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetDirName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetConfType( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetExtType( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;

    int size = atoi( chunk.c_str() );
    std::vector<double> boite;
    boite.resize( size );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boite[i] = strtod( chunk.c_str(), 0 );
    }
    cas.SetBoundingBox( boite );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;

    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      cas.AddIteration( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ )
    {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      cas.AddGroup( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      i++;
      chunkNext = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      cas.AddBoundaryGroup( chunk.c_str(), chunkNext.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    cas.SetPyram( atoi( chunk.c_str() ) );

    return true;
  }
//
// ==============
// 2.2. Iteration
// ==============
  /*!
    \brief Restore iteration from the string
    \param iteration iteration being restored
    \param stream string representation of the iteration
    \return \c true if iteration is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Iteration& iteration, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk;
    bool ok;
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;

    iteration.SetName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetState( atoi( chunk.c_str() ) );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetNumber( atoi( chunk.c_str() ) );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetMeshFile( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetLogFile( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetMeshName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetFieldFile( chunk.c_str() );
    // .
    int timestep, rank;
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    timestep = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    rank = atoi( chunk.c_str() );
    iteration.SetTimeStepRank( timestep, rank );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetIterParentName( chunk.c_str() );
    //
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      iteration.LinkNextIteration( chunk.c_str() );
    }
    //
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetHypoName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetCaseName( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    iteration.SetDirNameLoc( chunk.c_str() );
    return true;
  }

//
// ==============
// 2.3. hypothese
// ==============
  /*!
    \brief Restore hypothesis from the string
    \param hypothesis hypothesis being restored
    \param stream string representation of the hypothesis
    \return \c true if hypothesis is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Hypothesis& hypothesis, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk, chunkNext;
    bool ok;

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetCaseCreation( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetAdapType( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typeraff = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typedera = atoi( chunk.c_str() );
    hypothesis.SetRefinTypeDera( typeraff, typedera );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetField( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typethr = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    double threshr = strtod( chunk.c_str(), 0 );
    hypothesis.SetRefinThr( typethr, threshr );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int typethc = atoi( chunk.c_str() );
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    double threshc = strtod( chunk.c_str(), 0 );
    hypothesis.SetUnRefThr( typethc, threshc );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetUseField(atoi(chunk.c_str()));

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetUseComp(atoi(chunk.c_str()));

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetTypeFieldInterp(atoi(chunk.c_str()));

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      hypothesis.LinkIteration( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      i++;
      chunkNext = getNextChunk( stream, start, ok );
      int typeuse = atoi( chunkNext.c_str() );
      if ( !ok ) return false;
      hypothesis.AddZone( chunk.c_str(), typeuse );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      hypothesis.AddComp( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      hypothesis.AddGroup( chunk.c_str() );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      i++;
      chunkNext = getNextChunk( stream, start, ok );
      int TypeInterp = atoi( chunkNext.c_str() );
      if ( !ok ) return false;
      hypothesis.AddFieldInterpType( chunk.c_str(), TypeInterp );
    }

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetNivMax( atoi( chunk.c_str() ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetDiamMin( strtod( chunk.c_str(), 0 ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetAdapInit( strtod( chunk.c_str(), 0 ) );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    hypothesis.SetExtraOutput( strtod( chunk.c_str(), 0 ) );

    return true;
  }

//
// =========
// 2.4. Zone
// =========
  /*!
    \brief Restore zone from the string
    \param zone zone being restored
    \param stream string representation of the zone
    \return \c true if zone is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Zone& zone, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk;
    bool ok;
    //
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    zone.SetName( chunk.c_str() );
    //
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int ZoneType = atoi( chunk.c_str() ) ;
    zone.SetType( ZoneType );
    // Les coordonnees des zones : le nombre depend du type
    std::vector<double> coords;
    int lgcoords ;
    if ( ZoneType == 2 || ( ZoneType >= 11 && ZoneType <= 13 ) ) { lgcoords = 6 ; }
    else if ( ZoneType == 4 ) { lgcoords = 4 ; }
    else if ( ZoneType == 5 || ( ZoneType >= 31 && ZoneType <= 33 ) ) { lgcoords = 8 ; }
    else if ( ZoneType == 7 || ( ZoneType >= 61 && ZoneType <= 63 ) ) { lgcoords = 9 ; }
    else return false;
    coords.resize( lgcoords );
    for ( int i = 0; i < lgcoords; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      coords[i] = strtod( chunk.c_str(), 0 );
    }
    if ( ZoneType == 2 || ( ZoneType >= 11 && ZoneType <= 13 ) )
    { zone.SetBox( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5] ); }
    else if ( ZoneType == 4 )
    { zone.SetSphere( coords[0], coords[1], coords[2], coords[3] ); }
    else if ( ZoneType == 5 || ( ZoneType >= 31 && ZoneType <= 33 ) )
    { zone.SetCylinder( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6], coords[7] ); }
    else if ( ZoneType == 7 || ( ZoneType >= 61 && ZoneType <= 63 ) )
    { zone.SetPipe( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6], coords[7], coords[8] ); }
    // Remarque : la taille de coords est suffisante pour les limites
    for ( int i = 0; i < 3; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      coords[i] = strtod( chunk.c_str(), 0 );
    }
    zone.SetLimit( coords[0], coords[1], coords[2]);

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      zone.AddHypo( chunk.c_str() );
    }
    return true;
  }

//
// =================================
// 2.5. Restauration d'une frontiere
// =================================

  /*!
    \brief Restore boundary from the string
    \param boundary boundary being restored
    \param stream string representation of the boundary
    \return \c true if the boundary is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_Boundary& boundary, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk;
    bool ok;

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    boundary.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int BoundaryType = atoi( chunk.c_str() ) ;
    boundary.SetType( BoundaryType );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    boundary.SetCaseCreation( chunk.c_str() );

    // Si analytique, les coordonnees des frontieres : le nombre depend du type
    // Si discret, le maillage
    // Si CAO, la géométrie
    int lgcoords ;
    if ( BoundaryType == -1 ) { lgcoords = -1 ; }
    else if ( BoundaryType == 1 ) { lgcoords = 7 ; }
    else if ( BoundaryType == 2 ) { lgcoords = 4 ; }
    else { lgcoords = 0 ; }
//
    if ( lgcoords == -1 )
    {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.SetDataFile( chunk.c_str() );
    }
    else if ( lgcoords == 0 )
    {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.SetMeshName( chunk.c_str() );

      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.SetDataFile( chunk.c_str() );
    }
    else
    { std::vector<double> coords;
      coords.resize( lgcoords );
      for ( int i = 0; i < lgcoords; i++ ) {
        chunk = getNextChunk( stream, start, ok );
        if ( !ok ) return false;
        coords[i] = strtod( chunk.c_str(), 0 );
      }
      if ( BoundaryType == 1 )
      { boundary.SetCylinder(coords[0],coords[1],coords[2],coords[3],coords[4],coords[5],coords[6]); }
      else if ( BoundaryType == 2 )
      { boundary.SetSphere( coords[0], coords[1], coords[2], coords[3]); }
      else if ( BoundaryType == 3 )
      { boundary.SetConeA( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6]); }
      else if ( BoundaryType == 4 )
      { boundary.SetConeR( coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]); }
      // Remarque : la taille de coords est suffisante pour les limites
      for ( int i = 0; i < 3; i++ ) {
        chunk = getNextChunk( stream, start, ok );
        if ( !ok ) return false;
        coords[i] = strtod( chunk.c_str(), 0 );
      }
      boundary.SetLimit( coords[0], coords[1], coords[2]);
    }
    // Les groupes
    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int size = atoi( chunk.c_str() );
    for ( int i = 0; i < size; i++ ) {
      chunk = getNextChunk( stream, start, ok );
      if ( !ok ) return false;
      boundary.AddGroup( chunk.c_str() );
    }

    return true;
  }

//
// ==================================
// 2.6. Restauration d'un schema YACS
// ==================================

  /*!
    \brief Restore a schema YACS from the string
    \param yacs yacs being restored
    \param stream string representation of the schema yacs
    \return \c true if yacs is correctly restored or \c false otherwise
  */
  bool Restore( HOMARD_YACS& yacs, const std::string& stream )
  {
    std::string::size_type start = 0;
    std::string chunk;
    bool ok;

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    yacs.SetName( chunk.c_str() );

    chunk = getNextChunk( stream, start, ok );
    if ( !ok ) return false;
    int YACSType = atoi( chunk.c_str() ) ;
    yacs.SetType( YACSType );

    return true;
  }

} // namespace HOMARD /end/
