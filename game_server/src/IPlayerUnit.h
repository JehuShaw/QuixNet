/* 
 * File:   IPlayerUnit.h
 * Author: Jehu Shaw
 * 
 * Created on 2020_5_12, 16:00
 */

#ifndef IPLAYERUNIT_H
#define	IPLAYERUNIT_H

namespace game {
	class LoginResponse;
}

class IPlayerUnit
{
public:
	virtual ~IPlayerUnit() {}
	// Called when the character login
	virtual void OnLogin() = 0;
	// Called when the character logout
	virtual void OnLogout() = 0;
	// Called when the character login, then response
	virtual void OnInitClient(game::LoginResponse& outResponse) = 0;
};

#endif /* _IPLAYERUNIT_H_ */
