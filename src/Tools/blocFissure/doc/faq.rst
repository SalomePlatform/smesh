.. _faq:

#######################
FAQ
#######################

 - Does Bloc Fissure works on a 2D mesh ?

No, Bloc Fissure works only for 3D meshes. However, the user can extrude the 2D mesh to a 3D mesh, insert a crack and then extract a surface containing the crack.

 - Can I insert a crack in a symmetry plan ?

No, the user must insert the crack without using the symmetry plan passing through the crack lips.

 - I can't have Bloc Fissure working, it raises a pure python error, what can I do ?

Bloc Fissure has some error messages for the most commonly encountered misuses. If the program raises a pure python error, it means that this error is rare or has not been encountered by the programmers yet.

Still, either the case can't be treated using Bloc Fissure or the insertion procedure was not fully respected. In the first case, use Zcracks (soon available). In the second case, see the :ref:`tutorial <tutorials>`.