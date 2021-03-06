/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#ifndef MICROSTACK_NO_STDAFX
#include "stdafx.h"
#endif
#define _CRTDBG_MAP_ALLOC
#include <math.h>
#include <winerror.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winioctl.h>
#include <winbase.h>
#include <crtdbg.h>
#include "ILibParsers.h"
#include "UPnPMicroStack.h"
#include "ILibHTTPClient.h"

#define sem_t HANDLE
#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)
#define sem_destroy(x) (CloseHandle(*x)==0?1:0)
#define sem_wait(x) WaitForSingleObject(*x,INFINITE)
#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)
#define sem_post(x) ReleaseSemaphore(*x,1,NULL)
#define UPNP_PORT 1900
#define UPNP_GROUP "239.255.255.250"
#define UPnPMIN(a,b) (((a)<(b))?(a):(b))

#define LVL3DEBUG(x)

/* UPnP Set Function Pointers Methods */
void (*UPnPFP_PresentationPage) (void* upnptoken,struct packetheader *packet);
void (*UPnPFP_ConnectionManager_GetCurrentConnectionInfo) (void* upnptoken,int ConnectionID);
void (*UPnPFP_ConnectionManager_PrepareForConnection) (void* upnptoken,char* RemoteProtocolInfo,char* PeerConnectionManager,int PeerConnectionID,char* Direction);
void (*UPnPFP_ConnectionManager_ConnectionComplete) (void* upnptoken,int ConnectionID);
void (*UPnPFP_ConnectionManager_GetProtocolInfo) (void* upnptoken);
void (*UPnPFP_ConnectionManager_GetCurrentConnectionIDs) (void* upnptoken);
void (*UPnPFP_AVTransport_GetCurrentTransportActions) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_Play) (void* upnptoken,unsigned int InstanceID,char* Speed);
void (*UPnPFP_AVTransport_Previous) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_Next) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_Stop) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_GetTransportSettings) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_Seek) (void* upnptoken,unsigned int InstanceID,char* Unit,char* Target);
void (*UPnPFP_AVTransport_Pause) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_GetPositionInfo) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_GetTransportInfo) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_SetAVTransportURI) (void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData);
void (*UPnPFP_AVTransport_GetDeviceCapabilities) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_AVTransport_SetPlayMode) (void* upnptoken,unsigned int InstanceID,char* NewPlayMode);
void (*UPnPFP_AVTransport_GetMediaInfo) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetHorizontalKeystone) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetVolume) (void* upnptoken,unsigned int InstanceID,char* Channel);
void (*UPnPFP_RenderingControl_SelectPreset) (void* upnptoken,unsigned int InstanceID,char* PresetName);
void (*UPnPFP_RenderingControl_SetVolume) (void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume);
void (*UPnPFP_RenderingControl_ListPresets) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_SetVolumeDB) (void* upnptoken,unsigned int InstanceID,char* Channel,short DesiredVolume);
void (*UPnPFP_RenderingControl_SetRedVideoBlackLevel) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredRedVideoBlackLevel);
void (*UPnPFP_RenderingControl_SetContrast) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredContrast);
void (*UPnPFP_RenderingControl_SetLoudness) (void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredLoudness);
void (*UPnPFP_RenderingControl_SetBrightness) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredBrightness);
void (*UPnPFP_RenderingControl_GetLoudness) (void* upnptoken,unsigned int InstanceID,char* Channel);
void (*UPnPFP_RenderingControl_GetColorTemperature) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetSharpness) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetContrast) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetGreenVideoGain) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_SetRedVideoGain) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredRedVideoGain);
void (*UPnPFP_RenderingControl_SetGreenVideoBlackLevel) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredGreenVideoBlackLevel);
void (*UPnPFP_RenderingControl_GetVolumeDBRange) (void* upnptoken,unsigned int InstanceID,char* Channel);
void (*UPnPFP_RenderingControl_GetRedVideoBlackLevel) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetBlueVideoBlackLevel) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetBlueVideoGain) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_SetBlueVideoBlackLevel) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredBlueVideoBlackLevel);
void (*UPnPFP_RenderingControl_GetMute) (void* upnptoken,unsigned int InstanceID,char* Channel);
void (*UPnPFP_RenderingControl_SetBlueVideoGain) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredBlueVideoGain);
void (*UPnPFP_RenderingControl_GetVerticalKeystone) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_SetVerticalKeystone) (void* upnptoken,unsigned int InstanceID,short DesiredVerticalKeystone);
void (*UPnPFP_RenderingControl_GetBrightness) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetVolumeDB) (void* upnptoken,unsigned int InstanceID,char* Channel);
void (*UPnPFP_RenderingControl_GetGreenVideoBlackLevel) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_GetRedVideoGain) (void* upnptoken,unsigned int InstanceID);
void (*UPnPFP_RenderingControl_SetMute) (void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute);
void (*UPnPFP_RenderingControl_SetGreenVideoGain) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredGreenVideoGain);
void (*UPnPFP_RenderingControl_SetSharpness) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredSharpness);
void (*UPnPFP_RenderingControl_SetHorizontalKeystone) (void* upnptoken,unsigned int InstanceID,short DesiredHorizontalKeystone);
void (*UPnPFP_RenderingControl_SetColorTemperature) (void* upnptoken,unsigned int InstanceID,unsigned short DesiredColorTemperature);

const int UPnPDeviceDescriptionTemplateLengthUX = 1568;
const int UPnPDeviceDescriptionTemplateLength = 728;
const char UPnPDeviceDescriptionTemplate[728]={
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x63,0x2E,0x31,0x32,0x36,0x34,0x0D,0x0A,0x0D,0x0A,0x3C,0x3F
	,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x65,0x6E
	,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x72,0x6F,0x6F
	,0x74,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D,0x61,0x73
	,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x64,0x65,0x76,0x69,0x63,0x65,0x2D,0x31,0x2D,0x30
	,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0xC6,0x14,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F,0x72,0x3E,0x31
	,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02,0x02,0x3C,0x2F
	,0x8D,0x0B,0x00,0x06,0x12,0x00,0x08,0x02,0x05,0x54,0x79,0x70,0x65,0x3E,0x1B,0x1C,0x12,0x3A,0x4D,0x65
	,0x64,0x69,0x61,0x52,0x65,0x6E,0x64,0x65,0x72,0x65,0x72,0x3A,0x31,0x3C,0x2F,0x0B,0x0E,0x12,0x3C,0x66
	,0x72,0x69,0x65,0x6E,0x64,0x6C,0x79,0x4E,0x61,0x6D,0x65,0x3E,0x25,0x73,0x3C,0x2F,0x4D,0x04,0x0E,0x3C
	,0x6D,0x61,0x6E,0x75,0x66,0x61,0x63,0x74,0x75,0x72,0x65,0x72,0x3E,0x45,0x49,0x14,0x27,0x73,0x20,0x43
	,0x6F,0x6E,0x6E,0x65,0x63,0x74,0x65,0x64,0x20,0x61,0x6E,0x64,0x20,0x45,0x78,0x74,0x04,0x17,0x0A,0x64
	,0x20,0x50,0x43,0x20,0x4C,0x61,0x62,0x3C,0x2F,0x0D,0x0D,0x00,0x8D,0x10,0x10,0x55,0x52,0x4C,0x3E,0x68
	,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x69,0x44,0x5D,0x06,0x2E,0x63,0x6F,0x6D,0x3C,0x2F
	,0x90,0x09,0x11,0x3C,0x6D,0x6F,0x64,0x65,0x6C,0x44,0x65,0x73,0x63,0x72,0x69,0x70,0x74,0x69,0x6F,0x6E
	,0x49,0x1F,0x00,0x05,0x6D,0x03,0x41,0x56,0x20,0x85,0x35,0x01,0x20,0xC8,0x35,0x02,0x20,0x44,0x05,0x57
	,0x02,0x3C,0x2F,0x11,0x0E,0x00,0x86,0x12,0x00,0xC5,0x37,0x15,0x53,0x61,0x6D,0x70,0x6C,0x65,0x20,0x41
	,0x75,0x74,0x6F,0x2D,0x47,0x65,0x6E,0x65,0x72,0x61,0x74,0x65,0x64,0x8E,0x0E,0x00,0x87,0x3D,0x00,0xC5
	,0x0C,0x0A,0x75,0x6D,0x62,0x65,0x72,0x3E,0x58,0x31,0x3C,0x2F,0x0C,0x04,0x06,0x3C,0x73,0x65,0x72,0x69
	,0x61,0x88,0x07,0x00,0x84,0x4C,0x00,0x4D,0x04,0x0A,0x3C,0x55,0x44,0x4E,0x3E,0x75,0x75,0x69,0x64,0x3A
	,0x44,0x53,0x03,0x55,0x44,0x4E,0x45,0x0C,0x00,0x04,0x7E,0x04,0x4C,0x69,0x73,0x74,0x49,0x03,0x00,0x89
	,0x05,0x00,0x9A,0x6F,0x00,0xC7,0x0D,0x01,0x3A,0x86,0x6E,0x0C,0x69,0x6E,0x67,0x43,0x6F,0x6E,0x74,0x72
	,0x6F,0x6C,0x3A,0x31,0xC5,0x1C,0x00,0xCA,0x70,0x00,0x07,0x18,0x02,0x49,0x64,0x85,0x81,0x00,0x10,0x10
	,0x0D,0x49,0x64,0x3A,0x52,0x43,0x53,0x5F,0x30,0x2D,0x39,0x39,0x3C,0x2F,0xCA,0x0A,0x05,0x3C,0x53,0x43
	,0x50,0x44,0x04,0x64,0x00,0xD0,0x17,0x0B,0x2F,0x73,0x63,0x70,0x64,0x2E,0x78,0x6D,0x6C,0x3C,0x2F,0xC8
	,0x08,0x02,0x3C,0x63,0x86,0x1E,0x00,0xD5,0x0B,0x00,0x07,0x07,0x02,0x3C,0x2F,0x4B,0x09,0x09,0x3C,0x65
	,0x76,0x65,0x6E,0x74,0x53,0x75,0x62,0x55,0x18,0x00,0x45,0x07,0x02,0x3C,0x2F,0x0C,0x09,0x02,0x3C,0x2F
	,0x10,0x46,0x00,0x6B,0x48,0x0B,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x32,0x47,0x03
	,0x41,0x56,0x54,0x1A,0x47,0x00,0x8B,0x16,0x00,0xDF,0x45,0x00,0x8C,0x0A,0x00,0xA1,0x44,0x00,0xCC,0x15
	,0x00,0x7F,0x43,0x00,0x91,0x8B,0x00,0x07,0xE7,0x0A,0x69,0x6F,0x6E,0x4D,0x61,0x6E,0x61,0x67,0x65,0x72
	,0xF2,0x8B,0x04,0x43,0x4D,0x47,0x52,0x1A,0x8C,0x00,0x51,0x18,0x00,0x5F,0x8C,0x00,0x12,0x0C,0x00,0xA1
	,0x8C,0x00,0xD2,0x18,0x00,0xDE,0x8C,0x01,0x2F,0x8D,0xD8,0x03,0x2F,0x64,0x65,0xC7,0x05,0x02,0x72,0x6F
	,0x00,0x00,0x03,0x6F,0x74,0x3E,0x00,0x00};
/* RenderingControl */
const int UPnPRenderingControlDescriptionLengthUX = 17409;
const int UPnPRenderingControlDescriptionLength = 2426;
const char UPnPRenderingControlDescription[2426] = {
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x7B,0x2E,0x31,0x32,0x36,0x34,0x0D,0x0A,0x43,0x6F,0x6E,0x74
	,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3A,0x20,0x31,0x37,0x32,0x38,0x37,0x0D,0x0A,0x0D
	,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22
	,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C
	,0x73,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65
	,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65
	,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A
	,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46
	,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x1E
	,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x48,0x6F,0x72,0x69,0x7A,0x6F,0x6E,0x74,0x61,0x6C
	,0x4B,0x65,0x79,0x73,0x74,0x6F,0x6E,0x65,0x3C,0x2F,0x05,0x07,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65
	,0x6E,0x74,0x07,0x0E,0x00,0x87,0x03,0x00,0x87,0x0E,0x0A,0x49,0x6E,0x73,0x74,0x61,0x6E,0x63,0x65,0x49
	,0x44,0xC8,0x0B,0x04,0x64,0x69,0x72,0x65,0x06,0x17,0x04,0x69,0x6E,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72
	,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E
	,0x41,0x5F,0x41,0x52,0x47,0x5F,0x84,0x62,0x01,0x5F,0xCC,0x12,0x00,0x15,0x0B,0x02,0x3C,0x2F,0x4A,0x1F
	,0x00,0xCF,0x21,0x06,0x43,0x75,0x72,0x72,0x65,0x6E,0x5B,0x31,0x00,0x8A,0x25,0x03,0x6F,0x75,0x74,0xE2
	,0x25,0x00,0x94,0x43,0x00,0x21,0x25,0x01,0x2F,0x8E,0x4A,0x04,0x2F,0x61,0x63,0x74,0xCB,0x5D,0x00,0xCA
	,0x5A,0x05,0x56,0x6F,0x6C,0x75,0x6D,0xFF,0x57,0x00,0xFF,0x57,0x00,0xF0,0x57,0x06,0x68,0x61,0x6E,0x6E
	,0x65,0x6C,0xFF,0x78,0x02,0x45,0x5F,0x09,0x12,0x00,0x37,0x78,0x00,0x8E,0x4E,0x00,0x2F,0x75,0x00,0xC8
	,0x5D,0x00,0x3F,0x72,0x00,0xC7,0xCC,0x0C,0x53,0x65,0x6C,0x65,0x63,0x74,0x50,0x72,0x65,0x73,0x65,0x74
	,0xBF,0xCA,0x00,0xBF,0xCA,0x00,0xAE,0xCA,0x00,0x86,0x2C,0x04,0x4E,0x61,0x6D,0x65,0x7F,0xEC,0x02,0x45
	,0x5F,0xCC,0x12,0x00,0x7F,0x55,0x00,0x49,0x55,0x00,0x7F,0xC7,0x00,0x7F,0xC7,0x00,0x7F,0xC7,0x00,0x7F
	,0xC7,0x00,0x78,0xC7,0x07,0x44,0x65,0x73,0x69,0x72,0x65,0x64,0x58,0xC7,0x02,0x69,0x6E,0x3F,0xC7,0x00
	,0x31,0xC7,0x03,0x4C,0x69,0x73,0x87,0xC6,0x01,0x73,0x7F,0x72,0x00,0x7F,0x72,0x00,0x6F,0x72,0x06,0x75
	,0x72,0x72,0x65,0x6E,0x74,0x8A,0xC8,0x03,0x4C,0x69,0x73,0x09,0xF7,0x00,0x4A,0xEB,0x03,0x6F,0x75,0x74
	,0xA2,0xEB,0x00,0x50,0x11,0x00,0x3F,0xC8,0x00,0x10,0xC8,0x02,0x44,0x42,0xBF,0xC8,0x00,0xBF,0xC8,0x00
	,0xBF,0xC8,0x00,0xBF,0xC8,0x00,0xBF,0xC8,0x00,0xBB,0xC8,0x02,0x44,0x42,0xFF,0x72,0x00,0xCC,0x72,0x12
	,0x52,0x65,0x64,0x56,0x69,0x64,0x65,0x6F,0x42,0x6C,0x61,0x63,0x6B,0x4C,0x65,0x76,0x65,0x6C,0xBF,0xCB
	,0x00,0xBF,0xCB,0x00,0x35,0x55,0x00,0x5A,0x31,0x00,0x2E,0xF1,0x00,0x54,0x43,0x00,0x7F,0xCD,0x00,0x4A
	,0xCD,0x08,0x43,0x6F,0x6E,0x74,0x72,0x61,0x73,0x74,0x3F,0x58,0x00,0x3F,0x58,0x00,0x35,0xAD,0x00,0xD0
	,0x2E,0x00,0x6E,0xF0,0x00,0x4A,0x3E,0x00,0xBF,0xAD,0x00,0x8A,0xAD,0x08,0x4C,0x6F,0x75,0x64,0x6E,0x65
	,0x73,0x73,0x3F,0xAB,0x00,0x3F,0xAB,0x00,0x2E,0xAB,0x07,0x43,0x68,0x61,0x6E,0x6E,0x65,0x6C,0x3F,0xCC
	,0x02,0x45,0x5F,0x09,0x12,0x00,0x77,0xCB,0x00,0x10,0x4F,0x00,0x6E,0xEE,0x00,0x8A,0x5E,0x00,0x7F,0xC6
	,0x00,0x4A,0xC6,0x06,0x42,0x72,0x69,0x67,0x68,0x74,0xFF,0x73,0x00,0xFF,0xC6,0x00,0xF9,0xC6,0x00,0x52
	,0x2F,0x00,0x6E,0xEA,0x00,0x4C,0x3F,0x00,0xFF,0xC7,0x00,0x07,0xED,0x01,0x47,0xFF,0xC7,0x00,0xFF,0xC7
	,0x00,0xFF,0xC7,0x00,0xFF,0xC7,0x00,0xFB,0xC7,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0xDA,0xC7,0x03
	,0x6F,0x75,0x74,0x3F,0xC8,0x00,0xB6,0x73,0x10,0x43,0x6F,0x6C,0x6F,0x72,0x54,0x65,0x6D,0x70,0x65,0x72
	,0x61,0x74,0x75,0x72,0x65,0xBF,0x75,0x00,0xBF,0x75,0x00,0x75,0x55,0x00,0xD8,0x30,0x00,0x6F,0x57,0x00
	,0x92,0x42,0x00,0xFF,0xCC,0x00,0xCA,0xCC,0x05,0x53,0x68,0x61,0x72,0x70,0x3F,0xCD,0x00,0xBF,0x57,0x00
	,0xF9,0xAC,0x00,0x11,0x2F,0x00,0x2F,0xAD,0x00,0x0B,0x3F,0x00,0x7F,0xAD,0x00,0x4C,0xAD,0x06,0x6E,0x74
	,0x72,0x61,0x73,0x74,0x7F,0xAB,0x00,0x7F,0xAB,0x00,0x77,0xAB,0x00,0xCE,0x2E,0x00,0x71,0xA9,0x00,0x88
	,0x3E,0x00,0x7F,0xA7,0x00,0x4A,0xA7,0x0E,0x47,0x72,0x65,0x65,0x6E,0x56,0x69,0x64,0x65,0x6F,0x47,0x61
	,0x69,0x6E,0xBF,0xA8,0x00,0xBF,0xA8,0x00,0xB5,0xA8,0x00,0x56,0x30,0x00,0xAF,0xFF,0x00,0x90,0x41,0x00
	,0x3F,0xFF,0x00,0x07,0xFF,0x06,0x53,0x65,0x74,0x52,0x65,0x64,0x7F,0x57,0x00,0xFF,0xFF,0x00,0xF7,0xFF
	,0x0A,0x44,0x65,0x73,0x69,0x72,0x65,0x64,0x52,0x65,0x64,0xDB,0x56,0x00,0x24,0xD0,0x03,0x52,0x65,0x64
	,0x3F,0x56,0x00,0x15,0x56,0x00,0xCA,0xAD,0x0A,0x42,0x6C,0x61,0x63,0x6B,0x4C,0x65,0x76,0x65,0x6C,0x3F
	,0x58,0x00,0x3F,0x58,0x00,0x35,0x58,0x00,0xDC,0x31,0x00,0x6E,0xD5,0x00,0x56,0x44,0x00,0x3F,0xB2,0x00
	,0x47,0xFC,0x10,0x47,0x65,0x74,0x56,0x6F,0x6C,0x75,0x6D,0x65,0x44,0x42,0x52,0x61,0x6E,0x67,0x65,0x7F
	,0xB2,0x00,0x7F,0xB2,0x00,0x6E,0xB2,0x07,0x43,0x68,0x61,0x6E,0x6E,0x65,0x6C,0x7F,0xD3,0x02,0x45,0x5F
	,0x09,0x12,0x00,0xB0,0xD2,0x08,0x4D,0x69,0x6E,0x56,0x61,0x6C,0x75,0x65,0xD2,0xF3,0x03,0x6F,0x75,0x74
	,0x22,0xF4,0x00,0x48,0x5E,0x00,0x73,0x1E,0x02,0x61,0x78,0x7F,0x1E,0x00,0x68,0x1E,0x00,0x68,0x91,0x03
	,0x52,0x65,0x64,0xFF,0xEC,0x00,0xBF,0x92,0x00,0xBE,0x92,0x05,0x75,0x72,0x72,0x65,0x6E,0x5B,0x31,0x00
	,0xAF,0x76,0x03,0x52,0x65,0x64,0x3F,0xEC,0x00,0x1B,0xEC,0x04,0x42,0x6C,0x75,0x65,0x3F,0x5B,0x00,0x3F
	,0x5B,0x00,0x3F,0x5B,0x00,0x05,0x5B,0x00,0x9B,0x31,0x00,0xEF,0xD1,0x00,0x15,0x44,0x00,0xBF,0x5B,0x00
	,0x93,0x5B,0x04,0x47,0x61,0x69,0x6E,0x3F,0x5A,0x00,0x3F,0x5A,0x00,0x3E,0x5A,0x00,0x0C,0x30,0x00,0xB8
	,0x58,0x00,0x06,0x41,0x00,0xBF,0xB2,0x00,0xC7,0xFE,0x01,0x53,0xBF,0xB2,0x00,0xBF,0xB2,0x00,0xBF,0xB2
	,0x00,0x04,0xE3,0x07,0x44,0x65,0x73,0x69,0x72,0x65,0x64,0xA5,0xB2,0x02,0x69,0x6E,0x7F,0xB2,0x00,0x7F
	,0xB2,0x06,0x65,0x74,0x4D,0x75,0x74,0x65,0x3F,0xB0,0x00,0x3F,0xB0,0x00,0x2F,0xB0,0x06,0x68,0x61,0x6E
	,0x6E,0x65,0x6C,0x3F,0xD1,0x02,0x45,0x5F,0x09,0x12,0x00,0x77,0xD0,0x00,0x0C,0x4E,0x00,0x2F,0xCE,0x00
	,0xC6,0x5C,0x00,0xFF,0xCB,0x00,0xD3,0xCB,0x00,0xCC,0xF2,0x00,0x7F,0xCA,0x00,0x7F,0xCA,0x00,0x76,0xCA
	,0x00,0x0C,0x30,0x00,0xF7,0xC8,0x00,0xC6,0x40,0x00,0x7F,0xC7,0x00,0x4A,0xC7,0x0F,0x56,0x65,0x72,0x74
	,0x69,0x63,0x61,0x6C,0x4B,0x65,0x79,0x73,0x74,0x6F,0x6E,0x7F,0xCA,0x00,0x7F,0xCA,0x00,0x36,0xAA,0x00
	,0xD8,0x30,0x00,0x2F,0xAD,0x00,0x92,0x42,0x00,0x3F,0xB0,0x00,0x0A,0xB0,0x00,0x7F,0x59,0x00,0xFF,0xB0
	,0x00,0xFF,0xB0,0x00,0xC6,0xB0,0x00,0x62,0x59,0x02,0x69,0x6E,0x3F,0x59,0x00,0x7E,0xB2,0x0A,0x42,0x72
	,0x69,0x67,0x68,0x74,0x6E,0x65,0x73,0x73,0xFF,0xB0,0x00,0xFF,0xB0,0x00,0xF5,0xB0,0x00,0x52,0x2F,0x00
	,0x6F,0xAF,0x00,0x8C,0x3F,0x00,0xFF,0x54,0x00,0xCA,0x54,0x08,0x56,0x6F,0x6C,0x75,0x6D,0x65,0x44,0x42
	,0x7F,0x54,0x00,0x7F,0x54,0x00,0x6F,0x54,0x06,0x68,0x61,0x6E,0x6E,0x65,0x6C,0xFF,0xCC,0x02,0x45,0x5F
	,0x09,0x12,0x00,0xB7,0x74,0x00,0x06,0x4F,0x00,0xB7,0x73,0x00,0x4A,0x5E,0x00,0xFF,0xC7,0x00,0xCA,0xC7
	,0x14,0x47,0x72,0x65,0x65,0x6E,0x56,0x69,0x64,0x65,0x6F,0x42,0x6C,0x61,0x63,0x6B,0x4C,0x65,0x76,0x65
	,0x6C,0x7F,0xCA,0x00,0x7F,0xCA,0x00,0x75,0xCA,0x00,0xDC,0x31,0x00,0xEF,0xCC,0x00,0x96,0x44,0x00,0x7F
	,0xCF,0x00,0x4A,0xCF,0x03,0x52,0x65,0x64,0xC5,0x5B,0x04,0x47,0x61,0x69,0x6E,0x7F,0x5A,0x00,0x7F,0x5A
	,0x00,0x35,0xB0,0x00,0xD4,0x2F,0x00,0xAF,0xB1,0x00,0x8E,0x40,0x00,0xBF,0xB2,0x00,0xC7,0xF7,0x07,0x53
	,0x65,0x74,0x4D,0x75,0x74,0x65,0xBF,0xAE,0x00,0xBF,0xAE,0x00,0xAF,0xAE,0x06,0x68,0x61,0x6E,0x6E,0x65
	,0x6C,0xBF,0xCF,0x02,0x45,0x5F,0x09,0x12,0x00,0xF0,0xCE,0x07,0x44,0x65,0x73,0x69,0x72,0x65,0x64,0x0C
	,0x4E,0x00,0xEE,0xF0,0x00,0x86,0x5C,0x00,0x7F,0x70,0x00,0x4A,0x70,0x00,0x0A,0xF1,0x00,0x3F,0xC7,0x00
	,0x3F,0xC7,0x00,0xB9,0x52,0x00,0x56,0x30,0x00,0xAE,0xEB,0x00,0x50,0x41,0x00,0xFF,0xC7,0x00,0xCA,0xC7
	,0x09,0x53,0x68,0x61,0x72,0x70,0x6E,0x65,0x73,0x73,0x7F,0x56,0x00,0x7F,0x56,0x00,0xF5,0xA8,0x00,0x11
	,0x2F,0x00,0x6E,0xEC,0x00,0xCB,0x3E,0x00,0x7F,0xAB,0x00,0x4A,0xAB,0x12,0x48,0x6F,0x72,0x69,0x7A,0x6F
	,0x6E,0x74,0x61,0x6C,0x4B,0x65,0x79,0x73,0x74,0x6F,0x6E,0x65,0x7F,0xAC,0x00,0x7F,0xAC,0x00,0xF5,0xFE
	,0x00,0x5A,0x31,0x00,0xEE,0xD1,0x00,0x54,0x43,0x00,0x7F,0xAE,0x00,0x4A,0xAE,0x0F,0x43,0x6F,0x6C,0x6F
	,0x72,0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x3F,0x5A,0x00,0x3F,0xB0,0x00,0x36,0xB0,0x00
	,0xD8,0x30,0x00,0x2E,0xD5,0x00,0x52,0x42,0x00,0xB9,0xB3,0x00,0xC7,0xB5,0x00,0xC6,0xFE,0x07,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0x45,0xF0,0x01,0x54,0x86,0xE4,0x01,0x73,0x4C,0xF3,0x10,0x20,0x73,0x65,0x6E
	,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0x07,0xE7,0x06,0x56,0x65,0x72,0x74,0x69
	,0x63,0xD3,0x8E,0x0C,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x69,0x32,0x3C,0x2F,0x49,0x03,0x24,0x3C
	,0x61,0x6C,0x6C,0x6F,0x77,0x65,0x64,0x56,0x61,0x6C,0x75,0x65,0x52,0x61,0x6E,0x67,0x65,0x3E,0x3C,0x6D
	,0x69,0x6E,0x69,0x6D,0x75,0x6D,0x3E,0x2D,0x33,0x32,0x37,0x36,0x38,0x3C,0x2F,0x08,0x04,0x04,0x3C,0x6D
	,0x61,0x78,0x45,0x06,0x00,0x04,0x06,0x03,0x37,0x3C,0x2F,0xC8,0x03,0x09,0x3C,0x73,0x74,0x65,0x70,0x3E
	,0x31,0x3C,0x2F,0x05,0x02,0x02,0x3C,0x2F,0xD3,0x14,0x02,0x2F,0x73,0x8E,0xF6,0x00,0x5A,0x32,0x03,0x79
	,0x65,0x73,0x88,0x32,0x09,0x4C,0x61,0x73,0x74,0x43,0x68,0x61,0x6E,0x67,0x12,0x31,0x06,0x73,0x74,0x72
	,0x69,0x6E,0x67,0x0C,0x32,0x00,0xAA,0x18,0x00,0xCA,0x4A,0x00,0x8B,0xED,0x09,0x50,0x72,0x65,0x73,0x65
	,0x74,0x4E,0x61,0x6D,0x24,0x1B,0x00,0x0C,0x4D,0x00,0xC7,0xB9,0x00,0x8B,0x51,0x10,0x3E,0x46,0x61,0x63
	,0x74,0x6F,0x72,0x79,0x44,0x65,0x66,0x61,0x75,0x6C,0x74,0x73,0x4E,0x44,0x00,0x0F,0x0B,0x00,0x45,0xC4
	,0x03,0x6C,0x6C,0x61,0x04,0xF5,0x00,0x65,0x0C,0x0E,0x56,0x65,0x6E,0x64,0x6F,0x72,0x20,0x64,0x65,0x66
	,0x69,0x6E,0x65,0x64,0x10,0x17,0x01,0x2F,0xD2,0x26,0x00,0x74,0x46,0x08,0x4C,0x6F,0x75,0x64,0x6E,0x65
	,0x73,0x73,0x11,0x8F,0x07,0x62,0x6F,0x6F,0x6C,0x65,0x61,0x6E,0x7F,0x5E,0x09,0x3E,0x48,0x6F,0x72,0x69
	,0x7A,0x6F,0x6E,0x74,0xBF,0xA9,0x00,0xBF,0xA9,0x00,0xBB,0xA9,0x00,0x15,0x91,0x00,0x05,0x73,0x05,0x6E
	,0x63,0x65,0x49,0x44,0x11,0xDD,0x03,0x75,0x69,0x34,0x7F,0xAB,0x14,0x3E,0x42,0x6C,0x75,0x65,0x56,0x69
	,0x64,0x65,0x6F,0x42,0x6C,0x61,0x63,0x6B,0x4C,0x65,0x76,0x65,0x6C,0xD3,0x19,0x00,0x28,0xF7,0x01,0x30
	,0xD3,0xF5,0x03,0x31,0x30,0x30,0xFF,0x4B,0x00,0xE2,0xDC,0x03,0x52,0x65,0x64,0x45,0x31,0x04,0x47,0x61
	,0x69,0x6E,0xFF,0x2F,0x00,0xFF,0x2F,0x00,0x75,0xC6,0x05,0x47,0x72,0x65,0x65,0x6E,0xBF,0x61,0x00,0xBF
	,0x61,0x00,0x7F,0xAD,0x00,0x05,0xF8,0x06,0x56,0x6F,0x6C,0x75,0x6D,0x65,0xFF,0x8F,0x00,0xFF,0x8F,0x00
	,0xB5,0xDB,0x03,0x4D,0x75,0x74,0xD2,0x2D,0x07,0x62,0x6F,0x6F,0x6C,0x65,0x61,0x6E,0x7F,0xD8,0x0B,0x3E
	,0x42,0x72,0x69,0x67,0x68,0x74,0x6E,0x65,0x73,0x73,0x3F,0xD6,0x00,0x3F,0xD6,0x00,0x35,0xD6,0x0E,0x50
	,0x72,0x65,0x73,0x65,0x74,0x4E,0x61,0x6D,0x65,0x4C,0x69,0x73,0x74,0x91,0xD6,0x06,0x73,0x74,0x72,0x69
	,0x6E,0x67,0xBF,0x48,0x13,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F,0x54,0x59,0x50,0x45,0x5F,0x43,0x68,0x61
	,0x6E,0x6E,0x65,0x6C,0x63,0x1A,0x00,0x8C,0xF1,0x00,0x04,0x27,0x00,0x0E,0xF6,0x07,0x3E,0x4D,0x61,0x73
	,0x74,0x65,0x72,0x4E,0xE8,0x00,0xCF,0x08,0x02,0x4C,0x46,0xDD,0x07,0x01,0x52,0xD1,0x07,0x01,0x2F,0x12
	,0x1D,0x00,0x74,0xFB,0x0F,0x43,0x6F,0x6C,0x6F,0x72,0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72
	,0x3F,0xCC,0x00,0x51,0xFA,0x05,0x36,0x35,0x35,0x33,0x35,0xFF,0xFA,0x00,0xE8,0xFA,0x02,0x44,0x42,0x51
	,0xFB,0x00,0x29,0xFB,0x06,0x2D,0x33,0x32,0x37,0x36,0x38,0x53,0xFC,0x00,0x04,0x06,0x01,0x37,0xCB,0xFC
	,0x00,0x7F,0xF9,0x00,0x0B,0x5E,0x06,0x6E,0x74,0x72,0x61,0x73,0x74,0xFF,0xE1,0x00,0xFF,0xE1,0x00,0xF5
	,0xE1,0x0E,0x47,0x72,0x65,0x65,0x6E,0x56,0x69,0x64,0x65,0x6F,0x47,0x61,0x69,0x6E,0x7F,0x30,0x00,0x7F
	,0x30,0x00,0xF5,0xF8,0x03,0x52,0x65,0x64,0xC5,0x2F,0x0A,0x42,0x6C,0x61,0x63,0x6B,0x4C,0x65,0x76,0x65
	,0x6C,0xBF,0x61,0x00,0xBF,0x61,0x00,0x75,0xEE,0x04,0x42,0x6C,0x75,0x65,0x7F,0x61,0x00,0xBF,0x91,0x00
	,0x3E,0xED,0x09,0x53,0x68,0x61,0x72,0x70,0x6E,0x65,0x73,0x73,0xBF,0xC0,0x00,0xBF,0xC0,0x00,0x51,0xEF
	,0x09,0x2F,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x53,0x04,0xF5,0x01,0x54,0x08,0x05,0x01,0x63,0x00,0x00
	,0x03,0x70,0x64,0x3E,0x00,0x00};
