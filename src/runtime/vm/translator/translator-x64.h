/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef _TRANSLATOR_X64_H_
#define _TRANSLATOR_X64_H_

#include <signal.h>
#include <boost/noncopyable.hpp>

#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/asm-x64.h>
#include <runtime/vm/translator/srcdb.h>
#include <runtime/vm/translator/unwind-x64.h>
#include <runtime/vm/translator/regalloc.h>
#include <tbb/concurrent_hash_map.h>
#include <util/ringbuffer.h>
#include <runtime/vm/debug/debug.h>
#include <runtime/vm/translator/hopt/hhbctranslator.h>

namespace HPHP {

class ExecutionContext;

namespace VM {
namespace Transl {

class IRTranslator;
using HPHP::x64::register_name_t;

struct TraceletCounters {
  uint64_t m_numEntered, m_numExecuted;
};

struct TraceletCountersVec {
  int64_t m_size;
  TraceletCounters *m_elms;
  Mutex m_lock;

  TraceletCountersVec() : m_size(0), m_elms(NULL), m_lock() { }
};

class TranslatorX64;
extern __thread TranslatorX64* tx64;

extern void* interpOneEntryPoints[];

class TranslatorX64 : public Translator, public SpillFill,
  public boost::noncopyable {
  friend class SrcRec; // so it can smash code.
  friend class SrcDB;  // For write lock and code invalidation.
  friend class ArgManager;
  friend class WithCounters;
  friend class DiamondGuard;
  friend class DiamondReturn;
  friend class RedirectSpillFill;
  friend class Tx64Reaper;
  friend class IRTranslator;
  friend class HPHP::VM::JIT::CodeGenerator;
  friend class HPHP::VM::JIT::HhbcTranslator; // packBitVec()
  template<int, int, int> friend class CondBlock;
  template<int> friend class JccBlock;
  template<int> friend class IfElseBlock;
  template<int> friend class UnlikelyIfBlock;
  typedef HPHP::DataType DataType;

  typedef tbb::concurrent_hash_map<TCA, TCA> SignalStubMap;
  typedef void (*sigaction_t)(int, siginfo_t*, void*);

  typedef HPHP::x64::X64Assembler Asm;
  typedef HPHP::x64::DataBlock DataBlock;
  Asm                    a;
  Asm                    astubs;
  Asm                    atrampolines;
  PointerMap             trampolineMap;
  int                    m_numNativeTrampolines;
  size_t                 m_trampolineSize; // size of each trampoline
  // spillFillCode points to one of a or astubs. We need it to produce
  // reconciliation code to the alternate buffer. Don't directly manipulate;
  // use DiamondGuard instead.
  Asm*                   m_spillFillCode;

  SrcDB                  m_srcDB;
  SignalStubMap          m_segvStubs;
  sigaction_t            m_segvChain;
  TCA                    m_callToExit;
  TCA                    m_retHelper;
  TCA                    m_stackOverflowHelper;
  TCA                    m_dtorGenericStub;
  TCA                    m_dtorGenericStubRegs;
  TCA                    m_dtorStubs[MaxNumDataTypes];
  TCA                    m_interceptHelper;
  TCA                    m_defClsHelper;
  TCA                    m_funcPrologueRedispatch;
  DataBlock              m_globalData;
  size_t                 m_irAUsage;
  size_t                 m_irAstubsUsage;

  // Data structures for HHIR-based translation
  uint64_t               m_numHHIRTrans;
  JIT::IRFactory*        m_irFactory;
  JIT::CSEHash*          m_constTable;
  JIT::TraceBuilder*     m_traceBuilder;
  JIT::HhbcTranslator*   m_hhbcTrans;
  IRTranslator*          m_irTrans;

  void hhirTraceStart(Offset bcStartOffset);
  void hhirTraceCodeGen();
  void hhirTraceEnd(Offset bcSuccOffset);
  void hhirTraceFree();


  struct SavedRegState {
    explicit SavedRegState(void* saver, const RegAlloc& state)
      : saver(saver)
      , savedState(state)
    {}

    void* saver; // For debugging: ensure these are popped in the right order
    RegAlloc savedState;
  };

  RegAlloc                   m_regMap;
  std::stack<SavedRegState>  m_savedRegMaps;
  volatile bool              m_interceptsEnabled;
  FixupMap                   m_fixupMap;
  UnwindRegMap               m_unwindRegMap;
  UnwindInfoHandle           m_unwindRegistrar;

  struct PendingFixup {
    TCA m_tca;
    Fixup m_fixup;
    PendingFixup() { }
    PendingFixup(TCA tca, Fixup fixup) :
      m_tca(tca), m_fixup(fixup) { }
  };
  vector<PendingFixup> m_pendingFixups;
  UnwindRegInfo        m_pendingUnwindRegInfo;

  void drawCFG(std::ofstream& out) const;
  static vector<PhysReg> x64TranslRegs();

  PhysReg getReg(const Location& loc) {
    return m_regMap.getReg(loc);
  }

  Asm &getAsmFor(TCA addr) { return Asm::Choose(a, astubs, addr); }
  void emitIncRef(PhysReg base, DataType);
  void emitIncRefGenericRegSafe(PhysReg base, int disp, PhysReg tmp);
  void emitIncRefGeneric(PhysReg base, int disp = 0);
  void emitDecRef(Asm& a, const NormalizedInstruction& i, PhysReg rDatum,
                  DataType type);
  void emitDecRef(const NormalizedInstruction& i, PhysReg rDatum,
                  DataType type);
  void emitDecRefGeneric(const NormalizedInstruction& i, PhysReg srcReg,
                         int disp = 0);
  void emitDecRefGenericReg(const NormalizedInstruction& i,
                            PhysReg rData, PhysReg rType);
  void emitDecRefInput(Asm& a, const NormalizedInstruction& i, int input);
  void emitCopy(PhysReg srcCell, int disp, PhysReg destCell);
  void emitCopyToStack(Asm& a,
                       const NormalizedInstruction& ni,
                       PhysReg src,
                       int off);
  void emitCopyToStackRegSafe(Asm& a,
                              const NormalizedInstruction& ni,
                              PhysReg src,
                              int off,
                              PhysReg tmpReg);
  void emitTvSetRegSafe(const NormalizedInstruction&, PhysReg from,
    DataType fromType, PhysReg toPtr, int toOffset, PhysReg tmp1, PhysReg tmp2,
    bool incRefFrom);
  void emitTvSet(const NormalizedInstruction&, PhysReg from,
    DataType fromType, PhysReg toPtr, int toOffset = 0, bool incRefFrom = true);

  void emitPushAR(const NormalizedInstruction& i, const Func* func,
                  const int bytesPopped = 0, bool isCtor = false,
                  bool clearThis = true, uintptr_t varEnvInvName = 0);

  void emitCallSaveRegs();
  void prepareCallSaveRegs();
  void emitCallStaticLocHelper(x64::X64Assembler& as,
                               const NormalizedInstruction& i,
                               ScratchReg& output,
                               ptrdiff_t ch);
  void emitCall(Asm& a, TCA dest, bool killRegs=false);
  void emitCallFillCont(Asm& a, const Func* orig, const Func* gen);
  void emitCallUnpack(Asm& a, const NormalizedInstruction& i, int nCopy);
  void emitCallPack(Asm& a, const NormalizedInstruction& i, int nCopy);
  void emitContRaiseCheck(Asm& a, const NormalizedInstruction& i);
  void emitContPreNext(const NormalizedInstruction& i, ScratchReg&  rCont);
  void emitContNextCheck(const NormalizedInstruction& i, ScratchReg& rCont);
  template<bool raise>
  void translateContSendImpl(const NormalizedInstruction& i);
  void translateClassExistsImpl(const Tracelet& t,
                                const NormalizedInstruction& i,
                                Attr typeAttr);
  void recordSyncPoint(Asm& a, Offset pcOff, Offset spOff);
  template <bool reentrant>
  void recordCallImpl(Asm& a, const NormalizedInstruction& i,
                      bool advance = false);
  void recordReentrantCall(Asm& a, const NormalizedInstruction& i) {
    recordCallImpl<true>(a, i);
  }
  void recordReentrantCall(const NormalizedInstruction& i) {
    recordCallImpl<true>(a, i);
  }
  void recordReentrantStubCall(const NormalizedInstruction& i,
                               bool advance = false) {
    recordCallImpl<true>(astubs, i, advance);
  }
  void recordCall(Asm& a, const NormalizedInstruction& i) {
    recordCallImpl<false>(a, i);
  }
  void recordCall(const NormalizedInstruction& i) {
    recordCall(a, i);
  }
  void recordStubCall(const NormalizedInstruction& i) {
    recordCall(astubs, i);
  }
  void emitSideExit(Asm& a, const NormalizedInstruction& dest, bool next);
  void emitStringToClass(const NormalizedInstruction& i);
  void emitStringToKnownClass(const NormalizedInstruction& i,
                              const StringData* clssName);
  void emitObjToClass(const NormalizedInstruction& i);
  void emitClsAndPals(const NormalizedInstruction& i);
  void emitStaticPropInlineLookup(const NormalizedInstruction& i,
                                  const DynLocation& clsInput,
                                  const DynLocation& propInput,
                                  PhysReg scr);

  inline bool isValidCodeAddress(TCA tca) const {
    return a.code.isValidAddress(tca) || astubs.code.isValidAddress(tca) ||
      atrampolines.code.isValidAddress(tca);
  }
  template<int Arity> TCA emitNAryStub(Asm& a, void* fptr);
  TCA emitUnaryStub(Asm& a, void* fptr);
  TCA emitBinaryStub(Asm& a, void* fptr);
  TCA genericRefCountStub(Asm& a);
  TCA genericRefCountStubRegs(Asm& a);
  TCA emitPrologueRedispatch(Asm &a);
  TCA emitFuncGuard(Asm& a, const Func *f);
  template <bool reentrant>
  void callUnaryStubImpl(Asm& a, const NormalizedInstruction& i, TCA stub,
                         PhysReg arg, int disp = 0);
  void callUnaryReentrantStub(Asm& a, const NormalizedInstruction& i, TCA stub,
                              PhysReg arg, int disp = 0) {
    callUnaryStubImpl<true>(a, i, stub, arg, disp);
  }
  void callUnaryStub(Asm& a, const NormalizedInstruction& i, TCA stub,
                     PhysReg arg, int disp = 0) {
    callUnaryStubImpl<false>(a, i, stub, arg, disp);
  }
  void callBinaryStub(Asm& a, const NormalizedInstruction& i, TCA stub,
                      PhysReg arg1, PhysReg arg2);
  void emitDerefStoreToLoc(PhysReg srcReg, const Location& destLoc);

  void binaryIntegerArith(const NormalizedInstruction &i,
                          Opcode op, PhysReg srcReg, PhysReg srcDestReg);
  void binaryArithCell(const NormalizedInstruction &i,
                       Opcode op, const DynLocation& in1,
                       const DynLocation& inout);
  void binaryArithLocal(const NormalizedInstruction &i,
                        Opcode op,
                        const DynLocation& in1,
                        const DynLocation& in2,
                        const DynLocation& out);
  void emitRB(Asm& a, Trace::RingBufferType t, SrcKey sk,
              RegSet toSave = RegSet());
  void emitRB(Asm& a, Trace::RingBufferType t, const char* msgm,
              RegSet toSave = RegSet());

  enum {
    ArgDontAllocate = -1,
    ArgAnyReg = -2
  };
  void allocInputsForCall(const NormalizedInstruction& i,
                          const int* args);

 public:
  struct MInstrState {
    TypedValue tvScratch;
    TypedValue tvLiteral;
    TypedValue tvRef;
    TypedValue tvRef2;
    TypedValue tvResult;
    TypedValue tvVal;
    bool baseStrOff;
    Class* ctx;
  } __attribute__((aligned(16)));
 private:
  int mResultStackOffset(const NormalizedInstruction& ni) const;
  bool generateMVal(const Tracelet& t, const NormalizedInstruction& ni,
                    const MInstrInfo& mii) const;
  bool logicalTeleportMVal(const Tracelet& t, const NormalizedInstruction& ni,
                           const MInstrInfo& mii) const;
  bool teleportMVal(const Tracelet& t, const NormalizedInstruction& ni,
                    const MInstrInfo& mii) const;
  bool useTvResult(const Tracelet& t, const NormalizedInstruction& ni,
                   const MInstrInfo& mii) const;
  bool forceMValIncDec(const NormalizedInstruction& ni, const DynLocation& base,
                       const DynLocation& val) const;
  bool forceMValIncDec(const Tracelet& t, const NormalizedInstruction& ni,
                       const MInstrInfo& mii) const;
  void emitBaseLCR(const Tracelet& t, const NormalizedInstruction& ni,
                   const MInstrInfo& mii, unsigned iInd, PhysReg& rBase);
  void emitBaseN(const Tracelet& t, const NormalizedInstruction& ni,
                 const MInstrInfo& mii, unsigned iInd, PhysReg& rBase);
  void emitBaseG(const Tracelet& t, const NormalizedInstruction& ni,
                 const MInstrInfo& mii, unsigned iInd, PhysReg& rBase);
  void emitBaseS(const Tracelet& t, const NormalizedInstruction& ni,
                 unsigned iInd, bool ctxFixed, PhysReg& rBase);
  void emitBaseOp(const Tracelet& t, const NormalizedInstruction& ni,
                  const MInstrInfo& mii, unsigned iInd, bool ctxFixed,
                  PhysReg& rBase);
  void emitElem(const Tracelet& t, const NormalizedInstruction& ni,
                const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                PhysReg& rBase);
  void emitProp(const Tracelet& t, const NormalizedInstruction& ni,
                const MInstrInfo& mii, bool ctxFixed, unsigned mInd,
                unsigned iInd, PhysReg& rBase);
  void emitNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                   unsigned mInd, PhysReg& rBase);
  void emitIntermediateOp(const Tracelet& t, const NormalizedInstruction& ni,
                          const MInstrInfo& mii, bool ctxFixed, unsigned mInd,
                          unsigned& iInd, PhysReg& rBase);
  bool needFirstRatchet(const Tracelet& t, const NormalizedInstruction& ni,
                        const MInstrInfo& mii) const;
  bool needFinalRatchet(const Tracelet& t, const NormalizedInstruction& ni,
                        const MInstrInfo& mii) const;
  unsigned nLogicalRatchets(const Tracelet& t, const NormalizedInstruction& ni,
                            const MInstrInfo& mii) const;
  int ratchetInd(const Tracelet& t, const NormalizedInstruction& ni,
                 const MInstrInfo& mii, unsigned mInd) const;
  void emitRatchetRefs(const Tracelet& t, const NormalizedInstruction& ni,
                       const MInstrInfo& mii, unsigned mInd,
                       const PhysReg& rBase);
  template <bool useEmpty>
  void emitIssetEmptyElem(const Tracelet& t, const NormalizedInstruction& ni,
                          const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                          const PhysReg& rBase);
  template <bool useEmpty>
  void emitIssetEmptyProp(const Tracelet& t, const NormalizedInstruction& ni,
                          const MInstrInfo& mii, bool ctxFixed, unsigned mInd,
                          unsigned iInd, const PhysReg& rBase);
  void emitVGetNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                       const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                       const PhysReg& rBase);
  void emitSetNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                      const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                      const PhysReg& rBase);
  void emitSetOpNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                        const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                        const PhysReg& rBase);
  void emitIncDecNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                         const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                         const PhysReg& rBase);
  void emitBindNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                       const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                       const PhysReg& rBase);
  void emitNotSuppNewElem(const Tracelet& t, const NormalizedInstruction& ni,
                          const MInstrInfo& mii, unsigned mInd, unsigned iInd,
                          const PhysReg& rBase);
  bool needMInstrCtx(const Tracelet& t, const NormalizedInstruction& ni) const;
  void emitMPre(const Tracelet& t, const NormalizedInstruction& ni,
                const MInstrInfo& mii, bool& ctxFixed, unsigned& mInd,
                unsigned& iInd, PhysReg& rBase);
  void emitMPost(const Tracelet& t, const NormalizedInstruction& ni,
                 const MInstrInfo& mii);
