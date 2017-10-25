#!/bin/bash

#Need to remove and remake the MV1c files for cut based due to name conflict with MVA

NLEP=0
if [ $NLEP = 0 ]
then
  LEPSTRING="ZeroLepton"
elif [ $NLEP = 1 ]
then
  LEPSTRING="OneLepton"
elif [ $NLEP = 2 ]
then
  LEPSTRING="TwoLepton"
fi

#cut remove the 0-120 and 120- mv1
if [ $NLEP != 0 ]
then
  rm 8TeV_${LEPSTRING}_CUT*vpt0_120_MV1c*.root
  #rm 8TeV_${LEPSTRING}_CUT*vpt0_120_mjj*.root
fi
rm 8TeV_${LEPSTRING}_CUT*vpt120_MV1c*.root
#rm 8TeV_${LEPSTRING}_CUT*vpt120_mjj*.root

#mva remove the 0-90, 90-120, 120-160 and 160-200, 200- mv1
if [ $NLEP != 0 ]
then
  rm 8TeV_${LEPSTRING}_MVA*vpt0_90_MV1c*.root
  rm 8TeV_${LEPSTRING}_MVA*vpt90_120_MV1c*.root
fi
rm 8TeV_${LEPSTRING}_MVA*vpt120_160_MV1c*.root
rm 8TeV_${LEPSTRING}_MVA*vpt160_200_MV1c*.root
rm 8TeV_${LEPSTRING}_MVA*vpt200_MV1c*.root

#make merged cut bins for mv1cb1 and mv1cb2
if [ $NLEP != 0 ]
then
  hadd 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_90_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt90_120_MV1cB1.root 
fi
hadd 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_160_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt160_200_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt200_MV1cB1.root 
if [ $NLEP != 0 ]
then
  hadd 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_90_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt90_120_MV1cB1.root 
fi
hadd 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_160_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt160_200_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt200_MV1cB1.root 
if [ $NLEP != 0 ]
then
  hadd 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_120_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_90_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt90_120_MV1cB2.root 
fi
hadd 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_160_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt160_200_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt200_MV1cB2.root 
if [ $NLEP != 0 ]
then
  hadd 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_120_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_90_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt90_120_MV1cB2.root 
fi
hadd 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_160_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt160_200_MV1cB2.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt200_MV1cB2.root 


#make the btag files
if [ $NLEP != 0 ]
then
  hadd 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_120_MV1cBTag.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt0_120_MV1cB2.root
fi
hadd 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_MV1cBTag.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag2jet_vpt120_MV1cB2.root
if [ $NLEP != 0 ]
then
  hadd 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_120_MV1cBTag.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt0_120_MV1cB2.root
fi
hadd 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_MV1cBTag.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_MV1cB1.root 8TeV_${LEPSTRING}_CUT_1tag3jet_vpt120_MV1cB2.root
