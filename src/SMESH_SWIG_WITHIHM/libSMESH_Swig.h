// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

// SMESH SMESH : GUI for SMESH component
// File   : libSMESH_Swig.h
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
//
#ifndef LIBSMESH_SWIG_H
#define LIBSMESH_SWIG_H

#ifdef WNT
#if defined SMESH_SWIG_EXPORTS || defined _libSMESH_Swig_EXPORTS
  #define SMESH_SWIG_EXPORT __declspec( dllexport )
 #else
  #define SMESH_SWIG_EXPORT __declspec( dllimport )
 #endif
#else
 #define SMESH_SWIG_EXPORT
#endif

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SALOMEDS)

class SMESH_SWIG_EXPORT SMESH_Swig
{
public:
  SMESH_Swig();
  ~SMESH_Swig();
    
  void                       Init( int );

  const char*                AddNewMesh( const char* );

  const char*                AddNewHypothesis( const char* );
  const char*                AddNewAlgorithms( const char* );

  void                       SetShape( const char*, const char* );

  void                       SetHypothesis( const char*, const char* );
  void                       SetAlgorithms( const char*, const char* );

  void                       UnSetHypothesis( const char* );

  const char*                AddSubMesh( const char*, const char*, int );
  const char*                AddSubMeshOnShape( const char*, const char*, const char*, int );

  void                       CreateAndDisplayActor( const char* );

  void                       SetName( const char*, const char* );

  /*!
   * \brief Set mesh icon according to compute status
    * \param Mesh_Entry - entry of a mesh
    * \param isComputed - is mesh computed or not
   */
  void                       SetMeshIcon( const char*, const bool, const bool );

private:
  SALOMEDS::Study_var        myStudy;
  SALOMEDS::StudyBuilder_var myStudyBuilder;
  SALOMEDS::SComponent_var   mySComponentMesh;
};

#endif // LIBSMESH_SWIG_H
