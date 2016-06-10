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

#include "MeshCut_Maillage.hxx"
#include "MeshCut_Cube.hxx"

#include <iostream>
#include <sstream>
#include <list>
#include <algorithm>
#include <cmath>
#include <cstring>

using namespace MESHCUT;
using namespace std;

Maillage::Maillage(std::string _ID)
{
  ID = _ID;
  nombreNoeudsMaillage = 0;
  nombreMaillesMaillage = 0;
  //nPOI1=0; nSEG2=0; nSEG3=0; nTRIA3=0; nTRIA6=0; nQUAD4=0; nQUAD8=0; nTETRA4=0; nTETRA10=0; nPYRAM5=0; nPYRAM13=0; nPENTA6=0; nPENTA15=0; nHEXA8=0; nHEXA20=0;
  GM.clear();
  GN.clear();
  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    EFFECTIFS_TYPES[(TYPE_MAILLE) itm] = 0;
}

Maillage::~Maillage()
{
}

void Maillage::creationGMtype(TYPE_MAILLE tm, std::string nomGMtype)
{
  //cout << "Creation GM type, groupe " << nomGMtype << endl;
  for (int nl = 0; nl < EFFECTIFS_TYPES[tm]; nl++)
    GM[nomGMtype][tm].push_back(nl);
  GM[nomGMtype][tm].resize(EFFECTIFS_TYPES[tm]);
  sort(GM[nomGMtype][tm].begin(), GM[nomGMtype][tm].end());
}

void Maillage::afficheMailles(TYPE_MAILLE tm)
{
  cout << "Affichage des mailles du type " << TM2string(tm) << " (effectif " << EFFECTIFS_TYPES[tm] << "): " << endl;
  if (EFFECTIFS_TYPES[tm])
    {
      // Boucle sur les connectivités d'un type tm
      int nnoeuds = Nnoeuds(tm);
      for (int i = 0; i < EFFECTIFS_TYPES[tm]; i++)
        {
          cout << "\tMaille " << i << " :" << endl;
          //Boucle sur les noeuds de la maille de numéro local i dans le type tm
          int * offset = CNX[tm] + nnoeuds * i;
          for (int j = 0; j < nnoeuds; j++)
            {
              int ngnoeud = *(offset + j);
              //cout << "\t\t" << X[ngnoeud-1] << " " << Y[ngnoeud-1] << " " << Z[ngnoeud-1] << endl;
              cout << "\t" << ngnoeud << "\t" << *(XX + ngnoeud - 1) << " " << *(YY + ngnoeud - 1) << " " << *(ZZ + ngnoeud - 1) << endl;
            }
        }
      cout << endl;
    }
}

void Maillage::listeMaillesType(TYPE_MAILLE tm)
{
  cout << "La fonction \"Restitution des mailles par type\" est obsolète " << endl;

  //  cout << "Restitution des mailles du type " << TM2string(tm) << " (effectif " << EFFECTIFS_TYPES[tm] << "): " << endl;
  //  if (EFFECTIFS_TYPES[tm])
  //    for (int i = 0; i < IDS_MAILLES[tm].size(); i++)
  //      cout << IDS_MAILLES[tm][i] << " ";
  //  cout << endl;
}

void Maillage::listeMaillesTousTypes()
{
  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      listeMaillesType(tm);
    }
}

void Maillage::listeMaillesParGM()
{
  cout << "Liste des mailles par GM : " << endl;
  for (map<string, map<TYPE_MAILLE, vector<int> > >::iterator I = GM.begin(); I != GM.end(); I++)
    listeMaillesGM(I->first);
}

void Maillage::listeMaillesGM(std::string nomGM)
{
  cout << "Liste des mailles du groupe " << nomGM << " : " << endl;
  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      if (GM[nomGM][tm].size())
        {
          cout << "\t" << TM2string(tm) << " : ";
          for (unsigned int j = 0; j < GM[nomGM][tm].size(); j++)
            cout << GM[nomGM][tm][j] << " ";
          cout << endl;
        }
    }
}

//void Maillage::listeMaillesGMordonne(std::string nomGM)
//{
//  cout << "Liste ordonnée des mailles du groupe " << nomGM << " (" << GM[nomGM].size() << " mailles) : ";
//  sort(GM[nomGM].begin(), GM[nomGM].end());
//  for (int j = 0; j < GM[nomGM].size(); j++)
//    cout << GM[nomGM][j] << " ";
//  cout << endl;
//}

void Maillage::listeNoeuds()
{
  cout << "Liste des noeuds du maillage : " << endl;
  for (int i = 0; i < nombreNoeudsMaillage; i++)
    cout << "\t" << *(XX + i) << " " << *(YY + i) << " " << *(ZZ + i) << endl;
  cout << endl;
}

void Maillage::listeNoeudsGN(std::string nomGN)
{
  cout << "Liste brute des noeuds du groupe " << nomGN << " (" << GN[nomGN].size() << " noeuds) : ";
  for (unsigned int j = 0; j < GN[nomGN].size(); j++)
    cout << GN[nomGN][j] << " ";
  cout << endl;
}

void Maillage::listeNoeudsGNordonne(std::string nomGN)
{
  cout << "Liste ordonnée des noeuds du groupe " << nomGN << " (" << GN[nomGN].size() << " noeuds) : ";
  sort(GN[nomGN].begin(), GN[nomGN].end());
  for (unsigned int j = 0; j < GN[nomGN].size(); j++)
    cout << GN[nomGN][j] << " ";
  cout << endl;
}

std::vector<float> Maillage::G(int i, TYPE_MAILLE tm)
{
  vector<float> G;
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;
  int nn = NnoeudsGeom(tm);
  for (int j = 0; j < nn; j++)
    {
      int ng = CNX[tm][nn * i + j];
      x += XX[ng - 1];
      y += YY[ng - 1];
      z += ZZ[ng - 1];
    }
  G.push_back(x / nn);
  G.push_back(y / nn);
  G.push_back(z / nn);
  G.resize(3);
  return G;
}

float Maillage::distanceNoeudMaille(int ngnoeud, int imaille, TYPE_MAILLE tm)
{
  float x, y, z;
  float x0 = XX[ngnoeud - 1];
  float y0 = YY[ngnoeud - 1];
  float z0 = ZZ[ngnoeud - 1];
  int nn = NnoeudsGeom(tm);
  float d1 = 1000000000000.0;
  float d;
  for (int j = 0; j < nn; j++)
    {
      int ng = CNX[tm][nn * imaille + j]; // Noeud courant dans la maille
      x = XX[ng - 1];
      y = YY[ng - 1];
      z = ZZ[ng - 1];
      d = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0) + (z - z0) * (z - z0));
      if (d < d1)
        d1 = d;
    }
  return d1;
}

/*!
 *  Retourne le ng du noeud le plus proche de ngnoeud dans la maille imaille du type tm
 */
int Maillage::noeudVoisin(int ngnoeud, int imaille, TYPE_MAILLE tm)
{
  float x, y, z;
  int ngv = -1;
  float x0 = XX[ngnoeud - 1];
  float y0 = YY[ngnoeud - 1];
  float z0 = ZZ[ngnoeud - 1];
  int nn = NnoeudsGeom(tm);
  float d1 = 1000000000000.0;
  float d;
  for (int j = 0; j < nn; j++)
    {
      int ng = CNX[tm][nn * imaille + j]; // Noeud courant dans la maille
      x = XX[ng - 1];
      y = YY[ng - 1];
      z = ZZ[ng - 1];
      d = sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0) + (z - z0) * (z - z0));
      if (d < d1)
        {
          d1 = d;
          ngv = ng;
        }
    }
  return ngv;
}

float Maillage::distanceNoeudNoeud(int ng1, int ng2)
{
  float x1, x2, y1, y2, z1, z2;
  x1 = XX[ng1 - 1];
  y1 = YY[ng1 - 1];
  z1 = ZZ[ng1 - 1];
  x2 = XX[ng2 - 1];
  y2 = YY[ng2 - 1];
  z2 = ZZ[ng2 - 1];
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
}

//void Maillage::encombrements()
//{
//  float ex = 0.0;
//  float ey = 0.0;
//  float ez = 0.0;
//
//  for (int itm = (int) SEG2; itm <= (int) HEXA20; itm++)
//    {
//      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
//      if (MAILLAGE->EFFECTIFS_TYPES[tm])
//        {
//          int nnoeuds = Nnoeuds(tm);
//          for (int i = 0; i < CON[tm].size() / nnoeuds; i++)
//            {
//              //Boucle sur les noeuds de la maille de numéro local i dans le type tm
//              for (int j = 0; j < nnoeuds; j++)
//                {
//                  //..... CON[tm][nnoeuds*i+j];
//                }
//            }
//        }
//    }
//  // Boucle sur les connectivités d'un type tm
//}

