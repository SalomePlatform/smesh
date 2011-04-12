#include "MeshCut_Cube.hxx"

#include <iostream>

using namespace MESHCUT;
using namespace std;

Cube::Cube(float _x0, float _x1, float _y0, float _y1, float _z0, float _z1)
{
  x0 = _x0;
  x1 = _x1;
  y0 = _y0;
  y1 = _y1;
  z0 = _z0;
  z1 = _z1;
}

bool Cube::disjoint(Cube* c2)
{
  return (x0 > c2->x1 || x1 < c2->x0 || y0 > c2->y1 || y1 < c2->y0 || z0 > c2->z1 || z1 < c2->z0);
}

bool Cube::contientNoeud(int ngnoeud, Maillage *MAILLAGE)
{
  float x = *(MAILLAGE->XX + ngnoeud - 1);
  float y = *(MAILLAGE->YY + ngnoeud - 1);
  float z = *(MAILLAGE->ZZ + ngnoeud - 1);
  return (x >= x0 && x <= x1 && y >= y0 && y <= y1 && z >= z0 && z <= z1);
}

void Cube::affichage()
{
  cout << "x0=" << x0 << " ";
  cout << "x1=" << x1 << " ";
  cout << "y0=" << y0 << " ";
  cout << "y1=" << y1 << " ";
  cout << "z0=" << z0 << " ";
  cout << "z1=" << z1 << " ";
}
