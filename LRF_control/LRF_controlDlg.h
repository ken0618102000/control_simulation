
// LRF_controlDlg.h : 標頭檔
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <afxsock.h>
#include <fstream>
#include <vector>
#include "CvvImage.h"  
#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include "PSocket.h"

using namespace std;
using namespace cv;

#define PI 3.14159265358
// CLRF_controlDlg 對話方塊
class CLRF_controlDlg : public CDialogEx
{
// 建構
public:
	CLRF_controlDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LRF_CONTROL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;


	enum TCPEvent {
		CREATE_SOCKET_SUCCESSFUL,
		CREATE_SOCKET_FAIL,
		CONNECT_SUCCESSFUL,
		CONNECT_FAIL,
		DISCONNECT,
		SEND_MESSAGE_SUCCESSFUL,
		SENT_MESSAGE_FAIL
	};

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	void DoPoseSocketConnect();
	void DoPoseSocketDisconnect();
	void ReportSocketStatus(TCPEvent event_, CString &msg = CString(""));
	bool fg_pose_connected;
	CPSocket m_socket_pose;
	UINT m_socket_pose_port;

	//---------thread TARGET_control------------
private:
	CWinThread * m_pThread_TARGET_control;
	bool Continue_TARGET_control;
	static UINT ThreadFun_TARGET_control(LPVOID lParam);
	//---------thread car_draw------------
private:
	CWinThread * m_pThread_car_draw;
	bool Continue_car_draw;
	static UINT ThreadFun_car_draw(LPVOID lParam);

public:
	CIPAddressCtrl m_socket_ip_c;
	CButton m_socket_connect_c;
	UINT m_socket_port;
	afx_msg void OnBnClickedSocketConnect();
	CListBox m_socket_log_c;
	afx_msg void OnBnClickedSTART();
	CRect rect_map;
	CWnd* pWnd_map;
	static double vr, vl;    //要控制I90到一定位時所用
	static double vr_draw, vl_draw;
	static double car_x, car_y, car_zdir;
	static double target_pos[3];
	static int sampleTime;
	static bool carFLAG;
	CStatic m_PATH_c;
	static void I90_PWM_send(int L_PWM, int R_PWM);
	static void I90_Speed2PWM(double i_L_Speed, double i_R_Speed, double &o_L_PWM, double &o_R_PWM);
	static unsigned char checksun(int nStar, int nEnd);
	afx_msg void OnBnClickedconnect();
	afx_msg void OnBnClickedstop();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonspeedup();
};
