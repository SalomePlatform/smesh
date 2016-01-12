# -*- coding: utf-8 -*-

import os
from blocFissure import gmu

dicoParams = dict(nomCas            = 'cubeMilieu',
                  maillageSain      = os.path.join(gmu.pathBloc, 'materielCasTests/cubeFin.med'),
                  brepFaceFissure   = os.path.join(gmu.pathBloc, "materielCasTests/cubeFin_Milieu.brep"),
                  edgeFissIds       = [7],
                  lgInfluence       = 50,
                  meshBrep          = (5,10),
                  rayonPipe         = 10,
                  lenSegPipe        = 7,
                  nbSegRad          = 5,
                  nbSegCercle       = 10,
                  areteFaceFissure  = 10)

# ---------------------------------------------------------------------------

referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 0,
                                 Entity_Quad_Triangle   = 0,
                                 Entity_Quad_Edge       = 0,
                                 Entity_Quad_Penta      = 0,
                                 Entity_Quad_Hexa       = 0,
                                 Entity_Node            = 0,
                                 Entity_Quad_Tetra      = 0,
                                 Entity_Quad_Quadrangle = 0)

