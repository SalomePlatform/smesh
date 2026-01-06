// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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

// SMESH SMESHGUI : support of import / export with meshio library
// File   : SMESHGUI_Meshio.h
// Author : Konstantin Leontev, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_MESHIO_H
#define SMESHGUI_MESHIO_H

#include "SMESH_SMESHGUI.hxx"

#include <QList>
#include <QPair>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Gen)

class QStringList;

class SMESHGUI_EXPORT SMESHGUI_Meshio
{
  using meshList = QList<QPair<SMESH::SMESH_IDSource_var, QString>>;

public:
  SMESHGUI_Meshio();
  virtual ~SMESHGUI_Meshio();

  static void ExportMesh(
    const meshList& aMeshList, const QString& targetFileName, const QString& selectedFilter);
  static SMESH::mesh_array_var ImportMesh(
    SMESH::SMESH_Gen_ptr theComponentMesh, const QString& filename, QStringList& errors);

  static const QStringList& GetImportFileFilter();
  static const QStringList& GetExportFileFilter();

  static QString GetFileName(QString& selectedFilter, const bool isOpen = false);
  static bool CheckMeshCount(const meshList& aMeshList);

  static bool IsMeshioInstalled();
};

#endif // SMESHGUI_MESHIO_H
