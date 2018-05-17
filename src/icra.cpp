// ::wali::wpds::fwpds
#include "wali/wpds/fwpds/FWPDS.hpp"
// ::wali::wpds::ewpds
#include "wali/wpds/ewpds/EWPDS.hpp"
#include "wali/graph/RegExp.hpp"
// ::wali::wpds
#include "wali/wpds/WPDS.hpp"
// ::wali::wpds::nwpds
#include "wali/wpds/nwpds/NWPDS.hpp"
#if defined(USE_AKASH_EWPDS) || defined(USING_AKASH_FWPDS)
#include "wali/wpds/ewpds/ERule.hpp"
#endif
// ::wali::wfa
#include "wali/wfa/WFA.hpp"
#include "wali/wfa/Trans.hpp"
#include "wali/wfa/TransFunctor.hpp"
#include "wali/wfa/State.hpp"
// ::wali::wpds
#include "wali/wpds/RuleFunctor.hpp"
#include "wali/wpds/Rule.hpp"
#include "wali/wpds/GenKeySource.hpp"
// ::std
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <stack>
#include <getopt.h>
// ::wali
#include "wali/KeySpace.hpp"
#include "wali/Key.hpp"
#include "wali/ref_ptr.hpp"
#include "wali/IntSource.hpp"
#include "wali/KeyPairSource.hpp"
// ::wali::util
#include "wali/util/Timer.hpp"
// ::wali::cprover
#include <boost/unordered_map.hpp>

#include "icra.hpp"

#include "NewtonOcamlInterface.hpp"
extern "C" {
#include <caml/mlvalues.h>
#include <caml/callback.h>
}

#include "IRE.hpp"

#define USE_IRE

using namespace std;
using namespace wali;
using namespace wali::wfa;
using namespace wali::wpds;
using namespace wali::wpds::ewpds;
using namespace wali::wpds::fwpds;
using namespace wali::wpds::nwpds;

using namespace wali::domains::duetrel;

typedef wali::ref_ptr<graph::RegExp> reg_exp_t;
typedef std::map<int,IRE::regExpRefPtr> IRERegExpMap;

#include <boost/cast.hpp>

#define NEWTON_FROM_BELOW   1
#define NEWTON_FROM_ABOVE   3

// ----------------------------------------------------------------------------
//   The incoming program is encoded in the following variables,
//     whose types are defined in NewtonOcamlInterface.hpp, and 
//     which are set in NewtonOcamlInterface.cpp:

std::vector<caml_rule> ruleHolder;
std::vector<caml_call_rule> callRuleHolder;
std::vector<caml_epsilon_rule> epsilonRuleHolder;
std::vector<caml_error_rule> errorRuleHolder;
std::vector<caml_print_hull_rule> printHullRuleHolder;
wali::Key entry_key;
wali::Key exit_key;
//relation_t compareWeight; // No longer used

void push_rule(caml_rule rule) {
    ruleHolder.push_back(rule);
}
void push_call_rule(caml_call_rule rule) {
    callRuleHolder.push_back(rule);
}
void push_epsilon_rule(caml_epsilon_rule rule) {
    epsilonRuleHolder.push_back(rule);
}
void push_error_rule(caml_error_rule rule) {
    errorRuleHolder.push_back(rule);
}
void push_print_hull_rule(caml_print_hull_rule rule) {
    printHullRuleHolder.push_back(rule);
}
void set_compare_weight(relation_t compare) { // no longer used
    //compareWeight = compare;
}
void set_vertices_wfa(wali::Key entry, wali::Key exit) {
    entry_key = entry;
    exit_key = exit;
}

// ----------------------------------------------------------------------------

/*
 *  Functions to generate stack and state names
 */
static wali::Key st1()
{
  return getKey("Unique State Name");
}

static wali::Key stk(int k)
{
  return getKey(k);
}

static wali::Key getPdsState() 
{
  return st1();
}

// ----------------------------------------------------------------------------


std::map<reg_exp_t, IRE::regExpRefPtr> IREregExpConversionMap;

// ----------------------------------------------------------------------------
//   Global flag variables and similar:

// newtonVerbosity levels
#define NV_ZERO 0
#define NV_SUMMARIES 1
#define NV_STANDARD_WARNINGS 2
#define NV_NEWTON_LOOP 3
#define NV_NEWTON_STEPS 4
#define NV_ALPHA_HAT_STAR 5
#define NV_PDS 6
#define NV_GAUSS 7
#define NV_EVERYTHING 8

int newtonVerbosity = NV_EVERYTHING;

namespace wali {
  namespace wfa {
    extern bool automaticallyPrintOutput;
  }
}

int  gaussJordanMode; // 0 means NPA-TP (use "--npa-tp"); 1 means NPA-TP-GJ.
int  aboveBelowMode;
bool doSmtlibOutput;
std::vector<std::string> boundingVarAll;   // names of variables for which to print bounds for all procedures...
std::vector<std::string> boundingVarEntry; // ... and for the main procedure only

short dump = false;

//this is used for the testing suite
bool testMode = false;
string testFileName;

int maxRnds = -1;  //maximum number of rounds of newton loop

// ----------------------------------------------------------------------------

relation_t mkRelationFromSemElem(wali::sem_elem_t s) {
    relation_t r (dynamic_cast<Relation*>(s.get_ptr()));
    if (r == 0) {
        assert(0 && "Failed to cast a sem_elem_wrapper to a Relation.");
    }
    relation_t d(r); // This is unnecessary, right?
    return d;
}

#ifdef USE_IRE
/*
 *  convertToICRARegExp
 *
 *  Convert the WALi regular expression exp to an IRE regular expression
 *
 *  @param: reID - the varID of the regular expressiosn
 *          exp - the expression to be converted
 *          varDependencies - a map from reID to the set of variabels it depends on that will be populated as we convert, used for the newton round later
 *          updateableMap - a map from updateable node to the intergraph node number it depends on (that node number is still associated with the inter graph)
 *          oMap - a map from the intergraph node number to the correct unique number for the tsl regexps
 *
 *   Author: Jason Breck
 */
