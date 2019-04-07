#ifndef VirtualKeys_h
#define VirtualKeys_h

static const DWORD KEYBINDING_KEYPRESS_ALT = 0x20000;
static const DWORD KEYBINDING_KEYPRESS_CTRL = 0x10000;
static const DWORD KEYBINDING_KEYPRESS_SHIFT = 0x8000;
static const DWORD KEYBINDING_KEYPRESS = 0x4000;
static const DWORD KEYBINDING_KEYPRESS_MASK = 0x3FFF;

static const int VALID_VKEYS[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	VK_OEM_1,       // ';:' for US
	VK_OEM_2,       // '/?' for US
	VK_OEM_3,		// '`~' for US
	VK_OEM_4,		//  '[{' for US
	VK_OEM_5,		//  '\|' for US
	VK_OEM_102,		//  '\|' for RT-102
	VK_OEM_6,		//  ']}' for US
	VK_OEM_7,		//  ''"' for US
	VK_OEM_PLUS,    // '=+' any country
	VK_OEM_COMMA,   // ',<' any country
	VK_OEM_MINUS,   // '-_' any country
	VK_OEM_PERIOD,  // '.>' any country
	VK_SCROLL,
	VK_NUMLOCK,
	VK_CAPITAL,
	VK_TAB,
	VK_BACK,
	VK_RETURN,
	VK_PAUSE,
	VK_SPACE,
	VK_END,
	VK_HOME,
	VK_LEFT,
	VK_UP,
	VK_RIGHT,
	VK_DOWN,
	VK_PRIOR,
	VK_NEXT,
	VK_INSERT,
	VK_DELETE,
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_MULTIPLY,
	VK_ADD,
	VK_SEPARATOR,
	VK_SUBTRACT,
	VK_DECIMAL,
	VK_DIVIDE,
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12
};

class VirtualKeys
{
public:
	VirtualKeys();
	~VirtualKeys();
	static bool GetKeyPressName(int nVirtKey, bool bShift, bool bCtrl, bool bAlt, CString &nameStr);
	static bool GetKeyPressName(DWORD dwCmdId, CString &zName);
	static bool CommandIdToVirtualKey(DWORD dwCmdId, int &nVirtKey, bool &bShift, bool &bCtrl, bool &bAlt);
	static DWORD VirtualKeyToCommandId(int dwKeyPress, bool bShift, bool bCtrl, bool bAlt);
	static bool isValidKey(int nVirtKey, bool bShift, bool bCtrl, bool bAlt);
};

#endif // VirtualKeys_h