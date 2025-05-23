#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>

class Material {
public:
	Material(ID3D12Device* device, const std::wstring& vsPath, const std::wstring& psPath);
	void Bind(ID3D12GraphicsCommandList* cmdList);

	ID3D12RootSignature* GetRootSignature() const
	{
		return m_rootSignature.Get();
	}
	ID3D12PipelineState* GetPipelineState() const
	{
		return m_pso.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;
};