IRE::regExpRefPtr convertToICRARegExp(reg_exp_t exp, 
                                      std::map<int, int> & updateableMap, 
                                      std::map<int, int> & oMap, 
                                      bool insertProjects = true)
{
    // Check the function cache for this function, and if we get a hit, then return the cached value
    if (IREregExpConversionMap.count(exp) > 0) {
        return IREregExpConversionMap[exp];
    }

    IRE::regExpRefPtr retval;

    if (exp->isConstant()) {
        if (exp->isOne()) { retval = IRE::RegExp::mkOne(); }
        else if (exp->isZero()) { retval = IRE::RegExp::mkZero(); }
        else { 
            relation_t w = mkRelationFromSemElem(exp->get_weight());
            retval = IRE::RegExp::mkWeight(w); 
        }
    } else if (exp->isUpdatable()) { // This means a call; i.e., a variable
        int node_no = exp->updatableNumber();
        int mNum = oMap[updateableMap[node_no]];
        retval = insertProjects
                 ? IRE::RegExp::mkVar(mNum)->mkProject()
                 : IRE::RegExp::mkVar(mNum);
    } else if (exp->isStar()) {
        IRE::regExpRefPtr childRE = convertToICRARegExp(exp->getChildren().front(), updateableMap, oMap, insertProjects);
        retval = childRE->mkKleene();
    } else if (exp->isExtend()) {
        list<reg_exp_t>::iterator ch = exp->children.begin();
        reg_exp_t leftChild = *ch;
        IRE::regExpRefPtr leftChildRE = convertToICRARegExp(leftChild, updateableMap, oMap, insertProjects);
        ch++;
        reg_exp_t rightChild = *ch;
        IRE::regExpRefPtr rightChildRE = convertToICRARegExp(rightChild, updateableMap, oMap, insertProjects);
        retval = leftChildRE->mkDot(rightChildRE);
    } else if (exp->isCombine()) {
        list<reg_exp_t>::iterator ch = exp->children.begin();
        reg_exp_t leftChild = *ch;
        IRE::regExpRefPtr leftChildRE = convertToICRARegExp(leftChild, updateableMap, oMap, insertProjects);
        ch++;
        reg_exp_t rightChild = *ch;
        IRE::regExpRefPtr rightChildRE = convertToICRARegExp(rightChild, updateableMap, oMap, insertProjects);
        retval = leftChildRE->mkPlus(rightChildRE);
    } else {
        assert(0 && "Unrecognized regular expression input to convertToICRARegExp");
    }

    IREregExpConversionMap[exp] = retval; // Put retval into this function's cache
    return retval;
}
#endif


#define WIDENING_DELAY 6
// Default values:
#define MAX_ROUNDS_FROM_BELOW 50
#define MAX_ROUNDS_FROM_ABOVE 4

#ifdef USE_IRE
void newtonLoop_GJ_IRE(IRE_Assignment & IREnewAssignment, 
                       IRERegExpMap & IREregExpsAfterIsolation)
{
  int rnd = 0;
  IRERegExpMap::iterator varIt;
  
  IREnewStarBodies.clear();
  IRE::clearEvalCaches();

  if (maxRnds < 0) {      //maxRnds is not set yet
      if (aboveBelowMode == NEWTON_FROM_BELOW)
          maxRnds = MAX_ROUNDS_FROM_BELOW;
      else if (aboveBelowMode == NEWTON_FROM_ABOVE)
          maxRnds = MAX_ROUNDS_FROM_ABOVE;
  }
  
  // In Newton-from-below mode, we perform "Newton" rounds until convergence
  //   or until MAX_ROUNDS_FROM_BELOW; in Newton-from-above mode, we always 
  //   go through MAX_ROUNDS_FROM_ABOVE rounds.
  while (true){
NEWROUND:
      if (rnd >= maxRnds) {
          if (newtonVerbosity >= NV_NEWTON_LOOP) {
              std::cout << "Maximum number of rounds reached. ------------------------------------------" << std::endl;
          }

          if (aboveBelowMode == NEWTON_FROM_BELOW) {
              assert(false && "In Newton-from-below mode, we must abort if we reach the maximum number of rounds.");
          }

          break;
      }

      if (newtonVerbosity >= NV_NEWTON_LOOP) {
          std::cout << "-------------------------------------------------------------------------------" << std::endl;
          std::cout << "Round " << rnd << ":" << std::endl;
      }
      rnd++;
          
      IRE::clearEvalCaches();
          
      IREglobalAssignment = IREnewAssignment;
      IREnewAssignment.clear();
          
      IREoldStarBodies = IREnewStarBodies;
      IREnewStarBodies.clear();

      inNewtonLoop = true;
      if (rnd >= WIDENING_DELAY) {
          if (newtonVerbosity >= NV_NEWTON_LOOP) {
              std::cout << "Widening will be applied on this round." << std::endl;
          }
          doWideningThisRound = true;
      } else {
          doWideningThisRound = false;
      }

      // For each variable in the equation system, evaluate its regular expression
      for (varIt = IREregExpsAfterIsolation.begin(); varIt != IREregExpsAfterIsolation.end(); varIt++)
      {
          int var = varIt->first;
          
          if (newtonVerbosity >= NV_NEWTON_LOOP) {
              std::cout << "Evaluating variable number " << varIt->first << " (using IRE) " << std::endl;
          }

          relation_t IREnewValue = IREregExpsAfterIsolation[var]->eval();

          if (newtonVerbosity >= NV_NEWTON_LOOP) {
              std::cout << "\n  The IRE-evaluated value on this round is: \n\n";
              IREnewValue->print(std::cout); 
              std::cout << std::endl << std::endl;
          }
          
          IREnewAssignment[var] = IREnewValue;
      }
      
      if (aboveBelowMode == NEWTON_FROM_ABOVE) 
          goto NEWROUND;

      // Here is the main loop termination condition
      if (newtonVerbosity >= NV_NEWTON_LOOP) {
          std::cout << "    (IRE-tc) Checking termination condition." << std::endl;
      }
      for(IRE::StarMap::const_iterator starBody_it = IREnewStarBodies.begin(); starBody_it != IREnewStarBodies.end(); ++starBody_it) 
      {
          IRE::StarMap::const_iterator oldStarBody = IREoldStarBodies.find(starBody_it->first);
          if (oldStarBody == IREoldStarBodies.end()) {
              if (newtonVerbosity >= NV_NEWTON_LOOP) {
                  std::cout << "    (IRE-tc)   >> First round; nothing to compare against; continuing loop." << std::endl;
              }
              goto NEWROUND;
          } else {
              if (!(starBody_it->second->Equivalent(oldStarBody->second))) {
                  if (newtonVerbosity >= NV_NEWTON_LOOP) {
                      std::cout << "    (IRE-tc)   >> Inequivalent star body; continuing loop." << std::endl;
                  }
                  goto NEWROUND;
              } else {
                  if (newtonVerbosity >= NV_NEWTON_LOOP) {
                      std::cout << "    (IRE-tc)   >> Equivalent star body." << std::endl;
                  }
              }
          }
      }
      if (newtonVerbosity >= NV_NEWTON_LOOP) {
          std::cout << "    (IRE-tc)     >> All star bodies equivalent." << std::endl;
      }

      break;
  }
      
  if (newtonVerbosity >= NV_NEWTON_LOOP) {
      std::cout << std::endl << "NumRnds: " << rnd << std::endl;
  }

  if (testMode) {
      std::fstream testFile(testFileName.c_str(), std::fstream::out | std::fstream::app);
      testFile << "__NUMRNDS " << rnd << std::endl;
      testFile.close();
  }
  
  inNewtonLoop = false;
}
#endif


