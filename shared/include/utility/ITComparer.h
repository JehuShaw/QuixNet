/*
 * File:   ITComparer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_1 10:32
 */

#ifndef ITCOMPARER_H
#define ITCOMPARER_H

namespace util {

template<class T> class CAutoPointer;
class IPQElementBase;

class ITComparer
{
public:
    virtual ~ITComparer() {}
    // Summary:
    //     Compares two objects and returns a value indicating whether one is less than,
    //     equal to, or greater than the other.
    //
    // Parameters:
    //   x:
    //     The first object to compare.
    //
    //   y:
    //     The second object to compare.
    //
    // Returns:
    //     Value Condition Less than zero a is less than b.  Zero a equals b.  Greater
    //     than zero a is greater than b.
    virtual int Compare(const CAutoPointer<IPQElementBase>& a, const CAutoPointer<IPQElementBase>& b) const = 0;

};

}

#endif /* ITCOMPARER_H */
