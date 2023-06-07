#include "GraphicsPipeline.h"
#include <d3dcompiler.h>
#include "Debug.h"
#include "StringUtil.h"
#include "GraphicsAPI.h"
#include "TextureManager.h"
#include <cassert>

using namespace std;
using namespace IFE;

Microsoft::WRL::ComPtr<ID3DBlob> GraphicsPipeline::ErrorBlob = nullptr;

bool IFE::GraphicsPipeline::CreateGraphicsPpipeline(std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc, D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc)
{
	ID3D12Device* device = GraphicsAPI::GetDevice();
	ID3DBlob* rootSigBlob = nullptr;
	HRESULT result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &ErrorBlob);

	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		string error;
		error.resize(ErrorBlob->GetBufferSize());

		copy_n((char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize(), error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(SUCCEEDED(result));
		return true;
	}
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	assert(SUCCEEDED(result));
	rootSigBlob->Release();

	pipelineDesc.pRootSignature = rootsignature.Get();

	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelinestate));
	return false;
}

bool IFE::GraphicsPipeline::ShaderCompile(std::string shaderName, SHADER_COMPILE_SETTINGS setting)
{
	const static LPCSTR setting_shader_type[3] = { "vs_5_0","ps_5_0","gs_5_0" };
	size_t num = static_cast<size_t>(setting);
	std::wstring wname = StringToWString(shaderName);
	HRESULT result = D3DCompileFromFile(wname.c_str(),  // �V�F�[�_�t�@�C����
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", setting_shader_type[num], // �G���g���[�|�C���g���A�V�F�[�_�[���f��	�w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0, &blobs[num], &ErrorBlob);

	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		string error;
		error.resize(ErrorBlob->GetBufferSize());

		copy_n((char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize(), error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		return true;
	}
	return false;
}

void IFE::GraphicsPipeline::CreateBasicGraphicsPipeLine()
{
	string vs = defaultDirectory + "ModelVS.hlsl";
	ShaderCompile(vs, SHADER_COMPILE_SETTINGS::Vertex);
	string ps = defaultDirectory + "ModelPS.hlsl";
	ShaderCompile(ps, SHADER_COMPILE_SETTINGS::Pixel);
	string gs = defaultDirectory + "ModelGS.hlsl";
	ShaderCompile(gs, SHADER_COMPILE_SETTINGS::Geometry);
	name = "3dNormal";

	vector<D3D12_ROOT_PARAMETER> rootParams;

	for (size_t i = 0; i < 4; i++)
	{
		D3D12_ROOT_PARAMETER rootParamSeed;
		//�萔�p
		rootParamSeed.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				//���
		rootParamSeed.Descriptor.ShaderRegister = (UINT)i;								//�f�X�N���v�^�����W
		rootParamSeed.Descriptor.RegisterSpace = 0;									//�f�X�N���v�^�����W��
		rootParamSeed.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;				//���ׂẴV�F�[�_�[���猩����
		rootParams.push_back(rootParamSeed);
	}

	D3D12_ROOT_PARAMETER rootParamSeed2;
	rootParamSeed2.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;		//���
	rootParamSeed2.DescriptorTable.pDescriptorRanges = &TextureManager::Instance()->GetDescRangeSRV();				//�f�X�N���v�^�����W
	rootParamSeed2.DescriptorTable.NumDescriptorRanges = 1;							//�f�X�N���v�^�����W��
	rootParamSeed2.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;					//���ׂẴV�F�[�_�[���猩����
	rootParams.push_back(rootParamSeed2);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	inputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	inputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	inputLayout.push_back({ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//�c�J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���s�J��Ԃ�
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;					//���j�A�⊮
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;									//�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;												//�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//�s�N�Z���V�F�[�_�[����̂݉�

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootParams.front();
	rootSignatureDesc.NumParameters = (UINT)rootParams.size();
	//�e�N�X�`���ǉ�
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	pipelineDesc.VS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Vertex]->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Vertex]->GetBufferSize();
	if (blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry] != nullptr)
	{
		pipelineDesc.GS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry]->GetBufferPointer();
		pipelineDesc.GS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry]->GetBufferSize();
	}
	pipelineDesc.PS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Pixel]->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Pixel]->GetBufferSize();
	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState.DepthEnable = true;		//�[�x�e�X�g���s��
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;		//�[�x�l�t�H�[�}�b�g

	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;  // �w�ʂ��J�����O
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // �|���S�����h��Ԃ�
	pipelineDesc.RasterizerState.DepthClipEnable = true; // �[�x�N���b�s���O��L����

	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//�������݋���

	D3D12_INPUT_ELEMENT_DESC* layout = new D3D12_INPUT_ELEMENT_DESC[inputLayout.size()];
	for (size_t i = 0; i < inputLayout.size(); i++)
	{
		layout[i] = inputLayout[i];
	}

	pipelineDesc.InputLayout.pInputElementDescs = layout;
	pipelineDesc.InputLayout.NumElements = (UINT)inputLayout.size();

	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	pipelineDesc.NumRenderTargets = 1; // �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0�`255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = pipelineDesc.BlendState.RenderTarget[0];
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	blendDesc.BlendEnable = true;						//�u�����h��L���ɂ���
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;			//���Z
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//�\�[�X�̒l��100%�g��
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;			//�f�X�g�̒l��  0%�g��

	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;				//���Z
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//�\�[�X�̃A���t�@�l
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;		//1.0f-�\�[�X�̃A���t�@�l

	CreateGraphicsPpipeline(inputLayout, rootSignatureDesc, pipelineDesc);
	delete[] layout;
}

