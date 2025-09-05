/* 
 * File:   IObject.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef IOBJECT_H
#define IOBJECT_H

namespace util {

	template <class T>
	class IObject
	{
	public:
		virtual ~IObject(void) {}

		virtual T GetID() const = 0;
	};

} // namespace util

#endif /* IOBJECT_H */
