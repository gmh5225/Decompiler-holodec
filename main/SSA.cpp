#include "SSA.h"
#include "Architecture.h"

#include <cassert>

namespace holodec {

	void SSAExpression::addArgument(SSARepresentation* rep, SSAArgument arg) {
		if (arg.type == SSAArgType::eId) {//add ref
			SSAExpression& expr = rep->expressions[arg.ssaId];
			expr.directRefs.push_back(id);
		}
		subExpressions.push_back(arg);
	}
	void SSAExpression::setArgument(SSARepresentation* rep, int index, SSAArgument arg) {
		setArgument(rep, subExpressions.begin() + index, arg);
	}
	void SSAExpression::setArgument(SSARepresentation* rep, HList<SSAArgument>::iterator it, SSAArgument arg) {
		if (it->type == SSAArgType::eId) {//remove ref
			SSAExpression& expr = rep->expressions[it->ssaId];
			for (auto it = expr.directRefs.begin(); it != expr.directRefs.end(); ++it) {
				if (*it == id) {
					expr.directRefs.erase(it);//erase only one
					break;
				}
			}
		}
		if (arg.type == SSAArgType::eId) {//add ref
			SSAExpression& expr = rep->expressions[arg.ssaId];
			expr.directRefs.push_back(id);
		}
		it->set(arg);
	}
	HList<SSAArgument>::iterator SSAExpression::removeArgument(SSARepresentation* rep, HList<SSAArgument>::iterator it) {
		if (it->type == SSAArgType::eId) {//remove ref
			SSAExpression& expr = rep->expressions[it->ssaId];
			for (auto it = expr.directRefs.begin(); it != expr.directRefs.end(); ++it) {
				if (*it == id) {
					expr.directRefs.erase(it);//erase only one
					break;
				}
			}
		}
		return subExpressions.erase(it);
	}
	HList<SSAArgument>::iterator SSAExpression::removeArguments(SSARepresentation* rep, HList<SSAArgument>::iterator beginit, HList<SSAArgument>::iterator endit) {
		for (auto it = beginit; it != endit; ++it) {
			if (it->type == SSAArgType::eId) {//remove ref
				SSAExpression& expr = rep->expressions[it->ssaId];
				for (auto it = expr.directRefs.begin(); it != expr.directRefs.end(); ++it) {
					if (*it == id) {
						expr.directRefs.erase(it);//erase only one
						break;
					}
				}
			}
		}
		return subExpressions.erase(beginit, endit);
	}
	HList<SSAArgument>::iterator SSAExpression::insertArgument(SSARepresentation* rep, HList<SSAArgument>::iterator it, SSAArgument arg) {
		if (arg.type == SSAArgType::eId) {//remove ref
			rep->expressions[arg.ssaId].directRefs.push_back(id);
		}
		return subExpressions.insert(it, arg) + 1;
	}
	void SSAExpression::replaceArgument(SSARepresentation* rep, int index, SSAArgument arg) {
		if (subExpressions[index].type == SSAArgType::eId) {//remove ref
			SSAExpression& expr = rep->expressions[subExpressions[index].ssaId];
			for (auto it = expr.directRefs.begin(); it != expr.directRefs.end(); ++it) {
				if (*it == id) {
					expr.directRefs.erase(it);//erase only one
					break;
				}
			}
		}
		if (arg.type == SSAArgType::eId) {//add ref
			SSAExpression& expr = rep->expressions[arg.ssaId];
			expr.directRefs.push_back(id);
		}
		subExpressions[index].replace(arg);
	}
	void SSAExpression::setAllArguments(SSARepresentation* rep, HList<SSAArgument> args) {
		for (SSAArgument& arg : subExpressions) {//remove refs
			if (arg.type == SSAArgType::eId) {
				SSAExpression& expr = rep->expressions[arg.ssaId];
				for (auto it = expr.directRefs.begin(); it != expr.directRefs.end();) {
					if (*it == id) {
						it = expr.directRefs.erase(it);//erase all
						continue;
					}
					++it;
				}
			}
		}
		for (SSAArgument& arg : args) {//add refs
			if (arg.type == SSAArgType::eId) {
				SSAExpression& expr = rep->expressions[arg.ssaId];
				expr.directRefs.push_back(id);
			}
		}
		subExpressions = args;
	}
	void SSAExpression::print(Architecture* arch, int indent) {
		printIndent(indent);
		printf("0x%" PRIx64 ":", instrAddr);

		switch (this->exprtype) {
		case SSAType::eInt:
			printf("  int");
			break;
		case SSAType::eUInt:
			printf(" uint");
			break;
		case SSAType::eFloat:
			printf("float");
			break;
		case SSAType::ePc:
			printf("   pc");
			break;
		case SSAType::eMemaccess:
			printf("  mem");
			break;
		}
		printf("%.3d ", size);

		switch (type) {
		case SSAExprType::eInvalid:
			printf("---------------------------------------");
			printf("Invalid%d ", type);
			break;
		case SSAExprType::eLabel:
			printf("Label  ");
			break;
		case SSAExprType::eInput:
			printf("Input  ");
			break;
		case SSAExprType::eUndef:
			printf("Undef  ");
			break;
		case SSAExprType::eNop:
			printf("Nop    ");
			break;
		case SSAExprType::ePhi:
			printf("Phi    ");
			break;
		case SSAExprType::eAssign:
			printf("Assign ");
			break;
		case SSAExprType::eBranch:
			printf("Branch ");
			break;
		case SSAExprType::eOp:
			printf("Op");
			switch (opType) {
			case SSAOpType::eInvalid:
				printf(" Inv ");
				break;
			case SSAOpType::eAdd:
				printf(" +   ");
				break;
			case SSAOpType::eSub:
				printf(" -   ");
				break;
			case SSAOpType::eMul:
				printf(" *   ");
				break;
			case SSAOpType::eDiv:
				printf(" /   ");
				break;
			case SSAOpType::eMod:
				printf(" %%   ");
				break;
			case SSAOpType::eAnd:
				printf(" &&  ");
				break;
			case SSAOpType::eOr:
				printf(" ||  ");
				break;
			case SSAOpType::eNot:
				printf(" !   ");
				break;
			case SSAOpType::eEq:
				printf(" ==  ");
				break;
			case SSAOpType::eNe:
				printf(" !=  ");
				break;
			case SSAOpType::eLower:
				printf(" <   ");
				break;
			case SSAOpType::eLe:
				printf(" <=  ");
				break;
			case SSAOpType::eGreater:
				printf(" >   ");
				break;
			case SSAOpType::eGe:
				printf(" >=  ");
				break;
			case SSAOpType::eBAnd:
				printf(" &   ");
				break;
			case SSAOpType::eBOr:
				printf(" |   ");
				break;
			case SSAOpType::eBXor:
				printf(" ^   ");
				break;
			case SSAOpType::eBNot:
				printf(" ~   ");
				break;
			case SSAOpType::eShr:
				printf(" >>  ");
				break;
			case SSAOpType::eShl:
				printf(" <<  ");
				break;
			case SSAOpType::eRor:
				printf(" >>> ");
				break;
			case SSAOpType::eRol:
				printf(" <<< ");
				break;
			}
			break;
		case SSAExprType::eLoadAddr:
			printf("Lea    ");
			break;
		case SSAExprType::eCall:
			printf("Call   ");
			break;
		case SSAExprType::eOutput:
			printf("Output ");
			break;
		case SSAExprType::eMemOutput:
			printf("MemOut ");
			break;
		case SSAExprType::eReturn:
			printf("Return ");
			break;
		case SSAExprType::eSyscall:
			printf("Syscall");
			break;
		case SSAExprType::eTrap:
			printf("Trap   ");
			break;
		case SSAExprType::eBuiltin:
			printf("Builtin");
			break;
		case SSAExprType::eExtend:
			printf("Extend ");
			break;
		case SSAExprType::eAppend:
			printf("Append ");
			break;
		case SSAExprType::eCast:
			printf("Cast   ");
			break;
		case SSAExprType::eMemAccess:
			printf("MemAcc ");
			break;
		case SSAExprType::eStore:
			printf("Store  ");
			break;
		case SSAExprType::eLoad:
			printf("Load   ");
			break;
		case SSAExprType::eFlag:
			printf("Flag");
			switch (flagType) {
			case SSAFlagType::eUnknown:
				printf("Unk");
				break;
			case SSAFlagType::eC:
				printf("Cry");
				break;
			case SSAFlagType::eO:
				printf("Ovf");
				break;
			case SSAFlagType::eU:
				printf("Udf");
				break;
			}
			break;
		}
		switch (location) {
		case SSALocation::eReg:
			printf("Reg: %10.10s ", arch->getRegister(locref.refId)->name.cstr());
			break;
		case SSALocation::eMem:
			printf("Mem: %10.10s ", arch->getMemory(locref.refId)->name.cstr());
			break;
		case SSALocation::eNone:
			printf("                ");
			break;
		}
		printf("Ref: %2" PRId64 " UId: %4" PRIx32 " Block: %2" PRId32 " | %4" PRId32 " = ", refs.size(), uniqueId, blockId, id);
		for (SSAArgument& arg : subExpressions) {
			arg.print(arch);
			printf(", ");
		}

		printf(" | Refs: ");
		for (HId refId : refs) {
			printf("%d, ", refId);
		}
		printf("\n");
	}

