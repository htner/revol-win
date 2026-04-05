#include <windows.h>

typedef void* (*_custom_alloc_ptr)(size_t size);
typedef void (*_custom_free_ptr)(void* p);


_custom_alloc_ptr	g__channel_custom_alloc = NULL;
_custom_free_ptr	g_channle_custom_free = NULL;

void* _temp_alloc_(size_t size){
	return g__channel_custom_alloc(size);
}
void _temp_free_(void* p){
	g_channle_custom_free(p);
}


extern "C" __declspec(dllexport) void set_channel_mem_alloc(void* alloc_ptr,void* free_ptr)
{
	g__channel_custom_alloc = (_custom_alloc_ptr)alloc_ptr;
	g_channle_custom_free = (_custom_free_ptr)free_ptr;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	return TRUE;
}

struct NetInit{
	NetInit(){
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD( 1, 1 ), &wsaData);
	}
};

static NetInit netinit;