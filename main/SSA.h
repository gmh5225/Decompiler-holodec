
#ifndef SSA_H
#define SSA_H


#include "Stack.h"
#include "Register.h"
#include "General.h"
#include "Memory.h"
#include "HIdList.h"
#include "CHolodecHeader.h"

#include <assert.h>

#define SSA_LOCAL_USEID_MAX (4)

namespace holodec {

	struct Architecture;
	
	enum class SSAExprType {
		eInvalid	= SSA_EXPR_INVALID,
		
		eLabel		= SSA_EXPR_LABEL,
		eUndef		= SSA_EXPR_UNDEF,
		eNop		= SSA_EXPR_NOP,
		
		eOp			= SSA_EXPR_OP,
		eLoadAddr	= SSA_EXPR_LOADADDR,
		eFlag		= SSA_EXPR_FLAG,
		eBuiltin	= SSA_EXPR_BUILTIN,
		eExtend		= SSA_EXPR_EXTEND,
		eAppend		= SSA_EXPR_APPEND,
		eCast		= SSA_EXPR_CAST,
		
		eInput		= SSA_EXPR_INPUT,
		eOutput		= SSA_EXPR_OUTPUT,
		eMemOutput	= SSA_EXPR_MEMOUTPUT,
		
		
		eCall		= SSA_EXPR_CALL,
		eReturn		= SSA_EXPR_RETURN,
		eSyscall	= SSA_EXPR_SYSCALL,
		eTrap		= SSA_EXPR_TRAP,

		ePhi		= SSA_EXPR_PHI,
		eAssign		= SSA_EXPR_ASSIGN,

		eBranch		= SSA_EXPR_BRANCH,

		eStore		= SSA_EXPR_STORE,
		eLoad		= SSA_EXPR_LOAD,

	};
	enum class SSAOpType {
		eInvalid = SSA_OP_INVALID,
		eAdd = SSA_OP_ADD,
		eSub = SSA_OP_SUB,
		eMul = SSA_OP_MUL,
		eDiv = SSA_OP_DIV,
		eMod = SSA_OP_MOD,

		eAnd = SSA_OP_AND,
		eOr = SSA_OP_OR,
		eNot = SSA_OP_NOT,

		eEq = SSA_OP_EQ,
		eNe = SSA_OP_NE,
		eLower = SSA_OP_LOWER,
		eLe = SSA_OP_LE,
		eGreater = SSA_OP_GREATER,
		eGe = SSA_OP_GE,

		eBAnd = SSA_OP_BAND,
		eBOr = SSA_OP_BOR,
		eBXor = SSA_OP_BXOR,
		eBNot = SSA_OP_BNOT,

		eShr = SSA_OP_SHR,
		eShl = SSA_OP_SHL,
		eRor = SSA_OP_ROR,
		eRol = SSA_OP_ROL,
	};
	enum class SSAType {
		eUInt = SSA_TYPE_UINT,
		eInt = SSA_TYPE_INT,
		eFloat = SSA_TYPE_FLOAT,
		ePc = SSA_TYPE_PC,
		eMemaccess = SSA_TYPE_MEMACCESS,
	};
	enum class SSAFlagType {
		eUnknown = SSA_FLAG_UNKNOWN,
		eC = SSA_FLAG_C,
		eO = SSA_FLAG_O,
		eU = SSA_FLAG_U,
	};
	enum class SSALocation{
		eNone = SSA_LOCATION_NONE,
		eReg = SSA_LOCATION_REG,
		eMem = SSA_LOCATION_MEM,
	};
	
	enum class SSAArgType{
		eUndef = SSA_ARGTYPE_UNDEF,
		eValue = SSA_ARGTYPE_VALUE,
		eBlock = SSA_ARGTYPE_BLOCK,
		eId = SSA_ARGTYPE_ID,
	};
	struct Reference {
		HId refId;
		HId index;

