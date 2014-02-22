#ifndef KeyBinding_h
#define KeyBinding_h

/////////////////////////////////////////////////////////////////////////////

class CKeyBinding
{
public:
	CKeyBinding();
	virtual ~CKeyBinding();

	DWORD GetCommand()				{ return m_dwCmdId; };
	void SetCommand(DWORD dwCmdId)	{ m_dwCmdId = dwCmdId; };

	CString GetName()				{ return m_strName; };
	void SetName(CString str)		{ m_strName = str; };
	void SetName(const char *s)		{ m_strName = s; };

protected:
	DWORD m_dwCmdId;
	CString m_strName;
};

/////////////////////////////////////////////////////////////////////////////

#endif // KeyBinding_h