void Maillage::inputMED(std::string fichierMED)
{
  //cout << endl << "Début procédure inputMED, fichier "<< fichierMED << endl;

  //  int i, j, k, ichamp, igauss, ipt, ival;
  int j;
  //  med_err ret = 0; // Code retour
  med_idt fid; // Descripteur de fichier MED
  char maa[MED_NAME_SIZE + 1]; // nom du maillage de longueur maxi MED_NAME_SIZE
  med_int spacedim;
  med_int mdim; // Dimension du maillage
  med_mesh_type type;
  char desc[MED_COMMENT_SIZE + 1]; // Description du maillage

  // Profils
  //  med_int nprofils;
  //  int iprofil;
  //  char nomprofil[MED_NAME_SIZE + 1] = "";
  //  med_int nvalprofil, nvalprofil2;
  //  med_int *pflval;

  // Champs
  //  med_int nChamps, nCompChamp, nval;
  //  char *compChamp, *unitChamp, *nomChamp;
  //char nomChamp [ MED_NAME_SIZE+1 ] = "";
  //  med_field_type typeChamp;
  //  med_int nGauss, ngpdt, numdt, numo;
  med_int nPasTemps;
  //  char locname[MED_NAME_SIZE + 1] = "";
  //  med_geometry_type type_geo;
  //  med_int ngauss;
  char dtunit[MED_SNAME_SIZE + 1] = "";
  //  med_float dt = 0.0;
  //  med_bool local;
  //  med_int nbrefmaa;

  med_sorting_type sortingtype;
  med_axis_type axistype;

  // Initialisations
  FAMILLES.clear();
  FAM_TYPES.clear();
  FAMILLES_NOEUDS.clear();
  GROUPES_MAILLES.clear();
  GROUPES_NOEUDS.clear();
  RESIDU.clear(); // Sera initialisé à 1 par la routine acquisitionTYPE_inputMED
  tailleFAMILLES.clear();
  tailleGROUPES.clear();

  // Ouverture du fichier MED en lecture seule
  fid = MEDfileOpen(string2char(fichierMED), MED_ACC_RDONLY);
  if (fid < 0)
    {
      ERREUR("Error file open\n");
    }
  //cout << chrono() << " --- inputMED: MEDfileOpen: ouverture du maillage en lecture seule, OK" << endl;

  // Lecture des infos concernant le premier maillage
  if (MEDmeshInfo(fid, 1, maa, &spacedim, &mdim, &type, desc, dtunit, &sortingtype, &nPasTemps, &axistype, axisname,
                  unitname) < 0)
    ERREUR("Error while reading mesh informations ");
  //cout << chrono() << " --- inputMED: MEDmeshInfo: OK" << endl;

//  cerr << "maa=" << maa << endl;
//  cerr << "spacedim=" << spacedim << endl;
//  cerr << "mdim=" << mdim << endl;
//  cerr << "type=" << type << endl;
//  cerr << "desc=" << desc << endl;
//  cerr << "dtunit=" << dtunit << endl;
//  cerr << "sortingtype=" << sortingtype << endl;
//  cerr << "nPasTemps=" << nPasTemps << endl;
//  cerr << "axistype=" << axistype << endl;
//  cerr << "axisname=" << axisname << endl;
//  cerr << "unitname=" << unitname << endl;

  dimensionMaillage = mdim;
  dimensionEspace = spacedim;

  ID = (string) maa;

  //  nGauss = MEDnGauss(fid);
  //  if (debug > 0)
  //    cout << "Nombre d'éléments portant des points de Gauss: " << (int) nGauss << endl;
  //  map<string, int> REFGAUSS;
  //  map<string, int>::iterator iterGAUSS;
  //  for (igauss = 1; igauss <= nGauss; igauss++)
  //    {
  //      if (MEDgaussInfo(fid, igauss, locname, &type_geo, &ngauss) < 0)
  //        ERREUR("Erreur MEDgaussInfo");
  //      if (debug == 2)
  //        {
  //          cout << endl << "  Retour MEDgaussInfo, localisation gauss n°" << igauss << " : " << endl;
  //          cout << "    locname  = " << locname << endl;
  //          cout << "    type_geo = " << type_geo << endl;
  //          cout << "    ngauss   = " << ngauss << endl;
  //          cout << endl;
  //        }
  //      REFGAUSS[(string) locname] = (int) ngauss;
  //    }
  //
  //  if (debug == 2)
  //    {
  //      cout << endl << "Restitution de la table REFGAUSS:" << endl;
  //      for (iterGAUSS = REFGAUSS.begin(); iterGAUSS != REFGAUSS.end(); iterGAUSS++)
  //        {
  //          cout << iterGAUSS->first << " : " << iterGAUSS->second << endl;
  //        }
  //    }
  //
  //  nprofils = MEDnProfil(fid);
  //  if (debug)
  //    cout << endl << endl << "Nombre de profils: " << nprofils << endl;
  //  for (iprofil = 1; iprofil <= nprofils; iprofil++)
  //    {
  //      if (MEDprofilInfo(fid, iprofil, nomprofil, &nvalprofil) < 0)
  //        ERREUR("ERREUR MEDprofilInfo");
  //      nvalprofil2 = MEDnValProfil(fid, nomprofil);
  //      if (debug == 2)
  //        {
  //          cout << "Profil " << iprofil << " : " << endl;
  //          cout << "    Nom profil : " << nomprofil << endl;
  //          cout << "    Nombre de valeurs profil (par MEDprofilInfo) : " << nvalprofil << endl;
  //          cout << "    Nombre de valeurs profil (par MEDnValProfil) : " << nvalprofil2 << endl;
  //        }
  //      if (nvalprofil != nvalprofil2)
  //        ERREUR("Discordance nvalprofil (entre MEDprofilInfo et MEDnValProfil)");
  //      pflval = (med_int*) malloc(sizeof(med_int) * nvalprofil);
  //      if (MEDprofilLire(fid, pflval, nomprofil) < 0)
  //        ERREUR("ERREUR MEDprofilLire");
  //      //cout << "    Affichage des 100 premières valeurs:" << endl;
  //      //for (ival=0;ival<min(100,nvalprofil);ival++) cout << " " << *(pflval+ival) ;
  //      free(pflval);
  //    }
  //
  //  nChamps = MEDnChamp(fid, 0);
  //  cout << "Nombre de champs : " << (int) nChamps << endl;
  //
  //  if (nChamps > 0)
  //    {
  //
  //      for (ichamp = 1; ichamp <= nChamps; ichamp++)
  //        {
  //          //for (ichamp=4; ichamp<=4; ichamp++ ) {
  //          cout << endl << endl;
  //          cout << endl << endl << " ====================================================================" << endl;
  //          cout << endl << endl << "                             CHAMP " << ichamp << endl;
  //          cout << endl << endl << " ====================================================================" << endl;
  //          cout << endl << endl;
  //
  //          nCompChamp = MEDnChamp(fid, ichamp);
  //          if (nCompChamp < 0)
  //            ERREUR("Erreur Ncomposantes champ");
  //          cout << "Nombre de composantes du champ " << ichamp << " : " << (int) nCompChamp << endl;
  //
  //          nomChamp = (char*) malloc(MED_NAME_SIZE + 1);
  //          compChamp = (char*) malloc(nCompChamp * MED_SNAME_SIZE + 1);
  //          unitChamp = (char*) malloc(nCompChamp * MED_SNAME_SIZE + 1);
  //
  //          if (MEDchampInfo(fid, ichamp, nomChamp, &typeChamp, compChamp, unitChamp, nCompChamp) < 0)
  //            ERREUR("Erreur MEDchampInfo");
  //
  //          cout << "Infos sur le champ " << ichamp << " : " << endl;
  //          cout << "  Nom:  " << (string) nomChamp << endl;
  //          cout << "  Type:  " << typeChamp << endl;
  //          cout << "  Noms des composantes:  " << (string) compChamp << endl;
  //          cout << "  Unités des composantes:  " << (string) unitChamp << endl;
  //
  //          infoChamps((string) "NOEUDS", MED_NODE, MED_NONE, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "POI1", MED_CELL, MED_POINT1, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "SEG2", MED_CELL, MED_SEG2, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "SEG3", MED_CELL, MED_SEG3, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "TRIA3", MED_CELL, MED_TRIA3, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "TRIA6", MED_CELL, MED_TRIA6, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "QUAD4", MED_CELL, MED_QUAD4, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "QUAD8", MED_CELL, MED_QUAD8, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "TETRA4", MED_CELL, MED_TETRA4, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "TETRA10", MED_CELL, MED_TETRA10, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "PYRAM5", MED_CELL, MED_PYRA5, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "PYRAM13", MED_CELL, MED_PYRA13, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "PENTA6", MED_CELL, MED_PENTA6, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "PENTA15", MED_CELL, MED_PENTA15, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "HEXA8", MED_CELL, MED_HEXA8, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //          infoChamps((string) "HEXA20", MED_CELL, MED_HEXA20, fid, maa, nomChamp, typeChamp, nCompChamp, REFGAUSS);
  //
  //        }
  //
  //      cout << endl << "Rappel des codes de géométries: " << endl;
  //      cout << " TETRA4 = " << MED_TETRA4 << endl;
  //      cout << " PENTA6 = " << MED_PENTA6 << endl;
  //      cout << " HEXA8 = " << MED_HEXA8 << endl;
  //
  //    }
  //  else
  //    cout << "Pas de champs dans ce maillage" << endl;

  // ##################################################################################################
  // ##################################################################################################
  //
  //                  A C Q U I S I T I O N     D E S     F A M I L L E S
  //
  // ##################################################################################################
  // ##################################################################################################


  med_int nFamilles;
  char nomGroupeChar[MED_LNAME_SIZE + 1];
  if ((nFamilles = MEDnFamily(fid, maa)) < 0)
    ERREUR("ERROR MEDnFamily");

  // Initialisation des tailles:   tailleFAMILLES  et  tailleGROUPES

  //   for (int i = 0; i < nFamilles; i++)
  //    {
  //      char nomfam[MED_NAME_SIZE + 1];
  //      char *attdes, *gro;
  //      med_int numfam, *attide, *attval, natt, ngro;
  //
  //      if ((ngro = MEDnFamilyGroup(fid, maa, i + 1)) < 0)
  //        ERREUR("ERREUR MEDnFamilyGroup");
  //      if ((natt = MEDnFamily23Attribute(fid, maa, i + 1)) < 0)
  //        ERREUR("ERREUR MEDnFamily23Attribute");
  //
  //      attide = (med_int *) malloc(sizeof(med_int) * natt);
  //      attval = (med_int *) malloc(sizeof(med_int) * natt);
  //      attdes = (char *) malloc(MED_COMMENT_SIZE * natt + 1);
  //      gro = (char *) malloc(MED_LNAME_SIZE * ngro + 1);
  //
  //      if (MEDfamilyInfo(fid, maa, (med_int)(i + 1), nomfam, &numfam, attide, attval, attdes, &natt, gro, &ngro) < 0)
  //        ERREUR("ERREUR MEDfamilyInfo");
  //
  //      free(attide);
  //      free(attval);
  //      free(attdes);
  //      free(gro);
  //    }

  for (int i = 0; i < nFamilles; i++)
    {
      char nomfam[MED_NAME_SIZE + 1];
      char *attdes, *gro;
      med_int numfam, *attide, *attval, natt, ngro;

      if ((ngro = MEDnFamilyGroup(fid, maa, i + 1)) < 0)
        ERREUR("ERROR MEDnFamilyGroup");
      if ((natt = MEDnFamily23Attribute(fid, maa, i + 1)) < 0)
        ERREUR("ERROR MEDnFamily23Attribute");

      attide = (med_int *) malloc(sizeof(med_int) * natt);
      attval = (med_int *) malloc(sizeof(med_int) * natt);
      attdes = (char *) malloc(MED_COMMENT_SIZE * natt + 1);
      gro = (char *) malloc(MED_LNAME_SIZE * ngro + 1);

      if (MEDfamilyInfo(fid, maa, (med_int) (i + 1), nomfam, &numfam, gro) < 0)
        ERREUR("ERROR MEDfamilyInfo");

      for (int ig = 1; ig <= ngro; ig++)
        {
          for (j = 0; j < MED_LNAME_SIZE; j++)
            nomGroupeChar[j] = gro[(ig - 1) * MED_LNAME_SIZE + j];
          nomGroupeChar[MED_LNAME_SIZE] = '\0';
          //cout << "Groupe lu : " << (string)nomGroupeChar << endl;
          tailleGROUPES[strip((string) nomGroupeChar)]++;
          if (numfam > 0)
            GROUPES_NOEUDS[strip((string) nomGroupeChar)].push_back((int) numfam);
          else if (numfam < 0)
            GROUPES_MAILLES[strip((string) nomGroupeChar)].push_back((int) numfam);
        }

      free(attide);
      free(attval);
      free(attdes);
      free(gro);
    }

  // ##################################################################################################
  // ##################################################################################################
  //
  //                  A C Q U I S I T I O N     D E S     N O E U D S
  //
  // ##################################################################################################
  // ##################################################################################################

  //  class Noeud *n;
  //  list<Noeud*> listeNoeuds;
  //  float x, y, z, rx, ry, rz, tx, ty, tz;
  string line, IDnoeud;
  float x0, x1, y0, y1, z0, z1;

  vector<int> RESIDU_NOEUDS; // Table de vérité du résidu des noeuds

  ostringstream OSCOORD;

  med_int nnoe = 0; // Nbre de noeuds
  med_float *coo1 = 0; // Table des coordonnées
  //  char nomcoo[mdim * MED_SNAME_SIZE + 1]; // Table des noms des coordonnées
  //  char unicoo[mdim * MED_SNAME_SIZE + 1]; // Table des unités des coordonnées
  char *nomnoe = 0;

  med_int *numnoe = 0;
  med_int *nufano = 0;
  //  med_grid_type rep;
  //  med_bool inonoe, inunoe;
  //  med_int profil[2] = { 2, 3 };
  med_bool coordinatechangement;
  med_bool geotransformation;

  // Combien de noeuds a lire ?
  nnoe = MEDmeshnEntity(fid, maa, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NO_GEOTYPE, MED_COORDINATE, MED_NO_CMODE,
                        &coordinatechangement, &geotransformation);

  if (nnoe < 0)
    ERREUR("Error while reading number of nodes");

  nombreNoeudsMaillage = nnoe;

  // Lecture des familles des noeuds
  med_int *famNoeuds = (med_int*) malloc(sizeof(med_int) * nnoe);
  if (nnoe > 0)
    {
      if (MEDmeshEntityFamilyNumberRd(fid, maa, MED_NO_DT, MED_NO_IT, MED_NODE, MED_NONE, famNoeuds) < 0)
        ERREUR("Error while reading family node number (MEDmeshEntityFamilyNumberRd)");
    }

  /* Allocations memoires */
  if (nnoe > 0)
    {
      // table des coordonnees - profil : (dimension * nombre de noeuds )
      coo1 = (med_float*) calloc(nnoe * mdim, sizeof(med_float));
      // table des des numeros, des numeros de familles des noeuds - profil : (nombre de noeuds)
      numnoe = (med_int*) malloc(sizeof(med_int) * nnoe);
      nufano = (med_int*) malloc(sizeof(med_int) * nnoe);
      // table des noms des noeuds - profil : (nnoe*MED_SNAME_SIZE+1)
      nomnoe = (char*) malloc(MED_SNAME_SIZE * nnoe + 1);
    }

  // Lecture des composantes des coordonnees des noeuds
  if (nnoe > 0)
    if (MEDmeshNodeCoordinateRd(fid, maa, MED_NO_DT, MED_NO_IT, MED_FULL_INTERLACE, coo1) < 0)
      ERREUR("Error while reading nodes coordinates");

  //   // Les noeuds ont-ils un nom? un numéro?
  //  if (nnoe > 0)
  //    {
  //      if (MEDnomLire(fid, maa, nomnoe, nnoe, MED_NODE, (med_geometry_type) 0) < 0)
  //        inonoe = MED_FALSE;
  //      else
  //        inonoe = MED_TRUE;
  //      if (MEDnumLire(fid, maa, numnoe, nnoe, MED_NODE, (med_geometry_type) 0) < 0)
  //        inunoe = MED_FALSE;
  //      else
  //        inunoe = MED_TRUE;
  //    }
  //
  //  if (inonoe)
  //    cout << "WARNING input MED : les noms des noeuds sont ignorés" << endl;
  //  if (inunoe)
  //    cout << "WARNING input MED : les numéros des noeuds sont ignorés" << endl;
  //
  //  if (inonoe)
  //    {
  //      char str[MED_SNAME_SIZE + 1];
  //      for (int inoeud = 0; inoeud < nnoe; inoeud++)
  //        {
  //          strncpy(str, nomnoe + inoeud * MED_SNAME_SIZE, MED_SNAME_SIZE);
  //          str[MED_SNAME_SIZE] = '\0';
  //          IDS_NOEUDS.push_back((string) str);
  //        }
  //    }
  //  else if (inunoe)
  //    {
  //      for (int inoeud = 0; inoeud < nnoe; inoeud++)
  //        {
  //          int numnoeud = *(numnoe + inoeud);
  //          IDS_NOEUDS.push_back((string) "N" + int2string(numnoeud));
  //        }
  //    }
  //  else
  //    for (int inoeud = 0; inoeud < nnoe; inoeud++)
  //      IDS_NOEUDS.push_back((string) "N" + int2string(inoeud + 1));
  //  IDS_NOEUDS.resize(nnoe);

  /* ====================================================================== */
  /*               BOUCLE SUR LES NOEUDS LUS DANS LE FICHIER MED            */
  /* ====================================================================== */

  //  X.resize(nnoe);
  //  Y.resize(nnoe);
  //  Z.resize(nnoe);

  // Initialisation de l'enveloppe
  x0 = coo1[0];
  x1 = coo1[0];
  y0 = coo1[1];
  y1 = coo1[1];
  if (mdim == 3)
    {
      z0 = coo1[2];
      z1 = coo1[2];
    }
  else
    {
      z0 = 0.0;
      z1 = 0.0;
    }

  // Allocation mémoire pour les coordonnées XX YY ZZ
  XX = (float*) malloc(sizeof(float) * nombreNoeudsMaillage);
  if (mdim > 1)
    YY = (float*) malloc(sizeof(float) * nombreNoeudsMaillage);
  if (mdim > 2)
    ZZ = (float*) malloc(sizeof(float) * nombreNoeudsMaillage);

  for (int i = 0; i < nnoe; i++)
    {

      // Chargement des coordonnées X, Y et Z
      // Calcul de l'enveloppe du maillage

      FAMILLES_NOEUDS[*(famNoeuds + i)].push_back(i + 1); // ATTENTION! Les num. de noeuds commencent à 1
      tailleFAMILLES[*(famNoeuds + i)]++;

      // IDnoeud = "N"+int2string(i+1);

      float * XXi = XX + i;
      float * YYi = YY + i;
      float * ZZi = ZZ + i;

      if (mdim == 3)
        {
          *XXi = (float) coo1[3 * i];
          *YYi = (float) coo1[3 * i + 1];
          *ZZi = (float) coo1[3 * i + 2];
          if (*XXi < x0)
            x0 = *XXi;
          else if (*XXi > x1)
            x1 = *XXi;
          if (*YYi < y0)
            y0 = *YYi;
          else if (*YYi > y1)
            y1 = *YYi;
          if (*ZZi < z0)
            z0 = *ZZi;
          else if (*ZZi > z1)
            z1 = *ZZi;
        }
      else if (mdim == 2)
        {
          *XXi = (float) coo1[2 * i];
          *YYi = (float) coo1[2 * i + 1];
          if (*XXi < x0)
            x0 = *XXi;
          else if (*XXi > x1)
            x1 = *XXi;
          if (*YYi < y0)
            y0 = *YYi;
          else if (*YYi > y1)
            y1 = *YYi;
        }
      else if (mdim == 1)
        {
          *XXi = (float) coo1[1 * i];
          if (*XXi < x0)
            x0 = *XXi;
          else if (*XXi > x1)
            x1 = *XXi;
        }

      // Chargement des numéros de noeuds
      //      if (inunoe)
      //        NUM_NOEUDS.push_back(*(numnoe + i));
      //      else
      //        NUM_NOEUDS.push_back(i + 1);

    } // boucle sur les noeuds

  //  NUM_NOEUDS.resize(nnoe);

  // Enveloppe du maillage
  enveloppeMaillage = new Cube(x0, x1, y0, y1, z0, z1);

  // Libération mémoire
  if (nnoe > 0)
    {
      free(coo1);
      free(nomnoe);
      free(numnoe);
      free(nufano);
    }

  // ##################################################################################################
  // ##################################################################################################
  //
  //                  A C Q U I S I T I O N     D E S     M A I L L E S
  //
  // ##################################################################################################
  // ##################################################################################################

  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      EFFECTIFS_TYPES[tm] = MEDmeshnEntity(fid, maa, MED_NO_DT, MED_NO_IT, MED_CELL, InstanceMGE(tm), MED_CONNECTIVITY,
                                           MED_NODAL, &coordinatechangement, &geotransformation);
      if (EFFECTIFS_TYPES[tm])
        acquisitionTYPE_inputMED(tm, EFFECTIFS_TYPES[tm], fid, maa, mdim);
    }

  // Resize des vecteurs des maps FAMILLES et FAM_TYPES
  map<int, vector<int> >::iterator IF;
  for (IF = FAMILLES.begin(); IF != FAMILLES.end(); IF++)
    {
      IF->second.resize(tailleFAMILLES[IF->first]);
      FAM_TYPES[IF->first].resize(tailleFAMILLES[IF->first]);
    }

  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    nombreMaillesMaillage += EFFECTIFS_TYPES[(TYPE_MAILLE) itm];

  // ##################################################################################################
  // ##################################################################################################
  //
  //          A C Q U I S I T I O N     D E S     G R O U P E S
  //          D E    M A I L L E S    E T    D E    N O E U D S
  //
  // ##################################################################################################
  // ##################################################################################################

  // =============================================================================================
  //                 Chargement des groupes dans GM et GN
  // =============================================================================================

  string nomGM;
  vector<int> vfam;
  map<string, vector<int> >::iterator iterGRO;

  int cptGM = 0;
  for (iterGRO = GROUPES_MAILLES.begin(); iterGRO != GROUPES_MAILLES.end(); iterGRO++)
    {
      nomGM = iterGRO->first;
      vfam = iterGRO->second;
      cptGM++;
      map<TYPE_MAILLE, int> effectifGroupeType;
      for (unsigned int i = 0; i < vfam.size(); i++)
        {
          int numf = vfam[i];
          // Parcours simultané des vecteurs FAMILLES[numf] et FAM_TYPES[numfam] pour obtention du num local
          // et du type des mailles de la famille numf
          for (unsigned int imaille = 0; imaille < FAMILLES[numf].size(); imaille++)
            {
              TYPE_MAILLE tm = FAM_TYPES[numf][imaille];
              int nl = FAMILLES[numf][imaille];
              GM[nomGM][tm].push_back(nl);
              effectifGroupeType[tm]++;
            }
        }

      // Resize d'un GM
      for (map<TYPE_MAILLE, vector<int> >::iterator I = GM[nomGM].begin(); I != GM[nomGM].end(); I++)
        {
          TYPE_MAILLE tm = I->first;
          GM[nomGM][tm].resize(effectifGroupeType[tm]);
          sort(GM[nomGM][tm].begin(), GM[nomGM][tm].end());
        }
    }

  int cptGN = 0;
  for (iterGRO = GROUPES_NOEUDS.begin(); iterGRO != GROUPES_NOEUDS.end(); iterGRO++)
    {
      nomGM = iterGRO->first;
      vfam = iterGRO->second;
      cptGN++;
      int cptNoeudsGN = 0;
      for (unsigned int i = 0; i < vfam.size(); i++)
        {
          int numf = vfam[i];
          // Parcours vecteurs FAMILLES_NOEUDS[numf]
          for (unsigned int inoeud = 0; inoeud < FAMILLES_NOEUDS[numf].size(); inoeud++)
            {
              GN[nomGM].push_back(FAMILLES_NOEUDS[numf][inoeud]);
              cptNoeudsGN++;
            }
        }
      GN[nomGM].resize(cptNoeudsGN);
      sort(GN[nomGM].begin(), GN[nomGM].end());
    }

  MEDfileClose(fid);
  //  cout << "Fin procédure inputMED" << endl << endl;
}