	void SSAExpression::printSimple(Architecture* arch, int indent) {
		printIndent(indent);

		switch (this->exprtype) {
		case SSAType::eInt:
			printf("  int");
			break;
		case SSAType::eUInt:
			printf(" uint");
			break;
		case SSAType::eFloat:
			printf("float");
			break;
		case SSAType::ePc:
			printf("   pc");
			break;
		case SSAType::eMemaccess:
			printf("  mem");
			break;
		}
		printf("%3.3d ", size);

		switch (type) {
		case SSAExprType::eInvalid:
			printf("---------------------------------------");
			printf("Invalid%d ", type);
			break;
		case SSAExprType::eLabel:
			printf("Label  ");
			break;
		case SSAExprType::eInput:
			printf("Input  ");
			break;
		case SSAExprType::eUndef:
			printf("Undef  ");
			break;
		case SSAExprType::eNop:
			printf("Nop    ");
			break;
		case SSAExprType::ePhi:
			printf("Phi    ");
			break;
		case SSAExprType::eAssign:
			printf("Assign ");
			break;
		case SSAExprType::eBranch:
			printf("Branch ");
			break;
		case SSAExprType::eOp:
			printf("Op");
			switch (opType) {
			case SSAOpType::eInvalid:
				printf(" Inv ");
				break;
			case SSAOpType::eAdd:
				printf(" +   ");
				break;
			case SSAOpType::eSub:
				printf(" -   ");
				break;
			case SSAOpType::eMul:
				printf(" *   ");
				break;
			case SSAOpType::eDiv:
				printf(" /   ");
				break;
			case SSAOpType::eMod:
				printf(" %%   ");
				break;
			case SSAOpType::eAnd:
				printf(" &&  ");
				break;
			case SSAOpType::eOr:
				printf(" ||  ");
				break;
			case SSAOpType::eNot:
				printf(" !   ");
				break;
			case SSAOpType::eEq:
				printf(" ==  ");
				break;
			case SSAOpType::eNe:
				printf(" !=  ");
				break;
			case SSAOpType::eLower:
				printf(" <   ");
				break;
			case SSAOpType::eLe:
				printf(" <=  ");
				break;
			case SSAOpType::eGreater:
				printf(" >   ");
				break;
			case SSAOpType::eGe:
				printf(" >=  ");
				break;
			case SSAOpType::eBAnd:
				printf(" &   ");
				break;
			case SSAOpType::eBOr:
				printf(" |   ");
				break;
			case SSAOpType::eBXor:
				printf(" ^   ");
				break;
			case SSAOpType::eBNot:
				printf(" ~   ");
				break;
			case SSAOpType::eShr:
				printf(" >>  ");
				break;
			case SSAOpType::eShl:
				printf(" <<  ");
				break;
			case SSAOpType::eRor:
				printf(" >>> ");
				break;
			case SSAOpType::eRol:
				printf(" <<< ");
				break;
			}
			break;
		case SSAExprType::eLoadAddr:
			printf("Lea    ");
			break;
		case SSAExprType::eCall:
			printf("Call   ");
			break;
		case SSAExprType::eOutput:
			printf("Output ");
			break;
		case SSAExprType::eMemOutput:
			printf("MemOut ");
			break;
		case SSAExprType::eReturn:
			printf("Return ");
			break;
		case SSAExprType::eSyscall:
			printf("Syscall");
			break;
		case SSAExprType::eTrap:
			printf("Trap   ");
			break;
		case SSAExprType::eBuiltin:
			printf("Builtin");
			break;
		case SSAExprType::eExtend:
			printf("Extend ");
			break;
		case SSAExprType::eAppend:
			printf("Append ");
			break;
		case SSAExprType::eCast:
			printf("Cast   ");
			break;
		case SSAExprType::eMemAccess:
			printf("MemAcc ");
			break;
		case SSAExprType::eStore:
			printf("Store  ");
			break;
		case SSAExprType::eLoad:
			printf("Load   ");
			break;
		case SSAExprType::eFlag:
			printf("Flag");
			switch (flagType) {
			case SSAFlagType::eUnknown:
				printf("Unk");
				break;
			case SSAFlagType::eC:
				printf("Cry");
				break;
			case SSAFlagType::eO:
				printf("Ovf");
				break;
			case SSAFlagType::eU:
				printf("Udf");
				break;
			}
			break;
		}
		printf("%" PRId32 " = ", id);
		switch (location) {
		case SSALocation::eReg:
			printf("Reg: %s ", arch->getRegister(locref.refId)->name.cstr());
			break;
		case SSALocation::eMem:
			printf("Mem: %s ", arch->getMemory(locref.refId)->name.cstr());
			break;
		case SSALocation::eNone:
			break;
		}
		for (SSAArgument& arg : subExpressions) {
			arg.printSimple(arch);
			printf(", ");
		}
		printf("\n");
	}
	void SSAArgument::print(Architecture* arch) {
		switch (type) {
		case SSAArgType::eUndef:
			printf("Undef");
			break;
		case SSAArgType::eValue:
			switch (argtype) {
			case SSAType::eInt:
				if (sval < 0)
					printf("-0x%" PRIx64 " ", -sval);
				else
					printf("0x%" PRIx64 " ", sval);
				break;
			case SSAType::eUInt:
				printf("0x%" PRIx64 " ", uval);
				break;
			case SSAType::eFloat:
				printf("%f ", fval);
				break;
			}
			break;
		case SSAArgType::eId:
			printf("SSA: %d ", ssaId);
			break;
		case SSAArgType::eBlock:
			printf("Block: %" PRId32 " ", ssaId);
			break;
		case SSAArgType::eOther:
			break;
		default:
			printf("Unknown Argtype %x ", type);
		}

		switch (location) {
		case SSALocation::eReg:
			if (locref.refId)
				printf("Reg: %s ", arch->getRegister(locref.refId)->name.cstr());
			else
				printf("No Reg Def ");
			break;
		case SSALocation::eMem:
			printf("Memory %s ", arch->getMemory(locref.refId)->name.cstr());
			break;
		case SSALocation::eNone:
			break;
		}

		switch (argtype) {
		case SSAType::eInt:
			printf("s[%d,%d]", offset, size);
			break;
		case SSAType::eUInt:
			printf("u[%d,%d]", offset, size);
			break;
		case SSAType::eFloat:
			printf("f[%d,%d]", offset, size);
			break;
		}
	}
	void SSAArgument::printSimple(Architecture* arch) {
		switch (type) {
		case SSAArgType::eUndef:
			printf("Undef");
			break;
		case SSAArgType::eValue:
			switch (argtype) {
			case SSAType::eInt:
				if (sval < 0)
					printf("-0x%" PRIx64 " ", -sval);
				else
					printf("0x%" PRIx64 " ", sval);
				break;
			case SSAType::eUInt:
				printf("0x%" PRIx64 " ", uval);
				break;
			case SSAType::eFloat:
				printf("%f ", fval);
				break;
			}
			break;
		case SSAArgType::eId:
			printf("SSA: %" PRId32 " ", ssaId);
			break;
		case SSAArgType::eBlock:
			printf("Block: %" PRId32 " ", ssaId);
			break;
		case SSAArgType::eOther:
			break;
		default:
			printf("Unknown Argtype %d ", type);
		}

		switch (location) {
		case SSALocation::eMem:
			printf("Memory %s ", arch->getMemory(locref.refId)->name.cstr());
			break;
		case SSALocation::eNone:
			break;
		}
		switch (argtype) {
		case SSAType::eInt:
			printf("s[%d,%d]", offset, size);
			break;
		case SSAType::eUInt:
			printf("u[%d,%d]", offset, size);
			break;
		case SSAType::eFloat:
			printf("f[%d,%d]", offset, size);
			break;
		}
	}


