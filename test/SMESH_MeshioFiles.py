#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2024  CEA, EDF, OPEN CASCADE
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
#  File   : SMESH_MeshioFiles.py
#  Module : SMESH

import tempfile
import subprocess
import os
from pathlib import Path

from SMESH_MeshioShapes import export_mesh, import_file


def supported_extensions():
    """
    Returns a list of format extensions that salome already supports
    for import/export operations.
    """

    return [
        '.med',
        '.unv',
        '.stl',
        '.cgns',
        '.mesh',
        '.meshb'
    ]


def convert(source_file, target_file, errors):
    """
    Converts files with meshio shell command.
    """
    try:
        cmd = 'meshio convert {} {}'.format(source_file, target_file)
        print('{} start...'.format(cmd))

        subprocess.run(['meshio', 'convert', source_file, target_file], check=True)

        return True

    except Exception:
        source_ext = Path(source_file).suffix.upper()
        target_ext = Path(target_file).suffix.upper()

        errors.append('[{} -> {}]: {} failed!'.format(source_ext, target_ext, cmd))

        return False


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


def perform_ext(directory, ext, errors):
    """
    Exports a given mesh and imports it back for each mesh file
    through a file with a given extension.
    It uses shell meshio convert command for conversion.
    """

    for subdir, _, files in os.walk(directory):
        for file in files:

            # Skip helper files
            if file.endswith('.md') or file.endswith('Makefile'):
                continue

            filepath = os.path.join(subdir, file)

            with tempfile.NamedTemporaryFile(suffix=ext) as temp_file:
                temp_file.close() # prevents PermissionError on Windows
                if not convert(filepath, temp_file.name, errors):
                    continue

                file_extension = Path(file).suffix
                with tempfile.NamedTemporaryFile(suffix=file_extension) as temp_file_back:
                    temp_file.close() # prevents PermissionError on Windows
                    temp_file_back.close() # prevents PermissionError on Windows
                    convert(temp_file.name, temp_file_back.name, errors)


def test_shell(directory, errors):
    """
    Iterates salome extensions and convert meshes through
    a corresponding format.
    .
    """

    for ext in supported_extensions():
        perform_ext(directory, ext, errors)


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

            # Skip files those have issues with current meshio version
            if (file.endswith('.f3grid') or
                file.endswith('.ugrid') or
                file.endswith('.su2') or
                file.endswith('element_elset.inp') or
                file.endswith('insulated-4.1.msh') or
                file.endswith('insulated-2.2.msh') or
                file.endswith('simple1')):
                continue

            filepath = os.path.join(subdir, file)

            # Import a file
            mesh = import_file(filepath, errors)
            if not mesh:
                continue

            file_extension = Path(file).suffix
            with tempfile.NamedTemporaryFile(suffix=file_extension) as temp_file:
                temp_file.close() # prevents PermissionError on Windows
                export_mesh(mesh, temp_file.name, errors)


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
