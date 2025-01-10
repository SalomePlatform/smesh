// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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

// SMESH SMESH_I : support of import / export with meshio library
// File   : SMESH_Meshio.h
// Author : Konstantin Leontev, Open CASCADE S.A.S.
//
#ifndef SMESH_MESHIO_H
#define SMESH_MESHIO_H

#include "SMESH.hxx"

#include <QString>


class SMESH_I_EXPORT SMESH_Meshio
{
public:
  SMESH_Meshio();
  SMESH_Meshio(const QString& selectedFilter);
  virtual ~SMESH_Meshio();

  QString CreateTempFileName(const QString& targetFileName);
  void Convert(const QString& sourceFileName, const QString& targetFileName) const;

  static QString GetMeshioVersion();
  static bool IsModernMeshioVersion();
  static bool IsMeshioInstalled();
  static bool IsModernPythonVersion();

private:
  void RemoveTempFile();
  void CreateErrorFileName();
  void RemoveErrorFile();
  std::string ReadErrorsFromFile() const;

  QString GetConvertOptArgs() const;

private:
  QString myTempFileName;
  QString myErrorFileName;
  QString mySelectedFilter;
};

#endif // SMESH_MESHIO_H