	void SSARepresentation::replaceNodes (HMap<HId, SSAArgument>* replacements) {

		bool replaced = false;
		do {
			replaced = false;
			for (auto it = replacements->begin(); it != replacements->end(); ++it) {
				if (it->second.type != SSAArgType::eId || it->first == it->second.ssaId)
					continue;
				auto innerIt = replacements->find(it->second.ssaId);
				while (innerIt != replacements->end()) {//TODO infinite loop alarm!!!!!!
					if (it->first == innerIt->second.ssaId)
						break;
					it->second.replace(innerIt->second);
					innerIt = replacements->find(it->second.ssaId);
					replaced = true;
				}
			}
		} while (replaced);

		printf ("Remove\n");
		for (SSABB& bb : bbs) {
			for (auto it = bb.exprIds.begin(); it != bb.exprIds.end();) {
				if (replacements->find (*it) != replacements->end()) {
					it = removeExpr (bb.exprIds, it);
					continue;
				}
				it++;
			}
		}
		printf ("Change Args\n");
		for (SSAExpression& expr : expressions) {
			for (SSAArgument& arg : expr.subExpressions) {
				if (arg.type == SSAArgType::eId) {
					auto it = replacements->find(arg.ssaId);
					if (it != replacements->end()) {
						arg.replace(it->second);
					}
				}
			}
			for (auto it = expr.refs.begin(); it != expr.refs.end(); ) {
				HId refId = *it;
				auto repIt = replacements->find(refId);
				if (repIt != replacements->end()) {
					if (repIt->second.type == SSAArgType::eId){
						refId = repIt->second.ssaId;
					}
					else {
						it = expr.refs.erase(it);
						continue;
					}
				}
				++it;
			}
		}
		recalcRefCounts();
	}
	uint64_t SSARepresentation::replaceAllArgs(SSAExpression& origExpr, SSAArgument replaceArg) {

		uint64_t count = 0;
		if (replaceArg.ssaId == origExpr.id) {//don't replace refs and args if replace is the same
			for (HId id : origExpr.directRefs) {//iterate refs
				SSAExpression& expr = expressions[id];
				for (SSAArgument& arg : expr.subExpressions) {
					if (arg.type == SSAArgType::eId && arg.ssaId == origExpr.id) {
						arg.replace(replaceArg);
						count++;
					}
				}
			}
		}
		else {
			for (HId id : origExpr.directRefs) {//iterate refs
				SSAExpression& expr = expressions[id];
				for (SSAArgument& arg : expr.subExpressions) {
					if (arg.type == SSAArgType::eId && arg.ssaId == origExpr.id) {
						arg.replace(replaceArg);
						count++;
						if (replaceArg.type == SSAArgType::eId)
							expressions[replaceArg.ssaId].directRefs.push_back(id);
					}
				}
			}
		}
		if (!(replaceArg.type == SSAArgType::eId && replaceArg.ssaId == origExpr.id)) {
			origExpr.directRefs.clear();
			origExpr.refs.clear();
		}
		return count;
	}


