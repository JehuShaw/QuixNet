#include "CacheMySqlAdpt.h"

static const char* s_varType[] = {"tinyint","smallint","mediumint","integer","bigint","int",
	"bit","real","double","float","decimal","numeric","varchar","char","timestamp","datetime",
	"date","time","year","tinyblob","mediumblob","longblob","blob","tinytext","mediumtext",
	"longtext","text","enum","set","varbinary","binary"};

eDBType GetCacheDBTypeFromMySql(const std::string& strType)
{
	for(int i = 0; i < DB_TYPE_SIZE; ++i) {
		if(std::string::npos != strType.find(s_varType[i])) {
			return (eDBType)i;
		}
	}
	return DB_TYPE_SIZE;
}

