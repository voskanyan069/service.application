#ifndef SETUP_H
#define SETUP_H

#include <string>

/*
 * brief Install service for windows os
 * param SVCNAME Service name to install
 * param mode Service mode [auto/manual]
 * param SVCPATH Path to executable of service
 */
void install_windows_service(const wchar_t* SVCNAME,
		std::string mode, const wchar_t* SVCPATH)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];
	DWORD SVCMODE = SERVICE_DEMAND_START;
	if (mode == "auto")
	{
		SVCMODE = SERVICE_AUTO_START;
	}

	if (!GetModuleFileName(0, LPWSTR(SVCPATH), MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	schSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	schService = CreateService(
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCNAME,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SVCMODE,                   // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		SVCPATH,                   // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else
	{
		printf("Service installed successfully\n");
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

/*
 * brief Service uninstalling for windows os
 * param szSvcName Service name to uninstall
 */
void uninstall_windows_service(const wchar_t* szSvcName)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	schService = OpenService(
		schSCManager,       // SCM database 
		szSvcName,          // name of service 
		DELETE);            // need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	if (!DeleteService(schService))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
	}
	else
	{
		printf("Service deleted successfully\n");
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

#endif // SETUP_H
