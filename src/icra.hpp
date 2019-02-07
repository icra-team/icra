#ifndef CPROVER_NEWTON_COMPARE_HPP
#define CPROVER_NEWTON_COMPARE_HPP

#include "wali/domains/nparel/NPARel.hpp"
using namespace wali::domains::nparel;
typedef nparel_t relation_t;
typedef nparelpair_t relationpair_t;

#include "wali/domains/duet/DuetRel.hpp"
using namespace wali::domains::duetrel;
typedef DuetRel Relation; // Used for calling static methods

extern bool inNewtonLoop;
extern bool doWideningThisRound;

void set_vertices_wfa(wali::Key entry, wali::Key exit, int _entry_node, int _exit_node);

#endif
