#include <string>
#include "Triangle.hpp"
#include "StaticXORShift.hpp"
#include <SFML/Graphics.hpp>

namespace config
{
	const size_t width = 1600;
	const size_t height = 900;
	const bool vsync = false;
	const size_t triangleCount = 300;
	const size_t rows = 10;
	const size_t triangleMaxSize = 30;
}

namespace Scene
{
	sf::CircleShape drawCircle(1);
	std::vector<TriangleCollision::CollisiionTriangle> triangles;
	
	void MovePlayer(sf::Event::MouseMoveEvent m)
	{
		TriangleCollision::MoveTriangle(triangles[0], glm::vec2(m.x, m.y));
	}

	void SetupScene()
	{
		size_t dontOmptimizeAway = 0;

		//Warmup RNG
		for(size_t i = 0u; i < 1000000; ++i)
		{
			dontOmptimizeAway = StaticXorShift::GetNumber();
		}
		printf("Warmup: %d", dontOmptimizeAway);


		//Setup Rendering Stuff
		drawCircle.setOrigin(drawCircle.getRadius(), drawCircle.getRadius());
		drawCircle.setFillColor(sf::Color::Transparent);
		drawCircle.setOutlineColor(sf::Color::Yellow);
		drawCircle.setOutlineThickness(1.0f);


		//Generating Geometry
		triangles.reserve(config::triangleCount);

		int trianglesPerRow = glm::ceil((float)config::triangleCount / (float)(config::rows));

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

	static void DrawTriangle(sf::RenderWindow& window, const TriangleCollision::Triangle& triangle)
	{
		sf::VertexArray triArray;
		triArray.setPrimitiveType(sf::PrimitiveType::Triangles);

		triArray.append(sf::Vertex(sf::Vector2f(triangle.points[0].x, triangle.points[0].y), sf::Color::Green));
		triArray.append(sf::Vertex(sf::Vector2f(triangle.points[1].x, triangle.points[1].y), sf::Color::Green));
		triArray.append(sf::Vertex(sf::Vector2f(triangle.points[2].x, triangle.points[2].y), sf::Color::Green));

		window.draw(triArray);
	}

	static void DrawTriangle(sf::RenderWindow& window, const TriangleCollision::Triangle& triangle, sf::Color color)
	{
		sf::VertexArray triArray;
		triArray.setPrimitiveType(sf::PrimitiveType::Triangles);

		triArray.append(sf::Vertex(sf::Vector2f(triangle.points[0].x, triangle.points[0].y), color));
		triArray.append(sf::Vertex(sf::Vector2f(triangle.points[1].x, triangle.points[1].y), color));
		triArray.append(sf::Vertex(sf::Vector2f(triangle.points[2].x, triangle.points[2].y), color));

		window.draw(triArray);
	}

	static void DrawTriangleOutline(sf::RenderWindow& window, const TriangleCollision::Triangle& triangle, sf::Color color)
	{
		sf::VertexArray outlineArray;
		outlineArray.setPrimitiveType(sf::PrimitiveType::LineStrip);

		outlineArray.append(sf::Vertex(sf::Vector2f(triangle.points[0].x, triangle.points[0].y), color));
		outlineArray.append(sf::Vertex(sf::Vector2f(triangle.points[1].x, triangle.points[1].y), color));
		outlineArray.append(sf::Vertex(sf::Vector2f(triangle.points[2].x, triangle.points[2].y), color));
		outlineArray.append(sf::Vertex(sf::Vector2f(triangle.points[0].x, triangle.points[0].y), color));

		window.draw(outlineArray);
	}

	static void DrawBoundingBox(sf::RenderWindow& window, const TriangleCollision::BoundingBox& aabb, sf::Color color)
	{
		sf::VertexArray boxArray;
		boxArray.setPrimitiveType(sf::PrimitiveType::LineStrip);

		boxArray.append(sf::Vertex(sf::Vector2f(aabb.points[0].x, aabb.points[0].y), color));
		boxArray.append(sf::Vertex(sf::Vector2f(aabb.points[1].x, aabb.points[1].y), color));
		boxArray.append(sf::Vertex(sf::Vector2f(aabb.points[2].x, aabb.points[2].y), color));
		boxArray.append(sf::Vertex(sf::Vector2f(aabb.points[3].x, aabb.points[3].y), color));
		boxArray.append(sf::Vertex(sf::Vector2f(aabb.points[0].x, aabb.points[0].y), color));

		window.draw(boxArray);
	}

	static void DrawCircle(sf::RenderWindow& window, const TriangleCollision::BoundingCirlce& circle)
	{
		drawCircle.setPosition(sf::Vector2f(circle.center.x, circle.center.y));
		drawCircle.setScale(circle.radius, circle.radius);
		drawCircle.setOutlineThickness(1.0f / circle.radius);
		drawCircle.setFillColor(sf::Color::Transparent);

		window.draw(drawCircle);
	}
	
	static void DrawPoint(sf::RenderWindow& window, const glm::vec2& p, sf::Color color)
	{
		drawCircle.setPosition(sf::Vector2f(p.x, p.y));
		drawCircle.setScale(2.0f, 2.0f);
		drawCircle.setOutlineThickness(0);
		drawCircle.setFillColor(color);

		window.draw(drawCircle);
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

		window.clear();

		//Check Collisions and Render Tris
		for(size_t i = 0u; i < config::triangleCount; ++i)
		{
			const TriangleCollision::CollisiionTriangle& ct = Scene::triangles[i];

			Scene::DrawTriangle(window, ct.triangle);

			for(size_t j = i + 1; j < config::triangleCount; ++j)
			{
				const TriangleCollision::CollisiionTriangle& other = Scene::triangles[j];
				
				//Check Bounding Circle
				if (!TriangleCollision::DoesBoundingCircleCollide(ct.boundingCircle, other.boundingCircle))
				{
					continue;
				}

				Scene::DrawCircle(window, ct.boundingCircle);
				Scene::DrawCircle(window, other.boundingCircle);

				//Check AABB
				if (!TriangleCollision::DoesAABBCollide(ct.aabb, other.aabb))
				{
					continue;
				}

				Scene::DrawBoundingBox(window, ct.aabb, sf::Color::Cyan);
				Scene::DrawBoundingBox(window, other.aabb, sf::Color::Cyan);

				//Check OOBB
				if (!TriangleCollision::DoesOOBBCollide(ct.oobb, other.oobb))
				{
					continue;
				}

				Scene::DrawBoundingBox(window, ct.oobb.box, sf::Color::Magenta);
				Scene::DrawBoundingBox(window, other.oobb.box, sf::Color::Magenta);

				if (!TriangleCollision::DoesGJKCollide(ct.triangle.points, other.triangle.points, 3, 3))
				{
					continue;
				}

				Scene::DrawTriangleOutline(window, ct.triangle, sf::Color::Red);
				Scene::DrawTriangleOutline(window, other.triangle, sf::Color::Red);
			}		
			
		}		

		window.display();
	}

	return 0;
}