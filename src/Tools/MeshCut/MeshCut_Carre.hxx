#ifndef __MESHCUT_CARRE_HXX__
#define __MESHCUT_CARRE_HXX__

#include "MeshCut_Maillage.hxx"

namespace MESHCUT
  {
    class Carre
    {
    public:
      float x0, x1, y0, y1;
    public:
      Carre(float _x0, float _x1, float _y0, float _y1);
      bool disjoint(Carre* c2);
      bool contientNoeud(int ngnoeud, Maillage *MAILLAGE);
      void affichage();
    };
  }

#endif
