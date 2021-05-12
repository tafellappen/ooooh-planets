#include "Game.h"
#include "Vertex.h"
#include "Mesh.h"
#include "BufferStructs.h"

#include <vector>

//Joseph hong is here 
// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif


	camera = std::make_shared<Camera>(0, 0, -10, (float)width / (float)height);
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game

	//clean up entities
	for (int i = 0; i < entities.size(); i++)
	{
		delete entities[i];
		entities[i] = nullptr;
	}

	////clean up meshes
	//for (int i = 0; i < meshes.size(); i++) 
	//{
	//	delete meshes[i];
	//	meshes[i] = nullptr;
	//}

	////clean up materials
	//for (int i = 0; i < materials.size(); i++)
	//{
	//	delete materials[i];
	//	materials[i] = nullptr;
	//}

	//delete camera;
	//camera = nullptr;

	//delete pixelShader;
	//pixelShader = nullptr;

	//delete vertexShader;
	//vertexShader = nullptr;

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	//loading texture
//HRESULT res = CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo("../../Assets/Textures/foil_albedo.tif").c_str(), nullptr, srvTexture.GetAddressOf());

	//textures for bronze material
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		//GetFullPathTo_Wide(L"../../Assets/Textures/foil_albedo.tif").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/bronze_albedo.png").c_str(),
		nullptr,
		srvTexture1Albedo.GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		//GetFullPathTo_Wide(L"../../Assets/Textures/foil_normal.tif").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/bronze_normals.png").c_str(),
		nullptr,
		srvTexture1Normal.GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/bronze_roughness.png").c_str(),
		nullptr,
		srvTexture1Rough.GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/bronze_metal.png").c_str(),
		nullptr,
		srvTexture1Metal.GetAddressOf()
	);

	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/foil_albedo.tif").c_str(),
		nullptr,
		srvTexture2Albedo.GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/foil_normal.tif").c_str(),
		nullptr,
		srvTexture2Normal.GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/foil_roughness.tif").c_str(),
		nullptr,
		srvTexture2Rough.GetAddressOf()
	);
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/foil_metallic.tif").c_str(),
		nullptr,
		srvTexture2Metal.GetAddressOf()
	);
	


	//this is for that sampler warning
	D3D11_SAMPLER_DESC sampDesc = {};
	//u, v, w here define how to handle things outside the 0-1 uv range
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//how to handle things in between pixels
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, sampler.GetAddressOf());

	////rasterizer to reverse the cull mode for skybox
	//D3D11_RASTERIZER_DESC rastDesc = {};
	//rastDesc.CullMode = D3D11_CULL_FRONT;
	//rastDesc.FillMode = D3D11_FILL_SOLID;
	//rastDesc.DepthClipEnable = true;
	//device->CreateRasterizerState(&rastDesc, skyRasterState.GetAddressOf());

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	ParticleSetup();
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//unsigned int size = sizeof(constantBufferVS);
	//unsigned int size = sizeof(VertexShaderExternalData);
	//size = (size + 15) / 16 * 16;

	////describe the constant buffer
	//D3D11_BUFFER_DESC cbDesc = {}; //sets struct to all zeros
	//cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cbDesc.ByteWidth = size; //must be multiple of 16
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	//device->CreateBuffer(&cbDesc, 0, constantBufferVS.GetAddressOf());

	//set up initial transform
	//transform.SetPosition(0.0f, 0.0f, 0.0f);//uuh can i change this yet?
	//transform.SetScale(0.5f, 0.5f, 0.5f);
	/*transform.SetRotation(0, 0, XM_PIDIV4);*/

	skybox = std::make_shared<Sky>(
		GetFullPathTo("../../Assets/Models/cube.obj").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/SpaceCubeMap.dds").c_str(), 
		sampler,
		GetFullPathTo_Wide(L"SkyboxVS.cso").c_str(),
		GetFullPathTo_Wide(L"SkyboxPS.cso").c_str(),
		device,
		context
	);

	ambientColor = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMFLOAT3 red = XMFLOAT3(1.0f, 0.2f, 0.2f);
	XMFLOAT3 green = XMFLOAT3(0.2f, 0.8f, 0.2f);
	XMFLOAT3 purple = XMFLOAT3(0.5f, 0.0f, 1.0f);
	XMFLOAT3 lightBlue = XMFLOAT3(0.2f, 0.2f, 1.0f);
	XMFLOAT3 white = XMFLOAT3(1.0f, 1.0f, 1.0f);

	lights.push_back({
		0,
		green,
		1.0f,
		XMFLOAT3(1.0f, -1.0f, 0.0f)
		});

	lights.push_back({
		0,
		white,
		1.0f,
		XMFLOAT3(-1.0f, -1.0f, 0.0f)
		});

	//lights.push_back({
	//	0,
	//	purple,
	//	1.0f,
	//	XMFLOAT3(0.0f, 1.0f, 0.0f)
	//});

	//point light
	lights.push_back({
		1,
		lightBlue,
		1.0f,
		XMFLOAT3(-1.0f, -1.0f, -1.0f)
		});


}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	//smart pointers
	vertexShader = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PixelShader.cso").c_str());
	
	vertexShaderNormal = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"NormalMapVS.cso").c_str());
	pixelShaderNormal = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"NormalMapPS.cso").c_str());

	vertexShaderParticle = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"ParticleVS.cso").c_str());
	pixelShaderParticle = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"ParticlePS.cso").c_str());

	//skyVS = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"SkyboxVS.cso").c_str());
	//skyPS = std::make_shared<SimplePixelShader>(device.Get(), context.Get(),  GetFullPathTo_Wide(L"SkyboxPS.cso").c_str());

}