void IFE::GraphicsPipeline::CreateBasic2DGraphicsPipeLine()
{
	string vs = defaultDirectory + "SpriteVS.hlsl";
	ShaderCompile(vs, SHADER_COMPILE_SETTINGS::Vertex);
	string ps = defaultDirectory + "SpritePS.hlsl";
	ShaderCompile(ps, SHADER_COMPILE_SETTINGS::Pixel);
	name = "2dNormal";

	vector<D3D12_ROOT_PARAMETER> rootParams;

	for (size_t i = 0; i < 2; i++)
	{
		D3D12_ROOT_PARAMETER rootParamSeed;
		//�萔�p
		rootParamSeed.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				//���
		rootParamSeed.Descriptor.ShaderRegister = (UINT)i;								//�f�X�N���v�^�����W
		rootParamSeed.Descriptor.RegisterSpace = 0;									//�f�X�N���v�^�����W��
		rootParamSeed.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;				//���ׂẴV�F�[�_�[���猩����
		rootParams.push_back(rootParamSeed);
	}

	D3D12_ROOT_PARAMETER rootParamSeed2;
	rootParamSeed2.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;		//���
	rootParamSeed2.DescriptorTable.pDescriptorRanges = &TextureManager::Instance()->GetDescRangeSRV();				//�f�X�N���v�^�����W
	rootParamSeed2.DescriptorTable.NumDescriptorRanges = 1;							//�f�X�N���v�^�����W��
	rootParamSeed2.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;					//���ׂẴV�F�[�_�[���猩����
	rootParams.push_back(rootParamSeed2);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	inputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	inputLayout.push_back({ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//�c�J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���s�J��Ԃ�
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;					//���j�A�⊮
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;									//�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;												//�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//�s�N�Z���V�F�[�_�[����̂݉�

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootParams.front();
	rootSignatureDesc.NumParameters = (UINT)rootParams.size();
	//�e�N�X�`���ǉ�
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	pipelineDesc.VS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Vertex]->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Vertex]->GetBufferSize();
	if (blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry] != nullptr)
	{
		pipelineDesc.GS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry]->GetBufferPointer();
		pipelineDesc.GS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry]->GetBufferSize();
	}
	pipelineDesc.PS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Pixel]->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Pixel]->GetBufferSize();
	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState.DepthEnable = false;		//�[�x�e�X�g���s��Ȃ�
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;		//�[�x�l�t�H�[�}�b�g

	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // �w�ʂ��J�����O���Ȃ�
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // �|���S�����h��Ԃ�
	pipelineDesc.RasterizerState.DepthClipEnable = true; // �[�x�N���b�s���O��L����

	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//�������݋���

	D3D12_INPUT_ELEMENT_DESC* layout = new D3D12_INPUT_ELEMENT_DESC[inputLayout.size()];
	for (size_t i = 0; i < inputLayout.size(); i++)
	{
		layout[i] = inputLayout[i];
	}

	pipelineDesc.InputLayout.pInputElementDescs = layout;
	pipelineDesc.InputLayout.NumElements = (UINT)inputLayout.size();

	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	pipelineDesc.NumRenderTargets = 1; // �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0�`255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = pipelineDesc.BlendState.RenderTarget[0];
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	blendDesc.BlendEnable = true;						//�u�����h��L���ɂ���
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;			//���Z
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//�\�[�X�̒l��100%�g��
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;			//�f�X�g�̒l��  0%�g��

	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;				//���Z
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//�\�[�X�̃A���t�@�l
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;		//1.0f-�\�[�X�̃A���t�@�l

	CreateGraphicsPpipeline(inputLayout, rootSignatureDesc, pipelineDesc);
	delete[] layout;
}