		explicit operator bool() {
			return (bool)refId;
		}
		explicit operator HReference() {
			return { refId, index };
		}
		bool operator !() {
			return !refId;
		}
	};
	inline bool operator==(Reference& lhs, Reference& rhs) {
		return lhs.refId == rhs.refId && lhs.index == rhs.index;
	}
	
	struct SSAArgument {
		SSAArgType type = SSAArgType::eUndef;
		SSAType argtype = SSAType::eUInt;
		uint32_t offset = 0, size = 0;
		union {
			HId ssaId;
			int64_t sval;
			uint64_t uval;
			double fval;
		};
		SSALocation location = SSALocation::eNone;
		Reference locref = {0, 0};


		bool operator!() {
			return type != SSAArgType::eUndef;
		}
		explicit operator bool() {
			return type == SSAArgType::eUndef;
		}
		bool isConst() {
			return type == SSAArgType::eValue;
		}
		bool isConst(SSAType ssatype) {
			return type == SSAArgType::eValue && argtype == ssatype;
		}
		bool isValue(uint32_t val){
			if (type != SSAArgType::eValue)
				return false;
			if(argtype == SSAType::eInt){
				return sval == val;
			}else if(argtype == SSAType::eUInt){
				return uval == val;
			}else if(argtype == SSAType::eFloat){
				return fval == (double)val;
			}
			return false;
		}
		void replace(SSAArgument arg) {
			if (location != SSALocation::eNone) {
				arg.location = location;
				arg.locref = locref;
			}
			if (arg.type == SSAArgType::eValue && arg.argtype != argtype) {//convert the value
				if (arg.argtype == SSAType::eFloat && argtype == SSAType::eInt)
					arg.sval = static_cast<int64_t>(arg.fval);
				else if (arg.argtype == SSAType::eFloat && argtype == SSAType::eUInt)
					arg.uval = static_cast<uint64_t>(arg.fval);
				else if (arg.argtype == SSAType::eInt && argtype == SSAType::eFloat)
					arg.fval = static_cast<double>(arg.sval);
				else if (arg.argtype == SSAType::eInt && argtype == SSAType::eUInt)
					arg.uval = static_cast<uint64_t>(arg.sval);
				else if (arg.argtype == SSAType::eUInt && argtype == SSAType::eFloat)
					arg.fval = static_cast<double>(arg.uval);
				else if (arg.argtype == SSAType::eUInt && argtype == SSAType::eInt)
					arg.sval = static_cast<int64_t>(arg.uval);
			}
			arg.argtype = argtype;//keep the type
			arg.size = size;
			if (arg.size < 0) {
				puts(" ");
			}
			arg.offset += offset;
			*this = arg;
		}
		static SSAArgument replace(SSAArgument basearg, SSAArgument arg) {
			basearg.replace(arg);
			return basearg;
		}
		void set(SSAArgument arg) {
			*this = arg;
		}
		static inline SSAArgument create() {
			return SSAArgument();
		}
		static inline SSAArgument createSVal (int64_t val, uint32_t size, uint32_t offset = 0) {
			SSAArgument arg = { SSAArgType::eValue, SSAType::eInt };
			arg.sval = val;
			arg.size = size;
			arg.offset = offset;
			return arg;
		}
		static inline SSAArgument createUVal (uint64_t val, uint32_t size, uint32_t offset = 0) {
			SSAArgument arg = { SSAArgType::eValue, SSAType::eUInt };
			arg.uval = val;
			arg.size = size;
			arg.offset = offset;
			return arg;
		}
		static inline SSAArgument createFVal (double val, uint32_t size, uint32_t offset = 0) {
			SSAArgument arg = { SSAArgType::eValue, SSAType::eFloat };
			arg.fval = val;
			arg.size = size;
			arg.offset = offset;
			return arg;
		}
		static inline SSAArgument create(HId ssaId, SSAType ssatype, uint32_t size = 0, uint32_t offset = 0, SSALocation location = SSALocation::eNone, Reference locref = { 0, 0 }) {
			SSAArgument arg = { SSAArgType::eId, ssatype };
			arg.ssaId = ssaId;
			arg.location = location;
			arg.locref = locref;
			arg.size = size;
			arg.offset = offset;
			return arg;
		}
		static inline SSAArgument createOther(SSAArgType argType, SSAType ssatype, uint32_t size = 0, SSALocation location = SSALocation::eNone, Reference locref = { 0, 0 }) {
			SSAArgument arg = { argType, ssatype };
			arg.location = location;
			arg.locref = locref;
			arg.offset = 0;
			arg.size = size;
			return arg;
		}
		static inline SSAArgument createUndef (SSAType ssatype, SSALocation location, Reference locref, uint32_t size = 0) {
			SSAArgument arg = { SSAArgType::eUndef, ssatype };
			arg.location = location;
			arg.locref = locref;
			arg.size = size;
			return arg;
		}
		static inline SSAArgument createId(HId ssaId, SSAType ssatype, uint32_t size, uint32_t offset = 0) {
			assert(ssaId);
			assert(size > 0);
			return create(ssaId, ssatype, size, offset, SSALocation::eNone, { 0, 0 });
		}
		static inline SSAArgument createReg(Register* reg, SSAType ssatype, HId ssaId = 0, uint32_t offset = 0) {
			return create(ssaId, ssatype, reg->size - offset, offset, SSALocation::eReg, { reg->id, 0 });
		}
		static inline SSAArgument createReg (Reference ref, SSAType ssatype, uint32_t size, uint32_t offset, HId ssaId = 0) {
			return create(ssaId, ssatype, size, offset, SSALocation::eReg, ref);
		}
		static inline SSAArgument createMem(Memory* mem, HId ssaId = 0) {
			return  create(ssaId, SSAType::eUInt, 0, 0, SSALocation::eMem, {mem->id, 0});
		}
		static inline SSAArgument createMem (HId memId, HId ssaId = 0) {
			return  create(ssaId, SSAType::eUInt, 0, 0, SSALocation::eMem, {memId, 0});
		}
		static inline SSAArgument createBlock (HId blockId) {
			SSAArgument arg = { SSAArgType::eBlock, SSAType::eUInt };
			arg.ssaId = blockId;
			return arg;
		}

