#pragma once
#include <glm/glm.hpp>
#include <queue>

namespace TriangleCollision
{
	struct Triangle
	{
		glm::vec2 points[3];
		glm::vec2 position = glm::vec3();
	};

	struct BoundingBox
	{
		glm::vec2 points[4];
	};

	struct OOBB
	{
		BoundingBox box;
		glm::vec2 u[2]; //X and Y axis
	};

	struct BoundingCirlce
	{
		glm::vec2 center;
		float radius;
	};

	struct CollisiionTriangle
	{
		OOBB oobb;
		Triangle triangle;
		BoundingBox aabb;
		BoundingCirlce boundingCircle;
		size_t id;
	};

	static glm::vec2 GetSmallesPoint(const Triangle& triangle)
	{
		glm::vec2 result;

		result.x = triangle.points[0].x;
		result.y = triangle.points[0].y;

		for (size_t i = 0; i < 3; ++i)
		{
			if (triangle.points[i].x < result.x)
			{
				result.x = triangle.points[i].x;
			}
			if (triangle.points[i].y < result.y)
			{
				result.y = triangle.points[i].y;
			}
		}

		return result;
	}

	static glm::vec2 GetLargestPoint(const Triangle& triangle)
	{
		glm::vec2 result;

		result.x = triangle.points[0].x;
		result.y = triangle.points[0].y;

		for (size_t i = 0; i < 3; ++i)
		{
			if (triangle.points[i].x > result.x)
			{
				result.x = triangle.points[i].x;
			}
			if (triangle.points[i].y > result.y)
			{
				result.y = triangle.points[i].y;
			}
		}

		return result;
	}

