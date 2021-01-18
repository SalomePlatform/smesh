.. _introduction:


###################
Introduction
###################

.. image:: images/intro.png
   :scale: 50
   :align: center

« Bloc Fissure » is an extension of SMESH used to insert cracks in an existing mesh. It has the advantage of meshing the front of the crack and the surrounding elements with a torus containing ruled elements. The rest of the crack contains a free mesh. The torus consists of prismatic elements (extruded triangles connected to the crack front) and hexahedrons elsewhere. The main interests of this type of mesh are as follows :

- Having sets of Gauss points in plans perpendicular to the crack front in order to calculate stress field without any interpolation, which would be the case on free mesh. It avoids strong oscillations along the crack front on the energy release rate and stress intensity factors calculated by `extrapolation <http://code-aster.org/doc/default/fr/man_r/r7/r7.02.08.pdf>`_ or `G-theta method <http://code-aster.org/doc/default/fr/man_r/r7/r7.02.01.pdf>`_.

- Decrease in the number of elements. In fracture mechanics, a fine mesh is required radially at the fissure front and more rarely in the axis of the front. Hexhedral and prismatic elements can have geometric aspect ratios of up to 20 without major matrix conditioning problems. On the other hand, the tetrahedrons of free meshes are generally limited to aspect ratios of about 3. The use of very elongated elements is then no longer a limitation and the number of elements can be considerably reduced.

« Bloc Fissure » is not always usable. It is highly recommended to read the section on :ref:`general principles <general_principles>` to see how « Bloc Fissure » works. This section also gives the functional scope and the :ref:`limitations <recommendations>` of the tool. Finally, this part deals with cautions that the user must take using « Bloc Fissure ». The user can also refers himself to the :ref:`tutorial <tutorials>` to get some advice on how to make « Bloc Fissure » works.

If « Bloc Fissure » cannot be used on a case, the user can switch to the other Zcracks insertion tool in SALOME. It is more robust and has fewer limitations, but the result is a free mesh of the crack with tetrahedral elements. Another possibility is the `X-FEM method <http://www.code-aster.org/doc/v11/fr/man_u/u2/u2.05.02.pdf>`_ method in SALOME_MECA.


