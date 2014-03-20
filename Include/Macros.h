#ifndef _MACROS_H
#define _MACROS_H


////////////////////////////////////////////////////////////////////////////////
//
// Useful Macros

// Handy macro to give the number of elements in an array.
#define ELEMENTS( array ) (sizeof(array)/sizeof(array[0]))


/////////////////////////////////////////////////////////////////////////////

// These are defined in the SDK virtkey appendix, but not #define'd in
// WINDOWS.H [2-22-2000 they are now -- in the Platform SDK]
#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS	(0xBB)	// white + key, vs grey VK_ADD (0x6B)
#endif

#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS	(0xBD)	// white - key, vs grey VK_SUBTRACT (0x6D)
#endif


/////////////////////////////////////////////////////////////////////////////

// This macro reduces clutter/typing when a class wants to prohibit copying
// via copy ctor or operator=.  Classes want to do this when such operations
// may easily fail (e.g. due to allocated memory).  Instead, they provide a
// special member like CreateClone().
#define CANNOT_COPY_CLASS( T )		\
private:										\
	T( const T& rhs );					\
	T& operator=( const T& rhs );

// This macro reduces clutter/typing when a class wants to prohibit copying
// via copy ctor or operator=, and, prohibit using new or delete to create
// or destroy objects.  Classes want to do this when such operations may
// easily fail (e.g. due to allocated memory).  Instead, they provide
// special members like Create(), Destroy(), and CreateClone().
#define CANNOT_CONSTRUCT_CLASS( T )	\
private:										\
	T();										\
	~T();										\
	T( const T& rhs );					\
	T& operator=( const T& rhs );

// This macro reduces clutter/typing when a class wants to prohibit comparison
// via things like operator==(). Doing so is a good idea when the class doesn't
// provide specific definitions for these, and any compiler-generated solutions
// might be surprising to users of the class.
#define CANNOT_COMPARE_CLASS( T )			\
private:												\
	BOOL operator==( const T& rhs ) const;	\
	BOOL operator!=( const T& rhs ) const;	\
	BOOL operator<( const T& rhs ) const;	\
	BOOL operator>( const T& rhs ) const;	\
	BOOL operator<=( const T& rhs ) const;	\
	BOOL operator>=( const T& rhs ) const;

/////////////////////////////////////////////////////////////////////////////
// Helper macros for declaring and defining custom new/delete operators.
// If a class requires these operators, add one line to its declaration:
// 	class CFoo
//		{ ...
//			DECLARE_CUSTOM_NEW;
//		... };
// And then add one line to its implementation (.CPP) file:
//		DEFINE_CUSTOM_NEW(CFoo);

#ifdef _DEBUG

#define DECLARE_CUSTOM_NEW \
	public:	void* operator new( size_t size, const char* pcszFile, int nLine ); \
				void operator delete( void* p );

#define DEFINE_CUSTOM_NEW(_class) \
	void* _class::operator new( size_t size, const char* pcszFile, int nLine ) { return ::operator new( size, pcszFile, nLine ); } \
	void  _class::operator delete( void* p ) { ::delete[] p; }

#else

#define DECLARE_CUSTOM_NEW \
	public:	void* operator new( size_t size ); \
				void operator delete( void* p );

#define DEFINE_CUSTOM_NEW(_class) \
	void* _class::operator new( size_t size ) { return ::new char [ size ]; } \
	void  _class::operator delete( void* p ) { ::delete[] p; }

#endif

//----------------------------------------------------------------------------
// Utility macros
//----------------------------------------------------------------------------

// Check if a numeric value falls between two values.  Done as a macro to
// support all numeric types.
//
#define IsBetween(_min, _val, _max) ( (_min <= _val) && (_val <= _max) )

// Call an MMYSTEM function, converting errors to HRESULTs
//
#define MM_CHECK(_fn) { UINT uErr = _fn; if (uErr) CHECK(MM_HRESULT(uErr)); }

// Handy assert/trace macros
//
#define ASSERTFAIL()	ASSERT( FALSE ) 
#define VERIFYFAIL()	VERIFY( FALSE )
#define ASSERTNOT(x)	ASSERT( !(x) )
#define VERIFYNOT(x)	VERIFY( !(x) )

////////////////////////////////////////////////////////////////////////////////
// Keep track of newly written but untested code

#if _DEBUG
	static const TCHAR szUntestedCodeMsg[] = _T("%hs(%d): Untested code!\r\n");
	#define UNTESTED_CODE() WARN( szUntestedCodeMsg, THIS_FILE, __LINE__ )
#elif BETA
	#define UNTESTED_CODE() ASSERT(FALSE)
#else
	#define UNTESTED_CODE() Untested_Code // deliberately break the build
#endif

////////////////////////////////////////////////////////////////////////////////
// Memory allocation macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Release a pointer and set it to null

#undef SAFE_RELEASE
#define SAFE_RELEASE(_p) { if (_p) (_p)->Release(); (_p) = NULL; }


////////////////////////////////////////////////////////////////////////////////
// Delete a pointer and set it to null

#undef SAFE_DELETE
#define SAFE_DELETE(_p) { if (_p) delete (_p); (_p) = NULL; }

#undef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(_p) { if (_p) delete [] (_p); (_p) = NULL; }

