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
#include "SMESHGUI_Meshio.h"
#include "SMESH_Meshio.h"

// SMESH includes
#include "SMESHGUI.h"
#include "SMESHGUI_FieldSelectorWdg.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_FileDlg.h>
#include <SUIT_MessageBox.h>

// SALOME KERNEL includes
#include <utilities.h>

// Qt
#include <QStringList>


/*!
  Constructor
*/
SMESHGUI_Meshio::SMESHGUI_Meshio()
{
}

/*!
  Destructor
*/
SMESHGUI_Meshio::~SMESHGUI_Meshio()
{
}

/*!
  Check and warn about exporting many shapes 
*/
bool SMESHGUI_Meshio::CheckMeshCount(const meshList& aMeshList)
{
  if (aMeshList.size() == 1)
  {
    return true;
  }
  else if (!aMeshList.size())
  {
    // We shouldn't get here, but...
    MESSAGE("Error: empty mesh list. Export canceled.");
    return false;
  }

  const bool isOk = SUIT_MessageBox::warning(
    SMESHGUI::desktop(),
    QObject::tr("SMESH_WARNING"),
    QObject::tr("SMESH_EXPORT_MESHIO_ONLY_MESH"),
    SUIT_MessageBox::Yes |
    SUIT_MessageBox::No,
    SUIT_MessageBox::No) == SUIT_MessageBox::Yes;

  return isOk;
}

/*!
  Import mesh through an intermediate MED file
*/
SMESH::mesh_array_var SMESHGUI_Meshio::ImportMesh(
  SMESH::SMESH_Gen_ptr theComponentMesh, const QString& filename, QStringList& errors)
{
  SMESH::DriverMED_ReadStatus res;
  SMESH::mesh_array_var aMeshes = theComponentMesh->CreateMeshesFromMESHIO(filename.toUtf8().constData(), res);
  if (res != SMESH::DRS_OK)
  {
    errors.append(QString("%1 :\n\t%2").arg(filename).arg(
      QObject::tr(QString("SMESH_DRS_%1").arg(res).toLatin1().data())));
  }

  return aMeshes;
}

/*!
  Returns a filter for Import File dialog
*/
const QStringList& SMESHGUI_Meshio::GetImportFileFilter()
{
  auto addAllFiles = []() -> QStringList
  {
    QStringList filter = GetExportFileFilter();

    // Remove SVG because it works only for export
    const int svgIndex = filter.indexOf(QRegExp("^SVG.+"));
    if (svgIndex >= 0)
    {
      filter.removeAt(svgIndex);
    }

    filter << QObject::tr("ALL_FILES_FILTER") + " (*)";

    return filter;
  };

  static const QStringList filter = addAllFiles();
  return filter;
}

/*!
  Returns a filter for Export File dialog
*/
const QStringList& SMESHGUI_Meshio::GetExportFileFilter()
{
  static const QStringList filter = {
    "Abaqus (*.inp)",
    "ANSYS msh (*.msh)",
    "AVS-UCD (*.avs)",
    "CGNS (*.cgns)",
    "DOLFIN XML (*.xml)",
#if !defined(WIN32)
    "Exodus (*.e *.exo)",
#endif
    "FLAC3D (*.f3grid)",
    "Gmsh 2.2 (*.msh)",
    "Gmsh 4.0 (*.msh)",
    //"Gmsh 4.1 (*.msh)",
    "H5M (*.h5m)",
    "Kratos/MDPA (*.mdpa)",
    "MED/Salome (*.med)",
    "Medit (*.mesh *.meshb)",
    "Nastran (*.bdf *.fem *.nas)",
    "Netgen(*.vol *.vol.gz)",
    "OBJ (*.obj)",
    "OFF (*.off)",
    "PERMAS (*.post *.post.gz *.dato *.dato.gz)",
    "PLY (*.ply)",
    "STL (*.stl)",
    "SU2 (*.su2)",
    "SVG, 2D output only (*.svg)",
    "Tecplot (*.dat)",
    "TetGen (*.node *.ele)",
    "UGRID (*.ugrid)",
    "VTK (*.vtk)",
    "VTU (*.vtu)",
    "WKT, TIN (*.wkt)",
    "XDMF (*.xdmf *.xmf)"
  };

  return filter;
}

/*!
  Export mesh through an intermediate MED file
*/
void SMESHGUI_Meshio::ExportMesh(const meshList& aMeshList, const QString& targetFileName, const QString& selectedFilter)
{
  // Helper for indexed naming of the target files.
  // We need to save into separated files because meshio doesn't
  // support reading more than one mesh from a MED file.
  // Look at src/meshio/med/_med.py in meshio git repo for a reference.
  auto indexedFileName = [](const QString& targetFileName, const int index) -> QString
  {
    QString indexedFileName = targetFileName;

    const int lastIndex = indexedFileName.lastIndexOf(".");
    indexedFileName.insert(lastIndex, "_" + QString::number(index));

    return indexedFileName;
  };

  // Trim an extension from the filter like in example: 'VTK (.vtk)' => 'VTK'
  auto getFilterWithoutExt = [](const QString& selectedFilter) -> QString
  {
    // Find the start index for an extension in the filter string
    const int index = selectedFilter.indexOf('(');
    if (index != -1)
    {
      const QString filterWithoutExt = selectedFilter.left(index);
      return filterWithoutExt.trimmed();
    }

    return selectedFilter;
  };

  // Iterate all the meshes from a list
  auto aMeshIter = aMeshList.begin();
  for(int aMeshIndex = 0; aMeshIter != aMeshList.end(); aMeshIter++, aMeshIndex++)
  {
    SMESH::SMESH_IDSource_var aMeshOrGroup = (*aMeshIter).first;
    SMESH::SMESH_Mesh_var        aMeshItem = aMeshOrGroup->GetMesh();

    // Exprort this part.
    aMeshItem->ExportPartToMESHIO(
      aMeshOrGroup, // mesh part
      (aMeshIndex ? indexedFileName(targetFileName, aMeshIndex) : targetFileName).toUtf8().data(),
      getFilterWithoutExt(selectedFilter).toLatin1().data()
    );
  }
}

/*!
  Opens file dialog and returns a chosen target name
*/
QString SMESHGUI_Meshio::GetFileName(QString& selectedFilter, const bool isOpen/* = false*/)
{
  // Get a target directory name
  QString anInitialPath = SUIT_FileDlg::getLastVisitedPath();
  if (anInitialPath.isEmpty())
    anInitialPath = QDir::currentPath();

  // Return a target file name
  return SUIT_FileDlg::getFileName(
    SMESHGUI::desktop(),
    anInitialPath,
    GetExportFileFilter(),
    selectedFilter,
    QObject::tr("SMESH_EXPORT_MESH"),
    isOpen
  );
}

/*!
  Returns true if meshio package is installed
*/
bool SMESHGUI_Meshio::IsMeshioInstalled()
{
  const bool isInstalled = SMESH_Meshio::IsMeshioInstalled();
  if (!isInstalled)
  {
    SUIT_MessageBox::warning(
      SMESHGUI::desktop(),
      QObject::tr("SMESH_WARNING"),
      QObject::tr("SMESH_MESHIO_NOT_INSTALLED")
    );
  }

  return isInstalled;
}
