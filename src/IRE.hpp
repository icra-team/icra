#ifndef CPROVER_IRE_HPP
#define CPROVER_IRE_HPP

extern "C" {
#include <caml/custom.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/memory.h>
}

#include "wali/ref_ptr.hpp"
#include "wali/Countable.hpp"

#include "IRE_callbacks.hpp"
//#include "wali/domains/duet/DuetRel.hpp"

// ICRA Regular Expressions
namespace IRE {

  class RegExp;
  class RegExpT;

  typedef wali::ref_ptr<RegExp> regExpRefPtr;

  class RegExp : public wali::Countable {
    private:
      value v; // This is a pointer to the OCaml object representing the regular expression
    public:
      RegExp(value _v) : v(_v) {
          caml_register_generational_global_root(&v);
      }

      ~RegExp() {
          //caml_remove_global_root(&v); // currently we leak RegExps, but we could avoid it
          // Note that if we wanted to avoid this leakage, it would be wise to think about
          //   cases where we are destroying a RegExp that points to some child that we
          //   are still using elsewhere.
      }

      static regExpRefPtr mkWeightFromWrapper(value wrapper_value);
      static regExpRefPtr mkWeight(relation_t weight);
      static regExpRefPtr mkVar(int variable);

      static regExpRefPtr mkZero();
      static regExpRefPtr mkOne();

      regExpRefPtr mkPlus(regExpRefPtr other);
      regExpRefPtr mkDot(regExpRefPtr other);      
      regExpRefPtr mkKleene();      
      regExpRefPtr mkProject();      

      void print();

      bool isOne();
      bool isZero();
      bool isVar();
      bool isWeight();
      bool isProject();
      bool isKleene();
      bool isPlus();
      bool isDot();

      relation_t eval();
      regExpRefPtr getLChild();
      regExpRefPtr getRChild();

      //detensor
      
      //isolate
      //substFree
      
      regExpRefPtr isolate(int var);
      regExpRefPtr substFree(int var, regExpRefPtr substitute);

      //DuetRel::DuetRel(const DuetRel& that):
      //  //wali::SemElemTensor(that),
      //  NPARel(that),
      //  relId(that.relId),
      //  isTensored(that.isTensored)
      //{
      
      //DuetRel::~DuetRel() {}
      
      //ref_ptr& operator=( const ref_ptr& rp ) {
      //  return *this = rp.get_ptr();
      //}
  };

  
  class RegExpT {

      //mkKleeneT
      //mkPlusT
      //mkDotT
      //mkVarT
      //mkTensor
      //mkProjectT
      //
      //evalT

      //isOneT
      //isZeroT
      //isVarT
      //isProjectT
      //isKleeneT
      //isPlusT
      //isDotT
      //isTensor

      //getLChildT
      //getRChildT
  };

  void clearEvalCaches();


}

#endif
