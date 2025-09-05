/* 
 * File:   AreaOfInterest2D.h
 * Author: Jehu Shaw
 * 
 * Created on 2022_1_19, 20:00
 */

#ifndef AREAOFINTEREST2D_H
#define	AREAOFINTEREST2D_H

#include <stdint.h>
#include <float.h>
#include <math.h>
#include <set>
#include <vector>
#include <memory>
#include <functional>

namespace util
{
	template<typename coordinateType, typename valueType, valueType minValue>
	class AoiNode2D {
	public:
		AoiNode2D() : m_value(minValue), m_x(0), m_y(0), m_angle(CalcuAngle(m_x, m_y)) {}

		AoiNode2D(coordinateType x, coordinateType y, const valueType& v)
			: m_value(v), m_x(x), m_y(y), m_angle(CalcuAngle(m_x, m_y)) {}

		virtual ~AoiNode2D() {}

		inline valueType GetValue() const { return m_value; }

		inline coordinateType GetX() const { return m_x; }

		inline coordinateType GetY() const { return m_y; }

		inline void SetValue(const valueType& v) { m_value = v; }

		inline void SetX(const coordinateType x)
		{
			m_x = x;
			m_angle = CalcuAngle(m_x, m_y);
		}

		inline void SetY(const coordinateType y)
		{
			m_y = y;
			m_angle = CalcuAngle(m_x, m_y);
		}

	private:
		template<typename coordinateT, typename valueT, valueT minValue>
		friend class AreaOfInterest2D;

		template<typename coordinateT, typename valueT, valueT minValue>
		friend struct AoiNode2DCompare;

		inline static double CalcuAngle(coordinateType x, coordinateType y)
		{
			return atan2(y, x);
		}

