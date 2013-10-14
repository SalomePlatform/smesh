#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import  subprocess


def Chercheversion(salomePath):
    home=os.environ['HOME']
    commande=os.path.abspath(os.path.join(home,salomePath,'runAppli'))
    argus="--version"
    p = subprocess.Popen([commande,argus], stdout=subprocess.PIPE)
    (output, err) = p.communicate()
    version=' '.join(output.split())
    return version


if __name__ == "__main__":
      print Chercheversion("/local00/home/A96028/Appli")

