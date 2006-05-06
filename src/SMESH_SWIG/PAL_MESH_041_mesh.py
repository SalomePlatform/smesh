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
import geompy
import salome

import StdMeshers

#-----------------------------GEOM----------------------------------------

#----------Vertexes------------
p1 = geompy.MakeVertex(20.0,30.0,40.0)
p2 = geompy.MakeVertex(90.0,80.0,0.0)
p3 = geompy.MakeVertex(30.0,80.0,200.0)

#----------Edges---------------
e1 = geompy.MakeEdge(p1,p2)
e2 = geompy.MakeEdge(p2,p3)
e3 = geompy.MakeEdge(p3,p1)

#----------Wire----------------
ListOfEdges = []
ListOfEdges.append(e3)
ListOfEdges.append(e2)
ListOfEdges.append(e1)
wire1 = geompy.MakeWire(ListOfEdges)


#----------Face----------------
WantPlanarFace = 1
face1 = geompy.MakeFace(wire1,WantPlanarFace)

Id_face1 = geompy.addToStudy(face1,"Face1")



#-----------------------------SMESH-------------------------------------------
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

# -- Init --
plane_mesh = salome.IDToObject( Id_face1)
smesh.SetCurrentStudy(salome.myStudy)

mesh = smesh.CreateMesh(plane_mesh)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

id_mesh = salome.ObjectToID(mesh)
smeshgui.SetName( id_mesh, "Mesh_1")


print"---------------------Hypothesis"


#---------------- NumberOfSegments
numberOfSegment = 9

hypNbSeg = smesh.CreateHypothesis( "NumberOfSegments", "libStdMeshersEngine.so" )
hypNbSeg.SetNumberOfSegments( numberOfSegment )

print hypNbSeg.GetName()
print hypNbSeg.GetNumberOfSegments()
smeshgui.SetName(salome.ObjectToID(hypNbSeg), "Nb. Segments")


#--------------------------Max. Element Area
maxElementArea = 200

hypArea200 = smesh.CreateHypothesis("MaxElementArea","libStdMeshersEngine.so")
hypArea200.SetMaxElementArea( maxElementArea )
print hypArea200.GetName()
print hypArea200.GetMaxElementArea()

smeshgui.SetName(salome.ObjectToID(hypArea200), "Max. Element Area")

print"---------------------Algorithms"

#----------------------------Wire discretisation
algoWireDes = smesh.CreateHypothesis( "Regular_1D", "libStdMeshersEngine.so" )
listHyp = algoWireDes.GetCompatibleHypothesis()

print algoWireDes.GetName()
smeshgui.SetName(salome.ObjectToID(algoWireDes), "Ware descritisation")

#----------------------------Triangle (Mefisto)
algoMef = smesh.CreateHypothesis( "MEFISTO_2D", "libStdMeshersEngine.so" )
listHyp = algoMef.GetCompatibleHypothesis()

print algoMef.GetName()

#----------------------------Add hipothesis to the plane
mesh.AddHypothesis( plane_mesh, hypNbSeg )     # nb segments
mesh.AddHypothesis( plane_mesh, hypArea200 )   # max area

mesh.AddHypothesis( plane_mesh, algoWireDes )  # Regular 1D/wire discretisation
mesh.AddHypothesis( plane_mesh, algoMef )      # MEFISTO 2D

smeshgui.SetName(salome.ObjectToID(algoMef), "Triangle (Mefisto)")

print "---------------------Compute the mesh"

smesh.Compute(mesh, plane_mesh)

salome.sg.updateObjBrowser(1)

