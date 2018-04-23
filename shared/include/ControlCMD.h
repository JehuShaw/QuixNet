/* 
 * File:   ControlCMD.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _CONTROLCMD_H
#define	_CONTROLCMD_H

//--------------------------------------
//  Master Control CMD definition
//--------------------------------------
// C is Client, M is Master Server, T is 'To'

enum eControlCMD
{
	C_CMD_CTM_LOGIN = 1,
	C_CMD_CTM_LOGOUT,
	C_CMD_CTM_RESTART,
	C_CMD_CTM_AUTORESTART,
	C_CMD_CTM_SHUTDOWN,
	C_CMD_CTM_ERASE,
	C_CMD_CTM_SEND_MAIL,
	C_CMD_CTM_SIZE,
};

#endif	/* _CONTROLCMD_H */