#ifdef USE_IRE
double doNewtonSteps_GJ_IRE(WFA& outfa, wali::Key entry_key, FWPDS * originalPds)
{ 
  IREglobalAssignment.clear();
  IRERegExpMap IREregExpMap;  //The map of all IRE regular expressions
  IRE_Assignment IREaList;

  if (newtonVerbosity >= NV_NEWTON_STEPS) {
      cout << "#################################################################" << endl;
      cout << "           Beginning Interprocedural Anaylsis (with regexp=IRE)  " << endl;
      cout << "Step 1: =========================================================" << endl;
  }

  //Step 1 - Convert the program 'pg' into an fpds
  FWPDS * fpds;
  if(originalPds != NULL)
    fpds = new FWPDS(*originalPds);
  else{
    assert(0 && "doNewtonSteps should not be called with originalPds == NULL");
  }

  int dummy = -1;
  map<int, reg_exp_t> outNodeRegExpMap; // A map from a unique id of an outgoing node to the regular expression associated with it
  map<int, int> updateableMap;  //A map from an updateable node number to the id of the node it depends on 
  map<int, int> oMap;
  map<int, pair< pair<int, int>, int> > mapBack;  //A map from the node index to the struct of the <<src,tgt>,stack> - used to insert weights back into wfa
  map<pair<pair<int, int>, int>, int> transMap;
  vector<int> variableIDs; //A list of nodes with where the differential is to be taken

  map<std::pair<int, int>, std::pair<int, int> > mergeSrcMap; //The map that keeps track of the src of calls on call instructions
  std::vector<int> wl;
  std::set<int> vl;

  wali::set_verify_fwpds(false);
  fpds->useNewton(false);

  WFA fa;
  wali::Key acc = wali::getKeySpace()->getKey("accept");
  sem_elem_t temp = fpds->get_theZero()->one();
  fa.addTrans(getPdsState(),entry_key, acc, fpds->get_theZero()->one());
  fa.setInitialState(getPdsState());
  fa.addFinalState(acc);

  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "Step 2: =========================================================" << endl;
  /* Step 2 - Perform poststar on the fpds get the regular expressions associated with
  *           the outgoing nodes in the intra_graph associated with the fwpds
  */
  // This function performs poststar on fpds w.r.t. fa, and populates the maps as described above
  // The Boolean indicates whether this is the first time the function is called and will generate unique ids as needed
  
  double t1 = fpds->getOutRegExps(fa, outfa, outNodeRegExpMap, updateableMap, oMap, mapBack, transMap, variableIDs, mergeSrcMap);

  std::set<Key> faStates = outfa.getStates();
  std::set<Key>::iterator stateIter;
  int numTrans = 0;

  //Get the set of transitions on the remaining states and put that into the worklist
  for (stateIter = faStates.begin(); stateIter != faStates.end(); stateIter++)
  {
      TransSet & transSet = outfa.getState(*stateIter)->getTransSet();
      TransSet::iterator transIt;
      for (transIt = transSet.begin(); transIt != transSet.end(); transIt++)
      {
          ITrans * tr = *transIt;
          int tSrc = tr->from();
          int tTgt = tr->to();
          int tStack = tr->stack();
          int transReg = transMap[std::make_pair(std::make_pair(tSrc, tTgt), tStack)];
          wl.push_back(transReg);
          vl.insert(transReg);
          numTrans++;
      }
  }

  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "Step 3: =========================================================" << endl;
  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "        Converting to IRE regular expressions" << endl;
  while (!wl.empty())
  {
      int wlSzie = wl.size();
      int rToConvert = wl.back();
      wl.pop_back();
      bool insertProjects = false;
      IREregExpMap[rToConvert] = convertToICRARegExp(outNodeRegExpMap[rToConvert], updateableMap, oMap, insertProjects);
  }

  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "Step 4: =========================================================" << endl;
  // Perform Gaussian elimination

  if (newtonVerbosity >= NV_GAUSS) {
      std::cout << "The procedure (i.e., variable) numbers are: " << std::endl;
      for (vector<int>::iterator eit = variableIDs.begin(); eit != variableIDs.end(); eit++)
      {
          std::cout << *eit << "  "; 
      }
      std::cout << std::endl << std::endl << std::endl;
      for (vector<int>::iterator eit = variableIDs.begin(); eit != variableIDs.end(); eit++)
      {
          std::cout << "New-style (IRE) regular expression in IREregExpMap for reID=" << *eit << ": " << std::endl;
          IREregExpMap[*eit]->print(); 
          std::cout << std::endl << std::endl;
      }
  }

  std::sort(variableIDs.begin(), variableIDs.end());

  IRERegExpMap IREregExpsBeforeIsolation;
  IRERegExpMap IREregExpsAfterIsolation;

  if (newtonVerbosity >= NV_GAUSS) {
      std::cout << std::endl << "Performing Gaussian Elimination." << std::endl << std::endl;
  }

  for (vector<int>::iterator varIt = variableIDs.begin(); varIt != variableIDs.end(); varIt++)
  {
      int i = *varIt;
      IREregExpsBeforeIsolation[i] = IREregExpMap[i]->mkProject();
  }

  for (vector<int>::iterator varIt = variableIDs.begin(); varIt != variableIDs.end(); varIt++)
  {
      int i = *varIt;

      if (newtonVerbosity >= NV_GAUSS) {
          std::cout << std::endl << "  ------------------------------ " << std::endl;
          std::cout << "Working on variable " << i << std::endl;
          std::cout << "  New-style (IRE) regular expression for " << i << " just before isolating it: " << std::endl << std::endl;
          IREregExpsBeforeIsolation[i]->print(); 
          std::cout << std::endl << std::endl;
      }

      IRE::regExpRefPtr IREiRHS = IREregExpsBeforeIsolation[i]->isolate(i);

      IREregExpsAfterIsolation[i] = IREiRHS;
      for (vector<int>::iterator varIt2 = variableIDs.begin(); varIt2 != variableIDs.end(); varIt2++)
      {
          int j = *varIt2;

          if (j < i) {
              IREregExpsAfterIsolation[j] = IREregExpsAfterIsolation[j]->substFree(i, IREiRHS);
          } else if (j > i) {
              IREregExpsBeforeIsolation[j] = IREregExpsBeforeIsolation[j]->substFree(i, IREiRHS);
          }

      }

  }

  if (newtonVerbosity >= NV_GAUSS) {
      std::cout << std::endl << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  " << std::endl << std::endl;
      std::cout << std::endl << "Finished Gaussian Elimination." << std::endl << std::endl;
      for (vector<int>::iterator eit = variableIDs.begin(); eit != variableIDs.end(); eit++)
      {
          std::cout << "New-style (IRE) regular expression in IREregExpsAfterIsolation for reID=" << *eit << ": " << std::endl;
          IREregExpsAfterIsolation[*eit]->print(); 
          std::cout << std::endl << std::endl;
      }
  }
  
  if (aboveBelowMode == NEWTON_FROM_BELOW) {
      IREaList.clear();
  } else if (aboveBelowMode == NEWTON_FROM_ABOVE) {
      assert(false && "We should not use aboveBelowMode NEWTON_FROM_ABOVE in newtonLoop_GJ_IRE");
  } 

  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "Step 5: =========================================================" << endl;
  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "[Newton] Running Newton" << endl;

  newtonLoop_GJ_IRE(IREaList, IREregExpsAfterIsolation);

  if (newtonVerbosity >= NV_NEWTON_STEPS) cout << "\nStep 6: =========================================================" << endl;

  IREglobalAssignment = IREaList;

  //Map the evaluated weights back to the transitions the regexps came from
  for (stateIter = faStates.begin(); stateIter != faStates.end(); stateIter++)
  {
      TransSet & transSet = outfa.getState(*stateIter)->getTransSet();
      TransSet::iterator transIt;
      for (transIt = transSet.begin(); transIt != transSet.end(); transIt++)
      {
          ITrans * tt = *transIt;
          int tSrc = tt->from();
          int tTgt = tt->to();
          int tStack = tt->stack();
          int transReg = transMap[std::make_pair(std::make_pair(tSrc, tTgt), tStack)];
          tt->setWeight(IREregExpMap[transReg]->eval());
      }
  }

  //Perform the final path summary
  outfa.path_summary_tarjan_fwpds(true, true);

  if (testMode) {
      double tTime = 0.0;
      std::fstream testFile(testFileName.c_str(), std::fstream::out | std::fstream::app);
      testFile << "__TIME " << tTime << std::endl; // remove this dummy print statement eventually
      testFile.close();
  }
  
  return 0;
}
#endif








