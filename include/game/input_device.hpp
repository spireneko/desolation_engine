#pragma once

#include <wrl.h>
#include <memory>

class InputDevice {
   public:
	static InputDevice& Get()
	{
		static InputDevice instance;
		return instance;
	}

	void ProcessRawKeyboard(const RAWKEYBOARD& keyboard)
	{
		bool pressed = !(keyboard.Flags & RI_KEY_BREAK);
		if (keyboard.VKey < 256) {
			m_keys[keyboard.VKey] = pressed;
		}
	}

	bool IsKeyDown(unsigned char key) const { return m_keys[key]; }

	void Reset() { memset(m_keys, 0, sizeof(m_keys)); }

   private:
	InputDevice() { memset(m_keys, 0, sizeof(m_keys)); }

	bool m_keys[256];
};