// --------------------------------------------------------
// Creates the geometry we're going to draw - 
// --this is where i drew my shapes
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device));

	//skyMesh = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device);

	//materials.push_back(std::make_shared<Material>(red, pixelShader, vertexShader, srvTexture1Albedo, sampler, srvTexture1Metal, srvTexture1Rough));
	materials.push_back(std::make_shared<Material>(red, pixelShaderNormal, vertexShaderNormal, srvTexture1Albedo, sampler, srvTexture1Normal, srvTexture1Metal, srvTexture1Rough));
	materials.push_back(std::make_shared<Material>(red, pixelShaderNormal, vertexShaderNormal, srvTexture2Albedo, sampler, srvTexture2Normal, srvTexture2Metal, srvTexture2Rough));

	entities.push_back(new Entity(meshes[0], materials[0])); //sphere obj file
	entities.push_back(new Entity(meshes[0], materials[1])); //sphere obj file
	entities.push_back(new Entity(meshes[1], materials[0])); //cube obj file
	entities.push_back(new Entity(meshes[1], materials[1])); //cube obj file
	entities.push_back(new Entity(meshes[2], materials[0])); //helix obj file
	entities.push_back(new Entity(meshes[2], materials[1])); //helix obj file


	//give a starting position so they're not on top of each other
	entities[0]->GetTransform()->SetPosition(-1, 1, 0);
	entities[1]->GetTransform()->SetPosition(1, 1, 0);
	entities[2]->GetTransform()->SetPosition(-1, -1, 0);
	entities[3]->GetTransform()->SetPosition(1, -1, 0);
	entities[4]->GetTransform()->SetPosition(-3, 0, 0);
	entities[5]->GetTransform()->SetPosition(3, 0, 0);
}

void Game::ParticleSetup()
{
	//get the texture
	std::wstring particleTexPath = GetFullPathTo_Wide(L"../../Assets/Textures/particle.jpg").c_str();
	HRESULT r = CreateWICTextureFromFile(
		device.Get(), 
		context.Get(), 
		GetFullPathTo_Wide(L"../../Assets/Textures/particle.jpg").c_str(), 
		0, particleTexture.GetAddressOf());

	//make the depth state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //don't write depth
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, particleDepthState.GetAddressOf());

	//additive blending for particles
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //still respect pix shader output alpha
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, particleBlendState.GetAddressOf());

	//debug rasterizer state for particles
	D3D11_RASTERIZER_DESC rd = {};
	rd.CullMode = D3D11_CULL_BACK; //backface culling
	rd.DepthClipEnable = true;
	rd.FillMode = D3D11_FILL_WIREFRAME;
	device->CreateRasterizerState(&rd, particleDebugRasterState.GetAddressOf());
	
	//setting up the actual particles
	emitter1 = std::make_shared<Emitter>(
		110,							// Max particles
		20,								// Particles per second
		5,								// Particle lifetime
		0.1f,							// Start size
		2.0f,							// End size
		XMFLOAT4(1, 0.1f, 0.1f, 0.7f),	// Start color
		XMFLOAT4(1, 0.6f, 0.1f, 0),		// End color
		XMFLOAT3(-2, 2, 0),				// Start velocity
		XMFLOAT3(0.2f, 0.2f, 0.2f),		// Velocity randomness range
		XMFLOAT3(2, 0, 0),				// Emitter position
		XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
		XMFLOAT4(-2, 2, -2, 2),			// Random rotation ranges (startMin, startMax, endMin, endMax)
		XMFLOAT3(0, -1, 0),				// Constant acceleration
		device,
		vertexShaderParticle,
		pixelShaderParticle,
		particleTexture);
}

