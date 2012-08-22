#!/usr/bin/env python
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
# for generatation of the extra developer documentation for
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

import sys

def main(plugin, dummymeshhelp = True, output_file = "smesh.py"):
    plugin_module = plugin + "DC"
    try:
        mod = __import__(plugin_module)
        methods = {}
        for attr in dir( mod ):
            if attr.startswith( '_' ): continue
            algo = getattr( mod, attr )
            if type( algo ).__name__ == 'classobj' and hasattr( algo, "meshMethod" ):
                method = getattr( algo, "meshMethod" )
                if method not in methods: methods[ method ] = []
                methods[ method ].append( algo )
                pass
            pass
        if methods:
            output = []
            if dummymeshhelp:
                output.append( "## @package smesh" )
                output.append( "#  Documentation of the methods dynamically added by the " + plugin + " meshing plug-in to the Mesh class." )
                output.append( "" )
                pass
            output.append( "## This class allows defining and managing a mesh." )
            output.append( "#" )
            if dummymeshhelp:
                # Add dummy Mesh help
                # This is supposed to be done when generating documentation for meshing plug-ins
                output.append( "#  @note The documentation below does not provide complete description of class @b %Mesh" )
                output.append( "#  from @b %smesh.py package. This documentation provides only information about" )
                output.append( "#  the methods dynamically added to the %Mesh class by the " + plugin + " plugin" )
                output.append( "#  For more details on the %Mesh class, please refer to the SALOME %Mesh module" )
                output.append( "#  documentation." )
                pass
            else:
                # Extend documentation for Mesh class with information about dynamically added methods.
                # This is supposed to be done only when building documentation for SMESH module
                output.append( "#  @note Some methods are dynamically added to the @b %Mesh class in runtime by meshing " )
                output.append( "#  plug-in modules. If you fail to find help on some methods in the documentation of SMESH module, " )
                output.append( "#  try to look into the documentation for the meshing plug-ins." )
                pass
            output.append( "class Mesh:" )
            for method in methods:
                docHelper = ""
                for algo in methods[ method ]:
                    if hasattr( algo, "docHelper" ): docHelper = getattr( algo, "docHelper" )
                    if docHelper: break
                    pass
                if not docHelper: docHelper = "Creates new algorithm."
                output.append( " ## %s" % docHelper )
                output.append( " #" )
                output.append( " #  This method is dynamically added to %Mesh class by the meshing plug-in(s). " )
                output.append( " #" )
                output.append( " #  If the optional @a geom_shape parameter is not set, this algorithm is global (applied to whole mesh)." )
                output.append( " #  Otherwise, this algorithm defines a submesh based on @a geom_shape subshape." )
                output.append( " #  @param algo_type type of algorithm to be created; allowed values are specified by classes implemented by plug-in (see below)" )
                output.append( " #  @param geom_shape if defined, the subshape to be meshed (GEOM_Object)" )
                output.append( " #  @return An instance of Mesh_Algorithm sub-class according to the specified @a algo_type, see " )
                output.append( " #  %s" % ", ".join( [ "%s.%s" % ( plugin_module, algo.__name__ ) for algo in methods[ method ] ] ) )
                output.append( " def %s(algo_type, geom_shape=0):" % method )
                output.append( "   pass" )
                pass
            f = open(output_file, "w")
            for line in output: f.write( line + "\n" )
            f.close()
            pass
        pass
    except Exception, e:
        print e
        pass
    pass
    
if __name__ == "__main__":
    import optparse
    parser = optparse.OptionParser(usage="%prog [options] plugin")
    h  = "Output file (smesh.py by default)"
    parser.add_option("-o", "--output", dest="output",
                      action="store", default=None, metavar="file",
                      help=h)
    h  = "If this option is True, dummy help for Mesh class is added. "
    h += "This option should be False (default) when building documentation for SMESH module "
    h += "and True when building documentation for meshing plug-ins."
    parser.add_option("-d", "--dummy-mesh-help", dest="dummymeshhelp",
                      action="store_true", default=False,
                      help=h)
    (options, args) = parser.parse_args()

    if len( args ) < 1: sys.exit("Plugin name is not specified")
    main( args[0], options.dummymeshhelp, options.output )
    pass
