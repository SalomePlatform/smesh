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

// Classes et fonctions XMeshLab

#include "MeshCut_Utils.hxx"
#include "MeshCut_Maillage.hxx"

#include "MeshCut_Carre.hxx"
#include "MeshCut_Cube.hxx"

#include "MeshCut_Fonctions.hxx"
#include "MeshCut_Cas.hxx"

#include "MeshCut_Globals.hxx"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>

using namespace MESHCUT;
using namespace std;

// ==================================  DECLARATION DES VARIABLES GLOBALES  ==================================================

std::map<std::string, int> MESHCUT::intersections;

int MESHCUT::indexNouvellesMailles, MESHCUT::indexNouveauxNoeuds, MESHCUT::offsetMailles;
std::string MESHCUT::str_id_GMplus, MESHCUT::str_id_GMmoins;
Maillage *MESHCUT::MAILLAGE1, *MESHCUT::MAILLAGE2;

std::vector<float> MESHCUT::newXX, MESHCUT::newYY, MESHCUT::newZZ;
std::map<TYPE_MAILLE, std::vector<int> > MESHCUT::newCNX;
std::map<TYPE_MAILLE, int> MESHCUT::cptNouvellesMailles;
std::map<TYPE_MAILLE, std::vector<int> > MESHCUT::GMplus, MESHCUT::GMmoins;
std::vector<int> MESHCUT::cutTetras;

float *MESHCUT::DNP;
int *MESHCUT::POSN;

std::string MESHCUT::str_id_maillagenew;

float MESHCUT::normale[3], MESHCUT::pointPlan[3];
float MESHCUT::d;
float MESHCUT::epsilon;

bool MESHCUT::debug;
int MESHCUT::Naretes;

// ==================================   PROGRAMME PRINCIPAL  ==================================================

