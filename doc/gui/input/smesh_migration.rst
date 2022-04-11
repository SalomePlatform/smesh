.. _smesh_migration_page:

******************************************************
Modifying Mesh Python scripts from SALOME 6 and before
******************************************************

In SALOME 7.2, the Python interface for Mesh has been slightly modified to offer new functionality:


Scripts generated for SALOME 6 and older versions must be adapted to work in SALOME 7.2 with full functionality.
The compatibility mode allows old scripts to work in almost all cases, but with a warning.

See also *"Modifying Geometry Python scripts from SALOME 6 and before"* page in the Geometry module's
User Guide.

* **Salome initialisation** must always be done as shown below.

  (*salome_init()* can be invoked safely several times)::

	import salome
	salome.salome_init()

* **smesh initialisation** is modified.

  The old mode (from dump)::

	import smesh, SMESH, SALOMEDS
	smesh.SetCurrentStudy(salome.myStudy)

  The new mode::

	import SMESH, SALOMEDS
	from salome.smesh import smeshBuilder
	smesh =  smeshBuilder.New()


* Of course, **from smesh import** * is **no more possible.**

  You have to explicitly write *smesh.some_method()*.

* All **algorithms** have been transferred from the namespace *smesh* to the namespace *smeshBuilder*.

  For instance::

	MEFISTO_2D_1 = Mesh_1.Triangle(algo=smesh.MEFISTO,geom=Face_1)

  is replaced by::

	MEFISTO_2D_1 = Mesh_1.Triangle(algo=smeshBuilder.MEFISTO,geom=Face_1)

  StdMeshers algorithms concerned are *REGULAR, PYTHON, COMPOSITE, MEFISTO, Hexa, QUADRANGLE, RADIAL_QUAD*.

  SMESH Plugins provide such algorithms as: *NETGEN, NETGEN_FULL, FULL_NETGEN, NETGEN_1D2D3D, NETGEN_1D2D, NETGEN_2D, NETGEN_3D*.

  If you use DISTENE plugins, you also have *BLSURF, GHS3D, GHS3DPRL, Hexotic*.

* Some **variables** were available in both namespaces *smesh* and *SMESH*. Now they are available only in namespace *SMESH*.

  The dump function used only the namespace *SMESH*,
  so, if your script was built with the help of the dump function, it should be already OK in this respect.

  The most used variables concerned are:

  * *NODE, EDGE, FACE, VOLUME, ALL.*
  * *FT_xxx, geom_xxx, ADD_xxx...*

  For instance::

	srcFaceGroup = srcMesh.GroupOnGeom( midFace0, "src faces", smesh.FACE )
	mesh.MakeGroup("Tetras",smesh.VOLUME,smesh.FT_ElemGeomType,"=",smesh.Geom_TETRA)
	filter = smesh.GetFilter(smesh.FACE, smesh.FT_AspectRatio, smesh.FT_MoreThan, 6.5)

  is replaced by::

	srcFaceGroup = srcMesh.GroupOnGeom( midFace0, "src faces", SMESH.FACE )
	mesh.MakeGroup("Tetras",SMESH.VOLUME,SMESH.FT_ElemGeomType,"=",SMESH.Geom_TETRA)
	filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_AspectRatio, SMESH.FT_MoreThan, 6.5)


* The namespace **smesh.smesh** does not exist any more, use **smesh** instead.

  For instance::

	Compound1 = smesh.smesh.Concatenate([Mesh_inf.GetMesh(), Mesh_sup.GetMesh()], 0, 1, 1e-05)

  is replaced by::

	Compound1 = smesh.Concatenate([Mesh_inf.GetMesh(), Mesh_sup.GetMesh()], 0, 1, 1e-05)

* If you need to **import a SMESH Plugin** explicitly, keep in mind that they are now located in separate namespaces.

  For instance::

	import StdMeshers
	import NETGENPlugin
	import BLSURFPlugin
	import GHS3DPlugin
	import HexoticPLUGIN

  is replaced by:: 

	from salome.StdMeshers import StdMeshersBuilder
	from salome.NETGENPlugin import NETGENPluginBuilder
	from salome.BLSURFPlugin import BLSURFPluginBuilder
	from salome.GHS3DPlugin import GHS3DPluginBuilder
	from salome.HexoticPLUGIN import HexoticPLUGINBuilder


