#pragma once



#include "../../FunctionAnalyzer.h"
#include "../../General.h"

#include "../../Architecture.h"

namespace holoavr {

	using namespace holodec;

	enum AVR_INSTRUCTIONS {
		AVR_INSTR_ADC = 0,
		AVR_INSTR_ADD,
		AVR_INSTR_ADIW,
		AVR_INSTR_AND,
		AVR_INSTR_ANDI,
		AVR_INSTR_ASR,
		AVR_INSTR_BCLR,
		AVR_INSTR_BLD,
		AVR_INSTR_BRBC,
		AVR_INSTR_BRBS,
		AVR_INSTR_BRCC,
		AVR_INSTR_BRCS,
		AVR_INSTR_BREAK,
		AVR_INSTR_BREQ,
		AVR_INSTR_BRGE,
		AVR_INSTR_BRHC,
		AVR_INSTR_BRHS,
		AVR_INSTR_BRID,
		AVR_INSTR_BRIE,
		AVR_INSTR_BRLO,
		AVR_INSTR_BRLT,
		AVR_INSTR_BRMI,
		AVR_INSTR_BRNE,
		AVR_INSTR_BRPL,
		AVR_INSTR_BRSH,
		AVR_INSTR_BRTC,
		AVR_INSTR_BRTS,
		AVR_INSTR_BRVC,
		AVR_INSTR_BRVS,
		AVR_INSTR_BSET,
		AVR_INSTR_BST,
		AVR_INSTR_CALL,
		AVR_INSTR_CBI,
		AVR_INSTR_CBR,
		AVR_INSTR_CLC,
		AVR_INSTR_CLH,
		AVR_INSTR_CLI,
		AVR_INSTR_CLN,
		AVR_INSTR_CLR,
		AVR_INSTR_CLS,
		AVR_INSTR_CLT,
		AVR_INSTR_CLV,
		AVR_INSTR_CLZ,
		AVR_INSTR_COM,
		AVR_INSTR_CP,
		AVR_INSTR_CPC,
		AVR_INSTR_CPI,
		AVR_INSTR_CPSE,
		AVR_INSTR_DEC,
		AVR_INSTR_EICALL,
		AVR_INSTR_EIJMP,
		AVR_INSTR_ELPM,
		AVR_INSTR_EOR,
		AVR_INSTR_FMUL,
		AVR_INSTR_FMULS,
		AVR_INSTR_FMULSU,
		AVR_INSTR_ICALL,
		AVR_INSTR_IJMP,
		AVR_INSTR_IN,
		AVR_INSTR_INC,
		AVR_INSTR_JMP,
		AVR_INSTR_LAT,
		AVR_INSTR_LAS,
		AVR_INSTR_LAC,
		AVR_INSTR_LD,
		AVR_INSTR_LDD,
		AVR_INSTR_LDI,
		AVR_INSTR_LDS,
		AVR_INSTR_LPM,
		AVR_INSTR_LSL,
		AVR_INSTR_LSR,
		AVR_INSTR_MOV,
		AVR_INSTR_MOVW,
		AVR_INSTR_MUL,
		AVR_INSTR_MULS,
		AVR_INSTR_MULSU,
		AVR_INSTR_NEG,
		AVR_INSTR_NOP,
		AVR_INSTR_OR,
		AVR_INSTR_ORI,
		AVR_INSTR_OUT,
		AVR_INSTR_POP,
		AVR_INSTR_PUSH,
		AVR_INSTR_RCALL,
		AVR_INSTR_RET,
		AVR_INSTR_RETI,
		AVR_INSTR_RJMP,
		AVR_INSTR_ROL,
		AVR_INSTR_ROR,
		AVR_INSTR_SBC,
		AVR_INSTR_SBCI,
		AVR_INSTR_SBI,
		AVR_INSTR_SBIC,
		AVR_INSTR_SBIS,
		AVR_INSTR_SBR,
		AVR_INSTR_SBRC,
		AVR_INSTR_SBRS,
		AVR_INSTR_SEC,
		AVR_INSTR_SEH,
		AVR_INSTR_SEI,
		AVR_INSTR_SEN,
		AVR_INSTR_SER,
		AVR_INSTR_SES,
		AVR_INSTR_SET,
		AVR_INSTR_SEV,
		AVR_INSTR_SEZ,
		AVR_INSTR_SLEEP,
		AVR_INSTR_SPM,
		AVR_INSTR_ST,
		AVR_INSTR_STD,
		AVR_INSTR_STS,
		AVR_INSTR_SUB,
		AVR_INSTR_SUBI,
		AVR_INSTR_SWAP,
		AVR_INSTR_TST,
		AVR_INSTR_WDR,
		AVR_INSTR_XCH,


		AVR_INSTR_INC_PTR = 0x1000,
		AVR_INSTR_DEC_PTR = 0x2000,
	};

	extern Architecture avrarchitecture;

	class AVRFunctionAnalyzer : public FunctionAnalyzer {
	public:
		AVRFunctionAnalyzer(Architecture* arch);
		virtual ~AVRFunctionAnalyzer();

		virtual bool canAnalyze(Binary* binary);

		virtual bool init(Binary* binary);
		virtual bool terminate();

		virtual bool analyzeInsts(size_t addr);

	};
}