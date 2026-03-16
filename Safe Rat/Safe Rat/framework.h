#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif
//#include <vld.h>
#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 的一些常见且经常可放心忽略的隐藏警告消息
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展
#include <afxdisp.h>        // MFC 自动化类


#include<XTToolkitPro.h>
#define HPSOCKET_STATIC_LIB
#include <vector>
#include "HpTcpServer.h"
#define CButton CXTPButton
#define CEdit	CXTPEdit

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC 支持功能区和控制条


enum 
{
	WM_ADDTOLIST = WM_USER + 102,		// 添加到列表视图中 ,
	WM_REMOVEFROMLIST,					// 移除列表主机
	WM_CLOSEDLG,						// 关闭窗口
	WM_OPENSHELLDIALOG,					// 打开cmd
	WM_OPENREGEDITDIALOG,				// 打开注册表
	WM_OPENSPEAKDIALOG,					// 打开扬声器
	WM_OPENMIKEDIALOG,					// 打开麦克风
	WM_OPENSERVERDIALOG,				// 打开服务管理
	WM_OPENHOSTDIALOG,					// 计算机管理
	WM_OPENKEYBOARDDIALOG,				// 实时记录
	WM_OPENSCREENDIALOG,				// 屏幕监控
	WM_OPENFILEDIALOG,					// 文件管理
	WM_OPENHIDESCREENDIALOG,
	WM_OPENPLUGINSDIALOG,
	WM_OPENCLIPDIALOG,
	

	SHELL_DLG = 1,						//cmd窗口
	REG_DLG,							//注册表窗口
	SPEAKER_DLG,						//扬声器
	MIKE_DLG,							//麦克风
	SERVER_DLG,							//服务管理
	HOST_DLG,							//计算机管理
	KEYBOARD_DLG,						//实时记录
	SCREEN_DLG,							//屏幕监控
	FILE_DLG,							//文件管理
	HIDE_DLG,							//后台桌面
	PLUG_DLG,
	CLIP_DLG,

};

#define REG_PLUG			"reg.dll"
#define CMD_PLUG			"shell.dll"
#define SPEAKER_PLUG		"speaker.dll"
#define MIKE_PLUG			"mike.dll"
#define SERVER_PLUG			"server.dll"
#define HOST_PLUG			"computer.dll"
#define KEYBOARD_PLUG		"keyboard.dll"
#define SCREEN_PLUG			"screen.dll"
#define FILE_PLUG			"file.dll"
#define HIDE_PLUG			"hidescreen.dll"
#define PLUGINS_PLUG		"plugins.dll"
#define CLIP_PLUG		    "clip.dll"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


