//  HOMARD HOMARD : implementation of HOMARD idl descriptions
//
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
//  File   : HOMARD.hxx
//  Author : Gerald NICOLAS, EDF
//  Module : HOMARD

#ifndef _HOMARD_HXX_
#define _HOMARD_HXX_

// C'est le ASSERT de SALOMELocalTrace/utilities.h dans KERNEL
#ifndef VERIFICATION
#define VERIFICATION(condition) \
        if (!(condition)){INTERRUPTION("CONDITION "<<#condition<<" NOT VERIFIED")}
#endif /* VERIFICATION */

#ifdef WIN32
  #if defined HOMARDIMPL_EXPORTS || defined HOMARDImpl_EXPORTS
    #define HOMARDIMPL_EXPORT __declspec( dllexport )
  #else
    #define HOMARDIMPL_EXPORT __declspec( dllimport )
  #endif
#else
   #define HOMARDIMPL_EXPORT
#endif

// La gestion des repertoires
#ifndef CHDIR
  #ifdef WIN32
    #define CHDIR _chdir
  #else
    #define CHDIR chdir
  #endif
#endif

#endif