		void print(Architecture* arch);
		void printSimple(Architecture* arch);
	};


	inline bool operator== (SSAArgument& lhs, SSAArgument& rhs) {
		if (lhs.type == rhs.type && lhs.argtype == rhs.argtype && lhs.size == rhs.size && lhs.offset == rhs.offset && lhs.location == rhs.location && lhs.locref == rhs.locref) {
			switch (lhs.type) {
			case SSAArgType::eValue:
				switch (lhs.argtype) {
				case SSAType::eInt:
					return lhs.sval == rhs.sval;
				case SSAType::eUInt:
					return lhs.uval == rhs.uval;
				case SSAType::eFloat:
					return lhs.fval == rhs.fval;
				default:
					return false;
				}
			case SSAArgType::eId:
				return lhs.ssaId == rhs.ssaId;
			default:
				return false;
			}
			return true;
		}
		return false;
	}
	inline bool weak_equals(SSAArgument& lhs, SSAArgument& rhs) {
		if (lhs.type == rhs.type && lhs.argtype == rhs.argtype && lhs.size == rhs.size && lhs.offset == rhs.offset) {
			switch (lhs.type) {
			case SSAArgType::eValue:
				switch (lhs.argtype) {
				case SSAType::eInt:
					return lhs.sval == rhs.sval;
				case SSAType::eUInt:
					return lhs.uval == rhs.uval;
				case SSAType::eFloat:
					return lhs.fval == rhs.fval;
				default:
					return false;
				}
			case SSAArgType::eId:
				return lhs.ssaId == rhs.ssaId;
			default:
				return false;
			}
			return true;
		}
		return false;
	}
	inline bool consecutive_arg(SSAArgument& lhs, SSAArgument& rhs) {

		if (lhs.type == rhs.type && lhs.argtype == rhs.argtype && lhs.size + lhs.offset == rhs.offset) {
			switch (lhs.type) {
			case SSAArgType::eValue:
				switch (lhs.argtype) {
				case SSAType::eInt:
					return lhs.sval == rhs.sval;
				case SSAType::eUInt:
					return lhs.uval == rhs.uval;
				case SSAType::eFloat:
					return lhs.fval == rhs.fval;
				default:
					return false;
				}
				return false;
			case SSAArgType::eId:
				return lhs.ssaId == rhs.ssaId;
			default:
				return false;
			}
			return true;
		}
		return false;
	}
	inline bool consecutive_args(HList<SSAArgument>::iterator lhs, HList<SSAArgument>::iterator rhs) {
		if (std::distance(lhs, rhs) <= 0)
			return false;
		for (; lhs != rhs; ++lhs) {
			if (!consecutive_arg(*lhs, *(lhs + 1))) {
				return false;
			}
		}
		return true;
	}
	inline bool operator!= (SSAArgument& lhs, SSAArgument& rhs) {
		return ! (lhs == rhs);
	}
	
