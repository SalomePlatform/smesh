# -*- coding: utf-8 -*-

import os
from blocFissure import gmu

dicoParams = dict(nomCas            = 'disque',
                  maillageSain      = os.path.join(gmu.pathBloc, 'materielCasTests/disque.med'),
                  brepFaceFissure   = os.path.join(gmu.pathBloc, "materielCasTests/ellipse_disque.brep"),
                  edgeFissIds       = [4],
                  lgInfluence       = 10,
                  meshBrep          = (0.5,2.5),
                  rayonPipe         = 1.0,
                  lenSegPipe        = 1.5,
                  nbSegRad          = 6,
                  nbSegCercle       = 16,
                  areteFaceFissure  = 2.5)

  # ---------------------------------------------------------------------------

referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 610,
                                 Entity_Quad_Triangle   = 1284,
                                 Entity_Quad_Edge       = 393,
                                 Entity_Quad_Penta      = 592,
                                 Entity_Quad_Hexa       = 6952,
                                 Entity_Node            = 51119,
                                 Entity_Quad_Tetra      = 11672,
                                 Entity_Quad_Quadrangle = 3000)

