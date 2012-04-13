#!/bin/bash

# Copyright (C) 2011-2012  CEA/DEN, EDF R&D
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

# This script creates a source file that defines a SALOME shell
# environment. We assume here that the SALOME environment has been
# previously set, so that the env command get all environment
# variables required for building and executing SALOME. We talk
# about third party software programs and libraries. The environment
# variables defining SALOME module are exluded (i.e. *_ROOT_DIR)
# because they are automatically set when generating a SALOME application..
#
# The argument is the filepath to be created.
#

if [ $# == 1 ]; then
    ENVAPPLI_SH=$1
else
    ENVAPPLI_SH=envappli.sh
fi

function header {
    echo "#"
    echo "# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo "# THIS FILE IS GENERATED from the shell environment used to build the SALOME module."
    echo "# IT SHOULD NOT BE EDITED, it is generated for the need of the SALOME application   "
    echo "# that embeds the module (for test purposes).                                       "
    echo "# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo "#"
}
header > $ENVAPPLI_SH
env | grep -v -e PWD -e SalomeAppConfig -e _ROOT_DIR | while read f; do
    key=$(echo $f | cut -d"=" -f1)
    value=$(echo $f | cut -d"=" -f2-)

    # if the key is a path (LD_LIBRARY_PATH, PATH and PYTHONPATH) then
    # we must extends the variable.
    if [ $key == "LD_LIBRARY_PATH" -o $key == "PATH" -o $key == "PYTHONPATH" ]; then
        echo export $key=\"$value:\$$key\"
    else
        echo export $key=\"$value\"
    fi
done >> $ENVAPPLI_SH
