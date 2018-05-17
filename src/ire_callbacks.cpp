#include <iostream>
#include "ire_callbacks.hpp"

// --------------------------------------------------------------------------
IRE_Assignment IREglobalAssignment;
IRE::StarMap IREnewStarBodies;
IRE::StarMap IREoldStarBodies;
bool doWideningThisRound;
bool inNewtonLoop;
// --------------------------------------------------------------------------

void finalize_ww(value v) {
    weightWrapper * wrapper = (weightWrapper *) Data_custom_val(v);
    //if (wrapper->weight) { delete wrapper->weight; } // for pointer-based method
    
    if (wrapper->weight!=0) { wrapper->weight=0; } // for ref_ptr-based method
}

int compare_ww(value v1, value v2) {
    if (Data_custom_val(v1) < Data_custom_val(v2)) return -1;
    if (Data_custom_val(v1) == Data_custom_val(v2)) return 0;
    return 1;
}

intnat hash_ww(value v) {
    return ((long)(Data_custom_val(v)) % 999999999);
}

extern "C" {
  static struct custom_operations ww_custom_ops = {
      identifier: "weightWrapper handling",
      finalize:    finalize_ww, //custom_finalize_default,
      compare:     compare_ww, //custom_compare_default,
      hash:        hash_ww, //custom_hash_default,
      serialize:   custom_serialize_default,
      deserialize: custom_deserialize_default,
      compare_ext: custom_compare_ext_default
  };
}

CAMLprim value mkWeightWrapper(relation_t rel) {
    CAMLparam0();
    CAMLlocal1(retval);
    retval = caml_alloc_custom(&ww_custom_ops, sizeof(weightWrapper), 0, 1);

    // Pointer-based method (doesn't work because we can't construct a DuetRel 
    //   in the way we would need to.)

    ////// Copy the relation into a new object that is owned by this weight wrapper
    ////weightWrapper * wrapper = static_cast<weightWrapper *>(Data_custom_val(retval));
    ////Relation * ptr = dynamic_cast<Relation *>(wrapper->weight);
    ////if (!ptr) { assert("0 && Bad pointer in mkWeightWrapper"); }
    ////wrapper->weight = DuetRel::mkDuetRel(...);

    // ref_ptr-based method (ugly)
    //   I don't know a better way to handle this.  If you do, then please fix it!
    //   The following is a strange use of "placement new".
    new (&(((weightWrapper *)Data_custom_val(retval))->weight)) relation_t(rel);

    CAMLreturn(retval);
}

relation_t getRelationFromValueWrapper(value wrapper) {
    relation_t retval = (((weightWrapper *)Data_custom_val(wrapper))->weight);
    return retval;
}

relation_t mkRelation_tFromSemElem(wali::sem_elem_t se) {
    relation_t retval = dynamic_cast<Relation *>(se.get_ptr());
    if (retval == 0) { assert(0 && "Bad cast in mkWeightWrapperFormSemElem"); }
    return retval;
}

value mkWeightWrapperFromSemElem(wali::sem_elem_t se) {
    return mkWeightWrapper(mkRelation_tFromSemElem(se));
}

  // --------------------------------------------------------------------------

