#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

class Entity
{

public:
	Entity(std::shared_ptr<Mesh> meshIn, std::shared_ptr<Material> matIn);
	~Entity();

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();
	std::shared_ptr<Material> GetMaterial();
	// im going with making Entity a dumb container for now

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

