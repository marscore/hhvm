/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_JIT_INLINING_H_
#define incl_HPHP_JIT_INLINING_H_

#include "hphp/runtime/vm/jit/region-selection.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Func;
struct SrcKey;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct ProfDataSerializer;
struct ProfDataDeserializer;

struct RegionDesc;
namespace irgen { struct IRGS; }

///////////////////////////////////////////////////////////////////////////////

/*
 * Inlining decision-making mechanism.
 *
 * The intended client of the inlining decider is an arbitrary tracelet selector
 * that wishes to support inlining.  Correct usage of the inlining decider
 * produces a guarantee (or a refusal to guarantee) that a callee's region can
 * be correctly transcluded and translated within the caller's region.
 *
 * The usage is as follows:
 *
 *    - The client region selector calls canInlineAt() on an instruction.
 *
 *    - If the check succeeds, the client produces a region for the callee at
 *      the appropriate entrypoint for the call.  How this region is selected
 *      is a policy decision left up to the client---however, the region must
 *      not itself contain inlined blocks.
 *
 *    - The client passes this region to shouldInline() to determine whether it
 *      is inlinable.  If so, the client can mark the region as such and/or
 *      perform inlining translations as appropriate.
 */

/////////////////////////////////////////////////////////////////////////////
// Core API.

/*
 * Can we perform inlining of `callee' at `callSK' from within `region'?
 *
 * This is a shallow check---it asks whether `callSK' is an FCall with an
 * appropriate FPush* in the same region, and verifies that the call does not
 * block inlining (e.g., due to missing arguments, recursion, resumable
 * callee, etc.).  It does not peek into the callee's bytecode or regions,
 * and it is insensitive to inlining costs.
 *
 * This function assumes that region is fully-formed up to and including
 * `inst', because we reach backwards through the region to find the
 * corresponding FPush and validate the FPI region.
 *
 * NOTE: Inlining will fail during translation if the FPush was interpreted.
 * It is up to the client to ensure that this is not the case.
 */
bool canInlineAt(SrcKey callSK, const Func* callee, Annotations* annotations);

/*
 * Check that `region' of `callee' can be inlined (possibly via other inlined
 * callees) into the current function.
 *
 * If this function returns true, we guarantee (contingent on inlining decider
 * being used correctly) that it is safe and possible to inline the callee;
 * moreover, based on global inlining heuristics, we submit that the tracelet
 * selector /ought/ to do so.
 */
bool shouldInline(const irgen::IRGS& irgs, SrcKey callerSk, Op callerFPushOp,
                  const Func* callee, const RegionDesc& region,
                  uint32_t maxTotalCost, Annotations& annotations);

/*
 * Return the cost of inlining the given callee.
 */
int costOfInlining(SrcKey callerSk,
                   Op callerFPushOp,
                   const Func* callee,
                   const RegionDesc& region,
                   const irgen::IRGS& irgs,
                   Annotations& annotations);

/*
 * Select an inlining region for the call to `callee' at `sk'.
 */
RegionDescPtr selectCalleeRegion(const SrcKey& sk,
                                 const Func* callee,
                                 const irgen::IRGS& irgs,
                                 int32_t maxBCInstrs,
                                 Annotations& annotations);

void setBaseInliningProfCount(uint64_t value);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
