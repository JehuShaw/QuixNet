/* 
 * File:   IScirptOperation.h
 * Author: denghp
 *
 * Created on 2011年4月20日, 下午1:52
 */

#ifndef ISCRIPTOPERATION_H
#define	ISCRIPTOPERATION_H
#include "LuaPlus.h"
using namespace LuaPlus;
#define   FILENAME_SIZE 256

class IScriptOperation
{
public:
    IScriptOperation();
    virtual ~IScriptOperation();
public:
    virtual  bool  LoadData(){};

    bool           LoadScript(const char* strScriptDir,const char* strScriptFile);

    int   DoFunction(char* strFunction)
    {
        LuaObject object = m_luaScript->GetGlobal(strFunction);
        
        if(object.IsFunction())
        {
            LuaFunction<int> Function(object);
            
            return Function();
        }        
        return -1;
    }
    //
    template<typename T>
    int   DoFunction(char* strFunction,T p)
    {
        LuaObject object = m_luaScript->GetGlobal(strFunction);

        if(object.IsFunction())
        {
            LuaFunction<int> Function(object);

            return Function(p);
        }
        return -1;
    }
    //
    template<typename T,typename T1>
    int   DoFunction(char* strFunction,T p,T1 p1)
    {
        LuaObject object = m_luaScript->GetGlobal(strFunction);

        if(object.IsFunction())
        {
            LuaFunction<int> Function(object);

            return Function(p,p1);
        }
        return -1;
    }
    //
    template<typename T,typename T1,typename T2>
    int   DoFunction(char* strFunction,T p,T1 p1,T2 p2)
    {
         LuaObject object = m_luaScript->GetGlobal(strFunction);

        if(object.IsFunction())
        {
            LuaFunction<int> Function(object);

            return Function(p,p1,p2);
        }
        return -1;
     }
    //
    template<typename T,typename T1,typename T2,typename T3>
    int   DoFunction(char* strFunction,T p,T1 p1,T2 p2,T3 p3)
    {
        LuaObject object = m_luaScript->GetGlobal(strFunction);

        if(object.IsFunction())
        {
            LuaFunction<int> Function(object);

            return Function(p,p1,p2,p3);
        }
        return -1;
    }
    //
    template<typename T,typename T1,typename T2,typename T3,typename T4>
    int   DoFunction(char* strFunction,T p,T1 p1,T2 p2,T3 p3,T4 p4)
    {
        LuaObject object = m_luaScript->GetGlobal(strFunction);

        if(object.IsFunction())
        {
            LuaFunction<int> Function(object);

            return Function(p,p1,p2,p3,p4);
        }
        return -1;
    }    
    
    char*  GetName(){return m_strFile;}
    
    LuaStateOwner&   GetLuaStateOwner(){return m_luaScript;};
protected:
    //脚本驱动
    LuaStateOwner        m_luaScript;
    //
    char                 m_strFile[FILENAME_SIZE];  
};

#endif	/* ISCIRPTOPERATION_H */

