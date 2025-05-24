#include "KSContext.h"
#include <stdexcept>
#include "Logger/KSLogger.h"

KSContext& KSContext::Instance()
{
	static KSContext instance;
	return instance;
}

void KSContext::Initialize(const std::wstring& title, int width, int height)
{
	if (m_window)
	{
		LOG_FATAL(L"KSContext::Initialize called more than once!");
	}
	m_width = width;
	m_height = height;
	m_title = title;
	m_window = std::make_unique<KSWindow>(title, width, height);

	if (!m_window->GetHWND())
	{
		LOG_FATAL(L"KSContext::Initialize failed to create window!");
	}
	m_dx12 = std::make_unique<KSDirectX12>();
	if (!m_dx12->Initialize())
	{
		LOG_FATAL(L"KSContext::Initialize failed to initialize DirectX 12!");
	}
}

void KSContext::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
}

KSWindow& KSContext::GetWindow()
{
	if (!m_window)
	{
		LOG_ERROR(L"KSContext::GetWindow called before Initialize!");
	}
	return *m_window;
}

HWND KSContext::GetHWND() const
{
	return m_window->GetHWND();
}

int32_t KSContext::GetWidth() const
{
	return m_width;
}

int32_t KSContext::GetHeight() const
{
	return m_height;
}

float KSContext::GetAspectRatio() const
{
	return (float)m_width / (float)m_height;
}

bool KSContext::GetIsRunning() const
{
	return m_isRunning;
}

void KSContext::StartRunning()
{
	m_isRunning = true;
}

void KSContext::StopRunning()
{
	m_isRunning = false;
}

KSDirectX12& KSContext::GetDirectX12()
{
	if (!m_dx12)
	{
		LOG_ERROR(L"KSContext::GetDirectX12 called before Initialize!");
	}
	return *m_dx12;
}
