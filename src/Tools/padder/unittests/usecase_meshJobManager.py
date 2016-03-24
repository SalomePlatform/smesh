# -*- coding: iso-8859-1 -*-
# Copyright (C) 2011-2016  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# Author(s): Guillaume Boulant (23/03/2011)
#

# This script illustrates the standard use case of the component
# MeshJobManager from within a SALOME script. It could be used as a
# unit test of the component. The typical procedure is:
# $ <appli>/runAppli -t
# $ <appli>/runSession </path/to>/usecase_meshJobManager.py

#
# =======================================================================
# Preparing the configuration parameters
# =======================================================================
#
import sys
import os
import time
from salome.smesh.spadder.configreader import ConfigReader, printConfig, getPadderTestDir

configReader = ConfigReader()
defaultConfig = configReader.getDefaultConfig()
printConfig(defaultConfig)

from salome.smesh import spadder

import salome
import MESHJOB

#
# Setup the configuration in the component. We first have to load the
# catalog of SPADDER components, then load the component
# MeshJobManager, and finally configure this component.
#
spadder.loadSpadderCatalog()

salome.salome_init()
component = salome.lcc.FindOrLoadComponent("FactoryServer","MeshJobManager")
config = MESHJOB.ConfigParameter(resname=defaultConfig.resname,
                                 binpath=defaultConfig.binpath,
                                 envpath=defaultConfig.envpath)

configId = "localhost"
component.configure(configId,config)


#
# =======================================================================
# Define several datasets for the different use cases
# =======================================================================
#

# We define several functions that create each a dataset of med files
# for testing the component. The test function number corresponds to
# the number of the test defined in the SpherePadder installation
# directory.
PADDERTESTDIR = getPadderTestDir(defaultConfig)
#PADDERTESTDIR = spadder.getTestPadderDataDir()
#
# WARN: the above instruction (spadder.getTestPadderDataDir())
# localizes the PADDERTEST DIR using the PADDERDIR shell variable,
# while the previous one (getPadderTestDir) localizes this directory
# from data of the config (read from the configuration file
# padder.cfg).
#
def test00_parameters():
    """Test using a concrete mesh and a single steelbar mesh""" 
    file_concrete=os.path.join(spadder.getTestDataDir(),"concrete.med")
    file_steelbar=os.path.join(spadder.getTestDataDir(),"ferraill.med")

    meshJobParameterList = []
    param = MESHJOB.MeshJobParameter(file_name=file_concrete,
                                     file_type=MESHJOB.MED_CONCRETE,
                                     group_name="concrete")
    meshJobParameterList.append(param)

    param = MESHJOB.MeshJobParameter(file_name=file_steelbar,
                                     file_type=MESHJOB.MED_STEELBAR,
                                     group_name="steelbar")
    meshJobParameterList.append(param)
    return meshJobParameterList

def test01_parameters():
    """One concrete mesh and two steelbar meshes"""
    datadir = os.path.join(PADDERTESTDIR,"test01")
    meshJobParameterList = []

    medfile = os.path.join(datadir,"concrete.med")
    param = MESHJOB.MeshJobParameter(file_name=medfile,
                                     file_type=MESHJOB.MED_CONCRETE,
                                     group_name="concrete")
    meshJobParameterList.append(param)
    
    medfile = os.path.join(datadir,"ferraill.med")
    param = MESHJOB.MeshJobParameter(file_name=medfile,
                                     file_type=MESHJOB.MED_STEELBAR,
                                     group_name="ferraill")
    meshJobParameterList.append(param)

    medfile = os.path.join(datadir,"ferrtran.med")
    param = MESHJOB.MeshJobParameter(file_name=medfile,
                                     file_type=MESHJOB.MED_STEELBAR,
                                     group_name="ferrtran")
    meshJobParameterList.append(param)
    
    return meshJobParameterList

def test02_parameters():
    """One steelbar mesh only, without a concrete mesh"""
    datadir = os.path.join(PADDERTESTDIR,"test02")
    meshJobParameterList = []

    medfile = os.path.join(datadir,"cadreef.med")
    param = MESHJOB.MeshJobParameter(file_name=medfile,
                                     file_type=MESHJOB.MED_STEELBAR,
                                     group_name="cadre")
    meshJobParameterList.append(param)
    return meshJobParameterList

def test03_parameters():
    """One concrete mesh only, without a steelbar mesh"""
    datadir = os.path.join(PADDERTESTDIR,"test03")
    meshJobParameterList = []

    medfile = os.path.join(datadir,"concrete.med")
    param = MESHJOB.MeshJobParameter(file_name=medfile,
                                     file_type=MESHJOB.MED_CONCRETE,
                                     group_name="concrete")
    meshJobParameterList.append(param)
    return meshJobParameterList

#
# =======================================================================
# Prepare the job parameters and initialize the job
# =======================================================================
#

# Choose here the use case
#meshJobParameterList = test00_parameters()
#meshJobParameterList = test01_parameters()
#meshJobParameterList = test02_parameters()
meshJobParameterList = test03_parameters()

#
# Prepare, start and follow-up the job
#
jobid = component.initialize(meshJobParameterList, configId)
if jobid<0:
    msg = component.getLastErrorMessage()
    print "ERR: %s"%msg
    sys.exit(1)
    
created = False
nbiter  = 0
while not created:
    state = component.getState(jobid)
    print "MeshJobManager ["+str(nbiter)+"] : state = "+str(state)
    if state == "CREATED":
        created = True
    time.sleep(0.5)
    nbiter+=1


#
# =======================================================================
# Submit the job and start the supervision
# =======================================================================
#
# Start the execution of the job identified by its job id.
#
ok=component.start(jobid)
if not ok:
    msg = component.getLastErrorMessage()
    print "ERR: %s"%msg
    sys.exit(1)

print "job started: %s"%ok

#
# This part illustrates how you can follow the execution of the job.
#
run_states = ["CREATED", "IN_PROCESS", "QUEUED", "RUNNING", "PAUSED"];
end_states = ["FINISHED", "ERROR"]
all_states = run_states+end_states;

ended  = False
nbiter = 0
while not ended:
    state = component.getState(jobid)
    print "MeshJobManager ["+str(nbiter)+"] : state = "+str(state)
    if state not in run_states:
        ended=True
    time.sleep(0.5)
    nbiter+=1
        
if state not in end_states:
    print "ERR: jobid = "+str(jobid)+" ended abnormally with state="+str(state)
    msg = component.getLastErrorMessage()
    print "ERR: %s"%msg    
else:
    print "OK:  jobid = "+str(jobid)+" ended with state="+str(state)
    meshJobResults = component.finalize(jobid)
    print meshJobResults
    if meshJobResults.status is not True:
        print "ERR: the results are not OK: %s"%component.getLastErrorMessage()
        print "ERR: see log files in %s"%meshJobResults.results_dirname