#define MII(instr, attrs, bS, iS, vC, fN) \
  void emit##instr##Elem(const Tracelet& t, const NormalizedInstruction& ni, \
                         const MInstrInfo& mii, unsigned mInd, unsigned iInd, \
                         const PhysReg& rBase); \
  void emit##instr##Prop(const Tracelet& t, const NormalizedInstruction& ni, \
                         const MInstrInfo& mii, bool ctxFixed, unsigned mInd, \
                         unsigned iInd, const PhysReg& rBase); \
  void emitFinal##instr##MOp(const Tracelet& t, \
                             const NormalizedInstruction& ni, \
                             const MInstrInfo& mii, bool ctxFixed, \
                             unsigned mInd, unsigned iInd, \
                             const PhysReg& rBase);
MINSTRS
#undef MII

#define INSTRS \
  CASE(PopC) \
  CASE(PopV) \
  CASE(PopR) \
  CASE(UnboxR) \
  CASE(Null) \
  CASE(True) \
  CASE(False) \
  CASE(Int) \
  CASE(String) \
  CASE(Array) \
  CASE(NewArray) \
  CASE(Nop) \
  CASE(AddElemC) \
  CASE(AddNewElemC) \
  CASE(Cns) \
  CASE(DefCns) \
  CASE(ClsCnsD) \
  CASE(Concat) \
  CASE(Add) \
  CASE(Xor) \
  CASE(Not) \
  CASE(BitNot) \
  CASE(CastInt) \
  CASE(CastString) \
  CASE(Print) \
  CASE(Jmp) \
  CASE(Switch) \
  CASE(RetC) \
  CASE(RetV) \
  CASE(NativeImpl) \
  CASE(AGetC) \
  CASE(AGetL) \
  CASE(CGetL) \
  CASE(CGetL2) \
  CASE(CGetS) \
  CASE(CGetM) \
  CASE(CGetG) \
  CASE(VGetL) \
  CASE(VGetG) \
  CASE(VGetM) \
  CASE(IssetM) \
  CASE(EmptyM) \
  CASE(SetS) \
  CASE(SetG) \
  CASE(SetM) \
  CASE(SetOpL) \
  CASE(SetOpM) \
  CASE(IncDecL) \
  CASE(IncDecM) \
  CASE(UnsetL) \
  CASE(UnsetM) \
  CASE(BindM) \
  CASE(FPushFuncD) \
  CASE(FPushFunc) \
  CASE(FPushClsMethodD) \
  CASE(FPushClsMethodF) \
  CASE(FPushObjMethodD) \
  CASE(FPushCtor) \
  CASE(FPushCtorD) \
  CASE(FPushContFunc) \
  CASE(FPassR) \
  CASE(FPassL) \
  CASE(FPassM) \
  CASE(FPassS) \
  CASE(FPassG) \
  CASE(This) \
  CASE(InitThisLoc) \
  CASE(FCall) \
  CASE(VerifyParamType) \
  CASE(InstanceOfD) \
  CASE(StaticLocInit) \
  CASE(IterInit) \
  CASE(IterValueC) \
  CASE(IterKey) \
  CASE(IterNext) \
  CASE(ReqDoc) \
  CASE(ReqMod) \
  CASE(ReqSrc) \
  CASE(DefCls) \
  CASE(DefFunc) \
  CASE(Self) \
  CASE(Parent) \
  CASE(ClassExists) \
  CASE(InterfaceExists) \
  CASE(TraitExists) \
  CASE(Dup) \
  CASE(CreateCont) \
  CASE(UnpackCont) \
  CASE(PackCont) \
  CASE(ContReceive) \
  CASE(ContRaised) \
  CASE(ContDone) \
  CASE(ContNext) \
  CASE(ContSend) \
  CASE(ContRaise) \
  CASE(ContValid) \
  CASE(ContCurrent) \
  CASE(ContStopped) \
  CASE(ContHandle)

  // These are instruction-like functions which cover more than one
  // opcode.