int getGlobalBoundingVarFromName(const char * variableName) {
    CAMLparam0();
    CAMLlocal2(sval, idval);
    sval = caml_copy_string(variableName);
    value * proc_func = caml_named_value("get_global_var");
    idval = caml_callback(*proc_func, sval);
    CAMLreturnT(int, Int_val(idval));
}

// New code for manipulating procedure summaries

class Procedure : public Countable {
  public:
    relation_t summary;
    std::string name;

    Procedure(relation_t _summary, std::string _name) : 
        Countable(), summary(_summary), name(_name) {}
};
typedef wali::ref_ptr<Procedure> ProcedureRefPtr;

class Program : public Countable {
  public:
    std::map<std::string,ProcedureRefPtr> proceduresByName;
    ProcedureRefPtr main;

    Program() : Countable() {} 

    void addProcedure(ProcedureRefPtr p) { proceduresByName[p->name] = p; }

};
typedef wali::ref_ptr<Program> ProgramRefPtr;

std::string getProcedureNameFromNode(int nodeNumber) {
    CAMLparam0();
    CAMLlocal1(sval);
    value * proc_func = caml_named_value("procedure_of_vertex_callback");
    sval = caml_callback(*proc_func, Val_int(nodeNumber));
    CAMLreturnT(std::string, String_val(sval));
}

