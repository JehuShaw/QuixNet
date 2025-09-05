/* 
 * File:   AreaOfInterest3D.h
 * Author: Jehu Shaw
 * 
 * Created on 2022_1_19, 20:00
 */

#ifndef AREAOFINTEREST3D_H
#define	AREAOFINTEREST3D_H

#include <stdint.h>
#include <float.h>
#include <math.h>
#include <set>
#include <vector>
#include <memory>
#include <functional>

namespace util
{

#define DOUBLE_PI 6.28318530717958647692

	template<typename coordinateType>
	class AoiPoint3D {
	public:
		AoiPoint3D(coordinateType x, coordinateType y, coordinateType z)
			: m_x(x), m_y(y), m_z(z) {}

		inline coordinateType GetX() const { return m_x; }
		inline coordinateType GetY() const { return m_y; }
		inline coordinateType GetZ() const { return m_z; }

	private:
		coordinateType m_x;
		coordinateType m_y;
		coordinateType m_z;
	};

	class AoiKey {
	public:
		AoiKey(double v) : m_value(v) {}

		bool operator>(const AoiKey& right) const
		{
			if (fabs(m_value - right.m_value) < DBL_EPSILON) {
				return false;
			}
			return m_value > right.m_value;
		}

		bool operator<(const AoiKey& right) const
		{
			if (fabs(m_value - right.m_value) < DBL_EPSILON) {
				return false;
			}
			return m_value < right.m_value;
		}

		bool operator<=(const AoiKey& right) const
		{
			if (fabs(m_value - right.m_value) < DBL_EPSILON) {
				return true;
			}
			return m_value < right.m_value;
		}

		bool operator>=(const AoiKey& right) const
		{
			if (fabs(m_value - right.m_value) < DBL_EPSILON) {
				return true;
			}
			return m_value > right.m_value;
		}

		bool operator==(const AoiKey& right) const
		{
			return fabs(m_value - right.m_value) < DBL_EPSILON;
		}

		bool operator!=(const AoiKey& right) const
		{
			return fabs(m_value - right.m_value) >= DBL_EPSILON;
		}

