#include "ire.hpp"

#define STORE_CALLBACK_IN_VAR(N, F) static value * F = NULL; \
                                    if (F == NULL) { \
                                        F = caml_named_value(N); \
                                        if (F == NULL) { \
                                            assert(0 && "Failed to obtain OCaml callback: " N); \
                                        } \
                                    } 

namespace IRE {

  // ---------------------------------------------------------------------
  //   RegExp (untensored) methods:
  
  regExpRefPtr RegExp::mkWeightFromWrapper(value wrapper_value) {
      STORE_CALLBACK_IN_VAR("IRE_mkWeight", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, wrapper_value));
      return retval;
  }
  
  regExpRefPtr RegExp::mkWeight(relation_t weight) {
      value wrapper_value = mkWeightWrapper(weight);
      return mkWeightFromWrapper(wrapper_value);
  }
  
  regExpRefPtr RegExp::mkVar(int variable) {
      STORE_CALLBACK_IN_VAR("IRE_mkVar", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, Val_int(variable)));
      return retval;
  }
  
  regExpRefPtr RegExp::mkZero() {
      STORE_CALLBACK_IN_VAR("IRE_mkZero", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, Val_unit));
      return retval;
  }
  
  regExpRefPtr RegExp::mkOne() {
      STORE_CALLBACK_IN_VAR("IRE_mkOne", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, Val_unit));
      return retval;
  }
  
  regExpRefPtr RegExp::mkPlus(regExpRefPtr other) {
      STORE_CALLBACK_IN_VAR("IRE_mkPlus", func);
      regExpRefPtr retval = new RegExp(caml_callback2(*func, this->v, other->v));
      return retval;
  }
  
  regExpRefPtr RegExp::mkDot(regExpRefPtr other) {
      STORE_CALLBACK_IN_VAR("IRE_mkDot", func);
      regExpRefPtr retval = new RegExp(caml_callback2(*func, this->v, other->v));
      return retval;
  }
  
  regExpRefPtr RegExp::mkKleene() {
      STORE_CALLBACK_IN_VAR("IRE_mkKleene", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, this->v));
      return retval;
  }
  
  regExpRefPtr RegExp::mkProject() {
      STORE_CALLBACK_IN_VAR("IRE_mkProject", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, this->v));
      return retval;
  }
  
  void RegExp::print() {
      STORE_CALLBACK_IN_VAR("IRE_printRegExp", func);
      caml_callback(*func, this->v);
  }
  
  bool RegExp::isOne() {
      STORE_CALLBACK_IN_VAR("IRE_isOne", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isZero() {
      STORE_CALLBACK_IN_VAR("IRE_isZero", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isVar() {
      STORE_CALLBACK_IN_VAR("IRE_isVar", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isWeight() {
      STORE_CALLBACK_IN_VAR("IRE_isWeight", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isProject() {
      STORE_CALLBACK_IN_VAR("IRE_isProject", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isKleene() {
      STORE_CALLBACK_IN_VAR("IRE_isKleene", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isPlus() {
      STORE_CALLBACK_IN_VAR("IRE_isPlus", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  bool RegExp::isDot() {
      STORE_CALLBACK_IN_VAR("IRE_isDot", func);
      value retval = caml_callback(*func, this->v);
      return Bool_val(retval);
  }
  
  relation_t RegExp::eval() {
      STORE_CALLBACK_IN_VAR("IRE_evalRegExp", func);
      value weight = caml_callback(*func, this->v);
      return getRelationFromValueWrapper(weight);
  }
 
  // Is this the best way to do this?
  regExpRefPtr RegExp::getLChild() {
      STORE_CALLBACK_IN_VAR("IRE_getLChild", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, this->v));
      return retval;
  }

  regExpRefPtr RegExp::getRChild() {
      STORE_CALLBACK_IN_VAR("IRE_getRChild", func);
      regExpRefPtr retval = new RegExp(caml_callback(*func, this->v));
      return retval;
  }

  regExpRefPtr RegExp::isolate(int var) {
      STORE_CALLBACK_IN_VAR("IRE_isolate", func);
      regExpRefPtr retval = new RegExp(caml_callback2(*func, Val_int(var), this->v));
      return retval;
  }

  regExpRefPtr RegExp::substFree(int var, regExpRefPtr substitute) {
      // Note order of parameters: Haystack.substfree(5, needle)
      STORE_CALLBACK_IN_VAR("IRE_substFree", func);
      regExpRefPtr retval = new RegExp(caml_callback3(*func, substitute->v, Val_int(var), this->v));
      return retval;
  }

  //detensor
  
  //getLChild
  //getRChild
  
  //isolate
  //substFree
  
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
  
  // ---------------------------------------------------------------------
  //   RegExpT (tensored) methods:
    
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

  void clearEvalCaches() {
      STORE_CALLBACK_IN_VAR("IRE_clearEvalCaches", func);
      caml_callback(*func, Val_unit);
  }

}