// Extract the procedure summaries from outfaNewton and add them to program.
void extractProcedureSummaries(WFA& outfaNewton, ProgramRefPtr program) {

    static int numberOfUnrecognizedProcedures = 1;
    
    // Set exit_transitions to the set of all transitions in outfaNewton
    // of the form (st1,WALI_EPSILON,<st1,e>), where e is an entry node
    wali::wfa::TransSet exit_transitions;
    exit_transitions = outfaNewton.match(st1(), WALI_EPSILON);
    for(wali::wfa::TransSet::iterator tsit = exit_transitions.begin(); tsit != exit_transitions.end(); tsit++) {
        bool foundMain = false;
        std::string name = "";
        
        // First, we want to extract the procedure name; for that, we need to find
        //   the procedure's entry vertex, so we can call printProcedureNameFromNode.
        // After our various automaton manipulations, the following code seems to be
        //   what's required to obtain the procedure's entry vertex key from the
        //   exit transition whose weight is the procedure summary.
        wali::Key toKey = (*tsit)->to();
        wali::ref_ptr<wali::KeySource> ks = wali::getKeySource(toKey);
        wali::ref_ptr<wali::wpds::GenKeySource> gks = dynamic_cast<wali::wpds::GenKeySource *>(ks.get_ptr());
        if (gks != NULL) {
            wali::ref_ptr<wali::KeySource> gksks = wali::getKeySource(gks->getKey());
            wali::ref_ptr<wali::KeyPairSource> kps = dynamic_cast<wali::KeyPairSource *>(gksks.get_ptr());
            if (kps != NULL) {
                wali::ref_ptr<wali::KeySource> kpsks = wali::getKeySource(kps->second());
                wali::ref_ptr<wali::IntSource> is = dynamic_cast<wali::IntSource *>(kpsks.get_ptr());
                if (is != NULL) {
                    int entryVertex = is->getInt();
                    name = getProcedureNameFromNode(entryVertex);
                } else {
                    //if (newtonVerbosity >= NV_SUMMARIES) std::cout << "Warning: a procedure could not be identified by name.  (Case 1)." << std::endl;
                    std::stringstream ss;
                    ss << "unknown" << numberOfUnrecognizedProcedures++;
                    name = ss.str();
                }
            } else {
                //if (newtonVerbosity >= NV_SUMMARIES) std::cout << "Warning: a procedure could not be identified by name.  (Case 2)." << std::endl;
                std::stringstream ss;
                ss << "unknown" << numberOfUnrecognizedProcedures++;
                name = ss.str();
            }
        } else {
            foundMain = true;
            //name = "main";
            name = "procedure of entry";
            // Note: its name may not actually be main, but we have no mechanism to find out the main procedure's actual name
        }

        // Finally, we can extract the procedure summary itself:
        relation_t nval = mkRelationFromSemElem((*tsit)->weight());

        ProcedureRefPtr procedure = new Procedure(nval, name);

        if (foundMain) { program->main = procedure; }

        program->addProcedure(procedure);
    }
}

void writeSmtlibOutput(ProgramRefPtr program) {
    ofstream smtout;
    smtout.open("smtlib_output.smt2");
    for (std::map<std::string,ProcedureRefPtr>::iterator it=program->proceduresByName.begin(); 
         it!=program->proceduresByName.end(); ++it) {

        ProcedureRefPtr procedure = (*it).second;

        smtout << "; Procedure summary for " << procedure->name;
        procedure->summary->printSmtlib(smtout);
        smtout << std::endl;
    }
    smtout.close();
}

void printProcedureSummaries(ProgramRefPtr program) {
    std::cout << "================================================" << std::endl;
    std::cout << "Procedure Summaries" << std::endl << std::endl;
    for (std::map<std::string,ProcedureRefPtr>::iterator it=program->proceduresByName.begin(); 
         it!=program->proceduresByName.end(); ++it) {

        std::cout << "------------------------------------------------" << std::endl;

        ProcedureRefPtr procedure = (*it).second;

        std::cout << "Procedure summary for " << procedure->name << std::endl << std::endl;
        procedure->summary->print(std::cout);
        std::cout << std::endl << std::endl;
    }
}

