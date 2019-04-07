#include "stdafx.h"
#include "WinUser.h"
#include "VirtualKeys.h"

VirtualKeys::VirtualKeys()
{
}

VirtualKeys::~VirtualKeys()
{
}

bool VirtualKeys::GetKeyPressName(DWORD dwCmdId, CString &zName)
{
	int nVirtKey = ((dwCmdId & KEYBINDING_KEYPRESS) > 0) ? (int)(dwCmdId & KEYBINDING_KEYPRESS_MASK) : 0;
	bool bShift = ((dwCmdId & KEYBINDING_KEYPRESS_SHIFT) > 0);
	bool bCtrl = ((dwCmdId & KEYBINDING_KEYPRESS_CTRL) > 0);
	bool bAlt = ((dwCmdId & KEYBINDING_KEYPRESS_ALT) > 0);

	if (nVirtKey > 0)
	{
		return GetKeyPressName(nVirtKey, bShift, bCtrl, bAlt, zName);
	}
	else
	{
		zName.Empty();
		return false;
	}
}

bool VirtualKeys::GetKeyPressName(int nVirtKey, bool bShift, bool bCtrl, bool bAlt, CString &nameStr)
{
	nameStr.Empty();

	if (!isValidKey(nVirtKey, bShift, bCtrl, bAlt))
		return false;

	nameStr.Append(L"Key Press: ");

	if (bCtrl) nameStr.Append(L"Ctrl + ");
	if (bAlt) nameStr.Append(L"Alt + ");
	if (bShift) nameStr.Append(L"Shift + ");

	switch (nVirtKey)
	{
		case VK_OEM_1: nameStr.Append(L";"); break;       // ';:' for US
		case VK_OEM_2: nameStr.Append(L"/"); break;       // '/?' for US
		case VK_OEM_3: nameStr.Append(L"~"); break;		// '`~' for US
		case VK_OEM_4: nameStr.Append(L"["); break;		//  '[{' for US
		case VK_OEM_5: nameStr.Append(L"\\ (US)"); break;		//  '\|' for US
		case VK_OEM_6: nameStr.Append(L"]"); break;		//  ']}' for US
		case VK_OEM_7: nameStr.Append(L"'"); break;		//  ''"' for US
		case VK_OEM_102: nameStr.Append(L"\\ (RT-102)"); break;		//  '\|' for NON-US
		case VK_OEM_PLUS: nameStr.Append(L"="); break;    // '=+' any country
		case VK_OEM_COMMA: nameStr.Append(L", (comma)"); break;   // ',<' any country
		case VK_OEM_MINUS: nameStr.Append(L"- (minus)"); break;  // '-_' any country
		case VK_OEM_PERIOD: nameStr.Append(L". (period)"); break;  // '.>' any country
		case VK_SCROLL: nameStr.Append(L"SCROLL LOCK"); break; 
		case VK_NUMLOCK: nameStr.Append(L"NUM LOCK"); break;
		case VK_CAPITAL: nameStr.Append(L"CAPS LOCK"); break;
		case VK_TAB: nameStr.Append(L"TAB"); break;
		case VK_BACK: nameStr.Append(L"BACKSPACE"); break;
		case VK_RETURN: nameStr.Append(L"ENTER"); break;
		case VK_PAUSE: nameStr.Append(L"Pause"); break;
		case VK_SPACE: nameStr.Append(L"SPACE"); break;
		case VK_END: nameStr.Append(L"End"); break;
		case VK_HOME: nameStr.Append(L"Home"); break;
		case VK_LEFT: nameStr.Append(L"Left"); break;
		case VK_UP: nameStr.Append(L"Up"); break;
		case VK_RIGHT: nameStr.Append(L"Right"); break;
		case VK_DOWN: nameStr.Append(L"Down"); break;
		case VK_PRIOR: nameStr.Append(L"PgUp"); break;
		case VK_NEXT: nameStr.Append(L"PgDn"); break;
		case VK_INSERT: nameStr.Append(L"Insert"); break;
		case VK_DELETE: nameStr.Append(L"Delete"); break;
		case VK_NUMPAD0: nameStr.Append(L"NUM 0"); break;
		case VK_NUMPAD1: nameStr.Append(L"NUM 1"); break;
		case VK_NUMPAD2: nameStr.Append(L"NUM 2"); break;
		case VK_NUMPAD3: nameStr.Append(L"NUM 3"); break;
		case VK_NUMPAD4: nameStr.Append(L"NUM 4"); break;
		case VK_NUMPAD5: nameStr.Append(L"NUM 5"); break;
		case VK_NUMPAD6: nameStr.Append(L"NUM 6"); break;
		case VK_NUMPAD7: nameStr.Append(L"NUM 7"); break;
		case VK_NUMPAD8: nameStr.Append(L"NUM 8"); break;
		case VK_NUMPAD9: nameStr.Append(L"NUM 9"); break;
		case VK_MULTIPLY: nameStr.Append(L"NUMMULT"); break;
		case VK_ADD: nameStr.Append(L"NUM PLUS"); break;
		case VK_SEPARATOR: nameStr.Append(L"NUM ,"); break;
		case VK_SUBTRACT: nameStr.Append(L"NUM SUB"); break;
		case VK_DECIMAL: nameStr.Append(L"NUM ."); break;
		case VK_DIVIDE: nameStr.Append(L"Num /"); break;
		case VK_F1: nameStr.Append(L"F1"); break;
		case VK_F2: nameStr.Append(L"F2"); break;
		case VK_F3: nameStr.Append(L"F3"); break;
		case VK_F4: nameStr.Append(L"F4"); break;
		case VK_F5: nameStr.Append(L"F5"); break;
		case VK_F6: nameStr.Append(L"F6"); break;
		case VK_F7: nameStr.Append(L"F7"); break;
		case VK_F8: nameStr.Append(L"F8"); break;
		case VK_F9: nameStr.Append(L"F9"); break;
		case VK_F10: nameStr.Append(L"F10"); break;
		case VK_F11: nameStr.Append(L"F11"); break;
		case VK_F12: nameStr.Append(L"F12"); break;
		default: nameStr.AppendChar(nVirtKey);
	}

	return true;
}

