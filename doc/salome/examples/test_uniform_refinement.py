#!/usr/bin/env python3

import salome
salome.salome_init_without_session()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
import SMESHHOMARD

smesh = smeshBuilder.New()

import os, inspect, tempfile, shutil

data_dir = os.path.abspath(os.path.dirname(inspect.getfile(lambda: None)))
working_dir = tempfile.mkdtemp()

input_med_1 = os.path.join (data_dir, "tutorial_4.00.med")
input_xao_1 = os.path.join (data_dir, "tutorial_4.xao")
output_med_1 = os.path.join (working_dir, "tutorial_4.00_Uniform_R.med")
log_file_1 = os.path.join (working_dir, "tutorial_4.00_Uniform_R.log")

# Case 1: input: med file
#         output: med file, log file, published mesh
if os.path.isfile(output_med_1):
  os.remove(output_med_1)
if os.path.isfile(log_file_1):
  os.remove(log_file_1)

cao_name = "CAO_PIQUAGE"
smeshhomard = smesh.Adaptation("Uniform")
smeshhomard.CreateBoundaryCAO(cao_name, input_xao_1)
smeshhomard.CreateCase("PIQUAGE", input_med_1, working_dir)
smeshhomard.AddBoundary(cao_name)
smeshhomard.SetConfType(0)
smeshhomard.SetKeepMedOUT(True)
smeshhomard.SetPublishMeshOUT(True)
smeshhomard.SetMeshNameOUT("PIQUAGE_Uniform_R_01")
smeshhomard.SetMeshFileOUT(output_med_1)
smeshhomard.SetKeepWorkingFiles(False)
smeshhomard.SetLogInFile(True)
smeshhomard.SetLogFile(log_file_1)
smeshhomard.SetRemoveLogOnSuccess(False)
smeshhomard.SetVerboseLevel(3)
smeshhomard.Compute()

if os.path.isfile(output_med_1):
  os.remove(output_med_1)
else:
  print("Test Uniform refinement Case 1: Error: no output med file")
  assert(False)

if os.path.isfile(log_file_1):
  os.remove(log_file_1)
else:
  print("Test Uniform refinement Case 1: Error: no log file")
  assert(False)

# Case 2: input: mesh, boundaries
#         output: published mesh
input_med_2 = os.path.join (data_dir, "tutorial_5.00.med")
input_fr    = os.path.join (data_dir, "tutorial_5.fr.med")
output_med_2 = os.path.join (working_dir, "tutorial_5.00_Uniform_R.med")
log_file_2 = os.path.join (working_dir, "tutorial_5.00_Uniform_R.log")

if os.path.isfile(output_med_2):
  os.remove(output_med_2)
if os.path.isfile(log_file_2):
  os.remove(log_file_2)

# prepare input mesh
([MAILL], status) = smesh.CreateMeshesFromMED( input_med_2 )

smeshhomard = smesh.Adaptation("Uniform")
smeshhomard.CreateBoundaryDi("Boun_5_1", "MAIL_EXT", input_fr)
smeshhomard.CreateCaseOnMesh("COEUR_2D", MAILL.GetMesh(), working_dir)
smeshhomard.AddBoundary("Boun_5_1")
smeshhomard.SetConfType(1)
smeshhomard.SetKeepMedOUT(False)
smeshhomard.SetPublishMeshOUT(True)
smeshhomard.SetMeshNameOUT("COEUR_2D_Uniform_R")
smeshhomard.SetMeshFileOUT(output_med_2)
smeshhomard.SetKeepWorkingFiles(False)
smeshhomard.SetLogInFile(True)
smeshhomard.SetLogFile(log_file_2)
smeshhomard.SetRemoveLogOnSuccess(True)
smeshhomard.SetVerboseLevel(0)
smeshhomard.Compute()

if os.path.isfile(output_med_2):
  print("Test Uniform refinement Case 2: Error: output med file has not been removed")
  assert(False)

if os.path.isfile(log_file_2):
  print("Test Uniform refinement Case 2: Error: log file has not been removed")
  assert(False)

shutil.rmtree(working_dir)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
