
.. _tui_work_on_objects_from_gui: 

***************************************
How to work with objects from the GUI ?
***************************************

It is sometimes useful to work alternatively in the GUI of SALOME and in the Python Console. To fetch an object from the TUI simply type::

  myMesh_ref = salome.IDToObject( ID )
  # were ID is a string looking like "0:1:2:3" that appears in the Object Browser in the Entry column.
  # ( If hidden, show it by right clicking and checking the checkbox Entry )
  myMesh = smesh.Mesh(myMesh_ref)

or:: 

  myMesh_ref = salome.myStudy.FindObjectByPath("/Mesh/myMesh").GetObject() 
  #'/Mesh/myMesh' is a path to the desired object in the Object Browser
  myMesh = smesh.Mesh(myMesh_ref)

or::

  # get a selected mesh
  from salome.gui import helper
  myMesh_ref = helper.getSObjectSelected()[0].GetObject() 
  myMesh = smesh.Mesh(myMesh_ref)

A result myMesh is an object of :class:`Mesh <smeshBuilder.Mesh>` class.

.. note:: The first statement only gives you access to a reference to the object created via the GUI (`myMesh_ref`). But the methods available on this reference differ from those of :class:`Mesh <smeshBuilder.Mesh>` class documented in these help pages. That's why you have to create an instance of :class:`Mesh <smeshBuilder.Mesh>` class with the last statement. 
