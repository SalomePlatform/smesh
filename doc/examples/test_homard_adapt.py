import inspect
import os
import os.path as osp
import shutil
import tempfile

from salome.kernel import salome
salome.salome_init_without_session()

from salome.smesh import smeshBuilder
from salome.kernel import SMESHHOMARD

smesh = smeshBuilder.New()

data_dir = osp.abspath(osp.join(osp.dirname(inspect.getfile(lambda: None)), 'data'))
working_dir = tempfile.mkdtemp()

input_med = osp.join (data_dir, "test_homard_adapt.med")
output_med_1 = osp.join (working_dir, "test_1.00_Uniform_R_01.med")
output_med_2 = osp.join (working_dir, "test_1.00_Uniform_R_02.med")
log_file_1 = osp.join (working_dir, "test_1.00_Uniform_R_01.med.log")
log_file_2 = osp.join (working_dir, "test_1.00_Uniform_R_02.med.log")

# Case 1: input: med file
#         output: med file, log file, published mesh
if osp.isfile(output_med_1):
  os.remove(output_med_1)
if osp.isfile(log_file_1):
  os.remove(log_file_1)

smeshhomard = smesh.Adaptation("Uniform")
smeshhomard.CreateCase("MAILL", input_med, working_dir)
smeshhomard.SetConfType(0)
smeshhomard.SetKeepMedOUT(True)
smeshhomard.SetPublishMeshOUT(True)
smeshhomard.SetMeshNameOUT("MAILL_Uniform_R_01")
smeshhomard.SetMeshFileOUT(output_med_1)
smeshhomard.SetKeepWorkingFiles(False)
smeshhomard.SetLogInFile(True)
smeshhomard.SetLogFile(log_file_1)
smeshhomard.SetRemoveLogOnSuccess(False)
smeshhomard.SetVerboseLevel(3)
if smeshhomard.Compute() != 0: raise Exception("Error when computing Mesh")

if osp.isfile(output_med_1):
  os.remove(output_med_1)
else:
  print("Test Uniform refinement Case 1: Error: no output med file")
  assert(False)

if osp.isfile(log_file_1):
  os.remove(log_file_1)
else:
  print("Test Uniform refinement Case 1: Error: no log file")
  assert(False)

# Case 2: input: mesh, boundaries
#         output: published mesh
if osp.isfile(output_med_2):
  os.remove(output_med_2)
if osp.isfile(log_file_2):
  os.remove(log_file_2)

# prepare input mesh
([MAILL], status) = smesh.CreateMeshesFromMED( input_med )

#smeshhomard = smesh.Adaptation("Uniform")
Boun_1 = smeshhomard.CreateBoundaryCylinder("Boun_1", 0.5, 0.5, 0.5, 0, 0, 1, 0.25)
smeshhomard.CreateCaseOnMesh("MAILL", MAILL.GetMesh(), working_dir)
smeshhomard.SetConfType(0)
smeshhomard.AddBoundaryGroup("Boun_1", "BORD_EXT")
smeshhomard.AddBoundaryGroup("Boun_1", "MOITIE1")
smeshhomard.SetKeepMedOUT(False)
smeshhomard.SetPublishMeshOUT(True)
smeshhomard.SetMeshNameOUT("MAILL_Uniform_R_02")
smeshhomard.SetMeshFileOUT(output_med_2)
smeshhomard.SetKeepWorkingFiles(False)
smeshhomard.SetLogInFile(True)
smeshhomard.SetLogFile(log_file_2)
smeshhomard.SetRemoveLogOnSuccess(True)
smeshhomard.SetVerboseLevel(0)
if smeshhomard.Compute() != 0: raise Exception("Error when computing Mesh")

if osp.isfile(output_med_2):
  print("Test Uniform refinement Case 2: Error: output med file has not been removed")
  assert(False)

if osp.isfile(log_file_2):
  print("Test Uniform refinement Case 2: Error: log file has not been removed")
  assert(False)

shutil.rmtree(working_dir)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
