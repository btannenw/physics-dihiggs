#include "utility.hpp"

#include <sstream>
#include <vector>

#include <TObjString.h>
#include <TObjArray.h>
#include <TH1.h>
#include <TString.h>

#include <iostream>
namespace Utils {

  std::vector<TString> splitString(TString orig, const char separator) {
    // splits the original string at 'separator' and fills the list
    // 'splitV' with the primitive strings
    std::vector<TString> splitV;
    orig.ReplaceAll("\n"," ");
    orig.Remove(TString::kBoth,separator);
    while (orig.Length()>0) {
      // TODO condition in while could be omitted by appending separator before loop
      if ( !orig.Contains(separator) ) {
        splitV.push_back(orig);
        break;
      }
      else {
        int pos = orig.First(separator);
        TString toSave = orig(0, pos);
        splitV.push_back(toSave);
        orig.Remove(0, pos);
      }
      orig.Remove(TString::kLeading,separator);
    }
    return splitV;
  } // SplitString

  TString ftos(float d) {
    std::stringstream s;
    s << d;
    return TString(s.str().c_str());
  }

  TString group(TObjArray* res, int i) {
    if(i >= res->GetEntriesFast()) {
      return "";
    }
    return ((TObjString*)res->At(i))->GetString();
  }

  TH1* symmetrize(const TH1* up, const TH1* nominal) {
    TH1* down = (TH1*)(nominal->Clone(nominal->GetName()+TString("Sym")));
    down->Scale(2);
    down->Add(up, -1);
    // std::cout << "after Add " << down << " " << up << std::endl;
    for (int bin = 0; bin < up->GetNbinsX()+2; ++bin) {
       const double err = up->GetBinError(bin);
       down->SetBinError(bin, err);
    }
    return down;
  }

} // namespace Utils