	class SSARepresentation;

	struct SSAExpression {
		HId id = 0;
		HId uniqueId = 0;
		HId blockId = 0;
		SSAExprType type = SSAExprType::eInvalid;
		uint32_t size = 0;
		SSAType exprtype = SSAType::eUInt;
		union { //64 bit
			SSAFlagType flagType;
			SSAOpType opType;
			HId builtinId;
		};
		SSALocation location = SSALocation::eNone;
		Reference locref = {0,0};
		uint64_t instrAddr = 0;

		HList<HId> refs;
		HList<HId> directRefs;
		HList<SSAArgument> subExpressions;

		SSAExpression() {}
		SSAExpression(SSAExprType type, uint32_t size, SSAType exprtype) : type(type), size(size), exprtype(exprtype){}

		void addArgument(SSARepresentation* rep, SSAArgument arg);
		void setArgument(SSARepresentation* rep, int index, SSAArgument arg);
		void setArgument(SSARepresentation* rep, HList<SSAArgument>::iterator it, SSAArgument arg);
		HList<SSAArgument>::iterator removeArgument(SSARepresentation* rep, HList<SSAArgument>::iterator it);
		HList<SSAArgument>::iterator removeArguments(SSARepresentation* rep, HList<SSAArgument>::iterator beginit, HList<SSAArgument>::iterator endit);
		HList<SSAArgument>::iterator insertArgument(SSARepresentation* rep, HList<SSAArgument>::iterator it, SSAArgument arg);
		void replaceArgument(SSARepresentation* rep, int index, SSAArgument arg);
		void setAllArguments(SSARepresentation* rep, HList<SSAArgument> args);

		bool operator!() {
			return type == SSAExprType::eInvalid;
		}
		operator bool() {
			return type != SSAExprType::eInvalid;
		}
		void print(Architecture* arch, int indent = 0);
		void printSimple(Architecture* arch, int indent = 0);
	};
	inline bool operator== (SSAExpression& lhs, SSAExpression& rhs) {
		if (lhs.type == rhs.type && lhs.size == rhs.size && lhs.exprtype == rhs.exprtype && lhs.location == rhs.location && lhs.locref.refId == rhs.locref.refId && lhs.locref.index == rhs.locref.index) {
			if (lhs.subExpressions.size() == rhs.subExpressions.size()) {
				for (size_t i = 0; i < lhs.subExpressions.size(); i++) {
					if (lhs.subExpressions[i] != rhs.subExpressions[i])
						return false;
				}
			}
			switch (rhs.type) {
			case SSAExprType::eFlag:
				return lhs.flagType == rhs.flagType;
			case SSAExprType::eOp:
				return lhs.opType == rhs.opType;
			case SSAExprType::eBuiltin:
				return lhs.builtinId == rhs.builtinId;
			default:
				return true;
			}
		}
		return false;
	}
	
