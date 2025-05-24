#pragma once
#include <unordered_set>
#include "Engine/Platform/WinInclude.h"

namespace KSEngine
{
	class Keyboard {
	public:
		void OnKeyDown(int key)
		{
			m_keys.insert(key);
		}
		void OnKeyUp(int key)
		{
			m_keys.erase(key);
		}
		bool IsKeyDown(int key) const
		{
			return m_keys.count(key) > 0;
		}
		void Clear()
		{
			m_keys.clear();
		}

	private:
		std::unordered_set<int> m_keys;
	};
}