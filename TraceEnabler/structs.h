#ifndef _STRUCTS_H_
#define _STRUCTS_H_

// ripped out structs from Tork

typedef unsigned int U32;
typedef float F32;
typedef signed int S32;
typedef const char* StringTableEntry;

typedef const char* (*StringCallback)(DWORD* obj, int argc, const char* argv[]);
typedef int(*IntCallback)(DWORD* obj, int argc, const char* argv[]);
typedef float(*FloatCallback)(DWORD* obj, int argc, const char* argv[]);
typedef void(*VoidCallback)(DWORD* obj, int argc, const char* argv[]);
typedef bool(*BoolCallback)(DWORD* obj, int argc, const char* argv[]);

#include "tvector.h"

struct Namespace
{
	const char* mName;
	const char* mPackage;

	Namespace* mParent;
	Namespace* mNext;
	void* mClassRep;
	U32 mRefCountToParent;
	struct Entry
	{
		enum
		{
			GroupMarker = -3,
			OverloadMarker = -2,
			InvalidFunctionType = -1,
			ScriptFunctionType,
			StringCallbackType,
			IntCallbackType,
			FloatCallbackType,
			VoidCallbackType,
			BoolCallbackType
		};

		Namespace* mNamespace;
		//char _padding1[4];
		Entry* mNext;
		const char* mFunctionName;
		S32 mType;
		S32 mMinArgs;
		S32 mMaxArgs;
		const char* mUsage;
		const char* mPackage;
		void* mCode; // CodeBlock *mCode;
		U32 mFunctionOffset;
		union {
			StringCallback mStringCallbackFunc;
			IntCallback mIntCallbackFunc;
			VoidCallback mVoidCallbackFunc;
			FloatCallback mFloatCallbackFunc;
			BoolCallback mBoolCallbackFunc;
			const char* mGroupName;
		} cb;
	};
	Entry* mEntryList;
	Entry** mHashTable;
	U32 mHashSize;
	U32 mHashSequence;  ///< @note The hash sequence is used by the autodoc console facility
						///        as a means of testing reference state.
	char* lastUsage;
};

static Namespace* mGlobalNamespace;

struct CodeBlock
{
	const char* name;
	int unk_1;
	void* globalStrings;
	void* functionStrings;
	void* globalFloats;
	void* functionFloats;
	int codeSize;
	int unk_2;
	char* code;
	int refCount;
	int lineBreakPairCount;
	int* lineBreakPairs;
	int breakListSize;
	int* breakList;
	CodeBlock* nextFile;
	const char* mRoot;
};

struct Dictionary
{
	struct Entry {
		enum
		{
			TypeInternalInt = -3,
			TypeInternalFloat = -2,
			TypeInternalString = -1,
		};

		const char* name;
		Entry* next;
		S32 type;
		char* sval;
		U32 ival; // doubles as strlen
		F32 fval;
		U32 bufferLen;
		void* dataPtr;
	};

	struct HashTableData
	{
		int size;
		int count;
		Entry** data;
		void* owner;
	};

	HashTableData table;
	const char* scopeName;
	const char* scopeNamespace;
	CodeBlock* block;
	int ip;
};

struct ExprEvalState
{
	void* thisObject;
	Dictionary::Entry* currentVariable;
	bool traceOn;
	Dictionary globalVars;
	Vector<Dictionary> stack;
};

static ExprEvalState* gEvalState;

struct DataChunker
{
	struct DataBlock
	{
		DataBlock* next;
		char* data;
		int curIndex;
	};

	DataBlock* curBlock;
	signed int chunkSize;
};

struct StringStack
{
	char* mBuffer;
	int mBufferSize;
	const char* mArgv[20];
	int mFrameOffsets[1024];
	int mStartOffsets[1024];
	int mNumFrames;
	int mArgc;
	int mStart;
	int mLen;
	int mStartStackSize;
	int mFunctionOffset;
	int mArgBufferSize;
	char* mArgBuffer;
};

struct StringTable {
	struct Node {
		char* val;
		Node* next;
	};

	Node** buckets;
	U32 numBuckets;
	U32 itemCount;
	DataChunker memPool;
};

struct SimObject
{
	void* vtable;
	const char* objectName;
	SimObject* nextNameObject;
	SimObject* nextManagerNameObject;
	SimObject* nextIdObject;
	void* mGroup;
	unsigned int mFlags;
	void* mNotifyList;
	unsigned int id;
	Namespace* mNamespace;
	unsigned int mTypeMask;
	void* filler;
	void* mFieldDictionary;
};

#endif