#define PSEUDOINSTRS \
  CASE(BinaryArithOp) \
  CASE(SameOp) \
  CASE(EqOp) \
  CASE(LtGtOp) \
  CASE(UnaryBooleanOp) \
  CASE(BranchOp) \
  CASE(AssignToLocalOp) \
  CASE(FPushCufOp) \
  CASE(FPassCOp) \
  CASE(CheckTypeOp)

#define PAIR(nm) \
  void analyze ## nm(Tracelet& t, NormalizedInstruction& i); \
  void translate ## nm(const Tracelet& t, const NormalizedInstruction& i);
#define CASE PAIR

INSTRS
PSEUDOINSTRS

#undef CASE
#undef PAIR


  void branchWithFlagsSet(const Tracelet& t, const NormalizedInstruction& i,
                          HPHP::x64::ConditionCode cc);
  void fuseBranchSync(const Tracelet& t, const NormalizedInstruction& i);
  void fuseBranchAfterBool(const Tracelet& t, const NormalizedInstruction& i,
                           HPHP::x64::ConditionCode cc);
  void fuseBranchAfterStaticBool(const Tracelet& t,
                                 const NormalizedInstruction& i,
                                 bool resultIsTrue);
  void emitPropSet(const NormalizedInstruction& i,
                   const DynLocation& base,
                   const DynLocation& rhs,
                   PhysReg rhsReg,
                   PhysReg fieldAddr);
  void translateSetMProp(const Tracelet &t, const NormalizedInstruction& i);
  void translateSetMArray(const Tracelet &t, const NormalizedInstruction& i);
  void emitPropGet(const NormalizedInstruction& i,
                   const DynLocation& base,
                   PhysReg fieldAddr,
                   const Location& outLoc);
  void translateCGetMProp(const Tracelet &t, const NormalizedInstruction& i);
  void translateCGetM_LEE(const Tracelet &t, const NormalizedInstruction& i);
  void translateCGetM_GE(const Tracelet &t, const NormalizedInstruction& i);
  void emitGetGlobal(const NormalizedInstruction& i, int nameIdx,
                     bool allowCreate);
  void emitArrayElem(const NormalizedInstruction& i,
                     const DynLocation* baseInput,
                     PhysReg baseReg,
                     const DynLocation* keyIn,
                     const Location& outLoc);
  void translateIssetMSimple(const Tracelet& t,
                             const NormalizedInstruction& ni);
  void setupActRecClsForStaticCall(const NormalizedInstruction& i,
                                   const Func* func, const Class* cls,
                                   size_t clsOff, bool forward);

  const Func* findCuf(const NormalizedInstruction& ni,
                      Class* &cls, StringData*& invName, bool& forward);
  static void toStringHelper(ObjectData *obj);
  void invalidateSrcKey(const SrcKey& sk);
  bool dontGuardAnyInputs(Opcode op);
 public:
  template<typename T>
  void invalidateSrcKeys(const T& keys) {
    BlockingLeaseHolder writer(s_writeLease);
    ASSERT(writer);
    for (typename T::const_iterator i = keys.begin(); i != keys.end(); ++i) {
      invalidateSrcKey(*i);
    }
  }

  const UnwindRegInfo* getUnwindInfo(CTCA ip) const {
    return m_unwindRegMap.find(ip);
  }

  void enableIntercepts() {m_interceptsEnabled = true;}
  bool interceptsEnabled() {return m_interceptsEnabled;}

  static void SEGVHandler(int signum, siginfo_t *info, void *ctx);

  // SpillFill interface
  void spillTo(DataType t, PhysReg reg, bool writeType,
               PhysReg base, int disp);
  void spill(const Location& loc, DataType t, PhysReg reg,
             bool writeType);
  void fill(const Location& loc, PhysReg reg);
  void fillByMov(PhysReg src, PhysReg dst);
  void loadImm(int64 immVal, PhysReg reg);
  void poison(PhysReg dest);

  // public for syncing gdb state
  Debug::DebugInfo m_debugInfo;

  void fixupWork(VMExecutionContext* ec, ActRec* startRbp) const;
  void fixup(VMExecutionContext* ec) const;

  // helpers for srcDB.
  SrcRec* getSrcRec(const SrcKey& sk) {
    // TODO: add a insert-or-find primitive to THM
    if (SrcRec* r = m_srcDB.find(sk)) return r;
    ASSERT(s_writeLease.amOwner());
    return m_srcDB.insert(sk);
  }

  TCA getTopTranslation(const SrcKey& sk) {
    return getSrcRec(sk)->getTopTranslation();
  }

  TCA getCallToExit() {
    return m_callToExit;
  }

  TCA getRetFromInterpretedFrame() {
    return m_retHelper;
  }

  // If we were to shove every little helper function into this class
  // header, we'd spend the rest of our lives compiling. So, these public
  // functions are for static helpers private to translator-x64.cpp. Be
  // professional.

  Asm& getAsm()   { return a; }
  void emitChainTo(const SrcKey *dest, bool isCall = false);
  void syncOutputs(const Tracelet& t);
  void syncOutputs(const NormalizedInstruction& i);
  void syncOutputs(int stackOff);

  static bool isPseudoEvent(const char* event);
  void getPerfCounters(Array& ret);

