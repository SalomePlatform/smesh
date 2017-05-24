#!/usr/bin/env python3

import sys
import os
import  subprocess


def Chercheversion(salomePath):
    home=os.environ['HOME']
    commande=os.path.abspath(os.path.join(home,salomePath,'salome'))
    argus="--version"
    p = subprocess.Popen([commande,argus], stdout=subprocess.PIPE)
    (output, err) = p.communicate()
    version=' '.join(output.decode().split())
    return version


if __name__ == "__main__":
      print(Chercheversion("/local00/home/A96028/Appli"))

