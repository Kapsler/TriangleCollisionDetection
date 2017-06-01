#include <string>
#include "Triangle.hpp"
#include "StaticXORShift.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include <unordered_map>

namespace config
{
	const size_t width = 1600;
	const size_t height = 900;
	const bool vsync = false;
	const size_t triangleCount = 300;
	const size_t rows = 10;
	const size_t triangleMaxSize = 50;
}

namespace Scene
{
	sf::CircleShape drawCircle(1);
	std::vector<TriangleCollision::CollisiionTriangle> triangles;
	std::vector<sf::Vertex> trianglesToDraw;
	std::vector<sf::Vertex> boxesToDraw;
	std::vector<sf::Vertex> outlinesToDraw;
	size_t triangleDrawIndex = 0;
	size_t boxesDrawIndex = 0;
	size_t outlinsDrawIndex = 0;
	std::unordered_set<size_t> alreadyDrawnTriangles;
	std::unordered_set<size_t> alreadyDrawnAABB;
	std::unordered_set<size_t> alreadyDrawnOOBB;
	std::unordered_set<size_t> alreadyDrawnOutlines;
	std::unordered_map<size_t, TriangleCollision::BoundingCirlce> circlesToDraw;
	
	static void MovePlayer(sf::Event::MouseMoveEvent m)
	{
		TriangleCollision::MoveTriangle(triangles[0], glm::vec2(m.x, m.y));
	}	

	static void Frame()
	{
		triangleDrawIndex = 0;
		boxesDrawIndex = 0;
		outlinsDrawIndex = 0;
		alreadyDrawnTriangles.clear();
		alreadyDrawnAABB.clear();
		alreadyDrawnOOBB.clear();
		alreadyDrawnOutlines.clear();
		circlesToDraw.clear();
	}

	static void Render(sf::RenderWindow& window)
	{
		window.draw(&trianglesToDraw[0], triangleDrawIndex * 3, sf::PrimitiveType::Triangles);

		for(auto i = circlesToDraw.begin(); i != circlesToDraw.end(); ++i)
		{
			drawCircle.setPosition(sf::Vector2f(i->second.center.x, i->second.center.y));
			drawCircle.setScale(i->second.radius, i->second.radius);
			drawCircle.setFillColor(sf::Color::Transparent);

			window.draw(drawCircle);
		}

		window.draw(&boxesToDraw[0], boxesDrawIndex * 4 * 2, sf::PrimitiveType::Lines);
		window.draw(&outlinesToDraw[0], outlinsDrawIndex * 3 * 2, sf::PrimitiveType::Lines);
	}

	void SetupScene()
	{
		size_t dontOmptimizeAway = 0;

		//Warmup RNG
		for(size_t i = 0u; i < 1000000; ++i)
		{
			dontOmptimizeAway = StaticXorShift::GetNumber();
		}
		printf("Warmup: %zu", dontOmptimizeAway);

		//Setup Rendering Stuff
		drawCircle.setOrigin(drawCircle.getRadius(), drawCircle.getRadius());
		drawCircle.setFillColor(sf::Color::Transparent);
		drawCircle.setOutlineColor(sf::Color::Yellow);
		drawCircle.setOutlineThickness(1.0f);
		trianglesToDraw.resize(config::triangleCount * 3);
		boxesToDraw.resize(config::triangleCount * 4 * 2 * 2);
		outlinesToDraw.resize(config::triangleCount * 3 * 2);
		alreadyDrawnTriangles.reserve(config::triangleCount);
		alreadyDrawnAABB.reserve(config::triangleCount);
		alreadyDrawnOOBB.reserve(config::triangleCount);
		circlesToDraw.reserve(config::triangleCount);
		drawCircle.setOutlineThickness(0.02f);

		//Generating Geometry
		triangles.reserve(config::triangleCount);

		int trianglesPerRow = (int)glm::ceil((float)config::triangleCount / (float)(config::rows));

		float xOffset = (float)config::width / (float)(trianglesPerRow + 2);
		float yOffset = (float)config::height / ((float)config::rows + 2);
		
		glm::vec2 triPos(xOffset, yOffset);
		size_t index = 0;

		for(size_t row = 0u; row < config::rows; ++row)
		{
			for (size_t i = 0u; i < trianglesPerRow; ++i)
			{
				TriangleCollision::CollisiionTriangle ct;
				ct.id = index;
				++index;

				//Generate Triangle
				TriangleCollision::Triangle tri;

				tri.position = triPos;
				tri.points[0] = glm::vec2(
					tri.position.x + (StaticXorShift::GetZeroToOne() * config::triangleMaxSize) - (StaticXorShift::GetZeroToOne() * config::triangleMaxSize),
					tri.position.y - (StaticXorShift::GetZeroToOne() * config::triangleMaxSize));

				tri.points[1] = glm::vec2(
					tri.position.x + (StaticXorShift::GetZeroToOne() * config::triangleMaxSize),
					tri.position.y + (StaticXorShift::GetZeroToOne() * config::triangleMaxSize));

				tri.points[2] = glm::vec2(
					tri.position.x - (StaticXorShift::GetZeroToOne() * config::triangleMaxSize),
					tri.position.y + (StaticXorShift::GetZeroToOne() * config::triangleMaxSize));
				ct.triangle = tri;

				//AABB
				TriangleCollision::BoundingBox aaBoundingBox;
				aaBoundingBox = TriangleCollision::GenerateAABB(tri);
				ct.aabb = aaBoundingBox;

				//Bounding Circle
				TriangleCollision::BoundingCirlce boundingCircle;
				boundingCircle.center = TriangleCollision::CalculateCentroid(tri);
				boundingCircle.radius = TriangleCollision::GetBoundingCircleRadius(tri, boundingCircle.center);
				ct.boundingCircle = boundingCircle;

				//OOBB
				TriangleCollision::BoundingBox obb;
				TriangleCollision::OOBB oobbStruct;

				glm::vec2 longestAxisPoints[2];
				TriangleCollision::GetLongestAxis(tri, longestAxisPoints);
				glm::vec2 longestAxisVector = longestAxisPoints[1] - longestAxisPoints[0];
				longestAxisVector = glm::normalize(longestAxisVector);

				float rotation = glm::acos(glm::dot(glm::vec2(1.0f, 0.0f), longestAxisVector));
				printf("Rotation: %f\n\r", rotation);
				TriangleCollision::RotateAroundPointRads(tri.points, 3, longestAxisPoints[0], -rotation);

				obb = TriangleCollision::GenerateAABB(tri);

				TriangleCollision::RotateAroundPointRads(tri.points, 3, longestAxisPoints[0], rotation);
				TriangleCollision::RotateAroundPointRads(obb.points, 4, longestAxisPoints[0], rotation);

				oobbStruct.box = obb;
				oobbStruct.u[0] = glm::normalize(obb.points[3] - obb.points[0]);
				oobbStruct.u[1] = glm::normalize(obb.points[1] - obb.points[0]);

				ct.oobb = oobbStruct;
				triangles.push_back(ct);

				triPos.x += xOffset;
			}
			triPos.x = xOffset;
			triPos.y += yOffset;

		}
		
	}

