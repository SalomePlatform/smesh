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
// File      : FrontTrack_Utils.cxx
// Created   : Tue Apr 25 17:28:58 2017
// Author    : Edward AGAPOV (eap)

#include "FrontTrack_Utils.hxx"

#include <XAO_Xao.hxx>
#include <XAO_Group.hxx>

#include <fcntl.h>
#include <boost/filesystem.hpp>

namespace boofs = boost::filesystem;

//================================================================================
/*
 * \brief Check if a file exists
 */
//================================================================================

bool FT_Utils::fileExists( const std::string& path )
{
  if ( path.empty() )
    return false;

  boost::system::error_code err;
  bool res = boofs::exists( path, err );

  return err ? false : res;
}

//================================================================================
/*!
 * \brief Check if a file can be created/overwritten
 */
//================================================================================

bool FT_Utils::canWrite( const std::string& path )
{
  if ( path.empty() )
    return false;

  bool can = false;
#ifdef WIN32

  HANDLE file = CreateFile( path.c_str(),           // name of the write
                            GENERIC_WRITE,          // open for writing
                            0,                      // do not share
                            NULL,                   // default security
                            OPEN_ALWAYS,            // CREATE NEW or OPEN EXISTING
                            FILE_ATTRIBUTE_NORMAL,  // normal file
                            NULL);                  // no attr. template
  can = ( file != INVALID_HANDLE_VALUE );
  CloseHandle( file );

#else

  int file = ::open( path.c_str(),
                     O_WRONLY | O_CREAT,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ); // rw-r--r--
  can = ( file >= 0 );

#endif

  return can;
}

//================================================================================
/*!
 * \brief Make a map of XAO groups
 */
//================================================================================

FT_Utils::XaoGroups::XaoGroups( const XAO::Xao* theXao )
{
  XAO::Xao* xao = const_cast< XAO::Xao* >( theXao );

  for ( int iG = 0; iG < theXao->countGroups(); ++iG )
  {
    XAO::Group* group = xao->getGroup( iG );

    if ( group->getDimension() == 1 )
      
      _xaoGroups[ 0 ].insert( std::make_pair( group->getName(), group ));

    else if ( group->getDimension() == 2 )

      _xaoGroups[ 1 ].insert( std::make_pair( group->getName(), group ));
  }
}

//================================================================================
/*!
 * \brief Return FT_Projector's by a group name
 *  \param [in] groupName - the group name
 *  \param [in] dim - the group dimension
 *  \param [in] allProjectors - the projector of all shapes of \a dim dimension
 *  \param [out] groupProjectors - projectors to shapes of the group
 *  \return int - number of found shapes
 */
//================================================================================

int FT_Utils::XaoGroups::getProjectors( const std::string&                   groupName,
                                        const int                            dim,
                                        const std::vector< FT_Projector > &  allProjectors,
                                        std::vector< const FT_Projector* > & groupProjectors) const
{
  // get namesake groups

  const TGroupByNameMap* groupMap = 0;
  if ( dim == 1 )
    groupMap = &_xaoGroups[ 0 ];
  else if ( dim == 2 )
    groupMap = &_xaoGroups[ 1 ];
  else
    return 0;

  TGroupByNameMap::const_iterator name2gr = groupMap->find( groupName );
  if ( name2gr == groupMap->end() )
    return 0;

  std::vector< XAO::Group* > groups;
  groups.push_back( name2gr->second );

  for ( ++name2gr; name2gr != groupMap->end(); ++name2gr )
  {
    if ( name2gr->second->getName() == groupName )
      groups.push_back( name2gr->second );
    else
      break;
  }

  // get projectors

  int nbFound = 0;
  for ( size_t i = 0; i < groups.size(); ++i )
  {
    // IDs in XAO correspond to indices of allProjectors
    std::set<int>::iterator id = groups[i]->begin(), end = groups[i]->end();
    for ( ; id != end; ++id, ++nbFound )
      if ( *id < (int) allProjectors.size() )
        groupProjectors.push_back ( & allProjectors[ *id ]);
  }

  return nbFound;
}
