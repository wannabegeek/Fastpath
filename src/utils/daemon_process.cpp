 /***************************************************************************
                           DaemonProcess.cpp
                          -------------------
    copyright            : (C) 2004 by Tom Fewster
    email                : tom@wannabegeek.com
    version              : $Revision: 1.6 $
    date                 : $Date: 2004/10/05 21:08:04 $

 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the Free Software             *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "daemon_process.h"

#ifdef _WIN32
#	include <windows.h>
	TCHAR* serviceName = TEXT("dcserverd");
	SERVICE_STATUS serviceStatus;
	SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
#else
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <unistd.h>
#endif


#include <iostream>

namespace tf {
    void run_as_daemon() {
#ifdef _WIN32
#else
        pid_t pid;
        if ((pid = fork()) != 0) {
            exit(0);  // kill the parent
        }
        setsid();
        umask(0);
#endif
    }

    bool InstallService() {
#ifdef _WIN32
        SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
        if (serviceControlManager) {
            char path[ _MAX_PATH + 1 ];
            if (GetModuleFileName(0, path, sizeof(path)/sizeof(path[0])) > 0) {
                SC_HANDLE service = CreateService(serviceControlManager, serviceName, serviceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path, 0, 0, 0, 0, 0);
                if (service) {
                    CloseServiceHandle(service);
                }
            }
            CloseServiceHandle(serviceControlManager);
        }
        return true;
#else
        std::cerr << "InstallService not supported on unix" << std::endl;
        return false;
#endif
    }

    bool UninstallService() {
#ifdef _WIN32
        SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
        if (serviceControlManager) {
            SC_HANDLE service = OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | DELETE);
            if (service) {
                SERVICE_STATUS serviceStatus;
                if (QueryServiceStatus(service, &serviceStatus)) {
                    if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
                        DeleteService(service);
                }

                CloseServiceHandle(service);
            }
            CloseServiceHandle(serviceControlManager);
        }
        return true;
#else
        std::cerr << "InstallService not supported on unix" << std::endl;
        return false;
#endif
    }

#ifdef _WIN32
     void WINAPI ServiceControlHandler(DWORD controlCode) {
         switch (controlCode) {
             case SERVICE_CONTROL_INTERROGATE:
                 break;
             case SERVICE_CONTROL_SHUTDOWN:
             case SERVICE_CONTROL_STOP:
                 serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
                 SetServiceStatus(serviceStatusHandle, &serviceStatus);
                 // we need to shut down the process
                 entryShutdown();
                 return;
             case SERVICE_CONTROL_PAUSE:
                 break;
             case SERVICE_CONTROL_CONTINUE:
                 break;
             default:
                 if (controlCode >= 128 && controlCode <= 255)
                     // user defined control code
                     break;
                 else
                     // unrecognised control code
                     break;
         }
         SetServiceStatus(serviceStatusHandle, &serviceStatus);
     }

     void WINAPI ServiceMain(DWORD argc, TCHAR* argv[]) {
         // initialise service status
         serviceStatus.dwServiceType = SERVICE_WIN32;
         serviceStatus.dwCurrentState = SERVICE_STOPPED;
         serviceStatus.dwControlsAccepted = 0;
         serviceStatus.dwWin32ExitCode = NO_ERROR;
         serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
         serviceStatus.dwCheckPoint = 0;
         serviceStatus.dwWaitHint = 0;

         serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, ServiceControlHandler);

         if (serviceStatusHandle) {
             // Tell the service manager service is starting
             serviceStatus.dwCurrentState = SERVICE_START_PENDING;
             SetServiceStatus(serviceStatusHandle, &serviceStatus);

             // Tell the service manager we are running
             serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
             serviceStatus.dwCurrentState = SERVICE_RUNNING;
             SetServiceStatus(serviceStatusHandle, &serviceStatus);

             // Start the main method
             entryMain(argc, argv);

             // since we have fallen out of our loop service was stopped
             serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
             SetServiceStatus(serviceStatusHandle, &serviceStatus);

             // service is now stopped
             serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
             serviceStatus.dwCurrentState = SERVICE_STOPPED;
             SetServiceStatus(serviceStatusHandle, &serviceStatus);
         }
     }
#endif
}