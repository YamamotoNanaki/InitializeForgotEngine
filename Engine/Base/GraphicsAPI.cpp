#include "GraphicsAPI.h"
#include <cassert>
#include <thread>
#include "Debug.h"
#include "IFETime.h"

using namespace Microsoft::WRL;
using namespace std;
using namespace std::chrono;
using namespace IFE;

ID3D12Device* GraphicsAPI::device = nullptr;

void IFE::GraphicsAPI::InitializeFixFps()
{
	reference_ = IFETime::GetStartTime();
}

void IFE::GraphicsAPI::UpdateFixFps()
{
	const microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	const microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	steady_clock::time_point now = steady_clock::now();
	microseconds elapsed = duration_cast<microseconds>(now - reference_);

	if (elapsed < kMinCheckTime)
	{
		while (steady_clock::now() - reference_ < kMinTime)
		{
			this_thread::sleep_for(microseconds(1));
		}
	}
	IFETime::Update();
	reference_ = IFETime::GetNowTime();
}

GraphicsAPI* IFE::GraphicsAPI::Instance()
{
	static GraphicsAPI inst;
	return &inst;
}

void IFE::GraphicsAPI::Finalize()
{
#ifdef _DEBUG
	ID3D12DebugDevice* debugInterface = nullptr;
	if (SUCCEEDED(device->QueryInterface(&debugInterface)))
	{
		debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		debugInterface->Release();
	}
#endif
	device->Release();
}

void GraphicsAPI::Initialize(const HWND& hwnd, const uint32_t& window_width, const uint32_t& window_height)
{
	IFETime::Initialize();
	InitializeFixFps();
	//������
	ComPtr < IDXGIAdapter4> tmpAdapter = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//�[�x�o�b�t�@
	D3D12_RESOURCE_DESC depthResDesc{};
	D3D12_HEAP_PROPERTIES depthHeapProp{};
	D3D12_CLEAR_VALUE depthClearValue{};
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	// DXGI�t�@�N�g���[�̐���
	HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	// �A�_�v�^�[�̗񋓗p
	vector< ComPtr<IDXGIAdapter4>> adapters;
	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(tmpAdapter); //���I�z��ɒǉ�����
	}

	for (size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC3 adapterDesc;
		adapters[i]->GetDesc3(&adapterDesc); // �A�_�v�^�[�̏����擾

		// �\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			tmpAdapter = adapters[i]; //�̗p
			break;
		}
	}
	// �Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

#ifdef _DEBUG
	ID3D12Debug1* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	for (size_t i = 0; i < _countof(levels); i++)
	{
		// �̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		if (result == S_OK)
		{
			// �f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue>infoQueue;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		D3D12_MESSAGE_ID denyids[] = { D3D12_MESSAGE_ID_RESOURCE_BARRIER_BEFORE_AFTER_MISMATCH, D3D12_MESSAGE_ID_GPU_BASED_VALIDATION_INCOMPATIBLE_RESOURCE_STATE };
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyids);
		filter.DenyList.pIDList = denyids;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);
		infoQueue->Release();
	}
#endif
	// �R�}���h�A���P�[�^�𐶐�
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(result));

	// �R�}���h���X�g�̐���
	result = device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));
	// �e��ݒ�����ăX���b�v�`�F�[���𐶐�
	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �F���̏���
	swapChainDesc.SampleDesc.Count = 1; // �}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // �o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2; // �o�b�t�@�����Q�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //�t���b�v��͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr <IDXGISwapChain1> swapchain1;

	result = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapchain1);

	swapchain1.As(&swapchain);

	// �e��ݒ�����ăf�X�N���v�^�q�[�v�𐶐�
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �����_�[�^�[�Q�b�g�r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;	// ���\�̂Q��
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeaps));

	backBuffers.resize(swapChainDesc.BufferCount);

	for (size_t i = 0; i < backBuffers.size(); i++)
	{
		// �X���b�v�`�F�[������o�b�t�@���擾
		swapchain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		// �f�X�N���v�^�q�[�v�̃n���h���̎擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		// �����\���A�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		// �����_�[�^�[�Q�b�g�r���[�̐���
		device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
	}

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//���\�[�X�ݒ�
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = window_width;			//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResDesc.Height = window_height;		//�����_�[�^�[�Q�b�g�ɍ��킹��
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;	//�[�x�l�t�H�[�}�b�g
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//�f�v�X�X�e���V��

	//�[�x�n�p�q�[�v�v���p�e�B
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//�[�x�n�̃N���A��ݒ�
	depthClearValue.DepthStencil.Depth = 1.0f;			//�[�x�l1(�ő�l)�ŃN���A
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;		//�[�x�l�t�H�[�}�b�g

	result = device->CreateCommittedResource(
		&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,					//�[�x�l�������݂Ɏg�p
		&depthClearValue, IID_PPV_ARGS(&depthBuffer));

	//�[�x�r���[�p�f�X�N���v�^�q�[�v�쐬
	dsvHeapDesc.NumDescriptors = 1;						//�[�x�r���[��1��
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	//�f�v�X�X�e���V���r���[
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//�[�x�r���[�쐬
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
	//#ifdef _DEBUG
	//	SetNewViewPort(800, 450, 200, 40);
	//#else
	SetNewViewPort(static_cast<float>(window_width), static_cast<float>(window_height), 0, 0);
	//#endif
	SetScissorrect(0, static_cast<float>(window_width), 0, static_cast<float>(window_height));
}

