#pragma once
#include "../Macro/Macro.h"

template <typename C>
static int NClassCategoryGenerateTypeID() {
	static UINT32 TypeID;
	return ++TypeID;
}

template <typename C, typename T>
class NClassIDGenerator {
public:
	static const int GetTypeID() {
		static const UINT32 TypeID = NClassCategoryGenerateTypeID<C>();
		return TypeID;
	};
};

#define CLASS_ID_GEN_FUNC(__BASECLASS__,__CLASS__) \
	virtual UINT32 GetTypeID() { return NClassIDGenerator<__BASECLASS__ ,__CLASS__>::GetTypeID(); }