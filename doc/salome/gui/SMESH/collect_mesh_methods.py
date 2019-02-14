#!/usr/bin/env python3
# Copyright (C) 2012-2019  CEA/DEN, EDF R&D, OPEN CASCADE
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

#################################################################################
#
# File:   collect_mesh_methods.py
# Author: Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
#
#################################################################################
#
# Extraction of the meshing algorithm classes
# dynamically added by the plug-in to the Mesh
# class.
# 
# This script is intended for internal usage - only
# for generation of the extra developer documentation for
# the meshing plug-in(s).
# 
# Usage:
#       collect_mesh_methods.py <plugin_name>
# where
#   <plugin_name> is a name of the plug-in module
#
# Notes:
# - the script is supposed to be run in correct environment
# i.e. PYTHONPATH, SMESH_MeshersList and other important
# variables are set properly; otherwise the script will fail.
#
################################################################################

import inspect
import sys

def main(plugin_name, dummymeshhelp = True, output_file = "smeshBuilder.py", format = "doxygen"):
    plugin_module_name  = plugin_name + "Builder"
    plugin_module       = "salome.%s.%s" % (plugin_name, plugin_module_name)
    try:
        exec("from salome.smesh.smeshBuilder import *", globals())
        exec("import %s" % plugin_module, globals())
        exec("mod = %s" % plugin_module , globals())
        methods = {}
        for attr in dir( mod ):
            if attr.startswith( '_' ): continue
            algo = getattr( mod, attr )
            if inspect.isclass(algo) and hasattr(algo, "meshMethod"):
                method = getattr( algo, "meshMethod" )
                if method not in methods: methods[ method ] = []
                methods[ method ].append( algo )
                pass
            pass
        if methods:
            output = []
            if dummymeshhelp:
                if format == "doxygen":
                    output.append( "## @package smeshBuilder" )
                    output.append( "#  Documentation of the methods dynamically added by the " + plugin_name + " meshing plug-in to the Mesh class." )
                    output.append( "" )
                elif format == "sphinx":
                    output.append( '"""' )
                    output.append( 'Documentation of the methods dynamically added by the ' + plugin_name + ' meshing plug-in to the Mesh class.' )
                    output.append( '"""' )
                    output.append( '' )
                pass
            if format == "doxygen":
                output.append( "## This class allows defining and managing a mesh." )
                output.append( "#" )
            elif format == "sphinx":
                output.append( "class Mesh:" )
                output.append( '    """' )
                output.append( '    This class allows defining and managing a mesh.' )
                output.append( '    ' )
            if dummymeshhelp:
                # Add dummy Mesh help
                # This is supposed to be done when generating documentation for meshing plug-ins
                if format == "doxygen":
                    output.append( "#  @note The documentation below does not provide complete description of class @b %Mesh" )
                    output.append( "#  from @b smeshBuilder package. This documentation provides only information about" )
                    output.append( "#  the methods dynamically added to the %Mesh class by the " + plugin_name + " plugin" )
                    output.append( "#  For more details on the %Mesh class, please refer to the SALOME %Mesh module" )
                    output.append( "#  documentation." )
                elif format == "sphinx":
                    output.append( '    The documentation below does not provide complete description of class @b %Mesh' )
                    output.append( '    from @b smeshBuilder package. This documentation provides only information about' )
                    output.append( '    the methods dynamically added to the %Mesh class by the " + plugin_name + " plugin' )
                    output.append( '    For more details on the %Mesh class, please refer to the SALOME %Mesh module' )
                    output.append( '    documentation.' )
                    output.append( '    """' )
                    output.append( '    ' )
                pass
            else:
                # Extend documentation for Mesh class with information about dynamically added methods.
                # This is supposed to be done only when building documentation for SMESH module
                if format ==  "doxygen":
                    output.append( "#  @note Some methods are dynamically added to the @b %Mesh class in runtime by meshing " )
                    output.append( "#  plug-in modules. If you fail to find help on some methods in the documentation of SMESH module, " )
                    output.append( "#  try to look into the documentation for the meshing plug-ins." )
                elif format == "sphinx":
                    output.append( "    Note:")
                    output.append( "        Some methods are dynamically added to the @b %Mesh class in runtime by meshing " )
                    output.append( "        plug-in modules. If you fail to find help on some methods in the documentation of SMESH module, " )
                    output.append( "        try to look into the documentation for the meshing plug-ins." )
                    output.append( '    """' )
                    output.append( '    ' )
                pass
            if format == "doxygen":
                output.append( "class Mesh:" )
            for method in methods:
                docHelper = ""
                for algo in methods[ method ]:
                    if hasattr( algo, "docHelper" ): docHelper = getattr( algo, "docHelper" )
                    if docHelper: break
                    pass
                if not docHelper: docHelper = "Create new algorithm."
                if format == "doxygen":
                    output.append( " ## %s" % docHelper )
                    output.append( " #" )
                    output.append( " #  This method is dynamically added to %Mesh class by the meshing plug-in(s). " )
                    output.append( " #" )
                    output.append( " #  If the optional @a geom_shape parameter is not set, this algorithm is global (applied to whole mesh)." )
                    output.append( " #  Otherwise, this algorithm defines a submesh based on @a geom_shape subshape." )
                    output.append( " #  @param algo_type type of algorithm to be created; allowed values are specified by classes implemented by plug-in" )
                    output.append( " #  @param geom_shape if defined, the subshape to be meshed (GEOM_Object)" )
                    output.append( " #  @return An instance of Mesh_Algorithm sub-class according to the specified @a algo_type, see " )
                    output.append( " #  %s" % ", ".join( [ "%s.%s" % ( plugin_module_name, algo.__name__ ) for algo in methods[ method ] ] ) )
                    output.append( " def %s(algo_type, geom_shape=0):" % method )
                    output.append( "   pass" )
                elif format == "sphinx":
                    output.append( '    def %s(algo_type, geom_shape=0):' % method )
                    output.append( '        """' )
                    output.append( '        %s' % docHelper )
                    output.append( '        ' )
                    output.append( '        This method is dynamically added to :class:`Mesh <smeshBuilder.Mesh>` class by the meshing plug-in(s). ' )
                    output.append( '        ' )
                    output.append( '        If the optional *geom_shape* parameter is not set, this algorithm is global (applied to whole mesh).' )
                    output.append( '        Otherwise, this algorithm defines a submesh based on *geom_shape* subshape.' )
                    output.append( '        ' )
                    output.append( '        Parameters:' )
                    output.append( '            algo_type: type of algorithm to be created; allowed values are specified by classes implemented by plug-in' )
                    output.append( '            geom_shape (GEOM_Object): if defined, the subshape to be meshed' )
                    output.append( '        ' )
                    output.append( '        Returns:')
                    output.append( '            An instance of Mesh_Algorithm sub-class according to the specified *algo_type*, see ' )
                    output.append( '            %s' % ", ".join( [ ":class:`~%s.%s`" % ( plugin_module_name, algo.__name__ ) for algo in methods[ method ] ] ) )
                    output.append( '        """' )
                    output.append( '        pass' )
                pass
            with open(output_file, "w", encoding='utf8') as f:
                f.write('\n'.join(output))
            pass
        pass
    except Exception as e:
        print(e)
        pass
    pass
    
if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    h  = "Output file (smesh.py by default)"
    parser.add_argument("-o", "--output", dest="output",
                      action="store", default='smesh.py', metavar="file",
                      help=h)
    h  = "If this option is True, dummy help for Mesh class is added. "
    h += "This option should be False (default) when building documentation for SMESH module "
    h += "and True when building documentation for meshing plug-ins."
    parser.add_argument("-d", "--dummy-mesh-help", dest="dummymeshhelp",
                      action="store_true", default=False,
                      help=h)
    h = "Format of the documentation strings in the output file. Possible values are: "
    h+= "'doxygen' - documentation strings are generated in the doxygen format, before a method definition."
    h+= "'sphinx' - documentation strings are generated in the sphinx format, after a method definition."
    parser.add_argument("-f", "--format", dest="format",
                      action="store", default="doxygen", help=h)

    parser.add_argument("plugin_name")


    args = parser.parse_args()

    if args.plugin_name is None : sys.exit("Plugin name is not specified")
    main( args.plugin_name, args.dummymeshhelp, args.output, args.format )
    pass
