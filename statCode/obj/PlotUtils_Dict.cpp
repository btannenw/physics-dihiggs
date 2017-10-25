#include "plotUtils.hpp"
#include "roofitUtils.hpp"
#include <vector>
#include <utility>
#include <map>
#include <set>
#include <TGraph.h>
#include <RooAbsPdf.h>
#include <RooProdPdf.h>
#include <RooRealVar.h>
#include <RooAbsData.h>
#include <RooCurve.h>
#include <RooExpandedFitResult.h>
// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME objdIPlotUtils_Dict

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments

// Header files passed via #pragma extra_include

namespace PU {
   namespace ROOT {
      inline ::ROOT::TGenericClassInfo *GenerateInitInstance();
      static TClass *PU_Dictionary();

      // Function generating the singleton type initializer
      inline ::ROOT::TGenericClassInfo *GenerateInitInstance()
      {
         static ::ROOT::TGenericClassInfo 
            instance("PU", 0 /*version*/, "plotUtils.hpp", 42,
                     ::ROOT::Internal::DefineBehavior((void*)0,(void*)0),
                     &PU_Dictionary, 0);
         return &instance;
      }
      // Insure that the inline function is _not_ optimized away by the compiler
      ::ROOT::TGenericClassInfo *(*_R__UNIQUE_(InitFunctionKeeper))() = &GenerateInitInstance;  
      // Static variable to force the class initialization
      static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstance(); R__UseDummy(_R__UNIQUE_(Init));

      // Dictionary for non-ClassDef classes
      static TClass *PU_Dictionary() {
         return GenerateInitInstance()->GetClass();
      }

   }
}

namespace RU {
   namespace ROOT {
      inline ::ROOT::TGenericClassInfo *GenerateInitInstance();
      static TClass *RU_Dictionary();

      // Function generating the singleton type initializer
      inline ::ROOT::TGenericClassInfo *GenerateInitInstance()
      {
         static ::ROOT::TGenericClassInfo 
            instance("RU", 0 /*version*/, "roofitUtils.hpp", 21,
                     ::ROOT::Internal::DefineBehavior((void*)0,(void*)0),
                     &RU_Dictionary, 0);
         return &instance;
      }
      // Insure that the inline function is _not_ optimized away by the compiler
      ::ROOT::TGenericClassInfo *(*_R__UNIQUE_(InitFunctionKeeper))() = &GenerateInitInstance;  
      // Static variable to force the class initialization
      static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstance(); R__UseDummy(_R__UNIQUE_(Init));

      // Dictionary for non-ClassDef classes
      static TClass *RU_Dictionary() {
         return GenerateInitInstance()->GetClass();
      }

   }
}