	static void DrawTriangle(const TriangleCollision::Triangle& triangle, sf::Color color)
	{
		trianglesToDraw[(triangleDrawIndex * 3) + 0].position.x = triangle.points[0].x;
		trianglesToDraw[(triangleDrawIndex * 3) + 0].position.y = triangle.points[0].y;
		trianglesToDraw[(triangleDrawIndex * 3) + 0].color = color;

		trianglesToDraw[(triangleDrawIndex * 3) + 1].position.x = triangle.points[1].x;
		trianglesToDraw[(triangleDrawIndex * 3) + 1].position.y = triangle.points[1].y;
		trianglesToDraw[(triangleDrawIndex * 3) + 1].color = color;

		trianglesToDraw[(triangleDrawIndex * 3) + 2].position.x = triangle.points[2].x;
		trianglesToDraw[(triangleDrawIndex * 3) + 2].position.y = triangle.points[2].y;
		trianglesToDraw[(triangleDrawIndex * 3) + 2].color = color;

		++triangleDrawIndex;
	}

	static void DrawTriangle(const TriangleCollision::Triangle& triangle, size_t id)
	{
		if (alreadyDrawnTriangles.find(id) == alreadyDrawnTriangles.end())
		{
			DrawTriangle(triangle, sf::Color::Green);
			alreadyDrawnTriangles.insert(alreadyDrawnTriangles.end(), id);
		}
	}