extern "C" {

  CAMLprim value IRE_evalKleeneSemElemT(value weight, value starKey) {  // : weight -> int -> weight
      CAMLparam2(weight, starKey);
      CAMLlocal1(retval);
      
      relation_t childRelation = getRelationFromValueWrapper(weight);
      int key = Int_val(starKey);

      if (inNewtonLoop) {
          relation_t previousValue = 0;
          if (doWideningThisRound) {
              IRE::StarMap::iterator previousValueIterator = IREoldStarBodies.find(key);
              if (previousValueIterator != IREoldStarBodies.end()) {
                  previousValue = mkRelation_tFromSemElem(previousValueIterator->second);
              }
          }
          relationpair_t pair = childRelation->alphaHatStar(previousValue);
          IREnewStarBodies[key] = pair->first;
          retval = mkWeightWrapper(pair->second.get_ptr());
      } else {
          relationpair_t pair = childRelation->alphaHatStar();
          retval = mkWeightWrapper(pair->second.get_ptr());
      }

      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalKleeneSemElem(value weight, value starKey) {  // : weight -> int -> weight
      CAMLparam2(weight, starKey);
      CAMLlocal1(retval);
      
      relation_t childRelation = getRelationFromValueWrapper(weight);
      int key = Int_val(starKey);

      if (inNewtonLoop) {
          relation_t previousValue = 0;
          if (doWideningThisRound) {
              IRE::StarMap::iterator previousValueIterator = IREoldStarBodies.find(key);
              if (previousValueIterator != IREoldStarBodies.end()) {
                  previousValue = mkRelation_tFromSemElem(previousValueIterator->second);
              }
          }
          relationpair_t pair = childRelation->alphaHatStar(previousValue);
          IREnewStarBodies[key] = pair->first;
          retval = mkWeightWrapper(pair->second.get_ptr());
      } else {
          relationpair_t pair = childRelation->alphaHatStar();
          retval = mkWeightWrapper(pair->second.get_ptr());
      }

      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalPlusSemElem(value weight1, value weight2) {  // : weight -> weight -> weight
      CAMLparam2(weight1, weight2);
      CAMLlocal1(retval);
      relation_t childRelation1 = getRelationFromValueWrapper(weight1);
      relation_t childRelation2 = getRelationFromValueWrapper(weight2);
      retval = mkWeightWrapperFromSemElem(childRelation1->combine(childRelation2.get_ptr()));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalPlusSemElemT(value weight1, value weight2) {  // : weight -> weight -> weight
      CAMLparam2(weight1, weight2);
      CAMLlocal1(retval);
      relation_t childRelation1 = getRelationFromValueWrapper(weight1);
      relation_t childRelation2 = getRelationFromValueWrapper(weight2);
      retval = mkWeightWrapperFromSemElem(childRelation1->combine(childRelation2.get_ptr()));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalDotSemElem(value weight1, value weight2) {  // : weight -> weight -> weight
      CAMLparam2(weight1, weight2);
      CAMLlocal1(retval);
      relation_t childRelation1 = getRelationFromValueWrapper(weight1);
      relation_t childRelation2 = getRelationFromValueWrapper(weight2);
      retval = mkWeightWrapperFromSemElem(childRelation1->extend(childRelation2.get_ptr()));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalDotSemElemT(value weight1, value weight2) {  // : weight -> weight -> weight
      CAMLparam2(weight1, weight2);
      CAMLlocal1(retval);
      relation_t childRelation1 = getRelationFromValueWrapper(weight1);
      relation_t childRelation2 = getRelationFromValueWrapper(weight2);
      retval = mkWeightWrapperFromSemElem(childRelation1->extend(childRelation2.get_ptr()));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_detensor(value weight) {  // : weight -> weight
      CAMLparam1(weight);
      CAMLlocal1(retval);
      relation_t childRelation = getRelationFromValueWrapper(weight);
      retval = mkWeightWrapperFromSemElem(childRelation->detensorTranspose());
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalTensor(value weight1, value weight2) {  // : weight -> weight -> weight
      CAMLparam2(weight1, weight2);
      CAMLlocal1(retval);
      relation_t childRelation1 = getRelationFromValueWrapper(weight1);
      relation_t childRelation2 = getRelationFromValueWrapper(weight2);
      wali::sem_elem_tensor_t child1Transposed = childRelation1->transpose(); // this may be a no-op
      retval = mkWeightWrapperFromSemElem(child1Transposed->tensor(childRelation2.get_ptr()));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalProjectSemElem(value weight) {  // : weight -> weight
      CAMLparam1(weight);
      CAMLlocal1(retval);
      relation_t childRelation = getRelationFromValueWrapper(weight);
      retval = mkWeightWrapper(childRelation->Merge(0,0));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalProjectSemElemT(value weight) {  // : weight -> weight
      CAMLparam1(weight);
      CAMLlocal1(retval);
      relation_t childRelation = getRelationFromValueWrapper(weight);
      retval = mkWeightWrapper(childRelation->Merge(0,0));
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalVarSemElem(value number) {  // : int -> weight
      CAMLparam1(number);
      CAMLlocal1(retval);
      if (IREglobalAssignment.find(Int_val(number)) == IREglobalAssignment.end()) {
          // Return zero-weight for varibles not in the map
          retval = mkWeightWrapper(Relation::getBaseZero());
      } else {
          retval = mkWeightWrapper(IREglobalAssignment[Int_val(number)]);
      }
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_evalVarSemElemT(value number) {  // : int -> weight
      CAMLparam1(number);
      CAMLlocal1(retval);
      assert(false && "Attempted to evaluate a tensored variable. (IRE version)");
      retval = mkWeightWrapper(Relation::getTensorZero());
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_getZeroWt() {  // :  unit  -> weight
      CAMLparam0();
      CAMLlocal1(retval);
      retval = mkWeightWrapper(Relation::getBaseZero());
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_getOneWt() {  // :  unit  -> weight
      CAMLparam0();
      CAMLlocal1(retval);
      retval = mkWeightWrapper(Relation::getBaseOne());
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_getZeroTWt() {  // :  unit -> weight
      CAMLparam0();
      CAMLlocal1(retval);
      retval = mkWeightWrapper(Relation::getTensorZero());
      CAMLreturn(retval);
  }
  
  CAMLprim value IRE_getOneTWt() {  // :  unit  -> weight
      CAMLparam0();
      CAMLlocal1(retval);
      retval = mkWeightWrapper(Relation::getTensorOne());
      CAMLreturn(retval);
  }

  CAMLprim void IRE_printWrappedWeight(value weight, value indentation) { // weight -> int -> unit
      CAMLparam2(weight, indentation);
      relation_t nparel = getRelationFromValueWrapper(weight);
      duetrel_t duetrel = dynamic_cast<Relation *>(nparel.get_ptr());
      if (duetrel == 0) { assert("0 && Bad pointer in IRE_printWrappedWeight"); }
      duetrel->printIndented(std::cout, 2*Int_val(indentation));
#undef flush
      std::cout << std::flush; // required to achieve well-behaved cross-language printing
      CAMLreturn0;
  }

}
