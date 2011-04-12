#include "MeshCut_Carre.hxx"

#include <iostream>

using namespace MESHCUT;
using namespace std;

Carre::Carre(float _x0, float _x1, float _y0, float _y1)
{
  x0 = _x0;
  x1 = _x1;
  y0 = _y0;
  y1 = _y1;
}

bool Carre::disjoint(Carre* c2)
{
  return (x0 > c2->x1 || x1 < c2->x0 || y0 > c2->y1 || y1 < c2->y0);
}

bool Carre::contientNoeud(int ngnoeud, Maillage *MAILLAGE)
{
  float x = *(MAILLAGE->XX + ngnoeud - 1);
  float y = *(MAILLAGE->YY + ngnoeud - 1);
  return (x >= x0 && x <= x1 && y >= y0 && y <= y1);
}

void Carre::affichage()
{
  cout << "x0=" << x0 << " ";
  cout << "x1=" << x1 << " ";
  cout << "y0=" << y0 << " ";
  cout << "y1=" << y1 << " ";
}

