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

#include "MeshCut_Cas.hxx"

#include "MeshCut_Globals.hxx"

using namespace MESHCUT;
using namespace std;

/*!
 * Le cas 1 traduit le fait que deux des sommets du T4 initial sont dans le plan de coupe.
 * Le point d'intersection franc trouvé est sur l'arête opposée à ces deux points du T4.

 * Le T4 initial produit deux nouveaux T4.
 */
void MESHCUT::cas1(int VN[6], int it4)
{
  cutTetras.push_back(it4);
  //  cout << "Cas 1 - it4=" << it4 << ", VN = " << VN[0] << " " << VN[1] << " " << VN[2] << " " << VN[3] << " " << VN[4]
  //      << " " << VN[5] << " " << endl;

  // Numéros des noeuds du TETRA4
  int ng0 = MAILLAGE1->CNX[TETRA4][4 * it4 + 0];
  int ng1 = MAILLAGE1->CNX[TETRA4][4 * it4 + 1];
  int ng2 = MAILLAGE1->CNX[TETRA4][4 * it4 + 2];
  int ng3 = MAILLAGE1->CNX[TETRA4][4 * it4 + 3];

  int i1, i2;

  if (VN[0] != -1)
    {
      // Le sommet de T4new1 servant à la détermination du groupe est le noeud 1 du T4 d'origine

      // cout << "cas 1/0" << endl;
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i1 = cptNouvellesMailles[TETRA4] - 1;

      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i2 = cptNouvellesMailles[TETRA4] - 1;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
        }
      else
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
        }
    }

  else if (VN[1] != -1)
    {
      // Le sommet de T4new1 servant à la détermination du groupe est le noeud 0 du T4 d'origine

      // cout << "cas 1/1" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i1 = cptNouvellesMailles[TETRA4] - 1;

      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i2 = cptNouvellesMailles[TETRA4] - 1;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
        }
      else
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
        }
    }

  else if (VN[2] != -1)
    {
      // Le sommet de T4new1 servant à la détermination du groupe est le noeud 0 du T4 d'origine

      // cout << "cas 1/2" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[2]);
      cptNouvellesMailles[TETRA4]++;
      i1 = cptNouvellesMailles[TETRA4] - 1;

      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[2]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i2 = cptNouvellesMailles[TETRA4] - 1;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
        }
      else
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
        }
    }

  else if (VN[3] != -1)
    {
      // Le sommet de T4new1 servant à la détermination du groupe est le noeud 1 du T4 d'origine

      // cout << "cas 1/3" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i1 = cptNouvellesMailles[TETRA4] - 1;

      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i2 = cptNouvellesMailles[TETRA4] - 1;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
        }
      else
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
        }
    }

  else if (VN[4] != -1)
    {
      // Le sommet de T4new1 servant à la détermination du groupe est le noeud 1 du T4 d'origine

      // cout << "cas 1/4" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[4]);
      cptNouvellesMailles[TETRA4]++;
      i1 = cptNouvellesMailles[TETRA4] - 1;

      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(VN[4]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i2 = cptNouvellesMailles[TETRA4] - 1;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
        }
      else
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
        }
    }

  else if (VN[5] != -1)
    {
      // Le sommet de T4new1 servant à la détermination du groupe est le noeud 3 du T4 d'origine

      // cout << "cas 1/5" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(VN[5]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;
      i1 = cptNouvellesMailles[TETRA4] - 1;

      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[5]);
      cptNouvellesMailles[TETRA4]++;
      i2 = cptNouvellesMailles[TETRA4] - 1;

      if (POSN[ng3 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
        }
      else
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2);
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1);
        }
    }

  else
    ERREUR(" Intersections configuration not taken into account (case cptPI=1) ");

  //int nl1 = MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i1;
  //int nl2 = MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + i2;
  // cout << "La maille TETRA4 " << it4 << " produit les mailles TETRA4-" << nl1 << " et TETRA4-" << nl2 << endl;

}

/*!  Deux points d'intersection
 *  Le cas 2 traduit le fait qu'un des sommets du T4 est dans le plan de coupe.
 *  Ce sommet est celui des quatre qui n'appartient à aucune des deux arêtes sur lesquelles
 *  un point d'intersection non -1 a été calculé.
 *
 *  Le T4 initial produit un nouveau T4 et un élément PYRAM5.
 *
 */
