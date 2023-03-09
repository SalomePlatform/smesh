#!/usr/bin/env python3
# -*- coding: utf-8 -*-
## Copyright (C) 2021-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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
File to run mesher from command line
"""
#TODO: Make the execution path independant (output files are written in current directory)
from os import environ, path
import sys
import subprocess as sp

from argparse import ArgumentParser
import pydefx
import pylauncher

MESHER_HANDLED = ["NETGEN3D"]

CMD_TEMPLATE = \
"""{runner} {mesher} {mesh_file} {shape_file} {param_file} {elem_orientation_file} {new_element_file} {output_mesh_file} > {log_file} 2>&1"""

PYTHON_CODE = \
"""
import subprocess as sp
def _exec(cmd):
    error_code = -1
    try:
        output = sp.check_output(cmd, shell=True)
        error_code = 0
    except sp.CalledProcessError as e:
        print('Code crash')
        print(e.output)
        error_code = e.returncode
        raise e
    return error_code
"""

def create_launcher():
    """ Initialise pylauncher
    """
    launcher = pylauncher.Launcher_cpp()
    launcher.SetResourcesManager(create_resources_manager())
    return launcher

def create_resources_manager():
    """ Look for the catalog file and create a ressource manager with it """
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

def get_runner(mesher):
    """
    Get path to exe for mesher

    Arguments:
    mesher: Name of the mesher (NETGEN2D/NETGEN3D...)

    retuns (string) Path to the exe
    """
    if sys.platform.startswith('win'):
        ext = ".exe"
    else:
        ext = ""

    if mesher in ['NETGEN3D']:
        exe_path = path.join("${NETGENPLUGIN_ROOT_DIR}",
                             "bin",
                             "salome",
                             "NETGENPlugin_Runner"+ext)
    else:
        raise Exception("Mesher {mesher} is not handled".format(mesher=mesher))

    return exe_path

def run_local(args):
    """ Simple Local run """
    print("Local run")
    #TODO: Check on how to handle log for windows (through sp.check_output)
    cmd = CMD_TEMPLATE.format(\
        runner=get_runner(args.mesher),
        mesher=args.mesher,
        mesh_file=args.input_mesh_file,
        shape_file=args.shape_file,
        param_file=args.hypo_file,
        elem_orientation_file=args.elem_orient_file,
        new_element_file=args.new_element_file,
        log_file=path.join(path.dirname(args.shape_file), "run.log"),
        output_mesh_file=args.output_mesh_file)
    print("Executing:")
    print(cmd)
    sp.check_output(cmd, shell=True, cwd=path.dirname(args.shape_file))

def run_pylauncher(args):
    """ Run exe throuhg pylauncher """
    import time
    print("Cluster run")

    cmd = CMD_TEMPLATE.format(\
        runner=get_runner(args.mesher),
        mesher=args.mesher,
        mesh_file="../"+path.basename(args.input_mesh_file),
        shape_file=path.basename(args.shape_file),
        param_file=path.basename(args.hypo_file),
        elem_orientation_file=path.basename(args.elem_orient_file),
        new_element_file=path.basename(args.new_element_file),
        log_file="run.log",
        output_mesh_file=path.basename(args.output_mesh_file))

    print("Cmd: ", cmd)

    # salome launcher
    launcher = create_launcher()

    # See SALOME_Launcher documentation for parameters
    job_params = create_job_parameters()
    # different type are:
    # command Shell out of salome session
    # command_salome Shell in salome shell
    # python_salome Python script
    # yacs_file
    job_params.job_type = "command_salome" # creates CatalogResources.xml

    job_params.wckey = args.wc_key
    job_params.resource_required.nb_proc = args.nb_proc
    job_params.resource_required.nb_proc_per_node = args.nb_proc_per_node
    job_params.resource_required.nb_node = args.nb_node

    # job_params.pre_command = pre_command # command to run on frontal
    # script to run in batch mode
    run_script = path.join(path.dirname(args.shape_file), "run.sh")
    with open(run_script, "w") as f:
        f.write("#!/bin/bash\n")
        f.write(cmd)
    job_params.job_file = run_script

    local_dir = path.dirname(args.shape_file)

    # files to copy to remote working dir
    # Directories are copied recursively.
    # job_file script is automaticaly copied.
    job_params.in_files = [args.shape_file,
                           args.hypo_file,
                           args.elem_orient_file]

    print("in_files", job_params.in_files)
    # local path for in_files
    job_params.local_directory = local_dir
    # result files you want to bring back with getJobResults
    # TODO: replace run.log by argument ? by path
    out_files = ["run.log"]
    if args.new_element_file != "NONE":
        out_files.append(path.relpath(args.new_element_file, local_dir))
    if args.output_mesh_file != "NONE":
        out_files.append(path.relpath(args.output_mesh_file, local_dir))
    job_params.out_files = out_files
    print("out_files", job_params.out_files)
    # local path where to copy out_files
    job_params.result_directory = local_dir

    job_params.job_name = "SMESH_parallel"
    job_params.resource_required.name = args.resource

    # Extra parameters
    # String that is directly added to the job submission file
    # job_params.extra_params = "#SBATCH --nodes=2"

    # remote job directory
    # Retrieve working dir from catalog
    res_manager = create_resources_manager()
    res_params = res_manager.GetResourceDefinition(args.resource)
    job_params.work_directory = path.join(\
            res_params.working_directory,
            path.basename(path.dirname(path.dirname(args.shape_file))),
            path.basename(path.dirname(args.shape_file)))
    print("work directory", job_params.work_directory)

    job_id = launcher.createJob(job_params) #SALOME id of the job
    launcher.launchJob(job_id) # copy files, run pre_command, submit job

    # wait for the end of the job
    job_state = launcher.getJobState(job_id)
    print("Job %d state: %s" % (job_id, job_state))
    while job_state not in ["FINISHED", "FAILED"]:
        time.sleep(3)
        job_state = launcher.getJobState(job_id)

    if job_state == "FAILED":
        raise Exception("Job failed")
    else:
        # verify the return code of the execution
        if(launcher.getJobWorkFile(job_id,
                                   "logs/exit_code.log",
                                   job_params.result_directory)):
            exit_code_file = path.join(job_params.result_directory,
                                       "exit_code.log")
            exit_code = ""
            if path.isfile(exit_code_file):
                with open(exit_code_file) as myfile:
                    exit_code = myfile.read()
                    exit_code = exit_code.strip()
            if exit_code != "0":
                raise Exception(\
                    "An error occured during the execution of the job.")
        else:
            raise Exception("Failed to get the exit code of the job.")

    # Retrieve result files
    launcher.getJobResults(job_id, "")

    # Delete remote working dir
    launcher.clearJobWorkingDir(job_id)

def def_arg():
    """ Define and parse arguments for the script """
    parser = ArgumentParser()
    parser.add_argument("mesher",
                        choices=MESHER_HANDLED,
                        help="mesher to use from ("+",".join(MESHER_HANDLED)+")")
    parser.add_argument("input_mesh_file",\
        help="MED File containing lower-dimension-elements already meshed")
    parser.add_argument("shape_file",
                        help="STEP file containing the shape to mesh")
    parser.add_argument("hypo_file",
                        help="Ascii file containint the list of parameters")
    parser.add_argument("--elem-orient-file",\
        help="binary file containing the list of elements from "\
             "INPUT_MESH_FILE associated to the shape and their orientation")
    # Output file parameters
    output = parser.add_argument_group("Output files", "Possible output files")
    output.add_argument("--new-element-file",
                        default="NONE",
                        help="contains elements and nodes added by the meshing")
    output.add_argument(\
        "--output-mesh-file",
        default="NONE",
        help="MED File containing the mesh after the run of the mesher")

    # Run parameters
    run_param = parser.add_argument_group(\
            "Run parameters",
            "Parameters for the run of the mesher")
    run_param.add_argument("--method",
                           default="local",
                           choices=["local", "cluster"],
                           help="Running method (default: local)")

    run_param.add_argument("--resource",
                           help="resource from SALOME Catalog")
    run_param.add_argument("--nb-proc",
                           default=1,
                           type=int,
                           help="Number of processors")
    run_param.add_argument("--nb-proc-per-node",
                           default=1,
                           type=int,
                           help="Number of processeor per node")
    run_param.add_argument("--nb-node",
                           default=1,
                           type=int,
                           help="Number of node")
    run_param.add_argument("--wc-key",
                           default="P11N0:SALOME",
                           help="wc-key for job submission (default P11N0:SALOME)")

    args = parser.parse_args()

    return args

def main():
    """ Main function """
    args = def_arg()
    if args.method == "local":
        run_local(args)
    elif args.method == "cluster":
        run_pylauncher(args)
    else:
        raise Exception("Unknown method {}".format(args.method))

if __name__ == "__main__":
    main()