private:
  virtual void syncWork();

public:

#define SERVICE_REQUESTS \
  REQ(EXIT)              \
  REQ(BIND_CALL)         \
  REQ(BIND_JMP)          \
  REQ(BIND_ADDR)         \
  REQ(BIND_SIDE_EXIT)    \
  REQ(BIND_JMPCC_FIRST)  \
  REQ(BIND_JMPCC_SECOND) \
  REQ(BIND_REQUIRE)      \
  REQ(RETRANSLATE)       \
  REQ(INTERPRET)         \
  REQ(POST_INTERP_RET)   \
  REQ(STACK_OVERFLOW)    \
  REQ(RETRANSLATE_NO_IR) \
  REQ(RESUME)

  enum ServiceRequest {
#define REQ(nm) REQ_##nm,
    SERVICE_REQUESTS
#undef REQ
  };

  void analyzeInstr(Tracelet& t, NormalizedInstruction& i);
  bool acquireWriteLease(bool blocking) {
    return s_writeLease.acquire(blocking);
  }
  void dropWriteLease() {
    s_writeLease.drop();
  }
  void interceptPrologues(Func* func);

  void emitGuardChecks(Asm& a, const SrcKey&, const ChangeMap&,
    const RefDeps&, SrcRec&);
  void emitOneGuard(const Tracelet& t,
                    const NormalizedInstruction& i,
                    PhysReg reg, int disp, DataType type,
                    TCA &sideExit);
  void irEmitResolvedDeps(const ChangeMap& resolvedDeps);

  void emitVariantGuards(const Tracelet& t, const NormalizedInstruction& i);
  void emitPredictionGuards(const NormalizedInstruction& i);

  Debug::DebugInfo* getDebugInfo() { return &m_debugInfo; }