// Print bounds on program variables as specified by the data structure printHullRuleHolder
//   If we were instructed by the user to print bounds on some global variable
//   for the main procedure, we will also do that.
void printVariableBounds(WFA& outfaNewton, ProgramRefPtr program) {
    std::cout << "================================================" << std::endl;
    std::cout << "Bounds on Variables" << std::endl << std::endl;
    
    // First, handle variables mentioned in print_hull statements
    for (std::vector<caml_print_hull_rule>::iterator it = printHullRuleHolder.begin(); 
         it != printHullRuleHolder.end(); 
         it++)
    {
        wali::Key key = it->first;
        int line = it->second.first;
        int variableID = it->second.second;
        
        wali::wfa::TransSet print_hull_transitions;
        print_hull_transitions = outfaNewton.match(st1(), key);
        for(wali::wfa::TransSet::iterator tsit = print_hull_transitions.begin(); 
            tsit != print_hull_transitions.end(); 
            tsit++)
        {
            relation_t intraprocWeight = mkRelationFromSemElem((*tsit)->weight());  // Weight from containing procedure's entry to print-hull pt
            relation_t contextWeight = mkRelationFromSemElem(outfaNewton.getState((*tsit)->to())->weight());  // Weight of calling context

            //relation_t composedWeight = contextWeight->Compose(intraprocWeight);    // FIXME: Compose badly named: Compose should be Extend
            relation_t composedWeight = mkRelationFromSemElem(contextWeight->extend(intraprocWeight.get_ptr()));    
            
            //std::cout << "Variable Bounds at Line: " << line << std::endl;
            std::cout << "Variable bounds";
            if (line != -1) {
                std::cout << " at line " << line << " in ";
            } else {
                std::cout << " for procedure ";
            }
            wali::ref_ptr<wali::KeySource> ks = wali::getKeySource(key);
            wali::ref_ptr<wali::IntSource> is = dynamic_cast<wali::IntSource *>(ks.get_ptr());
            if (is != NULL) {
                int vertex = is->getInt();
                std::cout << getProcedureNameFromNode(vertex) << std::endl;
            } else {
                std::cout << "unknown procedure" << std::endl;
            }
            intraprocWeight->printHull(std::cout, 0, variableID);
            std::cout << std::endl;
        }
    }

    // Second, handle variables mentioned with the --bound-all command-line parameter
    if (boundingVarAll.size() > 0) {
        for (std::map<std::string,ProcedureRefPtr>::iterator it=program->proceduresByName.begin(); 
             it!=program->proceduresByName.end(); ++it) {
            ProcedureRefPtr procedure = (*it).second;
            for(std::vector<std::string>::iterator strit = boundingVarAll.begin();
                    strit != boundingVarAll.end(); ++strit) {
                std::string variableName = *strit;
                if (procedure == program->main) { continue; } // skip the main procedure
                int variableID = getGlobalBoundingVarFromName(variableName.c_str());
                std::cout << "Variable bounds for procedure '" << procedure->name << "': " << std::endl;
                procedure->summary->printHull(std::cout, 0, variableID);
                std::cout << std::endl;
            }
        }
    }

    // Third, handle variables to be printed in main, whether from the --bound-entry or --bound-all parameters 
    if (program->main != NULL) {
        for(std::vector<std::string>::iterator strit = boundingVarAll.begin();
                strit != boundingVarAll.end(); ++strit) {
            std::string variableName = *strit;
            if (std::find(boundingVarEntry.begin(), boundingVarEntry.end(), variableName) != boundingVarEntry.end()) {
                // The user mentioned this variable in both --bound-entry and --bound-all; let's not print it twice.
                continue;
            }
            int variableID = getGlobalBoundingVarFromName(variableName.c_str());
            //std::cout << "Variable bounds for main procedure: " << std::endl;
            std::cout << "Variable bounds for procedure of entry: " << std::endl;
            program->main->summary->printHull(std::cout, 0, variableID);
            std::cout << std::endl;
        }
        for(std::vector<std::string>::iterator strit = boundingVarEntry.begin();
                strit != boundingVarEntry.end(); ++strit) {
            std::string variableName = *strit;
            int variableID = getGlobalBoundingVarFromName(variableName.c_str());
            //std::cout << "Variable bounds for main procedure: " << std::endl;
            std::cout << "Variable bounds for procedure of entry: " << std::endl;
            program->main->summary->printHull(std::cout, 0, variableID);
            std::cout << std::endl;
        }
    }
    
    //if(dump){
    //    FWPDS * originalPds = new FWPDS();
    //    con = pds_from_prog(originalPds, pg);
    //    cout << "[Newton] Dumping PDS to pds.dot..." << endl;
    //    fstream pds_stream("pds.dot", fstream::out);
    //    RuleDotty rd(pds_stream);
    //    pds_stream << "digraph{" << endl;
    //    originalPds->for_each(rd);
    //    pds_stream << "}" << endl;
    //    delete(originalPds);
    //}

    std::cout << "================================================" << std::endl;
}


void checkAssertions(WFA& outfaNewton) {
    std::cout << "================================================" << std::endl;
    std::cout << "Assertion Checking at Error Points" << std::endl << std::endl;

    // Check the assertion at each error point
    if (testMode) {
        std::fstream testFile(testFileName.c_str(), std::fstream::out | std::fstream::app);
        testFile << "__ASSERTION ";
        testFile.close();
    }

    for (std::vector<caml_error_rule>::iterator it = errorRuleHolder.begin(); it != errorRuleHolder.end(); it++)
    {
        wali::wfa::TransSet error_transitions;
        error_transitions = outfaNewton.match(st1(), it->first);
        for(wali::wfa::TransSet::iterator tsit = error_transitions.begin(); tsit != error_transitions.end(); tsit++)
        {
            std::cout << "Checking assertion at vertex " << it->first
              << ", line " << it->second.second << std::endl
              << std::endl;

            // Check if is_sat ( (it->second) extend (*tsit)->weight() )

            relation_t negatedAssertionWeight = mkRelationFromSemElem(it->second.first);   // Negated assertion condition
            if (newtonVerbosity >= NV_EVERYTHING) {
                negatedAssertionWeight->print(std::cout);
                std::cout << std::endl << std::endl;
            }
            
            relation_t intraprocWeight = mkRelationFromSemElem((*tsit)->weight());  // Weight from containing procedure's entry to assertion pt
            relation_t contextWeight = mkRelationFromSemElem(outfaNewton.getState((*tsit)->to())->weight());  // Weight of calling context

            //relation_t composedWeight = contextWeight->Compose(intraprocWeight);    // FIXME: Compose badly named: Compose should be Extend
            relation_t composedWeight = mkRelationFromSemElem(contextWeight->extend(intraprocWeight.get_ptr()));    
            //relation_t finalWeight = composedWeight->Compose(negatedAssertionWeight);    // FIXME: Compose badly named: Compose should be Extend
            relation_t finalWeight = mkRelationFromSemElem(composedWeight->extend(negatedAssertionWeight.get_ptr()));    

            if (newtonVerbosity >= NV_EVERYTHING) {
                std::cout << std::endl << "contextWeight = " << std::endl;
                contextWeight->print(std::cout);
                std::cout << std::endl << std::endl;

                std::cout << std::endl << "intraproceduralWeight = " << std::endl;
                intraprocWeight->print(std::cout);
                std::cout << std::endl << std::endl;

                std::cout << std::endl << "contextWeight extend intraproceduralWeight = " << std::endl;
                composedWeight->print(std::cout);
                std::cout << std::endl << std::endl;
            }
    
            bool isSat = finalWeight->IsSat();

            if (isSat) {
                std::cout << "Is SAT! (Assertion on line " << it->second.second << " FAILED)" << std::endl ;    
            } else {
                std::cout << "Is not SAT! (Assertion on line " << it->second.second << " PASSED)" << std::endl;
            }

            if (testMode) {
                std::fstream testFile(testFileName.c_str(), std::fstream::out | std::fstream::app);
                testFile << (isSat ? "FAIL " : "PASS ");
                testFile.close();
            }

            std::cout << "---------------------------------------------" << std::endl << std::endl;
        }
    }
}

#undef flush

