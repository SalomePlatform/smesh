#ifndef __MESHCUT_CUBE_HXX__
#define __MESHCUT_CUBE_HXX__

#include "MeshCut_Maillage.hxx"

namespace MESHCUT
  {
    class Cube
    {
    public:
      float x0, x1, y0, y1, z0, z1;
    public:
      Cube(float _x0, float _x1, float _y0, float _y1, float _z0, float _z1);
      bool disjoint(Cube* c2);
      bool contientNoeud(int ngnoeud, Maillage *MAILLAGE);
      void affichage();
    };
  }

#endif