/* AVTransport */
const int UPnPAVTransportDescriptionLengthUX = 14990;
const int UPnPAVTransportDescriptionLength = 3041;
const char UPnPAVTransportDescription[3041] = {
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x7B,0x2E,0x31,0x32,0x36,0x34,0x0D,0x0A,0x43,0x6F,0x6E,0x74
	,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3A,0x20,0x31,0x34,0x38,0x36,0x38,0x0D,0x0A,0x0D
	,0x0A,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22
	,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C
	,0x73,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65
	,0x6D,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65
	,0x2D,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A
	,0x6F,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46
	,0x02,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x1B
	,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x54,0x72,0x61
	,0x6E,0x73,0x70,0x6F,0x72,0x74,0x41,0x05,0x0B,0x03,0x73,0x3C,0x2F,0x45,0x08,0x09,0x3C,0x61,0x72,0x67
	,0x75,0x6D,0x65,0x6E,0x74,0x47,0x0F,0x00,0x87,0x03,0x00,0xC7,0x0F,0x0A,0x49,0x6E,0x73,0x74,0x61,0x6E
	,0x63,0x65,0x49,0x44,0xC8,0x0B,0x04,0x64,0x69,0x72,0x65,0x46,0x18,0x04,0x69,0x6E,0x3C,0x2F,0x8A,0x03
	,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64,0x53,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62
	,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F,0xC4,0x63,0x01,0x5F,0xCC,0x12,0x00,0x15,0x0B,0x02,0x3C
	,0x2F,0x4A,0x1F,0x00,0xCF,0x21,0x00,0xCF,0x2C,0x00,0x0A,0x21,0x03,0x6F,0x75,0x74,0x62,0x21,0x00,0x59
	,0x40,0x00,0xE1,0x21,0x01,0x2F,0x4E,0x47,0x04,0x2F,0x61,0x63,0x74,0xCB,0x5B,0x00,0xC7,0x58,0x04,0x50
	,0x6C,0x61,0x79,0x7F,0x53,0x00,0x7F,0x53,0x00,0x6E,0x53,0x05,0x53,0x70,0x65,0x65,0x64,0xF6,0x73,0x00
	,0x09,0x91,0x00,0x04,0x3D,0x00,0x07,0x12,0x00,0x7F,0x51,0x00,0x48,0x51,0x06,0x72,0x65,0x76,0x69,0x6F
	,0x75,0xBF,0xA5,0x00,0xBF,0xA5,0x00,0xFF,0x83,0x00,0x86,0xDC,0x04,0x4E,0x65,0x78,0x74,0xBF,0x31,0x00
	,0xBF,0x31,0x00,0x7F,0xB5,0x00,0x46,0x89,0x03,0x74,0x6F,0x70,0x3F,0x63,0x00,0x3F,0x63,0x00,0xFF,0xE6
	,0x00,0xC5,0xE6,0x02,0x47,0x65,0x8A,0xFD,0x07,0x53,0x65,0x74,0x74,0x69,0x6E,0x67,0xBF,0x98,0x00,0xFF
	,0xEA,0x00,0xEF,0xEA,0x00,0xC4,0xD9,0x04,0x4D,0x6F,0x64,0x65,0x92,0xEB,0x03,0x6F,0x75,0x74,0xE2,0xEB
	,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x8A,0x11,0x00,0x30,0x20,0x09,0x52,0x65,0x63,0x51,0x75,0x61
	,0x6C,0x69,0x74,0xBF,0x21,0x00,0x84,0x21,0x06,0x52,0x65,0x63,0x6F,0x72,0x64,0xCD,0x13,0x00,0xFF,0xAA
	,0x00,0xC8,0xAA,0x03,0x65,0x65,0x6B,0x7F,0x75,0x00,0x7F,0x75,0x00,0x6E,0x75,0x04,0x55,0x6E,0x69,0x74
	,0xBF,0xFC,0x02,0x45,0x5F,0x44,0x3D,0x00,0x36,0x75,0x05,0x54,0x61,0x72,0x67,0x65,0x7F,0x20,0x00,0x47
	,0x20,0x00,0xC8,0x12,0x00,0x7F,0xEB,0x00,0x48,0xBB,0x04,0x61,0x75,0x73,0x65,0xBF,0xE7,0x00,0xBF,0xE7
	,0x00,0xFF,0xA3,0x00,0x05,0xED,0x07,0x47,0x65,0x74,0x50,0x6F,0x73,0x69,0x84,0xE9,0x04,0x49,0x6E,0x66
	,0x6F,0xBF,0xA6,0x00,0xBF,0xA6,0x00,0xEF,0x86,0x04,0x72,0x61,0x63,0x6B,0xBE,0xF9,0x00,0xC7,0x10,0x00
	,0xB5,0x1E,0x04,0x44,0x75,0x72,0x61,0x04,0xF8,0x00,0xBF,0x20,0x00,0xCE,0x12,0x00,0x35,0x41,0x08,0x4D
	,0x65,0x74,0x61,0x44,0x61,0x74,0x61,0x3F,0x43,0x00,0xCE,0x12,0x00,0xB5,0x63,0x03,0x55,0x52,0x49,0x7F
	,0x64,0x00,0x89,0x11,0x00,0xB0,0x83,0x07,0x52,0x65,0x6C,0x54,0x69,0x6D,0x65,0x37,0x84,0x01,0x52,0x44
	,0xDE,0x03,0x69,0x76,0x65,0xC4,0x10,0x00,0x08,0xC4,0x00,0xB2,0xA4,0x03,0x41,0x62,0x73,0x3B,0x21,0x07
	,0x41,0x62,0x73,0x6F,0x6C,0x75,0x74,0x3F,0x21,0x08,0x52,0x65,0x6C,0x43,0x6F,0x75,0x6E,0x74,0x7F,0x42
	,0x00,0x05,0x11,0x02,0x65,0x72,0x3D,0x43,0x00,0x3C,0x22,0x00,0x48,0x43,0x00,0x32,0x22,0x00,0xC9,0xED
	,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x87,0xE1,0x00,0x08,0x02,0x00,0x85,0xF0,0x02,0x47,0x65,0x04,0xFF
	,0x0A,0x6E,0x73,0x70,0x6F,0x72,0x74,0x49,0x6E,0x66,0x6F,0x48,0xF1,0x00,0x0E,0x0F,0x00,0xCF,0xFD,0x0A
	,0x49,0x6E,0x73,0x74,0x61,0x6E,0x63,0x65,0x49,0x44,0x12,0xFD,0x02,0x69,0x6E,0x63,0x78,0x0A,0x5F,0x41
	,0x52,0x47,0x5F,0x54,0x59,0x50,0x45,0x5F,0xCC,0x12,0x00,0x30,0xFD,0x00,0x0A,0xEC,0x00,0x06,0x30,0x00
	,0x85,0xF3,0x00,0x37,0xFF,0x00,0x50,0x11,0x00,0x3F,0x23,0x00,0x05,0x23,0x02,0x75,0x73,0x7F,0x23,0x00
	,0x89,0x11,0x00,0xB7,0x46,0x05,0x53,0x70,0x65,0x65,0x64,0x7F,0x44,0x05,0x74,0x50,0x6C,0x61,0x79,0x47
	,0x12,0x00,0xFF,0x9C,0x00,0x47,0xE6,0x05,0x53,0x65,0x74,0x41,0x56,0x49,0x9D,0x03,0x55,0x52,0x49,0x3F
	,0x9D,0x00,0x3F,0x9D,0x00,0x35,0x9D,0x03,0x55,0x52,0x49,0xF7,0xBE,0x00,0x8F,0x3E,0x00,0x3A,0x20,0x08
	,0x4D,0x65,0x74,0x61,0x44,0x61,0x74,0x61,0x3F,0x22,0x01,0x72,0x0E,0x13,0x00,0xFF,0x78,0x00,0x87,0xE6
	,0x15,0x47,0x65,0x74,0x44,0x65,0x76,0x69,0x63,0x65,0x43,0x61,0x70,0x61,0x62,0x69,0x6C,0x69,0x74,0x69
	,0x65,0x73,0xFF,0x79,0x00,0xFF,0x79,0x00,0xEE,0xF3,0x00,0x44,0xBD,0x05,0x4D,0x65,0x64,0x69,0x61,0xB7
	,0xF0,0x08,0x50,0x6F,0x73,0x73,0x69,0x62,0x6C,0x65,0x44,0xCF,0x0B,0x62,0x61,0x63,0x6B,0x53,0x74,0x6F
	,0x72,0x61,0x67,0x65,0xC7,0x14,0x00,0xF0,0xF3,0x03,0x52,0x65,0x63,0x7F,0x23,0x00,0x45,0x23,0x06,0x52
	,0x65,0x63,0x6F,0x72,0x64,0xFF,0x22,0x0E,0x65,0x63,0x51,0x75,0x61,0x6C,0x69,0x74,0x79,0x4D,0x6F,0x64
	,0x65,0x73,0xBF,0x24,0x00,0x86,0x24,0x00,0x4E,0x14,0x00,0xBF,0xA0,0x00,0x07,0xEA,0x03,0x53,0x65,0x74
	,0x05,0x71,0x03,0x6F,0x64,0x65,0x3F,0x9E,0x00,0x3F,0x9E,0x00,0xEE,0xF7,0x03,0x4E,0x65,0x77,0xD0,0x2D
	,0x00,0x2E,0xF6,0x06,0x43,0x75,0x72,0x72,0x65,0x6E,0x0B,0x3F,0x00,0x7F,0xF4,0x00,0x4A,0xF4,0x00,0xC5
	,0xC3,0x04,0x49,0x6E,0x66,0x6F,0x3F,0x54,0x00,0x3F,0x54,0x00,0x2F,0x54,0x06,0x72,0x54,0x72,0x61,0x63
	,0x6B,0x38,0xAA,0x08,0x4E,0x75,0x6D,0x62,0x65,0x72,0x4F,0x66,0x48,0x11,0x00,0x70,0xEE,0x00,0x05,0xFC
	,0x04,0x44,0x75,0x72,0x61,0x44,0xEB,0x00,0xB7,0xEF,0x00,0x87,0x74,0x00,0xCF,0x12,0x00,0x30,0xEE,0x00
	,0x07,0x86,0x03,0x55,0x52,0x49,0xF7,0xEC,0x0A,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x46
	,0x11,0x00,0x7A,0x20,0x08,0x4D,0x65,0x74,0x61,0x44,0x61,0x74,0x61,0x7F,0x22,0x02,0x6F,0x72,0x4E,0x13
	,0x00,0xF2,0xDA,0x01,0x78,0xFB,0x43,0x00,0x84,0x0F,0x00,0xFF,0x44,0x00,0x88,0x20,0x00,0x3F,0x44,0x00
	,0x92,0x22,0x00,0x3A,0x45,0x04,0x50,0x6C,0x61,0x79,0x04,0xFA,0x02,0x75,0x6D,0x77,0xCC,0x00,0x44,0x10
	,0x0B,0x62,0x61,0x63,0x6B,0x53,0x74,0x6F,0x72,0x61,0x67,0x65,0x08,0x13,0x00,0xF0,0xED,0x06,0x52,0x65
	,0x63,0x6F,0x72,0x64,0xBD,0x22,0x00,0xC6,0x10,0x00,0x3F,0x22,0x05,0x57,0x72,0x69,0x74,0x65,0x84,0xFB
	,0x02,0x75,0x73,0xFD,0x21,0x00,0x06,0x55,0x00,0x8D,0x13,0x00,0x61,0xF0,0x00,0x09,0xF3,0x03,0x4C,0x69
	,0x73,0xC5,0x03,0x00,0x87,0xE7,0x00,0x47,0x02,0x00,0x86,0x05,0x06,0x73,0x65,0x72,0x76,0x69,0x63,0xC5
	,0x28,0x02,0x65,0x54,0xC6,0xE0,0x01,0x73,0xCC,0xED,0x10,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65,0x6E
	,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0x4E,0xE3,0x00,0xC5,0x7B,0x03,0x6F,0x64,0x65,0x89,0xE2,0x10,0x61
	,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x0D,0x3C,0x61
	,0x6C,0x6C,0x6F,0x77,0x65,0x64,0x56,0x61,0x6C,0x75,0x65,0x46,0x24,0x00,0x8C,0x04,0x09,0x3E,0x4E,0x4F
	,0x52,0x4D,0x41,0x4C,0x3C,0x2F,0x4D,0x05,0x00,0xCE,0x08,0x09,0x52,0x45,0x50,0x45,0x41,0x54,0x5F,0x41
	,0x4C,0xDE,0x09,0x05,0x49,0x4E,0x54,0x52,0x4F,0x50,0x12,0x01,0x2F,0xD2,0x1F,0x07,0x64,0x65,0x66,0x61
	,0x75,0x6C,0x74,0xCE,0x1F,0x00,0x4D,0x05,0x03,0x3C,0x2F,0x73,0x8E,0xE2,0x00,0xA4,0x46,0x00,0xD5,0x8D
	,0x00,0xBF,0x47,0x08,0x3E,0x55,0x4E,0x4B,0x4E,0x4F,0x57,0x4E,0xDD,0x47,0x02,0x44,0x56,0x9D,0x4F,0x05
	,0x4D,0x49,0x4E,0x49,0x2D,0x1F,0x09,0x03,0x56,0x48,0x53,0x9D,0x60,0x02,0x57,0x2D,0xA0,0x08,0x01,0x53
	,0xA1,0x08,0x01,0x44,0x21,0x11,0x04,0x56,0x48,0x53,0x43,0xDE,0x29,0x05,0x49,0x44,0x45,0x4F,0x38,0x1D
	,0x8B,0x02,0x48,0x49,0x1E,0x08,0x06,0x43,0x44,0x2D,0x52,0x4F,0x4D,0xE0,0x08,0x02,0x44,0x41,0x61,0x11
	,0x00,0xA1,0x19,0x01,0x57,0xE2,0x32,0x03,0x2D,0x43,0x44,0x5E,0x55,0x01,0x41,0x5F,0x08,0x08,0x4D,0x44
	,0x2D,0x41,0x55,0x44,0x49,0x4F,0x60,0x09,0x07,0x50,0x49,0x43,0x54,0x55,0x52,0x45,0xDF,0x91,0x00,0xE2
	,0x46,0x00,0x04,0x09,0x00,0x45,0x61,0x00,0xA2,0x12,0x00,0x20,0x1B,0x03,0x2B,0x52,0x57,0xE2,0x23,0x00
	,0xE3,0x08,0x01,0x41,0xA2,0x2C,0x00,0xE2,0x48,0x03,0x44,0x41,0x54,0xDD,0xD8,0x01,0x4C,0x1E,0x6A,0x02
	,0x48,0x44,0xDF,0x69,0x06,0x49,0x43,0x52,0x4F,0x2D,0x4D,0x1E,0xEA,0x07,0x4E,0x45,0x54,0x57,0x4F,0x52
	,0x4B,0x1E,0x09,0x03,0x4F,0x4E,0x45,0x5F,0x08,0x0C,0x54,0x5F,0x49,0x4D,0x50,0x4C,0x45,0x4D,0x45,0x4E
	,0x54,0x45,0x9E,0x97,0x10,0x20,0x76,0x65,0x6E,0x64,0x6F,0x72,0x2D,0x64,0x65,0x66,0x69,0x6E,0x65,0x64
	,0x20,0x10,0xF8,0x00,0xCD,0xFB,0x16,0x4C,0x69,0x73,0x74,0x3E,0x3C,0x2F,0x73,0x74,0x61,0x74,0x65,0x56
	,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x3C,0xCD,0x03,0x24,0x20,0x73,0x65,0x6E,0x64,0x45,0x76,0x65
	,0x6E,0x74,0x73,0x3D,0x22,0x79,0x65,0x73,0x22,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x4C,0x61,0x73,0x74
	,0x43,0x68,0x61,0x6E,0x67,0x65,0x3C,0x2F,0x45,0x04,0x12,0x3C,0x64,0x61,0x74,0x61,0x54,0x79,0x70,0x65
	,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x00,0xAB,0x18,0x02,0x6E,0x6F,0x48,0x18,0x14
	,0x52,0x65,0x6C,0x61,0x74,0x69,0x76,0x65,0x54,0x69,0x6D,0x65,0x50,0x6F,0x73,0x69,0x74,0x69,0x6F,0x6E
	,0xFF,0x1A,0x00,0xD8,0x1A,0x0F,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x54,0x72,0x61,0x63,0x6B,0x55,0x52
	,0x49,0xBF,0x19,0x00,0xA4,0x19,0x04,0x44,0x75,0x72,0x61,0x7F,0x34,0x00,0x63,0x34,0x10,0x52,0x65,0x63
	,0x6F,0x72,0x64,0x51,0x75,0x61,0x6C,0x69,0x74,0x79,0x4D,0x6F,0x64,0xE4,0x6A,0x00,0xD2,0x87,0x00,0x8D
	,0xFC,0x04,0x30,0x3A,0x45,0x50,0x9D,0xF8,0x03,0x31,0x3A,0x4C,0x5E,0x08,0x03,0x32,0x3A,0x53,0x9E,0x10
	,0x07,0x30,0x3A,0x42,0x41,0x53,0x49,0x43,0x9F,0x19,0x06,0x4D,0x45,0x44,0x49,0x55,0x4D,0x9F,0x1A,0x04
	,0x48,0x49,0x47,0x48,0xBF,0xD6,0x00,0xBF,0xD6,0x00,0x79,0xA3,0x05,0x4D,0x65,0x64,0x69,0x61,0xFF,0x89
	,0x00,0xE0,0xD8,0x0F,0x41,0x62,0x73,0x6F,0x6C,0x75,0x74,0x65,0x43,0x6F,0x75,0x6E,0x74,0x65,0x72,0x99
	,0xD9,0x02,0x69,0x34,0x7F,0xF3,0x00,0x49,0xF3,0x00,0xBF,0x1A,0x00,0x24,0x35,0x14,0x5F,0x41,0x52,0x47
	,0x5F,0x54,0x59,0x50,0x45,0x5F,0x49,0x6E,0x73,0x74,0x61,0x6E,0x63,0x65,0x49,0x44,0x91,0xF4,0x01,0x75
	,0x7F,0x1A,0x00,0x44,0x4F,0x0D,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x55,0x52,0x49,0x7F
	,0xF2,0x00,0x5F,0xF2,0x0D,0x54,0x72,0x61,0x63,0x6B,0x4D,0x65,0x74,0x61,0x44,0x61,0x74,0x61,0x7F,0x83
	,0x00,0x26,0x34,0x00,0x7F,0x1B,0x00,0x60,0xB9,0x00,0xC9,0x4E,0x09,0x50,0x6C,0x61,0x79,0x53,0x70,0x65
	,0x65,0x64,0xE3,0xB8,0x00,0x12,0xD8,0x00,0x4D,0xFF,0x01,0x31,0xBF,0xEF,0x00,0xBF,0xEF,0x00,0x86,0xEF
	,0x04,0x4E,0x65,0x78,0x74,0xBF,0x86,0x00,0x26,0xEF,0x1A,0x50,0x6F,0x73,0x73,0x69,0x62,0x6C,0x65,0x52
	,0x65,0x63,0x6F,0x72,0x64,0x51,0x75,0x61,0x6C,0x69,0x74,0x79,0x4D,0x6F,0x64,0x65,0x73,0x7F,0x1C,0x00
	,0x66,0x1C,0x0B,0x53,0x74,0x6F,0x72,0x61,0x67,0x65,0x4D,0x65,0x64,0x69,0x7F,0xA4,0x00,0x9A,0xF2,0x13
	,0x62,0x73,0x6F,0x6C,0x75,0x74,0x65,0x54,0x69,0x6D,0x65,0x50,0x6F,0x73,0x69,0x74,0x69,0x6F,0x6E,0xFF
	,0xD9,0x00,0xAA,0x6D,0x00,0x7F,0xDB,0x00,0xA1,0x6F,0x07,0x6C,0x61,0x79,0x62,0x61,0x63,0x6B,0xCB,0x51
	,0x02,0x75,0x6D,0xFF,0xC0,0x0A,0x75,0x65,0x3E,0x55,0x4E,0x4B,0x4E,0x4F,0x57,0x4E,0x5D,0xC2,0x02,0x44
	,0x56,0x1D,0xCA,0x05,0x4D,0x49,0x4E,0x49,0x2D,0x1F,0x09,0x03,0x56,0x48,0x53,0x1D,0xDB,0x02,0x57,0x2D
	,0xA0,0x08,0x01,0x53,0xA1,0x08,0x01,0x44,0x21,0x11,0x04,0x56,0x48,0x53,0x43,0xDE,0x29,0x05,0x49,0x44
	,0x45,0x4F,0x38,0x5D,0x43,0x02,0x48,0x49,0x1E,0x08,0x06,0x43,0x44,0x2D,0x52,0x4F,0x4D,0xE0,0x08,0x02
	,0x44,0x41,0x61,0x11,0x00,0xA1,0x19,0x01,0x57,0xE2,0x32,0x03,0x2D,0x43,0x44,0x5E,0x55,0x01,0x41,0x5F
	,0x08,0x08,0x4D,0x44,0x2D,0x41,0x55,0x44,0x49,0x4F,0x60,0x09,0x07,0x50,0x49,0x43,0x54,0x55,0x52,0x45
	,0xDF,0x91,0x00,0xE2,0x46,0x00,0x04,0x09,0x00,0x45,0x61,0x00,0xA2,0x12,0x00,0x20,0x1B,0x03,0x2B,0x52
	,0x57,0xE2,0x23,0x00,0xE3,0x08,0x01,0x41,0xA2,0x2C,0x00,0xE2,0x48,0x03,0x44,0x41,0x54,0xDD,0xD8,0x01
	,0x4C,0x1E,0x6A,0x02,0x48,0x44,0xDF,0x69,0x06,0x49,0x43,0x52,0x4F,0x2D,0x4D,0x1E,0xEA,0x07,0x4E,0x45
	,0x54,0x57,0x4F,0x52,0x4B,0x1E,0x09,0x03,0x4F,0x4E,0x45,0x5F,0x08,0x0C,0x54,0x5F,0x49,0x4D,0x50,0x4C
	,0x45,0x4D,0x45,0x4E,0x54,0x45,0x9E,0x97,0x10,0x20,0x76,0x65,0x6E,0x64,0x6F,0x72,0x2D,0x64,0x65,0x66
	,0x69,0x6E,0x65,0x64,0x20,0x10,0xF8,0x00,0xCD,0xFB,0x16,0x4C,0x69,0x73,0x74,0x3E,0x3C,0x2F,0x73,0x74
	,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x3C,0xCD,0x03,0x30,0x20,0x73,0x65,0x6E
	,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0x3E,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x43
	,0x75,0x72,0x72,0x65,0x6E,0x74,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x41,0x63,0x74,0x69,0x6F
	,0x6E,0x73,0x3C,0x2F,0x85,0x07,0x12,0x3C,0x64,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72
	,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04,0x00,0xB5,0x1B,0x16,0x52,0x65,0x63,0x6F,0x72,0x64,0x4D,0x65,0x64
	,0x69,0x75,0x6D,0x57,0x72,0x69,0x74,0x65,0x53,0x74,0x61,0x74,0x75,0xA4,0x1B,0x00,0x92,0x3B,0x00,0x0D
	,0xFD,0x07,0x57,0x52,0x49,0x54,0x41,0x42,0x4C,0x1E,0xD1,0x09,0x50,0x52,0x4F,0x54,0x45,0x43,0x54,0x45
	,0x44,0x61,0x69,0x00,0xE5,0x13,0x07,0x55,0x4E,0x4B,0x4E,0x4F,0x57,0x4E,0xBC,0x7C,0x00,0x7F,0x71,0x00
	,0x48,0x71,0x17,0x50,0x6F,0x73,0x73,0x69,0x62,0x6C,0x65,0x50,0x6C,0x61,0x79,0x62,0x61,0x63,0x6B,0x53
	,0x74,0x6F,0x72,0x61,0x67,0x65,0x04,0x5A,0x01,0x61,0xBF,0x72,0x00,0x18,0x8E,0x00,0x49,0x8C,0x00,0x84
	,0x70,0x01,0x65,0x7F,0x70,0x08,0x75,0x65,0x3E,0x53,0x54,0x4F,0x50,0x50,0xDF,0xC4,0x0F,0x50,0x41,0x55
	,0x53,0x45,0x44,0x5F,0x50,0x4C,0x41,0x59,0x42,0x41,0x43,0x4B,0x24,0x0B,0x09,0x52,0x45,0x43,0x4F,0x52
	,0x44,0x49,0x4E,0x47,0x5E,0x86,0x03,0x4C,0x41,0x59,0x20,0x09,0x00,0xA6,0x12,0x0A,0x54,0x52,0x41,0x4E
	,0x53,0x49,0x54,0x49,0x4F,0x4E,0x20,0x1D,0x10,0x4E,0x4F,0x5F,0x4D,0x45,0x44,0x49,0x41,0x5F,0x50,0x52
	,0x45,0x53,0x45,0x4E,0x54,0x3F,0xF8,0x00,0x18,0xF8,0x0D,0x4E,0x75,0x6D,0x62,0x65,0x72,0x4F,0x66,0x54
	,0x72,0x61,0x63,0x6B,0xD2,0xF5,0x03,0x75,0x69,0x34,0x98,0xD9,0x12,0x52,0x61,0x6E,0x67,0x65,0x3E,0x3C
	,0x6D,0x69,0x6E,0x69,0x6D,0x75,0x6D,0x3E,0x30,0x3C,0x2F,0xC8,0x02,0x04,0x3C,0x6D,0x61,0x78,0x05,0x05
	,0x0C,0x34,0x32,0x39,0x34,0x39,0x36,0x37,0x32,0x39,0x35,0x3C,0x2F,0x08,0x05,0x02,0x3C,0x2F,0x53,0x11
	,0x00,0x74,0xB5,0x12,0x41,0x5F,0x41,0x52,0x47,0x5F,0x54,0x59,0x50,0x45,0x5F,0x53,0x65,0x65,0x6B,0x4D
	,0x6F,0x64,0xFF,0x99,0x00,0xC4,0xFA,0x08,0x41,0x42,0x53,0x5F,0x54,0x49,0x4D,0x45,0xDF,0x7A,0x01,0x4C
	,0x62,0x09,0x00,0x84,0x12,0x05,0x43,0x4F,0x55,0x4E,0x54,0xE1,0x12,0x00,0xA2,0x09,0x08,0x54,0x52,0x41
	,0x43,0x4B,0x5F,0x4E,0x52,0x9D,0xBF,0x0C,0x43,0x48,0x41,0x4E,0x4E,0x45,0x4C,0x5F,0x46,0x52,0x45,0x51
	,0x1E,0xA1,0x09,0x41,0x50,0x45,0x2D,0x49,0x4E,0x44,0x45,0x58,0x9D,0xD3,0x05,0x46,0x52,0x41,0x4D,0x45
	,0xBF,0x9D,0x00,0x98,0x9D,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x45,0x9D,0x00,0x3F,0x9D,0x00,0x25
	,0x9D,0x08,0x73,0x74,0x65,0x70,0x3E,0x31,0x3C,0x2F,0x05,0x02,0x00,0xBF,0xA0,0x00,0x0A,0xCF,0x0F,0x54
	,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x53,0x74,0x61,0x74,0x75,0x73,0xBF,0x9F,0x05,0x75,0x65,0x3E
	,0x4F,0x4B,0x1D,0x9E,0x0E,0x45,0x52,0x52,0x4F,0x52,0x5F,0x4F,0x43,0x43,0x55,0x52,0x52,0x45,0x44,0xDD
	,0xA8,0x10,0x20,0x76,0x65,0x6E,0x64,0x6F,0x72,0x2D,0x64,0x65,0x66,0x69,0x6E,0x65,0x64,0x20,0x3F,0x72
	,0x00,0x27,0xE1,0x06,0x54,0x61,0x72,0x67,0x65,0x74,0xA3,0xE1,0x00,0x10,0xFC,0x08,0x2F,0x73,0x65,0x72
	,0x76,0x69,0x63,0x65,0x44,0x52,0x02,0x65,0x54,0x08,0x05,0x01,0x63,0x00,0x00,0x03,0x70,0x64,0x3E,0x00
	,0x00};