void print_stats() {
    CAMLparam0();
    cout << "Printing stats" << std::endl;
    value * stats = caml_named_value("print_stats");
    caml_callback(*stats, Val_unit);
    CAMLreturn0;
}

int runBasicNewton(char **args)
{
    caml_startup(args); // This line calls Duet to analyze the program specified on the command
                        //   line and store the analyzed program, represented as PDS rules, into
                        //   the various ruleHolder data structures used in the code below.
    FWPDS * pds = new FWPDS();
    for (std::vector<caml_rule>::iterator it = ruleHolder.begin(); it != ruleHolder.end(); it++) {
        pds->add_rule(st1(), it->first.first, st1(), it->first.second, it->second);
    }
    for (std::vector<caml_call_rule>::iterator itc = callRuleHolder.begin(); itc != callRuleHolder.end(); itc++) {
        pds->add_rule(st1(), itc->first.first, st1(), itc->first.second.first, itc->first.second.second, itc->second);
    }
    for (std::vector<caml_epsilon_rule>::iterator ite = epsilonRuleHolder.begin(); ite != epsilonRuleHolder.end(); ite++) {
        pds->add_rule(st1(), ite->first, st1(), ite->second);
    }
    if (newtonVerbosity >= NV_PDS) { pds->print(std::cout); }
    
    //fstream pds_stream("pds.dot",fstream::out);RuleDotty rd(pds_stream);pds_stream<<"digraph{"<<endl;pds->for_each(rd);pds_stream<<"}"<<endl;

    doWideningThisRound = false; inNewtonLoop = false;
    WFA outfaNewton;
    if (aboveBelowMode == NEWTON_FROM_ABOVE || gaussJordanMode == 0) {
        //doNewtonSteps_NPATP(outfaNewton, entry_key, pds, false);
        assert(0 && "Running modes NEWTON_FROM_ABOVE and NPA_TP are not supported in the current (non-TSL) version of ICRA.");
    } else if (aboveBelowMode == NEWTON_FROM_BELOW) {
        //doNewtonSteps_GJ(outfaNewton, entry_key, pds, false); // HERE XXX
        doNewtonSteps_GJ_IRE(outfaNewton, entry_key, pds);
    } else { assert(false && "Unrecognized running mode."); }

    //std::cout << std::endl << "outfaNewton" << std::endl; outfaNewton.print(std::cout); std::cout << std::endl << std::endl;
    
    ProgramRefPtr program = new Program();
    extractProcedureSummaries(outfaNewton, program);
    if (newtonVerbosity >= NV_SUMMARIES) printProcedureSummaries(program);
    if (doSmtlibOutput) writeSmtlibOutput(program);
    checkAssertions(outfaNewton);
    printVariableBounds(outfaNewton, program);
    
    if (newtonVerbosity >= NV_SUMMARIES) { std::cout << "Finished!" << std::endl << std::flush; }

    return 0;
}   

void printUsageInstr() {
    std::cout << "Newton Duet" << std::endl;
    std::cout << std::endl << "Correct usage:\tNewtonOcaml running_mode [options] input_file [duet options]" << std::endl;
    std::cout << std::endl << "Running modes:" << std::endl;
    std::cout << "\t-cra_newton_basic\t[currently, this mode is the only one working]" << std::endl;
    std::cout << "\t-cra_newton_star" << std::endl;
    std::cout << "\t-cra_newton_above" << std::endl;
    std::cout << std::endl << "Available options:" << std::endl;
    std::cout << "\t-D,\t--dump" << std::endl;
    std::cout << "\t-H,\t--help" << std::endl;
    std::cout << "\t-P,\t--no_simplify_on_print" << std::endl;
    std::cout << "\t-S,\t--simplify" << std::endl;
    std::cout << "\t-R,\t--rounds" << "\n\t\t\tMaximum number of rounds" << std::endl;
}

void printHelp() {
    printUsageInstr();  
    std::cout << "\n\nDuet Options:\n\n";
    char **ocamlArgs = new char *[3];
    ocamlArgs[0] = "";
    ocamlArgs[1] = "-help";
    ocamlArgs[2] = 0;
    caml_startup(ocamlArgs);
}

char * removeDoubleDash(char * str) {
    if (str != NULL && str[0] == '-' && str[1] == '-') {
        if (newtonVerbosity >= NV_STANDARD_WARNINGS) {
            std::cout << "Note: sending command-line argument " << (str+1) << " to duet with single '-'" << std::endl;
        }
        return str+1;
    }
    return str;
}