int main(int argc, char *argv[])
{

  debug = false;
  string ficMEDin;
  string ficMEDout;
  float xNormal = 0;
  float yNormal = 0;
  float zNormal = 0;
  float xm = 0;
  float ym = 0;
  float zm = 0;
  float tolerance = 0;
  try
    {
      if (argc != 13)
        throw std::exception();
      char *ficMEDin0 = argv[1];
      ficMEDin = (string) ficMEDin0;
      char *ficMEDout0 = argv[2];
      ficMEDout = (string) ficMEDout0;
      char *id_maillagenew = argv[3];
      str_id_maillagenew = (string) id_maillagenew;

      // Groupes créés
      char *id_GMplus = argv[4];
      str_id_GMplus = (string) id_GMplus;
      char *id_GMmoins = argv[5];
      str_id_GMmoins = (string) id_GMmoins;

      // Vecteur normal au plan de coupe
      char *charxn = argv[6];
      xNormal = char2float(charxn);
      char *charyn = argv[7];
      yNormal = char2float(charyn);
      char *charzn = argv[8];
      zNormal = char2float(charzn);

      // Point du plan de coupe
      char *charxm = argv[9];
      xm = char2float(charxm);
      char *charym = argv[10];
      ym = char2float(charym);
      char *charzm = argv[11];
      zm = char2float(charzm);

      // Tolérance :  epsilon = tolérance * longueur arête moyenne - où epsilon est la tolérance absolue (distance)
      char *chtolerance = argv[12];
      tolerance = char2float(chtolerance);
    }
  catch (...)
    {
      cout << endl;
      cout << "                 Cut a tetrahedron mesh by a plane" << endl;
      cout << "                 ---------------------------------" << endl;
      cout << "Syntax:" << endl << endl;
      cout << argv[0] << " input.med output.med resuMeshName aboveGroup belowGroup nx ny nz px py pz T " << endl;
      cout << endl << "where:" << endl;
      cout << "  input.med    = name of the original mesh file in med format" << endl;
      cout << "  output.med   = name of the result mesh file in med format" << endl;
      cout << "  resuMeshName = name of the result mesh" << endl;
      cout << "  aboveGroup   = name of the group of volumes above the cut plane" << endl;
      cout << "  belowGroups  = name of the group of volumes below the cut plane" << endl;
      cout << "  nx ny nz     = vector normal to the cut plane" << endl;
      cout << "  px py pz     = a point of the cut plane" << endl;
      cout << "  T            = 0 < T < 1 : vertices of a tetrahedron are considered as belonging" << endl;
      cout << "                 the cut plane if their distance to the plane is inferior to L*T" << endl;
      cout << "                 where L is the mean edge size of the tetrahedron" << endl;
      ERREUR("--> check arguments!");
    }

  cout << "Cut by a plane :" << endl;
  cout << "  source mesh: " << ficMEDin << endl;
  cout << "  result mesh:  " << ficMEDout << endl;
  cout << "  mesh name:  " << str_id_maillagenew << endl;
  cout << "  group above plane:  " << str_id_GMplus << endl;
  cout << "  group below plane: " << str_id_GMmoins << endl;
  cout << "  vector normal to the cut plane: xn=" << xNormal << " yn=" << yNormal << " zn=" << zNormal << endl;
  cout << "  point in the cut plane: xm=" << xm << " ym=" << ym << " zm=" << zm << endl;
  cout << "  tolerance: " << tolerance << endl;
  cout << endl;

  if (tolerance <= 0.0)
    ERREUR("Tolerance must not be negative or null");

  // Il faut normer la normale
  float normeNormal = sqrt(xNormal * xNormal + yNormal * yNormal + zNormal * zNormal);
  if (normeNormal == 0.0)
    ERREUR("null normal vector");
  normale[0] = xNormal / normeNormal;
  normale[1] = yNormal / normeNormal;
  normale[2] = zNormal / normeNormal;

  pointPlan[0] = xm;
  pointPlan[1] = ym;
  pointPlan[2] = zm;

  // Calcul du coefficient d de l'équation du plan  xn x + yn y + zn n + d = 0
  d = -normale[0] * xm - normale[1] * ym - normale[2] * zm;

  intersections.clear();

  // Initialisation des compteurs de nouvelles mailles
  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      cptNouvellesMailles[tm] = 0;
    }

  int V[6];
  int S[4]; // Signature du T4 courant
  //int NG[4]; // Num. globaux des sommets

  // Acquisition maillage initial
  //cout << chrono() << " - Acquisition du maillage initial" << endl;
  MAILLAGE1 = new Maillage((string) "TEMP");
  MAILLAGE1->inputMED(ficMEDin);
  cout << chrono() << " - End of mesh read" << endl;
  indexNouveauxNoeuds = MAILLAGE1->nombreNoeudsMaillage;

  // Le maillage ne contient aucun TETRA4 : on rend le maillage initial sans modification
  if (!MAILLAGE1->EFFECTIFS_TYPES[TETRA4])
    {
      cout << "WARNING: mesh does not contain tetra4 elements, it will not be modified" << endl;
      MAILLAGE1->ID = str_id_maillagenew;
      MAILLAGE1->outputMED(ficMEDout);
      cout << chrono() << " - Finished!" << endl << endl;
      exit(0);
    }
  // A partir de cet instant le maillage contient forcément des TETRA4


  // Chargement des distances noeud-plan DNP
  DNP = (float*) malloc(sizeof(float) * MAILLAGE1->nombreNoeudsMaillage);
  for (int k = 0; k < MAILLAGE1->nombreNoeudsMaillage; k++)
    DNP[k] = distanceNoeudPlan(k + 1);
  cout << chrono() << " - End of computation of distances between nodes and plane" << endl;

  // Longueur d'arête moyenne des T4 intersectant le plan de coupe
  float LONGUEURS = 0.0;
  int cptLONGUEURS = 0;
  for (int it4 = 0; it4 < MAILLAGE1->EFFECTIFS_TYPES[TETRA4]; it4++)
    {
      bool plus = false;
      bool moins = false;
      int *offset = MAILLAGE1->CNX[TETRA4] + 4 * it4;
      for (int is = 0; is < 4; is++)
        {
          int ng = *(offset + is);
          if (DNP[ng - 1] > 0.0)
            plus = true;
          else if (DNP[ng - 1] < 0.0)
            moins = true;
        }
      if (plus && moins)
        {
          // Ce tetra est à cheval sur le plan de coupe: on calcule ses longueurs d'arêtes
          LONGUEURS += longueurSegment(*(offset + 0), *(offset + 1));
          cptLONGUEURS++;
          LONGUEURS += longueurSegment(*(offset + 0), *(offset + 2));
          cptLONGUEURS++;
          LONGUEURS += longueurSegment(*(offset + 0), *(offset + 3));
          cptLONGUEURS++;
          LONGUEURS += longueurSegment(*(offset + 1), *(offset + 2));
          cptLONGUEURS++;
          LONGUEURS += longueurSegment(*(offset + 1), *(offset + 3));
          cptLONGUEURS++;
          LONGUEURS += longueurSegment(*(offset + 2), *(offset + 3));
          cptLONGUEURS++;
        }
    }

  // Aucun TETRA4 intercepté par le plan de coupe : on rend MAILLAGE1
  if (cptLONGUEURS == 0)
    {
      cout
          << "WARNING: the cut plane does not cut any tetra4 element, initial mesh will not be modified"
          << endl;
      MAILLAGE1->ID = str_id_maillagenew;
      MAILLAGE1->outputMED(ficMEDout);
      cout << chrono() << " - Finished!" << endl << endl;
      exit(0);
    }
  // A partir de cet instant le maillage contient forcément des TETRA4 intersectant le plan de coupe


  float longueurMoyenne = LONGUEURS / cptLONGUEURS;
  epsilon = tolerance * longueurMoyenne;

  int nT4coupe = cptLONGUEURS / 6;
  cout << chrono() << " - End of computation of mean length of tetra4 edges near the cut plane" << endl;

  cout << "Number of tetra4 to be cut = " << nT4coupe << endl;
  cout << "Mean length = " << longueurMoyenne << endl;
  cout << "Tolerance = " << tolerance << endl;
  cout << "Epsilon = " << epsilon << endl;

  // Détermination des positions de noeuds par rapport au plan de coupe - POSN
  POSN = (int*) malloc(sizeof(int) * MAILLAGE1->nombreNoeudsMaillage);
  for (int k = 0; k < MAILLAGE1->nombreNoeudsMaillage; k++)
    {
      if (DNP[k] > epsilon)
        POSN[k] = 1;
      else if (DNP[k] < -epsilon)
        POSN[k] = -1;
      else
        POSN[k] = 0;
    }
  cout << chrono() << " - End of nodes qualification above or below the cut plane" << endl;
  cout << "Start of iteration on tetra4" << endl;

  for (int it4 = 0; it4 < MAILLAGE1->EFFECTIFS_TYPES[TETRA4]; it4++)
    {

      for (int is = 0; is < 4; is++)
        {
          int ng = *(MAILLAGE1->CNX[TETRA4] + 4 * it4 + is);
          //NG[is] = ng;
          S[is] = *(POSN + ng - 1);
        }

      // -------------------------------------------------------------------

      if (S[0] == -1 && S[1] == -1 && S[2] == -1 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == -1 && S[2] == -1 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == -1 && S[2] == -1 && S[3] == 1)
        { // Cas 3 - Arêtes 2 4 5
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = intersectionSegmentPlan(it4, 5);
          cas3(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == -1 && S[2] == 0 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == -1 && S[2] == 0 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == -1 && S[2] == 0 && S[3] == 1)
        { // Cas 2, arêtes 2 4
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == -1 && S[2] == 1 && S[3] == -1)
        { // Cas 3, arêtes 1 3 5
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas3(V, it4);
        }

      else if (S[0] == -1 && S[1] == -1 && S[2] == 1 && S[3] == 0)
        { // Cas 2, arêtes 1 3
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == -1 && S[1] == -1 && S[2] == 1 && S[3] == 1)
        { // Cas 4, arêtes 1 2 3 4
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas4(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == 0 && S[2] == -1 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == 0 && S[2] == -1 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == 0 && S[2] == -1 && S[3] == 1)
        { // Cas 2, arêtes 2 5
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == 0 && S[2] == 0 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == 0 && S[2] == 0 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == -1 && S[1] == 0 && S[2] == 0 && S[3] == 1)
        { // Cas 1, arête 2
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == 0 && S[2] == 1 && S[3] == -1)
        { // Cas 2, arêtes 1 5
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      else if (S[0] == -1 && S[1] == 0 && S[2] == 1 && S[3] == 0)
        { // Cas 1, arête 1
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == -1 && S[1] == 0 && S[2] == 1 && S[3] == 1)
        { // Cas 2, arêtes 1 2
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == 1 && S[2] == -1 && S[3] == -1)
        { // Cas 3, arêtes 0 3 4
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas3(V, it4);
        }

      else if (S[0] == -1 && S[1] == 1 && S[2] == -1 && S[3] == 0)
        { // Cas 2, arêtes 0 3
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == -1 && S[1] == 1 && S[2] == -1 && S[3] == 1)
        { // Cas 4, arêtes 0 2 3 5
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas4(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == 1 && S[2] == 0 && S[3] == -1)
        { // Cas 2, arêtes 0 4
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == -1 && S[1] == 1 && S[2] == 0 && S[3] == 0)
        { // Cas 1, arête 0
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == -1 && S[1] == 1 && S[2] == 0 && S[3] == 1)
        { // Cas 2, arêtes 0 2
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == -1 && S[1] == 1 && S[2] == 1 && S[3] == -1)
        { // Cas 4, arêtes 0 1 4 5
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = intersectionSegmentPlan(it4, 5);
          cas4(V, it4);
        }

      else if (S[0] == -1 && S[1] == 1 && S[2] == 1 && S[3] == 0)
        { // Cas 2, arêtes 0 1
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == -1 && S[1] == 1 && S[2] == 1 && S[3] == 1)
        { // Cas 3, arêtes 0 1 2
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas3(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == -1 && S[2] == -1 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == -1 && S[2] == -1 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == -1 && S[2] == -1 && S[3] == 1)
        { // Cas 2, arêtes 4 5
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == -1 && S[2] == 0 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == -1 && S[2] == 0 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == -1 && S[2] == 0 && S[3] == 1)
        { // Cas 1, arête 4
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas1(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == -1 && S[2] == 1 && S[3] == -1)
        { // Cas 2, arêtes 3 5
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      else if (S[0] == 0 && S[1] == -1 && S[2] == 1 && S[3] == 0)
        { // Cas 1, arête 3
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == 0 && S[1] == -1 && S[2] == 1 && S[3] == 1)
        { // Cas 2, arêtes 3 4
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == 0 && S[2] == -1 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == 0 && S[2] == -1 && S[3] == 0)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == 0 && S[2] == -1 && S[3] == 1)
        { // Cas 1, arête 5
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas1(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == 0 && S[2] == 0 && S[3] == -1)
        GMmoins[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == 0 && S[2] == 0 && S[3] == 0)
        {
          cout << "WARNING: TETRA4 number " << it4
              << " entirely in the tolerance zone near the cut plane" << endl;
          cout << " --> affected to group " << str_id_GMmoins << endl;
          GMmoins[TETRA4].push_back(it4);
        }

      else if (S[0] == 0 && S[1] == 0 && S[2] == 0 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == 0 && S[2] == 1 && S[3] == -1)
        { // Cas 1, arête 5
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas1(V, it4);
        }

      else if (S[0] == 0 && S[1] == 0 && S[2] == 1 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == 0 && S[2] == 1 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == 1 && S[2] == -1 && S[3] == -1)
        { // Cas 2, arêtes 3 4
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 0 && S[1] == 1 && S[2] == -1 && S[3] == 0)
        { // Cas 1, arête 3
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == 0 && S[1] == 1 && S[2] == -1 && S[3] == 1)
        { // Cas 2, arêtes 3 5
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == 1 && S[2] == 0 && S[3] == -1)
        { // Cas 1, arête 4
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == 0 && S[1] == 1 && S[2] == 0 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == 1 && S[2] == 0 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 0 && S[1] == 1 && S[2] == 1 && S[3] == -1)
        { // Cas 2, arêtes 4 5
          V[0] = -1;
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      else if (S[0] == 0 && S[1] == 1 && S[2] == 1 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 0 && S[1] == 1 && S[2] == 1 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == -1 && S[2] == -1 && S[3] == -1)
        { // Cas 3, arêtes 0 1 2
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas3(V, it4);
        }

      else if (S[0] == 1 && S[1] == -1 && S[2] == -1 && S[3] == 0)
        { // Cas 2, arêtes 0 1
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == -1 && S[2] == -1 && S[3] == 1)
        { // Cas 4, arêtes 0 1 4 5
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = intersectionSegmentPlan(it4, 5);
          cas4(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == -1 && S[2] == 0 && S[3] == -1)
        { // Cas 2, arêtes 0 2
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == -1 && S[2] == 0 && S[3] == 0)
        { // Cas 1, arête 0
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == 1 && S[1] == -1 && S[2] == 0 && S[3] == 1)
        { // Cas 2, arêtes 0 4
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == -1 && S[2] == 1 && S[3] == -1)
        { // Cas 4, arêtes 0 2 3 5
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas4(V, it4);
        }

      else if (S[0] == 1 && S[1] == -1 && S[2] == 1 && S[3] == 0)
        { // Cas 2, arêtes 0 3
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == -1 && S[2] == 1 && S[3] == 1)
        { // Cas 3, arêtes 0 3 4
          V[0] = intersectionSegmentPlan(it4, 0);
          V[1] = -1;
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas3(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == 0 && S[2] == -1 && S[3] == -1)
        { // Cas 2, arêtes 1 2
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == 0 && S[2] == -1 && S[3] == 0)
        { // Cas 1, arête 1
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == 1 && S[1] == 0 && S[2] == -1 && S[3] == 1)
        { // Cas 2, arêtes 1 5
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = -1;
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == 0 && S[2] == 0 && S[3] == -1)
        { // Cas 1, arête 2
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = -1;
          cas1(V, it4);
        }

      else if (S[0] == 1 && S[1] == 0 && S[2] == 0 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 1 && S[1] == 0 && S[2] == 0 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == 0 && S[2] == 1 && S[3] == -1)
        { // Cas 2, arêtes 2 5
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == 0 && S[2] == 1 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 1 && S[1] == 0 && S[2] == 1 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == 1 && S[2] == -1 && S[3] == -1)
        { // Cas 4, arêtes 1 2 3 4
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas4(V, it4);
        }

      else if (S[0] == 1 && S[1] == 1 && S[2] == -1 && S[3] == 0)
        { // Cas 2, arêtes 1 3
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == 1 && S[2] == -1 && S[3] == 1)
        { // Cas 3, arêtes 1 3 5
          V[0] = -1;
          V[1] = intersectionSegmentPlan(it4, 1);
          V[2] = -1;
          V[3] = intersectionSegmentPlan(it4, 3);
          V[4] = -1;
          V[5] = intersectionSegmentPlan(it4, 5);
          cas3(V, it4);
        }

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == 1 && S[2] == 0 && S[3] == -1)
        { // Cas 2, arêtes 2 4
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = -1;
          cas2(V, it4);
        }

      else if (S[0] == 1 && S[1] == 1 && S[2] == 0 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 1 && S[1] == 1 && S[2] == 0 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      // -------------------------------------------------------------------

      else if (S[0] == 1 && S[1] == 1 && S[2] == 1 && S[3] == -1)
        { // Cas 3, arêtes 2 4 5
          V[0] = -1;
          V[1] = -1;
          V[2] = intersectionSegmentPlan(it4, 2);
          V[3] = -1;
          V[4] = intersectionSegmentPlan(it4, 4);
          V[5] = intersectionSegmentPlan(it4, 5);
          cas3(V, it4);
        }

      else if (S[0] == 1 && S[1] == 1 && S[2] == 1 && S[3] == 0)
        GMplus[TETRA4].push_back(it4);

      else if (S[0] == 1 && S[1] == 1 && S[2] == 1 && S[3] == 1)
        GMplus[TETRA4].push_back(it4);

      else
        ERREUR("Case not taken into account");

    }
  cout << chrono() << " - End of iteration on tetra4" << endl;

  // cout << "indexNouveauxNoeuds = " << indexNouveauxNoeuds << endl;
  newXX.resize(indexNouveauxNoeuds - MAILLAGE1->nombreNoeudsMaillage);
  newYY.resize(indexNouveauxNoeuds - MAILLAGE1->nombreNoeudsMaillage);
  newZZ.resize(indexNouveauxNoeuds - MAILLAGE1->nombreNoeudsMaillage);

  if (cptNouvellesMailles[TETRA4])
    newCNX[TETRA4].resize(4 * cptNouvellesMailles[TETRA4]);
  if (cptNouvellesMailles[PYRAM5])
    newCNX[PYRAM5].resize(5 * cptNouvellesMailles[PYRAM5]);
  if (cptNouvellesMailles[PENTA6])
    newCNX[PENTA6].resize(6 * cptNouvellesMailles[PENTA6]);

  // =========================================================================================
  //                          2. Constitution du maillage final
  // =========================================================================================

  cout << chrono() << " - Constitution of final mesh" << endl;

  MAILLAGE2 = new Maillage(str_id_maillagenew);
  MAILLAGE2->dimensionMaillage = MAILLAGE1->dimensionMaillage;
  MAILLAGE2->dimensionEspace = MAILLAGE1->dimensionEspace;
  strcpy(MAILLAGE2->axisname, MAILLAGE1->axisname);
  strcpy(MAILLAGE2->unitname, MAILLAGE1->unitname);
  MAILLAGE2->nombreNoeudsMaillage = indexNouveauxNoeuds;
  MAILLAGE2->nombreMaillesMaillage = MAILLAGE1->nombreMaillesMaillage + cptNouvellesMailles[TETRA4]
      + cptNouvellesMailles[PYRAM5] + cptNouvellesMailles[PENTA6];

  // ---------- Coordonnées
  // Optimisation de la mémoire au détriment du temps

  // Héritage des coordonnées MAILLAGE1
  MAILLAGE2->XX = (float*) malloc(sizeof(float) * MAILLAGE2->nombreNoeudsMaillage);
  for (int i = 0; i < MAILLAGE1->nombreNoeudsMaillage; i++)
    *(MAILLAGE2->XX + i) = *(MAILLAGE1->XX + i);
  free(MAILLAGE1->XX);
  MAILLAGE2->YY = (float*) malloc(sizeof(float) * MAILLAGE2->nombreNoeudsMaillage);
  for (int i = 0; i < MAILLAGE1->nombreNoeudsMaillage; i++)
    *(MAILLAGE2->YY + i) = *(MAILLAGE1->YY + i);
  free(MAILLAGE1->YY);
  MAILLAGE2->ZZ = (float*) malloc(sizeof(float) * MAILLAGE2->nombreNoeudsMaillage);
  for (int i = 0; i < MAILLAGE1->nombreNoeudsMaillage; i++)
    *(MAILLAGE2->ZZ + i) = *(MAILLAGE1->ZZ + i);
  free(MAILLAGE1->ZZ);

  // Coordonnées des noeuds créés
  for (int i = 0; i < MAILLAGE2->nombreNoeudsMaillage - MAILLAGE1->nombreNoeudsMaillage; i++)
    {
      *(MAILLAGE2->XX + MAILLAGE1->nombreNoeudsMaillage + i) = newXX[i];
      *(MAILLAGE2->YY + MAILLAGE1->nombreNoeudsMaillage + i) = newYY[i];
      *(MAILLAGE2->ZZ + MAILLAGE1->nombreNoeudsMaillage + i) = newZZ[i];
      // cout << "Nouveaux noeuds, indice " << i << " : " << newXX[i] << " " << newYY[i] << " " << newZZ[i] << " " << endl;
    }

  // Legacy mailles maillage 1 (volumes seulement)
  for (int itm = (int) TETRA4; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      if (tm != TETRA4 && tm != PYRAM5 && tm != PENTA6)
        {
          // Pour les types autres que TETRA4 PYRAM5 PENTA6 on fait seulement pointer CNX2 vers CNX1
          if (MAILLAGE1->EFFECTIFS_TYPES[tm])
            MAILLAGE2->CNX[tm] = MAILLAGE1->CNX[tm];
          MAILLAGE2->EFFECTIFS_TYPES[tm] = MAILLAGE1->EFFECTIFS_TYPES[tm];
        }
      else
        {
          // Pour les types TETRA4 PYRAM5 PENTA6 on recopie CNX1 et on ajoute à la suite les newCNX
          // cout << "Legacy " << tm << " effectif " << MAILLAGE1->EFFECTIFS_TYPES[tm] << endl;
          int tailleType = Nnoeuds(tm);

          MAILLAGE2->CNX[tm] = (int*) malloc(sizeof(int) * tailleType * (MAILLAGE1->EFFECTIFS_TYPES[tm]
              + cptNouvellesMailles[tm]));
          for (int i = 0; i < MAILLAGE1->EFFECTIFS_TYPES[tm]; i++)
            for (int j = 0; j < tailleType; j++)
              *(MAILLAGE2->CNX[tm] + tailleType * i + j) = *(MAILLAGE1->CNX[tm] + tailleType * i + j);

          for (int i = 0; i < cptNouvellesMailles[tm]; i++)
            for (int j = 0; j < tailleType; j++)
              *(MAILLAGE2->CNX[tm] + tailleType * (MAILLAGE1->EFFECTIFS_TYPES[tm] + i) + j) = newCNX[tm][i * tailleType
                  + j];

          MAILLAGE2->EFFECTIFS_TYPES[tm] = MAILLAGE1->EFFECTIFS_TYPES[tm] + cptNouvellesMailles[tm];
        }
    }

  // Restit CNX

  //   cout << "Maillage 2 - CNX TETRA4 : " << endl;
  //  ;
  //  for (int i = 0; i < MAILLAGE2->EFFECTIFS_TYPES[TETRA4]; i++)
  //    {
  //      cout << "Maille " << i << " : ";
  //      for (int j = 0; j < 4; j++)
  //        cout << MAILLAGE2->CNX[TETRA4][i * 4 + j] << " ";
  //      cout << endl;
  //    }
  //  cout << endl;
  //  cout << "Maillage 2 - CNX PENTA6 : " << endl;
  //  ;
  //  for (int i = 0; i < MAILLAGE2->EFFECTIFS_TYPES[PENTA6]; i++)
  //    {
  //      cout << "Maille " << i << " : ";
  //      for (int j = 0; j < 6; j++)
  //        cout << MAILLAGE2->CNX[PENTA6][i * 6 + j] << " ";
  //      cout << endl;
  //    }
  //  cout << endl;

  // Groupes de mailles
  // MAILLAGE2->GM = MAILLAGE1->GM;
  MAILLAGE2->GN.clear();
  MAILLAGE2->GM.clear();
  MAILLAGE2->GM[str_id_GMplus] = GMplus;
  MAILLAGE2->GM[str_id_GMmoins] = GMmoins;

  // MAILLAGE2->GN = MAILLAGE1->GN;

  MAILLAGE2->eliminationMailles(TETRA4, cutTetras);

  cout << chrono() << " - MED file writing" << endl;

  MAILLAGE2->outputMED(ficMEDout);
  cout << chrono() << " - Finished!" << endl << endl;

  return 0;

}