	private:
		double m_value;
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	class AoiNode3D {
	public:
		AoiNode3D() : m_value(minValue), m_x(0), m_y(0), m_z(0), m_angleY(CalcuAngleY(m_x, m_y)), m_angleZ(static_cast<int32_t>(ceil(CalcuAngleZ(m_x, m_z)))) {}

		AoiNode3D(coordinateType x, coordinateType y, coordinateType z, const valueType& v)
			: m_value(v), m_x(x), m_y(y), m_z(z), m_angleY(CalcuAngleY(m_x, m_y)), m_angleZ(static_cast<int32_t>(ceil(CalcuAngleZ(m_x, m_z)))) {}

		virtual ~AoiNode3D() {}

		inline valueType GetValue() const { return m_value; }

		inline coordinateType GetX() const { return m_x; }

		inline coordinateType GetY() const { return m_y; }

		inline coordinateType GetZ() const { return m_z; }

		inline void SetValue(const valueType& v) { m_value = v; }

		inline void SetX(const coordinateType x)
		{
			m_x = x;
			m_angleY = CalcuAngleY(m_x, m_y);
			m_angleZ = static<int32_t>(ceil(CalcuAngleZ(m_x, m_z)));
		}

		inline void SetY(const coordinateType y)
		{
			m_y = y;
			m_angleY = CalcuAngleY(m_x, m_y);
		}

		inline void SetZ(const coordinateType z)
		{
			m_z = z;
			m_angleZ = static<int32_t>(ceil(CalcuAngleZ(m_x, m_z)));
		}

	private:
		template<typename coordinateT, typename valueT, valueT minValue>
		friend class AreaOfInterest3D;

		template<typename coordinateT, typename valueT, valueT minValue>
		friend struct AoiNode3DCompare;

		inline static double CalcuAngleY(coordinateType x, coordinateType y)
		{
			return atan2(y, x);
		}
		inline static double CalcuAngleZ(coordinateType x, coordinateType z)
		{
			return atan2(z, x);
		}

	private:
		valueType m_value;
		coordinateType m_x;
		coordinateType m_y;
		coordinateType m_z;
		double m_angleY;
		int32_t m_angleZ;
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	struct AoiNode3DCompare {
		typedef AoiNode3D<coordinateType, valueType, minValue> nodeType;
		typedef std::shared_ptr<nodeType> nodePtrType;

		bool operator()(const typename nodePtrType& pObject1, const typename nodePtrType& pObject2) const
		{
			if (fabs(pObject1->m_angleY - pObject2->m_angleY) < DBL_EPSILON) {
				return pObject1->m_value < pObject2->m_value;
			}
			else if (pObject1->m_angleY < pObject2->m_angleY) {
				return true;
			}
			return false;
		}
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	class AoiCircle3D {
	public:
		typedef AoiNode3D<coordinateType, valueType, minValue> nodeType;
		typedef std::shared_ptr<nodeType> nodePtrType;
		typedef std::multiset<nodePtrType, AoiNode3DCompare<coordinateType, valueType, minValue> > nodeSetType;
		typedef std::shared_ptr<nodeSetType> nodeSetPtrType;

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

	private:
		template<typename coordinateT, typename valueT, valueT minValue>
		friend class AreaOfInterest3D;

		nodeSetPtrType m_nodeSet;
	};

	template<typename coordinateType, typename valueType, valueType minValue>
	class AreaOfInterest3D {
		typedef AoiCircle3D<coordinateType, valueType, minValue> circleType;
		typedef std::map<uint32_t, circleType> radiusMapType;
		typedef std::map<AoiKey, radiusMapType> angleZMapType;

	public:
		typedef typename circleType::nodeType nodeType;
		typedef typename circleType::nodePtrType nodePtrType;
		typedef std::unique_ptr<nodeType> addPtrType;
		typedef std::vector<nodePtrType> resultSetType;
		typedef std::function<bool(const nodePtrType &v)> queryFilter;
		typedef AoiPoint3D<coordinateType> pointType;

		AreaOfInterest3D() : m_angleZMap(), m_size(0) {}

		void Insert(addPtrType &v)
		{
			if (v == nullptr) {
				return;
			}
			double angleZ = nodeType::CalcuAngleZ(v->m_x, v->m_z);
			radiusMapType& radiusMap = m_angleZMap[AoiKey(angleZ)];
			uint32_t radius = CalcuRadius(v->m_x, v->m_y, v->m_z);
			radiusMapType::iterator it(radiusMap.lower_bound(radius));
			if (it == radiusMap.end() || it->first != radius) {
				it = radiusMap.insert(it, radiusMapType::value_type(radius, circleType()));
			}
			nodePtrType temp(v.release());
			it->second.Insert(temp);
			++m_size;
		}

		void Query(resultSetType& outResult,
			pointType center,
			uint32_t radius,
			queryFilter filter = nullptr)
		{
			if (m_angleZMap.empty()) {
				return;
			}

			double centerAngleZ = nodeType::CalcuAngleZ(center.GetX(), center.GetZ());
			uint32_t centerRadius = CalcuRadius(center.GetX(), center.GetY(), center.GetZ());
			double angleZOffset = nodeType::CalcuAngleZ(centerRadius, radius);
			double minAngleZ = centerAngleZ - angleZOffset;
			if (minAngleZ < 0) {
				minAngleZ += DOUBLE_PI;
			}
			double maxAngleZ = centerAngleZ + angleZOffset;
			if (maxAngleZ > DOUBLE_PI) {
				maxAngleZ -= DOUBLE_PI;
			}

			// begin
			typename angleZMapType::const_iterator itB(m_angleZMap.lower_bound(AoiKey(minAngleZ)));
			if (m_angleZMap.end() == itB) {
				itB = m_angleZMap.begin();
			}
			// end
			typename angleZMapType::const_iterator itE(m_angleZMap.upper_bound(AoiKey(maxAngleZ)));

			nodePtrType findNode(new nodeType);
			findNode->m_value = minValue;
			findNode->m_angleY = nodeType::CalcuAngleY(center.GetX(), center.GetY());

			queryMethodType queryMethod = filter == nullptr ? &AreaOfInterest3D::QueryRing : &AreaOfInterest3D::QueryFilter;

			do {
				QueryRadius(outResult, itB->second, findNode, center, centerRadius, radius, queryMethod, filter);
				if (++itB == m_angleZMap.end()) {
					if (itB == itE) { break; }
					itB = m_angleZMap.begin();
				}
			} while (itB != itE);
		}

		bool Remove(const nodePtrType& v)
		{
			if (v == nullptr) {
				return false;
			}
			double angleZ = nodeType::CalcuAngleZ(v->m_x, v->m_z);
			typename angleZMapType::iterator itA(m_angleZMap.find(AoiKey(angleZ)));
			if (m_angleZMap.end() == itA) {
				return false;
			}
			radiusMapType& radiusMap = itA->second;
			uint32_t radius = CalcuRadius(v->m_x, v->m_y, v->m_z);
			typename radiusMapType::iterator itR(radiusMap.find(radius));
			if (radiusMap.end() == itR) {
				return false;
			}
			if (!itR->second.Erase(v)) {
				return false;
			}
			if (itR->second.Empty()) {
				radiusMap.erase(itR);
				if (radiusMap.empty()) {
					m_angleZMap.erase(itA);
				}
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
			m_angleZMap.clear();
			m_size = 0;
		}

	private:
		typedef void (AreaOfInterest3D::*queryMethodType)(
			resultSetType& outResult,
			const typename circleType::nodeSetPtrType& nodeSetPtr,
			const nodePtrType& findNode,
			const pointType& center,
			uint32_t radius,
			queryFilter filter);

		inline static uint32_t CalcuRadius(coordinateType x, coordinateType y, coordinateType z)
		{
			return static_cast<uint32_t>(ceil(sqrt(x*x + y * y + z * z)));
		}

		void QueryRadius(resultSetType& outResult,
			const radiusMapType& radiusMap,
			const nodePtrType& findNode,
			const pointType& center,
			uint32_t centerRadius,
			uint32_t radius,
			queryMethodType queryRing,
			queryFilter filter = nullptr)
		{
			int32_t minRadius = centerRadius - radius;
			if (minRadius < 0) {
				minRadius = 0;
			}
			uint32_t maxRadius = centerRadius + radius;

			typename radiusMapType::const_iterator itL(radiusMap.lower_bound(minRadius));
			if (radiusMap.end() == itL) {
				return;
			}
			typename radiusMapType::const_iterator itR(radiusMap.upper_bound(maxRadius));
			for (; itL != itR; ++itL) {
				if (itL->second.Empty()) {
					continue;
				}
				(this->*queryRing)(outResult, itL->second.m_nodeSet, findNode, center, radius, filter);
			}
		}

		void QueryRing(resultSetType& outResult,
			const typename circleType::nodeSetPtrType& nodeSetPtr,
			const nodePtrType& findNode,
			const pointType& center,
			uint32_t radius,
			queryFilter filter)
		{
			uint32_t curRadius;
			uint32_t maxRadius = radius + 1;
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
				curRadius = CalcuRadius(
					(*itL)->m_x - center.GetX(),
					(*itL)->m_y - center.GetY(),
					(*itL)->m_z - center.GetZ());
				if (curRadius <= radius) {
					outResult.push_back(*itL);
				}
				else if (curRadius > maxRadius) {
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
				curRadius = CalcuRadius(
					(*itR)->m_x - center.GetX(),
					(*itR)->m_y - center.GetY(),
					(*itR)->m_z - center.GetZ());
				if (curRadius <= radius) {
					outResult.push_back(*itR);
				}
				else if (curRadius > maxRadius) {
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
			const pointType& center,
			uint32_t radius,
			queryFilter filter)
		{
			uint32_t maxRadius = radius + 1;
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
				else if (CalcuRadius(
					(*itL)->m_x - center.GetX(),
					(*itL)->m_y - center.GetY(),
					(*itL)->m_z - center.GetZ()) > maxRadius) {
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
				else if (CalcuRadius(
					(*itR)->m_x - center.GetX(),
					(*itR)->m_y - center.GetY(),
					(*itR)->m_z - center.GetZ()) > maxRadius) {
					break;
				}
				if (++itR == nodeSetPtr->end()) {
					itR = nodeSetPtr->begin();
				}
			} while (itR != itL);
		}

	private:
		angleZMapType m_angleZMap;
		size_t m_size;
	};
}

#endif /* AREAOFINTEREST3D_H */



