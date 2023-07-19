// Copyright (C) 2007-2023  CEA, EDF, OPEN CASCADE
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
// File      : SMESH_BoostTxtArchive.cxx
// Created   : Thu Aug  5 21:19:31 2021
// Author    : Edward AGAPOV (eap)

#include "SMESH_BoostTxtArchive.hxx"

#include <cctype>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>


using namespace SMESHUtils;

BoostTxtArchive::BoostTxtArchive( const std::string& s ):
  myArchiveReader( nullptr ),
  myString( s ),
  myStringFixed( false ),
  myStream( nullptr ),
  myOwnStream( true )
{
  myStream = new std::istringstream( myString );
  makeReader();
}

BoostTxtArchive::BoostTxtArchive( std::istream& stream ):
  myArchiveReader( nullptr ),
  myStringFixed( false ),
  myStream( &stream ),
  myOwnStream( false )
{
  if ( std::istringstream * sstrm = dynamic_cast< std::istringstream* >( &stream ))
    myString = sstrm->str();

  makeReader();
}

BoostTxtArchive::~BoostTxtArchive()
{
  delete myArchiveReader; myArchiveReader = nullptr;
  if ( myOwnStream )
    delete myStream;
}

void BoostTxtArchive::makeReader()
{
  myArchiveReader = nullptr;
  try
  {
    myArchiveReader = new boost::archive::text_iarchive( *myStream );
  }
  catch (...)
  {
    if ( fixString() )
      try
      {
        myArchiveReader = new boost::archive::text_iarchive( *myStream );
      }
      catch(...)
      {
        myArchiveReader = nullptr;
      }
  }
}

namespace
{
  //================================================================================
  /*!
   * \brief Return archive created by current version of boost::archive
   */
  //================================================================================

  std::string getCurrentVersionArchive( BoostTxtArchive & bta)
  {
    std::ostringstream strm;
    boost::archive::text_oarchive archive( strm );
    archive << bta;
    return strm.str();
  }
}

//================================================================================
/*!
 * \brief Change boost::archive library version in myString to be equal to
 *        the current library version
 *  \return bool - return true if the version is changed
 */
//================================================================================

bool BoostTxtArchive::fixString()
{
  if ( myStringFixed )
    return false;
  myStringFixed = true;

  const char*     sub = "serialization::archive ";
  const size_t subLen = 23;

  size_t where1 = myString.find( sub );
  if ( where1 == std::string::npos )
    return false;

  std::string nowString = getCurrentVersionArchive( *this );
  size_t where2 = nowString.find( sub );
  if ( where2 == std::string::npos )
    return false;

  bool sameVersion = true;
  for ( size_t i1 = where1 + subLen, i2 = where2 + subLen;
        i2 < nowString.size();
        ++i1, ++i2 )
  {
    if ( myString[ i1 ] != nowString[ i2 ] )
    {
      sameVersion = false;
      myString[ i1 ] = nowString[ i2 ];
    }
    if ( isspace( myString[ i1 ]))
      break;
  }

  if ( !sameVersion )
  {
    if ( myOwnStream )
      delete myStream;
    myStream    = new std::istringstream( myString );
    myOwnStream = true;
  }

  return !sameVersion;
}
