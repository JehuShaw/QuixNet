/* 
 * File:   IRankData.h
 * Author: Jehu Shaw
 *
 * Created on 2016_5_13, 17:38
 */

#ifndef IRANKDATA_H
#define IRANKDATA_H

namespace util {

template <class ScoreType>
class IRankData
{
public:
	/** Destructor. */
	virtual ~IRankData() {}
	/** Get the score value */
	virtual ScoreType GetScore() const = 0;
	/** Get the update time value */
	virtual uint32_t GetTime() const = 0;
	/** Get the order count value */
	virtual uint32_t GetCount() const = 0;
	/** Get owner value */
	virtual uint64_t GetOwner() const = 0;
	/** Set the score value */
	virtual void SetScore(ScoreType value) = 0;
	/** Set the update time value */
	virtual void SetTime(uint32_t value) = 0;
	/** Set the order count value */
	virtual void SetCount(uint32_t value) = 0;
	/** Set owner */
	virtual void SetOwner(uint64_t value) = 0;
};

}

#endif /* IRANKDATA_H */