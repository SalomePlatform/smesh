// Copyright (C) 2006-2016  EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

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