void Maillage::acquisitionTYPE_inputMED(TYPE_MAILLE TYPE, int nTYPE, med_idt fid, char maa[MED_NAME_SIZE + 1],
                                        med_int mdim)
{

  //  int taille, numeromaille, numeroFamille;
  int numeroFamille;
  string line, IDmaille, IDnoeud, typeMaille;
  //  char str[MED_SNAME_SIZE + 1]; // Conteneur pour un nom de maille
  //  bool rejetMaille;
  med_int tTYPE = (med_int) Nnoeuds(TYPE);
  char *nomTYPE = (char*) malloc(MED_SNAME_SIZE * nTYPE + 1);
  med_int *numTYPE = (med_int*) malloc(sizeof(med_int) * nTYPE);
  med_int *famTYPE = (med_int*) malloc(sizeof(med_int) * nTYPE);

  //med_int *conTYPE = (med_int*) malloc(sizeof(med_int)*tTYPE*nTYPE);
  CNX[TYPE] = (int*) malloc(sizeof(int) * tTYPE * nTYPE);

  med_bool inomTYPE, inumTYPE, ifamTYPE;
  med_geometry_type typeBanaliseMED = InstanceMGE(TYPE);

  if (MEDmeshElementRd(fid, maa, MED_NO_DT, MED_NO_IT, MED_CELL, typeBanaliseMED, MED_NODAL, MED_FULL_INTERLACE,
                       CNX[TYPE], &inomTYPE, nomTYPE, &inumTYPE, numTYPE, &ifamTYPE, famTYPE) < 0)
    ERREUR("Error while reading elements");

  // Conversion HL
  conversionCNX(CNX[TYPE], TYPE, nTYPE);

  //  CON[TYPE].resize(tTYPE * nTYPE);
  //  for (int i = 0; i < tTYPE * nTYPE; i++)
  //    CON[TYPE][i] = (int) *(conTYPE + i);
  //  CNX[TYPE] = (int*) malloc(sizeof(int) * tTYPE * nTYPE);
  //  for (int i = 0; i < tTYPE * nTYPE; i++)
  //    *(CNX[TYPE] + i) = (int) *(conTYPE + i);

  for (int i = 0; i < nTYPE; i++)
    {
      numeroFamille = (int) *(famTYPE + i);
      FAMILLES[numeroFamille].push_back(i);
      tailleFAMILLES[numeroFamille]++;
      FAM_TYPES[numeroFamille].push_back(TYPE);

      // Chargement des numéros de mailles
      //      if (inumTYPE)
      //        NUM_MAILLES[TYPE].push_back(*(numTYPE + i));
      //      else
      //        NUM_MAILLES[TYPE].push_back(NGLOBAL(TYPE, i));

    }
  //  NUM_MAILLES[TYPE].resize(nTYPE);

  //   if (inomTYPE)
  //    {
  //      char str[MED_SNAME_SIZE + 1];
  //      for (int imaille = 0; imaille < nTYPE; imaille++)
  //        {
  //          strncpy(str, nomTYPE + imaille * MED_SNAME_SIZE, MED_SNAME_SIZE);
  //          str[MED_SNAME_SIZE] = '\0';
  //          IDS_MAILLES[TYPE].push_back((string) str);
  //        }
  //    }
  //  else if (inumTYPE)
  //    {
  //      for (int imaille = 0; imaille < nTYPE; imaille++)
  //        {
  //          int nummaille = *(numTYPE + imaille);
  //          IDS_MAILLES[TYPE].push_back((string) "M" + int2string(nummaille));
  //        }
  //    }
  //  else
  //    {
  //      for (int imaille = 0; imaille < nTYPE; imaille++)
  //        {
  //          IDS_MAILLES[TYPE].push_back((string) "M" + int2string(imaille + 1));
  //        }
  //    }
  //  IDS_MAILLES[TYPE].resize(nTYPE);
}

