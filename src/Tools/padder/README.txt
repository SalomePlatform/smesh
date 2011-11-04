

PADDER overview
---------------

PADDER is an algorithm that creates a set of particules called a "discrete mesh".
The particules are characterized by a location in space and a weight that can be considered
as the radius of a sphere whose center is the location of the particule.

Discrete meshes are typically used to modelize civil components in rapid dynamic
computation problems (seisms, chocs). These components consists in concrete parts
embedding steal bares for reinforcement. These parts are input to the algorithm
as standard finite elements meshes. The cells of theses meshes drive the location
and sizing of particules.

In the med representation, a discrete mesh is described as MED_BALL elements.
A MED_BALL element is defined by a location and a radius. 

PADDER plugin
-------------

This directory provides SMESH with a SALOME plugin that can be used to define
and then run a PADDER execution. The inputs are the FE meshes that describe
the concrete parts and steal bares parts. The output is a discrete mesh
containing MED_BALL elements.

A graphical interface is used to drive the user for data input and computation
supervision (the algorithm may last more than an hour long), and finally the publication
of the resulting mesh (when succeed) in the SALOME study.

Technically speaking, the PADDER plugin consists in:

* a SALOME component MESHJOB that do the computation job (wrapper to the padder executable program)
* a graphical interface composed of two dialog windows
* a configuration mechanism (data file and read function), to define
  the computation resource (a SALOME resource + the software configuration of the padder executable
  program on this resource)
* an integration file (salomeplugin.py)
