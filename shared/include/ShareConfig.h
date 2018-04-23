/*
 * File:   ShareConfig.h
 * Author: Jehu Shaw
 *
 */

// Configuration Header File

#ifndef _SHARECONFIG_H
#define _SHARECONFIG_H


/**
 * DATABASE LAYER SET UP
 */
#if !defined(NO_DBLAYER_MYSQL)
#define ENABLE_DATABASE_MYSQL 1
#endif

/**
 * IF THIS HOST IS BIG ENDIEAN
 */
//#define BIG_ENDIEAN_HOST

#endif  /*_SHARECONFIG_H*/