void Maillage::outputMED(std::string fichierMED)
{
  // int i, j, k;
  int nTYPE;//, tTYPE;
  string line, s, stype, nomnoeud;
  //  med_err ret = 0; // Code retour
  //  int ig, jg;
  //  cout << endl << endl << "Début procédure outputMED, fichier " << fichierMED << endl;

  // Sortie sur erreur en cas de maillage sans noeuds
  if (nombreNoeudsMaillage <= 0)
    {
      ERREUR("This mesh does not contain any node\n"); /* cout << "Maillage sans noeuds" << endl; */
    }

  // ########################################################################
  //    Ouverture du fichier MED et création du maillage
  // ########################################################################

  // Ouverture du fichier MED en création
  med_idt fid = MEDfileOpen(string2char(fichierMED), MED_ACC_CREAT);
  if (fid < 0)
    {
      ERREUR("Error MEDfileOpen\n");
      cout << "Error MEDfileOpen" << endl;
    }

  // Création du maillage
  char maa[MED_NAME_SIZE + 1]; // Nom du maillage de longueur maxi MED_NAME_SIZE
  strcpy(maa, string2char(ID));

  med_int mdim; // Dimension du maillage
  if (dimensionMaillage == 0)
    {
      mdim = 3;
      cout << "ATTENTION, dimension 3 attribuée par défaut!" << endl;
    }
  else
    mdim = dimensionMaillage;
  med_int spacedim = 3;
  if (dimensionEspace)
    spacedim = dimensionEspace;

  //med_int profil[2] = { 2, 3 };
  char desc[MED_COMMENT_SIZE + 1]; // Description du maillage
  strcpy(desc, string2char(ID));
  med_mesh_type type = MED_UNSTRUCTURED_MESH;
//  cerr << "maa=" << maa << endl;
//  cerr << "spacedim=" << spacedim << endl;
//  cerr << "mdim=" << mdim << endl;
//  cerr << "type=" << type << endl;
//  cerr << "axisname=" << axisname << endl;
//  cerr << "unitname=" << unitname << endl;
  if (MEDmeshCr(fid, maa, spacedim, mdim, type, desc, "s", MED_SORT_DTIT, MED_CARTESIAN, axisname, unitname) < 0)
    {
      ERREUR("Error MEDmeshCr");
      cout << "Error MEDmeshCr" << endl;
    }

  // =============================  CREATION FAMILLE ZERO
  char nomfam[MED_NAME_SIZE + 1];
  med_int numfam;
  //  char attdes[MED_COMMENT_SIZE + 1];
  //  med_int natt, attide, attval;
  int ngro;
  //  char gro[MED_LNAME_SIZE + 1];

  strcpy(nomfam, "FAMILLE_0");
  numfam = 0;
  if (MEDfamilyCr(fid, maa, nomfam, numfam, 0, MED_NO_GROUP) < 0)
    ERREUR("Error MEDfamilyCr (create family 0)");

  // ########################################################################
  //          GROUPES DE NOEUDS
  // ########################################################################

  int nGroupesNoeuds = GN.size();

  vector<vector<int> > ETIQUETTES_N;
  ETIQUETTES_N.resize(nombreNoeudsMaillage);
  vector<unsigned int> INDEX_N;
  INDEX_N.resize(GN.size());
  vector<string> NOMSFAM;
  vector<vector<int> > ETIQFAM;
  int cptNOMFAM = 0;
  map<string, int> NUMFAMETIQ; //  clé = étiquette  -  valeur = numéros de familles


  if (nGroupesNoeuds)
    {

      // Pérennisation d'un ordre sur les GM dans le vecteur NOMS_GROUPES_MAILLES
      vector<string> NOMS_GROUPES_NOEUDS;
      for (map<string, vector<int> >::iterator ITGN = GN.begin(); ITGN != GN.end(); ITGN++)
        {
          string nomGN = ITGN->first;
          NOMS_GROUPES_NOEUDS.push_back(nomGN);
        }
      NOMS_GROUPES_NOEUDS.resize(GN.size());

      // Tri des vecteurs de noeuds de GN
      for (unsigned int ig = 0; ig < NOMS_GROUPES_NOEUDS.size(); ig++)
        sort(GN[NOMS_GROUPES_NOEUDS[ig]].begin(), GN[NOMS_GROUPES_NOEUDS[ig]].end());

      // Construction des étiquettes (familles)

      // Initialisation des index de groupes
      for (unsigned int ig = 0; ig < NOMS_GROUPES_NOEUDS.size(); ig++)
        INDEX_N[ig] = 0;

      for (int k = 1; k <= nombreNoeudsMaillage; k++)
        { // k: num. global de noeud
          int tailleEtiquette = 0;
          string etiq = (string) "";
          // Boucle sur les groupes
          for (unsigned int ig = 0; ig < NOMS_GROUPES_NOEUDS.size(); ig++)
            {
              if (INDEX_N[ig] < GN[NOMS_GROUPES_NOEUDS[ig]].size())
                {
                  string nomgroupe = NOMS_GROUPES_NOEUDS[ig];
                  if (k == GN[nomgroupe][INDEX_N[ig]])
                    {
                      // Attention: l'indice 0 dans le vecteur ETIQUETTES correspond
                      // à l'élément (noeud ou maille) de num. global 1
                      // Par ailleurs, le numéro de groupe dans l'étiquette commence à 0
                      ETIQUETTES_N[k - 1].push_back(ig);
                      tailleEtiquette++;
                      etiq += int2string(ig);
                      INDEX_N[ig]++;
                    }
                }
            }
          ETIQUETTES_N[k - 1].resize(tailleEtiquette);
          // Stockage de l'étiquette dans NOMSFAM ETIQFAM, si pas déjà stockée
          //          bool trouve = false;
          //          for (int i = 0; i < NOMSFAM.size(); i++)
          //            if (NOMSFAM[i] == ((string) "ETIQUETTE_" + etiq))
          //              {
          //                trouve = true;
          //                break;
          //              }
          if (!NUMFAMETIQ[etiq] && etiq != (string) "")
            {
              NOMSFAM.push_back((string) "ETIQN_" + etiq);
              ETIQFAM.push_back(ETIQUETTES_N[k - 1]);
              NUMFAMETIQ[etiq] = cptNOMFAM + 1; // Famille de noeuds, num>0
              cptNOMFAM++;
            }
        }

      NOMSFAM.resize(cptNOMFAM);
      ETIQFAM.resize(cptNOMFAM);

      // Création des familles de noeuds
      for (unsigned int ifam = 0; ifam < NOMSFAM.size(); ifam++)
        {
          strcpy(nomfam, string2char(NOMSFAM[ifam]));
          // Numéro de famille: ifam+1 (positif pour les noeuds + non nul)
          numfam = ifam + 1;
          ngro = ETIQFAM[ifam].size();

          // Noms des groupes de la famille: variable nomsGN
          char *gro = new char[ngro * MED_LNAME_SIZE + 1];
          int cptGN = 0;
          for (unsigned int ign = 0; ign < ETIQFAM[ifam].size(); ign++)
            {
              string nomGNcourant = NOMS_GROUPES_NOEUDS[ETIQFAM[ifam][ign]];
              // ATTENTION! Il faut mettre à la fin de chaque segment un \0 qui est ensuite écrasé
              // par le premier caractère du champ suivant dans le strcat !!!!
              if (ign == 0)
                {
                  // Premier groupe
                  strcpy(gro, string2char(nomGNcourant));
                  for (int jg = nomGNcourant.size(); jg < MED_LNAME_SIZE; jg++)
                    gro[jg] = ' ';
                  gro[MED_LNAME_SIZE] = '\0';
                }
              else
                {
                  strcat(gro, string2char(nomGNcourant));
                  for (int jg = nomGNcourant.size(); jg < MED_LNAME_SIZE; jg++)
                    gro[cptGN * MED_LNAME_SIZE + jg] = ' ';
                  gro[(cptGN + 1) * MED_LNAME_SIZE] = '\0';
                }
              cptGN++;
            }

          // Création de la famille
          if (MEDfamilyCr(fid, maa, nomfam, numfam, 0, MED_NO_GROUP) < 0)
            ERREUR("Error MEDfamilyCr");
          delete gro;
        }

    }

  // ########################################################################
  //          NOEUDS
  // ########################################################################

  //  float x, y, z;

  med_int nnoe = nombreNoeudsMaillage; // Nombre de noeuds
  med_float *coo = 0; // Table des coordonnées

  // Noms des coordonnées (variable nomcoo)
  char* nomcoo = new char[mdim * MED_SNAME_SIZE + 1];
  string strX = (string) "X";
  while (strX.size() < MED_SNAME_SIZE)
    strX += (string) " ";
  string strY = (string) "Y";
  while (strY.size() < MED_SNAME_SIZE)
    strY += (string) " ";
  string strZ = (string) "Z";
  while (strZ.size() < MED_SNAME_SIZE)
    strZ += (string) " ";
  if (mdim == 3)
    strcpy(nomcoo, string2char(strX + strY + strZ));
  else if (mdim == 2)
    strcpy(nomcoo, string2char(strX + strY));
  else
    strcpy(nomcoo, string2char(strX));
  nomcoo[mdim * MED_SNAME_SIZE] = '\0';

  // Unités des coordonnées (variable unicoo)
  char* unicoo = new char[mdim * MED_SNAME_SIZE + 1];
  string strmesure = (string) "SI";
  while (strmesure.size() < MED_SNAME_SIZE)
    strmesure += (string) " ";
  if (mdim == 3)
    strcpy(unicoo, string2char(strmesure + strmesure + strmesure));
  else if (mdim == 2)
    strcpy(unicoo, string2char(strmesure + strmesure));
  else
    strcpy(unicoo, string2char(strmesure));
  unicoo[mdim * MED_SNAME_SIZE] = '\0';

  // Tables des noms, numeros, numeros de familles des noeuds
  //    autant d'elements que de noeuds - les noms ont pout longueur MED_SNAME_SIZE
  char *nomnoe = 0;
  med_int *numnoe = NULL;
  med_int *nufano = NULL;
  med_bool inonoe = MED_FALSE;
  med_bool inunoe = MED_FALSE;

  // Allocations memoire
  if (nnoe > 0)
    {
      // table des coordonnees - profil : (dimension * nombre de noeuds )
      coo = (med_float*) calloc(nnoe * mdim, sizeof(med_float));

      // table des des numeros, des numeros de familles des noeuds - profil : (nombre de noeuds)
      //      numnoe = (med_int*) malloc(sizeof(med_int) * nnoe);
      nufano = (med_int*) malloc(sizeof(med_int) * nnoe);

      // table des noms des noeuds - profil : (nnoe*MED_SNAME_SIZE+1)
      nomnoe = (char*) ""; // ATTENTION!

      //      nomnoe = (char*) malloc(MED_SNAME_SIZE * nnoe + 1);
      //      for (int inoeud = 0; inoeud < IDS_NOEUDS.size(); inoeud++)
      //        {
      //          string nomNoeud = IDS_NOEUDS[inoeud];
      //          if (inoeud == 0)
      //            {
      //              // Premier groupe
      //              strcpy(nomnoe, string2char(nomNoeud));
      //              for (int jg = nomNoeud.size(); jg < MED_SNAME_SIZE; jg++)
      //                nomnoe[jg] = ' ';
      //              nomnoe[MED_SNAME_SIZE] = '\0';
      //            }
      //          else
      //            {
      //              strcat(nomnoe, string2char(nomNoeud));
      //              for (int jg = nomNoeud.size(); jg < MED_SNAME_SIZE; jg++)
      //                nomnoe[inoeud * MED_SNAME_SIZE + jg] = ' ';
      //              nomnoe[(inoeud + 1) * MED_SNAME_SIZE] = '\0';
      //            }
      //        }
    }

  // Chargement des coordonnées, numéros de familles et numéros de noeuds
  if (dimensionMaillage == 3)
    {
      int i3 = 0;
      for (int i = 0; i < nnoe; i++)
        {
          //          coo[i3] = X[i];
          //          coo[i3 + 1] = Y[i];
          //          coo[i3 + 2] = Z[i];
          //          i3 = i3 + 3;
          coo[i3] = *(XX + i);
          coo[i3 + 1] = *(YY + i);
          coo[i3 + 2] = *(ZZ + i);
          i3 = i3 + 3;

          // Numéros de familles  -  Le num. global de noeud est i+1
          if (nGroupesNoeuds)
            {
              vector<int> v = ETIQUETTES_N[i];
              string sv = (string) "";
              for (unsigned int j = 0; j < v.size(); j++)
                sv += int2string(v[j]); // Etiquette du noeud au format string
              // cout << "Noeud " << i + 1 << " : sv=" << sv << endl;
              *(nufano + i) = (med_int) NUMFAMETIQ[sv];
            }
          else
            *(nufano + i) = (med_int) 0;

          // Numéros de noeuds
          // *(numnoe + i) = (med_int) NUM_NOEUDS[i];
        }
    }
  else /* dimension 2 */
    {
      int i2 = 0;
      for (int i = 0; i < nnoe; i++)
        {
          coo[i2] = *(XX + i);
          coo[i2 + 1] = *(YY + i);
          i2 = i2 + 2;
          // Numéros de familles  -  Le num. global de noeud est i+1
          if (nGroupesNoeuds)
            {
              vector<int> v = ETIQUETTES_N[i];
              string sv = (string) "";
              for (unsigned int j = 0; j < v.size(); j++)
                sv += int2string(v[j]); // Etiquette du noeud au format string
              // cout << "Noeud " << i + 1 << " : sv=" << sv << endl;
              *(nufano + i) = (med_int) NUMFAMETIQ[sv];
            }
          else
            *(nufano + i) = (med_int) 0;
          // Numéros de noeuds
          // *(numnoe + i) = (med_int) NUM_NOEUDS[i];
        }
    }

  //   // Restitution coo
  //  int i3 = 0;
  //  for (int i = 0; i < nnoe; i++)
  //    {
  //      cout << "Noeud " << i << " : " << coo[i3] << " " << coo[i3 + 1] << " " << coo[i3 + 2] << endl;
  //      i3 = i3 + 3;
  //    }

  if (MEDmeshNodeWr(fid, maa, MED_NO_DT, MED_NO_IT, MED_UNDEF_DT, MED_FULL_INTERLACE, nnoe, coo, inonoe, nomnoe,
                    inunoe, numnoe, MED_TRUE, nufano) < 0)
    {
      ERREUR("Error MEDmeshNodeWr");
      cout << "Error MEDmeshNodeWr" << endl;
    }

  // ########################################################################
  //          GROUPES DE MAILLES
  // ########################################################################

  int nGroupesMailles = GM.size();

  map<TYPE_MAILLE, vector<vector<int> > > ETIQUETTES_M; // [ tm => [ nl => [ig1, ig2, ... ] ] ]
  // INDEX_M :
  //  Clé :       tm
  //  Valeur :    vect. des compteurs par indice de GM dans NOMS_GROUPES_MAILLES
  map<TYPE_MAILLE, vector<unsigned int> > INDEX_M;
  NOMSFAM.clear();
  ETIQFAM.clear();
  NUMFAMETIQ.clear(); //  clé = étiquette  -  valeur = numéros de familles
  cptNOMFAM = 0;

  if (nGroupesMailles)
    {

      // Pérennisation d'un ordre sur les GM dans le vecteur NOMS_GROUPES_MAILLES
      vector<string> NOMS_GROUPES_MAILLES;
      for (map<string, map<TYPE_MAILLE, vector<int> > >::iterator ITGM = GM.begin(); ITGM != GM.end(); ITGM++)
        {
          string nomGM = ITGM->first;
          NOMS_GROUPES_MAILLES.push_back(nomGM);
        }
      NOMS_GROUPES_MAILLES.resize(GM.size());
      // Tri des vecteurs d'entiers de GM
      for (unsigned int ig = 0; ig < NOMS_GROUPES_MAILLES.size(); ig++)
        {
          string nomGM = NOMS_GROUPES_MAILLES[ig];
          for (map<TYPE_MAILLE, vector<int> >::iterator I = GM[nomGM].begin(); I != GM[nomGM].end(); I++)
            {
              TYPE_MAILLE tm = I->first;
              sort(GM[nomGM][tm].begin(), GM[nomGM][tm].end());
            }
        }

      // Construction des étiquettes (familles)

      // Initialisation 0 des index de groupes, et resize ETIQUETTES_M[tm]
      for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
        {
          TYPE_MAILLE tm = (TYPE_MAILLE) itm;
          if (EFFECTIFS_TYPES[tm])
            {
              for (unsigned int ig = 0; ig < NOMS_GROUPES_MAILLES.size(); ig++)
                INDEX_M[tm].push_back(0);
              ETIQUETTES_M[tm].resize(EFFECTIFS_TYPES[tm]);
            }
        }

      for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
        {
          TYPE_MAILLE tm = (TYPE_MAILLE) itm;
          int efftm = EFFECTIFS_TYPES[tm];
          // cout << endl << "*************** coucou ***************" << endl;
          // cout << "*************** Type " << TM2string(tm) << " effectif = " << efftm << endl;
          if (efftm)
            {
              // cout << "Traitement du type " << TM2string(tm) << endl;
              for (int nl = 0; nl < efftm; nl++)
                {
                  // nl = num. local de la maille dans son type
                  // cout << "\tMaille " << TM2string(tm) << " n° " << nl << endl;

                  int tailleEtiquette = 0;
                  string etiq = (string) "";
                  // Boucle sur les groupes
                  for (unsigned int ig = 0; ig < NOMS_GROUPES_MAILLES.size(); ig++)
                    {
                      string nomGM = NOMS_GROUPES_MAILLES[ig];
                      // cout << "\t\t" << "Groupe " << nomGM << endl;

                      if (INDEX_M[tm][ig] < GM[nomGM][tm].size())
                        {
                          if (nl == GM[nomGM][tm][INDEX_M[tm][ig]])
                            {
                              // Attention: l'indice 0 dans le vecteur ETIQUETTES correspond
                              // à l'élément (noeud ou maille) de num. global 1
                              // Par ailleurs, le numéro de groupe dans l'étiquette commence à 0
                              // cout << "\t\t\t" << "La maille est dans le groupe " << nomGM << endl;
                              ETIQUETTES_M[tm][nl].push_back(ig);
                              tailleEtiquette++;
                              etiq += int2string(ig);
                              INDEX_M[tm][ig]++;
                              // cout << "\t\t\t  OK" << endl;
                            }
                        }
                    }

                  ETIQUETTES_M[tm][nl].resize(tailleEtiquette);
                  // Stockage de l'étiquette dans NOMSFAM ETIQFAM, si pas déjà stockée
                  //                  bool trouve = false;
                  //                  for (int i = 0; i < NOMSFAM.size(); i++)
                  //                    if (NOMSFAM[i] == ((string) "ETIQUETTE_" + etiq))
                  //                      {
                  //                        trouve = true;
                  //                        break;
                  //                      }

                  if (!NUMFAMETIQ[etiq] && etiq != (string) "")
                    {
                      NOMSFAM.push_back((string) "ETIQM_" + etiq);
                      ETIQFAM.push_back(ETIQUETTES_M[tm][nl]);
                      NUMFAMETIQ[etiq] = -cptNOMFAM - 1; // Famille de mailles, num<0
                      cptNOMFAM++;
                    }

                }

            } // if (efftm)
        }

      NOMSFAM.resize(cptNOMFAM);
      ETIQFAM.resize(cptNOMFAM);

      // Création des familles de mailles
      for (unsigned int ifam = 0; ifam < NOMSFAM.size(); ifam++)
        {
          strcpy(nomfam, string2char(NOMSFAM[ifam]));
          // Numéro de famille: -ifam-1 (négatif pour les mailles, et non nul)
          numfam = -ifam - 1;
          ngro = ETIQFAM[ifam].size();

          // Noms des groupes de la famille
          char* gro = new char[ngro * MED_LNAME_SIZE + 1];
          int cptGM = 0;
          for (unsigned int ign = 0; ign < ETIQFAM[ifam].size(); ign++)
            {
              string nomGMcourant = NOMS_GROUPES_MAILLES[ETIQFAM[ifam][ign]];
              // ATTENTION! Il faut mettre à la fin de chaque segment un \0 qui est ensuite écrasé
              // par le premier caractère du champ suivant dans le strcat !!!!
              if (ign == 0)
                {
                  // Premier groupe
                  strcpy(gro, string2char(nomGMcourant));
                  for (int jg = nomGMcourant.size(); jg < MED_LNAME_SIZE; jg++)
                    gro[jg] = ' ';
                  gro[MED_LNAME_SIZE] = '\0';
                }
              else
                {
                  strcat(gro, string2char(nomGMcourant));
                  for (int jg = nomGMcourant.size(); jg < MED_LNAME_SIZE; jg++)
                    gro[cptGM * MED_LNAME_SIZE + jg] = ' ';
                  gro[(cptGM + 1) * MED_LNAME_SIZE] = '\0';
                }
              cptGM++;
            }

          // Création de la famille
          if (MEDfamilyCr(fid, maa, nomfam, numfam, 1, gro) < 0)
            ERREUR("Error MEDfamilyCr");

          delete gro;
        }
    }

  // ########################################################################
  //                                MAILLES
  // ########################################################################
  //               Appel de la routine ecritureTypeNew

  med_bool inomTYPE = MED_FALSE;
  med_bool inumTYPE = MED_FALSE;

  med_geometry_type MGE;

  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      if (EFFECTIFS_TYPES[tm])
        {
          nTYPE = EFFECTIFS_TYPES[tm];
          //tTYPE = Nnoeuds(tm);
          MGE = InstanceMGE(tm);
          stype = TM2string(tm);

          // Noms des mailles
          //          char *nomTYPE = (char*) malloc(MED_SNAME_SIZE * nTYPE + 1);
          //          strcpy(nomTYPE, ""); // ATTENTION!

          char *nomTYPE = (char*)""; // Attention! Ne pas faire strcpy !

          //           for (int imaille = 0; imaille < IDS_MAILLES[tm].size(); imaille++)
          //            {
          //              string nomMaille = IDS_MAILLES[tm][imaille];
          //              if (imaille == 0)
          //                {
          //                  // Premier groupe
          //                  strcpy(nomTYPE, string2char(nomMaille));
          //                  for (int jg = nomMaille.size(); jg < MED_SNAME_SIZE; jg++)
          //                    nomTYPE[jg] = ' ';
          //                  nomTYPE[MED_SNAME_SIZE] = '\0';
          //                }
          //              else
          //                {
          //                  strcat(nomTYPE, string2char(nomMaille));
          //                  for (int jg = nomMaille.size(); jg < MED_SNAME_SIZE; jg++)
          //                    nomTYPE[imaille * MED_SNAME_SIZE + jg] = ' ';
          //                  nomTYPE[(imaille + 1) * MED_SNAME_SIZE] = '\0';
          //                }
          //            }

          med_int *numTYPE = NULL; //  (med_int*) malloc(sizeof(med_int)*nTYPE);

          med_int *famTYPE = (med_int*) malloc(sizeof(med_int) * nTYPE);

          //          med_int *conTYPE = (med_int*) malloc(sizeof(med_int) * tTYPE * nTYPE);
          //          for (int i = 0; i < nTYPE * tTYPE; i++)
          //            *(conTYPE + i) = (med_int) CON[tm][i];

          // Chargement famTYPE
          if (nGroupesMailles)
            {
              // Boucle sur les mailles du type (indice = num. local)
              for (int nl = 0; nl < nTYPE; nl++)
                {
                  // Construction de l'étiquette de la maille au format string
                  vector<int> v = ETIQUETTES_M[tm][nl];
                  string sv = (string) "";
                  for (unsigned int j = 0; j < v.size(); j++)
                    sv += int2string(v[j]);
                  // Accès au num. de la famille
                  *(famTYPE + nl) = (med_int) NUMFAMETIQ[sv];
                } // Boucle sur les mailles du type
            } // if (nGroupesMailles)
          else
            for (int nl = 0; nl < nTYPE; nl++)
              *(famTYPE + nl) = (med_int) 0;

          // Formatage MED des CNX
          conversionCNX(CNX[tm], tm, nTYPE);

          // Chargement numTYPE
          //for (int nl=0; nl<nTYPE; nl++)    *(numTYPE+nl) = (med_int) ( NUM_MAILLES[tm][nl] );
//          cerr << "maa=" << maa << endl;
//          cerr << "MGE=" << MGE << endl;
//          cerr << "nTYPE=" << nTYPE << endl;
//          cerr << "inomTYPE=" << inomTYPE << endl;
//          //cerr << "nomTYPE=" << nomTYPE << endl;
//          cerr << "inumTYPE=" << inumTYPE << endl;
//          this->afficheMailles(tm);
          if (MEDmeshElementWr(fid, maa, MED_NO_DT, MED_NO_IT, MED_UNDEF_DT, MED_CELL, MGE, MED_NODAL,
                               MED_FULL_INTERLACE, nTYPE, CNX[tm], inomTYPE, nomTYPE, inumTYPE, numTYPE, MED_FALSE,
                               famTYPE) < 0)
            {
              ERREUR("Error MEDmeshElementWr");
              cout << "Error MEDmeshElementWr, type " << stype << endl;
            }
          if (MEDmeshEntityFamilyNumberWr(fid, maa, MED_NO_DT, MED_NO_IT,
                                          MED_CELL, MGE, nTYPE, famTYPE) < 0)
            {
              ERREUR("Error MEDmeshEntityFamilyNumberWr");
              cout << "Error MEDmeshEntityFamilyNumberWr, type " << stype << endl;
            }

          // free(nomTYPE);
          // free(numTYPE);
          free(famTYPE);
          // free(conTYPE);

        } // Effectif non vide
    }

  // ########################################################################
  //                            Fermeture du fichier MED
  // ########################################################################

  if (MEDfileClose(fid) < 0)
    {
      ERREUR("Error on close MED file\n");
      cout << "Error on close MED file" << endl;
    }

  delete unicoo;
  delete nomcoo;

  // cout << endl << endl << "Fin procédure outputMED" << endl;
} // outputMED


