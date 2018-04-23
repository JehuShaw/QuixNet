/*
 * File:   CreateInterface.cpp
 * Author: Jehu Shaw
 *
 */

#include "DatabaseEnv.h"
#include "Log.h"

#if defined(ENABLE_DATABASE_MYSQL)
#include "MySQLDatabase.h"
#endif

using namespace util;

namespace db {

void Database::CleanupLibs()
{
#if defined(ENABLE_DATABASE_MYSQL)
		mysql_library_end();
#endif
}

CAutoPointer<Database> Database::CreateDatabaseInterface()
{
#if defined(ENABLE_DATABASE_MYSQL)
		return CAutoPointer<Database>(new MySQLDatabase);
#else
		return CAutoPointer<Database>();
#endif
}

} // end namespace db