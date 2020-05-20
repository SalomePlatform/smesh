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
// File      : FrontTrack_Utils.hxx
// Created   : Tue Apr 25 17:23:33 2017
// Author    : Edward AGAPOV (eap)

#ifndef __FrontTrack_Utils_HXX__
#define __FrontTrack_Utils_HXX__

#include "FrontTrack_Projector.hxx"

#include <vector>
#include <string>
#include <map>

namespace XAO {
  class Xao;
  class Group;
}

namespace FT_Utils
{
  // Check if a file exists
  bool fileExists( const std::string& path );

  // Check if a file can be created/overwritten
  bool canWrite( const std::string& path );

  // Transform anything printable to a string
  template< typename T> std::string toStr( const T& t )
  {
    std::ostringstream s;
    s << t;
    return s.str();
  }

  //--------------------------------------------------------------------------------------------
  /*!
   * \brief Return projectors by group name
   */
  struct XaoGroups
  {
    XaoGroups( const XAO::Xao* xao );

    int getProjectors( const std::string&                   groupName,
                       const int                            dim,
                       const std::vector< FT_Projector > &  allProjectors,
                       std::vector< const FT_Projector* > & groupProjectors ) const;
  private:

    typedef std::multimap< std::string, XAO::Group* > TGroupByNameMap;
    TGroupByNameMap _xaoGroups[ 2 ]; // by dim
  };
}

#endif
