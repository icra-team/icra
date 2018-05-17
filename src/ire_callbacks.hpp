#ifndef CPROVER_IRECALLBACKS_HPP
#define CPROVER_IRECALLBACKS_HPP

extern "C" {
#include <caml/custom.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/memory.h>
}
#include <string.h>
#include "icra.hpp"

typedef struct _weightWrapper {
    relation_t weight;
} weightWrapper;

void finalize_ww(value v);

CAMLprim value mkWeightWrapper(relation_t rel);
relation_t getRelationFromValueWrapper(value wrapper);

// --------------------------------------------------------------------------
typedef std::map<int,relation_t> IRE_Assignment;
namespace IRE {
    typedef std::map<int /* starKey */, relation_t> StarMap;
}
extern IRE::StarMap IREnewStarBodies;
extern IRE::StarMap IREoldStarBodies;
extern IRE_Assignment IREglobalAssignment;
// --------------------------------------------------------------------------

extern "C" {
  CAMLprim value IRE_evalKleeneSemElemT(value weight, value starKey);
  CAMLprim value IRE_evalKleeneSemElem(value weight, value starKey);
  CAMLprim value IRE_evalPlusSemElem(value weight1, value weight2);
  CAMLprim value IRE_evalPlusSemElemT(value weight1, value weight2);
  CAMLprim value IRE_evalDotSemElem(value weight1, value weight2);
  CAMLprim value IRE_evalDotSemElemT(value weight1, value weight2);
  CAMLprim value IRE_detensor(value weight);
  CAMLprim value IRE_evalTensor(value weight1, value weight2);
  CAMLprim value IRE_evalProjectSemElem(value weight);
  CAMLprim value IRE_evalProjectSemElemT(value weight);
  CAMLprim value IRE_evalVarSemElem(value number);
  CAMLprim value IRE_evalVarSemElemT(value number);
  CAMLprim value IRE_getZeroWt();
  CAMLprim value IRE_getOneWt();
  CAMLprim value IRE_getZeroTWt();
  CAMLprim value IRE_getOneTWt();
  CAMLprim void IRE_printWrappedWeight(value weight, value indentation);
}

#endif