	inline bool shouldReplaceIn(SSAExpression& expr, SSAArgument* replaceArg, bool isCopy/*if the argument copies the referencing expression or value*/) {
		if (expr.type == SSAExprType::eFlag) {//ignore flags because they are operation specific and need the operation for it's meaning
			return false;
		}
		if (EXPR_IS_TRANSPARENT(expr.type) && (!isCopy || (!replaceArg || replaceArg->isConst()))) {
			return false;
		}
		return true;//otherwise can be replaced
	}

	bool SSARepresentation::isReplaceable(SSAExpression& origExpr) {
		for (HId directRefId : origExpr.directRefs) {
			SSAExpression& expr = expressions[directRefId];
			if (shouldReplaceIn(expr, nullptr, true)) {
				return true;
			}
		}
		return false;
	}
	uint64_t SSARepresentation::replaceExpr(SSAExpression& origExpr, SSAArgument replaceArg) {
		uint64_t count = 0;

		//if the argument is not the same as the expression it references then we do not replace them
		bool isCopy = (replaceArg.offset == 0);
		if (replaceArg.type == SSAArgType::eId) {
			SSAExpression& expr = expressions[replaceArg.ssaId];
			isCopy = isCopy && (expr.size == (replaceArg.offset + replaceArg.size));
		}
		for (HId directRefId : origExpr.directRefs) {//iterate refs
			SSAExpression& expr = expressions[directRefId];
			if (!shouldReplaceIn(expr, &replaceArg, isCopy)) {
				continue;
			}
			if (replaceArg.type == SSAArgType::eId && replaceArg.ssaId == origExpr.id) {//don't replace refs and args if replace is the same
				for (SSAArgument& arg : expr.subExpressions) {
					if (arg.type == SSAArgType::eId && arg.ssaId == origExpr.id) {
						arg.replace(replaceArg);
						count++;
					}
				}
			}
			else {
				for (SSAArgument& arg : expr.subExpressions) {
					if (arg.type == SSAArgType::eId && arg.ssaId == origExpr.id) {
						arg.replace(replaceArg);
						count++;
						if (replaceArg.type == SSAArgType::eId)
							expressions[replaceArg.ssaId].refs.push_back(directRefId);
					}
				}
			}
		}
		if (!(replaceArg.type == SSAArgType::eId && replaceArg.ssaId == origExpr.id)) {
			for (auto it = origExpr.refs.begin(); it != origExpr.refs.end();) {
				if (!shouldReplaceIn(expressions[*it], &replaceArg, isCopy)) {
					it++;
					continue;
				}
				it = origExpr.refs.erase(it);
			}
			for (auto it = origExpr.directRefs.begin(); it != origExpr.directRefs.end();) {
				if (!shouldReplaceIn(expressions[*it], &replaceArg, isCopy)) {
					it++;
					continue;
				}
				it = origExpr.directRefs.erase(it);
			}
		}
		return count;
	}
	void SSARepresentation::removeNodes (HSet<HId>* ids) {
		for (SSABB& bb : bbs) {
			for (auto it = bb.exprIds.begin(); it != bb.exprIds.end();) {
				if (ids->find(*it) != ids->end())
					it = removeExpr(bb.exprIds, it);
				else
					++it;
			}
		}
	}

