

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Thu May 11 13:42:12 2006
 */
/* Compiler settings for .\TTSLogger.idl:
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

#ifndef __TTSLogger_h__
#define __TTSLogger_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ITTSLog_FWD_DEFINED__
#define __ITTSLog_FWD_DEFINED__
typedef interface ITTSLog ITTSLog;
#endif 	/* __ITTSLog_FWD_DEFINED__ */


#ifndef __ITTSLogClientAdvise_FWD_DEFINED__
#define __ITTSLogClientAdvise_FWD_DEFINED__
typedef interface ITTSLogClientAdvise ITTSLogClientAdvise;
#endif 	/* __ITTSLogClientAdvise_FWD_DEFINED__ */


#ifndef __ITTSLogServer_FWD_DEFINED__
#define __ITTSLogServer_FWD_DEFINED__
typedef interface ITTSLogServer ITTSLogServer;
#endif 	/* __ITTSLogServer_FWD_DEFINED__ */


#ifndef __TTSLog_FWD_DEFINED__
#define __TTSLog_FWD_DEFINED__

#ifdef __cplusplus
typedef class TTSLog TTSLog;
#else
typedef struct TTSLog TTSLog;
#endif /* __cplusplus */

#endif 	/* __TTSLog_FWD_DEFINED__ */


#ifndef __TTSLogServer_FWD_DEFINED__
#define __TTSLogServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class TTSLogServer TTSLogServer;
#else
typedef struct TTSLogServer TTSLogServer;
#endif /* __cplusplus */

#endif 	/* __TTSLogServer_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_TTSLogger_0000 */
/* [local] */ 


enum LOG_PRESET_CHANNELS
    {	LOG_CHANNEL_ALL	= 0
    } ;

enum LOG_LEVEL
    {	F_LOGLEVEL_NONE	= 0,
	F_LOGLEVEL_ERR	= 1,
	F_LOGLEVEL_WARN	= 2,
	F_LOGLEVEL_TRACE	= 4,
	F_LOGLEVEL_FIXME	= 8,
	F_LOGLEVEL_ALL	= 15
    } ;

enum LOG_OPTIONS
    {	F_LOGOPTION_NONE	= 0,
	F_LOGOPTION_DEBUGTRACE	= 1,
	F_LOGOPTION_PRINTHEADER	= 2,
	F_LOGOPTION_PRINTFOOTER	= 4,
	F_LOGOPTION_BUFFERED	= 8
    } ;

enum LOG_OPEN_MODE
    {	LOGOPENMODE_APPEND	= 0,
	LOGOPENMODE_OVERWRITE	= 1,
	LOGOPENMODE_PERSESSION	= 2
    } ;

enum LOG_STATUS
    {	LOGSTAT_STOPPED	= 0,
	LOGSTAT_STARTED	= 1,
	LOGSTAT_PAUSED	= 2,
	LOGSTAT_ERROR	= 3
    } ;


extern RPC_IF_HANDLE __MIDL_itf_TTSLogger_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_TTSLogger_0000_v0_0_s_ifspec;

#ifndef __ITTSLog_INTERFACE_DEFINED__
#define __ITTSLog_INTERFACE_DEFINED__

