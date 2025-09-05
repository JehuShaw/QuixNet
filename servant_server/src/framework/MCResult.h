/* 
 * File:   MCResult.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef MCRESULT_H
#define MCRESULT_H

enum MCResult { 
	// success: n >= 0
	MCERR_OK        = 0,    //!< Success
	MCERR_NOREPLY   = 1,    //!< Success assumed (no reply requested)
	MCERR_NOTSTORED = 2,    //!< Success but item not stored (see memcached docs)
	MCERR_NOTFOUND  = 4,    //!< Success but item not found (see memcached docs)
	MCERR_EXISTS = 8,		//!< The cas code invalidates
	MCERR_ATTACH_CAS = 16,
	MCERR_ESCAPE_STRING = 32,
};

#endif /* MCRESULT_H */

