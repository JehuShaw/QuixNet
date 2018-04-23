/* 
 * File:   IObject.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __IOBJECT_H__
#define __IOBJECT_H__

template <class T>
class IObject
{
public:
	virtual ~IObject(void) {}

    virtual T GetID() const = 0;
};

#endif /* __IOBJECT_H__ */
