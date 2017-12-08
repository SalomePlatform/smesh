.. _tui_defining_hypotheses_page:

**********************************
Defining Hypotheses and Algorithms
**********************************

This page provides example codes of :ref:`tui_defining_meshing_algos`
"defining algorithms" and hypotheses. 

* Wire discretisation 1D algorithm
  
    * :ref:`tui_1d_adaptive` hypothesis
    * :ref:`tui_1d_arithmetic` hypothesis
    * :ref:`tui_deflection_1d` hypotheses
    * :ref:`tui_start_and_end_length` hypotheses
    * :ref:`tui_average_length` 
    * :ref:`tui_propagation` additional hypothesis 
    * :ref:`tui_fixed_points` hypothesis
  

* Triangle: Mefisto 2D algorithm
  
    * :ref:`tui_max_element_area` hypothesis 
    * :ref:`tui_length_from_edges` hypothesis 
  

* NETGEN 3D algorithm
  
    *  :ref:`tui_max_element_volume` hypothesis 
    *  :ref:`tui_viscous_layers`
  

* :ref:`tui_projection`
* :ref:`tui_radial_quadrangle` algorithm
* Quadrangle: Mapping 2D algorithm
  
    * :ref:`tui_quadrangle_parameters` hypothesis 
  

* :ref:`tui_import` from Another Mesh" algorithm



Defining 1D Hypotheses
######################



.. _tui_1d_arithmetic:

Arithmetic Progression and Geometric Progression
================================================

.. _defining_hypotheses_ex01.py:

``defining_hypotheses_ex01.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex01.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex01.py`

.. _tui_1d_adaptive:

Adaptive
========

.. _defining_hypotheses_adaptive1d.py:

``defining_hypotheses_adaptive1d.py``

.. literalinclude:: ../../../examples/defining_hypotheses_adaptive1d.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_adaptive1d.py`


.. _tui_deflection_1d:

Deflection and Number of Segments
=================================

.. _defining_hypotheses_ex02.py:

``defining_hypotheses_ex02.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex02.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex02.py`


.. _tui_start_and_end_length:

Start and End Length
====================

.. _creating_meshes_ex08.py:

``defining_hypotheses_ex03.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex03.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex03.py`


.. _tui_average_length:

Local Length
============

.. _defining_hypotheses_ex04.py:

``defining_hypotheses_ex04.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex04.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex04.py`

Defining 2D and 3D hypotheses
#############################


.. _tui_max_element_area:

Maximum Element Area
====================

.. _defining_hypotheses_ex05.py:

``defining_hypotheses_ex05.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex05.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex05.py`


.. _tui_max_element_volume:

Maximum Element Volume
======================

.. _defining_hypotheses_ex06.py:

``defining_hypotheses_ex06.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex06.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex06.py`


.. _tui_length_from_edges:

Length from Edges
=================

.. _defining_hypotheses_ex07.py:

``defining_hypotheses_ex07.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex07.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex07.py`

Defining Additional Hypotheses
##############################

.. _tui_propagation:

Propagation
===========

.. _defining_hypotheses_ex08.py:

``defining_hypotheses_ex08.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex08.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex08.py`


.. _tui_defining_meshing_algos:

Defining Meshing Algorithms
###########################

.. _defining_hypotheses_ex09.py:

``defining_hypotheses_ex09.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex09.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex09.py`


.. _tui_projection:

Projection Algorithms
=====================

.. _defining_hypotheses_ex10.py:

``defining_hypotheses_ex10.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex10.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex10.py`

Projection 1D2D
===============

.. _defining_hypotheses_ex11.py:

``defining_hypotheses_ex11.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex11.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex11.py`

.. _tui_fixed_points:

1D Mesh with Fixed Points example
#################################

.. _defining_hypotheses_ex12.py:

``defining_hypotheses_ex12.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex12.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex12.py`

.. _tui_radial_quadrangle:

Radial Quadrangle 1D-2D example
###############################

.. _defining_hypotheses_ex13.py:

``defining_hypotheses_ex13.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex13.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex13.py`

.. _tui_quadrangle_parameters:

Quadrangle Parameters example 1 (meshing a face with 3 edges)
##############################################################

.. _defining_hypotheses_ex14.py:

``defining_hypotheses_ex14.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex14.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex14.py`

Quadrangle Parameters example 2 (using different types)
#######################################################

.. _defining_hypotheses_ex15.py:

``defining_hypotheses_ex15.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex15.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex15.py`

.. _tui_import:

"Import 1D-2D Elements from Another Mesh" example
#################################################

.. _defining_hypotheses_ex16.py:

``defining_hypotheses_ex16.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex16.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex16.py`

.. _tui_viscous_layers:

Viscous layers construction
###########################

.. _defining_hypotheses_ex17.py:

``defining_hypotheses_ex17.py``

.. literalinclude:: ../../../examples/defining_hypotheses_ex17.py
    :linenos:
    :language: python

:download:`../../../examples/defining_hypotheses_ex17.py`