/* ConnectionManager */
const int UPnPConnectionManagerDescriptionLengthUX = 4793;
const int UPnPConnectionManagerDescriptionLength = 1098;
const char UPnPConnectionManagerDescription[1098] = {
	0x3F,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x20,0x32,0x30,0x30,0x20,0x20,0x4F,0x4B,0x0D,0x0A,0x43
	,0x4F,0x4E,0x54,0x45,0x4E,0x54,0x2D,0x54,0x59,0x50,0x45,0x3A,0x20,0x20,0x74,0x65,0x78,0x74,0x2F,0x78
	,0x6D,0x6C,0x0D,0x0A,0x53,0x65,0x72,0x76,0x65,0x72,0x3A,0x20,0x57,0x49,0x4E,0x44,0x4F,0x57,0x53,0x2C
	,0x20,0x55,0x50,0x6E,0x04,0x0F,0x13,0x30,0x2C,0x20,0x49,0x6E,0x74,0x65,0x6C,0x20,0x4D,0x69,0x63,0x72
	,0x6F,0x53,0x74,0x61,0x63,0x6B,0x84,0x05,0x7A,0x2E,0x31,0x32,0x36,0x34,0x0D,0x0A,0x43,0x6F,0x6E,0x74
	,0x65,0x6E,0x74,0x2D,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3A,0x20,0x34,0x36,0x37,0x32,0x0D,0x0A,0x0D,0x0A
	,0x3C,0x3F,0x78,0x6D,0x6C,0x20,0x76,0x65,0x72,0x73,0x69,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20
	,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x75,0x74,0x66,0x2D,0x38,0x22,0x3F,0x3E,0x3C,0x73
	,0x63,0x70,0x64,0x20,0x78,0x6D,0x6C,0x6E,0x73,0x3D,0x22,0x75,0x72,0x6E,0x3A,0x73,0x63,0x68,0x65,0x6D
	,0x61,0x73,0x2D,0x75,0x70,0x6E,0x70,0x2D,0x6F,0x72,0x67,0x3A,0x73,0x65,0x72,0x76,0x69,0x63,0x65,0x2D
	,0x31,0x2D,0x30,0x22,0x3E,0x3C,0x73,0x70,0x65,0x63,0x56,0x06,0x15,0x0B,0x3E,0x3C,0x6D,0x61,0x6A,0x6F
	,0x72,0x3E,0x31,0x3C,0x2F,0x46,0x02,0x0A,0x3C,0x6D,0x69,0x6E,0x6F,0x72,0x3E,0x30,0x3C,0x2F,0x46,0x02
	,0x02,0x3C,0x2F,0x8D,0x0B,0x0A,0x61,0x63,0x74,0x69,0x6F,0x6E,0x4C,0x69,0x73,0x74,0x08,0x03,0x16,0x3E
	,0x3C,0x6E,0x61,0x6D,0x65,0x3E,0x47,0x65,0x74,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x43,0x6F,0x6E,0x6E
	,0x65,0xC5,0x09,0x06,0x49,0x6E,0x66,0x6F,0x3C,0x2F,0xC5,0x07,0x09,0x3C,0x61,0x72,0x67,0x75,0x6D,0x65
	,0x6E,0x74,0xC7,0x0E,0x00,0x87,0x03,0x00,0x47,0x0F,0x00,0xCB,0x0C,0x01,0x44,0x48,0x0C,0x03,0x64,0x69
	,0x72,0x86,0x11,0x05,0x3E,0x69,0x6E,0x3C,0x2F,0x8A,0x03,0x1C,0x3C,0x72,0x65,0x6C,0x61,0x74,0x65,0x64
	,0x53,0x74,0x61,0x74,0x65,0x56,0x61,0x72,0x69,0x61,0x62,0x6C,0x65,0x3E,0x41,0x5F,0x41,0x52,0x47,0x5F
	,0x84,0x63,0x01,0x5F,0x4E,0x13,0x00,0x95,0x0B,0x02,0x3C,0x2F,0x4A,0x20,0x00,0xCF,0x22,0x03,0x52,0x63
	,0x73,0x14,0x21,0x03,0x6F,0x75,0x74,0x6D,0x21,0x03,0x52,0x63,0x73,0xB4,0x1F,0x0B,0x41,0x56,0x54,0x72
	,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0xBF,0x21,0x00,0xC5,0x42,0x00,0xCF,0x13,0x00,0x30,0x43,0x08,0x50
	,0x72,0x6F,0x74,0x6F,0x63,0x6F,0x6C,0x0C,0x72,0x00,0xFA,0x44,0x00,0x8E,0x13,0x00,0x31,0x23,0x03,0x65
	,0x65,0x72,0x8A,0x96,0x07,0x4D,0x61,0x6E,0x61,0x67,0x65,0x72,0x3F,0x6A,0x03,0x50,0x45,0x5F,0xD3,0x14
	,0x00,0xBE,0x26,0x00,0x7F,0x8F,0x00,0xBF,0xB0,0x00,0x84,0xE2,0x01,0x44,0x48,0xCE,0x00,0xBF,0xB1,0x03
	,0x50,0x45,0x5F,0xCB,0x12,0x00,0x30,0xD2,0x00,0xC4,0xE7,0x02,0x75,0x73,0x7F,0x68,0x00,0x8D,0xF3,0x00
	,0x88,0x14,0x00,0xA1,0xF4,0x00,0x49,0xF7,0x03,0x4C,0x69,0x73,0xC5,0x03,0x00,0x07,0xED,0x00,0x08,0x02
	,0x00,0x07,0xB7,0x07,0x65,0x70,0x61,0x72,0x65,0x46,0x6F,0x8B,0x95,0x00,0xC8,0xFD,0x00,0x0E,0x10,0x00
	,0xCF,0xE8,0x06,0x52,0x65,0x6D,0x6F,0x74,0x65,0xDE,0xC6,0x02,0x69,0x6E,0xBF,0xC6,0x00,0xBF,0xC6,0x02
	,0x65,0x72,0x3F,0x25,0x00,0x7F,0xC6,0x00,0x68,0xC6,0x02,0x69,0x6E,0x3F,0xC6,0x00,0x3F,0xC6,0x00,0x37
	,0x6C,0x00,0xFB,0xC5,0x00,0x1E,0x44,0x03,0x6F,0x75,0x74,0x7F,0x44,0x00,0xEC,0xE8,0x0B,0x41,0x56,0x54
	,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72,0x74,0x7F,0x23,0x00,0x85,0xEA,0x00,0xCF,0x13,0x00,0xF0,0xB1,0x03
	,0x52,0x63,0x73,0xFF,0x44,0x00,0x05,0xD3,0x03,0x52,0x63,0x73,0x65,0x87,0x01,0x2F,0x4E,0xF9,0x02,0x2F
	,0x61,0x07,0xEB,0x00,0x08,0x02,0x00,0x8F,0x6B,0x08,0x43,0x6F,0x6D,0x70,0x6C,0x65,0x74,0x65,0x08,0xFB
	,0x00,0x8E,0x0F,0x00,0x6D,0x79,0x00,0x7F,0xBD,0x00,0x3F,0x36,0x00,0x85,0xE6,0x0F,0x47,0x65,0x74,0x50
	,0x72,0x6F,0x74,0x6F,0x63,0x6F,0x6C,0x49,0x6E,0x66,0x6F,0x65,0x35,0x06,0x53,0x6F,0x75,0x72,0x63,0x65
	,0x37,0xAD,0x00,0x46,0x0F,0x00,0x0E,0x1D,0x00,0x30,0xF0,0x04,0x53,0x69,0x6E,0x6B,0xF8,0x1F,0x03,0x69
	,0x6E,0x6B,0x6F,0x1F,0x00,0x28,0x52,0x07,0x43,0x75,0x72,0x72,0x65,0x6E,0x74,0x0C,0xF6,0x01,0x73,0x71
	,0x89,0x01,0x73,0xB7,0xDF,0x00,0x56,0x1F,0x00,0x39,0xBF,0x00,0x47,0xC1,0x00,0x86,0xC4,0x07,0x73,0x65
	,0x72,0x76,0x69,0x63,0x65,0xC5,0xFB,0x01,0x54,0x46,0xEF,0x01,0x73,0xCC,0xFE,0x10,0x20,0x73,0x65,0x6E
	,0x64,0x45,0x76,0x65,0x6E,0x74,0x73,0x3D,0x22,0x6E,0x6F,0x22,0xC7,0xF1,0x00,0xCB,0xE2,0x00,0xD4,0x98
	,0x11,0x64,0x61,0x74,0x61,0x54,0x79,0x70,0x65,0x3E,0x73,0x74,0x72,0x69,0x6E,0x67,0x3C,0x2F,0x49,0x04
	,0x03,0x3C,0x2F,0x73,0x4E,0xEB,0x00,0xAF,0x1B,0x00,0x0A,0xEB,0x00,0x44,0xFD,0x02,0x75,0x73,0xA3,0x1C
	,0x0C,0x61,0x6C,0x6C,0x6F,0x77,0x65,0x64,0x56,0x61,0x6C,0x75,0x65,0x47,0xF2,0x00,0x8B,0x04,0x05,0x3E
	,0x4F,0x4B,0x3C,0x2F,0x4D,0x04,0x00,0xCE,0x07,0x15,0x43,0x6F,0x6E,0x74,0x65,0x6E,0x74,0x46,0x6F,0x72
	,0x6D,0x61,0x74,0x4D,0x69,0x73,0x6D,0x61,0x74,0x63,0x68,0x9D,0x0C,0x14,0x49,0x6E,0x73,0x75,0x66,0x66
	,0x69,0x63,0x69,0x65,0x6E,0x74,0x42,0x61,0x6E,0x64,0x77,0x69,0x64,0x74,0x9E,0x0C,0x05,0x55,0x6E,0x72
	,0x65,0x6C,0x45,0xF9,0x07,0x43,0x68,0x61,0x6E,0x6E,0x65,0x6C,0x9F,0x0B,0x05,0x6B,0x6E,0x6F,0x77,0x6E
	,0x90,0x2D,0x01,0x2F,0x12,0x3A,0x00,0x3F,0x5B,0x0D,0x41,0x56,0x54,0x72,0x61,0x6E,0x73,0x70,0x6F,0x72
	,0x74,0x49,0x44,0xD1,0x76,0x02,0x69,0x34,0xFF,0x75,0x00,0x4C,0x91,0x03,0x52,0x63,0x73,0xFF,0x18,0x00
	,0xAB,0x8E,0x00,0xBF,0x1A,0x00,0x2B,0xA9,0x07,0x4D,0x61,0x6E,0x61,0x67,0x65,0x72,0xFF,0xC5,0x00,0x4E
	,0xE1,0x03,0x79,0x65,0x73,0x88,0xE1,0x06,0x53,0x6F,0x75,0x72,0x63,0x65,0x7F,0xE0,0x00,0xA6,0x1A,0x03
	,0x69,0x6E,0x6B,0x7F,0xFA,0x00,0x6F,0xFA,0x03,0x44,0x69,0x72,0x06,0xFA,0x00,0xBF,0xF8,0x00,0x45,0xE4
	,0x03,0x70,0x75,0x74,0x5D,0xF9,0x06,0x4F,0x75,0x74,0x70,0x75,0x74,0xBF,0xD4,0x00,0xD9,0x69,0x07,0x43
	,0x75,0x72,0x72,0x65,0x6E,0x74,0x4C,0xA0,0x01,0x73,0xF3,0x84,0x09,0x2F,0x73,0x65,0x72,0x76,0x69,0x63
	,0x65,0x53,0x44,0xF5,0x01,0x54,0x08,0x05,0x01,0x63,0x00,0x00,0x03,0x70,0x64,0x3E,0x00,0x00};

