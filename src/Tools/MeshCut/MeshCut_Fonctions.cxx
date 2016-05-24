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

#include "MeshCut_Fonctions.hxx"
#include "MeshCut_Globals.hxx"

#include <iostream>
#include <cmath>

using namespace MESHCUT;
using namespace std;

//=================================================================================================
//                                    intersectionSegmentPlan
//=================================================================================================

float MESHCUT::longueurSegment(int ngA, int ngB)
{
  float A[3], B[3];
  A[0] = MAILLAGE1->XX[ngA - 1];
  A[1] = MAILLAGE1->YY[ngA - 1];
  A[2] = MAILLAGE1->ZZ[ngA - 1];
  B[0] = MAILLAGE1->XX[ngB - 1];
  B[1] = MAILLAGE1->YY[ngB - 1];
  B[2] = MAILLAGE1->ZZ[ngB - 1];
  float dx = B[0] - A[0];
  float dy = B[1] - A[1];
  float dz = B[2] - A[2];
  return sqrt(dx * dx + dy * dy + dz * dz);
}

float MESHCUT::distanceNoeudPlan(float point[3])
{
  return normale[0] * point[0] + normale[1] * point[1] + normale[2] * point[2] + d;
}

float MESHCUT::distanceNoeudPlan(int ng)
{
  float A[3];
  A[0] = MAILLAGE1->XX[ng - 1];
  A[1] = MAILLAGE1->YY[ng - 1];
  A[2] = MAILLAGE1->ZZ[ng - 1];
  return distanceNoeudPlan(A);
}

int MESHCUT::positionNoeudPlan(int indiceNoeud)
{
  if (distanceNoeudPlan(indiceNoeud + 1) > epsilon)
    return 1;
  else if (distanceNoeudPlan(indiceNoeud + 1) < -epsilon)
    return -1;
  else
    return 0;
}

/*!
 * Equation paramétrique de la droite AB:    OP = OA + lambda AB
 *
 * Fonction caractéristique du plan : PI(X,Y,Z) = nx X + ny Y + nz Z + d
 *
 * Pour un point P de la droite: PI(OP) = PI(OA) + lambda n.AB     avec n=(nx,ny,nz),
 * L'intersection AB/plan est donnée par le point P tel que PI(OP)=0.
 *
 * Il lui correspond la coordonnée   lambda = - PI(OA) / n.AB.
 *
 * Cette intersection est dans le segment si lambda est dans [0,1]
 */
int MESHCUT::intersectionSegmentPlan(int it4, int na)
{

  int ngA = -1, ngB = -1; // Numéros des noeuds extrémités AB
  float lambda, ps; //, ab; // ab = longueur AB
  float A[3], B[3];

  // Détermination des ng des extrémités de l'arête passée en argument na
  int * offset = MAILLAGE1->CNX[TETRA4] + 4 * it4;
  if (na == 0)
    {
      ngA = *(offset + 0);
      ngB = *(offset + 1);
    }
  else if (na == 1)
    {
      ngA = *(offset + 0);
      ngB = *(offset + 2);
    }
  else if (na == 2)
    {
      ngA = *(offset + 0);
      ngB = *(offset + 3);
    }
  else if (na == 3)
    {
      ngA = *(offset + 1);
      ngB = *(offset + 2);
    }
  else if (na == 4)
    {
      ngA = *(offset + 1);
      ngB = *(offset + 3);
    }
  else if (na == 5)
    {
      ngA = *(offset + 2);
      ngB = *(offset + 3);
    }
  else
    ERREUR("Edge number superior to 6");

  string cle1 = int2string(ngA) + (string) "_" + int2string(ngB);
  string cle2 = int2string(ngB) + (string) "_" + int2string(ngA);

  if (intersections[cle1])
    return intersections[cle1];

  else
    {

      A[0] = MAILLAGE1->XX[ngA - 1];
      A[1] = MAILLAGE1->YY[ngA - 1];
      A[2] = MAILLAGE1->ZZ[ngA - 1];
      B[0] = MAILLAGE1->XX[ngB - 1];
      B[1] = MAILLAGE1->YY[ngB - 1];
      B[2] = MAILLAGE1->ZZ[ngB - 1];

      //      // Longueur AB
      //      float lx = B[0] - A[0], ly = B[1] - A[1], lz = B[2] - A[2];
      //      ab = sqrt(lx * lx + ly * ly + lz * lz);
      //      // La longueur maximale théorique est 2 epsilon
      //      if (ab < 2 * epsilon * 0.9)
      //        ERREUR("Arête de longueur inférieure au minimum théorique 2 epsilon");

      // Calcul du produit scalaire AB.n
      ps = 0.0;
      for (int k = 0; k < 3; k++)
        ps += (B[k] - A[k]) * normale[k];
      // ps = ps / ab ;

      if (debug)
        {
          cout << "Routine ISP : arête " << na << " -  ngA=" << ngA << " ngB=" << ngB << endl;
          cout << "A : " << A[0] << ' ' << A[1] << ' ' << A[2] << endl;
          cout << "B : " << B[0] << ' ' << B[1] << ' ' << B[2] << endl;
          cout << "N : " << normale[0] << ' ' << normale[1] << ' ' << normale[2] << endl;
        }

      if (fabs(ps) == 0.0)
        ERREUR("Error on null scalar product");

      // PS non nul: l'intersection AB/plan existe

      lambda = -distanceNoeudPlan(A) / ps;

      float inter[3];
      for (int k = 0; k < 3; k++)
        inter[k] = A[k] + lambda * (B[k] - A[k]);
      newXX.push_back(inter[0]);
      newYY.push_back(inter[1]);
      newZZ.push_back(inter[2]);
      indexNouveauxNoeuds++;
      intersections[cle1] = indexNouveauxNoeuds;
      intersections[cle2] = indexNouveauxNoeuds;

      //      cout << "création noeud " << indexNouveauxNoeuds << " : " << inter[0] << " " << inter[1] << " " << inter[2]
      //          << endl;
      if (debug)
        cout << " sortie nouveau noeud, lambda = " << lambda << " , noeud = " << indexNouveauxNoeuds << endl;
      return indexNouveauxNoeuds;

    }
}

