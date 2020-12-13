#include <cstdlib>
#include "Torque.h"
#include <polyhook2/Detour/x86Detour.hpp>
#include <polyhook2/CapstoneDisassembler.hpp>
#include <iostream>
#include <fstream>

PLH::x86Detour* execDetour;
uint64_t execTramp = NULL;
CodeBlockExecFn CodeBlockExec = NULL;

PLH::x86Detour* SimInitDetour = NULL;
uint64_t SimInitTramp = NULL;
SimInitFn SimInit = NULL;

bool enableTrace = false;

// this is really ugly, but it works
void printTrace(bool entering, const char* retVal, const char* functionName, const char* packageName, Namespace* ns, int argc, const char** argv) {
	if (!enableTrace) return;

	// there might be some added benefit from having the filestream open, but whatever
	std::ofstream traceFile;
	traceFile.open("trace.txt", std::ios::out | std::ios::app);
	traceFile << entering ? "Entering " : "Leaving ";

	if (packageName) {
		traceFile << packageName;
	}
	if (ns && ns->mName) {
		traceFile << ns->mName << "::" << functionName;
	}
	else if (functionName) {
		// edge case we run into
		traceFile << functionName;
	}
	traceFile << "(";
	if (entering) {
		for (int i = 1; i < argc; i++) {
			traceFile << "\"" << argv[i] << "\"";
			if (i != argc - 1)
				traceFile << ", ";
		}
		traceFile << ")";
	}
	else {
		traceFile << ") - return ";
		traceFile << retVal;
	}
	traceFile << "\n";
	traceFile.close();
}

// since we can't technically declare a __thiscall since we're not in a class
// we have to use the next best thing (a __fastcall) and ignore the edx register
const char* __fastcall h_CodeBlockExec(CodeBlock* this_, int edx, int ip, const char* functionName, Namespace* ns, int argc, const char** argv, bool noCalls, const char* packageName, int setFrame) {
	printTrace(true, NULL, functionName, packageName, ns, argc, argv);
	const char* ret = PLH::FnCast(execTramp, CodeBlockExec)(this_, ip, functionName, ns, argc, argv, noCalls, packageName, setFrame);
	printTrace(false, ret, functionName, packageName, ns, argc, argv);
	return ret;
}

void h_SimInit() {
	if (!InitTorqueStuff()) // if we can't initialize our engine (and we should), just stop the entire thing
		return;

	PLH::CapstoneDisassembler dis(PLH::Mode::x86);
	CodeBlockExec = (CodeBlockExecFn)(ImageBase + 0x2FD20);

	Printf("TraceEnabler v0.1 initialized, use $EnableHackTrace to enable the trace");

	if (remove("trace.txt") != 0) {
		Printf("could not find previous trace (this is OK!)");
	}
	else {
		Printf("removed previous trace file");
	}

	execDetour = new PLH::x86Detour((char*)CodeBlockExec, (char*)&h_CodeBlockExec, &execTramp, dis);
	if (execDetour->hook()) {
		Printf("Was able to hook CodeBlock::exec successfully..");
		AddVariable("EnableHackTrace", &enableTrace);
	}
	else {
		Printf("Was not able to hook CodeBlock::exec! DLL will not function.");
	}

	return PLH::FnCast(SimInitTramp, h_SimInit)();
}


DWORD WINAPI Init(LPVOID args)
{
	InitScanner("Blockland.exe"); // get image base and whatnot

	PLH::CapstoneDisassembler dis(PLH::Mode::x86);
	SimInit = (SimInitFn)(ImageBase + 0x4C890);
	SimInitDetour = new PLH::x86Detour((char*)SimInit, (char*)&h_SimInit, &SimInitTramp, dis);

	return SimInitDetour->hook();
}

//Entry point
int WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Init, NULL, 0, NULL);
	}

	return true;
}

extern "C" void __declspec(dllexport) __cdecl init() {} // we need a stub function that will kickstart the DLL