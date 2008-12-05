#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
import salome
import geompy
import smesh


# ---- GEOM

box   = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
face   = subShapeList[0]
name   = geompy.SubShapeName(face, box)
idface = geompy.addToStudyInFather(box, face, name)

box  = salome.IDToObject(idbox)
face = salome.IDToObject(idface)

# ---- SMESH

mesh = smesh.Mesh(box, "Meshbox")

# Set 1D algorithm/hypotheses to mesh
algo1 = mesh.Segment()
algo1.NumberOfSegments(10)

# Set 2D algorithm/hypotheses to mesh
algo2 = mesh.Triangle(smesh.MEFISTO)
algo2.MaxElementArea(10)

# Create submesh on face
algo3 = mesh.Segment(face)
algo3.NumberOfSegments(10)
algo4 = mesh.Triangle(smesh.MEFISTO, face)
algo4.MaxElementArea(100)
submesh = algo4.GetSubMesh()
smesh.SetName(submesh, "SubMeshFace")


mesh.Compute()

faces = submesh.GetElementsByType(smesh.FACE)
if len(faces) > 1:
    print len(faces), len(faces)/2
    group1 = mesh.CreateEmptyGroup(smesh.FACE,"Group of faces")
    group2 = mesh.CreateEmptyGroup(smesh.FACE,"Another group of faces")
    group1.Add(faces[:int(len(faces)/2)])
    group2.Add(faces[int(len(faces)/2):])

salome.sg.updateObjBrowser(1)