private:
  TCA getInterceptHelper();
  void translateInstr(const Tracelet& t, const NormalizedInstruction& i);
  void translateInstrWork(const Tracelet& t, const NormalizedInstruction& i);
  void irTranslateInstr(const Tracelet& t, const NormalizedInstruction& i);
  void irTranslateInstrWork(const Tracelet& t, const NormalizedInstruction& i);
  void irTranslateInstrDefault(const Tracelet& t,
                               const NormalizedInstruction& i);
  bool checkTranslationLimit(const SrcKey&, const SrcRec&) const;
  void translateTracelet(const Tracelet& t);
  bool irTranslateTracelet(const Tracelet& t,
                           const TCA       start,
                           const TCA       stubStart);
  void emitStringCheck(Asm& _a, PhysReg base, int offset, PhysReg tmp);
  void emitTypeCheck(Asm& _a, DataType dt,
                     PhysReg base, int offset,
                     PhysReg tmp = InvalidReg);
  void irAssertType(const Location& l, const RuntimeType& rtt);
  void checkType(Asm&, const Location& l, const RuntimeType& rtt,
    SrcRec& fail);
  void irCheckType(Asm&, const Location& l, const RuntimeType& rtt,
                   SrcRec& fail);
  void checkRefs(Asm&, const SrcKey&, const RefDeps&, SrcRec&);

  void emitSmartAddImm(register_name_t rsrcdest, int64_t imm);
  void emitFrameRelease(Asm& a, const NormalizedInstruction& i,
                        bool noThis = false);
  void dumpStack(const char* msg, int offset) const;

  static const size_t kJmpTargetAlign = 16;
  static const int kJmpLen = 5;
  static const int kJmpccLen = 6;
  static const int kJcc8Len = 3;
  static const int kLeaRipLen = 7;
  // Cache alignment is required for mutable instructions to make sure
  // mutations don't "tear" on remote cpus.
  static const size_t kX64CacheLineSize = 64;
  void moveToAlign(Asm &aa, const size_t alignment = kJmpTargetAlign,
                   const bool unreachable = true);
  void prepareForSmash(Asm &a, int nBytes);
  void prepareForSmash(int nBytes);
  static bool isSmashable(Asm &a, int nBytes);
  static void smash(Asm &a, TCA src, TCA dest);

  TCA getTranslation(const SrcKey *sk, bool align);
  TCA retranslate(SrcKey sk, bool align, bool useHHIR);
  TCA retranslateOpt(TransID transId, bool align);
  TCA retranslateAndPatchNoIR(SrcKey sk,
                              bool   align,
                              TCA    toSmash);
  TCA bindJmp(TCA toSmash, SrcKey dest, bool isAddr = false);
  TCA bindJmpccFirst(TCA toSmash,
                     Offset offTrue, Offset offFalse,
                     bool toTake,
                     HPHP::x64::ConditionCode cc);
  TCA bindJmpccSecond(TCA toSmash, const Offset off,
                      HPHP::x64::ConditionCode cc);
  void emitFallbackJmp(SrcRec& dest);
  void emitFallbackJmp(Asm& as, SrcRec& dest);
  void emitFallbackUncondJmp(Asm& as, SrcRec& dest);

  TCA emitServiceReq(bool align, ServiceRequest, int numArgs, ...);
  TCA emitServiceReq(ServiceRequest, int numArgs, ...);
  TCA emitServiceReqVA(bool align, ServiceRequest, int numArgs, va_list args);
  void emitRetC(const NormalizedInstruction& i);
  TCA emitRetFromInterpretedFrame();
  TCA emitGearTrigger(Asm& a, const SrcKey& sk, TransID transId);
  void emitBox(DataType t, PhysReg rToBox);
  void emitUnboxTopOfStack(const NormalizedInstruction& ni);
  void emitBindCall(const Tracelet& t, const NormalizedInstruction &ni,
                    Offset atCall, Offset after);
  void emitCondJmp(const SrcKey &skTrue, const SrcKey &skFalse,
                   HPHP::x64::ConditionCode cc);
  void emitInterpOne(const Tracelet& t, const NormalizedInstruction& i);
  void emitMovRegReg(Asm& a, PhysReg src, PhysReg dest);
  void emitMovRegReg(PhysReg src, PhysReg dest);
  void enterTC(SrcKey sk);

  void recordGdbTranslation(const SrcKey& sk, const Unit* u,
                            const Asm& a,
                            const TCA start,
                            bool exit, bool inPrologue);
  void recordGdbStub(const Asm& a, TCA start, const char* name);
  void recordBCInstr(uint32_t op, const Asm& a, const TCA addr);

  void emitStackCheck(int funcDepth, Offset pc);
  void emitStackCheckDynamic(int numArgs, Offset pc);
  void emitTestSurpriseFlags();
  void emitCheckSurpriseFlagsEnter(bool inTracelet, Offset pcOff,
                                   Offset stackOff);
  TCA  emitTransCounterInc(Asm& a);

  static void trimExtraArgs(ActRec* ar);
  static int  shuffleArgsForMagicCall(ActRec* ar);
  static void setArgInActRec(ActRec* ar, int argNum, uint64_t datum,
                             DataType t);
  TCA funcPrologue(Func* func, int nArgs);
  bool checkCachedPrologue(const Func* func, int param, TCA& plgOut) const;
  SrcKey emitPrologue(Func* func, int nArgs);
  void emitNativeImpl(const Func*, bool emitSavedRIPReturn);
  TCA emitInterceptPrologue(Func* func, TCA next=NULL);
  void emitBindJmp(Asm& a, const SrcKey& dest,
                   ServiceRequest req = REQ_BIND_JMP);
  void emitBindJmp(const SrcKey& dest);
  void emitBindCallHelper(register_name_t stashedAR,
                          SrcKey srcKey,
                          const Func* funcd,
                          int numArgs,
                          bool isImmutable);
  void emitIncCounter(TCA start, int cntOfs);

  void analyzeReqLit(Tracelet& t, NormalizedInstruction& i,
                     InclOpFlags flags);
  void translateReqLit(const Tracelet& t, const NormalizedInstruction& i,
                       InclOpFlags flags);
  struct ReqLitStaticArgs {
    HPHP::Eval::PhpFile* m_efile;
    TCA m_pseudoMain;
    Offset m_pcOff;
    bool m_local;
  };
  static void reqLitHelper(const ReqLitStaticArgs* args);

  TCA getNativeTrampoline(TCA helperAddress);
  TCA emitNativeTrampoline(TCA helperAddress);

  // Utility function shared with IR code
  static uint64_t packBitVec(const vector<bool>& bits, unsigned i);