int Maillage::NGLOBAL(TYPE_MAILLE typeMaille, int nlocal)
{
  // Attention, les num. globaux commencent à 1, les num. locaux à 0
  int cpt = 1 + nlocal;
  for (int itm = (int) POI1; itm < (int) typeMaille; itm++)
    { // Attention! inférieur strict!
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      cpt += EFFECTIFS_TYPES[tm];
    }
  return cpt;
}

TYPE_MAILLE Maillage::TYPE(int nglobal)
{
  // Attention, les num. globaux commencent à 1, les num. locaux à 0
  TYPE_MAILLE resultat = (TYPE_MAILLE)-1;
  int cpt = 0;
  for (int itm = (int) POI1; itm <= (int) HEXA20; itm++)
    {
      TYPE_MAILLE tm = (TYPE_MAILLE) itm;
      cpt += EFFECTIFS_TYPES[tm];
      if (nglobal <= cpt)
        {
          resultat = tm;
          break;
        }
    }
  return resultat;
}

int Maillage::NLOCAL(int nglobal, TYPE_MAILLE tm)
{
  // Attention, les num. globaux commencent à 1, les num. locaux à 0
  int nPOI1 = EFFECTIFS_TYPES[POI1];
  int nSEG2 = EFFECTIFS_TYPES[SEG2];
  int nSEG3 = EFFECTIFS_TYPES[SEG3];
  int nTRIA3 = EFFECTIFS_TYPES[TRIA3];
  int nTRIA6 = EFFECTIFS_TYPES[TRIA6];
  int nQUAD4 = EFFECTIFS_TYPES[QUAD4];
  int nQUAD8 = EFFECTIFS_TYPES[QUAD8];
  int nTETRA4 = EFFECTIFS_TYPES[TETRA4];
  int nTETRA10 = EFFECTIFS_TYPES[TETRA10];
  int nPYRAM5 = EFFECTIFS_TYPES[PYRAM5];
  int nPYRAM13 = EFFECTIFS_TYPES[PYRAM13];
  int nPENTA6 = EFFECTIFS_TYPES[PENTA6];
  int nPENTA15 = EFFECTIFS_TYPES[PENTA15];
  int nHEXA8 = EFFECTIFS_TYPES[HEXA8];
  int nHEXA20 = EFFECTIFS_TYPES[HEXA20];

  if (nglobal <= nPOI1)
    {
      return nglobal - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2)
    {
      return nglobal - nPOI1 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3)
    {
      return nglobal - nPOI1 - nSEG2 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10 + nPYRAM5)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - nTETRA10 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10 + nPYRAM5
      + nPYRAM13)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - nTETRA10 - nPYRAM5 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10 + nPYRAM5
      + nPYRAM13 + nPENTA6)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - nTETRA10 - nPYRAM5
          - nPYRAM13 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10 + nPYRAM5
      + nPYRAM13 + nPENTA6 + nPENTA15)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - nTETRA10 - nPYRAM5
          - nPYRAM13 - nPENTA6 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10 + nPYRAM5
      + nPYRAM13 + nPENTA6 + nPENTA15 + nHEXA8)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - nTETRA10 - nPYRAM5
          - nPYRAM13 - nPENTA6 - nPENTA15 - 1;
    }
  else if (nglobal <= nPOI1 + nSEG2 + nSEG3 + nTRIA3 + nTRIA6 + nQUAD4 + nQUAD8 + nTETRA4 + nTETRA10 + nPYRAM5
      + nPYRAM13 + nPENTA6 + nPENTA15 + nHEXA8 + nHEXA20)
    {
      return nglobal - nPOI1 - nSEG2 - nSEG3 - nTRIA3 - nTRIA6 - nQUAD4 - nQUAD8 - nTETRA4 - nTETRA10 - nPYRAM5
          - nPYRAM13 - nPENTA6 - nPENTA15 - nHEXA8 - 1;
    }
  else
    ERREUR("method NLOCAL: unknown type");
  return 0;
}