	private:
		valueType m_value;
		coordinateType m_x;
		coordinateType m_y;
		double m_angle;
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	struct AoiNode2DCompare {
		typedef AoiNode2D<coordinateType, valueType, minValue> nodeType;
		typedef std::shared_ptr<nodeType> nodePtrType;

		bool operator()(const typename nodePtrType& pObject1, const typename nodePtrType& pObject2) const
		{
			if (fabs(pObject1->m_angle - pObject2->m_angle) < DBL_EPSILON) {
				return pObject1->m_value < pObject2->m_value;
			}
			else if (pObject1->m_angle < pObject2->m_angle) {
				return true;
			}
			return false;
		}
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	class AoiCircle2D {
	public:
		typedef AoiNode2D<coordinateType, valueType, minValue> nodeType;
		typedef std::shared_ptr<nodeType> nodePtrType;
		typedef std::multiset<nodePtrType, AoiNode2DCompare<coordinateType, valueType, minValue> > nodeSetType;
		typedef std::shared_ptr<nodeSetType> nodeSetPtrType;

		AoiCircle2D(uint32_t radius) : m_radius(radius) {}

		AoiCircle2D(coordinateType x, coordinateType y) : m_radius(CalcuRadius(x, y)) {}

		void Insert(const nodePtrType &v)
		{
			if (m_nodeSet == nullptr) {
				m_nodeSet.reset(new nodeSetType);
			}
			m_nodeSet->insert(v);
		}

		bool Erase(const nodePtrType &v)
		{
			if (m_nodeSet == nullptr) {
				return false;
			}
			nodeSetType::const_iterator it(m_nodeSet->find(v));
			if (it == m_nodeSet->end()) {
				return false;
			}
			m_nodeSet->erase(it);
			return true;
		}

		inline bool Empty() const
		{
			if (m_nodeSet == nullptr) {
				return true;
			}
			return m_nodeSet->empty();
		}

		bool operator>(const AoiCircle2D& right) const
		{
			return m_radius > right.m_radius;
		}

		bool operator<(const AoiCircle2D& right) const
		{
			return m_radius < right.m_radius;
		}

		bool operator<=(const AoiCircle2D& right) const
		{
			return m_radius <= right.m_radius;
		}

		bool operator>=(const AoiCircle2D& right) const
		{
			return m_radius >= right.m_radius;
		}

		bool operator==(const AoiCircle2D& right) const
		{
			return m_radius == right.m_radius;
		}

		bool operator!=(const AoiCircle2D& right) const
		{
			return m_radius != right.m_radius;
		}

	private:
		inline static uint32_t CalcuRadius(coordinateType x, coordinateType y)
		{
			return static_cast<uint32_t>(ceil(sqrt(x*x + y * y)));
		}

	private:
		template<typename coordinateT, typename valueT, valueT minValue>
		friend class AreaOfInterest2D;

		nodeSetPtrType m_nodeSet;
		uint32_t m_radius;
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	class AreaOfInterest2D {
		typedef AoiCircle2D<coordinateType, valueType, minValue> circleType;
		typedef std::set<circleType> circleSetType;
	public:
		typedef typename circleType::nodeType nodeType;
		typedef typename circleType::nodePtrType nodePtrType;
		typedef std::unique_ptr<nodeType> addPtrType;
		typedef std::vector<nodePtrType> resultSetType;
		typedef std::function<bool(const nodePtrType &v)> queryFilter;

		AreaOfInterest2D() : m_circleSet(), m_size(0) {}

		void Insert(addPtrType &v)
		{
			if (v == nullptr) {
				return;
			}
			circleType circle(v->m_x, v->m_y);
			circleSetType::iterator it(m_circleSet.lower_bound(circle));
			if (it == m_circleSet.end() || (*it) != circle) {
				it = m_circleSet.insert(it, circle);
			}
			nodePtrType temp(v.release());
			const_cast<circleType&>(*it).Insert(temp);
			++m_size;
		}

		void Query(resultSetType& outResult,
			coordinateType centerX,
			coordinateType centerY,
			uint32_t radius,
			queryFilter filter = nullptr)
		{
			uint32_t centerRadius = circleType::CalcuRadius(centerX, centerY);
			int32_t minRadius = centerRadius - radius;
			if (minRadius < 0) {
				minRadius = 0;
			}
			uint32_t maxRadius = centerRadius + radius;

			typename circleSetType::const_iterator itL(m_circleSet.lower_bound(circleType(minRadius)));
			if (itL == m_circleSet.end()) {
				return;
			}
			typename circleSetType::const_iterator itR(m_circleSet.upper_bound(circleType(maxRadius)));

			nodePtrType findNode(new nodeType);
			findNode->m_value = minValue;
			findNode->m_angle = nodeType::CalcuAngle(centerX, centerY);

			queryMethodType queryMethod = filter == nullptr ? &AreaOfInterest2D::QueryRing : &AreaOfInterest2D::QueryFilter;

			for (; itL != itR; ++itL) {
				if (itL->Empty()) {
					continue;
				}
				(this->*queryMethod)(outResult, itL->m_nodeSet,
					findNode, centerX, centerY, radius, filter);
			}
		}

		bool Remove(const nodePtrType& v)
		{
			if (v == nullptr) {
				return false;
			}
			typename circleSetType::iterator it(m_circleSet.find(circleType(v->m_x, v->m_y)));
			if (m_circleSet.end() == it) {
				return false;
			}
			if (!const_cast<circleType&>(*it).Erase(v)) {
				return false;
			}
			if (it->Empty()) {
				m_circleSet.erase(it);
			}
			--m_size;
			return true;
		}

		size_t Size() const
		{
			return m_size;
		}

		void Clear()
		{
			m_circleSet.clear();
			m_size = 0;
		}

	private:
		typedef void (AreaOfInterest2D::*queryMethodType)(
			resultSetType& outResult,
			const typename circleType::nodeSetPtrType& nodeSetPtr,
			const nodePtrType& findNode,
			coordinateType centerX,
			coordinateType centerY,
			uint32_t radius,
			queryFilter filter);

		inline static uint32_t CalcuSquared(coordinateType x, coordinateType y)
		{
			return static_cast<uint32_t>(ceil(x*x + y * y));
		}

		void QueryRing(resultSetType& outResult,
			const typename circleType::nodeSetPtrType& nodeSetPtr,
			const nodePtrType& findNode,
			coordinateType centerX,
			coordinateType centerY,
			uint32_t radius,
			queryFilter filter)
		{
			uint32_t curSquared;
			uint32_t radiusSquared = radius * radius;
			uint32_t maxSquared = (radius + 1) * (radius + 1);
			typename circleType::nodeSetType::const_iterator itC(nodeSetPtr->lower_bound(findNode));
			typename circleType::nodeSetType::const_iterator itL(itC);
			do {
				if (itL == nodeSetPtr->begin()) {
					itL = nodeSetPtr->end();
					if (itL == itC) {
						return;
					}
				}
				--itL;
				curSquared = CalcuSquared((*itL)->m_x - centerX, (*itL)->m_y - centerY);
				if (curSquared <= radiusSquared) {
					outResult.push_back(*itL);
				}
				else if (curSquared > maxSquared) {
					break;
				}
			} while (itL != itC);

			if (itL == itC) {
				return;
			}

			typename circleType::nodeSetType::const_iterator itR(itC);
			if (itR == nodeSetPtr->end()) {
				itR = nodeSetPtr->begin();
				if (itR == itL) {
					return;
				}
			}
			do {
				curSquared = CalcuSquared((*itR)->m_x - centerX, (*itR)->m_y - centerY);
				if (curSquared <= radiusSquared) {
					outResult.push_back(*itR);
				}
				else if (curSquared > maxSquared) {
					break;
				}
				if (++itR == nodeSetPtr->end()) {
					itR = nodeSetPtr->begin();
				}
			} while (itR != itL);
		}

		void QueryFilter(resultSetType& outResult,
			const typename circleType::nodeSetPtrType& nodeSetPtr,
			const nodePtrType& findNode,
			coordinateType centerX,
			coordinateType centerY,
			uint32_t radius,
			queryFilter filter)
		{
			uint32_t maxSquared = (radius + 1) * (radius + 1);
			typename circleType::nodeSetType::const_iterator itC(nodeSetPtr->lower_bound(findNode));
			typename circleType::nodeSetType::const_iterator itL(itC);
			do {
				if (itL == nodeSetPtr->begin()) {
					itL = nodeSetPtr->end();
					if (itL == itC) {
						return;
					}
				}
				--itL;
				if (filter(*itL)) {
					outResult.push_back(*itL);
				}
				else if (CalcuSquared((*itL)->m_x - centerX, (*itL)->m_y - centerY) > maxSquared) {
					break;
				}
			} while (itL != itC);

			if (itL == itC) {
				return;
			}

			typename circleType::nodeSetType::const_iterator itR(itC);
			if (itR == nodeSetPtr->end()) {
				itR = nodeSetPtr->begin();
				if (itR == itL) {
					return;
				}
			}
			do {
				if (filter(*itR)) {
					outResult.push_back(*itR);
				}
				else if (CalcuSquared((*itR)->m_x - centerX, (*itR)->m_y - centerY) > maxSquared) {
					break;
				}
				if (++itR == nodeSetPtr->end()) {
					itR = nodeSetPtr->begin();
				}
			} while (itR != itL);
		}

	private:
		circleSetType m_circleSet;
		size_t m_size;
	};

}

#endif /* AREAOFINTEREST2D_H */