#define SAFE_FREE(p)				{ if (p) free(p), p = 0; }

////////////////////////////////////////////////////////////////////////////////

#undef CHECK

#if defined(_DEBUG) || defined(BETA)

static const TCHAR szErrLine[] = _T("%hs(%ld): Runtime error %08lx\r\n");

// Call a function, logging and returning if the result failed
#define CHECK_RET(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			WARN( szErrLine, THIS_FILE, __LINE__, __hr ); \
			return __hr; \
		} } while (0)

// Call a function, logging and throwing the HRESULT as an exception if the result failed
#define CHECK_THROW(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			WARN( szErrLine, THIS_FILE, __LINE__, __hr ); \
			throw __hr; \
		} } while (0)

// Call a function, logging if the result failed
#define CHECK_LOG(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			WARN( szErrLine, THIS_FILE, __LINE__, __hr ); \
		} } while (0)

// Compare Ref Count of an IUnk* to _Rc and log if it is greater
#define TRACE_REFCNT(_I,_rc) \
	do{	_I->AddRef(); \
		long lrc = _I->Release(); \
		if ( lrc > _rc ) { \
			WARN( "%hs(%ld): IUnk %08lx RefCnt=%ld\n", THIS_FILE, __LINE__, _I, lrc ); \
		} } while (0)

#else	// _DEBUG

#define CHECK_RET(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			return __hr; } } while (0)

#define CHECK_THROW(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			throw __hr; } } while (0)

#define CHECK_LOG(_fn) _fn

#define TRACE_REFCNT(_I,_rc) void(0)

#endif // _DEBUG

////////////////////////////////////////////////////////////////////////////////
// brought in from SRC2002 7/16/04 (JEP)
// Call a function, logging and returning hr if the result is FAILURE or S_FALSE
#define CHECK_FALSE(_fn) \
	do {	HRESULT __hr = _fn; \
       if (FAILED(__hr) || (S_FALSE == __hr)) { \
          return __hr; } } while (0)


////////////////////////////////////////////////////////////////////////////////
// useful thing to put a bookmark in src code
// to use it just do this:
//
// #pragma message(BOOKMARK "Little compiler note to remind me of something")
// 
// when you run the compiler you will see the message like this:
// c:\somepath\myfile.cpp(123) : Little compiler note to remind me of something

// the __LINE__ needs two levels of indirection to expand correctly.
#ifdef _MSC_VER //Detect Microsoft C++ compiler
	#define IND2(x)				#x
	#define LINE_IND(x)			IND2(x) 			
	#define BOOKMARK __FILE__ "("LINE_IND(__LINE__)") :"
#else
	// otherwise use some other method to output file and line info
	// NOP
	#define BOOKMARK
#endif

#endif //_MACROS_H

////////////////////////////////////////////////////////////////////////////////
// 
// this is way faster than CDC::FillSolidRect.  
// Microsoft uses ExtTextOut in some of their OnEraseBkgnd() routines.
// this version takes an HDC
#define FastFillSolidRectHDC( __hDC, __prc, __cr ) \
 COLORREF __crOld = ::SetBkColor( __hDC, __cr ); \
 ::ExtTextOut( __hDC, __prc->left, __prc->top, ETO_OPAQUE, __prc, _T(""), 0, NULL ); \
 ::SetBkColor( __hDC, __crOld ); 
// version that takes a CDC*
#define FastFillSolidRectCDC( __pCDC, __prc, __cr ) \
 COLORREF __crOld = __pCDC->SetBkColor( __cr ); \
 __pCDC->ExtTextOut( __prc->left, __prc->top, ETO_OPAQUE, __prc, _T(""), 0, NULL ); \
 __pCDC->SetBkColor( __crOld ); 

////////////////////////////////////////////////////////////////////////////////
// 
// Some Set/Get WindowLong functions: Conditional on _MSC_VER  

#if _MSC_VER >= 1400 
	#define GetWindowLongUser( __hWnd ) \
			GetWindowLongPtr( __hWnd, DWLP_USER )
	#define SetWindowLongUser( __hWnd, __val ) \
			SetWindowLongPtr( __hWnd, DWLP_USER, (LONG_PTR)__val )	
	#define GetWindowLongRslt( __hWnd ) \
			GetWindowLongPtr( __hWnd, DWLP_MSGRESULT )
	#define SetWindowLongRslt( __hWnd, __val ) \
			SetWindowLongPtr( __hWnd, DWLP_MSGRESULT, (LONG_PTR)__val )	
#else
	#define GetWindowLongUser( hWnd ) \
			GetWindowLong( hWnd, DWL_USER )
	#define SetWindowLongUser( __hWnd, __val ) \
			SetWindowLong( __hWnd, DWL_USER, (long)__val )	
	#define GetWindowLongRslt( hWnd ) \
			GetWindowLong( hWnd, DWL_MSGRESULT )
	#define SetWindowLongRslt( __hWnd, __val ) \
			SetWindowLong( __hWnd, DWL_MSGRESULT, (long)__val )	
#endif

#if _MSC_VER < 1300	// Needed macros not defined in VC98
	#define LongToPtr( l )   ((VOID *)(LONG_PTR)((long)l))
	#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#endif

