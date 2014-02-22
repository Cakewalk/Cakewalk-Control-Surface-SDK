

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_ICSProperties,0x95FEB8DD,0xDD64,0x4460,0xAB,0x31,0xCA,0x56,0x10,0xBD,0xCC,0x33);


MIDL_DEFINE_GUID(IID, LIBID_SURFACEFRAMEWORKLib,0xEE2CAC15,0x64B1,0x45b9,0xBA,0x9B,0xB5,0x7A,0x0D,0x64,0x1E,0x74);


MIDL_DEFINE_GUID(CLSID, CLSID_ControlSurfacePropPage,0x75D0A649,0x84EE,0x479c,0x87,0x86,0x0A,0xA7,0xD7,0x73,0x8C,0x28);


MIDL_DEFINE_GUID(CLSID, CLSID_ControlSurface,0x5E604989,0xB1F0,0x4412,0xB9,0xD1,0x64,0xA5,0x4B,0x22,0x77,0xD0);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



