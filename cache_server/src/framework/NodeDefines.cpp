#include "NodeDefines.h"

volatile bool g_bExit = false;
volatile long g_serverStatus = SERVER_STATUS_IDLE;
int64_t g_recordExpireSec = 1;