public:
  void resume(SrcKey sk);

  TCA translate(const SrcKey *sk, bool align, bool useHHIR);

  TranslatorX64();
  virtual ~TranslatorX64();

  void initGdb();
  static TranslatorX64* Get();

  // Called before entering a new PHP "world."
  void requestInit();

  // Called at the end of eval()
  void requestExit();

  // Called when name is bound to a value
  void defineCns(StringData* name);

  // Returns a string with cache usage information
  std::string getUsage();

  // true iff calling thread is sole writer.
  static bool canWrite() {
    // We can get called early in boot, so allow null tx64.
    return !tx64 || s_writeLease.amOwner();
  }

  // Returns true on success
  bool dumpTC();

  // Returns true on success
  bool dumpTCCode(const char* filename);

  // Returns true on success
  bool dumpTCData();

  // Async hook for file modifications.
  bool invalidateFile(Eval::PhpFile* f);
  void invalidateFileWork(Eval::PhpFile* f);

  // Start a new translation space. Returns true IFF this thread created
  // a new space.
  bool replace();

  // Debugging interfaces to prevent tampering with code.
  void protectCode();
  void unprotectCode();

private:
  virtual bool addDbgGuards(const Unit* unit);
  virtual bool addDbgGuard(const Func* func, Offset offset);
  void addDbgGuardImpl(const SrcKey& sk, SrcRec& sr);