struct UPnPDataObject;

struct HTTPReaderObject
{
	char Header[2048];
	char* Body;
	struct packetheader *ParsedHeader;
	int BodySize;
	int HeaderIndex;
	int BodyIndex;
	SOCKET ClientSocket;
	int FinRead;
	struct UPnPDataObject *Parent;
};
struct SubscriberInfo
{
	char* SID;
	int SIDLength;
	int SEQ;
	
	int Address;
	unsigned short Port;
	char* Path;
	int PathLength;
	int RefCount;
	int Disposing;
	
	unsigned int RenewByTime;
	struct SubscriberInfo *Next;
	struct SubscriberInfo *Previous;
};
struct UPnPDataObject
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	
	void *EventClient;
	void *Chain;
	int UpdateFlag;
	
	
	int ForceExit;
	char *UUID;
	char *UDN;
	char *Serial;
	
	void *WebServerTimer;
	
	char *DeviceDescription;
	int DeviceDescriptionLength;
	int InitialNotify;
	char* ConnectionManager_SourceProtocolInfo;
	char* ConnectionManager_SinkProtocolInfo;
	char* ConnectionManager_CurrentConnectionIDs;
	char* AVTransport_LastChange;
	char* RenderingControl_LastChange;
	struct sockaddr_in addr;
	int addrlen;
	SOCKET MSEARCH_sock;
	struct ip_mreq mreq;
	char message[4096];
	int *AddressList;
	int AddressListLength;
	
	int _NumEmbeddedDevices;
	SOCKET WebSocket;
	int WebSocketPortNumber;
	struct HTTPReaderObject ReaderObjects[5];
	SOCKET *NOTIFY_SEND_socks;
	SOCKET NOTIFY_RECEIVE_sock;
	
	int SID;
	
	unsigned int CurrentTime;
	int NotifyCycleTime;
	unsigned int NotifyTime;
	
	sem_t EventLock;
	struct SubscriberInfo *HeadSubscriberPtr_ConnectionManager;
	int NumberOfSubscribers_ConnectionManager;
	struct SubscriberInfo *HeadSubscriberPtr_AVTransport;
	int NumberOfSubscribers_AVTransport;
	struct SubscriberInfo *HeadSubscriberPtr_RenderingControl;
	int NumberOfSubscribers_RenderingControl;
};

struct MSEARCH_state
{
	char *ST;
	int STLength;
	void *upnp;
	struct sockaddr_in dest_addr;
};

/* Pre-declarations */
void UPnPSendNotify(const struct UPnPDataObject *upnp);
void UPnPSendByeBye();
void UPnPMainInvokeSwitch();
void UPnPSendDataXmlEscaped(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
void UPnPSendData(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate);
int UPnPPeriodicNotify(struct UPnPDataObject *upnp);
void UPnPSendEvent_Body(void *upnptoken, char *body, int bodylength, struct SubscriberInfo *info);

void* UPnPGetInstance(const void* UPnPToken)
{
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	return (void*)(ReaderObject->Parent);
}

#define UPnPBuildSsdpResponsePacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,ST,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"HTTP/1.1 200 OK\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nEXT:\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nST: %s%s\r\n\r\n" ,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,ST,NTex);\
}

#define UPnPBuildSsdpNotifyPacket(outpacket,outlenght,ipaddr,port,EmbeddedDeviceNumber,USN,USNex,NT,NTex,NotifyTime)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.1\r\nLOCATION: http://%d.%d.%d.%d:%d/\r\nHOST: 239.255.255.250:1900\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264\r\nNTS: ssdp:alive\r\nUSN: uuid:%s%s\r\nCACHE-CONTROL: max-age=%d\r\nNT: %s%s\r\n\r\n",(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),port,USN,USNex,NotifyTime,NT,NTex);\
}