namespace ROOT {
   static TClass *pairlETStringcOintgR_Dictionary();
   static void pairlETStringcOintgR_TClassManip(TClass*);
   static void *new_pairlETStringcOintgR(void *p = 0);
   static void *newArray_pairlETStringcOintgR(Long_t size, void *p);
   static void delete_pairlETStringcOintgR(void *p);
   static void deleteArray_pairlETStringcOintgR(void *p);
   static void destruct_pairlETStringcOintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const pair<TString,int>*)
   {
      pair<TString,int> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(pair<TString,int>));
      static ::ROOT::TGenericClassInfo 
         instance("pair<TString,int>", "string", 96,
                  typeid(pair<TString,int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &pairlETStringcOintgR_Dictionary, isa_proxy, 4,
                  sizeof(pair<TString,int>) );
      instance.SetNew(&new_pairlETStringcOintgR);
      instance.SetNewArray(&newArray_pairlETStringcOintgR);
      instance.SetDelete(&delete_pairlETStringcOintgR);
      instance.SetDeleteArray(&deleteArray_pairlETStringcOintgR);
      instance.SetDestructor(&destruct_pairlETStringcOintgR);
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const pair<TString,int>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *pairlETStringcOintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const pair<TString,int>*)0x0)->GetClass();
      pairlETStringcOintgR_TClassManip(theClass);
   return theClass;
   }

   static void pairlETStringcOintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   static void delete_RooExpandedFitResult(void *p);
   static void deleteArray_RooExpandedFitResult(void *p);
   static void destruct_RooExpandedFitResult(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooExpandedFitResult*)
   {
      ::RooExpandedFitResult *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooExpandedFitResult >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooExpandedFitResult", ::RooExpandedFitResult::Class_Version(), "RooExpandedFitResult.h", 18,
                  typeid(::RooExpandedFitResult), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooExpandedFitResult::Dictionary, isa_proxy, 4,
                  sizeof(::RooExpandedFitResult) );
      instance.SetDelete(&delete_RooExpandedFitResult);
      instance.SetDeleteArray(&deleteArray_RooExpandedFitResult);
      instance.SetDestructor(&destruct_RooExpandedFitResult);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooExpandedFitResult*)
   {
      return GenerateInitInstanceLocal((::RooExpandedFitResult*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooExpandedFitResult*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooExpandedFitResult::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooExpandedFitResult::Class_Name()
{
   return "RooExpandedFitResult";
}

//______________________________________________________________________________
const char *RooExpandedFitResult::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooExpandedFitResult*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooExpandedFitResult::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooExpandedFitResult*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooExpandedFitResult::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooExpandedFitResult*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooExpandedFitResult::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooExpandedFitResult*)0x0)->GetClass(); }
   return fgIsA;
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_pairlETStringcOintgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) pair<TString,int> : new pair<TString,int>;
   }
   static void *newArray_pairlETStringcOintgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) pair<TString,int>[nElements] : new pair<TString,int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_pairlETStringcOintgR(void *p) {
      delete ((pair<TString,int>*)p);
   }
   static void deleteArray_pairlETStringcOintgR(void *p) {
      delete [] ((pair<TString,int>*)p);
   }
   static void destruct_pairlETStringcOintgR(void *p) {
      typedef pair<TString,int> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class pair<TString,int>

//______________________________________________________________________________
void RooExpandedFitResult::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooExpandedFitResult.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(RooExpandedFitResult::Class(),this);
   } else {
      R__b.WriteClassBuffer(RooExpandedFitResult::Class(),this);
   }
}

