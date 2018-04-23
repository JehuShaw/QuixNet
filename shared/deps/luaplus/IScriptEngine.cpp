
#include <string>

#include "IScriptEngine.h"
#include "GlobalAlgorithm.h"
#include "IScriptOperation.h"
#include "../../../defines.h"

void  IScriptEngine::Init(const char* scriptconfigPath )
{
     LuaStateOwner luaScript;
     char buffer[256];
     sprintf(buffer,"%s",scriptconfigPath);
     if(!luaScript->LoadFile(buffer))
     {
         luaScript->Call( 0, 0 );
     }

     LuaObject objectlist = luaScript->GetGlobal("scriptlist");

    if(objectlist.IsTable())
    {
//        printf("load script config\n");
         for(LuaTableIterator it(objectlist);it ;++it)
         {
             LuaObject gameObj = it.GetValue();
             //if( gameObj.IsTable())
             {
                 LuaObject objectid = gameObj.GetByName("id");
                 if(objectid.IsInteger())
                 {                     
                     int id = objectid.GetInteger();
//                     printf("id = %d\n",id);
                     LuaObject objectname = gameObj.GetByName("filename");

                     if(objectname.IsString())
                     {
                         std::string str;
                         str = objectname.GetString();

                         m_ResScript.insert(_mapResScript::value_type(id,str));
                     }
                 }
             }
        }
    }
}
IScriptOperation*  IScriptEngine::LoadScript(int ScriptId)
{
    _mapResScript::iterator itr = m_ResScript.find(ScriptId);

    if(itr != m_ResScript.end())
    {
        std::string str = itr->second;

        char filename[256];

        strcpy(filename,str.c_str());

        return LoadScript(filename);
    }

    return NULL;
}
//得到脚本
IScriptOperation*  IScriptEngine::LoadScript(char* szScriptName)
{
    if(!szScriptName)
        return NULL;

    //通过文件名hash一个关键字
    unsigned long key =  hash_PHP_String(szScriptName,strlen(szScriptName));

    _MapScript::iterator itr = m_script.find(key);
    //寻找原来是否已经加载过
    if(itr != m_script.end())
    {
        return itr->second;
    }
    //创建新的
    IScriptOperation* pOperation = new IScriptOperation;

    if(NULL == pOperation)
        return false;

    if(pOperation->LoadScript(SCRIPTDIR,szScriptName))
    {
        m_script.insert(_MapScript::value_type(key,pOperation));
        
        return pOperation;
    }
    delete pOperation;
    return NULL;
};
//注销脚本
void  IScriptEngine::UnloadScirpt(IScriptOperation* pOperation)
{
    unsigned long key =  hash_PHP_String(pOperation->GetName(),strlen(pOperation->GetName()));

    _MapScript::iterator itr = m_script.find(key);
    //寻找原来是否已经加载过
    if(itr != m_script.end())
    {
        m_script.erase(itr);
    }
    delete pOperation;
    pOperation = NULL;
};