	static glm::vec2 CalculateCircumcenter(const Triangle& triangle)
	{
		glm::vec2 result;
		glm::vec2 a = triangle.points[0];
		glm::vec2 b = triangle.points[1];
		glm::vec2 c = triangle.points[2];

		float d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));

		result.x = (1.0f / d) *
			(((a.x * a.x) + (a.y * a.y)) * (b.y - c.y)
				+ ((b.x * b.x) + (b.y * b.y)) * (c.y - a.y)
				+ ((c.x * c.x) + (c.y * c.y)) * (a.y - b.y));

		result.y = (1.0f / d) *
			(((a.x * a.x) + (a.y * a.y)) * (c.x - b.x)
				+ ((b.x * b.x) + (b.y * b.y)) * (a.x - c.x)
				+ ((c.x * c.x) + (c.y * c.y)) * (b.x - a.x));

		return result;
	}

	static glm::vec2 CalculateCentroid(const Triangle& triangle)
	{
		glm::vec2 result;
		glm::vec2 a = triangle.points[0];
		glm::vec2 b = triangle.points[1];
		glm::vec2 c = triangle.points[2];

		result = (a + b + c) / 3.0f;

		return result;
	}

	static glm::vec2 GetPixelSize(const Triangle& triangle)
	{
		glm::vec2 smallest = GetSmallesPoint(triangle);
		glm::vec2 largest = GetLargestPoint(triangle);

		return glm::vec2(largest.x - smallest.x, largest.y - smallest.y);

	}

	static float GetBoundingCircleRadius(const Triangle& triangle, glm::vec2& center)
	{
		float result = 0.0f;

		for(size_t i = 0u; i < 3; ++i)
		{
			float currentLength = glm::length(triangle.points[i] - center);
			if(currentLength > result)
			{
				result = currentLength;
			}
		}

		return result;
	}

	static void GetLongestAxis(const Triangle&triangle, glm::vec2* outAxis)
	{
		float length = 0.0f
		;
		for(size_t i = 1u; i < 4u; ++i)
		{
			float currentLength = glm::length(triangle.points[i] - triangle.points[i - 1]);
			if(currentLength > length)
			{
				length = currentLength;
				outAxis[0] = triangle.points[i - 1];
				outAxis[1] = triangle.points[i];
			}
		}
	}

	static BoundingBox GenerateAABB(const Triangle& triangle)
	{
		BoundingBox aabb;

		glm::vec2 smallest = TriangleCollision::GetSmallesPoint(triangle);
		glm::vec2 largest = TriangleCollision::GetLargestPoint(triangle);

		aabb.points[0] = glm::vec2(smallest.x, smallest.y);
		aabb.points[1] = glm::vec2(largest.x, smallest.y);
		aabb.points[2] = glm::vec2(largest.x, largest.y);
		aabb.points[3] = glm::vec2(smallest.x, largest.y);

		return aabb;
	}

	static void RotateAroundPointDegrees(glm::vec2* points, size_t count, glm::vec2& center, float angle)
	{
		float radians = angle * (3.141592653f / 180.0f);

		for (size_t i = 0u; i < count; ++i)
		{
			float Tx = points[i].x - center.x;
			float Ty = points[i].y - center.y;

			points[i].x = (Tx*cosf(radians)) - (Ty*sinf(radians)) + center.x;
			points[i].y = (Ty*cosf(radians)) + (Tx*sinf(radians)) + center.y;
		}
	}


	static void RotateAroundPointRads(glm::vec2* points, size_t count, glm::vec2& center, float angle)
	{
		float sinA = sinf(angle);
		float cosA = cosf(angle);
		
		for (size_t i = 0u; i < count; ++i)
		{
			float Tx = points[i].x - center.x;
			float Ty = points[i].y - center.y;

			points[i].x = (Tx*cosA) - (Ty*sinA) + center.x;
			points[i].y = (Ty*cosA) + (Tx*sinA) + center.y;
		}
	}

	static bool DoesBoundingCircleCollide(const BoundingCirlce& c1, const BoundingCirlce& c2)
	{
		glm::vec2 distanceVec = c2.center - c1.center;
		float distance = glm::length(distanceVec);

		if (distance <= c1.radius + c2.radius)
		{
			return true;
		}

		return false;
	}

	static bool DoesAABBCollide(const BoundingBox& b1, const BoundingBox& b2)
	{
		glm::vec2 b1Min = b1.points[0];
		glm::vec2 b2Min = b2.points[0];
		glm::vec2 b1Max = b1.points[2];
		glm::vec2 b2Max = b2.points[2];

		if (b1Min.x < b2Max.x &&
			b1Max.x > b2Min.x &&
			b1Min.y < b2Max.y &&
			b1Max.y > b2Min.y)
		{
			return true;
		}

		return false;
	}

	//Returns min as x and max as y
	static glm::vec2 ProjectOnAxis(const glm::vec2* points, size_t pointCount, glm::vec2 axis)
	{
		glm::vec2 result;

		result.x = glm::dot(axis, points[0]);
		result.y = result.x;

		for(size_t i = 1u; i < pointCount; ++i)
		{
			float temp = glm::dot(axis, points[i]);

			if(temp < result.x)
			{
				result.x = temp;
			} else if(temp > result.y)
			{
				result.y = temp;
			}
		}

		return result;
	}

	static bool DoesOOBBCollide(const OOBB& b1, const OOBB& b2)
	{
		glm::vec2 axis[4];

		axis[0] = b1.u[0];
		axis[1] = b1.u[1];
		axis[2] = b2.u[0];
		axis[3] = b2.u[1];

		for (size_t i = 0; i < 4; ++i)
		{
			glm::vec2 proj1 = ProjectOnAxis(b1.box.points, 4, axis[i]);

			glm::vec2 proj2 = ProjectOnAxis(b2.box.points, 4, axis[i]);

			if(proj1.x > proj2.y || proj2.x > proj1.y)
			{
				return false;
			}
		}

		return true;
	}

	static void MoveTriangle(CollisiionTriangle& ct, glm::vec2 newPosition)
	{
		ct.triangle.points[0] = ct.triangle.points[0] - ct.triangle.position + newPosition;
		ct.triangle.points[1] = ct.triangle.points[1] - ct.triangle.position + newPosition;
		ct.triangle.points[2] = ct.triangle.points[2] - ct.triangle.position + newPosition;

		ct.boundingCircle.center = ct.boundingCircle.center - ct.triangle.position + newPosition;

		ct.aabb.points[0] = ct.aabb.points[0] - ct.triangle.position + newPosition;
		ct.aabb.points[1] = ct.aabb.points[1] - ct.triangle.position + newPosition;
		ct.aabb.points[2] = ct.aabb.points[2] - ct.triangle.position + newPosition;
		ct.aabb.points[3] = ct.aabb.points[3] - ct.triangle.position + newPosition;

		ct.oobb.box.points[0] = ct.oobb.box.points[0] - ct.triangle.position + newPosition;
		ct.oobb.box.points[1] = ct.oobb.box.points[1] - ct.triangle.position + newPosition;
		ct.oobb.box.points[2] = ct.oobb.box.points[2] - ct.triangle.position + newPosition;
		ct.oobb.box.points[3] = ct.oobb.box.points[3] - ct.triangle.position + newPosition;

		ct.triangle.position = newPosition;
	}

	//Outputs all points of the sum/difference in the last pointer sumPoints
	//Output array to be the appropriate size of shape1Count * shape2Count points
	static void MinkowskiDifference(const glm::vec2* shape1, const glm::vec2* shape2, size_t shape1Count, size_t shape2Count, glm::vec2* sumPoints)
	{
		size_t sumIndex = 0;

		for(size_t i = 0u; i < shape1Count; ++i)
		{
			for (size_t j = 0u; j < shape2Count; ++j)
			{
				sumPoints[sumIndex] = shape1[i] - shape2[j];
				++sumIndex;
			}
		}
	}

	//Gets farthest point of shape in direction
	static glm::vec2 GetFarthestPoint(const glm::vec2* shape, size_t pointCount, const glm::vec2& direction)
	{
		size_t farthestIndex = 0;
		float farthestDistance = glm::dot(shape[0], direction);
		
		for(size_t i = 1u; i < pointCount; ++i)
		{
			float temp = glm::dot(shape[i], direction);
			if(temp > farthestDistance)
			{
				farthestDistance = temp;
				farthestIndex = i;
			}
		}

		return shape[farthestIndex];
	}

	//Gets one point in Minkowski Difference of shape1 and shape2 (farthest in direction)
	static glm::vec2 SupportSimplex(const glm::vec2* shape1, const glm::vec2* shape2, size_t shape1Count, size_t shape2Count, const glm::vec2& direction)
	{
		return GetFarthestPoint(shape1, shape1Count, direction) - GetFarthestPoint(shape2, shape2Count, -direction);
	}

	static glm::vec2 TripleProduct(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
	{
		return  b * glm::dot(c, a) - a * glm::dot(c, b);
	}

	static bool DoesSimplexContainOrigin(std::vector<glm::vec2>& simplex, glm::vec2& direction)
	{
		glm::vec2 a = simplex.back();
		glm::vec2 ao = -a;

		if(simplex.size() == 3)
		{
			//Triangle Case
			glm::vec2 b = simplex[1];
			glm::vec2 c = simplex[0];

			glm::vec2 ab = b - a;
			glm::vec2 ac = c - a;

			//computer normals
			glm::vec2 abPerp = TripleProduct(ac, ab, ab);
			glm::vec2 acPerp = TripleProduct(ab, ac, ac);

			//PlaneChecks
			if(glm::dot(abPerp, ao) > 0)
			{
				//Erase C
				simplex.erase(simplex.begin());
				direction = abPerp;
			} else if(glm::dot(acPerp, ao) > 0)
			{
				//Erase b
				simplex.erase(simplex.begin() + 1);
				direction = acPerp;
			} else
			{
				return true;
			}
		}
		else
		{
			//Line Case
			glm::vec2 b = simplex.front();
			glm::vec2 ab = b - a;

			glm::vec2 abPerp = TripleProduct(ab, ao, ab);
			direction = abPerp;
		}

		return false;
	}

	static bool DoesGJKCollide(const glm::vec2* shape1, const glm::vec2* shape2, size_t shape1Count, size_t shape2Count)
	{
		std::vector<glm::vec2> simplex;
		simplex.reserve(3);
		glm::vec2 direction(1, -1);

		//Add to simplex
		simplex.push_back(SupportSimplex(shape1, shape2, shape1Count, shape2Count, direction));

		direction = -direction;

		while(true)
		{
			simplex.push_back(SupportSimplex(shape1, shape2, shape1Count, shape2Count, direction));

			if(glm::dot(simplex.back(), direction) <= 0)
			{
				return false;
			}

			if(DoesSimplexContainOrigin(simplex, direction))
			{

				return true;
			}
		}
	}
}

//CrossProduct (Normalvector in 2D)
//	x,y -> -y,x