//void Game::DrawMesh(Mesh* mesh)
//{
//}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	camera->UpdateProjectionMatrix((float)width / (float)height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->Rotate(0, deltaTime, 0);
	}

	camera->Update(deltaTime, this->hWnd);
	emitter1->Update(deltaTime);
}
//
//void Game::CreateLights()
//{
//
//}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), backgroundColor);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	//----create lights----
	pixelShader->SetData(
		"light1",
		&lights[0],
		sizeof(DirectionalLight)
	);

	pixelShader->SetData(
		"light2",
		&lights[1],
		sizeof(DirectionalLight)
	);

	pixelShader->SetData(
		"light3",
		&lights[2],
		sizeof(DirectionalLight)
	);

	//pixelShader->SetData(
	//	"light4",
	//	&lights[3],
	//	sizeof(DirectionalLight)
	//);

	//----create lights - for normal maps----
	pixelShaderNormal->SetData(
		"light1",
		&lights[0],
		sizeof(DirectionalLight)
	);

	pixelShaderNormal->SetData(
		"light2",
		&lights[1],
		sizeof(DirectionalLight)
	);

	pixelShaderNormal->SetData(
		"light3",
		&lights[2],
		sizeof(DirectionalLight)
	);

	for (int i = 0; i < entities.size(); i++)
	{
		bool isNormalMapped = entities[i]->GetMaterial()->GetNormalSRV() == nullptr || entities[i]->GetMaterial()->GetNormalSRV() == NULL;
		if (isNormalMapped) //is there a better way to check than nullptr?
		{
			pixelShader->SetFloat3("ambientColor", ambientColor);
			pixelShader->SetFloat("specularExponent", entities[i]->GetMaterial()->GetSpecularExponent());
			pixelShader->SetFloat3("camWorldPos", camera->GetTransform()->GetPosition());

			pixelShader->CopyAllBufferData();

			//this is not part of the buffer data because it is not directly conected to the pipeline
			//pixelShader->SetShaderResourceView("diffuseTexture", srvTexture1Diffuse.Get());
			pixelShader->SetShaderResourceView("albedoTexture", entities[i]->GetMaterial()->GetTextureSRV().Get());
			pixelShader->SetShaderResourceView("roughnessMap", entities[i]->GetMaterial()->GetRoughnessSRV().Get());
			pixelShader->SetShaderResourceView("metalnessMap", entities[i]->GetMaterial()->GetMetalnessSRV().Get());

			pixelShader->SetSamplerState("basicSampler", entities[i]->GetMaterial()->GetSamplerState().Get());
		}
		else
		{
			pixelShaderNormal->SetFloat3("ambientColor", ambientColor);
			pixelShaderNormal->SetFloat("specularExponent", entities[i]->GetMaterial()->GetSpecularExponent());
			pixelShaderNormal->SetFloat3("camWorldPos", camera->GetTransform()->GetPosition());
			
			pixelShaderNormal->CopyAllBufferData();

			pixelShaderNormal->SetShaderResourceView("albedoTexture", entities[i]->GetMaterial()->GetTextureSRV().Get());
			//pixelShaderNormal->SetShaderResourceView("normalMap", srvTexture1Normal.Get());
			pixelShaderNormal->SetShaderResourceView("normalMap", entities[i]->GetMaterial()->GetNormalSRV().Get());
			pixelShader->SetShaderResourceView("roughnessMap", entities[i]->GetMaterial()->GetRoughnessSRV().Get());
			pixelShader->SetShaderResourceView("metalnessMap", entities[i]->GetMaterial()->GetMetalnessSRV().Get());
			pixelShaderNormal->SetSamplerState("basicSampler", entities[i]->GetMaterial()->GetSamplerState().Get());

		}

		entities[i]->GetMaterial()->GetVertexShader()->SetShader();
		entities[i]->GetMaterial()->GetPixelShader()->SetShader();






		// Ensure the pipeline knows how to interpret the data (numbers)
		// from the vertex buffer.  
		// - If all of your 3D models use the exact same vertex layout,
		//    this could simply be done once in Init()
		// - However, this isn't always the case (but might be for this course)
		//context->IASetInputLayout(inputLayout.Get());


		// Set buffers in the input assembler
		//  - Do this ONCE PER OBJECT you're drawing, since each object might
		//    have different geometry.
		//  - for this demo, this step *could* simply be done once during Init(),
		//    but I'm doing it here because it's often done multiple times per frame
		//    in a larger application/game
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		/*context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);*/

		std::shared_ptr<SimpleVertexShader> vertShader = entities[i]->GetMaterial()->GetVertexShader();
		vertShader->SetFloat4("colorTint", entities[i]->GetMaterial()->GetColorTint());//do i even still need color tint
		vertShader->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorldMatrix());
		vertShader->SetMatrix4x4("view", camera->GetViewMatrix());
		vertShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());

		vertShader->CopyAllBufferData();
		//D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		//context->Map(constantBufferVS.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

		//memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

		//context->Unmap(constantBufferVS.Get(), 0);
		//context->VSSetConstantBuffers(
		//0,
		//1,
		//constantBufferVS.GetAddressOf());

		///*context->IASetVertexBuffers(0, 1, meshes[i]->GetVertexBuffer().GetAddressOf(), &stride, &offset);
		//context->IASetIndexBuffer(meshes[i]->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);*/
		//vsData.worldMatrix = entities[i]->GetTransform()->GetWorldMatrix();//->Rotate(4, 0, 0);
		context->IASetVertexBuffers(0, 1, entities[i]->GetMesh()->GetVertexBuffer().GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);



		// Finally do the actual drawing
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		context->DrawIndexed(
			entities[i]->GetMesh()->GetIndexCount(),//meshes[i]->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices


	}

	

	skybox->DrawSky(context, camera);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}

