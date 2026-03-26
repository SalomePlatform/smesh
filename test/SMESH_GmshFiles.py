#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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

# =======================================
# Testing export/import of sample meshes from meshio repository
#  File   : SMESH_GmshFiles.py
#  Module : SMESH

import tempfile
import subprocess
import os
from pathlib import Path

from salome.kernel import SALOME_CMOD, SMESH
from salome.smesh import smeshBuilder

# Constants
EXPORT_TITLE = 'Export'
IMPORT_TITLE = 'Import'
UNKNOWN_EXCEPTION = 'Unknown exception'
OUTPUT_DIVIDER = '\n==============================================='

smesh = smeshBuilder.New()


def exception_handle(file_name, errors, operation_type, ex_text):
    """
    Pepares and saves an exception message.
    """

    ext = Path(file_name).suffix.upper()
    msg = '{} {} failed! File name: {}'.format(operation_type, ext, file_name)
    if ex_text:
        msg += ' Exception: ' + ex_text + OUTPUT_DIVIDER + '\n'

    errors.append(msg)

def export_mesh(mesh, file_name, filter='', errors=None):
    """
    Performs an export operation.
    """

    try:
        print('Export to file: ', file_name)

        mesh.ExportGMSHIO(file_name, filter, mesh)
        return True

    except SALOME_CMOD.SALOME_Exception as ex:
        exception_handle(file_name, errors, EXPORT_TITLE, ex.details.text)

    except:
        exception_handle(file_name, errors, EXPORT_TITLE, UNKNOWN_EXCEPTION)

    return False

def import_file(file_name, filter='', errors=None):
    """
    Performs an import operation.
    """

    try:
        print('Import from file: ', file_name)

        ([mesh], status) = smesh.CreateMeshesFromGMSHIO(file_name, filter)
        return mesh

    except SALOME_CMOD.SALOME_Exception as ex:
        exception_handle(file_name, errors, IMPORT_TITLE, ex.details.text)

    except:
        exception_handle(file_name, errors, IMPORT_TITLE, UNKNOWN_EXCEPTION)

    return None

def print_errors(errors):
    """
    Checks if we have got any saved error messages and print them.
    """

    if not len(errors):
        return

    print('\nErrors:')
    for idx, err in enumerate(errors):
        print('{:02}: {}'.format(idx, err))

    print('\n===============================================')

def test_salome(directory, errors):
    """
    Iterates over mesh files in a given directory and tries to import
    them into salome and export back to a temp file with the same format.
    """
    for subdir, _, files in os.walk(directory):
        for file in files:

            # Skip helper files
            if file.endswith('.md') or file.endswith('Makefile'):
                continue
            # Skip files those have issues current version
            if (file.endswith('.fem')    or
                file.endswith('.su2')    or
                file.endswith('.f3grid') or
                file.endswith('.ele')    or
                file.endswith('.inp')    or
                file.endswith('.tec')    or
                file.endswith('.vtk')    or
                file.endswith('.ugrid')  or
                file.endswith('simple1')):
                continue
            # Skip files that are not supported by Gmsh
            if (file.endswith('.vol') or
                file.endswith('.vtu') or
                file.endswith('.node') or
                file.endswith('.wkt') or
                file.endswith('.meshb')):
                continue

            filepath = os.path.join(subdir, file)

            # Import a file
            mesh = import_file(filepath, '', errors)
            if not mesh:
                continue

            file_extension = Path(file).suffix
            with tempfile.NamedTemporaryFile(suffix=file_extension) as temp_file:
                temp_file.close() # prevents PermissionError on Windows
                export_mesh(mesh, temp_file.name, '', errors)


def test():
    """
    Tests import/export of meshes from a given directory
    and prints errors if we have any.
    """

    errors = []
    directory = 'data/meshio_meshes'

    # Only for debug purpose.
    # Uncomment if you need to test meshio library shell command itself.
    # test_shell(directory, errors)

    test_salome(directory, errors)

    print_errors(errors)
    assert not len(errors)


if __name__ == "__main__":
    test()
