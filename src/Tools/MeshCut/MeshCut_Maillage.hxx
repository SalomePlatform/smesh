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

#ifndef __MESHCUT_MAILLAGE_HXX__
#define __MESHCUT_MAILLAGE_HXX__

#include "MeshCut_Utils.hxx"

#include <string>
#include <vector>
#include <map>

namespace MESHCUT
  {
    class Maillage
    {
    public:
      // Toutes ces variables doivent être placées au niveau du principal pour être connues de toutes les fonctions

      // Vecteurs des familles d'éléments
      std::map<int, std::vector<int> > FAMILLES;
      std::map<int, std::vector<TYPE_MAILLE> > FAM_TYPES;

      // Vecteurs des familles de noeuds
      std::map<int, std::vector<int> > FAMILLES_NOEUDS;

      // Description des groupes (par familles)
      std::map<std::string, std::vector<int> > GROUPES_MAILLES;
      std::map<std::string, std::vector<int> > GROUPES_NOEUDS;

      // Commun noeuds et mailles
      std::map<int, int> tailleFAMILLES; // la clé est un num. de famille
      std::map<std::string, int> tailleGROUPES; // Tailles des vecteurs GROUPES_MAILLES et GROUPES_NOEUDS - la clé est un nom de groupe (de noeuds ou de mailles)

      // Résidus mailles
      std::map<TYPE_MAILLE, std::vector<int> > RESIDU;

    public:
      std::string ID;
      int nombreNoeudsMaillage; // ****** MED-OBLIGATOIRE ******
      int nombreMaillesMaillage; // ****** MED-OBLIGATOIRE ******

      class Cube *enveloppeMaillage;

      int dimensionMaillage; // ****** MED-OBLIGATOIRE ******
      int dimensionEspace; // ****** MED-OBLIGATOIRE ******

      char axisname[3*MED_SNAME_SIZE+1]; // ****** MED-OBLIGATOIRE ******
      char unitname[3*MED_SNAME_SIZE+1]; // ****** MED-OBLIGATOIRE ******

      float *XX;
      float *YY;
      float *ZZ; // ****** MED-OBLIGATOIRE ******

      // Effectifs des éléments par type
      std::map<TYPE_MAILLE, int> EFFECTIFS_TYPES; // ****** MED-OBLIGATOIRE ******

      // Connectivités des types
      // Le numéro global du j-ième noeud de la maille de numéro global i est stocké à l'adresse
      //           CNX[tm]+t*(i-1)+(j-1)
      // (t = taille du type, i.e. nombre de noeuds de l'élément)
      std::map<TYPE_MAILLE, int*> CNX; // ****** MED-OBLIGATOIRE ******

      // Enveloppes cubiques
      std::map<TYPE_MAILLE, float*> EC;

      // Description des groupes
      std::map<std::string, std::map<TYPE_MAILLE, std::vector<int> > > GM;
      std::map<std::string, std::vector<int> > GN;

      //  std::vector<std::string> IDS_NOEUDS; // Indice = num. global - 1
      //  std::map<TYPE_MAILLE, std::vector<std::string> > IDS_MAILLES; // Indice = num local de maille dans le type
      //
      //  std::vector<int> NUM_NOEUDS; // Indice = num. global - 1
      //  std::map<TYPE_MAILLE, std::vector<int> > NUM_MAILLES; // Indice = num local de maille dans le type

      Maillage(std::string _ID);
      virtual ~Maillage();

      void creationGMtype(TYPE_MAILLE tm, std::string nomGMtype);
      void afficheMailles(TYPE_MAILLE tm);
      void listeMaillesType(TYPE_MAILLE tm);
      void listeMaillesTousTypes();
      void listeMaillesParGM();
      void listeMaillesGM(std::string nomGM);
      // void listeMaillesGMordonne(std::string nomGM);
      void listeNoeuds();
      void listeNoeudsGN(std::string nomGN);
      void listeNoeudsGNordonne(std::string nomGN);
      std::vector<float> G(int i, TYPE_MAILLE tm);
      float distanceNoeudMaille(int ngnoeud, int imaille, TYPE_MAILLE tm);
      int noeudVoisin(int ngnoeud, int imaille, TYPE_MAILLE tm);
      float distanceNoeudNoeud(int ng1, int ng2);
      //  void encombrements()

      void inputMED(std::string fichierMED);
      void outputMED(std::string fichierMED);
      void outputMEDold(std::string fichierMED);

      void inputHL(std::string fichierHL /*, std::string ficDICnoeuds, std::string ficDICmailles, bool DICO */);
      void outputHL(std::string fichierHL);

      void outputVRML(std::string ficVRML, float rNoeuds, char *renduAretes, char *renduFaces, float transparence);
      // std::string vrmlType(TYPE_MAILLE tm, char *renduAretes, char *renduFaces, float transparence);
      // void Maillage::creationGMtype(TYPE_MAILLE tm, std::vector<int> CON_TYPE);
      int NGLOBAL(TYPE_MAILLE typeMaille, int nlocal);
      int NLOCAL(int nglobal, TYPE_MAILLE tm);
      TYPE_MAILLE TYPE(int nglobal);
      void eliminationMailles(TYPE_MAILLE typeMaille, std::vector<int> listeMaillesSuppr);

      // acquisitionTYPE_inputMED appelée par inputMED
      void
          acquisitionTYPE_inputMED(TYPE_MAILLE TYPE, int nTYPE, med_idt fid, char maa[MED_NAME_SIZE + 1], med_int mdim);

      // void infoChamps(std::string type, med_entity_type MEM, med_geometry_type MGE, med_idt fid, char *maa,
      //                 char *nomChamp, med_field_type typeChamp, med_int nCompChamp, std::map<std::string, int> REFGAUSS);

      bool NoeudDansHEXA8(int n, int n0, int n1, int n2, int n3, int n4, int n5, int n6, int n7, float epsilon);
      bool NoeudDansPENTA6(int n, int n0, int n1, int n2, int n3, int n4, int n5, float epsilon);
      bool NoeudDansPYRAM5(int n, int n0, int n1, int n2, int n3, int n4, float epsilon);
      bool NoeudDansTETRA4(int n, int n1, int n2, int n3, int n4, float epsilon);
      bool NoeudDansQUAD4(int n, int n1, int n2, int n3, int n4, float epsilon);
      bool NoeudDansTRIA3(int n, int n1, int n2, int n3, float epsilon);
      double volumeTETRA(int n1, int n2, int n3, int n4);
      double aireTRIA(int n1, int n2, int n3);
      double DET3(int n1, int n2, int n3);
      double DET2(int n1, int n2);
      void * chargeEnveloppesCubiques(TYPE_MAILLE tm);
      void * chargeEnveloppesCarrees(TYPE_MAILLE tm);
      bool noeudDeMaille(int ngnoeud, int i, TYPE_MAILLE tm);
      bool NoeudDansMaille3D(int n, int i, TYPE_MAILLE tm, float epsilon);
      bool NoeudDansMaille2D(int n, int i, TYPE_MAILLE tm, float epsilon);
      bool NoeudDansEnveloppeMaille2D(int n, int i, TYPE_MAILLE tm, float epsilon);
      bool NoeudDansEnveloppeMaille3D(int n, int i, TYPE_MAILLE tm, float epsilon);
      void * afficheEnveloppesCubiques(TYPE_MAILLE tm);
      void * afficheEnveloppesCarrees(TYPE_MAILLE tm);

      std::vector<int> noeudsGeomCommuns(int i1, TYPE_MAILLE tm1, int i2, TYPE_MAILLE tm2);
      void creationGMresidu();

      float longueurMoyenne();
      // void Maillage::infoChamps2(std::string type, med_entity_type MEM, med_geometry_type MGE, med_idt fid,
      //                            char *maa, char *nomChamp, med_field_type typeChamp, med_int nCompChamp, map<string,
      //                            int> REFGAUSS);

    };
  }

#endif