	void SSARepresentation::compress() {

		std::map<HId, HId> replacements;

		for (SSAExpression& expr : expressions) {
			for (auto it = expr.refs.begin(); it != expr.refs.end();) {
				if (!expressions[*it].id)
					it = expr.refs.erase(it);
				else
					it++;
			}
			for (auto it = expr.directRefs.begin(); it != expr.directRefs.end();) {
				if (!expressions[*it].id)
					it = expr.directRefs.erase(it);
				else
					it++;
			}
		}

		expressions.shrink ([&replacements] (HId oldId, HId newId) {
			replacements[oldId] = newId;
		});

		for (SSABB& bb : bbs) {
			bb.exprIds.shrink_to_fit();
		}

		if (!replacements.empty()) {
			for (SSAExpression& expr : expressions) {
				for (SSAArgument& arg : expr.subExpressions) {
					if (arg.type == SSAArgType::eId) {
						auto it = replacements.find (arg.ssaId);
						if (it != replacements.end()) {
							arg.ssaId = it->second;
						}
					}
				}
				for (HId& ref : expr.refs) {
					auto it = replacements.find(ref);
					if (it != replacements.end()) {
						ref = it->second;
					}
				}
				for (HId& ref : expr.directRefs) {
					auto it = replacements.find(ref);
					if (it != replacements.end()) {
						ref = it->second;
					}
				}
			}
			for (SSABB& bb : bbs) {
				for (HId& id : bb.exprIds) {
					auto it = replacements.find (id);
					if (it != replacements.end()) {
						id = it->second;
					}
				}
			}
		}
	}

