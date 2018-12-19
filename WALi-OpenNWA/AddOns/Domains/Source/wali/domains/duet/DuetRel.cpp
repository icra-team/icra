
#include "DuetRel.hpp"

#include <iostream>
#include <sstream>
#include <cmath>

#include <boost/algorithm/string/predicate.hpp>

#include <tr1/unordered_map>

extern "C" {
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/alloc.h>
}

using namespace wali::domains::duetrel;
using std::endl;
using wali::waliErr;
using std::cout;

namespace wali
{
  namespace domains
  {
    namespace duetrel
    {

      //duetrelpair_t DuetRelPair::MkDuetRelPair(duetrel_t v1, duetrel_t v2) {
      //    duetrelpair_t d = new DuetRelPair(v1, v2);
      //    return d;
      //}

      static duetrel_t convert(wali::SemElem* se)
      {
          duetrel_t dr = dynamic_cast<DuetRel *>(se);
          if (dr == NULL){
              *waliErr<<"[ERROR] Cannot cast to class duetrel::DuetRel.\n";
              se->print (*waliErr << "    ") << endl;
              assert(false);
          }
          return dr;
      }

      //typedef std::pair< value, bool> StarCacheKey;
      //struct StarCacheHash
      //{
      //    size_t operator() (StarCacheKey k) const
      //    {
      //        return k.first << 1 & (int) k.second;
      //    }
      //};
      //std::tr1::unordered_map< StarCacheKey, sem_elem_t, StarCacheHash> star_cache;

    }
  }
}

// ////////////////////////////
// Static
void DuetRel::reset()
{
}

// ////////////////////////////
// Friends
//namespace wali{
//  namespace domains{
//    namespace duetrel{
//      duetrel_t operator*(duetrel_t a, duetrel_t b)
//      {
//        return a->Compose(b);    // FIXME: Compose badly named: Compose should be Extend
//      }
//
//      duetrel_t operator|(duetrel_t a, duetrel_t b)
//      {
//        return a->Union(b);
//      }
//
//      duetrel_t operator&(duetrel_t a, duetrel_t b)
//      {
//        return a->Intersect(b);
//      }
//    }
//  }
//}

int DuetRel::wCnt = 0;
bool DuetRel::simplify = false;
bool DuetRel::simplifyOnPrint = true;
bool DuetRel::printOnAlphaHatStar = true;

value DuetRel::caml_weights[MAX_WEIGHT_COUNT];
// ////////////////////////////

// ////////////////////////////
// Members and Con/Destructors
DuetRel::DuetRel(const DuetRel& that):
  //wali::SemElemTensor(that),
  NPARel(that),
  relId(that.relId),
  isTensored(that.isTensored)
{
}

DuetRel::DuetRel(int b, bool it):
  relId(b),
  isTensored(it)
{
}
DuetRel::~DuetRel() {}
// ////////////////////////////

duetrel_t DuetRel::MkDuetRel(value w, bool tensored){
    CAMLparam1(w);
    static bool hasBeenInvoked = false;
    if (!hasBeenInvoked){
      hasBeenInvoked = true;
    }
    if (wCnt >= MAX_WEIGHT_COUNT)
    {
      assert(0 && "Too many DuetRel weights were used (increase MAX_WEIGHT_COUNT).");
    }
    caml_register_global_root(&caml_weights[wCnt]);
    caml_weights[wCnt] = w;
    duetrel_t d = new DuetRel(wCnt, tensored);
    wCnt++;
    CAMLreturnT(duetrel_t,d);
}


value DuetRel::getValue() const
{
    CAMLparam0();
    CAMLlocal1(retVal);
    retVal = caml_weights[relId];
    CAMLreturn(retVal);
}

