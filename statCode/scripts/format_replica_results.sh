#!/bin/bash
MAX_REPLICA=999
WS=SMVH_mva_v14puw_bootstrapfulldataset
VERSION=test1
#NLEP="0"
NLEP="1"
#NLEP="2"
#NLEP="012"
#default variable to try
PARAM="SigXsec"

#choose value or error: default false
DOERR=false

#normal usage - this is default choice
#formatting "SigXsec" false
formatting()
{
  if [ "$1" ]
  then
    PARAM=$1
  fi
  if [ "$2" ]
  then
    DOERR=$2
  fi
  echo "Running over parameter $PARAM"
  echo "Running error is $DOERR"

  if [ "${DOERR}" = false ]
  then
    VALERR=2
  else
    VALERR=4
  fi

  OUT=${PARAM}

  if [ "${DOERR}" = true ]
  then
    OUT+="Err"
  fi

  OUT+="Results"
  OUT+=$NLEP
  OUT+=".txt"

  rm -f $OUT
  touch $OUT

  echo "Writing to file $OUT"

  for i in `seq 0 $MAX_REPLICA`;
  do
    #may need to tweak regions slightly
    CUT="fccs/FitCrossChecks_${WS}.${VERSION}_VHbbRun2_13TeV_${VERSION}_${NLEP}_125_Systs_use1tagFalse_use4pjetFalse_mBB_combined"
    MVA="fccs/FitCrossChecks_${WS}.${VERSION}_VHbbRun2_13TeV_${VERSION}_${NLEP}_125_Systs_use1tagFalse_use4pjetFalse_mva_combined"
    CUTVAL=-999
    MVAVAL=-999
    if [ -f "$CUT/output_0.log" -a -f "$MVA/output_0.log" ]
    then
      if grep -q 'FIT FAILED' "$CUT/output_0.log" || grep -q 'FIT FAILED' "$MVA/output_0.log"
      then
        continue
      fi
      CUTVAL=$(grep $PARAM $CUT/output_0.log|grep -v 'conditionnal'|tail -n1|awk '{print $'$VALERR'}')
      MVAVAL=$(grep $PARAM $MVA/output_0.log|grep -v 'conditionnal'|tail -n1|awk '{print $'$VALERR'}')
      if [ "${DOERR}" = true ]
      then
        echo ${CUTVAL#-} ${MVAVAL#-} ${i} >> $OUT
      else
        echo ${CUTVAL} ${MVAVAL} ${i} >> $OUT
      fi
    fi
  done

}

declare -a PARAMS=("SigXsec")
if [[ "$NLEP" =~ "012" ]]
then
  PARAMS=("SigXsec" "norm_Wbb" "norm_Zbb" "norm_Zcl" "norm_Wcl" "norm_ttbar_L0" "norm_ttbar_L1" "norm_ttbar_L2" "SysMETScaleSoftTerms" "SysJetNP1" "SysJetNP2" "SysJetNP3" "SysJetNP4" "SysJetNP6_rest")
elif [[ "$NLEP" =~ "2" ]]
then
  PARAMS=("SigXsec" "norm_Zbb" "norm_Zcl" "norm_ttbar" "SysMETScaleSoftTerms" "SysJetNP1" "SysJetNP2" "SysJetNP3" "SysJetNP4" "SysJetNP6_rest" "SysZblZbbRatio" "SysZDPhi_J2_ZbORc" "SysZMbb_ZbORc")
elif [[ "$NLEP" =~ "1" ]]
then
  PARAMS=("SigXsec" "norm_Wbb" "norm_ttbar" "SysMETScaleSoftTerms" "SysJetNP1" "SysJetNP2" "SysJetNP3" "SysJetNP4" "SysJetNP6_rest")
elif [[ "$NLEP" =~ "0" ]]
then
  PARAMS=("SigXsec" "norm_Zbb" "norm_ttbar" "SysJetNP1" "SysJetNP2" "SysJetNP3" "SysJetNP4" "SysJetNP6_rest" "SysMETScaleSoftTerms")
fi


#Run the central value and error for each of the parameters in PARAMS
echo "Format result for ${#PARAMS[@]} parameters"

for p in "${PARAMS[@]}"
do
  for er in false true
  do
    formatting $p $er
  done
done



