#pragma once

#ifdef UNICODE
#define _tcssprintf wsprintf
#define tcsplitpath _wsplitpath
#else
#define _tcssprintf sprintf
#define tcsplitpath _splitpath
#endif

typedef unsigned char							UCHAR;

typedef signed __int8                           INT8;
typedef signed __int16                          INT16;
typedef signed __int32                          INT32;
typedef signed __int64                          INT64;
typedef unsigned int                            UINT;
typedef unsigned __int8                         UINT8;
typedef unsigned __int16                        UINT16;
typedef unsigned __int32                        UINT32;
typedef unsigned __int64                        UINT64;
//typedef UINT64									SIZE_T;

#if defined(_WIN64)
typedef unsigned __int64                        UINTPTR;
#else
typedef unsigned long __w64                     UINTPTR;
#endif  // #if defined( _WIN64 )

typedef INT32                                 BOOL;

typedef UINT8                                 UTF8;
typedef UINT16                                UTF16;
typedef UINT32                                UTF32;

typedef wchar_t                                 EUTFX;

#define SUTF_INVALID                          ~static_cast<NSEUINT32>(0)
#define STD_MAX_INT32                         2147483647L
#define STD_MAX_UINT32                        0xFFFFFFFF
#define STD_MAX_INT64                         9223372036854775807LL
#define STD_MAX_UINT64                        0xFFFFFFFFFFFFFFFFULL

#if defined(_DEBUG)
#   define N_NEW  new//new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define N_NEW  new
#endif

#if !defined(N_DELETE)
#define N_DELETE(x) if(x) delete x; x=NULL;
#endif

#if !defined(N_DELETE_SIZE)
#define N_DELETE_SIZE(x,y) if(x) delete(x,y); x=NULL;
#endif

#if !defined(N_DELETE_ARRAY)
#define N_DELETE_ARRAY(x) if (x) delete [] x; x=NULL; 
#endif

#if !defined(N_RELEASE)
#define N_RELEASE(x) if(x) x->Release(); x=NULL;
#endif

// ============================
// Graphics - Begin
// ============================

#define N_G_DIRECTX_11
//#define N_G_OPENGL_4
//#define N_G_OPENGL_ES
//#define N_G_CUDA

#ifdef N_G_DIRECTX_11

#define N_G_MAX_TEXTURE_UNITS 10
#define N_G_MAX_RENDERTARGETVIEWS 7
#define N_G_MAX_TEXTURERESOURCEVIEWS 5
#define N_G_MAX_UNORDEREDRESOURCEVIEWS 5
#define N_G_MAX_VIEWPORTS 5

#else

//...

#endif

// ============================
// Graphics - End
// ============================