private: // Only for HackIR
  void emitReqRetransNoIR(Asm& as, SrcKey& sk);

public: // Only for HackIR
#define DECLARE_FUNC(nm) \
  void irTranslate ## nm(const Tracelet& t,               \
                         const NormalizedInstruction& i);
#define CASE DECLARE_FUNC

INSTRS
PSEUDOINSTRS

#undef CASE
#undef DECLARE_FUNC

  // Helper functions not covered by macros above
  void irTranslateSetMProp(const Tracelet& t, const NormalizedInstruction& i);
  void irTranslateCGetMProp(const Tracelet &t, const NormalizedInstruction& i);
  void irTranslateCGetM_LEE(const Tracelet &t, const NormalizedInstruction& i);
  void irTranslateCGetM_GE(const Tracelet &t, const NormalizedInstruction& i);
  void irTranslateReqLit(const Tracelet& t,
                         const NormalizedInstruction& i,
                         InclOpFlags flags);
  template<bool raise>
  void irTranslateContSendImpl(const NormalizedInstruction& i);


};


/*
 * RAII bookmark for temporarily rewinding a.code.frontier.
 */
class CodeCursor {
  typedef HPHP::x64::X64Assembler Asm;
  Asm& m_a;
  TCA m_oldFrontier;
  public:
  CodeCursor(Asm& a, TCA newFrontier) :
    m_a(a), m_oldFrontier(a.code.frontier) {
      ASSERT(TranslatorX64::canWrite());
      m_a.code.frontier = newFrontier;
      TRACE_MOD(Trace::trans, 1, "RewindTo: %p (from %p)\n",
                m_a.code.frontier, m_oldFrontier);
    }
  ~CodeCursor() {
    ASSERT(TranslatorX64::canWrite());
    m_a.code.frontier = m_oldFrontier;
    TRACE_MOD(Trace::trans, 1, "Restore: %p\n",
              m_a.code.frontier);
  }
};

