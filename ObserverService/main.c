#include <Windows.h>

#define SVCNAME TEXT("ObserverService")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

VOID WINAPI SvcMain(DWORD dwNumServicesArgs, LPTSTR *lpServiceArgVectors);

SERVICE_TABLE_ENTRY DispatchTable[] =
{
	{ SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
	{ NULL, NULL }
};

int main(int argc, char* argv[])
{
}
