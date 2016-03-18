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

import sys, os
import ConfigParser
from MESHJOB import ConfigParameter
from salome.kernel.uiexception import AdminException, UiException

from salome_pluginsmanager import PLUGIN_PATH_PATTERN
CONFIG_RELPATH  = os.path.join(PLUGIN_PATH_PATTERN,'smesh')
CONFIG_FILENAME = "padder.cfg"
TYPE_LOCAL   = 'local'
TYPE_REMOTE  = 'remote'
TYPES=[TYPE_LOCAL, TYPE_REMOTE]

class ConfigReader:
    def __init__(self):
        # The first step is to look for the config file. This file
        # is supposed to be located in the same directory than the
        # padder plugin. Then, we have to scan the directories
        # specified in the SALOME plugins path.
        self.__configFilename = None
        try:
            smeshpath=os.environ["SMESH_ROOT_DIR"]
        except KeyError, ex:
            raise AdminException("You should define the variable SMESH_ROOT_DIR")

        pluginspath = os.path.join(smeshpath,CONFIG_RELPATH)
        filename    = os.path.join(pluginspath,CONFIG_FILENAME)
        if os.path.exists(filename):
            self.__configFilename = filename
        else:
            msg = "The configuration file %s can't be found in the SMESH plugins path %s"
            raise AdminException(msg%(CONFIG_FILENAME,pluginspath))

        print "The configuration file is : %s"%self.__configFilename
        self.__configparser = ConfigParser.RawConfigParser()
        try:
            self.__configparser.read(self.__configFilename)
        except ConfigParser.ParsingError, ex:
            raise AdminException(ex.message)

    def getLocalConfig(self):
        return self.__getConfig(TYPE_LOCAL)
    
    def getRemoteConfig(self):
        return self.__getConfig(TYPE_REMOTE)

    def getDefaultConfig(self):
        defaultType = self.__getDefaultType()
        return self.__getConfig(defaultType)
        
    def __getConfig(self, type=TYPE_LOCAL):
        configName = self.__configparser.get('resources', type)
        resname = self.__configparser.get(configName, 'resname')
        binpath = self.__configparser.get(configName, 'binpath')
        envpath = self.__configparser.get(configName, 'envpath')
        config = ConfigParameter(resname, binpath, envpath)
        config.resname = resname
        return config

    def __getDefaultType(self):
        '''This returns the default type read in the config file ([resources], default)'''
        defaultType = self.__configparser.get('preferences', 'defaultres')
        if defaultType not in TYPES:
            return TYPE_LOCAL
        return defaultType

def printConfig(config):
    print "PADDER CONFIGURATION:"
    print "\tconfig.resname = %s"%config.resname
    print "\tconfig.binpath = %s"%config.binpath
    print "\tconfig.envpath = %s"%config.envpath
    
def getPadderTestDir(config):
    """
    This function returns the directory of the SpherePadder
    installation, where the tests cases are located. This should be
    used for test only. It makes the hypothesis that the binpath to
    the executable program is a path of the executable program of a
    complete installation of SpherePadder.
    """
    testdir=os.path.join(os.path.abspath(os.path.dirname(config.binpath)),"tests")
    return testdir

#
# =========================================================================
# Test runner
# =========================================================================
#
def TEST_getDefaultConfig():
    try:
        configReader = ConfigReader()
        defaultConfig = configReader.getDefaultConfig()
        print defaultConfig.resname
        print defaultConfig.binpath
        print defaultConfig.envpath
    except Exception, ex:
        sys.stderr.write('ERROR: %s\n' % str(ex))
        return False
    
    return True

def TEST_getDefaultConfig_withError():
    global CONFIG_FILENAME
    CONFIG_FILENAME = "toto.cfg"
    try:
        configReader = ConfigReader()
        defaultConfig = configReader.getDefaultConfig()
    except UiException, err:
        print 'ERROR: %s' % str(err)
        return True
    
    return False


from salome.kernel import unittester
moduleName = "configreader"

def testsuite():
    unittester.run(moduleName, "TEST_getDefaultConfig")
    unittester.run(moduleName, "TEST_getDefaultConfig_withError")
    
if __name__ == "__main__":
    #import os, sys
    #pluginspath=os.environ["SALOME_PLUGINS_PATH"]
    #for path in pluginspath.split(":"):
    #    sys.path.insert(0,path)
    
    testsuite()
