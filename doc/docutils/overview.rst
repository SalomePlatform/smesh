
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
General presentation of the SMESH python package
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

The SMESH python package contains (today) helper functions to
manipulate mesh elements and interact with these elements.

Note that these functions either encapsulate the python programming
interface of SMESH core (the CORBA or SWIG interface for example) or
extend existing utilities as the ``smesh.py`` module.

The functions are distributed in the python package
``salome.smesh``.

The specification of the programming interface of this package is
detailled in the part :doc:`Documentation of the programming interface
(API)</docapi>` of this documentation.

.. note::
   The main package ``salome`` contains other sub-packages that are
   distributed with the other SALOME modules. For example, the KERNEL
   module provides the python package ``salome.kernel`` and GEOM the
   package ``salome.geom``.
