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

# TODO: put all this stuff in the unitests package

import os
def getRootDir():
    '''
    This returns the root directory where the module SPADDER is
    installed. All test files are looked up from this location.
    '''
    return os.environ['SMESH_ROOT_DIR']

def getTestDataDir():
    '''
    This function gives the absolute path to the SMESH directory
    containing the data files for the padder plugin test (realistic
    med files).
    '''
    datadir = os.path.join(getRootDir(),"share/salome/resources/smesh/padderdata")
    return datadir

def getTestPadderDataDir():
    """
    This function gives the absolute path to the PADDER directory
    containing the data files for the padder plugin test. WARNING:
    this directory is a directory of the external program SpherePadder
    that is wrapped by the padder plugin. We use the shell variable
    PADDERHOME (defined by the SALOME environment) to localize this
    folder. 
    """
    PADDERHOME = os.environ['PADDERHOME']
    datadir = os.path.join(PADDERHOME,"tests")
    return datadir

import MESHJOB # to get the enum constant values
from MESHJOB import MeshJobParameter, MeshJobParameterList

DEFAULT_CONCRETE_FILENAME=os.path.join(getTestDataDir(),'concrete.med')
DEFAULT_STEELBAR_LISTFILENAME=[
    os.path.join(getTestDataDir(),'ferraill.med')
    ]

def getMeshJobParameterList(concrete_filename=DEFAULT_CONCRETE_FILENAME,
                            steelbar_listfilename=DEFAULT_STEELBAR_LISTFILENAME):
    '''
    This helper function creates a complete set of parameters (a
    MeshJobParameterList) for a simple test case, i.e. a case with a
    concrete filename and a single steelbar filename.
    '''
    # Note that a CORBA sequence (MeshJobParameterList) is mapped on a
    # simple list in python
    meshJobParameterList = []
    # We can add some parameters
    param = MeshJobParameter(
        file_name  = concrete_filename,
        file_type  = MESHJOB.MED_CONCRETE,
        group_name = "concrete")
    meshJobParameterList.append(param)

    for steelbar_filename in steelbar_listfilename:
        param = MeshJobParameter(
            file_name  = steelbar_filename,
            file_type  = MESHJOB.MED_STEELBAR,
            group_name = "steelbar")
        meshJobParameterList.append(param)

    return meshJobParameterList


def getSpadderCatalogFilename():
    filename=os.path.join(getRootDir(),"share/salome/resources/smesh/SPADDERCatalog.xml")
    return filename

def loadSpadderCatalog():
    import salome
    salome.salome_init()
    obj = salome.naming_service.Resolve('Kernel/ModulCatalog')
    import SALOME_ModuleCatalog
    catalog = obj._narrow(SALOME_ModuleCatalog.ModuleCatalog)
    if not catalog:
        raise RuntimeError, "Can't accesss module catalog"

    filename = getSpadderCatalogFilename()
    catalog.ImportXmlCatalogFile(filename)

    from salome.kernel import services
    print "The list of SALOME components is now:" 
    print services.getComponentList()