bool VirtualKeys::CommandIdToVirtualKey(DWORD dwCmdId, int &nVirtKey, bool &bShift, bool &bCtrl, bool &bAlt)
{
	bAlt = ((dwCmdId & KEYBINDING_KEYPRESS_ALT) > 0);
	bCtrl = ((dwCmdId & KEYBINDING_KEYPRESS_CTRL) > 0);
	bShift = ((dwCmdId & KEYBINDING_KEYPRESS_SHIFT) > 0);
	nVirtKey = ((dwCmdId & KEYBINDING_KEYPRESS) > 0) ? (int)(dwCmdId & KEYBINDING_KEYPRESS_MASK) : 0;

	return (nVirtKey > 0);
}

DWORD VirtualKeys::VirtualKeyToCommandId(int nVirtKey, bool bShift, bool bCtrl, bool bAlt)
{
	DWORD dwCmdId = nVirtKey;
	dwCmdId += KEYBINDING_KEYPRESS;
	dwCmdId += bShift ? KEYBINDING_KEYPRESS_SHIFT : 0;
	dwCmdId += bCtrl ? KEYBINDING_KEYPRESS_CTRL : 0;
	dwCmdId += bAlt ? KEYBINDING_KEYPRESS_ALT : 0;

	return dwCmdId;
}

bool VirtualKeys::isValidKey(int nVirtKey, bool bShift, bool bCtrl, bool bAlt)
{
	for (int i = 0; i < (sizeof(VALID_VKEYS) / sizeof(int)); i++)
	{
		if (nVirtKey == VALID_VKEYS[i])
			return true;
	}

	return false;
}

