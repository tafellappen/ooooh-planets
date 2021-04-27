#pragma once

#include "DXCore.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <wrl/client.h> 
#include <d3d11.h>
class Mesh
{
	//create vertex buffer description
	//create index buffer description
	//
public:
	Mesh(Vertex* vertArray, int vertArrayCt, unsigned int* indexArray, int indexArrayCt, Microsoft::WRL::ComPtr<ID3D11Device> device);
	Mesh(const char* fileName, Microsoft::WRL::ComPtr<ID3D11Device> device);

	//return ComPtrs to vertex and index buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	int GetIndexCount();

private:
	void DefineBuffers(Vertex* vertArray, int vertArrayCt, unsigned int* indexArray, int indexArrayCt, Microsoft::WRL::ComPtr<ID3D11Device> device);

	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);


	int indexBufferCt;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	/*void CreateVertexBufferDesc();
	void CreateIndexBufferDesc();*/
};

