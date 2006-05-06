#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
#  See http://www.salome-platform.org/
#
import salome
import geompy
import SMESH
import StdMeshers

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

box   = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
face   = subShapeList[0]
name   = geompy.SubShapeName(face, box)
idface = geompy.addToStudyInFather(box, face, name)

box  = salome.IDToObject(idbox)
face = salome.IDToObject(idface)

hyp1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hyp1.SetNumberOfSegments(10)
hyp2 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp2.SetMaxElementArea(10)
hyp3 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp3.SetMaxElementArea(100)

algo1 = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
algo2 = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")

mesh = smesh.CreateMesh(box)
mesh.AddHypothesis(box,hyp1)
mesh.AddHypothesis(box,hyp2)
mesh.AddHypothesis(box,algo1)
mesh.AddHypothesis(box,algo2)

submesh = mesh.GetSubMesh(face, "SubMeshFace")
mesh.AddHypothesis(face,hyp1)
mesh.AddHypothesis(face,hyp3)
mesh.AddHypothesis(face,algo1)
mesh.AddHypothesis(face,algo2)

smesh.Compute(mesh,box)

faces = submesh.GetElementsByType(SMESH.FACE)
if len(faces) > 1:
    print len(faces), len(faces)/2
    group1 = mesh.CreateGroup(SMESH.FACE,"Group of faces")
    group2 = mesh.CreateGroup(SMESH.FACE,"Another group of faces")
    group1.Add(faces[:int(len(faces)/2)])
    group2.Add(faces[int(len(faces)/2):])

salome.sg.updateObjBrowser(1)