	struct SSABB {
		HId id;
		uint64_t startaddr = (uint64_t)-1;
		uint64_t endaddr = 0;
		HList<HId> exprIds;
		HSet<HId> inBlocks;
		HSet<HId> outBlocks;

		SSABB() {}
		SSABB (uint64_t startaddr, uint64_t endaddr, HList<HId> exprIds, HSet<HId> inBlocks, HSet<HId> outBlocks) :
			id(0),startaddr(startaddr),endaddr(endaddr),exprIds(exprIds),inBlocks(inBlocks),outBlocks(outBlocks){}
		~SSABB() = default;


		HId getInputSSA (Register* reg);
	};


	class SSARepresentation {
		HList<HId>::iterator addBefore(SSAExpression* expr, HList<HId>& ids, HList<HId>::iterator it);
		HList<HId>::iterator addAfter(SSAExpression* expr, HList<HId>& ids, HList<HId>::iterator it);
	public:
		HIdList<SSABB> bbs;
		HSparseIdList<SSAExpression> expressions;

		HIdGenerator exprIdGen;

		void clear(){
			bbs.clear();
			expressions.clear();
		}

		void replaceNodes(HMap<HId,SSAArgument>* replacements);
		uint64_t replaceAllArgs(SSAExpression& origExpr, SSAArgument replaceArg);
		bool isReplaceable(SSAExpression& origExpr);
		uint64_t replaceExpr(SSAExpression& origExpr, SSAArgument replaceArg);
		void removeNodes(HSet<HId>* ids);
		
		void compress();
		
		bool checkIntegrity();

		void propagateRefCount(SSAExpression* expr);
		void propagateRefCount(HId id);
		
		void changeRefCount(HId id, HId refId);
		void changeRefCount(HId id, std::vector<bool>& visited, HId refId);
		
		void recalcRefCounts();

		bool calcConstValue(SSAArgument argument, uint64_t* result);

		HId addExpr(SSAExpression* expr);
		
		HId addAtEnd(SSAExpression* expr, HId blockId);
		HId addAtEnd(SSAExpression* expr, SSABB* bb);
		HId addAtStart(SSAExpression* expr, HId blockId);
		HId addAtStart(SSAExpression* expr, SSABB* bb);
		
		HId addBefore(SSAExpression* expr, HId ssaId);
		HId addAfter(SSAExpression* expr, HId ssaId);

		bool isNotUsedBefore(SSAExpression& firstExpr, SSAExpression& secondExpr);
		bool areArgsNotDefedBefore(SSAExpression& firstExpr, SSAExpression& secondExpr);

		HList<HId>::iterator removeExpr(HList<HId>& ids, HList<HId>::iterator it);
		void removeExpr(HId ssaId);

		bool usedOnlyInFlags(SSAExpression& expr) {
			for (HId id : expr.directRefs) {//iterate refs
				if (expressions[id].type != SSAExprType::eFlag)
					return false;
			}
			return true;
		}

		void print(Architecture* arch, int indent = 0);
		void printSimple(Architecture* arch, int indent = 0);
		
	};
	

	bool calculante_difference(SSARepresentation* ssaRep, HId firstid, HId secid, int64_t* change);

	//returns the distance traveled from the ssaId
	//if distance is 0 then the expression with id == ssaId was neither an addition or a subtraction
	uint64_t calculate_basearg_plus_offset(SSARepresentation* ssaRep, HId ssaId, 
		int64_t* arithchange/* result | the value that was added or subtracted */, SSAArgument* basearg/* result | the furthest argument we can travel to */);

}

#endif //SSA_H
