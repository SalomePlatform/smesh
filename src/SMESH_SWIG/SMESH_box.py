#==============================================================================
#  Info.
#  Bug (from script, bug)   : box.py, PAL5223
#  Modified                 : 25/11/2004
#  Author                   : Kovaltchuk Alexey
#  Project                  : PAL/SALOME
#==============================================================================
#
# Salome geometry and meshing for a box
#
import salome
from salome import sg
import geompy

import StdMeshers

# ---- launch GEOM

geom          = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
meshgenerator = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

###geom.GetCurrentStudy(salome.myStudy._get_StudyId())
meshgenerator.SetCurrentStudy(salome.myStudy)

# Plate

box    = geompy.MakeBox(0.,0.,0.,1.,1.,1.)
boxId  = geompy.addToStudy(box,"box")

# ---- launch SMESH

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)
# meshgenerator=smeshpy.smeshpy()


# Hypothesis

hypL1=meshgenerator.CreateHypothesis("LocalLength","libStdMeshersEngine.so")
hypL1.SetLength(0.25)
hypL1Id = salome.ObjectToID(hypL1) 
smeshgui.SetName(hypL1Id, "LocalLength")

# Algorithm

alg1D=meshgenerator.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
alg1DId = salome.ObjectToID(alg1D) 
smeshgui.SetName(alg1DId, "algo1D")

alg2D=meshgenerator.CreateHypothesis("Quadrangle_2D", "libStdMeshersEngine.so")
alg2DId = salome.ObjectToID(alg2D) 
smeshgui.SetName(alg2DId, "algo2D")

alg3D=meshgenerator.CreateHypothesis("Hexa_3D", "libStdMeshersEngine.so")
alg3DId = salome.ObjectToID(alg3D) 
smeshgui.SetName(alg3DId, "algo3D")
 
# ---- init a Mesh

box_mesh=meshgenerator.CreateMesh(box)
box_meshId = salome.ObjectToID(box_mesh)
smeshgui.SetName(box_meshId, "box_mesh")

# ---- set Hypothesis & Algorithm

box_mesh.AddHypothesis(box,alg1D)
box_mesh.AddHypothesis(box,alg2D)
box_mesh.AddHypothesis(box,alg3D)
box_mesh.AddHypothesis(box,hypL1)

meshgenerator.Compute(box_mesh,box)

sg.updateObjBrowser(1)
