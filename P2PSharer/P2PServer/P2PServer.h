
// P2PServer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CP2PServerApp:
// �йش����ʵ�֣������ P2PServer.cpp
//

class CP2PServerApp : public CWinAppEx
{
public:
	CP2PServerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CP2PServerApp theApp;