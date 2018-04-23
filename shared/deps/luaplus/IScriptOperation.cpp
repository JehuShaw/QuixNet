/* 
 * File:   IScirptOperation.cpp
 * Author: denghp
 * 
 * Created on 2011年4月20日, 下午1:52
 */

#include "IScriptOperation.h"
#include <unistd.h>
#include "../../../defines.h"
IScriptOperation::IScriptOperation()
{
    m_luaScript = LuaState::Create(true);
}
IScriptOperation::~IScriptOperation()
{
}
bool  IScriptOperation::LoadScript(const char* strScriptDir,const char* strScriptFile)
{
    char buffer[300];
    sprintf(buffer,"%s%s",strScriptDir,strScriptFile);
    int err = access(buffer,R_OK);
    if(err == -1)
        return false;
    
    strcpy(m_strFile,buffer);

    LoadData();
    //加载脚本
    if(!m_luaScript->LoadFile(buffer))
    {
        //加载成功进行编译
        m_luaScript->Call( 0, 0 );

        return true;
    }
    //没有加载成功
    return false;
}
