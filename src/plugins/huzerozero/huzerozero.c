// **************************************************************************
// **************************************************************************
//
// huzerozero.c
//
// A simple Windows DDE plugin for Cosmigo's "Pro Motion NG".
//
// Copyright John Brandwood 2025.
//
//  Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************
//
// Set all sub-palette transparent pixels (color 16,32,48,64,etc) to color 0.
//
// **************************************************************************
// **************************************************************************

// Set the minimum supported platform (WINXP/WIN7/etc).
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <memory.h>
#include <ddeml.h>

// Define the plugin's name for error messages.

const char * const pPluginName = "Zero Transparent Pixels";

// Define Windows message IDs.

enum eAppMessage {
    APP_NO_CONNECT = WM_APP,
    APP_DISCONNECT,
    APP_EXECUTE
};

// Global variables.

HWND    hMainWindow;
HSZ     hServerName;
HSZ     hTopicName;
HSZ     hItemName;
DWORD   dDdeInstance;
HCONV   hDdeConnection;
BOOL    bDdeReady;
BOOL    bShowDefaultError = TRUE;

#define DDE_RETRIES 5
#define DDE_TIMEOUT 3000



// **************************************************************************
// **************************************************************************
//
// Based on Thiadmer Riemersma's code in "dde_c.txt" from Pro Motion's plugin
// example "dde_plugin_sample.zip".

HDDEDATA CALLBACK DdeCallback(
    UINT wType, UINT wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2,
    HDDEDATA hData, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
    switch (wType) {

    // Server disconnected, handle no longer valid.
    case XTYP_DISCONNECT:
        hDdeConnection = 0;
        PostMessage( hMainWindow, APP_DISCONNECT, 0, 0 );
        break;

    // DDE request completed, response is ready.
    case XTYP_XACT_COMPLETE:
        bDdeReady = TRUE;
        break;
    }
    return NULL;
}



// **************************************************************************
// **************************************************************************
//
// Based on Thiadmer Riemersma's code in "dde_c.txt" from Pro Motion's plugin
// example "dde_plugin_sample.zip".

void DdeClose( void )
{
    if (hDdeConnection) {
        DdeDisconnect( hDdeConnection );
        hDdeConnection = 0;
    }
    if (hServerName) {
        DdeFreeStringHandle( dDdeInstance, hServerName );
        hServerName = 0;
    }
    if (hTopicName) {
        DdeFreeStringHandle( dDdeInstance, hTopicName );
        hTopicName = 0;
    }
    if (hItemName) {
        DdeFreeStringHandle( dDdeInstance, hItemName );
        hItemName = 0;
    }
    if (dDdeInstance) {
        DdeUninitialize( dDdeInstance );
        dDdeInstance = 0;
    }
}



// **************************************************************************
// **************************************************************************
//
// Based on Thiadmer Riemersma's code in "dde_c.txt" from Pro Motion's plugin
// example "dde_plugin_sample.zip".

