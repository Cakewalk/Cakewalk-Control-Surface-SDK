#ifndef _MACROS_H
#define _MACROS_H


////////////////////////////////////////////////////////////////////////////////
//
// Useful Macros

// Handy macro to give the number of elements in an array.
#define ELEMENTS( array ) (sizeof(array)/sizeof(array[0]))

//----------------------------------------------------------------------------
// Utility macros
//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////

// This macro reduces clutter/typing when a class wants to prohibit copying
// via copy ctor or operator=.  Classes want to do this when such operations
// may easily fail (e.g. due to allocated memory).  Instead, they provide a
// special member like CreateClone().
#define CANNOT_COPY_CLASS( T )		\
private:										\
	T( const T& rhs );					\
	T& operator=( const T& rhs );


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


#if defined(_DEBUG) || defined(BETA)

static const TCHAR szErrLine[] = _T("%hs(%ld): Runtime error %08lx\r\n");

// Call a function, logging and returning if the result failed
#define CHECK_RET(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			TRACE( szErrLine, THIS_FILE, __LINE__, __hr ); \
			return __hr; \
		} } while (0)

// Call a function, logging and throwing the HRESULT as an exception if the result failed
#define CHECK_THROW(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			TRACE( szErrLine, THIS_FILE, __LINE__, __hr ); \
			throw __hr; \
		} } while (0)

// Call a function, logging if the result failed
#define CHECK_LOG(_fn) \
	do{	HRESULT __hr = _fn; \
		if (FAILED(__hr)) { \
			TRACE( szErrLine, THIS_FILE, __LINE__, __hr ); \
		} } while (0)

// Compare Ref Count of an IUnk* to _Rc and log if it is greater
#define TRACE_REFCNT(_I,_rc) \
	do{	_I->AddRef(); \
		long lrc = _I->Release(); \
		if ( lrc > _rc ) { \
			TRACE( "%hs(%ld): IUnk %08lx RefCnt=%ld\n", THIS_FILE, __LINE__, _I, lrc ); \
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

#endif //_MACROS_H

