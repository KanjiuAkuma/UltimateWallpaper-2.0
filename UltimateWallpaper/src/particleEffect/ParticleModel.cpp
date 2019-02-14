#include "ParticleModel.h"

#include "renderer/Renderer.h"


ParticleModel::ParticleModel() {
	using namespace Renderer;

	const float sq3 = glm::sqrt(3.f);

	corners[0] = glm::vec2( 0.f	     ,  1.f );    // top
	corners[1] = glm::vec2(-sq3 / 2.f, -0.5f);	// left
	corners[2] = glm::vec2( sq3 / 2.f, -0.5f);	// right

	// vertices layout: x, y, isGlowInner, isGlowOuter, isGlowCenter

	float vertices[] = {
		// corners
		corners[0].x, corners[0].y, 0.f, 0.f, 0.f,	// 0: top
		corners[1].x, corners[1].y, 0.f, 0.f, 0.f,	// 1: left
		corners[2].x, corners[2].y, 0.f, 0.f, 0.f,	// 2: right

		// glow inner corners
		corners[0].x, corners[0].y, 1.f, 0.f, 0.f,	// 3: top
		corners[1].x, corners[1].y, 1.f, 0.f, 0.f,	// 4: left
		corners[2].x, corners[2].y, 1.f, 0.f, 0.f,	// 5: right

		// glow outer corners
		corners[0].x, corners[0].y, 0.f, 1.f, 0.f,	// 6: top
		corners[1].x, corners[1].y, 0.f, 1.f, 0.f,	// 7: left
		corners[2].x, corners[2].y, 0.f, 1.f, 0.f,	// 8: right

		// glow centers ((corner1 + corner2) / 2) + glow / 2)
		-sq3 / 4.f	,  .25f		  , 0.f, 1.f, 1.f,	// 9: glow center left
		 sq3 / 4.f	,  .25f		  , 0.f, 1.f, 1.f,	// 10: glow center right
		 0.f		, -.50f		  , 0.f, 1.f, 1.f,	// 11: glow center bottom

	};

	unsigned int indices[] = {
		/* glow: corners (6)*/
		 7,  4,  9,
		 7, 11,  4,
		11,  8,  5,
		 5, 10,  8,
		 9,  3,  6,
		 3, 10,  6,
		/* glow: centers (3)*/
		 4, 11,  5,
		 3,  5, 10,
		 9,  4,  3,
		/* core */
		 0,  1,  2,
	};

	auto* coreVa = new VertexArray();
	auto* coreVb = new VertexBuffer(vertices, 60 * sizeof(float));
	auto* coreVbl = new VertexBufferLayout();
	coreVbl->push<float>(2);
	coreVbl->push<float>(1);
	coreVbl->push<float>(1);
	coreVbl->push<float>(1);

	coreVa->addBuffer(coreVb, coreVbl);

	auto* coreIb = new IndexBuffer(indices, 30);
	coreMesh = new Mesh(coreVa, coreVb, coreVbl, coreIb);
}

ParticleModel::~ParticleModel() {
	delete coreMesh;
}
