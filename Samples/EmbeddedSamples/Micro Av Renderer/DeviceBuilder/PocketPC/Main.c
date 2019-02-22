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
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "ILibParsers.h"
#include "UpnpMicroStack.h"
#include "ILibWebServer.h"

void *UpnpmicroStackChain;
void *UpnpmicroStack;

void *UpnpMonitor;
int UpnpIPAddressLength;
int *UpnpIPAddressList;

void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	printf("Invoke: UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionInfo(%d);\r\n",ConnectionID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionInfo(upnptoken,25000,25000,"Sample String","Sample String",25000,"Sample String","Sample String");
}

void UpnpMediaRenderer_ConnectionManager_GetProtocolInfo(void* upnptoken)
{
	printf("Invoke: UpnpMediaRenderer_ConnectionManager_GetProtocolInfo();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_ConnectionManager_GetProtocolInfo(upnptoken,"Sample String","Sample String");
}

void UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	printf("Invoke: UpnpMediaRenderer_ConnectionManager_GetCurrentConnectionIDs();\r\n");
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_ConnectionManager_GetCurrentConnectionIDs(upnptoken,"Sample String");
}

void UpnpMediaRenderer_RenderingControl_SetVolume(void* upnptoken,unsigned int InstanceID,char* Channel,unsigned short DesiredVolume)
{
	printf("Invoke: UpnpMediaRenderer_RenderingControl_SetVolume(%u,%s,%u);\r\n",InstanceID,Channel,DesiredVolume);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_RenderingControl_SetVolume(upnptoken);
}

