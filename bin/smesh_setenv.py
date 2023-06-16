#!/usr/bin/env python3
# Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

"""
Set-up additional environment needed for SMESH module and meshing plugins.
"""

import os
import os.path as osp
import sys
from xml.dom.minidom import parse
from setenv import add_path, get_lib_dir, salome_subdir

# -----------------------------------------------------------------------------

def set_env(args):
    """Set-up additional environment needed for SMESH module and plugins"""
    py_version = 'python{}.{}'.format(*sys.version_info[:2])

    # search and set-up meshing plugins
    plugins = []
    resource_dirs = []
    for var in [i for i in os.environ if i.endswith('_ROOT_DIR') and os.environ[i]]:
        plugin_root = os.environ[var]
        plugin_name = var[:-9] # plugin name as extracted from environment variable
        plugin_lname = plugin_name.lower() # plugin name in lowercase

        # look for NAMEOFPlugin.xml file among resource files
        # resource dir must be <plugin_root>/share/salome/resources/<plugin_name_lowercase>
        resource_dir = osp.join(plugin_root, 'share', salome_subdir, 'resources', plugin_lname)
        if not os.access(resource_dir, os.F_OK):
            continue # directory does not exist or isn't accessible

        for resource_file in [i for i in os.listdir(resource_dir) \
                                  if osp.isfile(os.path.join(resource_dir, i))]:
            # look for resource file (XML) to extract valid plugin name
            if resource_file.lower() == '{plugin_lname}.xml'.format(plugin_lname=plugin_lname):
                try:
                    # get plugin name from 'resources' attribute of 'meshers-group' xml node
                    # as name extracted from environment variable can be in wrong case
                    xml_doc = parse(osp.join(resource_dir, resource_file))
                    plugin_name = xml_doc.getElementsByTagName('meshers-group')[0].getAttribute('resources')

                    # add plugin to the list of available meshing plugins
                    plugins.append(plugin_name)
                    resource_dirs.append(resource_dir)

                    # setup environment needed for plugin
                    add_path(osp.join(plugin_root, 'bin', salome_subdir), 'PATH')
                    add_path(osp.join(plugin_root, get_lib_dir(), salome_subdir), 'PATH' \
                                 if sys.platform == 'win32' else 'LD_LIBRARY_PATH')
                    add_path(osp.join(plugin_root, 'bin', salome_subdir), 'PYTHONPATH')
                    add_path(osp.join(plugin_root, get_lib_dir(), salome_subdir), 'PYTHONPATH')
                    add_path(osp.join(plugin_root, get_lib_dir(), py_version, 'site-packages',
                                      salome_subdir), 'PYTHONPATH')

                    break # one resource file is enough!
                except:
                    continue # invalid resource valid

    # full list of known meshers
    os.environ['SMESH_MeshersList'] = os.pathsep.join(['StdMeshers'] + plugins)
    # access to resources
    os.environ['SalomeAppConfig'] = os.pathsep.join(os.environ['SalomeAppConfig'].split(os.pathsep) + resource_dirs)
