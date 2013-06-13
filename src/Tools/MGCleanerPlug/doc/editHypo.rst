.. _hypothesis-label:

===========================
How to save MG-Cleaner Parameters 
===========================

MG-Cleaner hypothesis is not meshing hypothesis for Salome, but hypothesis for MG-Cleaner.
The current set of parameters is automatically written in the salome study object browser when you run computation.

Theses parameters could also be stored in a special file.
Default file is $HOME/.MGCleaner.dat.
This ASCII file is appended, and never cleaned.

In frame "Plug-in Generic Options":

- To save the current setting in this file, click on "Save" pushbutton.
- To load the last set of parameters saved, click on "Load" pushbutton.

At the bottom of the dialog window:  

- To save a current setting in the study object browser, click on "Save" pushbutton.
- To load a current setting from the study object browser, click on "Load" pushbutton.
- To load the default setting, click on "Default" pushbutton. .


**example of .MGCleaner.dat**


.. code-block:: python

   # MGCleaner hypothesis parameters
   # Params for mesh : Mesh_1
   # Date : 21/05/13 10:44:05
   # Command : mg-cleaner.exe --verbose 2 --in /tmp/ForMGCleaner_2.mesh --out /tmp/ForMGCleaner_2_fix.mesh --check --topology ignore --tolerance_displacement 0.0 --folding_angle 15.0 --overlap_angle 15.0
   CheckOrFix=check
   PreserveTopology=False
   FillHoles=False
   MinHoleSize=0.0
   ComputedToleranceDisplacement=True
   ToleranceDisplacement=0.0
   ComputedResolutionLength=False
   ResolutionLength=0.0
   FoldingAngle=15.0
   RemeshPlanes=False
   ComputedOverlapDistance=True
   OverlapDistance=0.0
   OverlapAngle=15.0
   Verbosity=2


