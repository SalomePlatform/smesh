# -*- coding: iso-8859-1 -*-
# Copyright (C) 2011-2012  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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
# MeshJobManager from within a SALOME script.


#
# Preparing the configuration parameters
#
import os
from salome.smesh.spadder.configreader import ConfigReader, printConfig

configReader = ConfigReader()
defaultConfig = configReader.getDefaultConfig()
printConfig(defaultConfig)

from salome.smesh import spadder
file_concrete=os.path.join(spadder.getTestDataDir(),"concrete.med")
file_steelbar=os.path.join(spadder.getTestDataDir(),"ferraill.med")

import salome
import MESHJOB

#
# Setup the configuration in the component. When first have to load
# the catalog of SPADDER components, then load the component
# MeshJobManager, and finally configure this component.
#
spadder.loadSpadderCatalog()

salome.salome_init()
component = salome.lcc.FindOrLoadComponent("FactoryServer","MeshJobManager")
config = MESHJOB.ConfigParameter(resname=defaultConfig.resname,
                                 binpath=defaultConfig.binpath,
                                 envpath=defaultConfig.envpath)
component.configure("localhost",config)

#
# Prepare the job parameters and initialize the job
#
meshJobParameterList = []
param = MESHJOB.MeshJobParameter(file_name=file_concrete,
                                 file_type=MESHJOB.MED_CONCRETE,
                                 group_name="concrete")
meshJobParameterList.append(param)

param = MESHJOB.MeshJobParameter(file_name=file_steelbar,
                                 file_type=MESHJOB.MED_STEELBAR,
                                 group_name="steelbar")
meshJobParameterList.append(param)
jobid = component.initialize(meshJobParameterList, "localhost")

#
# Start the execution of the job identified by its job id.
#
ok=component.start(jobid)

#
# This part illustrates how you can follow the execution of the job.
#
run_states = ["CREATED", "IN_PROCESS", "QUEUED", "RUNNING", "PAUSED"];
end_states = ["FINISHED", "ERROR"]
all_states = run_states+end_states;

ended  = False
nbiter = 0
import time
while not ended:
    state = component.getState(jobid)
    print "MeshJobManager ["+str(nbiter)+"] : state = "+str(state)
    if state not in run_states:
        ended=True
    time.sleep(0.5)
    nbiter+=1
        
if state not in end_states:
    print "ERR: jobid = "+str(jobid)+" ended abnormally with state="+str(state)
else:
    print "OK:  jobid = "+str(jobid)+" ended with state="+str(state)
    meshJobResults = component.finalize(jobid)
    print meshJobResults
    print "You will find the results files in the directory:\n%s"%meshJobResults.results_dirname