void GraphicsAPI::DrawBefore()
{
	//�`��O����
	UINT bbIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH;
	bbIndex = swapchain->GetCurrentBackBufferIndex();
	// 1�D���\�[�X�o���A�ŏ������݉\�ɕύX
	barrierDesc.Transition.pResource = backBuffers[bbIndex].Get(); // �o�b�N�o�b�t�@���w��
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT; // �\������
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // �`��
	commandList->ResourceBarrier(1, &barrierDesc);

	// 2�D�`���w��
	// �����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v�̃n���h�����擾
	rtvHandle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
	//�[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h�����擾
	dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvH);

	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->RSSetScissorRects(1, &scissorrect);
}

void IFE::GraphicsAPI::DrawSetViewport(const int32_t& viewportNum)
{
	commandList->RSSetViewports(1, &viewport[viewportNum]);
}

void GraphicsAPI::DrawAfter()
{
	// 5�D���\�[�X�o���A��߂�
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // �`��
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;   // �\����
	commandList->ResourceBarrier(1, &barrierDesc);

	// ���߂̃N���[�Y
	HRESULT result = commandList->Close();
	assert(SUCCEEDED(result));
	// �R�}���h���X�g�̎��s
	ID3D12CommandList* commandLists[] = { commandList.Get() }; // �R�}���h���X�g�̔z��
	commandQueue->ExecuteCommandLists(1, commandLists);

	// �o�b�t�@���t���b�v�i���\�̓��ւ��j
	result = swapchain->Present(1, 0);
	assert(SUCCEEDED(result));
	// �R�}���h���X�g�̎��s������҂�
	commandQueue->Signal(fence.Get(), ++fenceVal);

	if (fence->GetCompletedValue() != fenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	UpdateFixFps();

	result = commandAllocator->Reset(); // �L���[���N���A
	assert(SUCCEEDED(result));
	result = commandList->Reset(commandAllocator.Get(), nullptr);  // �ĂуR�}���h���X�g�𒙂߂鏀��
	assert(SUCCEEDED(result));
}

uint32_t  IFE::GraphicsAPI::SetNewViewPort(const float& width, const float& height, const float& topX, const float& topY, const float& minDepth, const float& maxDepth)
{
	D3D12_VIEWPORT view{};

	view.Width = width;
	view.Height = height;
	view.TopLeftX = topX;
	view.TopLeftY = topY;
	view.MinDepth = minDepth;
	view.MaxDepth = maxDepth;

	this->viewport.push_back(view);

	return static_cast<uint32_t>(viewport.size()) - 1;
}

void IFE::GraphicsAPI::SetScissorrect(const float& left, const float& right, const float& top, const float& bottom)
{
	scissorrect.left = static_cast<LONG>(left);
	scissorrect.right = scissorrect.left + static_cast<LONG>(right);
	scissorrect.top = static_cast<LONG>(top);
	scissorrect.bottom = scissorrect.top + static_cast<LONG>(bottom);
}

ID3D12GraphicsCommandList* IFE::GraphicsAPI::GetCmdList()
{
	return commandList.Get();
}

ID3D12Device* IFE::GraphicsAPI::GetDevice()
{
	return device;
}

void GraphicsAPI::SetClearColor(const Float4& color)
{
	clearColor[0] = color.x;
	clearColor[1] = color.y;
	clearColor[2] = color.z;
	clearColor[3] = color.w;
}

void GraphicsAPI::SetClearColor(const float& R, const float& G, const float& B)
{
	clearColor[0] = R;
	clearColor[1] = G;
	clearColor[2] = B;
	clearColor[3] = 1;
}

ID3D12CommandQueue* IFE::GraphicsAPI::GetCommandQueue()
{
	return commandQueue.Get();
}

ID3D12Fence* IFE::GraphicsAPI::GetFence()
{
	return fence.Get();
}

UINT64& IFE::GraphicsAPI::GetFenceVal()
{
	return fenceVal;
}

ID3D12CommandAllocator* IFE::GraphicsAPI::GetCommandAllocator()
{
	return commandAllocator.Get();
}