int main(int argc, char **argv)
{
    gaussJordanMode = 1; // 0 means NPA-TP (use "--npa-tp"); 1 means NPA-TP-GJ.
    aboveBelowMode = 0;
    doSmtlibOutput = false;
    std::vector <char *> unrecognizedArgs;

    // As a temporary measure, allow arguments to be given starting with a plus
    bool warnAboutPlus = true;
    for(int i = 1; i < argc; i++) {
        if (argv[i][0] == '+') {
            argv[i][0] =  '-';
            if (warnAboutPlus) {
                warnAboutPlus = false;
                if (newtonVerbosity >= NV_STANDARD_WARNINGS) {
                    std::cout << "Note: ICRA is translating '+' command-line arguments to '-'" << std::endl;
                }
            }
        }
    }

    static struct option long_options[] = {
        {"cra_newton_basic", no_argument,       &aboveBelowMode,  NEWTON_FROM_BELOW  },
        {"cra_newton_star",  no_argument,       &aboveBelowMode,  2  },
        {"cra_newton_above", no_argument,       &aboveBelowMode,  NEWTON_FROM_ABOVE  },
        {"cra-newton-basic", no_argument,       &aboveBelowMode,  NEWTON_FROM_BELOW  },
        {"cra-newton-above", no_argument,       &aboveBelowMode,  NEWTON_FROM_ABOVE  },
        {"simplify",         no_argument,       0,            'S' },
        {"no_simplify_on_print",no_argument,    0,            'P' },
        {"help",             no_argument,       0,            'H' },
        {"dump",             no_argument,       0,            'D' },
        {"npa-tp",           no_argument,       &gaussJordanMode, 0 },
        {"rounds",           required_argument, 0,            'R' },
        {"newton-verbosity", required_argument, 0,            'N' },
        {"test",             required_argument, 0,            'T' },

        {"domain",           required_argument, 0,            'M' }, // ----------------
        {"verbose",          required_argument, 0,            'V' }, //  Each argument in this group is passed
        {"verbosity",        required_argument, 0,            'I' }, //    to duet, along with an additional
        {"qe",               required_argument, 0,            'Q' }, //    argument.
        {"cra-guard",        required_argument, 0,            'G' },
        {"z3-timeout",       required_argument, 0,            'Z' },
        {"cra-abstraction-timeout",required_argument,0,       'A' },
        {"cra-abstract-limit",required_argument,0,            'L' }, // -----------------

        {"bound-entry",      required_argument, 0,            'B' },
        {"bound-all",        required_argument, 0,            'C' },
        {"smtlib-output",    no_argument,       0,            'U' },
        {"cra-split_loops",  no_argument,       0,            'E' }, // compensating for a typo; remove this later...
        {0,                  0,                 0,             0  }
    };

    std::string variableName;
    opterr = 0;
    int long_index = 0, opt = 0;    
    while ((opt = getopt_long_only(argc, argv, "SPHDR:T:M:V:I:Q:G:", 
                   long_options, &long_index )) != -1) {
        switch (opt) {
            case 0:
                break;
            case 'P':
                Relation::simplifyOnPrint = false;
                break;
            case 'S':
                Relation::simplify = true;
                break;
            case 'D':
                dump = true;
                break;
            case 'H':
                printHelp();
                return 0;
            case 'T':
                testMode = true;
                testFileName = optarg;
                break;
            case 'R':
                maxRnds = atoi(optarg);
                break;
            case 'N':
                newtonVerbosity = atoi(optarg);
                if (newtonVerbosity < NV_PDS) { wali::wfa::automaticallyPrintOutput = false; }
                if (newtonVerbosity < NV_ALPHA_HAT_STAR) { Relation::printOnAlphaHatStar = false; }
                //if (newtonVerbosity < NV_STANDARD_WARNINGS) { opterr = 0; }
                break;
            case 'U':
                doSmtlibOutput = true;  
                break;
            case 'B':
                variableName = optarg;
                boundingVarEntry.push_back(variableName);
                std::cout << "Printing main procedure variable bounds for " << variableName << std::endl;
                break;
            case 'C':
                variableName = optarg;
                boundingVarAll.push_back(variableName);
                std::cout << "Printing variable bounds for " << variableName << " in every procedure" << std::endl;
                break;
            // duet options with an argument
            case 'M':
            case 'V':
            case 'I':
            case 'Q':
            case 'G':
            case 'Z':
            case 'A':
            case 'L':
                if (newtonVerbosity >= NV_STANDARD_WARNINGS) {
                    std::cout << "Passing command-line option " <<  argv[optind - 2] << " " << optarg << " to duet." << std::endl;
                }
                unrecognizedArgs.push_back(removeDoubleDash(argv[optind - 2]));
                unrecognizedArgs.push_back(optarg);
                break;  
            case 'E': // compensating for a typo; remove this option later
                if (newtonVerbosity >= NV_STANDARD_WARNINGS) {
                    std::cout << "Passing command-line option " << "-cra-split-loops" << " to duet." << std::endl;
                }
                unrecognizedArgs.push_back("-cra-split-loops");
                break;  
            // unrecognized option, currently we just pass it to duet
            case '?':                       
                if (newtonVerbosity >= NV_STANDARD_WARNINGS) {
                    std::cout << "Passing command-line option " <<  argv[optind - 1] << " to duet." << std::endl;
                }
                unrecognizedArgs.push_back(removeDoubleDash(argv[optind - 1]));
                break;  
        }   
    }
    if (aboveBelowMode == 0) {
        std::cerr << "Error : Running mode not set!" << std::endl;
        printUsageInstr();
    }
    else if (optind == argc) {
        std::cout << "Error : No input file detected!" << std::endl;
        printUsageInstr();
    }
    
    else {
        //if (aboveBelowMode == NEWTON_FROM_ABOVE) {
        //    assert(false && "Newton-from-above is not supported in this version of NewtonOcaml.");
        //}
        if (aboveBelowMode == NEWTON_FROM_BELOW || aboveBelowMode == NEWTON_FROM_ABOVE) {
            if (testMode) {
                std::fstream testFile(testFileName.c_str(), std::fstream::out | std::fstream::app);
                testFile << "__FILENAME " << argv[optind] << " " << aboveBelowMode << std::endl;
                testFile.close();
            }
            char **ocamlArgs = new char *[3 + unrecognizedArgs.size() + argc - optind];
            ocamlArgs[0] = argv[0];
            ocamlArgs[unrecognizedArgs.size() + 1] = "-cra_newton_basic";
            ocamlArgs[unrecognizedArgs.size() + 2] = argv[optind];

            for (int i = 0; i < unrecognizedArgs.size(); i++) {
                ocamlArgs[1 + i] = unrecognizedArgs[i];

            }
            for (int i = 0; i < argc - optind; i++) {
                ocamlArgs[3 + unrecognizedArgs.size() + i] = argv[optind + i + 1];
            }

            if (aboveBelowMode == NEWTON_FROM_BELOW && gaussJordanMode == 0) {
                std::cout << "**************************************************" << std::endl
                          << "Warning: Running NEWTON_FROM_BELOW in NPA-TP mode." << std::endl
                          << "  This mode has not been extensively tested." << std::endl
                          << "**************************************************" << std::endl;
            }
            runBasicNewton(ocamlArgs);
        }
        else if (aboveBelowMode == 2) {
            std::cout << "Newton from below, with equivalence checks extracted from Kleene stars" << std::endl;
            std::cout << "Not implemented yet." << std::endl;
        }
    }   
    //cout << "Printing stats" << std::endl;
    //print_stats();
}

