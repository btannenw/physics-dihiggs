#!/usr/bin/env python

import sys
import ROOT


def main():
    wsname = sys.argv[1]
    mass = "125"
    if len(sys.argv) > 2:
        mass = sys.argv[2]
    f = ROOT.TFile.Open("workspaces/"+wsname+"/combined/"+mass+".root")
    ws = f.Get("combined")
    mc = ws.obj("ModelConfig")
    np = mc.GetNuisanceParameters()
    nbNP = np.getSize()
    nbMCstatNP = 0
    nbNormNP = 0

    itr = np.createIterator()
    var = itr.Next()
    dictMCstatMP={}
    dictNormNP={}

    while var:
        name = var.GetName()
        if "gamma_stat" in name:
            nbMCstatNP += 1
            reg = name[:name.find("_bin_")]
            reg = reg.split("_",2)[2]
            if reg in dictMCstatMP:
                dictMCstatMP[reg] += 1
            else:
                dictMCstatMP[reg] = 1
        elif "ATLAS_norm" in name:
            nbNormNP += 1
            if name in dictNormNP:
                dictNormNP[name] += 1
            else:
                dictNormNP[name] = 1
        var = itr.Next()
    nbOtherNP = nbNP - nbMCstatNP - nbNormNP

    print "{0:d} NP, among them {1:d} are MC stat, {2:d} are free normalizations, and {3:d} are other \
(systematics)".format(nbNP, nbMCstatNP, nbNormNP, nbOtherNP)
    for k in sorted(dictMCstatMP.keys()):
        print "{0:3d} MC stat NP for region {1}".format(dictMCstatMP[k],k)
    for k in sorted(dictNormNP.keys()):
        print "{0:3d} free normalization NP for region {1}".format(dictNormNP[k],k)






if __name__=="__main__":
    main()