	bool SSARepresentation::checkIntegrity() {
		for (SSAExpression& expr : expressions) {
			if (expr.id) {
				for (SSAArgument& arg : expr.subExpressions) {
					if (arg.type == SSAArgType::eId && !(arg.ssaId > 0 && arg.ssaId <= expressions.size())) {
						fprintf(stderr, "Invalid ssaId in arg from Expression %d\n", expr.id);
						return false;
					}
					if (arg.type == SSAArgType::eId && !expressions[arg.ssaId].id) {
						fprintf(stderr, "ssa refers to deleted Expression %d in arg from Expression %d\n", arg.ssaId, expr.id);
						return false;
					}
					if (arg.type == SSAArgType::eId && expressions[arg.ssaId].size < arg.offset + arg.size){
						fprintf(stderr, "Size of arg is bigger than Expression %d in arg from Expression %d\n", arg.ssaId, expr.id);
						return false;
					}
				}
				if (!expr.blockId){
					fprintf(stderr, "Invalid Blockid in Expression %d\n", expr.id);
					return false;
				}
				for (HId& id : expr.refs)
					if (!(id && id <= expressions.size() && expressions[id].id)){
						fprintf(stderr, "Invalid ssaId in ref from Expression %d\n", expr.id);
						return false;
					}
				for (HId& id : expr.directRefs)
					if (!(id && id <= expressions.size() && expressions[id].id)){
						fprintf(stderr, "Invalid ssaId in directRef from Expression %d\n", expr.id);
						return false;
					}
			}
		}
		return true;
	}

	void SSARepresentation::propagateRefCount (SSAExpression* expr) {
		for (SSAArgument& arg : expr->subExpressions) {
			if (arg.type == SSAArgType::eId && arg.ssaId) {
				expressions[arg.ssaId].directRefs.push_back(expr->id);
				changeRefCount(arg.ssaId, expr->id);
			}
		}
	}
	void SSARepresentation::propagateRefCount (HId id) {
		if (!id)
			return;
		propagateRefCount (&expressions[id]);
	}
	void SSARepresentation::changeRefCount (HId id, HId refId) {
		if (!id)
			return;
		SSAExpression& expr = expressions[id];
		expr.refs.push_back(refId);
		if (EXPR_IS_TRANSPARENT (expr.type)) {
			std::vector<bool> visited;
			visited.resize (expressions.size(), false);
			changeRefCount (id, visited, refId);
		}
	}
	void SSARepresentation::changeRefCount (HId id, std::vector<bool>& visited, HId refId) {
		if (!id || visited[id - 1])
			return;
		visited[id - 1] = true;
		SSAExpression& expr = expressions[id];
		expr.refs.push_back(refId);
		if (EXPR_IS_TRANSPARENT (expr.type)) {
			for (SSAArgument& arg : expr.subExpressions) {
				if (arg.type == SSAArgType::eId)
					changeRefCount (arg.ssaId, visited, refId);
			}
		}
		else {
			expr.refs.push_back(refId);
		}
	}
	void SSARepresentation::recalcRefCounts() {
		for (SSAExpression& expr : expressions) {
			expr.refs.clear();
			expr.directRefs.clear();
		}
		for (SSAExpression& expr : expressions) {
			if (expr.id) {
				propagateRefCount(&expr);
			}
		}
	}
	bool SSARepresentation::calcConstValue(SSAArgument argument, uint64_t* result) {
		if (argument.isConst()) {
			switch (argument.argtype) {
			//case SSAType::eSInt:
			//	*result = static_cast<uint64_t>(argument.sval);
			//	return true;
			case SSAType::eUInt:
				*result = argument.uval;
				return true;
			//case SSAType::eFloat:
			//	*result = static_cast<uint64_t>(argument.fval);
			//	return true;
			default:
				return false;
			}
		}
		else if (argument.type == SSAArgType::eId) {
			SSAExpression& expr = expressions[argument.ssaId];
			switch (expr.type) {
			case SSAExprType::eLoadAddr:
				uint64_t base, index, disp, offset;
				if (calcConstValue(expr.subExpressions[1], &base) &&
					calcConstValue(expr.subExpressions[2], &index) &&
					calcConstValue(expr.subExpressions[3], &disp) &&
					calcConstValue(expr.subExpressions[4], &offset)) {
					*result = base + (index * disp) + offset;
					return true;
				}
			}
		}
		return false;
	}
	HId SSARepresentation::addExpr (SSAExpression* expr) {
		expr->uniqueId = exprIdGen.next();
		expressions.push_back (*expr);
		HId newId = expressions.back().id;
		if (!EXPR_IS_TRANSPARENT (expr->type))
			propagateRefCount (newId);
		return newId;
	}

