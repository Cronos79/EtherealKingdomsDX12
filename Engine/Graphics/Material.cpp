#include "Material.h"
#include <d3dcompiler.h>
#include <vector>
#include <fstream>
#include <d3dx12.h>
#include "../Logger/KSLogger.h"

// Helper to load compiled shader bytecode from file
static std::vector<char> LoadShaderBytecode(const std::wstring& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file) return {};
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	return buffer;
}

Material::Material(ID3D12Device* device, const std::wstring& vsPath, const std::wstring& psPath)
{
	// Load shader bytecode
	auto vsBytecode = LoadShaderBytecode(vsPath);
	if (vsBytecode.empty())
	{
		throw std::runtime_error("Vertex shader bytecode is empty. Check file path and compilation.");
	}
	LOG_INFO(L"Vertex shader bytecode loaded successfully: {}", vsPath);
	auto psBytecode = LoadShaderBytecode(psPath);	
	if (psBytecode.empty())
	{
		throw std::runtime_error("Pixel shader bytecode is empty. Check file path and compilation.");
	}
	LOG_INFO(L"Pixel shader bytecode loaded successfully: {}", psPath);

	// Root signature: just a simple empty root signature for now
	CD3DX12_ROOT_PARAMETER rootParams[1];
	rootParams[0].InitAsConstantBufferView(0); // b0

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init(_countof(rootParams), rootParams, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSig, &errorBlob);

	device->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature));

	// Input layout: position, normal, texcoord
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vsBytecode.data(), vsBytecode.size() };
	psoDesc.PS = { psBytecode.data(), psBytecode.size() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso));
}

void Material::Bind(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
	cmdList->SetPipelineState(m_pso.Get());
}
