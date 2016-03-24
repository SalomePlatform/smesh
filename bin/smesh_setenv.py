#!/usr/bin/env python
#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

import os, sys
from setenv import add_path, get_lib_dir, salome_subdir

# -----------------------------------------------------------------------------

def set_env(args):
    """Add to the PATH-variables modules specific paths"""
    psep = os.pathsep
    python_version="python%d.%d" % sys.version_info[0:2]


    if not os.environ.has_key("SALOME_StdMeshersResources"):
        os.environ["SALOME_StdMeshersResources"] \
        = os.path.join(os.environ["SMESH_ROOT_DIR"],"share",salome_subdir,"resources","smesh")
        pass

    # find plugins
    plugin_list = ["StdMeshers"]
    resource_path_list = []
    for env_var in os.environ.keys():
        value = os.environ[env_var]
        if env_var[-9:] == "_ROOT_DIR" and value:
            plugin_root = value
            plugin = env_var[:-9] # plugin name may have wrong case

            # look for NAMEOFPlugin.xml file among resource files
            resource_dir = os.path.join(plugin_root,"share",salome_subdir,"resources",plugin.lower())
            if not os.access( resource_dir, os.F_OK ): continue
            for resource_file in os.listdir( resource_dir ):
                if not resource_file.endswith( ".xml") or \
                   resource_file.lower() != plugin.lower() + ".xml":
                    continue
                # use "resources" attribute of "meshers-group" as name of plugin in a right case
                from xml.dom.minidom import parse
                xml_doc = parse( os.path.join( resource_dir, resource_file ))
                meshers_nodes = xml_doc.getElementsByTagName("meshers-group")
                if not meshers_nodes or not meshers_nodes[0].hasAttribute("resources"): continue
                plugin = meshers_nodes[0].getAttribute("resources")
                if plugin in plugin_list: continue

                # add paths of plugin
		plugin_list.append(plugin)
                if not os.environ.has_key("SALOME_"+plugin+"Resources"):
                    resource_path = os.path.join(plugin_root,"share",salome_subdir,"resources",plugin.lower())
                    os.environ["SALOME_"+plugin+"Resources"] = resource_path
                    resource_path_list.append( resource_path )
                    add_path(os.path.join(plugin_root,get_lib_dir(),python_version, "site-packages",salome_subdir), "PYTHONPATH")
                    add_path(os.path.join(plugin_root,get_lib_dir(),salome_subdir), "PYTHONPATH")
                    
                    if sys.platform == "win32":
                        add_path(os.path.join(plugin_root,get_lib_dir(),salome_subdir), "PATH")
                        add_path(os.path.join(plugin_root,"bin",salome_subdir), "PYTHONPATH")
                    else:
                        add_path(os.path.join(plugin_root,get_lib_dir(),salome_subdir), "LD_LIBRARY_PATH")
                        add_path(os.path.join(plugin_root,"bin",salome_subdir), "PYTHONPATH")
                        add_path(os.path.join(plugin_root,"bin",salome_subdir), "PATH")
                        pass
                    pass
                break
    os.environ["SMESH_MeshersList"] = ":".join(plugin_list)
    os.environ["SalomeAppConfig"] = os.environ["SalomeAppConfig"] + psep + psep.join(resource_path_list)