	HId SSARepresentation::addAtEnd (SSAExpression* expr, HId blockId) {
		return addAtEnd (expr, &bbs[blockId]);
	}
	HId SSARepresentation::addAtEnd (SSAExpression* expr, SSABB* bb) {
		expr->blockId = bb->id;
		HId newId = addExpr (expr);
		bb->exprIds.push_back (newId);
		return newId;
	}
	HId SSARepresentation::addAtStart (SSAExpression* expr, HId blockId) {
		return addAtStart (expr, &bbs[blockId]);
	}
	HId SSARepresentation::addAtStart (SSAExpression* expr, SSABB* bb) {
		expr->blockId = bb->id;
		HId newId = addExpr (expr);
		bb->exprIds.insert (bb->exprIds.begin(), newId);
		return newId;
	}

	HId SSARepresentation::addBefore (SSAExpression* expr, HId ssaId) {
		if (! (ssaId && ssaId <= expressions.size() && expressions[ssaId].id))
			return 0;
		SSABB& bb = bbs[expressions[ssaId].blockId];
		for (auto it = bb.exprIds.begin(); it != bb.exprIds.end(); ++it) {
			if (*it == ssaId) {
				it = bb.exprIds.insert(it, addExpr(expr));
				expressions[*it].blockId = bb.id;
				return *it;
			}
		}
		assert(false);
		return 0;
	}
	HId SSARepresentation::addAfter (SSAExpression* expr, HId ssaId) {
		if (! (ssaId && ssaId <= expressions.size() && expressions[ssaId].id))
			return 0;
		SSABB& bb = bbs[expressions[ssaId].blockId];
		for (auto it = bb.exprIds.begin(); it != bb.exprIds.end(); ++it) {
			if (*it == ssaId) {
				it = bb.exprIds.insert(it + 1, addExpr(expr));
				expressions[*it].blockId = bb.id;
				return *it;
			}
		}
		assert(false);
		return 0;
	}
	HList<HId>::iterator SSARepresentation::addAfter (SSAExpression* expr, HList<HId>& ids, HList<HId>::iterator it) {
		return ids.insert (++it, addExpr (expr));
	}

	HList<HId>::iterator SSARepresentation::removeExpr (HList<HId>& ids, HList<HId>::iterator it) {
		SSAExpression& expr = expressions[*it];
		expr.id = 0;
		return ids.erase (it);
	}
	void SSARepresentation::removeExpr (HId ssaId) {
		if (! (ssaId && ssaId <= expressions.size() && expressions[ssaId].id))
			return;
		SSABB& bb = bbs[expressions[ssaId].blockId];
		for (auto it = bb.exprIds.begin(); it != bb.exprIds.end(); ++it) {
			if (*it == ssaId) {
				removeExpr (bb.exprIds, it);
				return;
			}
		}
	}
	/*
		Is the first argument used before the second
	*/
	bool SSARepresentation::isNotUsedBefore(SSAExpression& firstExpr, SSAExpression& secondExpr) {
		if (firstExpr.blockId != secondExpr.blockId)
			return false;
		SSABB& bb = this->bbs[firstExpr.blockId];
		auto firstit = bb.exprIds.begin();
		for (; firstit != bb.exprIds.end() && *firstit == firstExpr.id; firstit++);
		auto secit = bb.exprIds.begin();
		for (; secit != bb.exprIds.end() && *secit == secondExpr.id; secit++);
		if (firstit > secit || firstit == bb.exprIds.end() || secit == bb.exprIds.end())
			return false;
		if (firstit == secit)
			return true;
		for (auto it = firstit + 1; it != secit; it++) {
			SSAExpression& expr = expressions[*it];
			for (SSAArgument& arg : expr.subExpressions) {
				if (arg.type == SSAArgType::eId && arg.ssaId == *firstit)
					return false;
			}
		}
		return true;
	}
	/*
		Are the args from the second argument defined between the first and the second
	*/
	bool SSARepresentation::areArgsNotDefedBefore(SSAExpression& firstExpr, SSAExpression& secondExpr) {
		if (firstExpr.blockId != secondExpr.blockId)
			return false;
		SSABB& bb = this->bbs[firstExpr.blockId];
		auto firstit = bb.exprIds.begin();
		for (; firstit != bb.exprIds.end() && *firstit == firstExpr.id; firstit++);
		auto secit = bb.exprIds.begin();
		for (; secit != bb.exprIds.end() && *secit == secondExpr.id; secit++);
		if (firstit > secit || firstit == bb.exprIds.end() || secit == bb.exprIds.end())
			return false;
		if (firstit == secit)
			return true;
		SSAExpression& secexpr = expressions[*secit];
		for (auto it = firstit + 1; it != secit; it++) {
			for (SSAArgument& arg : secexpr.subExpressions) {
				if (arg.type == SSAArgType::eId && arg.ssaId == *it)
					return false;
			}
		}
		return true;
	}


