/* 
 * File:   IScirptEngine.h
 * Author: denghp
 *
 * Created on 2011年4月19日, 下午5:31
 */

#ifndef ISCRIPTENGINE_H
#define	ISCRIPTENGINE_H
#include "LuaPlus.h"
using namespace LuaPlus;
#include <map>
#include <string>

//这个脚本操作类(所谓为一个lua文件)脚本逻辑入口
class IScriptOperation;
class IScriptEngine
{
public:
    IScriptEngine(){};
    virtual ~IScriptEngine(){};
public:
    virtual void  Init(const char* scriptconfigPath );
////////////////
    virtual IScriptOperation*  LoadScript(int ScriptId);
    //得到脚本
    virtual IScriptOperation*  LoadScript(char* szScriptName);
    //注销脚本
    void  UnloadScirpt(IScriptOperation* pOperation);
protected:
    //游戏所有的逻辑脚本总集
    typedef std::map<unsigned long,IScriptOperation*>  _MapScript;
    //
    typedef std::map<int,std::string>  _mapResScript;
    ///
    _MapScript   m_script;

    _mapResScript m_ResScript;   
};



#endif	/* ISCIRPTENGINE_H */

