This package defines a build procedure that creates a
set of files ready to use to generate a SALOME application
embedding this module.

The files are created in the directory:

  <installdir>/share/salome/resources/<module_name>/appligen

Where <installdir> is the installation directory of the module.

To generate a SALOME application, just change directory to go
where you want to install the SALOME application and type the
following command in a standard shell (the SALOME environment
is not required, all paths are "hard" coded in the script):

  $ <installdir>/share/salome/resources/<module_name>/appligen/appligen.sh 

This script generates an application in a directory ./appli.
Then type the following command to run a SALOME application
embedding your module:

  $ ./appli/runAppli -k


