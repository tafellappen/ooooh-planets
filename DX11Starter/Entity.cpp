#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> meshIn, std::shared_ptr<Material> matIn)
{
	mesh = meshIn;
	transform = Transform();
	material = matIn;
}

Entity::~Entity()
{
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh; //mesh is already a pointer
}

Transform* Entity::GetTransform()
{
	return &transform; //transform is not stored here as a pointer, must send address
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return material;
}
