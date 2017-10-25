#include "AtlasStyle.C"
#include <TH1D.h>
#include <TLine.h>
#include <TLatex.h>

void makeABCDplot()
{
  SetAtlasStyle();

  TH2D* abcd = new TH2D("abcd", "abcd", 25, 0, 500, 20, 0, 4);
  abcd->SetXTitle("p_{T}^{bb} [GeV]");
  abcd->SetYTitle("Lepton |#sigma_{d_{0}}|");
  //abcd->GetYaxis()->SetNdivisions(0);

  abcd->Draw();

  TLine *line1=new TLine(150, 0, 150, 4.0);
  line1->SetLineWidth(3);
  line1->SetLineColor(kRed);
  line1->Draw("same");

  TLine *line2=new TLine(0, 1.5, 500, 1.5);
  line2->SetLineWidth(3);
  line2->SetLineColor(kRed);
  line2->Draw("same");

  TLatex lat;
  lat.SetTextAlign(9);
  lat.SetTextFont(62);
  lat.SetTextSize(0.07);
  lat.SetNDC();
  lat.DrawLatex(0.65, 0.28, "A");
  lat.DrawLatex(0.65, 0.69, "C");
  lat.DrawLatex(0.265, 0.28, "B");
  lat.DrawLatex(0.265, 0.69, "D");

  TLatex lat2;
  lat2.SetTextAlign(9);
  lat2.SetTextFont(42);
  lat2.SetTextSize(0.05);
  lat2.SetNDC();

  TLine *line3=new TLine(135, 0, 135, 4.0);
  line3->SetLineWidth(3);
  line3->SetLineStyle(9);
  line3->SetLineColor(kRed);
  line3->Draw("same");

  lat2.DrawLatex(0.335, 0.75, "#leftarrow");
  lat2.DrawLatex(0.335, 0.25, "#leftarrow");
  
}