/*!
 *  Suppression de mailles dans un type :
 *
 *  - Contraction de la connectivité concernée
 *  - Réécriture des GM avec les nouveaux numéros locaux des éléments du type concerné
 *  - Mise à jour nombreMaillesMaillage
 *
 *  Les noeuds ne sont pas affectés.
 */
void Maillage::eliminationMailles(TYPE_MAILLE tm, vector<int> listeMaillesSuppr)
{
  map<int, int> TABLE_NL; // Table des num. locaux dans le type tm

  cout << "Method eliminationMailles, listeMaillesSuppr.size()=" << listeMaillesSuppr.size() << endl;

  // ************* Modification de la connectivité du type concerné

  int* CNX2;
  int nNoeudsType = Nnoeuds(tm);
  int tailleCNX2 = nNoeudsType * (EFFECTIFS_TYPES[tm] - listeMaillesSuppr.size());
  CNX2 = (int*) malloc(sizeof(int) * tailleCNX2);
  // Recopie sélective des connectivités
  int isuppr = 0; // indice dans listeMaillesSuppr
  int ih2 = 0; // nouveau numéro local ( remarque: ih2 = ih1 - isuppr )
  for (int ih1 = 0; ih1 < EFFECTIFS_TYPES[tm]; ih1++)
    {
      if (listeMaillesSuppr[isuppr] != ih1)
        {
          for (int jh1 = 0; jh1 < nNoeudsType; jh1++)
            *(CNX2 + nNoeudsType * ih2 + jh1) = *(CNX[tm] + nNoeudsType * ih1 + jh1);
          ih2++;
        }
      else
        isuppr++;
    }
  free(CNX[tm]);
  CNX[tm] = CNX2;

  // ************* Construction de la table de correspondance des NL dans le type concerné
  unsigned int offset = 0;
  for (int i = 0; i < EFFECTIFS_TYPES[tm]; i++)
    {
      if (offset < listeMaillesSuppr.size())
        {
          if (i < listeMaillesSuppr[offset])
            TABLE_NL[i] = i - offset;
          else if (i == listeMaillesSuppr[offset])
            {
              TABLE_NL[i] = -1; // Element à supprimer
              offset++;
            }
        }
      else
        TABLE_NL[i] = i - offset;
    }

  // Contrôle
  if (offset != listeMaillesSuppr.size())
    {
      ERREUR("Incoherent offset, method eliminationMailles");
      exit(0);
    }

  // ************* Mise à jour du type concerné dans les GM
  for (map<string, map<TYPE_MAILLE, vector<int> > >::iterator I = GM.begin(); I != GM.end(); I++)
    {
      string nomGM = I->first;

      if (GM[nomGM][tm].size())
        {
          //cout << "GM[" << nomGM <<"][" << tm << "].size()=" << GM[nomGM][tm].size() << endl;
          vector<int> mailles = GM[nomGM][tm];
          vector<int> mailles2; //mailles2.resize(mailles.size()-listeMaillesSuppr.size());
          unsigned int cptMailles = 0;
          for (unsigned int i = 0; i < mailles.size(); i++)
            {
              int nl2 = TABLE_NL[mailles[i]];
              if (nl2 != -1)
                {
                  mailles2.push_back(nl2);
                  cptMailles++;
                }
            }

          GM[nomGM][tm].clear();
          mailles2.resize(cptMailles);
          GM[nomGM][tm] = mailles2;

        }
    }

  // ************* Mise à jour des effectifs

  EFFECTIFS_TYPES[tm] = EFFECTIFS_TYPES[tm] - listeMaillesSuppr.size();
  nombreMaillesMaillage = nombreMaillesMaillage - listeMaillesSuppr.size();

  TABLE_NL.clear();
}

