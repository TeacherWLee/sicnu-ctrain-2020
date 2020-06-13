
// demo2-chat.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号


// Cdemo2chatApp:
// 有关此类的实现，请参阅 demo2-chat.cpp
//

class Cdemo2chatApp : public CWinApp
{
public:
	Cdemo2chatApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern Cdemo2chatApp theApp;
