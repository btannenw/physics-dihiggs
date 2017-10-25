#ifndef MAKEASIMOV
#define MAKEASIMOV

#include "RooRealVar.h"
#include "RooStats/ModelConfig.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "RooMinimizer.h"

#include "TIterator.h"

#include <sstream>
#include <string>
#include <map>

using namespace std;
using namespace RooFit;
using namespace RooStats;

//pr_mode (profile mode):
//0=mu=0
//1=mu=1
//2=muhat
void locallocalUnfoldConstraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter);
RooDataSet* makeAsimovData(ModelConfig* mcInWs, bool doConditional, RooWorkspace* combWS, RooAbsPdf* combPdf, RooDataSet* combData, double mu_asimov)
{

  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
  ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);
  ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
  //RooMinuit::SetMaxIterations(10000);
  //RooMinimizer::SetMaxFunctionCalls(10000);

////////////////////
//make asimov data//
////////////////////

  int _printLevel = 0;
  stringstream muStr;
  //muStr << setprecision(3);
  muStr << "_" << mu_asimov;
  
  RooRealVar* mu = (RooRealVar*)mcInWs->GetParametersOfInterest()->first();//combWS->var("mu");
  mu->setVal(mu_asimov);

  RooArgSet mc_obs = *mcInWs->GetObservables();
  RooArgSet mc_globs = *mcInWs->GetGlobalObservables();
  RooArgSet mc_nuis = *mcInWs->GetNuisanceParameters();

//pair the nuisance parameter to the global observable
  RooArgSet mc_nuis_tmp = mc_nuis;
  RooArgList nui_list("ordered_nuis");
  RooArgList glob_list("ordered_globs");
  RooArgSet constraint_set_tmp(*combPdf->getAllConstraints(mc_obs, mc_nuis_tmp, false));
  RooArgSet constraint_set;
  int counter_tmp = 0;
  locallocalUnfoldConstraints(constraint_set_tmp, constraint_set, mc_obs, mc_nuis_tmp, counter_tmp);

  TIterator* cIter = constraint_set.createIterator();
  RooAbsArg* arg;
  while ((arg = (RooAbsArg*)cIter->Next()))
  {
    RooAbsPdf* pdf = (RooAbsPdf*)arg;
    if (!pdf) continue;
//     cout << "Printing pdf" << endl;
//     pdf->Print();
//     cout << "Done" << endl;
    TIterator* nIter = mc_nuis.createIterator();
    RooRealVar* thisNui = NULL;
    RooAbsArg* nui_arg;
    while ((nui_arg = (RooAbsArg*)nIter->Next()))
    {
      if (pdf->dependsOn(*nui_arg))
      {
	thisNui = (RooRealVar*)nui_arg;
	break;
      }
    }
    delete nIter;

    //RooRealVar* thisNui = (RooRealVar*)pdf->getObservables();


//need this incase the observable isn't fundamental. 
//in this case, see which variable is dependent on the nuisance parameter and use that.
    RooArgSet* components = pdf->getComponents();
//     cout << "\nPrinting components" << endl;
//     components->Print();
//     cout << "Done" << endl;
    components->remove(*pdf);
    if (components->getSize())
    {
      TIterator* itr1 = components->createIterator();
      RooAbsArg* arg1;
      while ((arg1 = (RooAbsArg*)itr1->Next()))
      {
	TIterator* itr2 = components->createIterator();
	RooAbsArg* arg2;
	while ((arg2 = (RooAbsArg*)itr2->Next()))
	{
	  if (arg1 == arg2) continue;
	  if (arg2->dependsOn(*arg1))
	  {
	    components->remove(*arg1);
	  }
	}
	delete itr2;
      }
      delete itr1;
    }
    if (components->getSize() > 1)
    {
      cout << "ERROR::Couldn't isolate proper nuisance parameter" << endl;
      return NULL;
    }
    else if (components->getSize() == 1)
    {
      thisNui = (RooRealVar*)components->first();
    }



    TIterator* gIter = mc_globs.createIterator();
    RooRealVar* thisGlob = NULL;
    RooAbsArg* glob_arg;
    while ((glob_arg = (RooAbsArg*)gIter->Next()))
    {
      if (pdf->dependsOn(*glob_arg))
      {
	thisGlob = (RooRealVar*)glob_arg;
	break;
      }
    }
    delete gIter;

    if (!thisNui || !thisGlob)
    {
      cout << "WARNING::Couldn't find nui or glob for constraint: " << pdf->GetName() << endl;
      //return;
      continue;
    }

    cout << "Pairing nui: " << thisNui->GetName() << ", with glob: " << thisGlob->GetName() << ", from constraint: " << pdf->GetName() << endl;

    nui_list.add(*thisNui);
    glob_list.add(*thisGlob);

//     cout << "\nPrinting Nui/glob" << endl;
//     thisNui->Print();
//     cout << "Done nui" << endl;
//     thisGlob->Print();
//     cout << "Done glob" << endl;
  }
  delete cIter;




