
// LRF_control.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CLRF_controlApp: 
// �аѾ\��@�����O�� LRF_control.cpp
//

class CLRF_controlApp : public CWinApp
{
public:
	CLRF_controlApp();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CLRF_controlApp theApp;