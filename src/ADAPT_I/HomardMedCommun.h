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

#ifndef HOMARDMEDCOMMUN_H
#define HOMARDMEDCOMMUN_H

#include "HOMARD_i.hxx"
#include <vector>
#include <set>
#include <string>

HOMARDENGINE_EXPORT int                    MEDFileExist( const char * aFile );
HOMARDENGINE_EXPORT std::vector<double>    GetBoundingBoxInMedFile( const char * aFile);
HOMARDENGINE_EXPORT std::set<std::string>  GetListeGroupesInMedFile(const char * aFile);

#endif // HOMARD%MEDCOMMUN_H