void IFE::GraphicsPipeline::CreateBasicParticleGraphicsPipeLine()
{
	string vs = defaultDirectory + "ParticleVS.hlsl";
	ShaderCompile(vs, SHADER_COMPILE_SETTINGS::Vertex);
	string ps = defaultDirectory + "ParticlePS.hlsl";
	ShaderCompile(ps, SHADER_COMPILE_SETTINGS::Pixel);
	string gs = defaultDirectory + "ParticleGS.hlsl";
	ShaderCompile(gs, SHADER_COMPILE_SETTINGS::Geometry);
	name = "ParticleNormal";

	vector<D3D12_ROOT_PARAMETER> rootParams;

	for (size_t i = 0; i < 2; i++)
	{
		D3D12_ROOT_PARAMETER rootParamSeed;
		//�萔�p
		rootParamSeed.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				//���
		rootParamSeed.Descriptor.ShaderRegister = (UINT)i;								//�f�X�N���v�^�����W
		rootParamSeed.Descriptor.RegisterSpace = 0;									//�f�X�N���v�^�����W��
		rootParamSeed.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;				//���ׂẴV�F�[�_�[���猩����
		rootParams.push_back(rootParamSeed);
	}

	D3D12_ROOT_PARAMETER rootParamSeed2;
	rootParamSeed2.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;		//���
	rootParamSeed2.DescriptorTable.pDescriptorRanges = &TextureManager::Instance()->GetDescRangeSRV();				//�f�X�N���v�^�����W
	rootParamSeed2.DescriptorTable.NumDescriptorRanges = 1;							//�f�X�N���v�^�����W��
	rootParamSeed2.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;					//���ׂẴV�F�[�_�[���猩����
	rootParams.push_back(rootParamSeed2);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	inputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//�c�J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;					//���s�J��Ԃ�
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;	//�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;					//���j�A�⊮
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;									//�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;												//�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//�s�N�Z���V�F�[�_�[����̂݉�

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootParams.front();
	rootSignatureDesc.NumParameters = (UINT)rootParams.size();
	//�e�N�X�`���ǉ�
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	pipelineDesc.VS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Vertex]->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Vertex]->GetBufferSize();
	if (blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry] != nullptr)
	{
		pipelineDesc.GS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry]->GetBufferPointer();
		pipelineDesc.GS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Geometry]->GetBufferSize();
	}
	pipelineDesc.PS.pShaderBytecode = blobs[(size_t)SHADER_COMPILE_SETTINGS::Pixel]->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = blobs[(size_t)SHADER_COMPILE_SETTINGS::Pixel]->GetBufferSize();
	//�f�v�X�X�e���V���X�e�[�g�̐ݒ�
	pipelineDesc.DepthStencilState.DepthEnable = true;		//�[�x�e�X�g���s��Ȃ�
	pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;		//�[�x�l�t�H�[�}�b�g

	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // �w�ʂ��J�����O���Ȃ�
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // �|���S�����h��Ԃ�
	pipelineDesc.RasterizerState.DepthClipEnable = true; // �[�x�N���b�s���O��L����

	pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//�������݋���

	D3D12_INPUT_ELEMENT_DESC* layout = new D3D12_INPUT_ELEMENT_DESC[inputLayout.size()];
	for (size_t i = 0; i < inputLayout.size(); i++)
	{
		layout[i] = inputLayout[i];
	}

	pipelineDesc.InputLayout.pInputElementDescs = layout;
	pipelineDesc.InputLayout.NumElements = (UINT)inputLayout.size();

	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	pipelineDesc.NumRenderTargets = 1; // �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0�`255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = pipelineDesc.BlendState.RenderTarget[0];
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	blendDesc.BlendEnable = true;						//�u�����h��L���ɂ���
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;			//���Z
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;			//�\�[�X�̒l��100%�g��
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;			//�f�X�g�̒l��  0%�g��

	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;				//���Z
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//�\�[�X�̃A���t�@�l
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;		//1.0f-�\�[�X�̃A���t�@�l

	CreateGraphicsPpipeline(inputLayout, rootSignatureDesc, pipelineDesc);
	delete[] layout;
}

void IFE::GraphicsPipeline::SetDrawBlendMode()
{
	ID3D12GraphicsCommandList* commandList = GraphicsAPI::Instance()->GetCmdList();
	commandList->SetGraphicsRootSignature(rootsignature.Get());
	commandList->SetPipelineState(pipelinestate.Get());
}