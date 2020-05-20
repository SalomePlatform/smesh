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
//  File   : HOMARD_DriverTools.hxx
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)
//
// ----------------------------------------------------------------------------

#ifndef HOMARD_DRIVERTOOLS_H
#define HOMARD_DRIVERTOOLS_H

#include "HOMARD.hxx"

#include <string>

class HOMARD_Boundary;
class HOMARD_Cas;
class HOMARD_Iteration;
class HOMARD_Hypothesis;
class HOMARD_Zone;
class HOMARD_YACS;

namespace HOMARD
{
  //! persistence entity type
  typedef enum { Case, Zone, Hypothesis, Iteration, Boundary, YACS } SignatureType;

  //! get persistence signature
  HOMARDIMPL_EXPORT std::string GetSignature( SignatureType type );

  //! get data separator
  HOMARDIMPL_EXPORT std::string separator();

  //! dump boundary to the string
  HOMARDIMPL_EXPORT std::string Dump( const ::HOMARD_Boundary& boundary );
  //! dump case to the string
  HOMARDIMPL_EXPORT std::string Dump( const ::HOMARD_Cas& cas );
  //! dump iteration to the string
  HOMARDIMPL_EXPORT std::string Dump( const ::HOMARD_Iteration& iteration );
  //! dump hypothesis to the string
  HOMARDIMPL_EXPORT std::string Dump( const ::HOMARD_Hypothesis& hypothesis );
  //! dump zone to the string
  HOMARDIMPL_EXPORT std::string Dump( const ::HOMARD_Zone& zone );
  //! dump yacs to the string
  HOMARDIMPL_EXPORT std::string Dump( const ::HOMARD_YACS& yacs );

  //! restore boundary from the string
  HOMARDIMPL_EXPORT bool Restore( ::HOMARD_Boundary& boundary, const std::string& stream );
  //! restore case from the string
  HOMARDIMPL_EXPORT bool Restore( ::HOMARD_Cas& cas, const std::string& stream );
  //! restore hypothesis from the string
  HOMARDIMPL_EXPORT bool Restore( ::HOMARD_Hypothesis& hypothesis, const std::string& stream );
  //! restore iteration from the string
  HOMARDIMPL_EXPORT bool Restore( ::HOMARD_Iteration& iteration, const std::string& stream );
  //! restore zone from the string
  HOMARDIMPL_EXPORT bool Restore( ::HOMARD_Zone& zone, const std::string& stream );
  //! restore yacs from the string
  HOMARDIMPL_EXPORT bool Restore( ::HOMARD_YACS& yacs, const std::string& stream );
};

#endif // HOMARD_DRIVERTOOLS_H
