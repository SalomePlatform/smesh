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

# Create box without one plane

box = geompy.MakeBox(0., 0., 0., 10., 20., 30.)
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])

FaceList  = []
for i in range( 5 ):
  FaceList.append( subShapeList[ i ] )

aComp = geompy.MakeCompound( FaceList )
aBox = geompy.Sew( aComp, 1. )
idbox = geompy.addToStudy( aBox, "box" )
  
aBox  = salome.IDToObject( idbox )

# Create mesh

hyp1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hyp1.SetNumberOfSegments(5)
hyp2 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp2.SetMaxElementArea(20)
hyp3 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp3.SetMaxElementArea(50)

algo1 = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
algo2 = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")

mesh = smesh.CreateMesh(aBox)
mesh.AddHypothesis(aBox,hyp1)
mesh.AddHypothesis(aBox,hyp2)
mesh.AddHypothesis(aBox,algo1)
mesh.AddHypothesis(aBox,algo2)

smesh.Compute(mesh,aBox)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);
smeshgui.SetName( salome.ObjectToID( mesh ), "Mesh_freebord" );

# Criterion : Free edges
aFilterMgr = smesh.CreateFilterManager()
aPredicate = aFilterMgr.CreateFreeBorders()
aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Free edges Nb = ", len( anIds )
for i in range( len( anIds ) ):
  print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.EDGE, "Free edges" )
aGroup.Add( anIds )


salome.sg.updateObjBrowser(1)