//save the snapshots of nominal parameters
  combWS->saveSnapshot("nominalGlobs",*mcInWs->GetGlobalObservables());
  combWS->saveSnapshot("nominalNuis", *mcInWs->GetNuisanceParameters());

  RooArgSet nuiSet_tmp(nui_list);

  mu->setVal(mu_asimov);
  mu->setConstant(1);
  

  int status = 0;
  if (doConditional)
  {
    cout << "Starting minimization.." << endl;
    RooAbsReal* nll = combPdf->createNLL(*combData, RooFit::Constrain(nuiSet_tmp));
    RooMinimizer minim(*nll);
    minim.setStrategy(2);
    minim.setPrintLevel(1);
    status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
    if (status != 0)
    {
      cout << "Fit failed for mu = " << mu->getVal() << " with status " << status << endl;
    }
    cout << "Done" << endl;

    //combPdf->fitTo(*combData,Hesse(false),Minos(false),PrintLevel(0),Extended(), Constrain(nuiSet_tmp));
  }
  mu->setConstant(0);



//loop over the nui/glob list, grab the corresponding variable from the tmp ws, and set the glob to the value of the nui
  int nrNuis = nui_list.getSize();
  if (nrNuis != glob_list.getSize())
  {
    cout << "ERROR::nui_list.getSize() != glob_list.getSize()!" << endl;
    return NULL;
  }

  for (int i=0;i<nrNuis;i++)
  {
    RooRealVar* nui = (RooRealVar*)nui_list.at(i);
    RooRealVar* glob = (RooRealVar*)glob_list.at(i);

    cout << "nui: " << nui << ", glob: " << glob << endl;
    cout << "Setting glob: " << glob->GetName() << ", which had previous val: " << glob->getVal() << ", to conditional val: " << nui->getVal() << endl;

    glob->setVal(nui->getVal());
  }

