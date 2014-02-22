

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Thu Jul 20 14:53:10 2006
 */
/* Compiler settings for .\MotorMix.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __MotorMix_h__
#define __MotorMix_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICSProperties_FWD_DEFINED__
#define __ICSProperties_FWD_DEFINED__
typedef interface ICSProperties ICSProperties;
#endif 	/* __ICSProperties_FWD_DEFINED__ */


#ifndef __ControlSurfacePropPage_FWD_DEFINED__
#define __ControlSurfacePropPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class ControlSurfacePropPage ControlSurfacePropPage;
#else
typedef struct ControlSurfacePropPage ControlSurfacePropPage;
#endif /* __cplusplus */

#endif 	/* __ControlSurfacePropPage_FWD_DEFINED__ */


#ifndef __ControlSurface_FWD_DEFINED__
#define __ControlSurface_FWD_DEFINED__

#ifdef __cplusplus
typedef class ControlSurface ControlSurface;
#else
typedef struct ControlSurface ControlSurface;
#endif /* __cplusplus */

#endif 	/* __ControlSurface_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"
#include "ControlSurface.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ICSProperties_INTERFACE_DEFINED__
#define __ICSProperties_INTERFACE_DEFINED__

/* interface ICSProperties */
/* [unique][helpstring][uuid][local][object] */ 


EXTERN_C const IID IID_ICSProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("95FEB8DD-DD64-4460-AB31-CA5610BDCC33")
    ICSProperties : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct ICSPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICSProperties * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICSProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICSProperties * This);
        
        END_INTERFACE
    } ICSPropertiesVtbl;

    interface ICSProperties
    {
        CONST_VTBL struct ICSPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICSProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICSProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICSProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICSProperties_INTERFACE_DEFINED__ */



#ifndef __SURFACEFRAMEWORKLib_LIBRARY_DEFINED__
#define __SURFACEFRAMEWORKLib_LIBRARY_DEFINED__

/* library SURFACEFRAMEWORKLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_SURFACEFRAMEWORKLib;

EXTERN_C const CLSID CLSID_ControlSurfacePropPage;

#ifdef __cplusplus

class DECLSPEC_UUID("75D0A649-84EE-479c-8786-0AA7D7738C28")
ControlSurfacePropPage;
#endif

EXTERN_C const CLSID CLSID_ControlSurface;

#ifdef __cplusplus

class DECLSPEC_UUID("5E604989-B1F0-4412-B9D1-64A54B2277D0")
ControlSurface;
#endif
#endif /* __SURFACEFRAMEWORKLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


