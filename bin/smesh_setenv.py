#!/usr/bin/env python
#  -*- coding: iso-8859-1 -*-
#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

import os, sys
from setenv import add_path, get_lib_dir, salome_subdir

# -----------------------------------------------------------------------------

def set_env(args):
    """Add to the PATH-variables modules specific paths"""

    python_version="python%d.%d" % sys.version_info[0:2]

    os.environ["SMESH_MeshersList"]="StdMeshers"
    if not os.environ.has_key("SALOME_StdMeshersResources"):
        os.environ["SALOME_StdMeshersResources"] \
        = os.environ["SMESH_ROOT_DIR"]+"/share/"+salome_subdir+"/resources/smesh"
        pass
    if args.has_key("SMESH_plugins"):
        for plugin in args["SMESH_plugins"]:
            plugin_root = ""
            if os.environ.has_key(plugin+"_ROOT_DIR"):
                plugin_root = os.environ[plugin+"_ROOT_DIR"]
            else:
                # workaround to avoid modifications of existing environment
                if os.environ.has_key(plugin.upper()+"_ROOT_DIR"):
                    plugin_root = os.environ[plugin.upper()+"_ROOT_DIR"]
                    pass
                pass
            if plugin_root != "":
                os.environ["SMESH_MeshersList"] \
                = os.environ["SMESH_MeshersList"]+":"+plugin
                if not os.environ.has_key("SALOME_"+plugin+"Resources"):
                    os.environ["SALOME_"+plugin+"Resources"] \
                    = plugin_root+"/share/"+salome_subdir+"/resources/"+plugin.lower()
                    add_path(os.path.join(plugin_root,get_lib_dir(),python_version, "site-packages",salome_subdir), "PYTHONPATH")
                    add_path(os.path.join(plugin_root,get_lib_dir(),salome_subdir), "PYTHONPATH")
                    
                    if sys.platform == "win32":
                        add_path(os.path.join(plugin_root,get_lib_dir(),salome_subdir), "PATH")
                    else:
                        add_path(os.path.join(plugin_root,get_lib_dir(),salome_subdir), "LD_LIBRARY_PATH")
                        add_path(os.path.join(plugin_root,"bin",salome_subdir), "PYTHONPATH")
                        add_path(os.path.join(plugin_root,"bin",salome_subdir), "PATH")
                        pass
                    pass
                pass
            pass