void MESHCUT::cas2(int VN[6], int it4)
{
  cutTetras.push_back(it4);
  //  cout << "Cas 2 - it4=" << it4 << ", VN = " << VN[0] << " " << VN[1] << " " << VN[2] << " " << VN[3] << " " << VN[4]
  //      << " " << VN[5] << " " << endl;

  // Numéros des noeuds du TETRA4
  int ng0 = MAILLAGE1->CNX[TETRA4][4 * it4 + 0];
  int ng1 = MAILLAGE1->CNX[TETRA4][4 * it4 + 1];
  int ng2 = MAILLAGE1->CNX[TETRA4][4 * it4 + 2];
  int ng3 = MAILLAGE1->CNX[TETRA4][4 * it4 + 3];

  if (VN[0] != -1 && VN[1] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 0 du T4 d'origine

      // cout << "cas 2.01" << endl;
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(ng3);
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(ng0);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(VN[0]);
      newCNX[PYRAM5].push_back(ng1);
      newCNX[PYRAM5].push_back(ng2);
      newCNX[PYRAM5].push_back(VN[1]);
      newCNX[PYRAM5].push_back(ng3);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }
  else if (VN[0] != -1 && VN[2] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 0 du T4 d'origine

      // cout << "cas 2.02" << endl;
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(VN[2]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng0);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(VN[0]);
      newCNX[PYRAM5].push_back(VN[2]);
      newCNX[PYRAM5].push_back(ng3);
      newCNX[PYRAM5].push_back(ng1);
      newCNX[PYRAM5].push_back(ng2);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }
  else if (VN[0] != -1 && VN[3] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 1 du T4 d'origine

      // cout << "cas 2.03" << endl;
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(ng3);
      newCNX[TETRA4].push_back(ng1);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(VN[0]);
      newCNX[PYRAM5].push_back(VN[3]);
      newCNX[PYRAM5].push_back(ng2);
      newCNX[PYRAM5].push_back(ng0);
      newCNX[PYRAM5].push_back(ng3);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }
  else if (VN[0] != -1 && VN[4] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 1 du T4 d'origine

      // cout << "cas 2.04" << endl;
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[4]);
      newCNX[TETRA4].push_back(ng1);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(ng0);
      newCNX[PYRAM5].push_back(ng3);
      newCNX[PYRAM5].push_back(VN[4]);
      newCNX[PYRAM5].push_back(VN[0]);
      newCNX[PYRAM5].push_back(ng2);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[0] != -1 && VN[5] != -1)
    ERREUR("Case 2/05 forbidden");

  else if (VN[1] != -1 && VN[2] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 0 du T4 d'origine

      // cout << "cas 2.12" << endl;
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(VN[2]);
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(ng0);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(ng2);
      newCNX[PYRAM5].push_back(ng3);
      newCNX[PYRAM5].push_back(VN[2]);
      newCNX[PYRAM5].push_back(VN[1]);
      newCNX[PYRAM5].push_back(ng1);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }
  else if (VN[1] != -1 && VN[3] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 2 du T4 d'origine

      // cout << "cas 2.13" << endl;
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(ng3);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(ng2);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(ng0);
      newCNX[PYRAM5].push_back(ng1);
      newCNX[PYRAM5].push_back(VN[3]);
      newCNX[PYRAM5].push_back(VN[1]);
      newCNX[PYRAM5].push_back(ng3);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng2 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[1] != -1 && VN[4] != -1)
    ERREUR("Case 2/14 excluded");

  else if (VN[1] != -1 && VN[5] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 2 du T4 d'origine

      // cout << "cas 2.15" << endl;
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(VN[5]);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(VN[1]);
      newCNX[PYRAM5].push_back(VN[5]);
      newCNX[PYRAM5].push_back(ng3);
      newCNX[PYRAM5].push_back(ng0);
      newCNX[PYRAM5].push_back(ng1);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng2 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[2] != -1 && VN[3] != -1)
    ERREUR("Case 2/23 excluded");

  else if (VN[2] != -1 && VN[4] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 3 du T4 d'origine

      // cout << "cas 2.24" << endl;
      newCNX[TETRA4].push_back(VN[2]);
      newCNX[TETRA4].push_back(VN[4]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(ng0);
      newCNX[PYRAM5].push_back(VN[2]);
      newCNX[PYRAM5].push_back(VN[4]);
      newCNX[PYRAM5].push_back(ng1);
      newCNX[PYRAM5].push_back(ng2);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng3 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[2] != -1 && VN[5] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 3 du T4 d'origine

      // cout << "cas 2.25" << endl;
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(VN[5]);
      newCNX[TETRA4].push_back(VN[2]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(ng0);
      newCNX[PYRAM5].push_back(ng2);
      newCNX[PYRAM5].push_back(VN[5]);
      newCNX[PYRAM5].push_back(VN[2]);
      newCNX[PYRAM5].push_back(ng1);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng3 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[3] != -1 && VN[4] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 1 du T4 d'origine

      // cout << "cas 2.34" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(VN[4]);
      newCNX[TETRA4].push_back(ng1);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(VN[3]);
      newCNX[PYRAM5].push_back(VN[4]);
      newCNX[PYRAM5].push_back(ng3);
      newCNX[PYRAM5].push_back(ng2);
      newCNX[PYRAM5].push_back(ng0);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[3] != -1 && VN[5] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 2 du T4 d'origine

      // cout << "cas 2.35" << endl;
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(VN[5]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(ng0);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(VN[3]);
      newCNX[PYRAM5].push_back(ng1);
      newCNX[PYRAM5].push_back(ng3);
      newCNX[PYRAM5].push_back(VN[5]);
      newCNX[PYRAM5].push_back(ng0);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng2 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else if (VN[4] != -1 && VN[5] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 3 du T4 d'origine

      // cout << "cas 2.35" << endl;
      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(VN[4]);
      newCNX[TETRA4].push_back(VN[5]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PYRAM5].push_back(ng1);
      newCNX[PYRAM5].push_back(VN[4]);
      newCNX[PYRAM5].push_back(VN[5]);
      newCNX[PYRAM5].push_back(ng2);
      newCNX[PYRAM5].push_back(ng0);
      cptNouvellesMailles[PYRAM5]++;

      if (POSN[ng3 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PYRAM5].push_back(MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1);
        }
    }

  else
    ERREUR(" Intersections configuration not taken into account (case cptPI=2) ");

  // int ngT4 = MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1;
  // int ngP5 = MAILLAGE1->EFFECTIFS_TYPES[PYRAM5] + cptNouvellesMailles[PYRAM5] - 1;
  // cout << "La maille TETRA4 " << it4 << " produit les mailles TETRA4-" << ngT4 << " et PYRAM5-" << ngP5 << endl;

}

/*! Trois points d'intersection
 *  ATTENTION: pour les PENTA6 on adopte la convention d'orientation SALOME :
 *
 *  N1 N2 N3 N4 N5 N6
 *
 *  où N1 N2 N3 sont les sommets du haut et N4 N5 N6 les sommets du bas
 *  (selon l'orientation donnée par le sens des triangles)
 */
void MESHCUT::cas3(int VN[6], int it4)
{
  cutTetras.push_back(it4);
  // cout << "Cas 3 - it4="<<it4<<", VN = " << VN[0] << " " << VN[1] << " " << VN[2] << " " << VN[3] << " " << VN[4] << " " << VN[5] << " " << endl;

  // Numéros des noeuds du TETRA4
  int ng0 = MAILLAGE1->CNX[TETRA4][4 * it4 + 0];
  int ng1 = MAILLAGE1->CNX[TETRA4][4 * it4 + 1];
  int ng2 = MAILLAGE1->CNX[TETRA4][4 * it4 + 2];
  int ng3 = MAILLAGE1->CNX[TETRA4][4 * it4 + 3];

  if (VN[0] != -1 && VN[1] != -1 && VN[2] != -1)
    {

      // Le sommet du nouveau T4 est le noeud 0 du T4 d'origine

      newCNX[TETRA4].push_back(ng0);
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(VN[2]);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PENTA6].push_back(ng1);
      newCNX[PENTA6].push_back(ng3);
      newCNX[PENTA6].push_back(ng2);
      newCNX[PENTA6].push_back(VN[0]);
      newCNX[PENTA6].push_back(VN[2]);
      newCNX[PENTA6].push_back(VN[1]);
      cptNouvellesMailles[PENTA6]++;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
    }

  else if (VN[0] != -1 && VN[3] != -1 && VN[4] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 1 du T4 d'origine

      //cout << "cas 3.2 (noeud sommet 1)" << endl;
      newCNX[TETRA4].push_back(VN[0]);
      newCNX[TETRA4].push_back(ng1);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(VN[4]);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PENTA6].push_back(ng0);
      newCNX[PENTA6].push_back(ng2);
      newCNX[PENTA6].push_back(ng3);
      newCNX[PENTA6].push_back(VN[0]);
      newCNX[PENTA6].push_back(VN[3]);
      newCNX[PENTA6].push_back(VN[4]);
      cptNouvellesMailles[PENTA6]++;

      if (POSN[ng1 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
    }
  else if (VN[1] != -1 && VN[3] != -1 && VN[5] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 2 du T4 d'origine

      //cout << "cas 3.3 (noeud sommet 2)" << endl;

      newCNX[TETRA4].push_back(VN[1]);
      newCNX[TETRA4].push_back(VN[3]);
      newCNX[TETRA4].push_back(ng2);
      newCNX[TETRA4].push_back(VN[5]);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PENTA6].push_back(ng0);
      newCNX[PENTA6].push_back(ng3);
      newCNX[PENTA6].push_back(ng1);
      newCNX[PENTA6].push_back(VN[1]);
      newCNX[PENTA6].push_back(VN[5]);
      newCNX[PENTA6].push_back(VN[3]);
      cptNouvellesMailles[PENTA6]++;

      if (POSN[ng2 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
    }
  else if (VN[2] != -1 && VN[4] != -1 && VN[5] != -1)
    {
      // Le sommet du nouveau T4 est le noeud 3 du T4 d'origine

      newCNX[TETRA4].push_back(VN[2]);
      newCNX[TETRA4].push_back(VN[4]);
      newCNX[TETRA4].push_back(VN[5]);
      newCNX[TETRA4].push_back(ng3);
      cptNouvellesMailles[TETRA4]++;

      newCNX[PENTA6].push_back(ng0);
      newCNX[PENTA6].push_back(ng1);
      newCNX[PENTA6].push_back(ng2);
      newCNX[PENTA6].push_back(VN[2]);
      newCNX[PENTA6].push_back(VN[4]);
      newCNX[PENTA6].push_back(VN[5]);
      cptNouvellesMailles[PENTA6]++;

      if (POSN[ng3 - 1] == 1)
        {
          GMplus[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
      else
        {
          GMmoins[TETRA4].push_back(MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1);
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1);
        }
    }
  else
    {
      //      cout << "Cas 3 - it4=" << it4 << ", VN = " << VN[0] << " " << VN[1] << " " << VN[2] << " " << VN[3] << " "
      //          << VN[4] << " " << VN[5] << " " << endl;
      //
      //      int n0 = *(MAILLAGE1->CNX[TETRA4] + it4 * 4 + 0);
      //      float x0 = MAILLAGE1->XX[n0 - 1];
      //      float y0 = MAILLAGE1->YY[n0 - 1];
      //      float z0 = MAILLAGE1->ZZ[n0 - 1];
      //
      //      int n1 = *(MAILLAGE1->CNX[TETRA4] + it4 * 4 + 1);
      //      float x1 = MAILLAGE1->XX[n1 - 1];
      //      float y1 = MAILLAGE1->YY[n1 - 1];
      //      float z1 = MAILLAGE1->ZZ[n1 - 1];
      //
      //      int n2 = *(MAILLAGE1->CNX[TETRA4] + it4 * 4 + 2);
      //      float x2 = MAILLAGE1->XX[n2 - 1];
      //      float y2 = MAILLAGE1->YY[n2 - 1];
      //      float z2 = MAILLAGE1->ZZ[n2 - 1];
      //
      //      int n3 = *(MAILLAGE1->CNX[TETRA4] + it4 * 4 + 3);
      //      float x3 = MAILLAGE1->XX[n3 - 1];
      //      float y3 = MAILLAGE1->YY[n3 - 1];
      //      float z3 = MAILLAGE1->ZZ[n3 - 1];
      //
      //      cout << x0 << " " << y0 << " " << z0 << " " << endl;
      //      cout << x1 << " " << y1 << " " << z1 << " " << endl;
      //      cout << x2 << " " << y2 << " " << z2 << " " << endl;
      //      cout << x3 << " " << y3 << " " << z3 << " " << endl;

      ERREUR(" Intersections configuration not taken into account (case cptPI=3) ");
    }

  // int ngT4 = MAILLAGE1->EFFECTIFS_TYPES[TETRA4] + cptNouvellesMailles[TETRA4] - 1;
  // int ngP6 = MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + cptNouvellesMailles[PENTA6] - 1;
  // cout << "La maille TETRA4 " << it4 << " produit les mailles TETRA4-" << ngT4 << " et PENTA6-" << ngP6 << endl;
}

/*! Quatre points d'intersection
 *
 *  ATTENTION: pour les PENTA6 on adopte la convention d'orientation SALOME
 *
 *  N1 N2 N3 N4 N5 N6
 *
 *  où N1 N2 N3 sont les sommets du haut et N4 N5 N6 les sommets du bas
 *  (selon l'orientation donnée par le sens des triangles)
 */
void MESHCUT::cas4(int VN[6], int it4)
{
  cutTetras.push_back(it4);
  //  cout << "Cas 4 - it4=" << it4 << ", VN = " << VN[0] << " " << VN[1] << " " << VN[2] << " " << VN[3] << " " << VN[4]
  //      << " " << VN[5] << " " << endl;

  // Numéros des noeuds du TETRA4
  int ng0 = MAILLAGE1->CNX[TETRA4][4 * it4 + 0];
  int ng1 = MAILLAGE1->CNX[TETRA4][4 * it4 + 1];
  int ng2 = MAILLAGE1->CNX[TETRA4][4 * it4 + 2];
  int ng3 = MAILLAGE1->CNX[TETRA4][4 * it4 + 3];

  int i1, i2; // Numéros locaux dans le type des mailles créées

  if (VN[0] == -1 && VN[5] == -1)
    {
      // Les deux arêtes opposées sont [0,1] et [2,3]

      newCNX[PENTA6].push_back(ng0);
      newCNX[PENTA6].push_back(VN[1]);
      newCNX[PENTA6].push_back(VN[2]);
      newCNX[PENTA6].push_back(ng1);
      newCNX[PENTA6].push_back(VN[3]);
      newCNX[PENTA6].push_back(VN[4]);
      cptNouvellesMailles[PENTA6]++;
      i1 = cptNouvellesMailles[PENTA6] - 1;

      newCNX[PENTA6].push_back(ng3);
      newCNX[PENTA6].push_back(VN[4]);
      newCNX[PENTA6].push_back(VN[2]);
      newCNX[PENTA6].push_back(ng2);
      newCNX[PENTA6].push_back(VN[3]);
      newCNX[PENTA6].push_back(VN[1]);
      cptNouvellesMailles[PENTA6]++;
      i2 = cptNouvellesMailles[PENTA6] - 1;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2);
        }
      else
        {
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1);
        }

    } // if ( VN[0]==-1 && VN[5]==-1 )

  else if (VN[1] == -1 && VN[4] == -1)
    {
      // Les deux arêtes opposées sont [0,2] et [1,3]

      newCNX[PENTA6].push_back(ng2);
      newCNX[PENTA6].push_back(VN[3]);
      newCNX[PENTA6].push_back(VN[5]);
      newCNX[PENTA6].push_back(ng0);
      newCNX[PENTA6].push_back(VN[0]);
      newCNX[PENTA6].push_back(VN[2]);
      cptNouvellesMailles[PENTA6]++;
      i1 = cptNouvellesMailles[PENTA6] - 1;

      newCNX[PENTA6].push_back(ng1);
      newCNX[PENTA6].push_back(VN[3]);
      newCNX[PENTA6].push_back(VN[0]);
      newCNX[PENTA6].push_back(ng3);
      newCNX[PENTA6].push_back(VN[5]);
      newCNX[PENTA6].push_back(VN[2]);
      cptNouvellesMailles[PENTA6]++;
      i2 = cptNouvellesMailles[PENTA6] - 1;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2);
        }
      else
        {
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1);
        }
    }

  else if (VN[2] == -1 && VN[3] == -1)
    {
      // Les deux arêtes opposées sont [0,3] et [1,2]

      newCNX[PENTA6].push_back(ng0);
      newCNX[PENTA6].push_back(VN[0]);
      newCNX[PENTA6].push_back(VN[1]);
      newCNX[PENTA6].push_back(ng3);
      newCNX[PENTA6].push_back(VN[4]);
      newCNX[PENTA6].push_back(VN[5]);
      cptNouvellesMailles[PENTA6]++;
      i1 = cptNouvellesMailles[PENTA6] - 1;

      newCNX[PENTA6].push_back(ng2);
      newCNX[PENTA6].push_back(VN[5]);
      newCNX[PENTA6].push_back(VN[1]);
      newCNX[PENTA6].push_back(ng1);
      newCNX[PENTA6].push_back(VN[4]);
      newCNX[PENTA6].push_back(VN[0]);
      cptNouvellesMailles[PENTA6]++;
      i2 = cptNouvellesMailles[PENTA6] - 1;

      if (POSN[ng0 - 1] == 1)
        {
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2);
        }
      else
        {
          GMplus[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2);
          GMmoins[PENTA6].push_back(MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1);
        }
    }
  else
    ERREUR(" Intersection configuration not taken into account (case cptPI=4) ");

  // int nl1 = MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i1;
  // int nl2 = MAILLAGE1->EFFECTIFS_TYPES[PENTA6] + i2;
  // cout << "La maille TETRA4 " << it4 << " produit les mailles PENTA6-" << nl1 << " et PENTA6-" << nl2 << endl;
}