const size_t kTrampolinesBlockSize = 8 << 12;

// minimum length in bytes of each trampoline code sequence
// Note that if stats is on, then this size is ~24 bytes due to the
// instrumentation code that counts the number of calls through each
// trampoline
const size_t kMinPerTrampolineSize = 11;

const size_t kMaxNumTrampolines = kTrampolinesBlockSize /
  kMinPerTrampolineSize;

void fcallHelperThunk() asm ("__fcallHelperThunk");

// These could be static but are used in hopt/codegen.cpp
void raiseUndefVariable(StringData* nm);
void defFuncHelper(Func *f);
Instance* newInstanceHelper(Class* cls, int numArgs, ActRec* ar,
                            ActRec* prevAr);
Instance* newInstanceHelperCached(Class** classCache,
                                  const StringData* clsName, int numArgs,
                                  ActRec* ar, ActRec* prevAr);

// These could be static but are used in hopt/irtranslator.cpp
SrcKey nextSrcKey(const Tracelet& t, const NormalizedInstruction& i);
bool isNormalPropertyAccess(const NormalizedInstruction& i,
                       int propInput,
                       int objInput);
bool isContextFixed();
Slot getPropertyOffset(const NormalizedInstruction& i,
                  int propInput, int objInput);
bool isSupportedCGetMProp(const NormalizedInstruction& i);
bool isSupportedCGetM_LE(const NormalizedInstruction& i);
bool isSupportedCGetM_LEE(const NormalizedInstruction& i);
bool isSupportedCGetM_RE(const NormalizedInstruction& i);
bool isSupportedCGetM_GE(const NormalizedInstruction& i);
bool isSupportedCGetM(const NormalizedInstruction& i);
bool isSupportedSetMProp(const NormalizedInstruction& i);
TXFlags planInstrAdd_Int(const NormalizedInstruction& i);
TXFlags planInstrAdd_Array(const NormalizedInstruction& i);
void dumpTranslationInfo(const Tracelet& t, TCA postGuards);

// SpaceRecorder is used in translator-x64.cpp and in hopt/irtranslator.cpp
// RAII logger for TC space consumption.
struct SpaceRecorder {
  const char *m_name;
  const HPHP::x64::X64Assembler m_a;
  // const X64Assembler& m_a;
  const uint8_t *m_start;
  SpaceRecorder(const char* name, const HPHP::x64::X64Assembler& a) :
      m_name(name), m_a(a), m_start(a.code.frontier)
    { }
  ~SpaceRecorder() {
    if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
      ptrdiff_t diff = m_a.code.frontier - m_start;
      if (diff) Trace::traceRelease("TCSpace %10s %3d\n", m_name, diff);
    }
    if (Trace::moduleEnabledRelease(Trace::tcdump, 1)) {
      Trace::traceRelease("TCDump %s", m_name);
      for (const uint8_t* p = m_start; p < m_a.code.frontier; p++) {
        Trace::traceRelease(" %x", *p);
      }
      Trace::traceRelease("\n");
    }
  }
};


typedef const size_t COff; // Const offsets

} } }

#endif