BOOL DdeOpen( void )
{
    if (DdeInitialize( &dDdeInstance, &DdeCallback, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR )
        return FALSE;

    hServerName = DdeCreateStringHandle( dDdeInstance, "PMOTION", CP_WINANSI );
    hTopicName = DdeCreateStringHandle( dDdeInstance, "ImageServer", CP_WINANSI );
    hItemName = DdeCreateStringHandle( dDdeInstance, "ImageServerItem", CP_WINANSI );

    if (hServerName && hTopicName && hItemName) {
        hDdeConnection = DdeConnect( dDdeInstance, hServerName, hTopicName, NULL );
        if (hDdeConnection)
            return TRUE;
    }

    DdeClose();
    return FALSE;
}



// **************************************************************************
// **************************************************************************
//
// Based on Thiadmer Riemersma's code in "dde_c.txt" from Pro Motion's plugin
// example "dde_plugin_sample.zip".

BOOL DdeSendString( LPSTR pCommand )
{
    HDDEDATA    hData;
    DWORD       dTime;
    MSG         cMessage;

    hData = DdeClientTransaction(
        (LPBYTE) pCommand, (DWORD) strlen( pCommand ) + 1,
        hDdeConnection, hItemName, CF_TEXT, XTYP_EXECUTE, TIMEOUT_ASYNC, NULL );

    bDdeReady = FALSE;

    dTime = GetTickCount();

    while (!bDdeReady) {
        if (!hDdeConnection)
            break;

        if ((GetTickCount() - dTime) > DDE_TIMEOUT)
            break;

        if (PeekMessage( &cMessage, NULL, 0, 0, PM_REMOVE )) {
            TranslateMessage( &cMessage );
            DispatchMessage( &cMessage );
        }
    }

    DdeFreeDataHandle( hData );

    return bDdeReady;
}



// **************************************************************************
// **************************************************************************
//
// Based on Thiadmer Riemersma's code in "dde_c.txt" from Pro Motion's plugin
// example "dde_plugin_sample.zip".

BOOL DdeRecvString( LPSTR pReply, DWORD dMaxSize )
{
    HDDEDATA    hData;
    DWORD       dGetSize;
    UINT        uError;
    unsigned    uRetry = 0;

    if (pReply == NULL || dMaxSize < 5)
        return FALSE;

    pReply[0] = '\0';

    do {
        uError = DMLERR_NO_ERROR;

        hData = DdeClientTransaction(
            NULL, 0, hDdeConnection, hItemName, CF_TEXT, XTYP_REQUEST, 1000, NULL );
        if (!hData)
            uError = DdeGetLastError( dDdeInstance );

    } while (uError != DMLERR_NO_ERROR && uRetry++ < DDE_RETRIES);

    if (uError)
        return FALSE;

    dGetSize = DdeGetData( hData, NULL, 0, 0 );
    if (dGetSize > (dMaxSize - 1))
        dGetSize = (dMaxSize - 1);

    DdeGetData( hData, (LPBYTE) pReply, dGetSize, 0L );
    DdeFreeDataHandle( hData );

    // Remove the trailing "\r\n" if it is present.

    if (pReply[dGetSize - 1] == '\0') --dGetSize;
    if (pReply[dGetSize - 1] == '\n') --dGetSize;
    if (pReply[dGetSize - 1] == '\r') --dGetSize;

    pReply[dGetSize] = '\0';

    // Is "Pro Motion" telling us to go away?

    if (strcmp( pReply, "EXIT" ) == 0)
        return bShowDefaultError = FALSE;

    return TRUE;
}



// **************************************************************************
// **************************************************************************

BOOL GetClipboardDIB8( BITMAPINFO **ppDIB8 )
{
    HGLOBAL     hGlobal;
    BITMAPINFO *pGlobalDIB;

    if (ppDIB8 == NULL)
        return FALSE;

    *ppDIB8 = NULL;

    if (IsClipboardFormatAvailable( CF_DIB ) &&
        OpenClipboard( NULL ))
    {
        hGlobal = GetClipboardData( CF_DIB );
        if (hGlobal != NULL)
        {
            pGlobalDIB = (BITMAPINFO *) GlobalLock( hGlobal );
            if (pGlobalDIB != NULL)
            {
                if (pGlobalDIB->bmiHeader.biBitCount    == 8 &&
                    pGlobalDIB->bmiHeader.biPlanes  == 1 &&
                    pGlobalDIB->bmiHeader.biCompression == BI_RGB &&
                   (pGlobalDIB->bmiHeader.biClrUsed == 0 ||
                    pGlobalDIB->bmiHeader.biClrUsed == 256) &&
                    pGlobalDIB->bmiHeader.biWidth   <= 32768 &&
                    abs(pGlobalDIB->bmiHeader.biHeight) <= 32768)
                {
                    SIZE_T uSize =
                        sizeof(BITMAPINFOHEADER) +
                        sizeof(RGBQUAD) * 256 +
                        abs( pGlobalDIB->bmiHeader.biHeight ) *
                        ((pGlobalDIB->bmiHeader.biWidth + 3) & ~3);

                    if (GlobalSize( hGlobal ) >= uSize) {
                        *ppDIB8 = malloc( uSize );
                        if (*ppDIB8)
                            memcpy( *ppDIB8, pGlobalDIB, uSize );
                    }
                }

                GlobalUnlock( hGlobal );
            }
        }

        CloseClipboard();
    }

    return (*ppDIB8 != NULL);
}



// **************************************************************************
// **************************************************************************

BOOL PutClipboardDIB8( BITMAPINFO *pDIB8 )
{
    HGLOBAL     hGlobal;
    BITMAPINFO *pGlobalDIB;
    BOOL        bResult = FALSE;

    if (pDIB8 != NULL &&
        pDIB8->bmiHeader.biBitCount    == 8 &&
        pDIB8->bmiHeader.biPlanes      == 1 &&
        pDIB8->bmiHeader.biCompression == BI_RGB &&
       (pDIB8->bmiHeader.biClrUsed     == 0 ||
        pDIB8->bmiHeader.biClrUsed     == 256) &&
        pDIB8->bmiHeader.biWidth       <= 32768 &&
        abs(pDIB8->bmiHeader.biHeight) <= 32768 &&
        OpenClipboard( NULL ))
    {
        if (EmptyClipboard())
        {
            SIZE_T uSize =
                sizeof(BITMAPINFOHEADER) +
                sizeof(RGBQUAD) * 256 +
                abs( pDIB8->bmiHeader.biHeight ) *
                ((pDIB8->bmiHeader.biWidth + 3) & ~3);

            hGlobal = GlobalAlloc( GMEM_MOVEABLE, uSize );
            if (hGlobal != NULL)
            {
                pGlobalDIB = (BITMAPINFO *) GlobalLock( hGlobal );
                if (pGlobalDIB != NULL)
                {
                    memcpy( pGlobalDIB, pDIB8, uSize );

                    GlobalUnlock( hGlobal );

                    if (SetClipboardData( CF_DIB, hGlobal ))
                        bResult = TRUE;
                }
            }
        }

        CloseClipboard();
    }

    return (bResult);
}



// **************************************************************************
// **************************************************************************
//
// Process a frame of bitmap data.
//
// Set all sub-palette transparent pixels (color 16,32,48,64,etc) to color 0.
//
// N.B. Remember that a DIB is stored from bottom to top in memory!

BOOL ProcessFrame( BITMAPINFO *pDIB8 )
{
    uint8_t    *pLineY = (uint8_t *) &(pDIB8->bmiColors[256]);
    uint8_t    *pPixel;
    unsigned    uX, uY;

    if (pDIB8->bmiHeader.biWidth == 0 || pDIB8->bmiHeader.biHeight == 0)
        return FALSE;

    uY = abs(pDIB8->bmiHeader.biHeight);

    while (uY-- != 0) {
        pPixel = pLineY;

        uX = pDIB8->bmiHeader.biWidth;

        while (uX-- != 0) {
            if ((*pPixel & 15) == 0)
                *pPixel = 0;
            ++pPixel;
        }

        pLineY += (pDIB8->bmiHeader.biWidth + 3) & ~3;
    }

    return TRUE;
}



// **************************************************************************
// **************************************************************************
//
// Process this plugin's conversation with the "Pro Motion NG" DDE server.

#define ADD_AS_NEW_LAYER 0
#define ADD_AS_NEW_FRAME 0

BOOL DoPluginAction( void )
{
    BOOL        bResult = FALSE;
    BITMAPINFO *pDIB8 = NULL;
    int         iLayerNum;
#if ADD_AS_NEW_FRAME
    int         iFrameCnt;
#endif
    char        aReply[256];

    #define aCommand aReply

    // Check whether there is a active layer, and give "Pro Motion" the
    // opportunity to just tell us to go away if no project is open!

    if (!DdeSendString( "GetActiveLayerNumber" ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

    iLayerNum = atoi( aReply );

    sprintf( aCommand, "GetLayerType(%d)", iLayerNum );
    if (!DdeSendString( aCommand ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

#if ADD_AS_NEW_FRAME
    if (strcmp( aReply, "animation" ) != 0) {
        MessageBox( hMainWindow, "Plugin cannot add a frame to a Static Image layer!",
            pPluginName, MB_ICONEXCLAMATION | MB_OK );
        bShowDefaultError = FALSE;
        goto error;
    }
#endif

    // Get the current frame.

    if (!DdeSendString( "SendFrame" ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

    if (!GetClipboardDIB8( &pDIB8 ))
        goto error;

    // Process it.

    if (!ProcessFrame( pDIB8 ))
        goto error;

#if ADD_AS_NEW_FRAME

    // Add a new frame to the current animation.

    if (!DdeSendString( "GetFrameCount" ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

    iFrameCnt = atoi( aReply );

    sprintf( aCommand, "SetFrameCount(%d)", ++iFrameCnt );
    if (!DdeSendString( aCommand ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

    sprintf( aCommand, "GotoFrame(%d)", iFrameCnt );
    if (!DdeSendString( aCommand ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

#elif ADD_AS_NEW_LAYER

    // Add a new layer to the current project.

    sprintf( aCommand, "AddImageLayer(%s)", pPluginName );
    if (!DdeSendString( aCommand ))
        goto error;

    if (!DdeSendString( "GetNumberOfLayers" ))
        goto error;
    if (!DdeRecvString( aReply, sizeof(aReply) ))
        goto error;

    iLayer = atoi( aReply ) - 1;

    sprintf( aCommand, "SelectLayer(%d)", iLayer );
    if (!DdeSendString( aCommand ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

#endif

    // Write the modified frame back.

    if (!PutClipboardDIB8( pDIB8 ))
        goto error;

    if (!DdeSendString( "ReceiveFrame" ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;

    if (strcmp( aReply, "OK" ) != 0)
        goto error;

#if ADD_AS_NEW_FRAME
    sprintf( aCommand, "SetDisplayFrameNumber(%d)", iFrameCnt );
    if (!DdeSendString( aCommand ))
        goto error;
    if ((!DdeRecvString( aReply, sizeof(aReply) )) || (aReply[0] == '-'))
        goto error;
#endif

    // All done!

    bResult = TRUE;

    // Yes, I know that people hate goto, but it is the
    // easiest way to clean up local allocations in C.

    error:

    if (pDIB8)
        free( pDIB8 );
    pDIB8 = NULL;

    return bResult;
}



// **************************************************************************
// **************************************************************************

LRESULT CALLBACK WindowProc( HWND hWindow, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
    switch (uMessage)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case APP_NO_CONNECT:
        MessageBox( hWindow,
            "Unable to connect to the Pro Motion DDE server.\n\n"
            "Is Pro Motion running, and with a project open?",
            pPluginName, MB_ICONEXCLAMATION | MB_OK );

    case APP_DISCONNECT:
        DestroyWindow( hWindow );
        break;

    case APP_EXECUTE: {
        if (!DoPluginAction()) {
            if (bShowDefaultError)
                MessageBox( hWindow, "Whoops, something went wrong!",
                    pPluginName, MB_ICONEXCLAMATION | MB_OK );
        }
        DestroyWindow( hWindow );
        break;
    }

    default:
        return DefWindowProc( hWindow, uMessage, wParam, lParam );
    }
    return 0;
}



// **************************************************************************
// **************************************************************************

int APIENTRY WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    WNDCLASSEX  cClass;
    MSG         cMessage;

    // Create a hidden "message-only" window to get a message queue.

    cClass.cbSize       = sizeof( WNDCLASSEX );
    cClass.style        = 0;
    cClass.lpfnWndProc  = WindowProc;
    cClass.cbClsExtra   = 0;
    cClass.cbWndExtra   = 0;
    cClass.hInstance    = hInstance;
    cClass.hIcon        = NULL;
    cClass.hCursor      = NULL;
    cClass.hbrBackground= NULL;
    cClass.lpszMenuName = NULL;
    cClass.lpszClassName= "pmPlugin";
    cClass.hIconSm      = NULL;

    RegisterClassEx( &cClass );

    hMainWindow = CreateWindow( "pmPlugin", "myPlugin", 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL );

    if (!hMainWindow) {
        return FALSE;
    }

    // Open the connection to the Pro Motion DDE server.

    if (DdeOpen()) {
        PostMessage( hMainWindow, APP_EXECUTE, 0, 0 );
    } else {
        PostMessage( hMainWindow, APP_NO_CONNECT, 0, 0 );
    }

    // Process messages until we get a WM_QUIT.

    while (GetMessage( &cMessage, NULL, 0, 0 ))
    {
        TranslateMessage( &cMessage );
        DispatchMessage( &cMessage );
    }

    // Close the connection to the Pro Motion DDE server.

    DdeClose();

    // Return the code from WM_QUIT.

    return (int) cMessage.wParam;
}
