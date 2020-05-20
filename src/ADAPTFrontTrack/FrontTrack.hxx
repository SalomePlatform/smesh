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
// File      : FrontTrack.hxx
// Created   : Tue Apr 25 17:08:52 2017
// Author    : Edward AGAPOV (eap)


#ifndef __FrontTrack_HXX__
#define __FrontTrack_HXX__

#include <vector>
#include <string>

class FrontTrack
{
public:

  /*!
   * \brief Relocate nodes to lie on geometry
   *  \param [in] theInputMedFile - a MED file holding a mesh including nodes that will be
   *         moved onto the geometry
   *  \param [in] theOutputMedFile - a MED file to create, that will hold a modified mesh
   *  \param [in] theInputNodeFiles - an array of names of files describing groups of nodes that
   *         will be moved onto the geometry
   *  \param [in] theXaoFileName - a path to a file in XAO format  holding the geometry and
   *         the geometrical groups.
   *  \param [in] theIsParallel - if \c true, all processors are used to treat boundary shapes
   *          in parallel.
   */
  void track( const std::string&                 theInputMedFile,
              const std::string&                 theOutputMedFile,
              const std::vector< std::string > & theInputNodeFiles,
              const std::string&                 theXaoFileName,
              bool                               theIsParallel=true);

};

#endif
