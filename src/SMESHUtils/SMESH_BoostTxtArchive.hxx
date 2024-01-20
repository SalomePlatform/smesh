// Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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
// File      : SMESH_BoostTxtArchive.hxx
// Created   : Thu Aug  5 19:10:24 2021
// Author    : Edward AGAPOV (eap)


#ifndef __SMESH_BoostTxtArchive_HXX__
#define __SMESH_BoostTxtArchive_HXX__

#include "SMESH_Utils.hxx"

#include <boost/archive/text_iarchive.hpp>

namespace SMESHUtils
{
  /*!
   * \brief Load an object from a string created by boost::archive::text_oarchive.
   *
   * Try to workaround the issue that loading fails if the archive string
   * is created by a newer version of boost::archive library.
   *
   * Usage: ObjType obj;  SMESHUtils::BoostTxtArchive( arcString ) >> obj;
   */
  class SMESHUtils_EXPORT BoostTxtArchive
  {
  public:

    BoostTxtArchive( const std::string& s );
    BoostTxtArchive( std::istream& stream );
    ~BoostTxtArchive();

    template< class T >
      BoostTxtArchive & operator>>( T & t )
    {
      if ( myArchiveReader )
        try
        {
          (*myArchiveReader) >> t;
        }
        catch (...)
        {
          if ( fixString() )
            try
            {
              (*myArchiveReader) >> t;
            }
            catch(...)
            {
            }
        }
      return *this;
    }

  private:

    void makeReader();
    bool fixString();

    boost::archive::text_iarchive* myArchiveReader;
    std::string                    myString;        // archive to read
    bool                           myStringFixed;   // is archive version changed
    std::istream*                  myStream;        // stream holding myString
    bool                           myOwnStream;     // is myStream created by me

    // persistence used to create a current version archive
    friend class boost::serialization::access;
    template<class Archive>
      void serialize(Archive & /*ar*/, const unsigned int /*version*/)
    {
    }
  };
}


#endif
