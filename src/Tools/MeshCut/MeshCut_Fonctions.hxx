#ifndef __MESHCUT_FONCTION_HXX__
#define __MESHCUT_FONCTION_HXX__

namespace MESHCUT
  {
    float longueurSegment(int ngA, int ngB);

    float distanceNoeudPlan(float point[3]);

    float distanceNoeudPlan(int ng);

    int positionNoeudPlan(int indiceNoeud);

    int intersectionSegmentPlan(int it4, int na);
  }

#endif