	void SSARepresentation::print(Architecture* arch, int indent) {
		printIndent(indent);
		printf("Printing SSA-Gen DataSegment\n");
		printIndent(indent);
		printf("Number Of Expressions: %" PRId64 "\n", expressions.size());

		for (SSABB& bb : bbs) {
			printIndent(indent + 1);
			printf("Block bb Id: %d 0x%" PRIx64 " - 0x%" PRIx64 "\n", bb.id, bb.startaddr, bb.endaddr);

			printIndent(indent + 1);
			printf("InBlocks ");
			for (HId id : bb.inBlocks) printf("%d, ", id);
			printf("\n");

			printIndent(indent + 1);
			printf("OutBlocks ");
			for (HId id : bb.outBlocks) printf("%d, ", id);
			printf("\n");

			printIndent(indent + 1);
			printf("Number Of Ids: %" PRId64 "\n", bb.exprIds.size());
			for (HId id : bb.exprIds) {
				expressions[id].print(arch, indent + 2);
			}
		}
	}
	void SSARepresentation::printSimple(Architecture* arch, int indent) {
		printIndent(indent);
		printf("Printing SSA-Gen DataSegment\n");

		for (SSABB& bb : bbs) {
			printIndent(indent + 1);
			printf("Block bb Id: %d\n", bb.id);

			printIndent(indent + 1);
			for (HId id : bb.inBlocks) printf("%d, ", id);
			printf("-> Block -> ");

			for (HId id : bb.outBlocks) printf("%d, ", id);
			printf("\n");

			printIndent(indent + 1);
			printf("Number Of Ids: %" PRId64 "\n", bb.exprIds.size());
			for (HId id : bb.exprIds) {
				expressions[id].printSimple(arch, indent + 2);
			}
		}
	}
	uint64_t calculate_basearg_plus_offset(SSARepresentation* ssaRep, HId ssaId, int64_t* arithchange, SSAArgument* basearg) {
		SSAExpression& referencedExpr = ssaRep->expressions[ssaId];

		if (referencedExpr.type == SSAExprType::eOp && (referencedExpr.opType == SSAOpType::eAdd || referencedExpr.opType == SSAOpType::eSub)) {
			SSAArgument* idArg = nullptr;
			int64_t change = 0;
			for (size_t i = 0; i < referencedExpr.subExpressions.size(); ++i) {
				SSAArgument& onearg = referencedExpr.subExpressions[i];
				if (onearg.type == SSAArgType::eId) {
					if (idArg)
						return 0;
					idArg = &onearg;
					continue;
				}
				if (i == 0 || referencedExpr.opType == SSAOpType::eAdd) {
					if (onearg.type != SSAArgType::eValue)
						return 0;
					if (onearg.argtype == SSAType::eUInt)
						change += onearg.uval;
					else if (onearg.argtype == SSAType::eInt)
						change += onearg.sval;
					else
						return 0;
				}
				else {
					if (onearg.type != SSAArgType::eValue)
						return 0;
					if (onearg.argtype == SSAType::eUInt)
						change = onearg.uval;
					else if (onearg.argtype == SSAType::eInt)
						change = onearg.sval;
					else
						return 0;
				}
			}
			if (idArg && !idArg->offset) {
				*arithchange += change;
				*basearg = *idArg;
				return calculate_basearg_plus_offset(ssaRep, idArg->ssaId, arithchange, basearg) + 1;
			}
			else {
				return 0;
			}
		}
		return 0;
	}

	bool calculante_difference(SSARepresentation* ssaRep, HId firstid, HId secid, int64_t* change) {
		SSAArgument basearg1;
		int64_t change1 = 0;
		SSAArgument basearg2;
		int64_t change2 = 0;
		calculate_basearg_plus_offset(ssaRep, firstid, &change1, &basearg1);
		calculate_basearg_plus_offset(ssaRep, secid, &change2, &basearg2);
		if (basearg1 == basearg2) {
			*change = change2 - change1;
			return true;
		}
		return false;
	}

}