namespace ROOT {
   // Wrapper around operator delete
   static void delete_RooExpandedFitResult(void *p) {
      delete ((::RooExpandedFitResult*)p);
   }
   static void deleteArray_RooExpandedFitResult(void *p) {
      delete [] ((::RooExpandedFitResult*)p);
   }
   static void destruct_RooExpandedFitResult(void *p) {
      typedef ::RooExpandedFitResult current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::RooExpandedFitResult

namespace ROOT {
   static TClass *vectorlEpairlETStringcOintgRsPgR_Dictionary();
   static void vectorlEpairlETStringcOintgRsPgR_TClassManip(TClass*);
   static void *new_vectorlEpairlETStringcOintgRsPgR(void *p = 0);
   static void *newArray_vectorlEpairlETStringcOintgRsPgR(Long_t size, void *p);
   static void delete_vectorlEpairlETStringcOintgRsPgR(void *p);
   static void deleteArray_vectorlEpairlETStringcOintgRsPgR(void *p);
   static void destruct_vectorlEpairlETStringcOintgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<pair<TString,int> >*)
   {
      vector<pair<TString,int> > *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<pair<TString,int> >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<pair<TString,int> >", -2, "vector", 214,
                  typeid(vector<pair<TString,int> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEpairlETStringcOintgRsPgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<pair<TString,int> >) );
      instance.SetNew(&new_vectorlEpairlETStringcOintgRsPgR);
      instance.SetNewArray(&newArray_vectorlEpairlETStringcOintgRsPgR);
      instance.SetDelete(&delete_vectorlEpairlETStringcOintgRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEpairlETStringcOintgRsPgR);
      instance.SetDestructor(&destruct_vectorlEpairlETStringcOintgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<pair<TString,int> > >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<pair<TString,int> >*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEpairlETStringcOintgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<pair<TString,int> >*)0x0)->GetClass();
      vectorlEpairlETStringcOintgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEpairlETStringcOintgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEpairlETStringcOintgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<pair<TString,int> > : new vector<pair<TString,int> >;
   }
   static void *newArray_vectorlEpairlETStringcOintgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<pair<TString,int> >[nElements] : new vector<pair<TString,int> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEpairlETStringcOintgRsPgR(void *p) {
      delete ((vector<pair<TString,int> >*)p);
   }
   static void deleteArray_vectorlEpairlETStringcOintgRsPgR(void *p) {
      delete [] ((vector<pair<TString,int> >*)p);
   }
   static void destruct_vectorlEpairlETStringcOintgRsPgR(void *p) {
      typedef vector<pair<TString,int> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<pair<TString,int> >

namespace ROOT {
   static TClass *vectorlETStringgR_Dictionary();
   static void vectorlETStringgR_TClassManip(TClass*);
   static void *new_vectorlETStringgR(void *p = 0);
   static void *newArray_vectorlETStringgR(Long_t size, void *p);
   static void delete_vectorlETStringgR(void *p);
   static void deleteArray_vectorlETStringgR(void *p);
   static void destruct_vectorlETStringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TString>*)
   {
      vector<TString> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TString>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TString>", -2, "vector", 214,
                  typeid(vector<TString>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETStringgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TString>) );
      instance.SetNew(&new_vectorlETStringgR);
      instance.SetNewArray(&newArray_vectorlETStringgR);
      instance.SetDelete(&delete_vectorlETStringgR);
      instance.SetDeleteArray(&deleteArray_vectorlETStringgR);
      instance.SetDestructor(&destruct_vectorlETStringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TString> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<TString>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETStringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TString>*)0x0)->GetClass();
      vectorlETStringgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETStringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETStringgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TString> : new vector<TString>;
   }
   static void *newArray_vectorlETStringgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TString>[nElements] : new vector<TString>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETStringgR(void *p) {
      delete ((vector<TString>*)p);
   }
   static void deleteArray_vectorlETStringgR(void *p) {
      delete [] ((vector<TString>*)p);
   }
   static void destruct_vectorlETStringgR(void *p) {
      typedef vector<TString> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TString>

namespace ROOT {
   static TClass *vectorlETGraphmUgR_Dictionary();
   static void vectorlETGraphmUgR_TClassManip(TClass*);
   static void *new_vectorlETGraphmUgR(void *p = 0);
   static void *newArray_vectorlETGraphmUgR(Long_t size, void *p);
   static void delete_vectorlETGraphmUgR(void *p);
   static void deleteArray_vectorlETGraphmUgR(void *p);
   static void destruct_vectorlETGraphmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TGraph*>*)
   {
      vector<TGraph*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TGraph*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TGraph*>", -2, "vector", 214,
                  typeid(vector<TGraph*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETGraphmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<TGraph*>) );
      instance.SetNew(&new_vectorlETGraphmUgR);
      instance.SetNewArray(&newArray_vectorlETGraphmUgR);
      instance.SetDelete(&delete_vectorlETGraphmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlETGraphmUgR);
      instance.SetDestructor(&destruct_vectorlETGraphmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TGraph*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<TGraph*>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETGraphmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<TGraph*>*)0x0)->GetClass();
      vectorlETGraphmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETGraphmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETGraphmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TGraph*> : new vector<TGraph*>;
   }
   static void *newArray_vectorlETGraphmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<TGraph*>[nElements] : new vector<TGraph*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETGraphmUgR(void *p) {
      delete ((vector<TGraph*>*)p);
   }
   static void deleteArray_vectorlETGraphmUgR(void *p) {
      delete [] ((vector<TGraph*>*)p);
   }
   static void destruct_vectorlETGraphmUgR(void *p) {
      typedef vector<TGraph*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<TGraph*>

namespace ROOT {
   static TClass *vectorlERooRealVarmUgR_Dictionary();
   static void vectorlERooRealVarmUgR_TClassManip(TClass*);
   static void *new_vectorlERooRealVarmUgR(void *p = 0);
   static void *newArray_vectorlERooRealVarmUgR(Long_t size, void *p);
   static void delete_vectorlERooRealVarmUgR(void *p);
   static void deleteArray_vectorlERooRealVarmUgR(void *p);
   static void destruct_vectorlERooRealVarmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<RooRealVar*>*)
   {
      vector<RooRealVar*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<RooRealVar*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<RooRealVar*>", -2, "vector", 214,
                  typeid(vector<RooRealVar*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlERooRealVarmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<RooRealVar*>) );
      instance.SetNew(&new_vectorlERooRealVarmUgR);
      instance.SetNewArray(&newArray_vectorlERooRealVarmUgR);
      instance.SetDelete(&delete_vectorlERooRealVarmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlERooRealVarmUgR);
      instance.SetDestructor(&destruct_vectorlERooRealVarmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<RooRealVar*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<RooRealVar*>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlERooRealVarmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<RooRealVar*>*)0x0)->GetClass();
      vectorlERooRealVarmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlERooRealVarmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlERooRealVarmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooRealVar*> : new vector<RooRealVar*>;
   }
   static void *newArray_vectorlERooRealVarmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooRealVar*>[nElements] : new vector<RooRealVar*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlERooRealVarmUgR(void *p) {
      delete ((vector<RooRealVar*>*)p);
   }
   static void deleteArray_vectorlERooRealVarmUgR(void *p) {
      delete [] ((vector<RooRealVar*>*)p);
   }
   static void destruct_vectorlERooRealVarmUgR(void *p) {
      typedef vector<RooRealVar*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<RooRealVar*>

namespace ROOT {
   static TClass *vectorlERooCurvemUgR_Dictionary();
   static void vectorlERooCurvemUgR_TClassManip(TClass*);
   static void *new_vectorlERooCurvemUgR(void *p = 0);
   static void *newArray_vectorlERooCurvemUgR(Long_t size, void *p);
   static void delete_vectorlERooCurvemUgR(void *p);
   static void deleteArray_vectorlERooCurvemUgR(void *p);
   static void destruct_vectorlERooCurvemUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<RooCurve*>*)
   {
      vector<RooCurve*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<RooCurve*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<RooCurve*>", -2, "vector", 214,
                  typeid(vector<RooCurve*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlERooCurvemUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<RooCurve*>) );
      instance.SetNew(&new_vectorlERooCurvemUgR);
      instance.SetNewArray(&newArray_vectorlERooCurvemUgR);
      instance.SetDelete(&delete_vectorlERooCurvemUgR);
      instance.SetDeleteArray(&deleteArray_vectorlERooCurvemUgR);
      instance.SetDestructor(&destruct_vectorlERooCurvemUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<RooCurve*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<RooCurve*>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlERooCurvemUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<RooCurve*>*)0x0)->GetClass();
      vectorlERooCurvemUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlERooCurvemUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlERooCurvemUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooCurve*> : new vector<RooCurve*>;
   }
   static void *newArray_vectorlERooCurvemUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooCurve*>[nElements] : new vector<RooCurve*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlERooCurvemUgR(void *p) {
      delete ((vector<RooCurve*>*)p);
   }
   static void deleteArray_vectorlERooCurvemUgR(void *p) {
      delete [] ((vector<RooCurve*>*)p);
   }
   static void destruct_vectorlERooCurvemUgR(void *p) {
      typedef vector<RooCurve*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<RooCurve*>

namespace ROOT {
   static TClass *vectorlERooAbsPdfmUgR_Dictionary();
   static void vectorlERooAbsPdfmUgR_TClassManip(TClass*);
   static void *new_vectorlERooAbsPdfmUgR(void *p = 0);
   static void *newArray_vectorlERooAbsPdfmUgR(Long_t size, void *p);
   static void delete_vectorlERooAbsPdfmUgR(void *p);
   static void deleteArray_vectorlERooAbsPdfmUgR(void *p);
   static void destruct_vectorlERooAbsPdfmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<RooAbsPdf*>*)
   {
      vector<RooAbsPdf*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<RooAbsPdf*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<RooAbsPdf*>", -2, "vector", 214,
                  typeid(vector<RooAbsPdf*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlERooAbsPdfmUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<RooAbsPdf*>) );
      instance.SetNew(&new_vectorlERooAbsPdfmUgR);
      instance.SetNewArray(&newArray_vectorlERooAbsPdfmUgR);
      instance.SetDelete(&delete_vectorlERooAbsPdfmUgR);
      instance.SetDeleteArray(&deleteArray_vectorlERooAbsPdfmUgR);
      instance.SetDestructor(&destruct_vectorlERooAbsPdfmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<RooAbsPdf*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<RooAbsPdf*>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlERooAbsPdfmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<RooAbsPdf*>*)0x0)->GetClass();
      vectorlERooAbsPdfmUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlERooAbsPdfmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlERooAbsPdfmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooAbsPdf*> : new vector<RooAbsPdf*>;
   }
   static void *newArray_vectorlERooAbsPdfmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooAbsPdf*>[nElements] : new vector<RooAbsPdf*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlERooAbsPdfmUgR(void *p) {
      delete ((vector<RooAbsPdf*>*)p);
   }
   static void deleteArray_vectorlERooAbsPdfmUgR(void *p) {
      delete [] ((vector<RooAbsPdf*>*)p);
   }
   static void destruct_vectorlERooAbsPdfmUgR(void *p) {
      typedef vector<RooAbsPdf*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<RooAbsPdf*>

namespace ROOT {
   static TClass *vectorlERooAbsDatamUgR_Dictionary();
   static void vectorlERooAbsDatamUgR_TClassManip(TClass*);
   static void *new_vectorlERooAbsDatamUgR(void *p = 0);
   static void *newArray_vectorlERooAbsDatamUgR(Long_t size, void *p);
   static void delete_vectorlERooAbsDatamUgR(void *p);
   static void deleteArray_vectorlERooAbsDatamUgR(void *p);
   static void destruct_vectorlERooAbsDatamUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<RooAbsData*>*)
   {
      vector<RooAbsData*> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<RooAbsData*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<RooAbsData*>", -2, "vector", 214,
                  typeid(vector<RooAbsData*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlERooAbsDatamUgR_Dictionary, isa_proxy, 4,
                  sizeof(vector<RooAbsData*>) );
      instance.SetNew(&new_vectorlERooAbsDatamUgR);
      instance.SetNewArray(&newArray_vectorlERooAbsDatamUgR);
      instance.SetDelete(&delete_vectorlERooAbsDatamUgR);
      instance.SetDeleteArray(&deleteArray_vectorlERooAbsDatamUgR);
      instance.SetDestructor(&destruct_vectorlERooAbsDatamUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<RooAbsData*> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const vector<RooAbsData*>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlERooAbsDatamUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<RooAbsData*>*)0x0)->GetClass();
      vectorlERooAbsDatamUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlERooAbsDatamUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlERooAbsDatamUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooAbsData*> : new vector<RooAbsData*>;
   }
   static void *newArray_vectorlERooAbsDatamUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<RooAbsData*>[nElements] : new vector<RooAbsData*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlERooAbsDatamUgR(void *p) {
      delete ((vector<RooAbsData*>*)p);
   }
   static void deleteArray_vectorlERooAbsDatamUgR(void *p) {
      delete [] ((vector<RooAbsData*>*)p);
   }
   static void destruct_vectorlERooAbsDatamUgR(void *p) {
      typedef vector<RooAbsData*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<RooAbsData*>

namespace ROOT {
   static TClass *setlETStringgR_Dictionary();
   static void setlETStringgR_TClassManip(TClass*);
   static void *new_setlETStringgR(void *p = 0);
   static void *newArray_setlETStringgR(Long_t size, void *p);
   static void delete_setlETStringgR(void *p);
   static void deleteArray_setlETStringgR(void *p);
   static void destruct_setlETStringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const set<TString>*)
   {
      set<TString> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(set<TString>));
      static ::ROOT::TGenericClassInfo 
         instance("set<TString>", -2, "set", 90,
                  typeid(set<TString>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &setlETStringgR_Dictionary, isa_proxy, 4,
                  sizeof(set<TString>) );
      instance.SetNew(&new_setlETStringgR);
      instance.SetNewArray(&newArray_setlETStringgR);
      instance.SetDelete(&delete_setlETStringgR);
      instance.SetDeleteArray(&deleteArray_setlETStringgR);
      instance.SetDestructor(&destruct_setlETStringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Insert< set<TString> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const set<TString>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *setlETStringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const set<TString>*)0x0)->GetClass();
      setlETStringgR_TClassManip(theClass);
   return theClass;
   }

   static void setlETStringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_setlETStringgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) set<TString> : new set<TString>;
   }
   static void *newArray_setlETStringgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) set<TString>[nElements] : new set<TString>[nElements];
   }
   // Wrapper around operator delete
   static void delete_setlETStringgR(void *p) {
      delete ((set<TString>*)p);
   }
   static void deleteArray_setlETStringgR(void *p) {
      delete [] ((set<TString>*)p);
   }
   static void destruct_setlETStringgR(void *p) {
      typedef set<TString> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class set<TString>

namespace ROOT {
   static TClass *maplETStringcOintgR_Dictionary();
   static void maplETStringcOintgR_TClassManip(TClass*);
   static void *new_maplETStringcOintgR(void *p = 0);
   static void *newArray_maplETStringcOintgR(Long_t size, void *p);
   static void delete_maplETStringcOintgR(void *p);
   static void deleteArray_maplETStringcOintgR(void *p);
   static void destruct_maplETStringcOintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<TString,int>*)
   {
      map<TString,int> *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<TString,int>));
      static ::ROOT::TGenericClassInfo 
         instance("map<TString,int>", -2, "map", 96,
                  typeid(map<TString,int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplETStringcOintgR_Dictionary, isa_proxy, 4,
                  sizeof(map<TString,int>) );
      instance.SetNew(&new_maplETStringcOintgR);
      instance.SetNewArray(&newArray_maplETStringcOintgR);
      instance.SetDelete(&delete_maplETStringcOintgR);
      instance.SetDeleteArray(&deleteArray_maplETStringcOintgR);
      instance.SetDestructor(&destruct_maplETStringcOintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<TString,int> >()));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const map<TString,int>*)0x0); R__UseDummy(_R__UNIQUE_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplETStringcOintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<TString,int>*)0x0)->GetClass();
      maplETStringcOintgR_TClassManip(theClass);
   return theClass;
   }

   static void maplETStringcOintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplETStringcOintgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<TString,int> : new map<TString,int>;
   }
   static void *newArray_maplETStringcOintgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<TString,int>[nElements] : new map<TString,int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplETStringcOintgR(void *p) {
      delete ((map<TString,int>*)p);
   }
   static void deleteArray_maplETStringcOintgR(void *p) {
      delete [] ((map<TString,int>*)p);
   }
   static void destruct_maplETStringcOintgR(void *p) {
      typedef map<TString,int> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<TString,int>

namespace {
  void TriggerDictionaryInitialization_PlotUtils_Dict_Impl() {
    static const char* headers[] = {
0
    };
    static const char* includePaths[] = {
"./include/",
"./transform/",
"/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/6.06.02-x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker_oct6_svnCheck/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "PlotUtils_Dict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate("$clingAutoload$plotUtils.hpp")))  TString;
namespace std{template <typename _Tp> class __attribute__((annotate("$clingAutoload$string")))  allocator;
}
namespace std{template <typename _Tp> struct __attribute__((annotate("$clingAutoload$string")))  less;
}
namespace std{template <class _T1, class _T2> struct __attribute__((annotate("$clingAutoload$string")))  pair;
}
class __attribute__((annotate("$clingAutoload$roofitUtils.hpp")))  TGraph;
class __attribute__((annotate("$clingAutoload$roofitUtils.hpp")))  RooAbsPdf;
class __attribute__((annotate("$clingAutoload$roofitUtils.hpp")))  RooRealVar;
class __attribute__((annotate("$clingAutoload$roofitUtils.hpp")))  RooAbsData;
class __attribute__((annotate("$clingAutoload$roofitUtils.hpp")))  RooCurve;
class __attribute__((annotate(R"ATTRDUMP(Container class for expanded fit result)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$RooExpandedFitResult.h")))  RooExpandedFitResult;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "PlotUtils_Dict dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooExpandedFitResult", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("PlotUtils_Dict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_PlotUtils_Dict_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_PlotUtils_Dict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_PlotUtils_Dict() {
  TriggerDictionaryInitialization_PlotUtils_Dict_Impl();
}
