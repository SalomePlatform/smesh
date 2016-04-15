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
