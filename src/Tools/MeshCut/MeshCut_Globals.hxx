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

#ifndef __MESHCUT_GLOBALS_HXX__
#define __MESHCUT_GLOBALS_HXX__

#include "MeshCut_Maillage.hxx"

#include <map>
#include <string>
#include <vector>

namespace MESHCUT
  {
    /*! Table des points d'intersection calculés.
     *  Si on a calculé une intersection entre le plan et un segment reliant N1 et N2
     *  de numéros globaux n1 et n2, on stocke dans ce tableau, sous les libellés "n1_n2" et "n2_n1",
     *  le numéro global du point d'intersection (noeud créé).
     *  On évite ainsi de calculer deux fois l'intersection d'une même arête de T4 avec le plan
     */
    extern std::map<std::string, int> intersections;

    extern int indexNouvellesMailles, indexNouveauxNoeuds, offsetMailles;
    extern std::string str_id_GMplus, str_id_GMmoins;
    extern Maillage *MAILLAGE1, *MAILLAGE2;

    extern std::vector<float> newXX, newYY, newZZ;
    extern std::map<TYPE_MAILLE, std::vector<int> > newCNX;
    extern std::map<TYPE_MAILLE, int> cptNouvellesMailles;
    extern std::map<TYPE_MAILLE, std::vector<int> > GMplus, GMmoins;
    extern std::vector<int> cutTetras;

    extern float *DNP; //!< Distance Noeud Plan
    extern int *POSN; //!<  Version -1/0/+1 du précédent, selon epsilon

    extern std::string str_id_maillagenew;

    extern float normale[3], pointPlan[3]; //!<  Définition du plan de coupe
    extern float d; //!<  coefficient constant de l'équation du plan de coupe
    extern float epsilon; //!<  distance en dessous de laquelle un point est considéré comme appartenant au plan de coupe

    extern bool debug;
    extern int Naretes;
  }

#endif
