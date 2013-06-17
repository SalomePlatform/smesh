.. _hypothesis-label:

===========================
How to save Yams Parameters 
===========================

Yams hypothesis is not meshing hypothesis for Salome, but hypothesis for yams.
The current set of parameters is automatically written in the salome study object browser when you run computation.

Theses parameters could also be stored in a special file.
Default file is $HOME/.yams.dat.
This ASCII file is appended, and never cleaned.

In frame "Plug-in Generic Options":

- To save the current setting in this file, click on "Save" pushbutton.
- To load the last set of parameters saved, click on "Load" pushbutton.

At the bottom of the dialog window:
  
- To save a current setting in the study object browser, click on "Save" pushbutton.
- To load a current setting from the study object browser, click on "Load" pushbutton.
- To load the default setting, click on "Default" pushbutton. .


**example of .yams.dat**


.. code-block:: python

   # YAMS hypothesis parameters
   # Params for mesh : Mesh_1
   # Date : 23/05/13 14:23:18
   # Command : yams -v 3 -O 0 -Drelative,tolerance=0.100000,maxsize=0.010000,minsize=0.000000 /tmp/ForYams_1.mesh
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
   




