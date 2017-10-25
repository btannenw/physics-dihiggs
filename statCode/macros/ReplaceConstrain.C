/*
  Description: Replace a gaussian in the workspace by a gaussian with a large standard deviation : allow to float the related nuisance parameters 
*/

#include "RooWorkspace.h"
#include "RooSimultaneous.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooGaussian.h"

#include "TFile.h"
#include "TString.h"

using namespace std;
using namespace RooFit;


void ReplaceConstrain(TString filename = "125.root",TString constrain_name = "SysZMbb"){
  
  TFile *f = new TFile(filename,"READ");
  RooWorkspace *combined = (RooWorkspace*)f->Get("combined");

  /*Define the name of the variable of the gaussian*/
  TString np_name = "alpha_"+constrain_name;
  TString go_name = "nom_alpha_"+constrain_name;
  TString gaussian_name = "alpha_"+constrain_name+"Constraint";   
  RooRealVar *var1 = (RooRealVar*)combined->var(np_name);
  RooRealVar *var2 = (RooRealVar*)combined->var(go_name);
  RooConstVar *const2 = new RooConstVar("10000000","10000000",10000000);

  /* The new gaussian with large standard deviation*/
  RooGaussian *gauss = new RooGaussian(gaussian_name+"Not",gaussian_name+"Not",*var1,*var2,*const2);
  
  /* Import in the workspace*/
  cout << "Import new gaussian in the workspace" << endl;
  combined->import(*gauss);
  cout << "Edit the workspace" << endl;
  /* Replace the gaussian in the workspace by the gaussian with large standard deviation*/
  TString factory = "EDIT::simPdf_2(simPdf,"+gaussian_name+"="+gaussian_name+"Not)";
  combined->factory(factory.Data());
  cout << "Check the changes" << endl;
  RooSimultaneous *simPdf = (RooSimultaneous*)combined->pdf("simPdf");
  RooSimultaneous *simPdf_2 = (RooSimultaneous*)combined->pdf("simPdf_2");
  cout << "Value of the pdf for " << np_name  << " = " << var1->getVal() << endl;
  cout <<"OLD pdf : " << simPdf->getVal() << endl; 
  cout <<"NEW pdf : " << simPdf_2->getVal() << endl; 
  var1->setVal(5); 
  cout << "Value of the pdf for " << np_name  << " = " << var1->getVal() << endl;
  cout <<"OLD pdf : " << simPdf->getVal() << endl; 
  cout <<"NEW pdf : " << simPdf_2->getVal() << endl; 
  var1->setVal(0); 
  cout << endl;

  /* New pdf has the default name*/
  simPdf->SetName("simPdf_OLD");
  simPdf->SetTitle("simPdf_OLD");  
  simPdf_2->SetName("simPdf");
  simPdf_2->SetTitle("simPdf");
  
  /*Writ in the new file*/
  filename.ReplaceAll(".root","cor.root");
  combined->writeToFile(filename);

  /* Check the change in the new file  */  
  cout << "Check in the new file" << endl;
  TFile *f2 = new TFile(filename,"READ");
  RooWorkspace *combined2 = (RooWorkspace*)f2->Get("combined");

  RooRealVar *var3 = (RooRealVar*)combined2->var(np_name);
  RooSimultaneous *simPdf_OLD = (RooSimultaneous*)combined2->pdf("simPdf_OLD");
  RooSimultaneous *simPdf_NEW = (RooSimultaneous*)combined2->pdf("simPdf");
    
  cout << "Value of the pdf for " << np_name  << " = " << var3->getVal() << endl;
  cout <<"OLD pdf : " << simPdf_OLD->getVal() << endl; 
  cout <<"NEW pdf : " << simPdf_NEW->getVal() << endl; 
  var3->setVal(5); 
  cout << "Value of the pdf for " << np_name  << " = " << var3->getVal() << endl;
  cout <<"OLD pdf : " << simPdf_OLD->getVal() << endl; 
  cout <<"NEW pdf : " << simPdf_NEW->getVal() << endl; 

}
