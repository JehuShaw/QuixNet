/* 
 * File:   INotificationBody.h
 * Author: Jehu Shaw
 *
 * Created on 2011年3月14日, 下午3:27
 */

#ifndef NOTIFICATIONBODY_H
#define	NOTIFICATIONBODY_H

namespace mdl 
{

    class IBody {
    public:
        virtual ~IBody(){}

        /** 
         * reset body data;
         */
        virtual void ResetBody() = 0;
    };

}

#endif	/* NOTIFICATIONBODY_H */