void UPnPIPAddressListChanged(void *MicroStackToken)
{
	((struct UPnPDataObject*)MicroStackToken)->UpdateFlag = 1;
	ILibForceUnBlockChain(((struct UPnPDataObject*)MicroStackToken)->Chain);
}
void UPnPInit(struct UPnPDataObject *state,const int NotifyCycleSeconds,const unsigned short PortNumber)
{
	int ra = 1;
	int i;
	int flags;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	unsigned char TTL = 4;
	
	/* Complete State Reset */
	memset(state,0,sizeof(struct UPnPDataObject));
	
	/* Setup Notification Timer */
	state->NotifyCycleTime = NotifyCycleSeconds;
	state->CurrentTime = GetTickCount() / 1000;
	state->NotifyTime = state->CurrentTime  + (state->NotifyCycleTime/2);
	
	/* Setup WebSocket */
	if(PortNumber!=0)
	{
		memset((char *)&(addr), 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = (unsigned short)htons(PortNumber);
		state->WebSocket = socket(AF_INET, SOCK_STREAM, 0);
		flags = 1;
		ioctlsocket(state->WebSocket,FIONBIO,&flags);
		if (setsockopt(state->WebSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&ra, sizeof(ra)) < 0)
		{
			printf("Setting SockOpt SO_REUSEADDR failed (HTTP)");
			exit(1);
		}
		if (bind(state->WebSocket, (struct sockaddr *) &(addr), sizeof(addr)) < 0)
		{
			printf("WebSocket bind");
			exit(1);
		}
		state->WebSocketPortNumber = PortNumber;
	}
	else
	{
		state->WebSocketPortNumber = ILibGetStreamSocket(htonl(INADDR_ANY),0,(HANDLE*)&(state->WebSocket));
		flags = 1;
		ioctlsocket(state->WebSocket,FIONBIO,&flags);
	}
	if (listen(state->WebSocket,5)!=0)
	{
		printf("WebSocket listen");
		exit(1);
	}
	memset((char *)&(state->addr), 0, sizeof(state->addr));
	state->addr.sin_family = AF_INET;
	state->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	state->addr.sin_port = (unsigned short)htons(UPNP_PORT);
	state->addrlen = sizeof(state->addr);
	/* Set up socket */
	state->AddressListLength = ILibGetLocalIPAddressList(&(state->AddressList));
	state->NOTIFY_SEND_socks = (SOCKET*)MALLOC(sizeof(int)*(state->AddressListLength));
	state->NOTIFY_RECEIVE_sock = socket(AF_INET, SOCK_DGRAM, 0);
	memset((char *)&(addr), 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = (unsigned short)htons(UPNP_PORT);
	if (setsockopt(state->NOTIFY_RECEIVE_sock, SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) < 0)
	{
		printf("Setting SockOpt SO_REUSEADDR failed\r\n");
		exit(1);
	}
	if (bind(state->NOTIFY_RECEIVE_sock, (struct sockaddr *) &(addr), sizeof(addr)) < 0)
	{
		printf("Could not bind to UPnP Listen Port\r\n");
		exit(1);
	}
	for(i=0;i<state->AddressListLength;++i)
	{
		state->NOTIFY_SEND_socks[i] = socket(AF_INET, SOCK_DGRAM, 0);
		memset((char *)&(addr), 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = state->AddressList[i];
		addr.sin_port = (unsigned short)htons(UPNP_PORT);
		if (setsockopt(state->NOTIFY_SEND_socks[i], SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) == 0)
		{
			if (setsockopt(state->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL)) < 0)
			{
				/* Ignore this case */
			}
			if (bind(state->NOTIFY_SEND_socks[i], (struct sockaddr *) &(addr), sizeof(addr)) == 0)
			{
				mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);
				mreq.imr_interface.s_addr = state->AddressList[i];
				if (setsockopt(state->NOTIFY_RECEIVE_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)
				{
					/* Does not matter */
				}
			}
		}
	}
}
void UPnPPostMX_Destroy(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	FREE(mss->ST);
	FREE(mss);
}
void UPnPPostMX_MSEARCH(void *object)
{
	struct MSEARCH_state *mss = (struct MSEARCH_state*)object;
	
	char *b = (char*)MALLOC(sizeof(char)*5000);
	int packetlength;
	struct sockaddr_in response_addr;
	int response_addrlen;
	SOCKET *response_socket;
	int cnt;
	int i;
	struct sockaddr_in dest_addr = mss->dest_addr;
	char *ST = mss->ST;
	int STLength = mss->STLength;
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)mss->upnp;
	
	response_socket = (SOCKET*)MALLOC(upnp->AddressListLength*sizeof(int));
	for(i=0;i<upnp->AddressListLength;++i)
	{
		response_socket[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if (response_socket[i]< 0)
		{
			printf("response socket");
			exit(1);
		}
		memset((char *)&(response_addr), 0, sizeof(response_addr));
		response_addr.sin_family = AF_INET;
		response_addr.sin_addr.s_addr = upnp->AddressList[i];
		response_addr.sin_port = (unsigned short)htons(0);
		response_addrlen = sizeof(response_addr);	
		if (bind(response_socket[i], (struct sockaddr *) &(response_addr), sizeof(response_addr)) < 0)
		{
			/* Ignore if this happens */
		}
	}
	if(STLength==15 && memcmp(ST,"upnp:rootdevice",15)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	else if(STLength==8 && memcmp(ST,"ssdp:all",8)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength==(int)strlen(upnp->UUID) && memcmp(ST,upnp->UUID,(int)strlen(upnp->UUID))==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"",upnp->UUID,"",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=43 && memcmp(ST,"urn:schemas-upnp-org:device:MediaRenderer:1",43)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=47 && memcmp(ST,"urn:schemas-upnp-org:service:RenderingControl:1",47)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=42 && memcmp(ST,"urn:schemas-upnp-org:service:AVTransport:1",42)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	if(STLength>=48 && memcmp(ST,"urn:schemas-upnp-org:service:ConnectionManager:1",48)==0)
	{
		for(i=0;i<upnp->AddressListLength;++i)
		{
			UPnPBuildSsdpResponsePacket(b,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
			cnt = sendto(response_socket[i], b, packetlength, 0,
			(struct sockaddr *) &dest_addr, sizeof(dest_addr));
		}
	}
	for(i=0;i<upnp->AddressListLength;++i)
	{
		closesocket(response_socket[i]);
	}
	FREE(response_socket);
	FREE(mss->ST);
	FREE(mss);
	FREE(b);
}
void UPnPProcessMSEARCH(struct UPnPDataObject *upnp, struct packetheader *packet)
{
	char* ST = NULL;
	int STLength = 0;
	struct packetheader_field_node *node;
	int MANOK = 0;
	unsigned long MXVal;
	int MXOK = 0;
	int MX;
	struct MSEARCH_state *mss = NULL;
	
	if(memcmp(packet->DirectiveObj,"*",1)==0)
	{
		if(memcmp(packet->Version,"1.1",3)==0)
		{
			node = packet->FirstField;
			while(node!=NULL)
			{
				if(_strnicmp(node->Field,"ST",2)==0)
				{
					ST = (char*)MALLOC(1+node->FieldDataLength);
					memcpy(ST,node->FieldData,node->FieldDataLength);
					ST[node->FieldDataLength] = 0;
					STLength = node->FieldDataLength;
				}
				else if(_strnicmp(node->Field,"MAN",3)==0 && memcmp(node->FieldData,"\"ssdp:discover\"",15)==0)
				{
					MANOK = 1;
				}
				else if(_strnicmp(node->Field,"MX",2)==0 && ILibGetULong(node->FieldData,node->FieldDataLength,&MXVal)==0)
				{
					MXOK = 1;
					MXVal = MXVal>10?10:MXVal;
				}
				node = node->NextField;
			}
			if(MANOK!=0 && MXOK!=0)
			{
				MX = (int)(0 + ((unsigned short)rand() % MXVal));
				mss = (struct MSEARCH_state*)MALLOC(sizeof(struct MSEARCH_state));
				mss->ST = ST;
				mss->STLength = STLength;
				mss->upnp = upnp;
				memset((char *)&(mss->dest_addr), 0, sizeof(mss->dest_addr));
				mss->dest_addr.sin_family = AF_INET;
				mss->dest_addr.sin_addr = packet->Source->sin_addr;
				mss->dest_addr.sin_port = packet->Source->sin_port;
				
				ILibLifeTime_Add(upnp->WebServerTimer,mss,MX,&UPnPPostMX_MSEARCH,&UPnPPostMX_Destroy);
			}
			else
			{
				FREE(ST);
			}
		}
	}
}
void UPnPDispatch_ConnectionManager_GetCurrentConnectionInfo(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	int OK = 0;
	char *p_ConnectionID = NULL;
	int p_ConnectionIDLength = 0;
	int _ConnectionID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==12 && memcmp(VarName,"ConnectionID",12) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ConnectionID = temp3->LastResult->data;
					p_ConnectionIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetLong(p_ConnectionID,p_ConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[ConnectionID] illegal value");
		return;
	}
	_ConnectionID = (int)TempLong;
	UPnPFP_ConnectionManager_GetCurrentConnectionInfo((void*)ReaderObject,_ConnectionID);
}

void UPnPDispatch_ConnectionManager_PrepareForConnection(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	int OK = 0;
	char *p_RemoteProtocolInfo = NULL;
	int p_RemoteProtocolInfoLength = 0;
	char* _RemoteProtocolInfo = "";
	int _RemoteProtocolInfoLength;
	char *p_PeerConnectionManager = NULL;
	int p_PeerConnectionManagerLength = 0;
	char* _PeerConnectionManager = "";
	int _PeerConnectionManagerLength;
	char *p_PeerConnectionID = NULL;
	int p_PeerConnectionIDLength = 0;
	int _PeerConnectionID = 0;
	char *p_Direction = NULL;
	int p_DirectionLength = 0;
	char* _Direction = "";
	int _DirectionLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==18 && memcmp(VarName,"RemoteProtocolInfo",18) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_RemoteProtocolInfo = temp3->LastResult->data;
					p_RemoteProtocolInfoLength = temp3->LastResult->datalength;
					p_RemoteProtocolInfo[p_RemoteProtocolInfoLength] = 0;
				}
				else
				{
					p_RemoteProtocolInfo = temp3->LastResult->data;
					p_RemoteProtocolInfoLength = 0;
					p_RemoteProtocolInfo[p_RemoteProtocolInfoLength] = 0;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==21 && memcmp(VarName,"PeerConnectionManager",21) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_PeerConnectionManager = temp3->LastResult->data;
					p_PeerConnectionManagerLength = temp3->LastResult->datalength;
					p_PeerConnectionManager[p_PeerConnectionManagerLength] = 0;
				}
				else
				{
					p_PeerConnectionManager = temp3->LastResult->data;
					p_PeerConnectionManagerLength = 0;
					p_PeerConnectionManager[p_PeerConnectionManagerLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==16 && memcmp(VarName,"PeerConnectionID",16) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_PeerConnectionID = temp3->LastResult->data;
					p_PeerConnectionIDLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==9 && memcmp(VarName,"Direction",9) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Direction = temp3->LastResult->data;
					p_DirectionLength = temp3->LastResult->datalength;
					p_Direction[p_DirectionLength] = 0;
				}
				else
				{
					p_Direction = temp3->LastResult->data;
					p_DirectionLength = 0;
					p_Direction[p_DirectionLength] = 0;
				}
				OK |= 8;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 15)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	_RemoteProtocolInfoLength = ILibInPlaceXmlUnEscape(p_RemoteProtocolInfo);
	_RemoteProtocolInfo = p_RemoteProtocolInfo;
	_PeerConnectionManagerLength = ILibInPlaceXmlUnEscape(p_PeerConnectionManager);
	_PeerConnectionManager = p_PeerConnectionManager;
	OK = ILibGetLong(p_PeerConnectionID,p_PeerConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[PeerConnectionID] illegal value");
		return;
	}
	_PeerConnectionID = (int)TempLong;
	_DirectionLength = ILibInPlaceXmlUnEscape(p_Direction);
	_Direction = p_Direction;
	if(memcmp(_Direction, "Input\0",6) != 0
	&& memcmp(_Direction, "Output\0",7) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Direction] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_ConnectionManager_PrepareForConnection((void*)ReaderObject,_RemoteProtocolInfo,_PeerConnectionManager,_PeerConnectionID,_Direction);
}

void UPnPDispatch_ConnectionManager_ConnectionComplete(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	int OK = 0;
	char *p_ConnectionID = NULL;
	int p_ConnectionIDLength = 0;
	int _ConnectionID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==12 && memcmp(VarName,"ConnectionID",12) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_ConnectionID = temp3->LastResult->data;
					p_ConnectionIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetLong(p_ConnectionID,p_ConnectionIDLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[ConnectionID] illegal value");
		return;
	}
	_ConnectionID = (int)TempLong;
	UPnPFP_ConnectionManager_ConnectionComplete((void*)ReaderObject,_ConnectionID);
}

#define UPnPDispatch_ConnectionManager_GetProtocolInfo(xml, ReaderObject)\
{\
	UPnPFP_ConnectionManager_GetProtocolInfo((void*)ReaderObject);\
}

#define UPnPDispatch_ConnectionManager_GetCurrentConnectionIDs(xml, ReaderObject)\
{\
	UPnPFP_ConnectionManager_GetCurrentConnectionIDs((void*)ReaderObject);\
}

void UPnPDispatch_AVTransport_GetCurrentTransportActions(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_GetCurrentTransportActions((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_Play(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Speed = NULL;
	int p_SpeedLength = 0;
	char* _Speed = "";
	int _SpeedLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==5 && memcmp(VarName,"Speed",5) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Speed = temp3->LastResult->data;
					p_SpeedLength = temp3->LastResult->datalength;
					p_Speed[p_SpeedLength] = 0;
				}
				else
				{
					p_Speed = temp3->LastResult->data;
					p_SpeedLength = 0;
					p_Speed[p_SpeedLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_SpeedLength = ILibInPlaceXmlUnEscape(p_Speed);
	_Speed = p_Speed;
	if(memcmp(_Speed, "1\0",2) != 0
	&& memcmp(_Speed, " vendor-defined \0",17) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Speed] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_AVTransport_Play((void*)ReaderObject,_InstanceID,_Speed);
}

void UPnPDispatch_AVTransport_Previous(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_Previous((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_Next(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_Next((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_Stop(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_Stop((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_GetTransportSettings(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_GetTransportSettings((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_Seek(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Unit = NULL;
	int p_UnitLength = 0;
	char* _Unit = "";
	int _UnitLength;
	char *p_Target = NULL;
	int p_TargetLength = 0;
	char* _Target = "";
	int _TargetLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==4 && memcmp(VarName,"Unit",4) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Unit = temp3->LastResult->data;
					p_UnitLength = temp3->LastResult->datalength;
					p_Unit[p_UnitLength] = 0;
				}
				else
				{
					p_Unit = temp3->LastResult->data;
					p_UnitLength = 0;
					p_Unit[p_UnitLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==6 && memcmp(VarName,"Target",6) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Target = temp3->LastResult->data;
					p_TargetLength = temp3->LastResult->datalength;
					p_Target[p_TargetLength] = 0;
				}
				else
				{
					p_Target = temp3->LastResult->data;
					p_TargetLength = 0;
					p_Target[p_TargetLength] = 0;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_UnitLength = ILibInPlaceXmlUnEscape(p_Unit);
	_Unit = p_Unit;
	if(memcmp(_Unit, "ABS_TIME\0",9) != 0
	&& memcmp(_Unit, "REL_TIME\0",9) != 0
	&& memcmp(_Unit, "ABS_COUNT\0",10) != 0
	&& memcmp(_Unit, "REL_COUNT\0",10) != 0
	&& memcmp(_Unit, "TRACK_NR\0",9) != 0
	&& memcmp(_Unit, "CHANNEL_FREQ\0",13) != 0
	&& memcmp(_Unit, "TAPE-INDEX\0",11) != 0
	&& memcmp(_Unit, "FRAME\0",6) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Unit] contains a value that is not in AllowedValueList");
		return;
	}
	_TargetLength = ILibInPlaceXmlUnEscape(p_Target);
	_Target = p_Target;
	UPnPFP_AVTransport_Seek((void*)ReaderObject,_InstanceID,_Unit,_Target);
}

void UPnPDispatch_AVTransport_Pause(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_Pause((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_GetPositionInfo(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_GetPositionInfo((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_GetTransportInfo(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_GetTransportInfo((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_SetAVTransportURI(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_CurrentURI = NULL;
	int p_CurrentURILength = 0;
	char* _CurrentURI = "";
	int _CurrentURILength;
	char *p_CurrentURIMetaData = NULL;
	int p_CurrentURIMetaDataLength = 0;
	char* _CurrentURIMetaData = "";
	int _CurrentURIMetaDataLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==10 && memcmp(VarName,"CurrentURI",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_CurrentURI = temp3->LastResult->data;
					p_CurrentURILength = temp3->LastResult->datalength;
					p_CurrentURI[p_CurrentURILength] = 0;
				}
				else
				{
					p_CurrentURI = temp3->LastResult->data;
					p_CurrentURILength = 0;
					p_CurrentURI[p_CurrentURILength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==18 && memcmp(VarName,"CurrentURIMetaData",18) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_CurrentURIMetaData = temp3->LastResult->data;
					p_CurrentURIMetaDataLength = temp3->LastResult->datalength;
					p_CurrentURIMetaData[p_CurrentURIMetaDataLength] = 0;
				}
				else
				{
					p_CurrentURIMetaData = temp3->LastResult->data;
					p_CurrentURIMetaDataLength = 0;
					p_CurrentURIMetaData[p_CurrentURIMetaDataLength] = 0;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_CurrentURILength = ILibInPlaceXmlUnEscape(p_CurrentURI);
	_CurrentURI = p_CurrentURI;
	_CurrentURIMetaDataLength = ILibInPlaceXmlUnEscape(p_CurrentURIMetaData);
	_CurrentURIMetaData = p_CurrentURIMetaData;
	UPnPFP_AVTransport_SetAVTransportURI((void*)ReaderObject,_InstanceID,_CurrentURI,_CurrentURIMetaData);
}

void UPnPDispatch_AVTransport_GetDeviceCapabilities(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_GetDeviceCapabilities((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_AVTransport_SetPlayMode(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_NewPlayMode = NULL;
	int p_NewPlayModeLength = 0;
	char* _NewPlayMode = "";
	int _NewPlayModeLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==11 && memcmp(VarName,"NewPlayMode",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_NewPlayMode = temp3->LastResult->data;
					p_NewPlayModeLength = temp3->LastResult->datalength;
					p_NewPlayMode[p_NewPlayModeLength] = 0;
				}
				else
				{
					p_NewPlayMode = temp3->LastResult->data;
					p_NewPlayModeLength = 0;
					p_NewPlayMode[p_NewPlayModeLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_NewPlayModeLength = ILibInPlaceXmlUnEscape(p_NewPlayMode);
	_NewPlayMode = p_NewPlayMode;
	if(memcmp(_NewPlayMode, "NORMAL\0",7) != 0
	&& memcmp(_NewPlayMode, "REPEAT_ALL\0",11) != 0
	&& memcmp(_NewPlayMode, "INTRO\0",6) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[NewPlayMode] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_AVTransport_SetPlayMode((void*)ReaderObject,_InstanceID,_NewPlayMode);
}

void UPnPDispatch_AVTransport_GetMediaInfo(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_AVTransport_GetMediaInfo((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetHorizontalKeystone(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetHorizontalKeystone((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetVolume(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_RenderingControl_GetVolume((void*)ReaderObject,_InstanceID,_Channel);
}

void UPnPDispatch_RenderingControl_SelectPreset(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_PresetName = NULL;
	int p_PresetNameLength = 0;
	char* _PresetName = "";
	int _PresetNameLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==10 && memcmp(VarName,"PresetName",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_PresetName = temp3->LastResult->data;
					p_PresetNameLength = temp3->LastResult->datalength;
					p_PresetName[p_PresetNameLength] = 0;
				}
				else
				{
					p_PresetName = temp3->LastResult->data;
					p_PresetNameLength = 0;
					p_PresetName[p_PresetNameLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_PresetNameLength = ILibInPlaceXmlUnEscape(p_PresetName);
	_PresetName = p_PresetName;
	if(memcmp(_PresetName, "FactoryDefaults\0",16) != 0
	&& memcmp(_PresetName, "InstallationDefaults\0",21) != 0
	&& memcmp(_PresetName, "Vendor defined\0",15) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[PresetName] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_RenderingControl_SelectPreset((void*)ReaderObject,_InstanceID,_PresetName);
}

void UPnPDispatch_RenderingControl_SetVolume(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	char *p_DesiredVolume = NULL;
	int p_DesiredVolumeLength = 0;
	unsigned short _DesiredVolume = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==13 && memcmp(VarName,"DesiredVolume",13) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredVolume = temp3->LastResult->data;
					p_DesiredVolumeLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	OK = ILibGetULong(p_DesiredVolume,p_DesiredVolumeLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredVolume] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredVolume] out of Range");
			return;
		}
		_DesiredVolume = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetVolume((void*)ReaderObject,_InstanceID,_Channel,_DesiredVolume);
}

void UPnPDispatch_RenderingControl_ListPresets(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_ListPresets((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_SetVolumeDB(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	char *p_DesiredVolume = NULL;
	int p_DesiredVolumeLength = 0;
	short _DesiredVolume = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==13 && memcmp(VarName,"DesiredVolume",13) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredVolume = temp3->LastResult->data;
					p_DesiredVolumeLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	OK = ILibGetLong(p_DesiredVolume,p_DesiredVolumeLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredVolume] illegal value");
		return;
	}
	else
	{
		if(!(TempLong>=(long)0xFFFF8000 && TempLong<=(long)0x7FFF))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredVolume] out of Range");
			return;
		}
		_DesiredVolume = (short)TempLong;
	}
	UPnPFP_RenderingControl_SetVolumeDB((void*)ReaderObject,_InstanceID,_Channel,_DesiredVolume);
}

void UPnPDispatch_RenderingControl_SetRedVideoBlackLevel(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredRedVideoBlackLevel = NULL;
	int p_DesiredRedVideoBlackLevelLength = 0;
	unsigned short _DesiredRedVideoBlackLevel = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==25 && memcmp(VarName,"DesiredRedVideoBlackLevel",25) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredRedVideoBlackLevel = temp3->LastResult->data;
					p_DesiredRedVideoBlackLevelLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredRedVideoBlackLevel,p_DesiredRedVideoBlackLevelLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredRedVideoBlackLevel] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredRedVideoBlackLevel] out of Range");
			return;
		}
		_DesiredRedVideoBlackLevel = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetRedVideoBlackLevel((void*)ReaderObject,_InstanceID,_DesiredRedVideoBlackLevel);
}

void UPnPDispatch_RenderingControl_SetContrast(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredContrast = NULL;
	int p_DesiredContrastLength = 0;
	unsigned short _DesiredContrast = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==15 && memcmp(VarName,"DesiredContrast",15) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredContrast = temp3->LastResult->data;
					p_DesiredContrastLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredContrast,p_DesiredContrastLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredContrast] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredContrast] out of Range");
			return;
		}
		_DesiredContrast = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetContrast((void*)ReaderObject,_InstanceID,_DesiredContrast);
}

void UPnPDispatch_RenderingControl_SetLoudness(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	char *p_DesiredLoudness = NULL;
	int p_DesiredLoudnessLength = 0;
	int _DesiredLoudness = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==15 && memcmp(VarName,"DesiredLoudness",15) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredLoudness = temp3->LastResult->data;
					p_DesiredLoudnessLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	OK=0;
	if(p_DesiredLoudnessLength==4)
	{
		if(_strnicmp(p_DesiredLoudness,"true",4)==0)
		{
			OK = 1;
			_DesiredLoudness = 1;
		}
	}
	if(p_DesiredLoudnessLength==5)
	{
		if(_strnicmp(p_DesiredLoudness,"false",5)==0)
		{
			OK = 1;
			_DesiredLoudness = 0;
		}
	}
	if(p_DesiredLoudnessLength==1)
	{
		if(memcmp(p_DesiredLoudness,"0",1)==0)
		{
			OK = 1;
			_DesiredLoudness = 0;
		}
		if(memcmp(p_DesiredLoudness,"1",1)==0)
		{
			OK = 1;
			_DesiredLoudness = 1;
		}
	}
	if(OK==0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredLoudness] illegal value");
		return;
	}
	UPnPFP_RenderingControl_SetLoudness((void*)ReaderObject,_InstanceID,_Channel,_DesiredLoudness);
}

void UPnPDispatch_RenderingControl_SetBrightness(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredBrightness = NULL;
	int p_DesiredBrightnessLength = 0;
	unsigned short _DesiredBrightness = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==17 && memcmp(VarName,"DesiredBrightness",17) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredBrightness = temp3->LastResult->data;
					p_DesiredBrightnessLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredBrightness,p_DesiredBrightnessLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredBrightness] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredBrightness] out of Range");
			return;
		}
		_DesiredBrightness = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetBrightness((void*)ReaderObject,_InstanceID,_DesiredBrightness);
}

void UPnPDispatch_RenderingControl_GetLoudness(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_RenderingControl_GetLoudness((void*)ReaderObject,_InstanceID,_Channel);
}

void UPnPDispatch_RenderingControl_GetColorTemperature(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetColorTemperature((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetSharpness(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetSharpness((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetContrast(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetContrast((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetGreenVideoGain(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetGreenVideoGain((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_SetRedVideoGain(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredRedVideoGain = NULL;
	int p_DesiredRedVideoGainLength = 0;
	unsigned short _DesiredRedVideoGain = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==19 && memcmp(VarName,"DesiredRedVideoGain",19) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredRedVideoGain = temp3->LastResult->data;
					p_DesiredRedVideoGainLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredRedVideoGain,p_DesiredRedVideoGainLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredRedVideoGain] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredRedVideoGain] out of Range");
			return;
		}
		_DesiredRedVideoGain = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetRedVideoGain((void*)ReaderObject,_InstanceID,_DesiredRedVideoGain);
}

void UPnPDispatch_RenderingControl_SetGreenVideoBlackLevel(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredGreenVideoBlackLevel = NULL;
	int p_DesiredGreenVideoBlackLevelLength = 0;
	unsigned short _DesiredGreenVideoBlackLevel = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==27 && memcmp(VarName,"DesiredGreenVideoBlackLevel",27) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredGreenVideoBlackLevel = temp3->LastResult->data;
					p_DesiredGreenVideoBlackLevelLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredGreenVideoBlackLevel,p_DesiredGreenVideoBlackLevelLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredGreenVideoBlackLevel] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredGreenVideoBlackLevel] out of Range");
			return;
		}
		_DesiredGreenVideoBlackLevel = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetGreenVideoBlackLevel((void*)ReaderObject,_InstanceID,_DesiredGreenVideoBlackLevel);
}

void UPnPDispatch_RenderingControl_GetVolumeDBRange(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_RenderingControl_GetVolumeDBRange((void*)ReaderObject,_InstanceID,_Channel);
}

void UPnPDispatch_RenderingControl_GetRedVideoBlackLevel(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetRedVideoBlackLevel((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetBlueVideoBlackLevel(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetBlueVideoBlackLevel((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetBlueVideoGain(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetBlueVideoGain((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_SetBlueVideoBlackLevel(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredBlueVideoBlackLevel = NULL;
	int p_DesiredBlueVideoBlackLevelLength = 0;
	unsigned short _DesiredBlueVideoBlackLevel = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==26 && memcmp(VarName,"DesiredBlueVideoBlackLevel",26) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredBlueVideoBlackLevel = temp3->LastResult->data;
					p_DesiredBlueVideoBlackLevelLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredBlueVideoBlackLevel,p_DesiredBlueVideoBlackLevelLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredBlueVideoBlackLevel] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredBlueVideoBlackLevel] out of Range");
			return;
		}
		_DesiredBlueVideoBlackLevel = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetBlueVideoBlackLevel((void*)ReaderObject,_InstanceID,_DesiredBlueVideoBlackLevel);
}

void UPnPDispatch_RenderingControl_GetMute(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_RenderingControl_GetMute((void*)ReaderObject,_InstanceID,_Channel);
}

void UPnPDispatch_RenderingControl_SetBlueVideoGain(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredBlueVideoGain = NULL;
	int p_DesiredBlueVideoGainLength = 0;
	unsigned short _DesiredBlueVideoGain = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==20 && memcmp(VarName,"DesiredBlueVideoGain",20) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredBlueVideoGain = temp3->LastResult->data;
					p_DesiredBlueVideoGainLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredBlueVideoGain,p_DesiredBlueVideoGainLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredBlueVideoGain] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredBlueVideoGain] out of Range");
			return;
		}
		_DesiredBlueVideoGain = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetBlueVideoGain((void*)ReaderObject,_InstanceID,_DesiredBlueVideoGain);
}

void UPnPDispatch_RenderingControl_GetVerticalKeystone(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetVerticalKeystone((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_SetVerticalKeystone(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredVerticalKeystone = NULL;
	int p_DesiredVerticalKeystoneLength = 0;
	short _DesiredVerticalKeystone = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==23 && memcmp(VarName,"DesiredVerticalKeystone",23) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredVerticalKeystone = temp3->LastResult->data;
					p_DesiredVerticalKeystoneLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetLong(p_DesiredVerticalKeystone,p_DesiredVerticalKeystoneLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredVerticalKeystone] illegal value");
		return;
	}
	else
	{
		if(!(TempLong>=(long)0xFFFF8000 && TempLong<=(long)0x7FFF))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredVerticalKeystone] out of Range");
			return;
		}
		_DesiredVerticalKeystone = (short)TempLong;
	}
	UPnPFP_RenderingControl_SetVerticalKeystone((void*)ReaderObject,_InstanceID,_DesiredVerticalKeystone);
}

void UPnPDispatch_RenderingControl_GetBrightness(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetBrightness((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetVolumeDB(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	UPnPFP_RenderingControl_GetVolumeDB((void*)ReaderObject,_InstanceID,_Channel);
}

void UPnPDispatch_RenderingControl_GetGreenVideoBlackLevel(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetGreenVideoBlackLevel((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_GetRedVideoGain(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 1)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	UPnPFP_RenderingControl_GetRedVideoGain((void*)ReaderObject,_InstanceID);
}

void UPnPDispatch_RenderingControl_SetMute(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_Channel = NULL;
	int p_ChannelLength = 0;
	char* _Channel = "";
	int _ChannelLength;
	char *p_DesiredMute = NULL;
	int p_DesiredMuteLength = 0;
	int _DesiredMute = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==7 && memcmp(VarName,"Channel",7) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = temp3->LastResult->datalength;
					p_Channel[p_ChannelLength] = 0;
				}
				else
				{
					p_Channel = temp3->LastResult->data;
					p_ChannelLength = 0;
					p_Channel[p_ChannelLength] = 0;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==11 && memcmp(VarName,"DesiredMute",11) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredMute = temp3->LastResult->data;
					p_DesiredMuteLength = temp3->LastResult->datalength;
				}
				OK |= 4;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 7)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	_ChannelLength = ILibInPlaceXmlUnEscape(p_Channel);
	_Channel = p_Channel;
	if(memcmp(_Channel, "Master\0",7) != 0
	&& memcmp(_Channel, "LF\0",3) != 0
	&& memcmp(_Channel, "RF\0",3) != 0
	)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[Channel] contains a value that is not in AllowedValueList");
		return;
	}
	OK=0;
	if(p_DesiredMuteLength==4)
	{
		if(_strnicmp(p_DesiredMute,"true",4)==0)
		{
			OK = 1;
			_DesiredMute = 1;
		}
	}
	if(p_DesiredMuteLength==5)
	{
		if(_strnicmp(p_DesiredMute,"false",5)==0)
		{
			OK = 1;
			_DesiredMute = 0;
		}
	}
	if(p_DesiredMuteLength==1)
	{
		if(memcmp(p_DesiredMute,"0",1)==0)
		{
			OK = 1;
			_DesiredMute = 0;
		}
		if(memcmp(p_DesiredMute,"1",1)==0)
		{
			OK = 1;
			_DesiredMute = 1;
		}
	}
	if(OK==0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredMute] illegal value");
		return;
	}
	UPnPFP_RenderingControl_SetMute((void*)ReaderObject,_InstanceID,_Channel,_DesiredMute);
}

void UPnPDispatch_RenderingControl_SetGreenVideoGain(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredGreenVideoGain = NULL;
	int p_DesiredGreenVideoGainLength = 0;
	unsigned short _DesiredGreenVideoGain = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==21 && memcmp(VarName,"DesiredGreenVideoGain",21) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredGreenVideoGain = temp3->LastResult->data;
					p_DesiredGreenVideoGainLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredGreenVideoGain,p_DesiredGreenVideoGainLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredGreenVideoGain] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredGreenVideoGain] out of Range");
			return;
		}
		_DesiredGreenVideoGain = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetGreenVideoGain((void*)ReaderObject,_InstanceID,_DesiredGreenVideoGain);
}

void UPnPDispatch_RenderingControl_SetSharpness(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredSharpness = NULL;
	int p_DesiredSharpnessLength = 0;
	unsigned short _DesiredSharpness = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==16 && memcmp(VarName,"DesiredSharpness",16) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredSharpness = temp3->LastResult->data;
					p_DesiredSharpnessLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredSharpness,p_DesiredSharpnessLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredSharpness] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0x64))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredSharpness] out of Range");
			return;
		}
		_DesiredSharpness = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetSharpness((void*)ReaderObject,_InstanceID,_DesiredSharpness);
}

void UPnPDispatch_RenderingControl_SetHorizontalKeystone(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	long TempLong;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredHorizontalKeystone = NULL;
	int p_DesiredHorizontalKeystoneLength = 0;
	short _DesiredHorizontalKeystone = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==25 && memcmp(VarName,"DesiredHorizontalKeystone",25) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredHorizontalKeystone = temp3->LastResult->data;
					p_DesiredHorizontalKeystoneLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetLong(p_DesiredHorizontalKeystone,p_DesiredHorizontalKeystoneLength, &TempLong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredHorizontalKeystone] illegal value");
		return;
	}
	else
	{
		if(!(TempLong>=(long)0xFFFF8000 && TempLong<=(long)0x7FFF))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredHorizontalKeystone] out of Range");
			return;
		}
		_DesiredHorizontalKeystone = (short)TempLong;
	}
	UPnPFP_RenderingControl_SetHorizontalKeystone((void*)ReaderObject,_InstanceID,_DesiredHorizontalKeystone);
}

void UPnPDispatch_RenderingControl_SetColorTemperature(struct parser_result *xml, struct HTTPReaderObject *ReaderObject)
{
	struct parser_result *temp;
	struct parser_result *temp2;
	struct parser_result *temp3;
	struct parser_result_field *field;
	char *VarName;
	int VarNameLength;
	int i;
	unsigned long TempULong;
	int OK = 0;
	char *p_InstanceID = NULL;
	int p_InstanceIDLength = 0;
	unsigned int _InstanceID = 0;
	char *p_DesiredColorTemperature = NULL;
	int p_DesiredColorTemperatureLength = 0;
	unsigned short _DesiredColorTemperature = 0;
	field = xml->FirstResult;
	while(field!=NULL)
	{
		if((memcmp(field->data,"?",1)!=0) && (memcmp(field->data,"/",1)!=0))
		{
			temp = ILibParseString(field->data,0,field->datalength," ",1);
			temp2 = ILibParseString(temp->FirstResult->data,0,temp->FirstResult->datalength,":",1);
			if(temp2->NumResults==1)
			{
				VarName = temp2->FirstResult->data;
				VarNameLength = temp2->FirstResult->datalength;
			}
			else
			{
				temp3 = ILibParseString(temp2->FirstResult->data,0,temp2->FirstResult->datalength,">",1);
				if(temp3->NumResults==1)
				{
					VarName = temp2->FirstResult->NextResult->data;
					VarNameLength = temp2->FirstResult->NextResult->datalength;
				}
				else
				{
					VarName = temp2->FirstResult->data;
					VarNameLength = temp2->FirstResult->datalength;
				}
				ILibDestructParserResults(temp3);
			}
			for(i=0;i<VarNameLength;++i)
			{
				if( i!=0 && ((VarName[i]==' ')||(VarName[i]=='/')||(VarName[i]=='>')) )
				{
					VarNameLength = i;
					break;
				}
			}
			if(VarNameLength==10 && memcmp(VarName,"InstanceID",10) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_InstanceID = temp3->LastResult->data;
					p_InstanceIDLength = temp3->LastResult->datalength;
				}
				OK |= 1;
				ILibDestructParserResults(temp3);
			}
			else if(VarNameLength==23 && memcmp(VarName,"DesiredColorTemperature",23) == 0)
			{
				temp3 = ILibParseString(field->data,0,field->datalength,">",1);
				if(memcmp(temp3->FirstResult->data+temp3->FirstResult->datalength-1,"/",1) != 0)
				{
					p_DesiredColorTemperature = temp3->LastResult->data;
					p_DesiredColorTemperatureLength = temp3->LastResult->datalength;
				}
				OK |= 2;
				ILibDestructParserResults(temp3);
			}
			ILibDestructParserResults(temp2);
			ILibDestructParserResults(temp);
		}
		field = field->NextResult;
	}
	
	if (OK != 3)
	{
		UPnPResponse_Error(ReaderObject,402,"Incorrect Arguments");
		return;
	}
	
	/* Type Checking */
	OK = ILibGetULong(p_InstanceID,p_InstanceIDLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[InstanceID] illegal value");
		return;
	}
	_InstanceID = (unsigned int)TempULong;
	OK = ILibGetULong(p_DesiredColorTemperature,p_DesiredColorTemperatureLength, &TempULong);
	if(OK!=0)
	{
		UPnPResponse_Error(ReaderObject,402,"Argument[DesiredColorTemperature] illegal value");
		return;
	}
	else
	{
		if(!(TempULong>=(unsigned long)0x0 && TempULong<=(unsigned long)0xFFFF))
		{
			UPnPResponse_Error(ReaderObject,402,"Argument[DesiredColorTemperature] out of Range");
			return;
		}
		_DesiredColorTemperature = (unsigned short)TempULong;
	}
	UPnPFP_RenderingControl_SetColorTemperature((void*)ReaderObject,_InstanceID,_DesiredColorTemperature);
}

void UPnPProcessPOST(struct packetheader* header, struct HTTPReaderObject *ReaderObject)
{
	struct packetheader_field_node *f = header->FirstField;
	char* HOST;
	char* SOAPACTION = NULL;
	int SOAPACTIONLength = 0;
	struct parser_result *r;
	struct parser_result *xml;
	
	xml = ILibParseString(header->Body,0,header->BodyLength,"<",1);
	while(f!=NULL)
	{
		if(f->FieldLength==4 && _strnicmp(f->Field,"HOST",4)==0)
		{
			HOST = f->FieldData;
		}
		else if(f->FieldLength==10 && _strnicmp(f->Field,"SOAPACTION",10)==0)
		{
			r = ILibParseString(f->FieldData,0,f->FieldDataLength,"#",1);
			SOAPACTION = r->LastResult->data;
			SOAPACTIONLength = r->LastResult->datalength-1;
			ILibDestructParserResults(r);
		}
		f = f->NextField;
	}
	if(header->DirectiveObjLength==25 && memcmp((header->DirectiveObj)+1,"RenderingControl/control",24)==0)
	{
		if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetHorizontalKeystone",21)==0)
		{
			UPnPDispatch_RenderingControl_GetHorizontalKeystone(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==9 && memcmp(SOAPACTION,"GetVolume",9)==0)
		{
			UPnPDispatch_RenderingControl_GetVolume(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"SelectPreset",12)==0)
		{
			UPnPDispatch_RenderingControl_SelectPreset(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==9 && memcmp(SOAPACTION,"SetVolume",9)==0)
		{
			UPnPDispatch_RenderingControl_SetVolume(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"ListPresets",11)==0)
		{
			UPnPDispatch_RenderingControl_ListPresets(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"SetVolumeDB",11)==0)
		{
			UPnPDispatch_RenderingControl_SetVolumeDB(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"SetRedVideoBlackLevel",21)==0)
		{
			UPnPDispatch_RenderingControl_SetRedVideoBlackLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"SetContrast",11)==0)
		{
			UPnPDispatch_RenderingControl_SetContrast(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"SetLoudness",11)==0)
		{
			UPnPDispatch_RenderingControl_SetLoudness(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==13 && memcmp(SOAPACTION,"SetBrightness",13)==0)
		{
			UPnPDispatch_RenderingControl_SetBrightness(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"GetLoudness",11)==0)
		{
			UPnPDispatch_RenderingControl_GetLoudness(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"GetColorTemperature",19)==0)
		{
			UPnPDispatch_RenderingControl_GetColorTemperature(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"GetSharpness",12)==0)
		{
			UPnPDispatch_RenderingControl_GetSharpness(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"GetContrast",11)==0)
		{
			UPnPDispatch_RenderingControl_GetContrast(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==17 && memcmp(SOAPACTION,"GetGreenVideoGain",17)==0)
		{
			UPnPDispatch_RenderingControl_GetGreenVideoGain(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"SetRedVideoGain",15)==0)
		{
			UPnPDispatch_RenderingControl_SetRedVideoGain(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==23 && memcmp(SOAPACTION,"SetGreenVideoBlackLevel",23)==0)
		{
			UPnPDispatch_RenderingControl_SetGreenVideoBlackLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==16 && memcmp(SOAPACTION,"GetVolumeDBRange",16)==0)
		{
			UPnPDispatch_RenderingControl_GetVolumeDBRange(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetRedVideoBlackLevel",21)==0)
		{
			UPnPDispatch_RenderingControl_GetRedVideoBlackLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==22 && memcmp(SOAPACTION,"GetBlueVideoBlackLevel",22)==0)
		{
			UPnPDispatch_RenderingControl_GetBlueVideoBlackLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==16 && memcmp(SOAPACTION,"GetBlueVideoGain",16)==0)
		{
			UPnPDispatch_RenderingControl_GetBlueVideoGain(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==22 && memcmp(SOAPACTION,"SetBlueVideoBlackLevel",22)==0)
		{
			UPnPDispatch_RenderingControl_SetBlueVideoBlackLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==7 && memcmp(SOAPACTION,"GetMute",7)==0)
		{
			UPnPDispatch_RenderingControl_GetMute(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==16 && memcmp(SOAPACTION,"SetBlueVideoGain",16)==0)
		{
			UPnPDispatch_RenderingControl_SetBlueVideoGain(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"GetVerticalKeystone",19)==0)
		{
			UPnPDispatch_RenderingControl_GetVerticalKeystone(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"SetVerticalKeystone",19)==0)
		{
			UPnPDispatch_RenderingControl_SetVerticalKeystone(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==13 && memcmp(SOAPACTION,"GetBrightness",13)==0)
		{
			UPnPDispatch_RenderingControl_GetBrightness(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"GetVolumeDB",11)==0)
		{
			UPnPDispatch_RenderingControl_GetVolumeDB(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==23 && memcmp(SOAPACTION,"GetGreenVideoBlackLevel",23)==0)
		{
			UPnPDispatch_RenderingControl_GetGreenVideoBlackLevel(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetRedVideoGain",15)==0)
		{
			UPnPDispatch_RenderingControl_GetRedVideoGain(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==7 && memcmp(SOAPACTION,"SetMute",7)==0)
		{
			UPnPDispatch_RenderingControl_SetMute(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==17 && memcmp(SOAPACTION,"SetGreenVideoGain",17)==0)
		{
			UPnPDispatch_RenderingControl_SetGreenVideoGain(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"SetSharpness",12)==0)
		{
			UPnPDispatch_RenderingControl_SetSharpness(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"SetHorizontalKeystone",21)==0)
		{
			UPnPDispatch_RenderingControl_SetHorizontalKeystone(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==19 && memcmp(SOAPACTION,"SetColorTemperature",19)==0)
		{
			UPnPDispatch_RenderingControl_SetColorTemperature(xml, ReaderObject);
		}
	}
	else if(header->DirectiveObjLength==20 && memcmp((header->DirectiveObj)+1,"AVTransport/control",19)==0)
	{
		if(SOAPACTIONLength==26 && memcmp(SOAPACTION,"GetCurrentTransportActions",26)==0)
		{
			UPnPDispatch_AVTransport_GetCurrentTransportActions(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Play",4)==0)
		{
			UPnPDispatch_AVTransport_Play(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==8 && memcmp(SOAPACTION,"Previous",8)==0)
		{
			UPnPDispatch_AVTransport_Previous(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Next",4)==0)
		{
			UPnPDispatch_AVTransport_Next(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Stop",4)==0)
		{
			UPnPDispatch_AVTransport_Stop(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==20 && memcmp(SOAPACTION,"GetTransportSettings",20)==0)
		{
			UPnPDispatch_AVTransport_GetTransportSettings(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==4 && memcmp(SOAPACTION,"Seek",4)==0)
		{
			UPnPDispatch_AVTransport_Seek(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==5 && memcmp(SOAPACTION,"Pause",5)==0)
		{
			UPnPDispatch_AVTransport_Pause(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetPositionInfo",15)==0)
		{
			UPnPDispatch_AVTransport_GetPositionInfo(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==16 && memcmp(SOAPACTION,"GetTransportInfo",16)==0)
		{
			UPnPDispatch_AVTransport_GetTransportInfo(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==17 && memcmp(SOAPACTION,"SetAVTransportURI",17)==0)
		{
			UPnPDispatch_AVTransport_SetAVTransportURI(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==21 && memcmp(SOAPACTION,"GetDeviceCapabilities",21)==0)
		{
			UPnPDispatch_AVTransport_GetDeviceCapabilities(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==11 && memcmp(SOAPACTION,"SetPlayMode",11)==0)
		{
			UPnPDispatch_AVTransport_SetPlayMode(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==12 && memcmp(SOAPACTION,"GetMediaInfo",12)==0)
		{
			UPnPDispatch_AVTransport_GetMediaInfo(xml, ReaderObject);
		}
	}
	else if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"ConnectionManager/control",25)==0)
	{
		if(SOAPACTIONLength==24 && memcmp(SOAPACTION,"GetCurrentConnectionInfo",24)==0)
		{
			UPnPDispatch_ConnectionManager_GetCurrentConnectionInfo(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==20 && memcmp(SOAPACTION,"PrepareForConnection",20)==0)
		{
			UPnPDispatch_ConnectionManager_PrepareForConnection(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==18 && memcmp(SOAPACTION,"ConnectionComplete",18)==0)
		{
			UPnPDispatch_ConnectionManager_ConnectionComplete(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==15 && memcmp(SOAPACTION,"GetProtocolInfo",15)==0)
		{
			UPnPDispatch_ConnectionManager_GetProtocolInfo(xml, ReaderObject);
		}
		else if(SOAPACTIONLength==23 && memcmp(SOAPACTION,"GetCurrentConnectionIDs",23)==0)
		{
			UPnPDispatch_ConnectionManager_GetCurrentConnectionIDs(xml, ReaderObject);
		}
	}
	ILibDestructParserResults(xml);
}
struct SubscriberInfo* UPnPRemoveSubscriberInfo(struct SubscriberInfo **Head, int *TotalSubscribers,char* SID, int SIDLength)
{
	struct SubscriberInfo *info = *Head;
	struct SubscriberInfo **ptr = Head;
	while(info!=NULL)
	{
		if(info->SIDLength==SIDLength && memcmp(info->SID,SID,SIDLength)==0)
		{
			*ptr = info->Next;
			if(info->Next!=NULL) 
			{
				(*ptr)->Previous = info->Previous;
				if((*ptr)->Previous!=NULL) 
				{
					(*ptr)->Previous->Next = info->Next;
					if((*ptr)->Previous->Next!=NULL)
					{
						(*ptr)->Previous->Next->Previous = (*ptr)->Previous;
					}
				}
			}
			break;
		}
		ptr = &(info->Next);
		info = info->Next;
	}
	if(info!=NULL)
	{
		info->Previous = NULL;
		info->Next = NULL;
		--(*TotalSubscribers);
	}
	return(info);
}

#define UPnPDestructSubscriberInfo(info)\
{\
	FREE(info->Path);\
	FREE(info->SID);\
	FREE(info);\
}

#define UPnPDestructEventObject(EvObject)\
{\
	FREE(EvObject->PacketBody);\
	FREE(EvObject);\
}

#define UPnPDestructEventDataObject(EvData)\
{\
	FREE(EvData);\
}
void UPnPExpireSubscriberInfo(struct UPnPDataObject *d, struct SubscriberInfo *info)
{
	struct SubscriberInfo *t = info;
	while(t->Previous!=NULL)
	{
		t = t->Previous;
	}
	if(d->HeadSubscriberPtr_ConnectionManager==t)
	{
		--(d->NumberOfSubscribers_ConnectionManager);
	}
	else if(d->HeadSubscriberPtr_AVTransport==t)
	{
		--(d->NumberOfSubscribers_AVTransport);
	}
	else if(d->HeadSubscriberPtr_RenderingControl==t)
	{
		--(d->NumberOfSubscribers_RenderingControl);
	}
	if(info->Previous!=NULL)
	{
		// This is not the Head
		info->Previous->Next = info->Next;
		if(info->Next!=NULL)
		{
			info->Previous->Next->Previous = info->Previous;
		}
	}
	else
	{
		// This is the Head
		if(d->HeadSubscriberPtr_ConnectionManager==info)
		{
			d->HeadSubscriberPtr_ConnectionManager = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_ConnectionManager;
			}
		}
		else if(d->HeadSubscriberPtr_AVTransport==info)
		{
			d->HeadSubscriberPtr_AVTransport = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_AVTransport;
			}
		}
		else if(d->HeadSubscriberPtr_RenderingControl==info)
		{
			d->HeadSubscriberPtr_RenderingControl = info->Next;
			if(info->Next!=NULL)
			{
				info->Next->Previous = d->HeadSubscriberPtr_RenderingControl;
			}
		}
		else
		{
			// Error
			return;
		}
	}
	ILibDeleteRequests(d->EventClient,info);
	--info->RefCount;
	if(info->RefCount==0)
	{
		UPnPDestructSubscriberInfo(info);
	}
}

int UPnPSubscriptionExpired(struct SubscriberInfo *info)
{
	int RetVal = 0;
	if(info->RenewByTime < GetTickCount()/1000) {RetVal = -1;}
	return(RetVal);
}
void UPnPGetInitialEventBody_ConnectionManager(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(177+(int)strlen(UPnPObject->ConnectionManager_SourceProtocolInfo)+(int)strlen(UPnPObject->ConnectionManager_SinkProtocolInfo)+(int)strlen(UPnPObject->ConnectionManager_CurrentConnectionIDs));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"SourceProtocolInfo>%s</SourceProtocolInfo></e:property><e:property><SinkProtocolInfo>%s</SinkProtocolInfo></e:property><e:property><CurrentConnectionIDs>%s</CurrentConnectionIDs",UPnPObject->ConnectionManager_SourceProtocolInfo,UPnPObject->ConnectionManager_SinkProtocolInfo,UPnPObject->ConnectionManager_CurrentConnectionIDs);
}
void UPnPGetInitialEventBody_AVTransport(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(25+(int)strlen(UPnPObject->AVTransport_LastChange));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"LastChange>%s</LastChange",UPnPObject->AVTransport_LastChange);
}
void UPnPGetInitialEventBody_RenderingControl(struct UPnPDataObject *UPnPObject,char ** body, int *bodylength)
{
	int TempLength;
	TempLength = (int)(25+(int)strlen(UPnPObject->RenderingControl_LastChange));
	*body = (char*)MALLOC(sizeof(char)*TempLength);
	*bodylength = sprintf(*body,"LastChange>%s</LastChange",UPnPObject->RenderingControl_LastChange);
}
void UPnPProcessUNSUBSCRIBE(struct packetheader *header, struct HTTPReaderObject *ReaderObject)
{
	char* SID = NULL;
	int SIDLength = 0;
	struct SubscriberInfo *Info;
	struct packetheader_field_node *f;
	char* packet = (char*)MALLOC(sizeof(char)*50);
	int packetlength;
	
	f = header->FirstField;
	while(f!=NULL)
	{
		if(f->FieldLength==3)
		{
			if(_strnicmp(f->Field,"SID",3)==0)
			{
				SID = f->FieldData;
				SIDLength = f->FieldDataLength;
			}
		}
		f = f->NextField;
	}
	sem_wait(&(ReaderObject->Parent->EventLock));
	if(header->DirectiveObjLength==24 && memcmp(header->DirectiveObj + 1,"ConnectionManager/event",23)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_ConnectionManager),&(ReaderObject->Parent->NumberOfSubscribers_ConnectionManager),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
	}
	else if(header->DirectiveObjLength==18 && memcmp(header->DirectiveObj + 1,"AVTransport/event",17)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_AVTransport),&(ReaderObject->Parent->NumberOfSubscribers_AVTransport),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
	}
	else if(header->DirectiveObjLength==23 && memcmp(header->DirectiveObj + 1,"RenderingControl/event",22)==0)
	{
		Info = UPnPRemoveSubscriberInfo(&(ReaderObject->Parent->HeadSubscriberPtr_RenderingControl),&(ReaderObject->Parent->NumberOfSubscribers_RenderingControl),SID,SIDLength);
		if(Info!=NULL)
		{
			--Info->RefCount;
			if(Info->RefCount==0)
			{
				UPnPDestructSubscriberInfo(Info);
			}
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",200,"OK");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
		else
		{
			packetlength = sprintf(packet,"HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",412,"Invalid SID");
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket=0;
		}
	}
	sem_post(&(ReaderObject->Parent->EventLock));
	FREE(packet);
}
void UPnPTryToSubscribe(char* ServiceName, long Timeout, char* URL, int URLLength,struct HTTPReaderObject *ReaderObject)
{
	int *TotalSubscribers = NULL;
	struct SubscriberInfo **HeadPtr = NULL;
	struct SubscriberInfo *NewSubscriber,*TempSubscriber;
	int SIDNumber;
	char *SID;
	char *TempString;
	int TempStringLength;
	char *TempString2;
	long TempLong;
	char *packet;
	int packetlength;
	char* path;
	
	char *packetbody = NULL;
	int packetbodyLength;
	
	struct parser_result *p;
	struct parser_result *p2;
	
	if(strncmp(ServiceName,"ConnectionManager",17)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_ConnectionManager);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_ConnectionManager);
	}
	if(strncmp(ServiceName,"AVTransport",11)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_AVTransport);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_AVTransport);
	}
	if(strncmp(ServiceName,"RenderingControl",16)==0)
	{
		TotalSubscribers = &(ReaderObject->Parent->NumberOfSubscribers_RenderingControl);
		HeadPtr = &(ReaderObject->Parent->HeadSubscriberPtr_RenderingControl);
	}
	if(*HeadPtr!=NULL)
	{
		NewSubscriber = *HeadPtr;
		while(NewSubscriber!=NULL)
		{
			if(UPnPSubscriptionExpired(NewSubscriber)!=0)
			{
				TempSubscriber = NewSubscriber->Next;
				NewSubscriber = UPnPRemoveSubscriberInfo(HeadPtr,TotalSubscribers,NewSubscriber->SID,NewSubscriber->SIDLength);
				UPnPDestructSubscriberInfo(NewSubscriber);
				NewSubscriber = TempSubscriber;
			}
			else
			{
				NewSubscriber = NewSubscriber->Next;
			}
		}
	}
	if(*TotalSubscribers<10)
	{
		NewSubscriber = (struct SubscriberInfo*)MALLOC(sizeof(struct SubscriberInfo));
		SIDNumber = ++ReaderObject->Parent->SID;
		SID = (char*)MALLOC(10 + 6);
		sprintf(SID,"uuid:%d",SIDNumber);
		p = ILibParseString(URL,0,URLLength,"://",3);
		if(p->NumResults==1)
		{
			send(ReaderObject->ClientSocket,"HTTP/1.1 412 Precondition Failed\r\n\r\n",36,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			ILibDestructParserResults(p);
			return;
		}
		TempString = p->LastResult->data;
		TempStringLength = p->LastResult->datalength;
		ILibDestructParserResults(p);
		p = ILibParseString(TempString,0,TempStringLength,"/",1);
		p2 = ILibParseString(p->FirstResult->data,0,p->FirstResult->datalength,":",1);
		TempString2 = (char*)MALLOC(1+sizeof(char)*p2->FirstResult->datalength);
		memcpy(TempString2,p2->FirstResult->data,p2->FirstResult->datalength);
		TempString2[p2->FirstResult->datalength] = '\0';
		NewSubscriber->Address = inet_addr(TempString2);
		if(p2->NumResults==1)
		{
			NewSubscriber->Port = 80;
			path = (char*)MALLOC(1+TempStringLength - p2->FirstResult->datalength -1);
			memcpy(path,TempString + p2->FirstResult->datalength,TempStringLength - p2->FirstResult->datalength -1);
			path[TempStringLength - p2->FirstResult->datalength - 1] = '\0';
			NewSubscriber->Path = path;
			NewSubscriber->PathLength = (int)strlen(path);
		}
		else
		{
			ILibGetLong(p2->LastResult->data,p2->LastResult->datalength,&TempLong);
			NewSubscriber->Port = (unsigned short)TempLong;
			if(TempStringLength==p->FirstResult->datalength)
			{
				path = (char*)MALLOC(2);
				memcpy(path,"/",1);
				path[1] = '\0';
			}
			else
			{
				path = (char*)MALLOC(1+TempStringLength - p->FirstResult->datalength -1);
				memcpy(path,TempString + p->FirstResult->datalength,TempStringLength - p->FirstResult->datalength -1);
				path[TempStringLength - p->FirstResult->datalength -1] = '\0';
			}
			NewSubscriber->Path = path;
			NewSubscriber->PathLength = (int)strlen(path);
		}
		ILibDestructParserResults(p);
		ILibDestructParserResults(p2);
		FREE(TempString2);
		NewSubscriber->RefCount = 1;
		NewSubscriber->Disposing = 0;
		NewSubscriber->Previous = NULL;
		NewSubscriber->SID = SID;
		NewSubscriber->SIDLength = (int)strlen(SID);
		NewSubscriber->SEQ = 0;
		NewSubscriber->RenewByTime = (GetTickCount() / 1000) + Timeout;
		NewSubscriber->Next = *HeadPtr;
		if(*HeadPtr!=NULL) {(*HeadPtr)->Previous = NewSubscriber;}
		*HeadPtr = NewSubscriber;
		++(*TotalSubscribers);
		LVL3DEBUG(printf("\r\n\r\nSubscribed [%s] %d.%d.%d.%d:%d FOR %d Duration\r\n",NewSubscriber->SID,(NewSubscriber->Address)&0xFF,(NewSubscriber->Address>>8)&0xFF,(NewSubscriber->Address>>16)&0xFF,(NewSubscriber->Address>>24)&0xFF,NewSubscriber->Port,Timeout);)
		LVL3DEBUG(printf("TIMESTAMP: %d <%d>\r\n\r\n",(NewSubscriber->RenewByTime)-Timeout,NewSubscriber);)
		packet = (char*)MALLOC(134 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",SID,Timeout);
		if(strcmp(ServiceName,"ConnectionManager")==0)
		{
			UPnPGetInitialEventBody_ConnectionManager(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		else if(strcmp(ServiceName,"AVTransport")==0)
		{
			UPnPGetInitialEventBody_AVTransport(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		else if(strcmp(ServiceName,"RenderingControl")==0)
		{
			UPnPGetInitialEventBody_RenderingControl(ReaderObject->Parent,&packetbody,&packetbodyLength);
		}
		if (packetbody != NULL)	    {
			send(ReaderObject->ClientSocket,packet,packetlength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			FREE(packet);
			
			UPnPSendEvent_Body(ReaderObject->Parent,packetbody,packetbodyLength,NewSubscriber);
			FREE(packetbody);
		} 
	}
	else
	{
		/* Too many subscribers */
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Too Many Subscribers\r\n\r\n",37,0);
		closesocket(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}
void UPnPSubscribeEvents(char* path,int pathlength,char* Timeout,int TimeoutLength,char* URL,int URLLength,struct HTTPReaderObject* ReaderObject)
{
	long TimeoutVal;
	char* buffer = (char*)MALLOC(1+sizeof(char)*pathlength);
	
	ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
	memcpy(buffer,path,pathlength);
	buffer[pathlength] = '\0';
	FREE(buffer);
	if(TimeoutVal>7200) {TimeoutVal=7200;}
	
	if(pathlength==23 && memcmp(path+1,"RenderingControl/event",22)==0)
	{
		UPnPTryToSubscribe("RenderingControl",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else if(pathlength==18 && memcmp(path+1,"AVTransport/event",17)==0)
	{
		UPnPTryToSubscribe("AVTransport",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else if(pathlength==24 && memcmp(path+1,"ConnectionManager/event",23)==0)
	{
		UPnPTryToSubscribe("ConnectionManager",TimeoutVal,URL,URLLength,ReaderObject);
	}
	else
	{
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Invalid Service Name\r\n\r\n",37,0);
		closesocket(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
	}
}
void UPnPRenewEvents(char* path,int pathlength,char *_SID,int SIDLength, char* Timeout, int TimeoutLength, struct HTTPReaderObject *ReaderObject)
{
	struct SubscriberInfo *info = NULL;
	long TimeoutVal;
	char* packet;
	int packetlength;
	char* SID = (char*)MALLOC(SIDLength+1);
	memcpy(SID,_SID,SIDLength);
	SID[SIDLength] ='\0';
	LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",GetTickCount()/1000);)
	LVL3DEBUG(printf("SUBSCRIBER [%s] attempting to Renew Events for %s Duration [",SID,Timeout);)
	if(pathlength==24 && memcmp(path+1,"ConnectionManager/event",23)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_ConnectionManager;
	}
	else if(pathlength==18 && memcmp(path+1,"AVTransport/event",17)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_AVTransport;
	}
	else if(pathlength==23 && memcmp(path+1,"RenderingControl/event",22)==0)
	{
		info = ReaderObject->Parent->HeadSubscriberPtr_RenderingControl;
	}
	while(info!=NULL && strcmp(info->SID,SID)!=0)
	{
		info = info->Next;
	}
	if(info!=NULL)
	{
		ILibGetLong(Timeout,TimeoutLength,&TimeoutVal);
		info->RenewByTime = TimeoutVal + (GetTickCount() / 1000);
		packet = (char*)MALLOC(134 + (int)strlen(SID) + 4);
		packetlength = sprintf(packet,"HTTP/1.1 200 OK\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264\r\nSID: %s\r\nTIMEOUT: Second-%ld\r\nContent-Length: 0\r\n\r\n",SID,TimeoutVal);
		send(ReaderObject->ClientSocket,packet,packetlength,0);
		FREE(packet);
		LVL3DEBUG(printf("OK] {%d} <%d>\r\n\r\n",TimeoutVal,info);)
	}
	else
	{
		LVL3DEBUG(printf("FAILED]\r\n\r\n");)
		send(ReaderObject->ClientSocket,"HTTP/1.1 412 Precondition Failed\r\n\r\n",36,0);
	}
	closesocket(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(SID);
}
void UPnPProcessSUBSCRIBE(struct packetheader *header, struct HTTPReaderObject *ReaderObject)
{
	char* SID = NULL;
	int SIDLength = 0;
	char* Timeout = NULL;
	int TimeoutLength = 0;
	char* URL = NULL;
	int URLLength = 0;
	struct parser_result *p;
	struct packetheader_field_node *f;
	
	f = header->FirstField;
	while(f!=NULL)
	{
		if(f->FieldLength==3 && _strnicmp(f->Field,"SID",3)==0)
		{
			SID = f->FieldData;
			SIDLength = f->FieldDataLength;
		}
		else if(f->FieldLength==8 && _strnicmp(f->Field,"Callback",8)==0)
		{
			URL = f->FieldData;
			URLLength = f->FieldDataLength;
		}
		else if(f->FieldLength==7 && _strnicmp(f->Field,"Timeout",7)==0)
		{
			Timeout = f->FieldData;
			TimeoutLength = f->FieldDataLength;
		}
		f = f->NextField;
	}
	if(Timeout==NULL)
	{
		Timeout = "7200";
		TimeoutLength = 4;
	}
	else
	{
		p = ILibParseString(Timeout,0,TimeoutLength,"-",1);
		if(p->NumResults==2)
		{
			Timeout = p->LastResult->data;
			TimeoutLength = p->LastResult->datalength;
			if(TimeoutLength==8 && _strnicmp(Timeout,"INFINITE",8)==0)
			{
				Timeout = "7200";
				TimeoutLength = 4;
			}
		}
		else
		{
			Timeout = "7200";
			TimeoutLength = 4;
		}
		ILibDestructParserResults(p);
	}
	if(SID==NULL)
	{
		/* Subscribe */
		UPnPSubscribeEvents(header->DirectiveObj,header->DirectiveObjLength,Timeout,TimeoutLength,URL,URLLength,ReaderObject);
	}
	else
	{
		/* Renew */
		UPnPRenewEvents(header->DirectiveObj,header->DirectiveObjLength,SID,SIDLength,Timeout,TimeoutLength,ReaderObject);
	}
}
void UPnPProcessHTTPPacket(struct packetheader* header, struct HTTPReaderObject *ReaderObject)
{
	char *errorTemplate = "HTTP/1.1 %d %s\r\nServer: %s\r\nContent-Length: 0\r\n\r\n";
	char errorPacket[100];
	int errorPacketLength;
	char *buffer;
	/* Virtual Directory Support */
	if(header->DirectiveObjLength>=4 && memcmp(header->DirectiveObj,"/web",4)==0)
	{
		UPnPFP_PresentationPage((void*)ReaderObject,header);
	}
	else if(header->DirectiveLength==3 && memcmp(header->Directive,"GET",3)==0)
	{
		if(header->DirectiveObjLength==1 && memcmp(header->DirectiveObj,"/",1)==0)
		{
			send(ReaderObject->ClientSocket,ReaderObject->Parent->DeviceDescription,ReaderObject->Parent->DeviceDescriptionLength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			return;
		}
		else if(header->DirectiveObjLength==26 && memcmp((header->DirectiveObj)+1,"RenderingControl/scpd.xml",25)==0)
		{
			buffer = ILibDecompressString((char*)UPnPRenderingControlDescription,UPnPRenderingControlDescriptionLength,UPnPRenderingControlDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPRenderingControlDescriptionLengthUX, 0);
			FREE(buffer);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
		else if(header->DirectiveObjLength==21 && memcmp((header->DirectiveObj)+1,"AVTransport/scpd.xml",20)==0)
		{
			buffer = ILibDecompressString((char*)UPnPAVTransportDescription,UPnPAVTransportDescriptionLength,UPnPAVTransportDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPAVTransportDescriptionLengthUX, 0);
			FREE(buffer);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
		else if(header->DirectiveObjLength==27 && memcmp((header->DirectiveObj)+1,"ConnectionManager/scpd.xml",26)==0)
		{
			buffer = ILibDecompressString((char*)UPnPConnectionManagerDescription,UPnPConnectionManagerDescriptionLength,UPnPConnectionManagerDescriptionLengthUX);
			send(ReaderObject->ClientSocket, buffer, UPnPConnectionManagerDescriptionLengthUX, 0);
			FREE(buffer);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
		}
		else
		{
			errorPacketLength = sprintf(errorPacket,errorTemplate,404,"File Not Found","WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264");
			send(ReaderObject->ClientSocket,errorPacket,errorPacketLength,0);
			closesocket(ReaderObject->ClientSocket);
			ReaderObject->ClientSocket = 0;
			return;
		}
	}
	else if(header->DirectiveLength==4 && memcmp(header->Directive,"POST",4)==0)
	{
		UPnPProcessPOST(header,ReaderObject);
	}
	else if(header->DirectiveLength==9 && memcmp(header->Directive,"SUBSCRIBE",9)==0)
	{
		UPnPProcessSUBSCRIBE(header,ReaderObject);
	}
	else if(header->DirectiveLength==11 && memcmp(header->Directive,"UNSUBSCRIBE",11)==0)
	{
		UPnPProcessUNSUBSCRIBE(header,ReaderObject);
	}
	else
	{
		errorPacketLength = sprintf(errorPacket,errorTemplate,400,"Bad Request","WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264");
		send(ReaderObject->ClientSocket,errorPacket,errorPacketLength,0);
		closesocket(ReaderObject->ClientSocket);
		ReaderObject->ClientSocket = 0;
		return;
	}
}
void UPnPProcessHTTPSocket(struct HTTPReaderObject *ReaderObject)
{
	int bytesReceived = 0;
	int ContentLength = 0;
	struct packetheader_field_node *field;
	int headsize = 0;
	int x;
	
	if(ReaderObject->Body == NULL)
	{
		/* Still Reading Headers */
		bytesReceived = recv(ReaderObject->ClientSocket,ReaderObject->Header+ReaderObject->HeaderIndex,2048-ReaderObject->HeaderIndex,0);
		if(bytesReceived!=0 && bytesReceived!=0xFFFFFFFF)
		{
			/* Received Data
			*/
			ReaderObject->HeaderIndex += bytesReceived;
			if(ReaderObject->HeaderIndex >= 4)
			{
				/* Must have read at least 4 bytes to have a header */
				
				headsize = 0;
				for(x=0;x<(ReaderObject->HeaderIndex - 3);x++)
				{
					//printf("CMP: %x\r\n",*((int*)(ReaderObject->Header + x)));
					//if (*((int*)((ReaderObject->Header) + x)) == 0x0A0D0A0D)
					if (ReaderObject->Header[x] == '\r' && ReaderObject->Header[x+1] == '\n' && ReaderObject->Header[x+2] == '\r' && ReaderObject->Header[x+3] == '\n')
					{
						headsize = x + 4;
						break;
					}
				}
				
				if(headsize != 0)
				{
					/* Complete reading header */
					ReaderObject->ParsedHeader = ILibParsePacketHeader(ReaderObject->Header,0,headsize);
					field = ReaderObject->ParsedHeader->FirstField;
					while(field!=NULL)
					{
						if(field->FieldLength>=14)
						{
							if(_strnicmp(field->Field,"content-length",14)==0)
							{
								ContentLength = atoi(field->FieldData);
								break;
							}
						}
						field = field->NextField;
					}
					if(ContentLength==0)
					{
						/* No Body */
						ReaderObject->FinRead = 1;
						UPnPProcessHTTPPacket(ReaderObject->ParsedHeader, ReaderObject);
					}
					else
					{
						/* There is a Body */
						
						/* Check to see if over reading has occured */
						if (headsize < ReaderObject->HeaderIndex)
						{
							if(ReaderObject->HeaderIndex - headsize >= ContentLength)
							{
								ReaderObject->FinRead=1;
								ReaderObject->ParsedHeader->Body = ReaderObject->Header + headsize;
								ReaderObject->ParsedHeader->BodyLength = ContentLength;
								UPnPProcessHTTPPacket(ReaderObject->ParsedHeader, ReaderObject);
							}
							else
							{
								ReaderObject->Body = (char*)MALLOC(sizeof(char)*ContentLength);
								ReaderObject->BodySize = ContentLength;
								
								memcpy(ReaderObject->Body,ReaderObject->Header + headsize,UPnPMIN(ReaderObject->HeaderIndex - headsize,ContentLength));
								ReaderObject->BodyIndex = ReaderObject->HeaderIndex - headsize;
							}
						}
						else
						{
							ReaderObject->Body = (char*)MALLOC(sizeof(char)*ContentLength);
							ReaderObject->BodySize = ContentLength;
						}
					}
					//ILibDestructPacket(header);
				}
			}
		}
		else
		if(bytesReceived==0)
		{
			/* Socket Closed */
			ReaderObject->ClientSocket = 0;
		}
	}
	else
	{
		/* Reading Body */
		bytesReceived = recv(ReaderObject->ClientSocket,
		ReaderObject->Body+ReaderObject->BodyIndex,
		ReaderObject->BodySize-ReaderObject->BodyIndex,
		0);
		if(bytesReceived!=0)
		{
			/* Received Data */
			ReaderObject->BodyIndex += bytesReceived;
			if(ReaderObject->BodyIndex==ReaderObject->BodySize)
			{
				ReaderObject->FinRead=1;
				//header = ILibParsePacketHeader(ReaderObject->Header,0,ReaderObject->HeaderIndex);
				ReaderObject->ParsedHeader->Body = ReaderObject->Body;
				ReaderObject->ParsedHeader->BodyLength = ReaderObject->BodySize;
				UPnPProcessHTTPPacket(ReaderObject->ParsedHeader, ReaderObject);
				//ILibDestructPacket(header);
			}
		}
		else
		{
			/* Socket Closed/Error */
			ReaderObject->ClientSocket = 0;
		}
	}
}
void UPnPMasterPreSelect(void* object,fd_set *socketset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	int i;
	int NumFree = 5;
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)object;
	int notifytime;
	
	int ra = 1;
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	unsigned char TTL = 4;
	
	if(UPnPObject->InitialNotify==0)
	{
		UPnPObject->InitialNotify = -1;
		UPnPSendByeBye(UPnPObject);
		UPnPSendNotify(UPnPObject);
	}
	if(UPnPObject->UpdateFlag!=0)
	{
		UPnPObject->UpdateFlag = 0;
		
		/* Clear Sockets */
		for(i=0;i<UPnPObject->AddressListLength;++i)
		{
			closesocket(UPnPObject->NOTIFY_SEND_socks[i]);
		}
		FREE(UPnPObject->NOTIFY_SEND_socks);
		
		/* Set up socket */
		FREE(UPnPObject->AddressList);
		UPnPObject->AddressListLength = ILibGetLocalIPAddressList(&(UPnPObject->AddressList));
		UPnPObject->NOTIFY_SEND_socks = (SOCKET*)MALLOC(sizeof(int)*(UPnPObject->AddressListLength));
		
		for(i=0;i<UPnPObject->AddressListLength;++i)
		{
			UPnPObject->NOTIFY_SEND_socks[i] = socket(AF_INET, SOCK_DGRAM, 0);
			memset((char *)&(addr), 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = UPnPObject->AddressList[i];
			addr.sin_port = (unsigned short)htons(UPNP_PORT);
			if (setsockopt(UPnPObject->NOTIFY_SEND_socks[i], SOL_SOCKET, SO_REUSEADDR,(char*)&ra, sizeof(ra)) == 0)
			{
				if (setsockopt(UPnPObject->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_TTL,(char*)&TTL, sizeof(TTL)) < 0)
				{
					// Ignore the case if setting the Multicast-TTL fails
				}
				if (bind(UPnPObject->NOTIFY_SEND_socks[i], (struct sockaddr *) &(addr), sizeof(addr)) == 0)
				{
					mreq.imr_multiaddr.s_addr = inet_addr(UPNP_GROUP);
					mreq.imr_interface.s_addr = UPnPObject->AddressList[i];
					if (setsockopt(UPnPObject->NOTIFY_RECEIVE_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char*)&mreq, sizeof(mreq)) < 0)
					{
						// Does not matter if it fails, just ignore
					}
				}
			}
		}
		UPnPSendNotify(UPnPObject);
	}
	FD_SET(UPnPObject->NOTIFY_RECEIVE_sock,socketset);
	for(i=0;i<5;++i)
	{
		if(UPnPObject->ReaderObjects[i].ClientSocket!=0)
		{
			if(UPnPObject->ReaderObjects[i].FinRead==0)
			{
				FD_SET(UPnPObject->ReaderObjects[i].ClientSocket,socketset);
				FD_SET(UPnPObject->ReaderObjects[i].ClientSocket,errorset);
			}
			--NumFree;
		}
	}
	
	notifytime = UPnPPeriodicNotify(UPnPObject);
	if(NumFree!=0)
	{
		FD_SET(UPnPObject->WebSocket,socketset);
		if(notifytime<*blocktime) {*blocktime=notifytime;}
	}
	else
	{
		if(*blocktime>1)
		{
			*blocktime = 1;
		}
	}
}

void UPnPWebServerTimerSink(void *data)
{
	struct HTTPReaderObject* RO = (struct HTTPReaderObject*)data;
	
	if(RO->ClientSocket!=0)
	{
		closesocket(RO->ClientSocket);
		RO->ClientSocket = 0;
	}
}
void UPnPMasterPostSelect(void* object,int slct, fd_set *socketset, fd_set *writeset, fd_set *errorset)
{
	unsigned long flags=0;
	int cnt = 0;
	struct packetheader *packet;
	int i;
	SOCKET NewSocket;
	struct sockaddr addr;
	int addrlen;
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)object;
	
	if(slct>0)
	{
		if(FD_ISSET(UPnPObject->WebSocket,socketset)!=0)
		{
			for(i=0;i<5;++i)
			{
				if(UPnPObject->ReaderObjects[i].ClientSocket==0)
				{
					addrlen = sizeof(addr);
					NewSocket = accept(UPnPObject->WebSocket,&addr,&addrlen);
					ioctlsocket(NewSocket,FIONBIO,&flags);
					if (NewSocket != 0xFFFFFFFF)
					{
						ILibLifeTime_Add(UPnPObject->WebServerTimer,&(UPnPObject->ReaderObjects[i]),3,&UPnPWebServerTimerSink,NULL);
						if(UPnPObject->ReaderObjects[i].Body != NULL)
						{
							FREE(UPnPObject->ReaderObjects[i].Body);
							UPnPObject->ReaderObjects[i].Body = NULL;
						}
						if(UPnPObject->ReaderObjects[i].ParsedHeader!=NULL)
						{
							ILibDestructPacket(UPnPObject->ReaderObjects[i].ParsedHeader);
						}
						UPnPObject->ReaderObjects[i].ClientSocket = NewSocket;
						UPnPObject->ReaderObjects[i].HeaderIndex = 0;
						UPnPObject->ReaderObjects[i].BodyIndex = 0;
						UPnPObject->ReaderObjects[i].Body = NULL;
						UPnPObject->ReaderObjects[i].BodySize = 0;
						UPnPObject->ReaderObjects[i].FinRead = 0;
						UPnPObject->ReaderObjects[i].Parent = UPnPObject;
						UPnPObject->ReaderObjects[i].ParsedHeader = NULL;
					}
					else {break;}
				}
			}
		}
		for(i=0;i<5;++i)
		{
			if(UPnPObject->ReaderObjects[i].ClientSocket!=0)
			{
				if(FD_ISSET(UPnPObject->ReaderObjects[i].ClientSocket,socketset)!=0)
				{
					UPnPProcessHTTPSocket(&(UPnPObject->ReaderObjects[i]));
				}
				if(FD_ISSET(UPnPObject->ReaderObjects[i].ClientSocket,errorset)!=0)
				{
					/* Socket is probably closed */
					UPnPObject->ReaderObjects[i].ClientSocket = 0;
					if(UPnPObject->ReaderObjects[i].Body != NULL)
					{
						FREE(UPnPObject->ReaderObjects[i].Body);
						UPnPObject->ReaderObjects[i].Body = NULL;
					}
				}
				if(UPnPObject->ReaderObjects[i].ClientSocket==0 || UPnPObject->ReaderObjects[i].FinRead!=0 || UPnPObject->ReaderObjects[i].Body!=NULL || (UPnPObject->ReaderObjects[i].ParsedHeader!=NULL && UPnPObject->ReaderObjects[i].ParsedHeader->Body != NULL))
				{
					ILibLifeTime_Remove(UPnPObject->WebServerTimer,&(UPnPObject->ReaderObjects[i]));
				}
			}
		}
		if(FD_ISSET(UPnPObject->NOTIFY_RECEIVE_sock,socketset)!=0)
		{	
			cnt = recvfrom(UPnPObject->NOTIFY_RECEIVE_sock, UPnPObject->message, sizeof(UPnPObject->message), 0,
			(struct sockaddr *) &(UPnPObject->addr), &(UPnPObject->addrlen));
			if (cnt < 0)
			{
				printf("recvfrom");
				exit(1);
			}
			else if (cnt == 0)
			{
				/* Socket Closed? */
			}
			packet = ILibParsePacketHeader(UPnPObject->message,0,cnt);
			packet->Source = (struct sockaddr_in*)&(UPnPObject->addr);
			packet->ReceivingAddress = 0;
			if(packet->StatusCode==-1 && memcmp(packet->Directive,"M-SEARCH",8)==0)
			{
				UPnPProcessMSEARCH(UPnPObject, packet);
			}
			ILibDestructPacket(packet);
		}
		
	}
}
int UPnPPeriodicNotify(struct UPnPDataObject *upnp)
{
	upnp->CurrentTime = GetTickCount() / 1000;
	if(upnp->CurrentTime >= upnp->NotifyTime)
	{
		upnp->NotifyTime = upnp->CurrentTime + (upnp->NotifyCycleTime / 3);
		UPnPSendNotify(upnp);
	}
	return(upnp->NotifyTime-upnp->CurrentTime);
}
void UPnPSendNotify(const struct UPnPDataObject *upnp)
{
	int packetlength;
	char* packet = (char*)MALLOC(5000);
	int i,i2;
	struct sockaddr_in addr;
	int addrlen;
	struct in_addr interface_addr;
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);
	addr.sin_port = (unsigned short)htons(UPNP_PORT);
	addrlen = sizeof(addr);
	
	memset((char *)&interface_addr, 0, sizeof(interface_addr));
	
	for(i=0;i<upnp->AddressListLength;++i)
	{
		interface_addr.s_addr = upnp->AddressList[i];
		if (setsockopt(upnp->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)
		{
			for (i2=0;i2<2;i2++)
			{
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"","uuid:",upnp->UDN,upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
				UPnPBuildSsdpNotifyPacket(packet,&packetlength,upnp->AddressList[i],(unsigned short)upnp->WebSocketPortNumber,0,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","",upnp->NotifyCycleTime);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) {exit(1);}
			}
		}
	}
	FREE(packet);
}

#define UPnPBuildSsdpByeByePacket(outpacket,outlenght,USN,USNex,NT,NTex)\
{\
	*outlenght = sprintf(outpacket,"NOTIFY * HTTP/1.0\r\nHOST: 239.255.255.250:1900\r\nNTS: ssdp:byebye\r\nUSN: uuid:%s%s\r\nNT: %s%s\r\nContent-Length: 0\r\n\r\n",USN,USNex,NT,NTex);\
}

void UPnPSendByeBye(const struct UPnPDataObject *upnp)
{
	int packetlength;
	char* packet = (char*)MALLOC(5000);
	int i, i2;
	struct sockaddr_in addr;
	int addrlen;
	struct in_addr interface_addr;
	
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(UPNP_GROUP);
	addr.sin_port = (unsigned short)htons(UPNP_PORT);
	addrlen = sizeof(addr);
	
	memset((char *)&interface_addr, 0, sizeof(interface_addr));
	
	for(i=0;i<upnp->AddressListLength;++i)
	{
		
		interface_addr.s_addr = upnp->AddressList[i];
		if (setsockopt(upnp->NOTIFY_SEND_socks[i], IPPROTO_IP, IP_MULTICAST_IF,(char*)&interface_addr, sizeof(interface_addr)) == 0)
		{
			
			for (i2=0;i2<2;i2++)
			{
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::upnp:rootdevice","upnp:rootdevice","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"","uuid:",upnp->UDN);
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:device:MediaRenderer:1","urn:schemas-upnp-org:device:MediaRenderer:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:RenderingControl:1","urn:schemas-upnp-org:service:RenderingControl:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:AVTransport:1","urn:schemas-upnp-org:service:AVTransport:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
				UPnPBuildSsdpByeByePacket(packet,&packetlength,upnp->UDN,"::urn:schemas-upnp-org:service:ConnectionManager:1","urn:schemas-upnp-org:service:ConnectionManager:1","");
				if (sendto(upnp->NOTIFY_SEND_socks[i], packet, packetlength, 0, (struct sockaddr *) &addr, addrlen) < 0) exit(1);
			}
		}
	}
	FREE(packet);
}

void UPnPResponse_Error(const void* UPnPToken, const int ErrorCode, const char* ErrorMsg)
{
	char* body;
	int bodylength;
	char* head;
	int headlength;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	body = (char*)MALLOC(395 + (int)strlen(ErrorMsg));
	bodylength = sprintf(body,"<s:Envelope\r\n xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><s:Fault><faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring><detail><UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\"><errorCode>%d</errorCode><errorDescription>%s</errorDescription></UPnPError></detail></s:Fault></s:Body></s:Envelope>",ErrorCode,ErrorMsg);
	head = (char*)MALLOC(59);
	headlength = sprintf(head,"HTTP/1.1 500 Internal\r\nContent-Length: %d\r\n\r\n",bodylength);
	send(ReaderObject->ClientSocket,head,headlength,0);
	send(ReaderObject->ClientSocket,body,bodylength,0);
	closesocket(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(head);
	FREE(body);
}

int UPnPPresentationResponse(const void* UPnPToken, const char* Data, const int DataLength, const int Terminate)
{
	int status = -1;
	SOCKET TempSocket;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	if(DataLength>0)
	{
		status = send(ReaderObject->ClientSocket,Data,DataLength,0);
	}
	if (Terminate != 0)
	{
		TempSocket = ReaderObject->ClientSocket;
		ReaderObject->ClientSocket = 0;
		closesocket(TempSocket);
	}
	return status;
}

int UPnPGetLocalInterfaceToHost(const void* UPnPToken)
{
	struct sockaddr_in addr;
	int addrsize = sizeof(addr);
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	if (getsockname(ReaderObject->ClientSocket, (struct sockaddr*) &addr, &addrsize) != 0) return 0;
	return (addr.sin_addr.s_addr);
}

void UPnPResponseGeneric(const void* UPnPToken,const char* ServiceURI,const char* MethodName,const char* Params)
{
	char* packet;
	int packetlength;
	struct HTTPReaderObject *ReaderObject = (struct HTTPReaderObject*)UPnPToken;
	
	packet = (char*)MALLOC(67+strlen(ServiceURI)+strlen(Params)+(strlen(MethodName)*2));
	packetlength = sprintf(packet,"<u:%sResponse xmlns:u=\"%s\">%s</u:%sResponse></s:Body></s:Envelope>",MethodName,ServiceURI,Params,MethodName);
	send(ReaderObject->ClientSocket,"HTTP/1.0 200 OK\r\nEXT:\r\nCONTENT-TYPE: text/xml\r\nSERVER: WINDOWS, UPnP/1.0, Intel MicroStack/1.0.1264\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body>",275,0);
	send(ReaderObject->ClientSocket,packet,packetlength,0);
	closesocket(ReaderObject->ClientSocket);
	ReaderObject->ClientSocket = 0;
	FREE(packet);}

void UPnPResponse_RenderingControl_GetHorizontalKeystone(const void* UPnPToken, const short CurrentHorizontalKeystone)
{
	char* body;
	
	body = (char*)MALLOC(62);
	sprintf(body,"<CurrentHorizontalKeystone>%d</CurrentHorizontalKeystone>",CurrentHorizontalKeystone);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetHorizontalKeystone",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetVolume(const void* UPnPToken, const unsigned short CurrentVolume)
{
	char* body;
	
	body = (char*)MALLOC(38);
	sprintf(body,"<CurrentVolume>%u</CurrentVolume>",CurrentVolume);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetVolume",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_SelectPreset(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SelectPreset","");
}

void UPnPResponse_RenderingControl_SetVolume(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetVolume","");
}

void UPnPResponse_RenderingControl_ListPresets(const void* UPnPToken, const char* unescaped_CurrentPresetNameList)
{
	char* body;
	char *CurrentPresetNameList = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentPresetNameList));
	
	ILibXmlEscape(CurrentPresetNameList,unescaped_CurrentPresetNameList);
	body = (char*)MALLOC(48+strlen(CurrentPresetNameList));
	sprintf(body,"<CurrentPresetNameList>%s</CurrentPresetNameList>",CurrentPresetNameList);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","ListPresets",body);
	FREE(body);
	FREE(CurrentPresetNameList);
}

void UPnPResponse_RenderingControl_SetVolumeDB(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetVolumeDB","");
}

void UPnPResponse_RenderingControl_SetRedVideoBlackLevel(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetRedVideoBlackLevel","");
}

void UPnPResponse_RenderingControl_SetContrast(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetContrast","");
}

void UPnPResponse_RenderingControl_SetLoudness(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetLoudness","");
}

void UPnPResponse_RenderingControl_SetBrightness(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetBrightness","");
}

void UPnPResponse_RenderingControl_GetLoudness(const void* UPnPToken, const int CurrentLoudness)
{
	char* body;
	
	body = (char*)MALLOC(37);
	sprintf(body,"<CurrentLoudness>%d</CurrentLoudness>",(CurrentLoudness!=0?1:0));
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetLoudness",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetColorTemperature(const void* UPnPToken, const unsigned short CurrentColorTemperature)
{
	char* body;
	
	body = (char*)MALLOC(58);
	sprintf(body,"<CurrentColorTemperature>%u</CurrentColorTemperature>",CurrentColorTemperature);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetColorTemperature",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetSharpness(const void* UPnPToken, const unsigned short CurrentSharpness)
{
	char* body;
	
	body = (char*)MALLOC(44);
	sprintf(body,"<CurrentSharpness>%u</CurrentSharpness>",CurrentSharpness);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetSharpness",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetContrast(const void* UPnPToken, const unsigned short CurrentContrast)
{
	char* body;
	
	body = (char*)MALLOC(42);
	sprintf(body,"<CurrentContrast>%u</CurrentContrast>",CurrentContrast);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetContrast",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetGreenVideoGain(const void* UPnPToken, const unsigned short CurrentGreenVideoGain)
{
	char* body;
	
	body = (char*)MALLOC(54);
	sprintf(body,"<CurrentGreenVideoGain>%u</CurrentGreenVideoGain>",CurrentGreenVideoGain);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetGreenVideoGain",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_SetRedVideoGain(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetRedVideoGain","");
}

void UPnPResponse_RenderingControl_SetGreenVideoBlackLevel(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetGreenVideoBlackLevel","");
}

void UPnPResponse_RenderingControl_GetVolumeDBRange(const void* UPnPToken, const short MinValue, const short MaxValue)
{
	char* body;
	
	body = (char*)MALLOC(55);
	sprintf(body,"<MinValue>%d</MinValue><MaxValue>%d</MaxValue>",MinValue,MaxValue);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetVolumeDBRange",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetRedVideoBlackLevel(const void* UPnPToken, const unsigned short CurrentRedVideoBlackLevel)
{
	char* body;
	
	body = (char*)MALLOC(62);
	sprintf(body,"<CurrentRedVideoBlackLevel>%u</CurrentRedVideoBlackLevel>",CurrentRedVideoBlackLevel);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetRedVideoBlackLevel",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetBlueVideoBlackLevel(const void* UPnPToken, const unsigned short CurrentBlueVideoBlackLevel)
{
	char* body;
	
	body = (char*)MALLOC(64);
	sprintf(body,"<CurrentBlueVideoBlackLevel>%u</CurrentBlueVideoBlackLevel>",CurrentBlueVideoBlackLevel);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetBlueVideoBlackLevel",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetBlueVideoGain(const void* UPnPToken, const unsigned short CurrentBlueVideoGain)
{
	char* body;
	
	body = (char*)MALLOC(52);
	sprintf(body,"<CurrentBlueVideoGain>%u</CurrentBlueVideoGain>",CurrentBlueVideoGain);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetBlueVideoGain",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_SetBlueVideoBlackLevel(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetBlueVideoBlackLevel","");
}

void UPnPResponse_RenderingControl_GetMute(const void* UPnPToken, const int CurrentMute)
{
	char* body;
	
	body = (char*)MALLOC(29);
	sprintf(body,"<CurrentMute>%d</CurrentMute>",(CurrentMute!=0?1:0));
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetMute",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_SetBlueVideoGain(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetBlueVideoGain","");
}

void UPnPResponse_RenderingControl_GetVerticalKeystone(const void* UPnPToken, const short CurrentVerticalKeystone)
{
	char* body;
	
	body = (char*)MALLOC(58);
	sprintf(body,"<CurrentVerticalKeystone>%d</CurrentVerticalKeystone>",CurrentVerticalKeystone);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetVerticalKeystone",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_SetVerticalKeystone(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetVerticalKeystone","");
}

void UPnPResponse_RenderingControl_GetBrightness(const void* UPnPToken, const unsigned short CurrentBrightness)
{
	char* body;
	
	body = (char*)MALLOC(46);
	sprintf(body,"<CurrentBrightness>%u</CurrentBrightness>",CurrentBrightness);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetBrightness",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetVolumeDB(const void* UPnPToken, const short CurrentVolume)
{
	char* body;
	
	body = (char*)MALLOC(38);
	sprintf(body,"<CurrentVolume>%d</CurrentVolume>",CurrentVolume);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetVolumeDB",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetGreenVideoBlackLevel(const void* UPnPToken, const unsigned short CurrentGreenVideoBlackLevel)
{
	char* body;
	
	body = (char*)MALLOC(66);
	sprintf(body,"<CurrentGreenVideoBlackLevel>%u</CurrentGreenVideoBlackLevel>",CurrentGreenVideoBlackLevel);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetGreenVideoBlackLevel",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_GetRedVideoGain(const void* UPnPToken, const unsigned short CurrentRedVideoGain)
{
	char* body;
	
	body = (char*)MALLOC(50);
	sprintf(body,"<CurrentRedVideoGain>%u</CurrentRedVideoGain>",CurrentRedVideoGain);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","GetRedVideoGain",body);
	FREE(body);
}

void UPnPResponse_RenderingControl_SetMute(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetMute","");
}

void UPnPResponse_RenderingControl_SetGreenVideoGain(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetGreenVideoGain","");
}

void UPnPResponse_RenderingControl_SetSharpness(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetSharpness","");
}

void UPnPResponse_RenderingControl_SetHorizontalKeystone(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetHorizontalKeystone","");
}

void UPnPResponse_RenderingControl_SetColorTemperature(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:RenderingControl:1","SetColorTemperature","");
}

void UPnPResponse_AVTransport_GetCurrentTransportActions(const void* UPnPToken, const char* unescaped_Actions)
{
	char* body;
	char *Actions = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Actions));
	
	ILibXmlEscape(Actions,unescaped_Actions);
	body = (char*)MALLOC(20+strlen(Actions));
	sprintf(body,"<Actions>%s</Actions>",Actions);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetCurrentTransportActions",body);
	FREE(body);
	FREE(Actions);
}

void UPnPResponse_AVTransport_Play(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Play","");
}

void UPnPResponse_AVTransport_Previous(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Previous","");
}

void UPnPResponse_AVTransport_Next(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Next","");
}

void UPnPResponse_AVTransport_Stop(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Stop","");
}

void UPnPResponse_AVTransport_GetTransportSettings(const void* UPnPToken, const char* unescaped_PlayMode, const char* unescaped_RecQualityMode)
{
	char* body;
	char *PlayMode = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PlayMode));
	char *RecQualityMode = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecQualityMode));
	
	ILibXmlEscape(PlayMode,unescaped_PlayMode);
	ILibXmlEscape(RecQualityMode,unescaped_RecQualityMode);
	body = (char*)MALLOC(55+strlen(PlayMode)+strlen(RecQualityMode));
	sprintf(body,"<PlayMode>%s</PlayMode><RecQualityMode>%s</RecQualityMode>",PlayMode,RecQualityMode);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetTransportSettings",body);
	FREE(body);
	FREE(PlayMode);
	FREE(RecQualityMode);
}

void UPnPResponse_AVTransport_Seek(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Seek","");
}

void UPnPResponse_AVTransport_Pause(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","Pause","");
}

void UPnPResponse_AVTransport_GetPositionInfo(const void* UPnPToken, const unsigned int Track, const char* unescaped_TrackDuration, const char* unescaped_TrackMetaData, const char* unescaped_TrackURI, const char* unescaped_RelTime, const char* unescaped_AbsTime, const int RelCount, const int AbsCount)
{
	char* body;
	char *TrackDuration = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TrackDuration));
	char *TrackMetaData = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TrackMetaData));
	char *TrackURI = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_TrackURI));
	char *RelTime = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RelTime));
	char *AbsTime = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_AbsTime));
	
	ILibXmlEscape(TrackDuration,unescaped_TrackDuration);
	ILibXmlEscape(TrackMetaData,unescaped_TrackMetaData);
	ILibXmlEscape(TrackURI,unescaped_TrackURI);
	ILibXmlEscape(RelTime,unescaped_RelTime);
	ILibXmlEscape(AbsTime,unescaped_AbsTime);
	body = (char*)MALLOC(212+strlen(TrackDuration)+strlen(TrackMetaData)+strlen(TrackURI)+strlen(RelTime)+strlen(AbsTime));
	sprintf(body,"<Track>%u</Track><TrackDuration>%s</TrackDuration><TrackMetaData>%s</TrackMetaData><TrackURI>%s</TrackURI><RelTime>%s</RelTime><AbsTime>%s</AbsTime><RelCount>%d</RelCount><AbsCount>%d</AbsCount>",Track,TrackDuration,TrackMetaData,TrackURI,RelTime,AbsTime,RelCount,AbsCount);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetPositionInfo",body);
	FREE(body);
	FREE(TrackDuration);
	FREE(TrackMetaData);
	FREE(TrackURI);
	FREE(RelTime);
	FREE(AbsTime);
}

void UPnPResponse_AVTransport_GetTransportInfo(const void* UPnPToken, const char* unescaped_CurrentTransportState, const char* unescaped_CurrentTransportStatus, const char* unescaped_CurrentSpeed)
{
	char* body;
	char *CurrentTransportState = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentTransportState));
	char *CurrentTransportStatus = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentTransportStatus));
	char *CurrentSpeed = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentSpeed));
	
	ILibXmlEscape(CurrentTransportState,unescaped_CurrentTransportState);
	ILibXmlEscape(CurrentTransportStatus,unescaped_CurrentTransportStatus);
	ILibXmlEscape(CurrentSpeed,unescaped_CurrentSpeed);
	body = (char*)MALLOC(126+strlen(CurrentTransportState)+strlen(CurrentTransportStatus)+strlen(CurrentSpeed));
	sprintf(body,"<CurrentTransportState>%s</CurrentTransportState><CurrentTransportStatus>%s</CurrentTransportStatus><CurrentSpeed>%s</CurrentSpeed>",CurrentTransportState,CurrentTransportStatus,CurrentSpeed);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetTransportInfo",body);
	FREE(body);
	FREE(CurrentTransportState);
	FREE(CurrentTransportStatus);
	FREE(CurrentSpeed);
}

void UPnPResponse_AVTransport_SetAVTransportURI(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","SetAVTransportURI","");
}

void UPnPResponse_AVTransport_GetDeviceCapabilities(const void* UPnPToken, const char* unescaped_PlayMedia, const char* unescaped_RecMedia, const char* unescaped_RecQualityModes)
{
	char* body;
	char *PlayMedia = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PlayMedia));
	char *RecMedia = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecMedia));
	char *RecQualityModes = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecQualityModes));
	
	ILibXmlEscape(PlayMedia,unescaped_PlayMedia);
	ILibXmlEscape(RecMedia,unescaped_RecMedia);
	ILibXmlEscape(RecQualityModes,unescaped_RecQualityModes);
	body = (char*)MALLOC(80+strlen(PlayMedia)+strlen(RecMedia)+strlen(RecQualityModes));
	sprintf(body,"<PlayMedia>%s</PlayMedia><RecMedia>%s</RecMedia><RecQualityModes>%s</RecQualityModes>",PlayMedia,RecMedia,RecQualityModes);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetDeviceCapabilities",body);
	FREE(body);
	FREE(PlayMedia);
	FREE(RecMedia);
	FREE(RecQualityModes);
}

void UPnPResponse_AVTransport_SetPlayMode(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","SetPlayMode","");
}

void UPnPResponse_AVTransport_GetMediaInfo(const void* UPnPToken, const unsigned int NrTracks, const char* unescaped_MediaDuration, const char* unescaped_CurrentURI, const char* unescaped_CurrentURIMetaData, const char* unescaped_NextURI, const char* unescaped_NextURIMetaData, const char* unescaped_PlayMedium, const char* unescaped_RecordMedium, const char* unescaped_WriteStatus)
{
	char* body;
	char *MediaDuration = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_MediaDuration));
	char *CurrentURI = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentURI));
	char *CurrentURIMetaData = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_CurrentURIMetaData));
	char *NextURI = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_NextURI));
	char *NextURIMetaData = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_NextURIMetaData));
	char *PlayMedium = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PlayMedium));
	char *RecordMedium = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_RecordMedium));
	char *WriteStatus = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_WriteStatus));
	
	ILibXmlEscape(MediaDuration,unescaped_MediaDuration);
	ILibXmlEscape(CurrentURI,unescaped_CurrentURI);
	ILibXmlEscape(CurrentURIMetaData,unescaped_CurrentURIMetaData);
	ILibXmlEscape(NextURI,unescaped_NextURI);
	ILibXmlEscape(NextURIMetaData,unescaped_NextURIMetaData);
	ILibXmlEscape(PlayMedium,unescaped_PlayMedium);
	ILibXmlEscape(RecordMedium,unescaped_RecordMedium);
	ILibXmlEscape(WriteStatus,unescaped_WriteStatus);
	body = (char*)MALLOC(265+strlen(MediaDuration)+strlen(CurrentURI)+strlen(CurrentURIMetaData)+strlen(NextURI)+strlen(NextURIMetaData)+strlen(PlayMedium)+strlen(RecordMedium)+strlen(WriteStatus));
	sprintf(body,"<NrTracks>%u</NrTracks><MediaDuration>%s</MediaDuration><CurrentURI>%s</CurrentURI><CurrentURIMetaData>%s</CurrentURIMetaData><NextURI>%s</NextURI><NextURIMetaData>%s</NextURIMetaData><PlayMedium>%s</PlayMedium><RecordMedium>%s</RecordMedium><WriteStatus>%s</WriteStatus>",NrTracks,MediaDuration,CurrentURI,CurrentURIMetaData,NextURI,NextURIMetaData,PlayMedium,RecordMedium,WriteStatus);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:AVTransport:1","GetMediaInfo",body);
	FREE(body);
	FREE(MediaDuration);
	FREE(CurrentURI);
	FREE(CurrentURIMetaData);
	FREE(NextURI);
	FREE(NextURIMetaData);
	FREE(PlayMedium);
	FREE(RecordMedium);
	FREE(WriteStatus);
}

void UPnPResponse_ConnectionManager_GetCurrentConnectionInfo(const void* UPnPToken, const int RcsID, const int AVTransportID, const char* unescaped_ProtocolInfo, const char* unescaped_PeerConnectionManager, const int PeerConnectionID, const char* unescaped_Direction, const char* unescaped_Status)
{
	char* body;
	char *ProtocolInfo = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_ProtocolInfo));
	char *PeerConnectionManager = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_PeerConnectionManager));
	char *Direction = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Direction));
	char *Status = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Status));
	
	ILibXmlEscape(ProtocolInfo,unescaped_ProtocolInfo);
	ILibXmlEscape(PeerConnectionManager,unescaped_PeerConnectionManager);
	ILibXmlEscape(Direction,unescaped_Direction);
	ILibXmlEscape(Status,unescaped_Status);
	body = (char*)MALLOC(233+strlen(ProtocolInfo)+strlen(PeerConnectionManager)+strlen(Direction)+strlen(Status));
	sprintf(body,"<RcsID>%d</RcsID><AVTransportID>%d</AVTransportID><ProtocolInfo>%s</ProtocolInfo><PeerConnectionManager>%s</PeerConnectionManager><PeerConnectionID>%d</PeerConnectionID><Direction>%s</Direction><Status>%s</Status>",RcsID,AVTransportID,ProtocolInfo,PeerConnectionManager,PeerConnectionID,Direction,Status);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionInfo",body);
	FREE(body);
	FREE(ProtocolInfo);
	FREE(PeerConnectionManager);
	FREE(Direction);
	FREE(Status);
}

void UPnPResponse_ConnectionManager_PrepareForConnection(const void* UPnPToken, const int ConnectionID, const int AVTransportID, const int RcsID)
{
	char* body;
	
	body = (char*)MALLOC(109);
	sprintf(body,"<ConnectionID>%d</ConnectionID><AVTransportID>%d</AVTransportID><RcsID>%d</RcsID>",ConnectionID,AVTransportID,RcsID);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","PrepareForConnection",body);
	FREE(body);
}

void UPnPResponse_ConnectionManager_ConnectionComplete(const void* UPnPToken)
{
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","ConnectionComplete","");
}

void UPnPResponse_ConnectionManager_GetProtocolInfo(const void* UPnPToken, const char* unescaped_Source, const char* unescaped_Sink)
{
	char* body;
	char *Source = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Source));
	char *Sink = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_Sink));
	
	ILibXmlEscape(Source,unescaped_Source);
	ILibXmlEscape(Sink,unescaped_Sink);
	body = (char*)MALLOC(31+strlen(Source)+strlen(Sink));
	sprintf(body,"<Source>%s</Source><Sink>%s</Sink>",Source,Sink);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetProtocolInfo",body);
	FREE(body);
	FREE(Source);
	FREE(Sink);
}

void UPnPResponse_ConnectionManager_GetCurrentConnectionIDs(const void* UPnPToken, const char* unescaped_ConnectionIDs)
{
	char* body;
	char *ConnectionIDs = (char*)MALLOC(1+ILibXmlEscapeLength(unescaped_ConnectionIDs));
	
	ILibXmlEscape(ConnectionIDs,unescaped_ConnectionIDs);
	body = (char*)MALLOC(32+strlen(ConnectionIDs));
	sprintf(body,"<ConnectionIDs>%s</ConnectionIDs>",ConnectionIDs);
	UPnPResponseGeneric(UPnPToken,"urn:schemas-upnp-org:service:ConnectionManager:1","GetCurrentConnectionIDs",body);
	FREE(body);
	FREE(ConnectionIDs);
}

void UPnPSendEventSink(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* subscriber, void *upnp)
{
	if(done!=0 && ((struct SubscriberInfo*)subscriber)->Disposing==0)
	{
		sem_wait(&(((struct UPnPDataObject*)upnp)->EventLock));
		--((struct SubscriberInfo*)subscriber)->RefCount;
		if(((struct SubscriberInfo*)subscriber)->RefCount==0)
		{
			LVL3DEBUG(printf("\r\n\r\nSubscriber at [%s] %d.%d.%d.%d:%d was/did UNSUBSCRIBE while trying to send event\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
			UPnPDestructSubscriberInfo(((struct SubscriberInfo*)subscriber));
		}
		else if(header==NULL)
		{
			LVL3DEBUG(printf("\r\n\r\nCould not deliver event for [%s] %d.%d.%d.%d:%d UNSUBSCRIBING\r\n\r\n",((struct SubscriberInfo*)subscriber)->SID,(((struct SubscriberInfo*)subscriber)->Address&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>8)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>16)&0xFF),((((struct SubscriberInfo*)subscriber)->Address>>24)&0xFF),((struct SubscriberInfo*)subscriber)->Port);)
			// Could not send Event, so unsubscribe the subscriber
			((struct SubscriberInfo*)subscriber)->Disposing = 1;
			UPnPExpireSubscriberInfo(upnp,subscriber);
		}
		sem_post(&(((struct UPnPDataObject*)upnp)->EventLock));
	}
}
void UPnPSendEvent_Body(void *upnptoken,char *body,int bodylength,struct SubscriberInfo *info)
{
	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;
	struct sockaddr_in dest;
	int packetLength;
	char *packet;
	int ipaddr;
	
	memset(&dest,0,sizeof(dest));
	dest.sin_addr.s_addr = info->Address;
	dest.sin_port = htons(info->Port);
	dest.sin_family = AF_INET;
	ipaddr = info->Address;
	
	packet = (char*)MALLOC(info->PathLength + bodylength + 383);
	packetLength = sprintf(packet,"NOTIFY %s HTTP/1.1\r\nHOST: %d.%d.%d.%d:%d\r\nContent-Type: text/xml\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\nSID: %s\r\nSEQ: %d\r\nContent-Length: %d\r\n\r\n<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s></e:property></e:propertyset>",info->Path,(ipaddr&0xFF),((ipaddr>>8)&0xFF),((ipaddr>>16)&0xFF),((ipaddr>>24)&0xFF),info->Port,info->SID,info->SEQ,bodylength+137,body);
	++info->SEQ;
	
	++info->RefCount;
	ILibAddRequest_Direct(UPnPObject->EventClient,packet,packetLength,&dest,&UPnPSendEventSink,info,upnptoken);
}
void UPnPSendEvent(void *upnptoken, char* body, const int bodylength, const char* eventname)
{
	struct SubscriberInfo *info = NULL;
	struct UPnPDataObject* UPnPObject = (struct UPnPDataObject*)upnptoken;
	struct sockaddr_in dest;
	LVL3DEBUG(struct timeval tv;)
	
	if(UPnPObject==NULL)
	{
		FREE(body);
		return;
	}
	sem_wait(&(UPnPObject->EventLock));
	if(strncmp(eventname,"ConnectionManager",17)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_ConnectionManager;
	}
	if(strncmp(eventname,"AVTransport",11)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_AVTransport;
	}
	if(strncmp(eventname,"RenderingControl",16)==0)
	{
		info = UPnPObject->HeadSubscriberPtr_RenderingControl;
	}
	memset(&dest,0,sizeof(dest));
	while(info!=NULL)
	{
		if(!UPnPSubscriptionExpired(info))
		{
			UPnPSendEvent_Body(upnptoken,body,bodylength,info);
		}
		else
		{
			//Remove Subscriber
			LVL3DEBUG(printf("\r\n\r\nTIMESTAMP: %d\r\n",GetTickCount()/1000);)
			LVL3DEBUG(printf("Did not renew [%s] %d.%d.%d.%d:%d UNSUBSCRIBING <%d>\r\n\r\n",((struct SubscriberInfo*)info)->SID,(((struct SubscriberInfo*)info)->Address&0xFF),((((struct SubscriberInfo*)info)->Address>>8)&0xFF),((((struct SubscriberInfo*)info)->Address>>16)&0xFF),((((struct SubscriberInfo*)info)->Address>>24)&0xFF),((struct SubscriberInfo*)info)->Port,info);)
		}
		
		info = info->Next;
	}
	
	sem_post(&(UPnPObject->EventLock));
}

void UPnPSetState_RenderingControl_LastChange(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->RenderingControl_LastChange != NULL) FREE(UPnPObject->RenderingControl_LastChange);
	UPnPObject->RenderingControl_LastChange = valstr;
	body = (char*)MALLOC(30 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","LastChange",valstr,"LastChange");
	UPnPSendEvent(upnptoken,body,bodylength,"RenderingControl");
	FREE(body);
}

void UPnPSetState_AVTransport_LastChange(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->AVTransport_LastChange != NULL) FREE(UPnPObject->AVTransport_LastChange);
	UPnPObject->AVTransport_LastChange = valstr;
	body = (char*)MALLOC(30 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","LastChange",valstr,"LastChange");
	UPnPSendEvent(upnptoken,body,bodylength,"AVTransport");
	FREE(body);
}

void UPnPSetState_ConnectionManager_SourceProtocolInfo(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ConnectionManager_SourceProtocolInfo != NULL) FREE(UPnPObject->ConnectionManager_SourceProtocolInfo);
	UPnPObject->ConnectionManager_SourceProtocolInfo = valstr;
	body = (char*)MALLOC(46 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SourceProtocolInfo",valstr,"SourceProtocolInfo");
	UPnPSendEvent(upnptoken,body,bodylength,"ConnectionManager");
	FREE(body);
}

void UPnPSetState_ConnectionManager_SinkProtocolInfo(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ConnectionManager_SinkProtocolInfo != NULL) FREE(UPnPObject->ConnectionManager_SinkProtocolInfo);
	UPnPObject->ConnectionManager_SinkProtocolInfo = valstr;
	body = (char*)MALLOC(42 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","SinkProtocolInfo",valstr,"SinkProtocolInfo");
	UPnPSendEvent(upnptoken,body,bodylength,"ConnectionManager");
	FREE(body);
}

void UPnPSetState_ConnectionManager_CurrentConnectionIDs(void *upnptoken, char* val)
{
	struct UPnPDataObject *UPnPObject = (struct UPnPDataObject*)upnptoken;
	char* body;
	int bodylength;
	char* valstr;
	valstr = (char*)MALLOC(ILibXmlEscapeLength(val)+1);
	ILibXmlEscape(valstr,val);
	if (UPnPObject->ConnectionManager_CurrentConnectionIDs != NULL) FREE(UPnPObject->ConnectionManager_CurrentConnectionIDs);
	UPnPObject->ConnectionManager_CurrentConnectionIDs = valstr;
	body = (char*)MALLOC(50 + (int)strlen(valstr));
	bodylength = sprintf(body,"%s>%s</%s","CurrentConnectionIDs",valstr,"CurrentConnectionIDs");
	UPnPSendEvent(upnptoken,body,bodylength,"ConnectionManager");
	FREE(body);
}


void UPnPDestroyMicroStack(void *object)
{
	struct UPnPDataObject *upnp = (struct UPnPDataObject*)object;
	struct SubscriberInfo  *sinfo,*sinfo2;
	int i;
	UPnPSendByeBye(upnp);
	
	sem_destroy(&(upnp->EventLock));
	FREE(upnp->ConnectionManager_SourceProtocolInfo);
	FREE(upnp->ConnectionManager_SinkProtocolInfo);
	FREE(upnp->ConnectionManager_CurrentConnectionIDs);
	FREE(upnp->AVTransport_LastChange);
	FREE(upnp->RenderingControl_LastChange);
	
	FREE(upnp->AddressList);
	FREE(upnp->NOTIFY_SEND_socks);
	FREE(upnp->UUID);
	FREE(upnp->Serial);
	FREE(upnp->DeviceDescription);
	
	sinfo = upnp->HeadSubscriberPtr_ConnectionManager;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UPnPDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_AVTransport;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UPnPDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	sinfo = upnp->HeadSubscriberPtr_RenderingControl;
	while(sinfo!=NULL)
	{
		sinfo2 = sinfo->Next;
		UPnPDestructSubscriberInfo(sinfo);
		sinfo = sinfo2;
	}
	
	for(i=0;i<5;++i)
	{
		if(upnp->ReaderObjects[i].Body!=NULL) {FREE(upnp->ReaderObjects[i].Body);}
		if(upnp->ReaderObjects[i].ParsedHeader!=NULL) {ILibDestructPacket(upnp->ReaderObjects[i].ParsedHeader);}
	}
	WSACleanup();
}
int UPnPGetLocalPortNumber(void *token)
{
	return(((struct UPnPDataObject*)token)->WebSocketPortNumber);
}
void *UPnPCreateMicroStack(void *Chain, const char* FriendlyName, const char* UDN, const char* SerialNumber, const int NotifyCycleSeconds, const unsigned short PortNum)
{
	struct UPnPDataObject* RetVal = (struct UPnPDataObject*)MALLOC(sizeof(struct UPnPDataObject));
	char* DDT;
	WORD wVersionRequested;
	WSADATA wsaData;
	
	srand((int)GetTickCount());
	wVersionRequested = MAKEWORD( 1, 1 );
	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1);}
	UPnPInit(RetVal,NotifyCycleSeconds,PortNum);
	RetVal->ForceExit = 0;
	RetVal->PreSelect = &UPnPMasterPreSelect;
	RetVal->PostSelect = &UPnPMasterPostSelect;
	RetVal->Destroy = &UPnPDestroyMicroStack;
	RetVal->InitialNotify = 0;
	if (UDN != NULL)
	{
		RetVal->UUID = (char*)MALLOC((int)strlen(UDN)+6);
		sprintf(RetVal->UUID,"uuid:%s",UDN);
		RetVal->UDN = RetVal->UUID + 5;
	}
	if (SerialNumber != NULL)
	{
		RetVal->Serial = (char*)MALLOC((int)strlen(SerialNumber)+1);
		strcpy(RetVal->Serial,SerialNumber);
	}
	
	RetVal->DeviceDescription = (char*)MALLOC(UPnPDeviceDescriptionTemplateLengthUX + (int)strlen(FriendlyName) + (((int)strlen(RetVal->Serial) + (int)strlen(RetVal->UUID)) * 1));
	DDT = ILibDecompressString((char*)UPnPDeviceDescriptionTemplate,UPnPDeviceDescriptionTemplateLength,UPnPDeviceDescriptionTemplateLengthUX);
	RetVal->DeviceDescriptionLength = sprintf(RetVal->DeviceDescription,DDT,FriendlyName,RetVal->Serial,RetVal->UDN);
	FREE(DDT);
	RetVal->ConnectionManager_SourceProtocolInfo = NULL;
	RetVal->ConnectionManager_SinkProtocolInfo = NULL;
	RetVal->ConnectionManager_CurrentConnectionIDs = NULL;
	RetVal->AVTransport_LastChange = NULL;
	RetVal->RenderingControl_LastChange = NULL;
	
	RetVal->WebServerTimer = ILibCreateLifeTime(Chain);
	
	ILibAddToChain(Chain,RetVal);
	RetVal->EventClient = ILibCreateHTTPClientModule(Chain,5);
	RetVal->Chain = Chain;
	RetVal->UpdateFlag = 0;
	
	sem_init(&(RetVal->EventLock),0,1);
	return(RetVal);
}

