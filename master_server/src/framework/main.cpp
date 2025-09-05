/*
 * File:  main.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_3_1 PM 10:32
 */

#include "Common.h"
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
#include "ExceptionManager.h"
#include "CrashDump.h"
#include <conio.h>

#define CH_ESC 27
#define CH_BACKSPACE 8
#define CH_ENTER 13

#define CH_DIRECT 224
#define CH_UP 72
#define CH_DOWN 80
#define CH_LEFT 75
#define CH_RIGHT 77
#else
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <thread>
#include <bitset>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define CH_ESC 27
#define CH_BACKSPACE 127
#define CH_ENTER 10

#define CH_DIRECT 91
#define CH_UP 65
#define CH_DOWN 66
#define CH_LEFT 68
#define CH_RIGHT 67

void _inittcattr(void)
{
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
}

#define _getch getchar

int _kbhit(void)
{
    int bytesWaiting;
    ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

#endif

#include <iostream>
#include "MasterServer.h"
#include "CommandManager.h"
#include "TimestampManager.h"

using namespace evt;
using namespace util;

const char VERTION_TRUNK = 0;
const char VERTION_BRANCH = 1;



#define ROLLBACK_SIZE 5

int CommandHelp(const util::CWeakPointer<ArgumentBase>& arg) {
	CommandManager::PTR_T pCmdMgr(CommandManager::Pointer());
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
	SHOW_COMMAND_LIST_T outList;
	pCmdMgr->ShowCommand(outList);
	const int nMaxSpace = 20;
	std::string strSpace;
	printf("***************************** %s V%u.%u *******************************\n",
		strServerName.c_str(),VERTION_TRUNK,VERTION_BRANCH);
	printf("Commands:\n");
	SHOW_COMMAND_LIST_T::iterator it = outList.begin();
	for(;outList.end() != it; ++it) {
		int nCount = nMaxSpace - it->cmd.size();
		if(nCount > 0) {
			strSpace.resize(nCount,' ');
		}
		printf("'%s'%s %s\n\n",it->cmd.substr(0,nMaxSpace).c_str(),strSpace.c_str(),it->spec.c_str());
	}
	printf("********************************************************************************\n");
	return COMMAND_RESULT_SUCCESS;
}

int CommandClear(const util::CWeakPointer<ArgumentBase>& arg) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
	system("cls");
#else
    //system("clear");
    printf("\033c");
#endif
	return COMMAND_RESULT_SUCCESS;
}

int CommandQuit(const util::CWeakPointer<ArgumentBase>& arg) {
    atomic_xchg8(&g_bExit, true);
	return COMMAND_RESULT_EXIT;
}

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
BOOL WINAPI ConsoleHandler(DWORD msgType) {
	if(msgType == CTRL_CLOSE_EVENT) {
		atomic_xchg8(&g_bExit, true);
		return TRUE;
	} else if (msgType == CTRL_C_EVENT) {
        if(MessageBox(NULL, TEXT("Do you really want to exit ?"), TEXT("MessageBox"), MB_OKCANCEL) == IDOK) {
            atomic_xchg8(&g_bExit, true);
        }
        return TRUE;
    } else if(msgType == CTRL_SHUTDOWN_EVENT) {
		atomic_xchg8(&g_bExit, true);
	}
    return FALSE;
}
#else 
void SignalHandler(int nSignalValue) {
	atomic_xchg8(&g_bExit, true);
}
#endif


#define COMMAND_DONE()\
startCommIdx = -1;\
startParamIdx = -1;\
offset = -1;\
szBuf[0] = '\0';\
printf("\n%s",prompt)

struct CommandRollback {
    std::string input;
    int offset;
    int startCommIdx;
    int startParamIdx;
};

extern void LoadAppConfig();