//save the snapshots of conditional parameters
  cout << "Saving conditional snapshots" << endl;
  combWS->saveSnapshot(("conditionalGlobs"+muStr.str()).c_str(),*mcInWs->GetGlobalObservables());
  combWS->saveSnapshot(("conditionalNuis" +muStr.str()).c_str(),*mcInWs->GetNuisanceParameters());

  if (!doConditional)
  {
    combWS->loadSnapshot("nominalGlobs");
    combWS->loadSnapshot("nominalNuis");
  }

  cout << "Making asimov" << endl;

  mu->setVal(mu_asimov);
  ModelConfig* mc = mcInWs;

  int iFrame=0;

  const char* weightName="weightVar";
  RooArgSet obsAndWeight;
  cout << "adding obs" << endl;
  obsAndWeight.add(*mc->GetObservables());
  cout << "adding weight" << endl;

  RooRealVar* weightVar = NULL;
  if (!(weightVar = combWS->var(weightName)))
  {
    combWS->import(*(new RooRealVar(weightName, weightName, 1,0,10000000)));
    weightVar = combWS->var(weightName);
  }
  cout << "weightVar: " << weightVar << endl;
  obsAndWeight.add(*combWS->var(weightName));

  cout << "defining set" << endl;
  combWS->defineSet("obsAndWeight",obsAndWeight);


  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////
  // MAKE ASIMOV DATA FOR OBSERVABLES

  // dummy var can just have one bin since it's a dummy
  if(combWS->var("ATLAS_dummyX"))  combWS->var("ATLAS_dummyX")->setBins(1);

  cout <<" check expectedData by category"<<endl;
  //RooDataSet* simData=NULL;
  RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mc->GetPdf());

  map<string, RooDataSet*> asimovDataMap;

    
  //try fix for sim pdf
  RooCategory* channelCat = (RooCategory*)&simPdf->indexCat();//(RooCategory*)combWS->cat("master_channel");//(RooCategory*) (&simPdf->indexCat());
  //    TIterator* iter = simPdf->indexCat().typeIterator() ;
  TIterator* iter = channelCat->typeIterator() ;
  RooCatType* tt = NULL;
  int nrIndices = 0;
  while((tt=(RooCatType*) iter->Next())) {
    nrIndices++;
  }
  for (int i=0;i<nrIndices;i++){
    channelCat->setIndex(i);
    iFrame++;
    // Get pdf associated with state from simpdf
    RooAbsPdf* pdftmp = simPdf->getPdf(channelCat->getLabel()) ;
	
    // Generate observables defined by the pdf associated with this state
    RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

    if (_printLevel >= 1)
    {
      obstmp->Print();
      cout << "on type " << channelCat->getLabel() << " " << iFrame << endl;
    }

    RooDataSet* obsDataUnbinned = new RooDataSet(Form("combAsimovData%d",iFrame),Form("combAsimovData%d",iFrame),RooArgSet(obsAndWeight,*channelCat),WeightVar(*weightVar));
    RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
    double expectedEvents = pdftmp->expectedEvents(*obstmp);
    double thisNorm = 0;
    for(int jj=0; jj<thisObs->numBins(); ++jj){
      thisObs->setBin(jj);

      thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
      if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18)) obsDataUnbinned->add(*mc->GetObservables(), thisNorm*expectedEvents);
    }
    
    if (_printLevel >= 1)
    {
      obsDataUnbinned->Print();
      cout <<"sum entries "<<obsDataUnbinned->sumEntries()<<endl;
    }
    if(obsDataUnbinned->sumEntries()!=obsDataUnbinned->sumEntries()){
      cout << "sum entries is nan"<<endl;
      exit(1);
    }

    ((RooRealVar*)obstmp->first())->Print();
    cout << "pdf: " << pdftmp->GetName() << endl;
    cout << "expected events " << pdftmp->expectedEvents(*obstmp) << endl;
    cout << "-----" << endl;

    asimovDataMap[string(channelCat->getLabel())] = obsDataUnbinned;//tempData;

    if (_printLevel >= 1)
    {
      cout << "channel: " << channelCat->getLabel() << ", data: ";
      obsDataUnbinned->Print();
      cout << endl;
    }
  }

  RooDataSet* asimovData = new RooDataSet(("asimovData"+muStr.str()).c_str(),("asimovData"+muStr.str()).c_str(),RooArgSet(obsAndWeight,*channelCat),Index(*channelCat),Import(asimovDataMap),WeightVar(*weightVar));
  if (combWS->data(asimovData->GetName()))
  {
    cout << "Import 1" << endl;
    double status_asimov = combWS->import(*asimovData, true);
    cout << "->status=" << status_asimov << endl;
  }
  else
  {
    cout << "Import 2" << endl;
    combWS->import(*asimovData);
  }

//bring us back to nominal for exporting
  combWS->loadSnapshot("nominalNuis");
  combWS->loadSnapshot("nominalGlobs");

  return asimovData;
}


 void locallocalUnfoldConstraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter)
 {
   if (counter > 50)
   {
     cout << "ERROR::Couldn't unfold constraints!" << endl;
     cout << "Initial: " << endl;
     initial.Print("v");
     cout << endl;
     cout << "Final: " << endl;
     final.Print("v");
     exit(1);
   }
   TIterator* itr = initial.createIterator();
   RooAbsPdf* pdf;
   while ((pdf = (RooAbsPdf*)itr->Next()))
   {
     RooArgSet nuis_tmp = nuis;
     RooArgSet constraint_set(*pdf->getAllConstraints(obs, nuis_tmp, false));
     //if (constraint_set.getSize() > 1)
     //{
     string className(pdf->ClassName());
     if (className != "RooGaussian" && className != "RooLognormal" && className != "RooGamma" && className != "RooPoisson" && className != "RooBifurGauss")
     {
       counter++;
       locallocalUnfoldConstraints(constraint_set, final, obs, nuis, counter);
     }
     else
     {
       final.add(*pdf);
     }
   }
   delete itr;
 }


#endif
