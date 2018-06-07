#include "SSAApplyRegRef.h"



namespace holodec {

	bool SSAApplyRegRef::getRegisterState(Register* reg, int64_t* arithchange, SSAArgument* arg) {
		SSAExpression& referencedExpr = function->ssaRep.expressions[arg->ssaId];
		if (referencedExpr.type == SSAExprType::eInput && arg->location == SSALocation::eReg) {
			*arithchange = 0;
			return true;
		}
		if (referencedExpr.type == SSAExprType::eOp && (referencedExpr.opType == SSAOpType::eAdd || referencedExpr.opType == SSAOpType::eSub)) {
			SSAArgument* idArg = nullptr;
			int64_t change = 0;
			for (SSAArgument& onearg : referencedExpr.subExpressions) {
				if (onearg.type == SSAArgType::eId) {
					if (idArg)
						return false;
					idArg = &onearg;
				}
				if (referencedExpr.opType == SSAOpType::eAdd) {
					if (onearg.type == SSAArgType::eUInt)
						change += onearg.uval;
					else if (onearg.type == SSAArgType::eSInt)
						change += onearg.sval;
				}
				else if(referencedExpr.opType == SSAOpType::eSub){
					if (onearg.type == SSAArgType::eUInt)
						change -= onearg.uval;
					else if (onearg.type == SSAArgType::eSInt)
						change -= onearg.sval;
				}
			}
			if (idArg) {
				*arithchange += change;
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}


	bool SSAApplyRegRef::doTransformation(Binary* binary, Function* function) {// results of interprocedural liveness analysis
		bool applied = false;
		this->function = function;
		function->regStates.reg_states.clear();
		function->regStates.mem_states.clear();
		for (SSAExpression& expr : function->ssaRep.expressions) {
			if (expr.type == SSAExprType::eReturn) {
				for (auto argIt = expr.subExpressions.begin() + 1; argIt != expr.subExpressions.end(); ++argIt) {
					if (argIt->type == SSAArgType::eId) {
						Register* reg = arch->getRegister(argIt->locref.refId);
						int64_t change = 0;
						if (getRegisterState(reg, &change, &*argIt)) {
							RegisterState* state = function->regStates.getNewRegisterState(reg->parentRef.refId);
							state->arithChange = change;
							continue;
						}
					}
					if (argIt->location == SSALocation::eReg) {
						RegisterState* state = function->usedRegStates.getRegisterState(argIt->locref.refId);//reverse check if the argument is used outside in another function
						if (!function->exported && !state || !state->flags.contains(UsageFlags::eRead)) {
							argIt = expr.subExpressions.erase(argIt) - 1;
							applied = true;
						}
						else {
							Register* reg = arch->getRegister(argIt->locref.refId);
							RegisterState* state = function->regStates.getNewRegisterState(reg->parentRef.refId);
							state->flags |= UsageFlags::eWrite;
						}
					}
					else if (argIt->location == SSALocation::eMem) {
						MemoryState* state = function->usedRegStates.getMemoryState(argIt->locref.refId);//reverse check if the argument is used outside in another function
						if (!function->exported && !state || !state->flags.contains(UsageFlags::eRead)) {
							argIt = expr.subExpressions.erase(argIt) - 1;
							applied = true;
						}
						else {
							Memory* mem = arch->getMemory(argIt->locref.refId);
							MemoryState* state = function->regStates.getNewMemoryState(mem->id);
							state->flags |= UsageFlags::eWrite;
						}
					}
				}
			}
			else if (expr.type == SSAExprType::eInput) {
				if (expr.location == SSALocation::eReg) {
					Register* reg = arch->getRegister(expr.locref.refId);
					RegisterState* state = function->regStates.getNewRegisterState(reg->parentRef.refId);
					state->flags |= UsageFlags::eRead;
				}
				else if (expr.location == SSALocation::eMem) {
					Memory* mem = arch->getMemory(expr.locref.refId);
					MemoryState* state = function->regStates.getNewMemoryState(mem->id);
					state->flags |= UsageFlags::eRead;
				}
			}
			else if (expr.type == SSAExprType::eCall) {
				if (expr.subExpressions[0].type != SSAArgType::eUInt)
					continue;
				Function* callFunc = binary->getFunctionByAddr(expr.subExpressions[0].uval);
				if (!(callFunc && callFunc->regStates.parsed))
					continue;
				//first argument is the call destination so skip it
				for (auto argIt = expr.subExpressions.begin() + 1; argIt != expr.subExpressions.end(); ++argIt) {
					if (argIt->location == SSALocation::eReg) {
						Register* reg = arch->getRegister(argIt->locref.refId);
						if (!reg)
							continue;
						RegisterState* state = callFunc->regStates.getRegisterState(reg->parentRef.refId);
						if (!state || !state->flags.contains(UsageFlags::eRead)) {
							argIt = expr.subExpressions.erase(argIt) - 1;//the minus 1 works because we start of at the second argument
							applied = true;
							continue;
						}
					}
					else if (argIt->location == SSALocation::eMem) {
						Memory* mem = arch->getMemory(argIt->locref.refId);
						if (!mem)
							continue;
						MemoryState* state = callFunc->regStates.getMemoryState(mem->id);
						if (!state || !state->flags.contains(UsageFlags::eRead)) {
							argIt = expr.subExpressions.erase(argIt) - 1;//the minus 1 works because we start of at the second argument
							applied = true;
							continue;
						}
					}
				}
			}
			else if (expr.type == SSAExprType::eOutput) {
				if (expr.subExpressions[0].type != SSAArgType::eId || expr.location != SSALocation::eReg)
					continue;
				SSAExpression& callExpr = function->ssaRep.expressions[expr.subExpressions[0].ssaId];
				if (callExpr.subExpressions[0].type != SSAArgType::eUInt)
					continue;
				Function* callFunc = binary->getFunctionByAddr(callExpr.subExpressions[0].uval);
				if (!(callFunc && callFunc->regStates.parsed))
					continue;
				if (expr.location == SSALocation::eReg) {
					Register* reg = arch->getRegister(expr.locref.refId);
					if (!reg)
						continue;
					RegisterState* state = callFunc->regStates.getRegisterState(reg->parentRef.refId);
					if (state) {
						if (expr.subExpressions.size() > 1) {
							if (!state->flags.contains(UsageFlags::eWrite)) {//if the register was not written to ther is no change or just an arithmetic one
								expr.type = SSAExprType::eAssign;
								expr.removeArgument(&function->ssaRep, expr.subExpressions.begin());
								if (state->arithChange) {
									expr.type = SSAExprType::eOp;
									expr.opType = SSAOpType::eAdd;
									expr.addArgument(&function->ssaRep, SSAArgument::createSVal(state->arithChange, arch->bitbase * arch->bytebase, 0));
								}
								applied = true;
							}
							else if (!state->flags.contains(UsageFlags::eRead)) {//if the register was written to but not read, we remove the input as a second argument
								expr.subExpressions.erase(expr.subExpressions.begin() + 1);
								applied = true;
							}
						}
					}
				}
			}
			else if (expr.type == SSAExprType::eMemOutput) {
				if (expr.subExpressions[0].type != SSAArgType::eId || expr.location != SSALocation::eMem)
					continue;
				SSAExpression& callExpr = function->ssaRep.expressions[expr.subExpressions[0].ssaId];
				if (callExpr.subExpressions[0].type != SSAArgType::eUInt)
					continue;
				Function* callFunc = binary->getFunctionByAddr(callExpr.subExpressions[0].uval);
				if (!(callFunc && callFunc->regStates.parsed))
					continue;
				if (expr.location == SSALocation::eMem) {
					Memory* mem = arch->getMemory(expr.locref.refId);
					if (!mem)
						continue;
					MemoryState* state = callFunc->regStates.getMemoryState(mem->id);
					if (!state || !state->flags.contains(UsageFlags::eWrite)) {
						expr.type = SSAExprType::eAssign;
						expr.subExpressions.erase(expr.subExpressions.begin());
						applied = true;
					}
					else if (!state || !state->flags.contains(UsageFlags::eRead)) {
						if (expr.subExpressions.size() > 1) {
							expr.subExpressions.erase(expr.subExpressions.begin() + 1);
							applied = true;
						}
					}
				}
			}
		}
		function->regStates.parsed = true;
		return applied;
	}


}