int main(int argc, char** argv) {

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
	Frame::CrashDump::Begin(UnknowExceptionHandler, MiniDumpWithFullMemory);

	HWND hWnd = GetConsoleWindow();
	HMENU hMenu = NULL;
	if(NULL != hWnd) {
		hMenu = GetSystemMenu(hWnd, FALSE);
		if(NULL != hMenu) {
			DeleteMenu(hMenu, SC_MAXIMIZE, MF_GRAYED | MF_DISABLED);
			DeleteMenu(hMenu, SC_SIZE, MF_GRAYED | MF_DISABLED);
			DeleteMenu(hMenu, SC_CLOSE , MF_GRAYED | MF_DISABLED);
			DrawMenuBar(hWnd);
		}
	}
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
#else
    _inittcattr();
	
	struct sigaction action;
	action.sa_handler = SignalHandler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
#endif

	CTimestampManager::Pointer();

	LoadAppConfig();

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
	printf("**************************** %s V%u.%u *******************************\n",
		strServerName.c_str(), VERTION_TRUNK, VERTION_BRANCH);

	// initialize
	CMasterServer::PTR_T pInstance(CMasterServer::Pointer());
	if(!pInstance->Init(argc, argv)){
		pInstance->Dispose();
		printf("Init() fail.\n");
		return 0;
	}

	// register command
	CommandManager::PTR_T pCmdMgr(CommandManager::Pointer());
	CAutoPointer<GlobalMethodRIP1> pCommandHelp(new GlobalMethodRIP1(CommandHelp));
	pCmdMgr->AddCommand("help", pCommandHelp, "Print help list. >> help");
	CAutoPointer<GlobalMethodRIP1> pCommandClear(new GlobalMethodRIP1(CommandClear));
	pCmdMgr->AddCommand("clear", pCommandClear, "Clear screen. >> clear");
	CAutoPointer<GlobalMethodRIP1> pCommandQuit(new GlobalMethodRIP1(CommandQuit));
	pCmdMgr->AddCommand("quit", pCommandQuit, "Quit server. >> quit");
	pCmdMgr->AddCommand("exit", pCommandQuit, "Exit server. >> exit");
	//////////////////////////////////////////////////////////////////////////

	// console
	int nKbhit = 0;
    bool bDirect = false;
    int nRollbackCurIdx = 0;
    int nRollbackOffset = 0;
    int nRollbackCount = 0;
    struct CommandRollback rollback[ROLLBACK_SIZE];
    char szBuf[MAX_BUF_SIZE] = {'\0'};
    int offset(-1);
    int startCommIdx(-1);
    int startParamIdx(-1);
    std::string strCmd;
    const char* prompt = (const char*)">> ";
    printf("You can type 'help' for some helps.\n");
    printf("%s",prompt);
    while(true) {
		if(g_bExit) {
			printf("The server is quitting now,please wait...\n");
			break;
		}
		nKbhit = _kbhit();
        if(!nKbhit) {
            Sleep(6);
            continue;
        }
        int key = _getch();

        if(CH_BACKSPACE == key) {
            bDirect = false;
            if(offset > -1) {
                szBuf[offset] = '\0';
                printf( "\b \b" );
                if(startCommIdx == offset) {
                    startCommIdx = -1;
                }
                if((startParamIdx - 1) == offset) {
                    startParamIdx = -1;
                }
                --offset;
            }
        } else if(CH_ESC == key) {
            bDirect = false;
            if(1 == nKbhit) {
                COMMAND_DONE();
            }
            continue;
        } else if(CH_DIRECT == key) {
            bDirect = true;
            continue;
        } else if(bDirect && CH_UP == key) {
            bDirect = false;
            if(--nRollbackOffset < 0) {
                nRollbackOffset = 0;
            }
            const struct CommandRollback& comRollback = rollback[nRollbackOffset];
            if(comRollback.input.empty()) {
                continue;
            }
            int nLastOffset = offset;
            memcpy(szBuf,comRollback.input.c_str(), comRollback.input.size()+1);
            offset = comRollback.offset;
            startCommIdx = comRollback.startCommIdx;
            startParamIdx = comRollback.startParamIdx;
            while(nLastOffset > 0) {
                printf("\b \b");
                --nLastOffset;
            }
            printf("\r%s%s", prompt,szBuf);
        } else if(bDirect && CH_DOWN == key) {
            bDirect = false;
            if(nRollbackCount <= 0) {
                nRollbackOffset = 0;
            } else {
                if(++nRollbackOffset >= nRollbackCount) {
                    nRollbackOffset = nRollbackCount - 1;
                }
            }
            const struct CommandRollback& comRollback = rollback[nRollbackOffset];
            if(comRollback.input.empty()) {
                continue;
            }
            int nLastOffset = offset;
            memcpy(szBuf,comRollback.input.c_str(), comRollback.input.size()+1);
            offset = comRollback.offset;
            startCommIdx = comRollback.startCommIdx;
            startParamIdx = comRollback.startParamIdx;
            while(nLastOffset > 0) {
                printf("\b \b");
                --nLastOffset;
            }
            printf("\r%s%s", prompt,szBuf);
        } else if(bDirect && CH_LEFT == key) {
            bDirect = false;
        } else if(bDirect && CH_RIGHT == key) {
            bDirect = false;
        }  else {
			if(!isprint(key) && CH_ENTER != key) {
				Sleep(30);
				continue;
			}
			bDirect = false;
            if(offset + 1 >= (int)sizeof(szBuf) - 1) {
                printf("\nThe buffer size {%d} overflow !!\n",(int)sizeof(szBuf) - 1);
                continue;
            }
            if(offset < 0) {
                offset = 0;
            } else {
                ++offset;
            }
            szBuf[offset] = key;
            printf("%c", key);
            if(-1 == startCommIdx && !isspace(key)) {
                startCommIdx = offset;
            }
            if(-1 != startCommIdx && -1 == startParamIdx && isspace(key)) {
                startParamIdx = offset + 1;
            }
        }

        if(CH_ENTER != key) {
            continue;
        }else {
            if(CH_ENTER == szBuf[offset]) {
                szBuf[offset] = '\0';
            } else {
                szBuf[offset + 1] = '\0';
            }
        }

        if(-1 == startCommIdx) {
            COMMAND_DONE();
            continue;
        }

        if(startCommIdx >= (startParamIdx - 1)) {
            COMMAND_DONE();
            continue;
        }

		bool bBackup = true;
		int nLastIdx = nRollbackCurIdx - 1;
		if(nLastIdx > -1) {
			struct CommandRollback& comLast = rollback[nLastIdx];
			bBackup = (comLast.input != szBuf);
		}
		if(bBackup) {
			struct CommandRollback& comRollback = rollback[nRollbackCurIdx];
			comRollback.input = szBuf;
			if(szBuf[offset] == '\0') {
				comRollback.offset = offset - 1;
			} else {
				comRollback.offset = offset;
			}
			comRollback.startCommIdx = startCommIdx;
			if((startParamIdx - 1) == offset) {
				comRollback.startParamIdx = -1;
			} else {
				comRollback.startParamIdx = startParamIdx;
			}
			if(nRollbackCount < ROLLBACK_SIZE) {
				++nRollbackCount;
			}
			if(++nRollbackCurIdx >= ROLLBACK_SIZE) {
				nRollbackCurIdx = 0;
			}
		}
		nRollbackOffset = nRollbackCurIdx;

        strCmd.append(szBuf + startCommIdx, (std::string::size_type)(startParamIdx - 1 - startCommIdx));
        CAutoPointer<CommandArgument> commandArgument;
        if(sizeof(szBuf) - 1 > startParamIdx && strlen(&szBuf[startParamIdx]) > 0) {
            commandArgument.SetRawPointer(new CommandArgument(&szBuf[startParamIdx]));
        }
        CommandResult rc = pCmdMgr->DispatchCommand(strCmd, commandArgument);
        if(COMMAND_RESULT_INVALID == rc) {
            printf("\n'%s' unrecognized command !\n", strCmd.c_str());
        } else if(COMMAND_RESULT_FAIL == rc) {
            printf("\n'%s' fail. \n", strCmd.c_str());
        } else if(COMMAND_RESULT_SUCCESS == rc) {
            printf("\n'%s' success. \n", strCmd.c_str());
        }
        strCmd.clear();

        COMMAND_DONE();
    }

    pInstance->Dispose();
    printf("************************************ Done *************************************\n");
    return 0;
}