/* interface ITTSLog */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ITTSLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35A0DBB9-1E74-453D-8DC1-130AF6B3C98A")
    ITTSLog : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFileName( 
            LPOLESTR *ppwszName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Open( 
            LPCOLESTR pwszName,
            int nOptions) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsOpen( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddChannel( 
            LPCOLESTR pwszChanName,
            int *phChan) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveChannel( 
            int hChan) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveAllChannels( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsChannelActive( 
            int hChan,
            int nLevel) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetLoggingLevel( 
            int hChan,
            int nLevel) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLoggingLevel( 
            int hChan,
            int *pnLevel) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetChannelName( 
            int hChan,
            LPOLESTR *ppwszName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OutputText( 
            int hChan,
            int nLogLevel,
            LPCOLESTR pwszText) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OutputHexDump( 
            int hChan,
            int nLogLevel,
            const unsigned char *pBuffer,
            unsigned int size,
            int nCharsPerLine,
            LPCOLESTR pcszLinePrefix) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindChannel( 
            LPCOLESTR pwszChanName,
            int *phChan) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE LoadSettings( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SaveSettings( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLoggedItemCount( 
            int nLogLevel,
            int *pnCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITTSLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTSLog * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTSLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetFileName )( 
            ITTSLog * This,
            LPOLESTR *ppwszName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Open )( 
            ITTSLog * This,
            LPCOLESTR pwszName,
            int nOptions);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Close )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsOpen )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *AddChannel )( 
            ITTSLog * This,
            LPCOLESTR pwszChanName,
            int *phChan);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RemoveChannel )( 
            ITTSLog * This,
            int hChan);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RemoveAllChannels )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsChannelActive )( 
            ITTSLog * This,
            int hChan,
            int nLevel);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SetLoggingLevel )( 
            ITTSLog * This,
            int hChan,
            int nLevel);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetLoggingLevel )( 
            ITTSLog * This,
            int hChan,
            int *pnLevel);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetChannelName )( 
            ITTSLog * This,
            int hChan,
            LPOLESTR *ppwszName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *OutputText )( 
            ITTSLog * This,
            int hChan,
            int nLogLevel,
            LPCOLESTR pwszText);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *OutputHexDump )( 
            ITTSLog * This,
            int hChan,
            int nLogLevel,
            const unsigned char *pBuffer,
            unsigned int size,
            int nCharsPerLine,
            LPCOLESTR pcszLinePrefix);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *FindChannel )( 
            ITTSLog * This,
            LPCOLESTR pwszChanName,
            int *phChan);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *LoadSettings )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *SaveSettings )( 
            ITTSLog * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetLoggedItemCount )( 
            ITTSLog * This,
            int nLogLevel,
            int *pnCount);
        
        END_INTERFACE
    } ITTSLogVtbl;

    interface ITTSLog
    {
        CONST_VTBL struct ITTSLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTSLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTSLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTSLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTSLog_GetFileName(This,ppwszName)	\
    (This)->lpVtbl -> GetFileName(This,ppwszName)

#define ITTSLog_Open(This,pwszName,nOptions)	\
    (This)->lpVtbl -> Open(This,pwszName,nOptions)

#define ITTSLog_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ITTSLog_IsOpen(This)	\
    (This)->lpVtbl -> IsOpen(This)

#define ITTSLog_AddChannel(This,pwszChanName,phChan)	\
    (This)->lpVtbl -> AddChannel(This,pwszChanName,phChan)

#define ITTSLog_RemoveChannel(This,hChan)	\
    (This)->lpVtbl -> RemoveChannel(This,hChan)

#define ITTSLog_RemoveAllChannels(This)	\
    (This)->lpVtbl -> RemoveAllChannels(This)

#define ITTSLog_IsChannelActive(This,hChan,nLevel)	\
    (This)->lpVtbl -> IsChannelActive(This,hChan,nLevel)

#define ITTSLog_SetLoggingLevel(This,hChan,nLevel)	\
    (This)->lpVtbl -> SetLoggingLevel(This,hChan,nLevel)

#define ITTSLog_GetLoggingLevel(This,hChan,pnLevel)	\
    (This)->lpVtbl -> GetLoggingLevel(This,hChan,pnLevel)

#define ITTSLog_GetChannelName(This,hChan,ppwszName)	\
    (This)->lpVtbl -> GetChannelName(This,hChan,ppwszName)

#define ITTSLog_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define ITTSLog_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define ITTSLog_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define ITTSLog_OutputText(This,hChan,nLogLevel,pwszText)	\
    (This)->lpVtbl -> OutputText(This,hChan,nLogLevel,pwszText)

#define ITTSLog_OutputHexDump(This,hChan,nLogLevel,pBuffer,size,nCharsPerLine,pcszLinePrefix)	\
    (This)->lpVtbl -> OutputHexDump(This,hChan,nLogLevel,pBuffer,size,nCharsPerLine,pcszLinePrefix)

#define ITTSLog_FindChannel(This,pwszChanName,phChan)	\
    (This)->lpVtbl -> FindChannel(This,pwszChanName,phChan)

#define ITTSLog_LoadSettings(This)	\
    (This)->lpVtbl -> LoadSettings(This)

#define ITTSLog_SaveSettings(This)	\
    (This)->lpVtbl -> SaveSettings(This)

#define ITTSLog_GetLoggedItemCount(This,nLogLevel,pnCount)	\
    (This)->lpVtbl -> GetLoggedItemCount(This,nLogLevel,pnCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_GetFileName_Proxy( 
    ITTSLog * This,
    LPOLESTR *ppwszName);


void __RPC_STUB ITTSLog_GetFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_Open_Proxy( 
    ITTSLog * This,
    LPCOLESTR pwszName,
    int nOptions);


void __RPC_STUB ITTSLog_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_Close_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_IsOpen_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_IsOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_AddChannel_Proxy( 
    ITTSLog * This,
    LPCOLESTR pwszChanName,
    int *phChan);


void __RPC_STUB ITTSLog_AddChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_RemoveChannel_Proxy( 
    ITTSLog * This,
    int hChan);


void __RPC_STUB ITTSLog_RemoveChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_RemoveAllChannels_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_RemoveAllChannels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_IsChannelActive_Proxy( 
    ITTSLog * This,
    int hChan,
    int nLevel);


void __RPC_STUB ITTSLog_IsChannelActive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_SetLoggingLevel_Proxy( 
    ITTSLog * This,
    int hChan,
    int nLevel);


void __RPC_STUB ITTSLog_SetLoggingLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_GetLoggingLevel_Proxy( 
    ITTSLog * This,
    int hChan,
    int *pnLevel);


void __RPC_STUB ITTSLog_GetLoggingLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_GetChannelName_Proxy( 
    ITTSLog * This,
    int hChan,
    LPOLESTR *ppwszName);


void __RPC_STUB ITTSLog_GetChannelName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_Start_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_Stop_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_Pause_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_OutputText_Proxy( 
    ITTSLog * This,
    int hChan,
    int nLogLevel,
    LPCOLESTR pwszText);


void __RPC_STUB ITTSLog_OutputText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_OutputHexDump_Proxy( 
    ITTSLog * This,
    int hChan,
    int nLogLevel,
    const unsigned char *pBuffer,
    unsigned int size,
    int nCharsPerLine,
    LPCOLESTR pcszLinePrefix);


void __RPC_STUB ITTSLog_OutputHexDump_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_FindChannel_Proxy( 
    ITTSLog * This,
    LPCOLESTR pwszChanName,
    int *phChan);


void __RPC_STUB ITTSLog_FindChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_LoadSettings_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_LoadSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_SaveSettings_Proxy( 
    ITTSLog * This);


void __RPC_STUB ITTSLog_SaveSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLog_GetLoggedItemCount_Proxy( 
    ITTSLog * This,
    int nLogLevel,
    int *pnCount);


void __RPC_STUB ITTSLog_GetLoggedItemCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITTSLog_INTERFACE_DEFINED__ */


#ifndef __ITTSLogClientAdvise_INTERFACE_DEFINED__
#define __ITTSLogClientAdvise_INTERFACE_DEFINED__

/* interface ITTSLogClientAdvise */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ITTSLogClientAdvise;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E3369462-2F19-43db-B1D1-B72ABBF7BFA9")
    ITTSLogClientAdvise : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OnServerShutdown( 
            DWORD dwStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITTSLogClientAdviseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTSLogClientAdvise * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTSLogClientAdvise * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTSLogClientAdvise * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *OnServerShutdown )( 
            ITTSLogClientAdvise * This,
            DWORD dwStatus);
        
        END_INTERFACE
    } ITTSLogClientAdviseVtbl;

    interface ITTSLogClientAdvise
    {
        CONST_VTBL struct ITTSLogClientAdviseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTSLogClientAdvise_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTSLogClientAdvise_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTSLogClientAdvise_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTSLogClientAdvise_OnServerShutdown(This,dwStatus)	\
    (This)->lpVtbl -> OnServerShutdown(This,dwStatus)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLogClientAdvise_OnServerShutdown_Proxy( 
    ITTSLogClientAdvise * This,
    DWORD dwStatus);


void __RPC_STUB ITTSLogClientAdvise_OnServerShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITTSLogClientAdvise_INTERFACE_DEFINED__ */


#ifndef __ITTSLogServer_INTERFACE_DEFINED__
#define __ITTSLogServer_INTERFACE_DEFINED__

/* interface ITTSLogServer */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ITTSLogServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("138F8CC2-ECFB-413D-BE53-5B216937AC32")
    ITTSLogServer : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OpenLog( 
            LPCOLESTR pwszFileName,
            int nOptions,
            int *phLog) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CloseLog( 
            int hLog) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Advise( 
            ITTSLogClientAdvise *pClient) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Unadvise( 
            ITTSLogClientAdvise *pClient) = 0;
        
        virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_Log( 
            int hLog,
            /* [retval][out] */ ITTSLog **ppLog) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITTSLogServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTSLogServer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTSLogServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTSLogServer * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *OpenLog )( 
            ITTSLogServer * This,
            LPCOLESTR pwszFileName,
            int nOptions,
            int *phLog);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *CloseLog )( 
            ITTSLogServer * This,
            int hLog);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Advise )( 
            ITTSLogServer * This,
            ITTSLogClientAdvise *pClient);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            ITTSLogServer * This,
            ITTSLogClientAdvise *pClient);
        
        /* [helpstring][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Log )( 
            ITTSLogServer * This,
            int hLog,
            /* [retval][out] */ ITTSLog **ppLog);
        
        END_INTERFACE
    } ITTSLogServerVtbl;

    interface ITTSLogServer
    {
        CONST_VTBL struct ITTSLogServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTSLogServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTSLogServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTSLogServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTSLogServer_OpenLog(This,pwszFileName,nOptions,phLog)	\
    (This)->lpVtbl -> OpenLog(This,pwszFileName,nOptions,phLog)

#define ITTSLogServer_CloseLog(This,hLog)	\
    (This)->lpVtbl -> CloseLog(This,hLog)

#define ITTSLogServer_Advise(This,pClient)	\
    (This)->lpVtbl -> Advise(This,pClient)

#define ITTSLogServer_Unadvise(This,pClient)	\
    (This)->lpVtbl -> Unadvise(This,pClient)

#define ITTSLogServer_get_Log(This,hLog,ppLog)	\
    (This)->lpVtbl -> get_Log(This,hLog,ppLog)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLogServer_OpenLog_Proxy( 
    ITTSLogServer * This,
    LPCOLESTR pwszFileName,
    int nOptions,
    int *phLog);


void __RPC_STUB ITTSLogServer_OpenLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLogServer_CloseLog_Proxy( 
    ITTSLogServer * This,
    int hLog);


void __RPC_STUB ITTSLogServer_CloseLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLogServer_Advise_Proxy( 
    ITTSLogServer * This,
    ITTSLogClientAdvise *pClient);


void __RPC_STUB ITTSLogServer_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITTSLogServer_Unadvise_Proxy( 
    ITTSLogServer * This,
    ITTSLogClientAdvise *pClient);


void __RPC_STUB ITTSLogServer_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE ITTSLogServer_get_Log_Proxy( 
    ITTSLogServer * This,
    int hLog,
    /* [retval][out] */ ITTSLog **ppLog);


void __RPC_STUB ITTSLogServer_get_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITTSLogServer_INTERFACE_DEFINED__ */



#ifndef __TTSLOGGERLib_LIBRARY_DEFINED__
#define __TTSLOGGERLib_LIBRARY_DEFINED__

/* library TTSLOGGERLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_TTSLOGGERLib;

EXTERN_C const CLSID CLSID_TTSLog;

#ifdef __cplusplus

class DECLSPEC_UUID("94862986-87FD-427B-9044-35189BBAB078")
TTSLog;
#endif

EXTERN_C const CLSID CLSID_TTSLogServer;

#ifdef __cplusplus

class DECLSPEC_UUID("1B587A81-B2D4-48FD-A6C2-74C37700F5B4")
TTSLogServer;
#endif
#endif /* __TTSLOGGERLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


