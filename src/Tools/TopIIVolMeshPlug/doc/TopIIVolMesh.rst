Introduction
============

**topIIvol** meshing tool provides sequential/parallel tools for creating volumetric tetrahedral meshes from a given topology (point-cloud `*.xyz`). 

Running topIIvol Plug-in
========================

**topIIVolMesh** plug-in can be invoked via SMESH Plugin item in Mesh menu bar 

.. image:: images/callTopIIVolMesh.png
   :align: center


**topIIVolMesh** Options
========================


Sequential mode
---------------
If the number of processors is set to 1, **topIIvol_Mesher** sequential tool is called for creating volumetric tetrahedral meshes from a given topology. The volumetric mesh can be displayed in SALOME by ticking the "Display mesh" check-box.

The list of input parameters are:

- DEM input file: input point cloud file;
- Number of X points present in the input point cloud;
- Number of Y points present in the input point cloud;
- Number of Z points intended in the z direction;
- Depth of the mesh needed;
- temporary directory for calculation.

Parallel mode
---------------
If the number of processors is greater than one, **topIIvol_ParMesher** parallel computing tool is called for creating volumetric tetrahedral meshes from a given topology. The volumetric mesh can be displayed in SALOME by ticking the "Display mesh" check-box.

The list of input parameters are:

- DEM input file: input point cloud file;
- Number of X points present in the input point cloud;
- Number of Y points present in the input point cloud;
- Number of Z points intended in the z direction;
- Depth of the mesh needed;
- Number of MPI ranks
- temporary directory for calculation.


Distributed mode
-----------------
If the check-box **Distributed** is ticked,  **topIIvol_DistMesher** computing tool is called for creating embarassingly parallel distributed meshes from a given topology.

The list of input parameters are:

- DEM input file: input point cloud file;
- Number of X points present in the input point cloud;
- Number of Y points present in the input point cloud;
- Number of Z points intended in the z direction;
- Number of partitions in X direction;
- Number of partitions in Y direction;
- Number of partitions in Z direction;
- Depth of the mesh needed;
- Number of MPI ranks
- temporary directory for calculation.

