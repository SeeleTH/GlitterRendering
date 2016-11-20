#pragma once
#include "../Macro/Macro.h"
#include "../Template/NNonCopyable.h"
#include "../Debug/NAssert.h"

class NStackAlloc : public NNonCopyable
{
public:
	//NStackAlloc();
	NStackAlloc(void* begin, void* end);
	NStackAlloc(void* begin, UINT size);

	//bool Valid() const;
	
	void Clear();
	//bool IsClear() const { return cursor == begin; }
	
	bool Align(UINT);

	inline UINT8* Alloc(UINT32 required, UINT32 reserve);
	inline UINT8* Alloc(UINT32 size);

	template<class T> T* Alloc();
	template<class T> T* Alloc(UINT32 count);

	void Unwind(const void* p);
	const UINT8* Mark() const;
	const UINT8* Begin() const;
	UINT32 Capacity() const { return end - begin; }

	void TransferOwnership(const void* from, const void* to) { N_ASSERT(from == owner); owner = to; }
	const void* Owner() const { return owner; }
private:
	UINT8* begin;
	UINT8* end;
	UINT8* cursor;
	const void* owner;
};

class NStackMeasure : public NNonCopyable
{
public:
	NStackMeasure() : cursor() {}
	void Clear();
	void Align(UINT);
	inline            UINT32 Alloc(UINT32 size);
	template<class T> UINT32 Alloc();
	template<class T> UINT32 Alloc(UINT32 count);
	UINT32 Bytes() const { return cursor; }
private:
	UINT32 cursor;
};

//------------------------------------------------------------------------------
//inline StackAlloc::StackAlloc()
//                      : begin(), end(), cursor(), owner() {}

inline NStackAlloc::NStackAlloc(void* begin, void* end)
	: begin((UINT8*)begin), end((UINT8*)end), cursor((UINT8*)begin), owner() {}

inline NStackAlloc::NStackAlloc(void* begin, UINT size)
	: begin((UINT8*)begin), end((UINT8*)begin + size), cursor((UINT8*)begin), owner() {}

/*inline bool StackAlloc::Valid() const
{
return begin != end;
}*/

inline void NStackAlloc::Clear()
{
	cursor = begin;
}

inline bool NStackAlloc::Align(UINT align)
{
	UINT8* null = (UINT8*)0;
	ptrdiff_t offset = cursor - null;
	offset = ((offset + align - 1) / align) * align;
	UINT8* newCursor = null + offset;
	if (newCursor <= end)
	{
		cursor = newCursor;
		return  true;
	}
	return false;
}

//bool IsClear() const { return cursor == begin; }

inline const UINT8* NStackAlloc::Begin() const { return begin; }

inline UINT8* NStackAlloc::Alloc(UINT32 required, UINT32 reserve)
{
	N_ASSERT(reserve >= required);
	if (cursor + reserve <= end)
	{
		UINT8* allocation = cursor;
		cursor += required;
		N_ASSERT(allocation);
		N_DEBUG(memset(allocation, 0xCDCDCDCD, required);)
			return allocation;
	}
	return 0;
}

inline UINT8* NStackAlloc::Alloc(UINT32 size)
{
	return Alloc(size, size);
}

template<class T> inline T* NStackAlloc::Alloc()
{
	return (T*)Alloc(sizeof(T));
}

template<class T> inline T* NStackAlloc::Alloc(UINT32 count)
{
	return (T*)Alloc(sizeof(T)*count);
}

inline void NStackAlloc::Unwind(const void* p)
{
	N_ASSERT(p >= begin && p<end && p <= cursor);
	N_DEBUG(UINT bytes = cursor - (UINT8*)p);
	cursor = (UINT8*)p;
	N_DEBUG(memset(cursor, 0xFEFEFEFE, bytes));
}

inline const UINT8* NStackAlloc::Mark() const
{
	return cursor;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

inline void NStackMeasure::Clear()
{
	cursor = 0;
}

inline void NStackMeasure::Align(UINT align)
{
	cursor = ((cursor + align - 1) / align) * align;
}

inline UINT32 NStackMeasure::Alloc(UINT32 size)
{
	UINT32 mark = cursor;
	cursor += size;
	return mark;
}

template<class T>
UINT32 NStackMeasure::Alloc()
{
	return Alloc(sizeof(T));
}

template<class T>
UINT32 NStackMeasure::Alloc(UINT32 count)
{
	return Alloc(sizeof(T)*count);
}

//------------------------------------------------------------------------------