void UpnpMediaRenderer_RenderingControl_GetMute(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UpnpMediaRenderer_RenderingControl_GetMute(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_RenderingControl_GetMute(upnptoken,1);
}

void UpnpMediaRenderer_RenderingControl_SetMute(void* upnptoken,unsigned int InstanceID,char* Channel,int DesiredMute)
{
	printf("Invoke: UpnpMediaRenderer_RenderingControl_SetMute(%u,%s,%d);\r\n",InstanceID,Channel,DesiredMute);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_RenderingControl_SetMute(upnptoken);
}

void UpnpMediaRenderer_RenderingControl_GetVolume(void* upnptoken,unsigned int InstanceID,char* Channel)
{
	printf("Invoke: UpnpMediaRenderer_RenderingControl_GetVolume(%u,%s);\r\n",InstanceID,Channel);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MediaRenderer_RenderingControl_GetVolume(upnptoken,250);
}

void UpnpMedaRenderer_AVTransport_GetCurrentTransportActions(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_GetCurrentTransportActions(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_GetCurrentTransportActions(upnptoken,"Sample String");
}

void UpnpMedaRenderer_AVTransport_Play(void* upnptoken,unsigned int InstanceID,char* Speed)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_Play(%u,%s);\r\n",InstanceID,Speed);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_Play(upnptoken);
}

void UpnpMedaRenderer_AVTransport_Previous(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_Previous(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_Previous(upnptoken);
}

void UpnpMedaRenderer_AVTransport_Next(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_Next(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_Next(upnptoken);
}

void UpnpMedaRenderer_AVTransport_Stop(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_Stop(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_Stop(upnptoken);
}

void UpnpMedaRenderer_AVTransport_GetTransportSettings(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_GetTransportSettings(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_GetTransportSettings(upnptoken,"Sample String","Sample String");
}

void UpnpMedaRenderer_AVTransport_Seek(void* upnptoken,unsigned int InstanceID,char* Unit,char* Target)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_Seek(%u,%s,%s);\r\n",InstanceID,Unit,Target);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_Seek(upnptoken);
}

void UpnpMedaRenderer_AVTransport_Pause(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_Pause(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_Pause(upnptoken);
}

void UpnpMedaRenderer_AVTransport_GetPositionInfo(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_GetPositionInfo(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_GetPositionInfo(upnptoken,250,"Sample String","Sample String","Sample String","Sample String","Sample String",25000,25000);
}

void UpnpMedaRenderer_AVTransport_GetTransportInfo(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_GetTransportInfo(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_GetTransportInfo(upnptoken,"Sample String","Sample String","Sample String");
}

void UpnpMedaRenderer_AVTransport_SetAVTransportURI(void* upnptoken,unsigned int InstanceID,char* CurrentURI,char* CurrentURIMetaData)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_SetAVTransportURI(%u,%s,%s);\r\n",InstanceID,CurrentURI,CurrentURIMetaData);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_SetAVTransportURI(upnptoken);
}

void UpnpMedaRenderer_AVTransport_GetDeviceCapabilities(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_GetDeviceCapabilities(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_GetDeviceCapabilities(upnptoken,"Sample String","Sample String","Sample String");
}

void UpnpMedaRenderer_AVTransport_SetPlayMode(void* upnptoken,unsigned int InstanceID,char* NewPlayMode)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_SetPlayMode(%u,%s);\r\n",InstanceID,NewPlayMode);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_SetPlayMode(upnptoken);
}

void UpnpMedaRenderer_AVTransport_GetMediaInfo(void* upnptoken,unsigned int InstanceID)
{
	printf("Invoke: UpnpMedaRenderer_AVTransport_GetMediaInfo(%u);\r\n",InstanceID);
	
	/* TODO: Place Action Code Here... */
	
	/* UpnpResponse_Error(upnptoken,404,"Method Not Implemented"); */
	UpnpResponse_MedaRenderer_AVTransport_GetMediaInfo(upnptoken,250,"Sample String","Sample String","Sample String","Sample String","Sample String","Sample String","Sample String","Sample String");
}

void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	printf("Upnp Presentation Request: %s %s\r\n", packet->Directive,packet->DirectiveObj);
	
	/* TODO: Add Web Response Code Here... */
	printf("HOST: %x\r\n",UpnpGetLocalInterfaceToHost(upnptoken));
	
	ILibWebServer_Send_Raw(upnptoken, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n" , 38 , 1, 1);
}

DWORD WINAPI Run(LPVOID args)
{
	getchar();
	ILibStopChain(UpnpmicroStackChain);
	return 0;
}

void UpnpIPAddressMonitor(void *data)
{
	int length;
	int *list;
	
	length = ILibGetLocalIPAddressList(&list);
	if(length!=UpnpIPAddressLength || memcmp((void*)list,(void*)UpnpIPAddressList,sizeof(int)*length)!=0)
	{
		UpnpIPAddressListChanged(UpnpmicroStack);
		
		FREE(UpnpIPAddressList);
		UpnpIPAddressList = list;
		UpnpIPAddressLength = length;
	}
	else
	{
		FREE(list);
	}
	
	
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
}
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD ptid=0;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	UpnpmicroStackChain = ILibCreateChain();
	
	/* TODO: Each device must have a unique device identifier (UDN) */
	UpnpmicroStack = UpnpCreateMicroStack(UpnpmicroStackChain,"Micro Media Renderer","9d6d1ecc-ebd2-4f98-9c91-56af15b7c60d","0000001",1800,0);
	
	/* All evented state variables MUST be initialized before UPnPStart is called. */
	UpnpSetState_MediaRenderer_ConnectionManager_SourceProtocolInfo(UpnpmicroStack,"Sample String");
	UpnpSetState_MediaRenderer_ConnectionManager_SinkProtocolInfo(UpnpmicroStack,"Sample String");
	UpnpSetState_MediaRenderer_ConnectionManager_CurrentConnectionIDs(UpnpmicroStack,"Sample String");
	UpnpSetState_MediaRenderer_RenderingControl_LastChange(UpnpmicroStack,"Sample String");
	UpnpSetState_MedaRenderer_AVTransport_LastChange(UpnpmicroStack,"Sample String");
	
	printf("Intel MicroStack 1.0 - Micro Media Renderer\r\n\r\n");
	CreateThread(NULL,0,&Run,NULL,0,&ptid);
	
	UpnpMonitor = ILibCreateLifeTime(UpnpmicroStackChain);
	UpnpIPAddressLength = ILibGetLocalIPAddressList(&UpnpIPAddressList);
	ILibLifeTime_Add(UpnpMonitor,NULL,4,&UpnpIPAddressMonitor,NULL);
	
	ILibStartChain(UpnpmicroStackChain);
	
	FREE(UpnpIPAddressList);
	return 0;
}