	static void DrawTriangleOutline(const TriangleCollision::Triangle& triangle, sf::Color color)
	{
		//Line1
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 0].position.x = triangle.points[0].x;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 0].position.y = triangle.points[0].y;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 0].color = color;

		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 1].position.x = triangle.points[1].x;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 1].position.y = triangle.points[1].y;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 1].color = color;

		//Line 2
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 2].position.x = triangle.points[1].x;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 2].position.y = triangle.points[1].y;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 2].color = color;

		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 3].position.x = triangle.points[2].x;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 3].position.y = triangle.points[2].y;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 3].color = color;

		//Line 3
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 4].position.x = triangle.points[2].x;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 4].position.y = triangle.points[2].y;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 4].color = color;

		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 5].position.x = triangle.points[0].x;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 5].position.y = triangle.points[0].y;
		outlinesToDraw[(outlinsDrawIndex * 3 * 2) + 5].color = color;

		++outlinsDrawIndex;
	}

	static void DrawTriangleOutline(const TriangleCollision::Triangle& triangle, size_t id)
	{
		if(alreadyDrawnOutlines.find(id) == alreadyDrawnOutlines.end())
		{
			DrawTriangleOutline(triangle, sf::Color::Red);
			alreadyDrawnOutlines.insert(id);
		}
	}

	static void DrawBoundingBox(const TriangleCollision::BoundingBox& aabb, sf::Color color)
	{
		//Line1
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 0].position.x = aabb.points[0].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 0].position.y = aabb.points[0].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 0].color = color;

		boxesToDraw[(boxesDrawIndex * 4 * 2) + 1].position.x = aabb.points[1].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 1].position.y = aabb.points[1].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 1].color = color;

		//Line 2
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 2].position.x = aabb.points[1].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 2].position.y = aabb.points[1].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 2].color = color;

		boxesToDraw[(boxesDrawIndex * 4 * 2) + 3].position.x = aabb.points[2].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 3].position.y = aabb.points[2].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 3].color = color;

		//Line 3
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 4].position.x = aabb.points[2].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 4].position.y = aabb.points[2].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 4].color = color;

		boxesToDraw[(boxesDrawIndex * 4 * 2) + 5].position.x = aabb.points[3].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 5].position.y = aabb.points[3].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 5].color = color;

		//Line 4
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 6].position.x = aabb.points[3].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 6].position.y = aabb.points[3].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 6].color = color;

		boxesToDraw[(boxesDrawIndex * 4 * 2) + 7].position.x = aabb.points[0].x;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 7].position.y = aabb.points[0].y;
		boxesToDraw[(boxesDrawIndex * 4 * 2) + 7].color = color;

		++boxesDrawIndex;
	}

	static void DrawAABB(const TriangleCollision::BoundingBox& aabb, size_t id)
	{
		if(alreadyDrawnAABB.find(id) == alreadyDrawnAABB.end())
		{
			DrawBoundingBox(aabb, sf::Color::Cyan);
			alreadyDrawnAABB.insert(alreadyDrawnAABB.begin(), id);
		}
	}

	static void DrawOOBB(const TriangleCollision::BoundingBox& oobb, size_t id)
	{
		if (alreadyDrawnOOBB.find(id) == alreadyDrawnOOBB.end())
		{
			DrawBoundingBox(oobb, sf::Color::Magenta);
			alreadyDrawnOOBB.insert(alreadyDrawnOOBB.begin(), id);
		}
	}

	static void DrawCircle(const TriangleCollision::BoundingCirlce& circle, size_t id)
	{
		if(circlesToDraw.find(id) == circlesToDraw.end())
		{
			circlesToDraw.insert_or_assign(id, circle);
		}
	}
}


int main()
{
	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 4;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(config::width, config::height), "Collision Detection", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(config::vsync);

	Scene::SetupScene();

	sf::Clock deltaTimer;
	float fpsDelay = 0.0f;

	while (window.isOpen())
	{
		float deltaTime = deltaTimer.restart().asSeconds();
		fpsDelay += deltaTime;

		//FPS Counter
		if (fpsDelay > 0.5f)
		{
			int fps = static_cast<int>(1.0f / deltaTime);
			window.setTitle("Collision Detection FPS: " + std::to_string(fps));
			fpsDelay = 0.0f;
		}

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Key::Escape)
				{
					window.close();
				}
			}
			if (event.type == sf::Event::MouseMoved)
			{
				Scene::MovePlayer(event.mouseMove);
			}
		}

		//Reset Rendering stuff
		Scene::Frame();

		//Check Collisions
		for (size_t i = 0u; i < config::triangleCount; ++i)
		{
			const TriangleCollision::CollisiionTriangle& ct = Scene::triangles[i];

			Scene::DrawTriangle(ct.triangle, ct.id);

			for (size_t j = i + 1; j < config::triangleCount; ++j)
			{
				const TriangleCollision::CollisiionTriangle& other = Scene::triangles[j];

				//Check Bounding Circle
				if (!TriangleCollision::DoesBoundingCircleCollide(ct.boundingCircle, other.boundingCircle))
				{
					continue;
				}

				Scene::DrawCircle(ct.boundingCircle, ct.id);
				Scene::DrawCircle(other.boundingCircle, other.id);

				//Check AABB
				if (!TriangleCollision::DoesAABBCollide(ct.aabb, other.aabb))
				{
					continue;
				}

				Scene::DrawAABB(ct.aabb, ct.id);
				Scene::DrawAABB(other.aabb, ct.id);

				//Check OOBB
				if (!TriangleCollision::DoesOOBBCollide(ct.oobb, other.oobb))
				{
					continue;
				}

				Scene::DrawOOBB(ct.oobb.box, ct.id);
				Scene::DrawOOBB(other.oobb.box, other.id);

				//Check GJK
				if (!TriangleCollision::DoesGJKCollide(ct.triangle.points, other.triangle.points, 3, 3))
				{
					continue;
				}

				Scene::DrawTriangleOutline(ct.triangle, ct.id);
				Scene::DrawTriangleOutline(other.triangle, other.id);
			}

		}

		//Rendering
		window.clear();		

		Scene::Render(window);

		window.display();
	}

	return 0;
}