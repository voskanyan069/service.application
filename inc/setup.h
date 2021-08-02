#ifndef SETUP_H
#define SETUP_H

#include <string>

const wchar_t* to_wchart(std::string str)
{
	std::wstring wide_str = std::wstring(str.begin(), str.end());
	return wide_str.c_str();
}

void install_windows_service(std::string name, std::string display,
		std::string mode, std::string path)
{
	DWORD mode = SERVICE_AUTO_START;
	if (mode == "manual")
	{
		mode = SERVICE_DEMAND_START;
	}
	install_windows_service_(
			to_wchart(name),
			to_wchart(display),
			mode,
			to_wchart(path)
			);
}

void uninstall_windows_service(std::string name)
{
	uninstall_windows_service_(to_wchart(name));
}

void install_windows_service_(const wchar_t* SVCNAME, const wchar_t* SVCDISPLAY,
		DWORD SVCMODE, const wchar_t* SVCPATH)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

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
		SVCDISPLAY,                // service name to display 
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

void uninstall_windows_service_(const wchar_t* szSvcName)
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
