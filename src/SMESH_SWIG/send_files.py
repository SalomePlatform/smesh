#!/usr/bin/env python3
# -*- coding: utf-8 -*-
## Copyright (C) 2021-2026  CEA/DEN, EDF R&D, OPEN CASCADE
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this library; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
##
## See http://www.salome-platform.org/ or email :
##     webmaster.salome@opencascade.com
##
"""
File to send files on remote resource
"""
from os import environ, path

from argparse import ArgumentParser
import pydefx
from salome.kernel import pylauncher

def create_launcher():
    """ Initialise pylauncher
    """
    launcher = pylauncher.Launcher_cpp()
    launcher.SetResourcesManager(create_resources_manager())
    return launcher

def create_resources_manager():
    """ Look for the catalog file and create a resource manager with it """
    # localhost is defined anyway, even if the catalog file does not exist.
    catalog_path = environ.get("USER_CATALOG_RESOURCES_FILE", "")
    if not path.isfile(catalog_path):
        salome_path = environ.get("ROOT_SALOME_INSTALL", "")
        catalog_path = path.join(salome_path, "CatalogResources.xml")
    if not path.isfile(catalog_path):
        catalog_path = ""

    return pylauncher.ResourcesManager_cpp(catalog_path)

def create_job_parameters():
    """ Initialsie JobParameters """
    jparam = pylauncher.JobParameters_cpp()
    jparam.resource_required = create_resource_parameters()
    return jparam

def create_resource_parameters():
    """ Init resourceParams """
    return pylauncher.resourceParams()

def send_file(args):
    """ job to send a file to the cluster """
    # salome launcher
    launcher = create_launcher()

    # See SALOME_Launcher documentation for parameters
    job_params = create_job_parameters()
    job_params.job_type = "command_salome" # creates CatalogResources.xml

    local_dir = path.dirname(args.input_file)

    # job_params.pre_command = pre_command # command to run on frontal
    # script to run in batch mode
    run_script = path.join(path.dirname(args.input_file), "run.sh")
    with open(run_script, "w") as f:
        f.write("#!/bin/bash\n")
    job_params.job_file = run_script
    job_params.resource_required.nb_proc = 1

    # files to copy to remote working dir
    # Directories are copied recursively.
    # job_file script is automatically copied.
    job_params.in_files = [args.input_file]
    print("in_files", job_params.in_files)
    # local path where to copy out_files
    job_params.result_directory = local_dir

    job_params.job_name = "SMESH_transfer"
    job_params.resource_required.name = args.resource

    # remote job directory
    # Retrieve working dir from catalog
    res_manager = create_resources_manager()
    res_params = res_manager.GetResourceDefinition(args.resource)
    job_params.work_directory = path.join(\
            res_params.working_directory,
            path.basename(path.dirname(args.input_file)))

    print("work_directory", job_params.work_directory)

    job_id = launcher.createJob(job_params) #SALOME id of the job
    launcher.exportInputFiles(job_id)


def def_arg():
    """ Define and parse arguments for the script """
    parser = ArgumentParser()
    parser.add_argument("input_file",\
        help="file to copy")

    # Run parameters

    parser.add_argument("--resource",
                        help="resource from SALOME Catalog")

    args = parser.parse_args()

    return args

def main():
    """ Main function """
    args = def_arg()
    send_file(args)

if __name__ == "__main__":
    main()
