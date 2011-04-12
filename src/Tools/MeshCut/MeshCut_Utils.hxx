#ifndef __MESHCUT_UTILS_HXX__
#define __MESHCUT_UTILS_HXX__

extern "C"
  {
#include <med.h>
  }

#include <string>
#include <vector>
#include <map>

namespace MESHCUT
  {

    enum TYPE_MAILLE
    {
      POI1, SEG2, SEG3, TRIA3, TRIA6, QUAD4, QUAD8, TETRA4, TETRA10, PYRAM5, PYRAM13, PENTA6, PENTA15, HEXA8, HEXA20
    };

    bool estUnTypeMaille(std::string S);
    void ERREUR(const char* msg);
    char* string2char(std::string str);
    std::string int2string(int k);
    float char2float(const char* ch);
    std::string float2string(float f);
    bool appartient(std::string e, std::string tableau[], int taille);
    float arrondi(float x);
    int numNoeudPointe(std::string b1, std::string b2, std::string b3);
    std::string strip(std::string S);
    std::string entierSur10_g(int i);
    std::string entierSur10_d(int i);
    std::string typeEnsight(std::string type);
    int Nnoeuds(TYPE_MAILLE type);
    int NnoeudsGeom(TYPE_MAILLE type);
    int codeGMSH(std::string type);
    std::string floatEnsight(float x);
    bool typeComplexe(std::string type);
    std::string ASTER8(std::string s);
    float dObservateur(float a, float b, float c);

    int copieFichier(std::string source, std::string cible);
    med_geometry_type InstanceMGE(TYPE_MAILLE TYPE);
    int chrono();
    TYPE_MAILLE typeMaille(std::string type);
    std::string MGE2string(med_geometry_type MGE);
    std::string TM2string(TYPE_MAILLE MGE);
    TYPE_MAILLE string2TM(std::string stm);
    std::string coordIndex_ILS(TYPE_MAILLE tm);
    std::string coordIndex_IFS(TYPE_MAILLE tm);
    std::string SIGNE(double x);
    void champType(std::string type, med_entity_type MEM, med_geometry_type MGE, med_idt fid, med_idt fidout,
                   char *maa, char *nomChamp, char *nomChampMoy, med_field_type typeChamp, char *compChamp,
                   char *unitChamp, med_int nCompChamp, std::map<std::string, int> REFGAUSS, int ichamp);
    std::string nomMaille(TYPE_MAILLE tm, int nl);
    bool appartientVN(int n, std::vector<int> V);
    float distance2(float x1, float y1, float z1, float x2, float y2, float z2);
    void conversionCNX(int *CNXtm, TYPE_MAILLE tm, int N);

  }

#endif
