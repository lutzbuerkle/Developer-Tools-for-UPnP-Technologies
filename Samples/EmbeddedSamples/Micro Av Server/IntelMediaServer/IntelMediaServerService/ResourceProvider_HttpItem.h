#ifndef _RESOURCE_PROVIDER_HTTP_ITEM_H
#define _RESOURCE_PROVIDER_HTTP_ITEM_H

#include "ResourceProvider.h"

class DiskIO;
//	Provides HTTP-GET resources for all item objects in the CDS.
//	Also ties into the DeviceBuilder-generated HTTP web server modules
//	to provide the HTTP transfer capabilities.
class ResourceProvider_HttpItem : public ResourceProvider
{
private:
	void*			m_TransferTable;		// TransferTable containing Sessions
	DiskIO*			m_Disk;					// Async Disk IO 
	MediaDatabase*	m_Database;				// pointer to database
	void*			m_WebServer;			// webserver token
	int				m_PortNumber;			// port number of HTTP request
	char			m_VirDirName[24];		// virtual directory name
	int				m_VirDirNameLen;		// length of virtual directory name

	//	Any time a URI (generated by this ResourceProvider) is requested, the request is routed to this
	//	static method. 
	static void Sink_HandleWebRequest(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user);

	//	Called whenever the asynchronous socket finished sending a block of data that was given to it.
	static void Sink_HandleSendOK(struct ILibWebServer_Session *session);

	//	Called whenever the transfer session ends.
	static void Sink_HandleDisconnect(struct ILibWebServer_Session *session);

	//	Called whenever a DiskIO operation completes
	static void Sink_ReadFileEx(HANDLE fileHandle, char* buffer, int dwNumberOfBytesTransfered, void *user);

	//	The static method calls this method.
	void Sink_HandleWebRequest (struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done);

public:
	//	Constructor.
	//
	//	Requires a webserver token.
	//	(acquired from ContentDirectoryServer.GetWebServerToken())
	//
	//	Requires a pointer to the database for doing a lookup
	//	for handling an HTTP-GET requests.
	ResourceProvider_HttpItem(MediaDatabase *database, void* webServerToken);

	//	Destructor.
	virtual ~ResourceProvider_HttpItem();

	//	Given content metadata (eg, dbEntry)
	//	and the IP addresses available for the system, the implementation
	//	of this method will return a linked list of CdsMediaResource objects
	//	to represent the 'res' elements of the content.
	//
	//	The purpose of sending the IP addresses is to allow this method's
	//	implementation to return multiple CdsMediaResource objects, so
	//	that their is one 'res' element for each IP address of the content.
	//
	//	This method only returns resources only when dbEntry->Class is
	//	equal to 
	//		MEDIA_DB_CL_IMAGE,
	//		MEDIA_DB_CL_AUDIO,
	//		MEDIA_DB_CL_VIDEO,
	//		MEDIA_DB_CL_OTHER.
	//	All other forms of database entries will cause this method to return NULL.
	virtual struct CdsMediaResource* GetResources(const MediaItemData* dbEntry, const int *ipAddrList, const int ipAddrListLen);

	//	Returns the number of HTTP transfers associated with this ResourceProvider.
	virtual int GetNumTransfers();

	//	Returns a thread-safe array of TransferProgress objects.
	//
	//	The caller can specify the range of desired transfers. 
	//	This is useful for user-interfaces that want to request
	//	a subset of the transfers that can be displayed.
	//
	//	startIndex represents the zero-based index of the first transfer.
	//	maxCount represents the maximum number of TransferProgress objects to return.
	//	numReturned represents the size of the TransferProgress array.
	virtual TransferProgress** GetTransferList(int startIndex, int maxCount, int *numReturned);
	virtual void DestroyTransferList(TransferProgress **tp);
};

#endif
