#!/usr/bin/env python3
# Copyright (C) 2017-2020  CEA/DEN, EDF R&D, OPEN CASCADE
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
import inspect
import sys
from types import FunctionType
import copy

ORIGIN_MODULE_SUFFIX = "_origin"
DYNAMIC_MODULE_SUFFIX = "_dynamic"


def main(module_name, output_file = "smeshBuilder.py"):
    oringin_module_name = module_name + ORIGIN_MODULE_SUFFIX
    dynamic_module_name = module_name + DYNAMIC_MODULE_SUFFIX
    try:
        exec( "import %s" % oringin_module_name )
        origin_module = locals()[ oringin_module_name ]
        origin_module_lines = inspect.getsourcelines( origin_module )[0]
        origin_meshClass_lines = inspect.getsourcelines(origin_module.Mesh)[0]
        origin_module_text = "".join( origin_module_lines )
        origin_meshClass_text = "".join( origin_meshClass_lines )
        
        exec( "import %s" % dynamic_module_name )
        dynanmic_module = locals()[ dynamic_module_name ]
        dynanmic_meshClass = dynanmic_module.Mesh
    
        new_meshClass_lines = copy.copy(origin_meshClass_lines)
        # remove end of class 'pass'
        if new_meshClass_lines[-1].find("pass") > 0:
            new_meshClass_lines.pop()
            
        dynanmic_meshClass_methods = [x for x, y in dynanmic_meshClass.__dict__.items() if type(y) == FunctionType]
        for method in dynanmic_meshClass_methods:
            exec( "method_lines = inspect.getsourcelines(dynanmic_module.Mesh.%s)[0]"  % method)
            new_meshClass_lines+=locals()['method_lines']
            pass
        new_meshClass_text = "".join( new_meshClass_lines )            

        f = open( output_file, "w" )
        
        f.write( origin_module_text.replace( origin_meshClass_text, new_meshClass_text) )
        f.close()        
    except Exception as e:
        print(e)
        pass
    pass


if __name__ == "__main__":
    import optparse
    parser = optparse.OptionParser(usage="%prog [options] modulename")
    h  = "Output file (smeshBuilder.py by default)"
    parser.add_option("-o", "--output", dest="output",
                      action="store", default="smeshBuilder.py", metavar="file",
                      help=h)

    (options, args) = parser.parse_args()

    if len( args ) < 1: sys.exit("Module name is not specified")
    main( args[0], options.output )
    pass
