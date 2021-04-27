#include "Sky.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

Sky::Sky(const char* meshPath, std::wstring ddsTexturePath, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, std::wstring vertShaderPath, std::wstring pixShaderPath, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	samplerState = sampler;

	skyMesh = std::make_shared<Mesh>(meshPath, device);
	vertShader = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), vertShaderPath.c_str());
	pixShader = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), pixShaderPath.c_str());
	
	CreateDDSTextureFromFile(
		device.Get(),
		//context.Get(),
		ddsTexturePath.c_str(),
		nullptr,
		cubeMapTexSRV.GetAddressOf()
	);

	//rasterizer to reverse the cull mode for skybox
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rastDesc, rasterizerOptions.GetAddressOf());

	//to put the edges of the skybox just within range of the far clip plane
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	device->CreateDepthStencilState(&depthDesc, depthBufferComparisonType.GetAddressOf());


}

Sky::~Sky()
{
}

void Sky::DrawSky(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	//change to sky-specific rasterizer state
	context->RSSetState(rasterizerOptions.Get());
	context->OMSetDepthStencilState(depthBufferComparisonType.Get(), 0);

	//set sky shaders
	vertShader->SetShader();
	pixShader->SetShader();

	//pixShader->CopyAllBufferData();
	

	//give proper data
	vertShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vertShader->CopyAllBufferData();

	pixShader->SetShaderResourceView("textureCube", cubeMapTexSRV.Get());
	pixShader->SetSamplerState("samplerState", samplerState.Get());

	//set mesh buffers and draw
	//skyMesh->SetBuffersAndDraw(context); //not yet an existing method
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, skyMesh->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(skyMesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	// Draw this mesh
	context->DrawIndexed(skyMesh->GetIndexCount(), 0, 0);

	//reset back to default
	context->RSSetState(0); // null or 0 puts back the defaults
	context->OMSetDepthStencilState(0, 0);
}

std::shared_ptr<SimpleVertexShader> Sky::GetVertexShader()
{
	return vertShader;
}

std::shared_ptr<SimplePixelShader> Sky::GetPixelShader()
{
	return pixShader;
}

Microsoft::WRL::ComPtr<ID3D11RasterizerState> Sky::GetRasterizerOptions()
{
	return rasterizerOptions;
}

std::shared_ptr<Mesh> Sky::GetMesh()
{
	return skyMesh;
}