duetrel_t DuetRel::Compose( duetrel_t that ) const    // FIXME: Compose badly named: Compose should be Extend
{
    CAMLparam0();
    CAMLlocal3(dval0, dval1, retval);
    dval0 = this->getValue();
    dval1 = that->getValue();
    duetrel_t d;

    if (!isTensored){  
        value * com_func = caml_named_value("compose_callback");    // FIXME: Compose badly named: Compose should be Extend
        retval = caml_callback2(*com_func, dval0, dval1);
        
        if (simplify) {
            value * simp_func = caml_named_value("simplify_callback");
            retval = caml_callback(*simp_func, retval);
        }

        d = MkDuetRel(retval);
    }
    else
    {
        value * com_func = caml_named_value("tensorCompose_callback");    // FIXME: Compose badly named: Compose should be Extend
        retval = caml_callback2(*com_func, dval0, dval1);
       
        if (simplify) {
            value * simp_func = caml_named_value("tensorSimplify_callback");
            retval = caml_callback(*simp_func, retval);
        }

        d = MkDuetRel(retval, true);
    }
    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::Union( duetrel_t that ) const
{
    CAMLparam0();
    CAMLlocal3(dval0, dval1, retval);

    dval0 = this->getValue();
    dval1 = that->getValue();
    duetrel_t d;  


    if (!isTensored){
        value * union_func = caml_named_value("union_callback");
        retval = caml_callback2(*union_func, dval0, dval1);
     
        if (simplify) {
            value * simp_func = caml_named_value("simplify_callback");
            retval = caml_callback(*simp_func, retval);
        }

        d = MkDuetRel(retval);
    }
    else
    {
        value * union_func = caml_named_value("tensorUnion_callback");
        retval = caml_callback2(*union_func, dval0, dval1);

        if (simplify) {
            value * simp_func = caml_named_value("tensorSimplify_callback");
            retval = caml_callback(*simp_func, retval);
        }
 
        d = MkDuetRel(retval, true);
    }
    CAMLreturnT(duetrel_t,d);
}

//duetrel_t DuetRel::Intersect( duetrel_t that ) const
//{
//  CAMLparam0();
//  std::cout << "INTERSECT!" << std::endl;
//  duetrel_t d = MkDuetRel(that->getValue());
//  CAMLreturnT(duetrel_t, d);
//}

bool DuetRel::Equal( duetrel_t that) const
{
    // Note: This equivalence check is different from Equivalent;
    //   * Equal is for weights
    //   * Equivalent is for iteration domain elements
    
    CAMLparam0();
    CAMLlocal3(ret_d, dval0, dval1);

    dval0 = this->getValue();
    dval1 = that->getValue();
    //We skip this test if you insist
#ifndef BINREL_HASTY
    if(isTensored != that->isTensored){
        that->print(print(*waliErr) << endl) << endl;
        assert(false);
        return false;
    }
#endif
    value * eq_func = caml_named_value("eq_callback");
    ret_d = caml_callback2(*eq_func, dval0, dval1);

    CAMLreturnT(bool,Bool_val(ret_d));
}


bool DuetRel::IsSat() const
{
    CAMLparam0();
    CAMLlocal2(ret_d, dval);

    dval = this->getValue();
    
    value * is_sat_func = caml_named_value("is_sat_callback");
    ret_d = caml_callback(*is_sat_func, dval);
    CAMLreturnT(bool, Bool_val(ret_d));
}

duetrel_t DuetRel::Transpose() const
{
    CAMLparam0();
    CAMLlocal2(dval, retval);

    dval = this->getValue();
    
    value * transpose_func = caml_named_value("transpose_callback");
    retval = caml_callback(*transpose_func, dval);

    duetrel_t d = MkDuetRel(retval);
    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::Kronecker(duetrel_t that) const
{
    CAMLparam0();
    CAMLlocal3(dval0, dval1, retval);

    dval0 = this->getValue();
    dval1 = that->getValue();
    
    value * tensor_func = caml_named_value("tensor_callback");
    retval = caml_callback2(*tensor_func, dval0, dval1);

    duetrel_t d = MkDuetRel(retval, true);
    CAMLreturnT(duetrel_t,d);
}

nparel_t DuetRel::Merge(int v, int c) const
{
    CAMLparam0();
    CAMLlocal2(dval, retval);

    dval = this->getValue();

    value * merge_func;
    if (isTensored) {
        merge_func = caml_named_value("tensorMerge_callback");
    } else {
        merge_func = caml_named_value("merge_callback");
    }
    retval = caml_callback(*merge_func, dval);

    duetrel_t d = MkDuetRel(retval, isTensored);
    CAMLreturnT(duetrel_t,d);
}

//duetrel_t DuetRel::Normalize() const
//{
//    CAMLparam0();
//    CAMLlocal2(dval, retval);
//    
//    dval = this->getValue();
//
//    value * normalize_func = caml_named_value("normalize_callback");
//    retval = caml_callback(*normalize_func, dval);
//
//    duetrel_t d = MkDuetRel(retval);
//    CAMLreturnT(duetrel_t, d);
//}

//duetrel_t DuetRel::TensorMerge(int v, int c) const
//{
//    CAMLparam0();
//    CAMLlocal2(dval, retval);
//
//    dval = this->getValue();
//    value * tensorMerge_func = caml_named_value("tensorMerge_callback");
//    retval = caml_callback(*tensorMerge_func, dval);
//
//    duetrel_t d = MkDuetRel(retval, true);
//    CAMLreturnT(duetrel_t, d);
//}

// ////////////////////////////
// SemElem Interface functions

wali::sem_elem_t DuetRel::star()
{
    CAMLparam0();
    CAMLlocal2(dval, retVal);

    //std::cout << "star() called on the DuetRel @ " << this << std::endl;

    dval = this->getValue();
    duetrel_t d;
    
    if(!isTensored) {
        value * star_func = caml_named_value("star_callback");
        retVal = caml_callback(*star_func, dval);

        d = MkDuetRel(retVal);
    } else {
        value * star_func = caml_named_value("tensorStar_callback");
        retVal = caml_callback(*star_func, dval);

        d = MkDuetRel(retVal, true);
    }
    CAMLreturnT(duetrel_t,d);
}

// duetrelpair_t DuetRel::alphaHatStar()
// {
//   CAMLparam0();
//   CAMLlocal3(dval, abstract, star_formula);
// 
//   dval = this->getValue();
//   duetrel_t d1, d2;
//   duetrelpair_t d;
// 
//   value * print_transition_func;
//   value * print_abstract_func;
//   value * alpha_hat_func;
//   value * abstract_star_func;
// 
//   if (isTensored) {
//     alpha_hat_func = caml_named_value("tensor_alpha_hat_callback");
//     abstract_star_func = caml_named_value("tensor_abstract_star_callback");
//     print_transition_func = caml_named_value("tensor_print_robust_callback");
//     print_abstract_func = caml_named_value("tensor_print_abstract_callback");
//   } else {
//     alpha_hat_func = caml_named_value("alpha_hat_callback");
//     abstract_star_func = caml_named_value("abstract_star_callback");
//     print_transition_func = caml_named_value("print_robust_callback");
//     print_abstract_func = caml_named_value("print_abstract_callback");
//   }
// 
//   abstract = caml_callback(*alpha_hat_func, dval);
//   star_formula = caml_callback(*abstract_star_func, abstract);
// 
//   d1 = MkDuetRel(abstract, isTensored);
//   d2 = MkDuetRel(star_formula, isTensored);
//   d = DuetRelPair::MkDuetRelPair(d1, d2);
// 
//   std::cout << "alphaHatStar {" << std::endl;
//   std::cout << "**** alpha hat: ";
//   std::cout << String_val(caml_callback2(*print_abstract_func, Val_int(2), abstract)) << std::endl;
//   std::cout << "**** star transition: ";
//   std::cout << String_val(caml_callback2(*print_transition_func, Val_int(2), star_formula)) << std::endl;
//   std::cout << "}" << std::endl;
// 
//   CAMLreturnT(duetrelpair_t,d);
// }

// Note: this function will perform widening with previousAbstractValue.
//
// If you don't want widening, call this with previousAbstractValue = 0, 
//   which is the default parameter value.
nparelpair_t DuetRel::alphaHatStar(nparel_t previousAbstractValue)
{
    CAMLparam0();
    CAMLlocal4(dval, abstract, star_formula, widened); 
    
    dval = this->getValue();
    duetrel_t d1, d2;
    nparelpair_t d;

    value * print_transition_func;
    value * print_abstract_func;
    value * alpha_hat_func;
    value * abstract_star_func;
    value * widen_func;

    if (isTensored) {
        alpha_hat_func = caml_named_value("tensor_alpha_hat_callback");
        abstract_star_func = caml_named_value("tensor_abstract_star_callback");
        print_transition_func = caml_named_value("tensor_print_robust_callback");
        print_abstract_func = caml_named_value("tensor_print_abstract_callback");
        widen_func = caml_named_value("tensor_abstract_widen_callback");
    } else {
        alpha_hat_func = caml_named_value("alpha_hat_callback");
        abstract_star_func = caml_named_value("abstract_star_callback");
        print_transition_func = caml_named_value("print_robust_callback");
        print_abstract_func = caml_named_value("print_abstract_callback");
        widen_func = caml_named_value("abstract_widen_callback");
    }

    abstract = caml_callback(*alpha_hat_func, dval);

    if (printOnAlphaHatStar) {
        if (previousAbstractValue == 0) {
            std::cout << "(Not performing widening.)" << std::endl;
        } else {
            std::cout << "(Performing widening.)" << std::endl;
        }

        std::cout << "alphaHatStar {" << std::endl;
        std::cout << "**** body value: ";
        print(std::cout);
        
        std::cout << std::endl << "**** alpha hat: ";
        std::cout << String_val(caml_callback2(*print_abstract_func, Val_int(2), abstract)) << std::endl;
    }

    if (previousAbstractValue != 0) {
        duetrel_t previousDuetRel( convert(previousAbstractValue.get_ptr()) );
        widened = caml_callback2(*widen_func, previousDuetRel->getValue(), abstract);
        star_formula = caml_callback(*abstract_star_func, widened);
    } else {
        widened = abstract; // FIXME is this right?
        star_formula = caml_callback(*abstract_star_func, abstract);
    }

    // If we perform widening, we should return the widened value instead of the un-widened alpha-hat;
    //   if we are not widening, then we can return the un-widened alpha-hat.
    d1 = MkDuetRel(widened, isTensored); 
    //d1 = MkDuetRel(abstract, isTensored);
    d2 = MkDuetRel(star_formula, isTensored);
    d = NPARelPair::MkNPARelPair(d1, d2);

    if (printOnAlphaHatStar) {
        if (previousAbstractValue != 0) {
            std::cout << "**** result of widening: ";
            std::cout << String_val(caml_callback2(*print_abstract_func, Val_int(2), widened)) << std::endl;
        }
        std::cout << "**** star transition: ";
        std::cout << String_val(caml_callback2(*print_transition_func, Val_int(2), star_formula)) << std::endl;
        std::cout << "}" << std::endl;
    }

    CAMLreturnT(nparelpair_t,d);
}

bool DuetRel::Equivalent(nparel_t that) const
{
    // Note: This equivalence check is different from Equal;
    //   * Equal is for weights
    //   * Equivalent is for iteration domain elements
    
    // We assume that a quantifier elimination procedure
    //   has been called on both arguments of the equivalence check.

    // I copied this call to CAMLparam0 from Equiv; I'm not sure
    //   if we should use CAMLparam0 or CAMLparam1 here.
    CAMLparam0();
    CAMLlocal3(retVal, dval0, dval1);

    duetrel_t d_that( convert( that.get_ptr() ) );
    dval0 = this->getValue();
    dval1 = d_that->getValue();
    
    if (isTensored) {
        value * eq_func = caml_named_value("tensor_abstract_equiv_callback");
        retVal = caml_callback2(*eq_func, dval0, dval1);
    } else {
        value * eq_func = caml_named_value("abstract_equiv_callback");
        retVal = caml_callback2(*eq_func, dval0, dval1);
    }

    CAMLreturnT(bool,Bool_val(retVal));
}

wali::sem_elem_t DuetRel::combine(wali::SemElem* se) 
{ 
    duetrel_t that( convert(se) );
    return Union(that);
}

//
// extend
//
// Return the extend of x (this) and y.
// Considering x and y as functions, x extend y = y o x,
// where (g o f)(v) = g(f(v)).
//
wali::sem_elem_t DuetRel::extend(wali::SemElem* se) 
{
    duetrel_t that( convert(se) );
    return Compose(that);    // FIXME: Compose badly named: Compose should be Extend
}

bool DuetRel::equal(wali::SemElem* se) const 
{
    duetrel_t that( convert(se) );
    //return Equal(that);
    bool ans = Equal(that);
    //assert(
    //   ans
    //   ? this->hash() == se->hash()
    //   : true);
    return ans;
}

bool DuetRel::containerLessThan(wali::SemElem const * se) const
{
    DuetRel const * other = dynamic_cast<DuetRel const *>(se);
    return this->getValue() < other->getValue();
}


wali::sem_elem_t DuetRel::one() const
{
    CAMLparam0();
    CAMLlocal1(oneV);

    value * one_func = caml_named_value("one_callback");
    oneV = caml_callback(*one_func, Val_unit);

    duetrel_t d = MkDuetRel(oneV);

    CAMLreturnT(duetrel_t,d);
}

wali::sem_elem_t DuetRel::zero() const
{
    CAMLparam0();
    CAMLlocal1(zeroV);
    duetrel_t d;

    value * zero_func = caml_named_value("zero_callback");
    zeroV = caml_callback(*zero_func, Val_unit);

    d = MkDuetRel(zeroV);

    CAMLreturnT(duetrel_t,d);
}

wali::sem_elem_t DuetRel::tensorZero() const
{
    CAMLparam0();
    CAMLlocal1(tzeroV);
    duetrel_t d;
 
    value * zero_func = caml_named_value("tensorZero_callback");
    tzeroV = caml_callback(*zero_func, Val_unit);

    d = MkDuetRel(tzeroV, true);

    CAMLreturnT(duetrel_t,d);
}

wali::sem_elem_t DuetRel::tensorOne() const
{
    CAMLparam0();
    CAMLlocal1(toneV);
    duetrel_t d;
 
    value * one_func = caml_named_value("tensorOne_callback");
    toneV = caml_callback(*one_func, Val_unit);

    d = MkDuetRel(toneV, true);

    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::getBaseOne()
{
    CAMLparam0();
    CAMLlocal1(oneV);
    duetrel_t d;
  
    value * one_func = caml_named_value("one_callback");
    oneV = caml_callback(*one_func, Val_unit);

    d = MkDuetRel(oneV);

    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::getBaseZero()
{
    CAMLparam0();
    CAMLlocal1(zeroV);
    duetrel_t d;

    value * zero_func = caml_named_value("zero_callback");
    zeroV = caml_callback(*zero_func, Val_unit);

    d = MkDuetRel(zeroV);

    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::getBaseTop()
{
    CAMLparam0();
    CAMLlocal1(topV);
    duetrel_t d;
    
    value * top_func = caml_named_value("top_callback");
    assert(top_func != 0 /* check that top_callback exists */);
    topV = caml_callback(*top_func, Val_unit);

    d = MkDuetRel(topV);

    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::getTensorZero()
{
    CAMLparam0();
    CAMLlocal1(tzeroV);
    duetrel_t d;

    value * zero_func = caml_named_value("tensorZero_callback");
    tzeroV = caml_callback(*zero_func, Val_unit);

    d = MkDuetRel(tzeroV, true);

    CAMLreturnT(duetrel_t,d);
}

duetrel_t DuetRel::getTensorOne()
{
    CAMLparam0();
    CAMLlocal1(toneV);
    duetrel_t d;

    value * one_func = caml_named_value("tensorOne_callback");
    toneV = caml_callback(*one_func, Val_unit);

    d = MkDuetRel(toneV, true);

    CAMLreturnT(duetrel_t,d);
}

// Print a value from the abstract domain of recurrences.
// FIXME: obviously these abstract values should either be made into a separate
//   class, or else there should be a flag in a DuetRel indicating whether it 
//   is one of these, instead of a usual DuetRel.  But, I'm in a hurry.
std::ostream& DuetRel::printAbstract( std::ostream& o ) const 
{
    CAMLparam0();
    CAMLlocal1(dval); 
    
    dval = this->getValue();

    value * print_abstract_func;

    if (isTensored) {
        print_abstract_func = caml_named_value("tensor_print_abstract_callback");
    } else {
        print_abstract_func = caml_named_value("print_abstract_callback");
    }

    o << String_val(caml_callback2(*print_abstract_func, Val_int(2), dval)) << std::endl;

    CAMLreturnT(std::ostream&, o);
}

std::ostream& DuetRel::printSmtlib( std::ostream& o ) const 
{
  
    CAMLparam0();
    CAMLlocal1(dval); 
    
    dval = this->getValue();

    value * print_smtlib_func;

    if (isTensored) {
        print_smtlib_func = caml_named_value("tensor_print_smtlib");
    } else {
        print_smtlib_func = caml_named_value("print_smtlib");
    }

    o << String_val(caml_callback(*print_smtlib_func, dval)) << std::endl;

    CAMLreturnT(std::ostream&, o);
}

std::ostream& DuetRel::print( std::ostream& o ) const 
{
    return printIndented(o, 0);
}

std::ostream& DuetRel::printIndented( std::ostream& o, unsigned int indent ) const 
{
    CAMLparam0();
    CAMLlocal3(dval,sval,simpval);

    if(!isTensored)
        o << "Base relation: ";
    else
        o << "Tensored relation: ";

    dval = this->getValue();
    value * simplify_func;
    value * print_func;

    if(!isTensored) {
        simplify_func = caml_named_value("simplify_callback");
        print_func = caml_named_value("print_robust_callback");
    } else {
        simplify_func = caml_named_value("tensorSimplify_callback");
        print_func = caml_named_value("tensor_print_robust_callback");
    }

    if (simplifyOnPrint) {
        simpval = caml_callback(*simplify_func, dval);
        sval = caml_callback2(*print_func, Val_int(indent), simpval);
    } else {
        sval = caml_callback2(*print_func, Val_int(indent), dval);
    }

    o << String_val(sval);
    CAMLreturnT(std::ostream&, o);
}

wali::sem_elem_tensor_t DuetRel::transpose() 
{
    return Transpose();
}

wali::sem_elem_tensor_t DuetRel::tensor(wali::SemElemTensor* se)
{
    duetrel_t that( convert(se) );
    return Kronecker(that);
}

wali::sem_elem_tensor_t DuetRel::detensor()
{
    assert(false); //This function, detensor, should never be called
#ifdef BINREL_STATS
    con->numDetensor++;
#endif
    return NULL;
}

wali::sem_elem_tensor_t DuetRel::detensorTranspose()
{
    CAMLparam0();
    CAMLlocal2(dval, retval);

    dval = this->getValue();
    value * detensorTranspose_func = caml_named_value("detensorTranspose_callback");
    retval = caml_callback(*detensorTranspose_func, dval);

    duetrel_t d = MkDuetRel(retval);

    CAMLreturnT(duetrel_t,d);
}

std::ostream& DuetRel::printHull( std::ostream& o, unsigned int indent, int var, const char* procedure ) const
{
    CAMLparam0();
    CAMLlocal3(sval,dval,procedure_name);

    dval = this->getValue();
    assert(!isTensored);
    value * print_hull = caml_named_value("print_var_bounds_callback");
    procedure_name = caml_copy_string(procedure);
    value args[4] = { Val_int(indent), Val_int(var), dval, procedure_name };

    sval = caml_callbackN(*print_hull, 4, args);

    o << String_val(sval);
    CAMLreturnT(std::ostream&, o);
}

// Yo, Emacs!
// Local Variables:
//   c-file-style: "ellemtel"
//   c-basic-offset: 2
// End:
