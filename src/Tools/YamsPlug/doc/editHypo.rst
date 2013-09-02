.. _hypothesis-label:

===========================
How to save MeshGems-SurfOpt Parameters 
===========================

MeshGems-SurfOpt hypothesis is not a meshing hypothesis for Salome, but an hypothesis for MeshGems-SurfOpt.
The current set of parameters is automatically written in the salome study object browser when you run computation.

Theses parameters could also be stored in a special file.
Default file is $HOME/.MGSurfOpt.dat.
This ASCII file is appended, and never cleaned.

In frame "Plug-in Generic Options":

- To save the current setting in this file, click on "Save" pushbutton.
- To load the last set of parameters saved, click on "Load" pushbutton.

At the bottom of the dialog window:
  
- To save a current setting in the study object browser, click on "Save" pushbutton.
- To load a current setting from the study object browser, click on "Load" pushbutton.
- To load the default setting, click on "Default" pushbutton. .


**example of .MGSurfOpt.dat**


.. code-block:: python

   # MGSurfOpt hypothesis parameters
   # Params for mesh : Mesh_1
   # Date : 23/05/13 14:23:18
   # Command : mg-surfopt.exe --verbose 3 --chordal_error 0.1 --max_size 0.01 --minsize 0.0 --in /tmp/ForSurfOpt_1.mesh
   Optimisation=Quality improvement Only (0)
   Units=Relative
   ChordalToleranceDeviation=0.1
   RidgeDetection=True
   SplitEdge=False
   PointSmoothing=True
   GeometricalApproximation=0.04
   RidgeAngle=45.0
   MaximumSize=0.01
   MinimumSize=0.0
   MeshGradation=1.3
   Verbosity=3
   Memory=0
   