void Game::DrawParticles()
{
	//particle drawing-----
// Particle states
	context->OMSetBlendState(particleBlendState.Get(), 0, 0xffffffff);	// Additive blending
	context->OMSetDepthStencilState(particleDepthState.Get(), 0);		// No depth WRITING

	// wireframe setting - set to 0 for no wireframe, 1 for wireframe
	pixelShaderParticle->SetInt("debugWireframe", 0); //<-- here
	pixelShaderParticle->CopyAllBufferData();

	//draw emitters
	emitter1->Draw(context, camera);

	// Reset to default states for next frame
	context->OMSetBlendState(0, 0, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);

	////change to sky-specific rasterizer state
	//context->RSSetState(skybox->GetRasterizerOptions().Get());

	////set sky shaders
	///*skyVS->SetShader();
	//skyPS->SetShader();*/
	//std::shared_ptr<SimpleVertexShader> vertShad = skybox->GetVertexShader();
	//std::shared_ptr<SimplePixelShader> pixShad = skybox->GetPixelShader();
	//vertShad->SetShader();
	//pixShad->SetShader();

	////XMMATRIX centerBoxOnCamera

	////give proper data
	//vertShad->SetMatrix4x4("view", camera->GetViewMatrix());
	//vertShad->SetMatrix4x4("projection", camera->GetViewMatrix());

	////set mesh buffers and draw
	////skyMesh->SetBuffersAndDraw(context); //not yet an existing method
	//// Set buffers in the input assembler
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, skybox->GetMesh()->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	//context->IASetIndexBuffer(skybox->GetMesh()->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	//// Draw this mesh
	//context->DrawIndexed(skybox->GetMesh()->GetIndexCount(), 0, 0);

	////reset back to default
	//context->RSSetState(0); // null or 0 puts back the defaults

}