
// LRF_controlDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "LRF_control.h"
#include "LRF_controlDlg.h"
#include "afxdialogex.h"
//#include "RSocket.h"
#include "CvvImage.cpp"  
#include "Serial.cpp"
#include <windows.h>
#define WNU_THREAD_EXIT (WM_USER + 1)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

double CLRF_controlDlg::vr = 0, CLRF_controlDlg::vl = 0;
double CLRF_controlDlg::car_x = 0, CLRF_controlDlg::car_y = 0, CLRF_controlDlg::car_zdir = 0;
double  CLRF_controlDlg::vr_draw = 0, CLRF_controlDlg::vl_draw = 0;
double  CLRF_controlDlg::target_pos[3] = { 0 };
int CLRF_controlDlg::sampleTime = 0;
bool CLRF_controlDlg::carFLAG = true;
//double CRSocket::m_pose_data_static[SIZE_POSE_DATA] = { 0 };
PoseData CPSocket::m_pose_data_2;
CSerial Connect_I90;
double upupup = 0;

struct THREAD_INFO_TARGET_control
{
	bool * Continue;//是否繼續執行
	HWND hWnd;//產生執行續的視窗物件
}Thread_Info_TARGET_control;

struct THREAD_INFO_car_draw
{
	bool * Continue;//是否繼續執行
	HWND hWnd;//產生執行續的視窗物件
}Thread_Info_car_draw;

// 對 App About 使用 CAboutDlg 對話方塊
int c_Synchronous = 0;
char I90_PWM_control[15] = {
	94,    //0  STX  0x5e|0x02
	2,		//1  STX
	1,		//2  RID
	0,		//3  Reserved
	5,		//4  DID  5的話是各輪子PWM控制
	6,		 //5  Length
	1,		//6   選擇輪子1(右輪)
	160,		//7   Low_8bit
	15,	//8   High_8bit(64就是16384，為中間值，右輪比16384小為前進)
	0,		 //9  選擇輪子0(左輪)
	160,		//10  Low_8bit
	15,	//11  High_8bit(64就是16384，為中間值，左輪比16384大為前進)
	0,		//12  Checksum
	94,	//13  ETX 0x5E|0X0D
	13 }; //14

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CPSocket::CPSocket()
{
	memset(&m_pose_data, 0, sizeof(PoseData));
}

CPSocket::~CPSocket() {}

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CLRF_controlDlg 對話方塊

CLRF_controlDlg::CLRF_controlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LRF_CONTROL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLRF_controlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOCKET_IP, m_socket_ip_c);
	DDX_Control(pDX, IDC_SOCKET_CONNECT, m_socket_connect_c);
	DDX_Control(pDX, IDC_SOCKET_LOG, m_socket_log_c);
	DDX_Control(pDX, IDC_Path, m_PATH_c);
}

BEGIN_MESSAGE_MAP(CLRF_controlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SOCKET_CONNECT, &CLRF_controlDlg::OnBnClickedSocketConnect)
	ON_BN_CLICKED(IDC_BUTTON2, &CLRF_controlDlg::OnBnClickedSTART)
	ON_BN_CLICKED(IDC_connect, &CLRF_controlDlg::OnBnClickedconnect)
	ON_BN_CLICKED(IDC_stop, &CLRF_controlDlg::OnBnClickedstop)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_BUTTON_speedup, &CLRF_controlDlg::OnBnClickedButtonspeedup)
END_MESSAGE_MAP()

// CLRF_controlDlg 訊息處理常式

BOOL CLRF_controlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定

	m_PATH_c.SetWindowPos(&wndTop, 10, 10, 700, 700, SWP_SHOWWINDOW);

	AfxSocketInit();
	m_socket_pose.registerParent(this);
	m_socket_ip_c.SetAddress(192, 168, 0, 105);
	m_socket_pose_port = 25652;

	UpdateData(false);



	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CLRF_controlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CLRF_controlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CLRF_controlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLRF_controlDlg::OnBnClickedSocketConnect()
{
	if (fg_pose_connected)
	{
		if (fg_pose_connected) DoPoseSocketDisconnect();
	}
	else
	{
		// If all socket not connect yet
		DoPoseSocketConnect();
	}

	m_socket_connect_c.SetCheck(fg_pose_connected);


}

void CLRF_controlDlg::DoPoseSocketConnect()
{
	UpdateData(true);

	// Read the TCP/IP address setting from User
	byte aIpAddressUnit[4];
	m_socket_ip_c.GetAddress(
		aIpAddressUnit[0], aIpAddressUnit[1], aIpAddressUnit[2], aIpAddressUnit[3]);
	CString aStrIpAddress;
	aStrIpAddress.Format(_T("%d.%d.%d.%d"),
		aIpAddressUnit[0], aIpAddressUnit[1], aIpAddressUnit[2], aIpAddressUnit[3]);

	// Create a TCP Socket for transfer Camera data
	// m_tcp_socket.Create(m_tcp_ip_port, 1, aStrIpAddress);
	if (!m_socket_pose.Create()) {

		// If Socket Create Fail Report Message
		TCHAR szMsg[1024] = { 0 };
		wsprintf(szMsg, _T("Pose socket create faild: %d"), m_socket_pose.GetLastError());
		ReportSocketStatus(TCPEvent::CREATE_SOCKET_FAIL, CString("Pose Socket"));
		AfxMessageBox(szMsg);
	}
	else {

		ReportSocketStatus(TCPEvent::CREATE_SOCKET_SUCCESSFUL, CString("Pose Socket"));
		// Connect to the Server ( Raspberry Pi Server )
		fg_pose_connected = m_socket_pose.Connect(aStrIpAddress, m_socket_pose_port);

		//For Test
		m_socket_pose.Send("Test from here", 14);
	}

	if (fg_pose_connected)
		ReportSocketStatus(TCPEvent::CONNECT_SUCCESSFUL, aStrIpAddress);
	else
	{
		ReportSocketStatus(TCPEvent::CONNECT_FAIL, aStrIpAddress);
		m_socket_pose.Close();
	}
}

void CLRF_controlDlg::DoPoseSocketDisconnect()
{
	// Setup Connect-staus flag
	fg_pose_connected = false;
	//fg_tcp_ip_read = false;

	// Close the TCP/IP Socket
	m_socket_pose.Close();

	// Report TCP/IP connect status
	CString tmp_log; tmp_log.Format(_T("I/O event: %s"), _T("Close Pose Socket"));
	m_socket_log_c.AddString(tmp_log);
}

void CLRF_controlDlg::ReportSocketStatus(TCPEvent event_, CString &msg)
{
	CString tmp_log;
	CTime current_time = CTime::GetCurrentTime();
	tmp_log.Format(_T("[ %02d點%02d分%02d秒: ] "),
		current_time.GetHour(), current_time.GetMinute(), current_time.GetSecond());
	switch (event_) {
	case CREATE_SOCKET_SUCCESSFUL:
		tmp_log.AppendFormat(_T("I/O event: %s <%s>"),
			_T("Create Socket Successful"), msg);
		break;
	case CREATE_SOCKET_FAIL:
		tmp_log.AppendFormat(_T("I/O event: %s"),
			_T("Create Socket Fail <%s>"), msg);
		break;
	case CONNECT_SUCCESSFUL:
		tmp_log.AppendFormat(_T("I/O event: %s%s%s"),
			_T("Connect "), msg, _T(" Successful"));
		break;
	case CONNECT_FAIL:
		tmp_log.AppendFormat(_T("I/O event: %s%s%s"),
			_T("Connect "), msg, _T(" Fail"));
		break;
	case DISCONNECT:
		tmp_log.AppendFormat(_T("I/O event: %s"),
			_T("Disconnect"));
		break;
	case SEND_MESSAGE_SUCCESSFUL:
		tmp_log.AppendFormat(_T("I/O event: %s%s%s"),
			_T("Sent Message"), msg, _T("Successful"));
		break;
	case SENT_MESSAGE_FAIL:
		tmp_log.AppendFormat(_T("I/O event: %s%s%s"),
			_T("Sent Message"), msg, _T("Fail"));
		break;
	}
	m_socket_log_c.AddString(tmp_log);
}

template<size_t LENGTH> void CLRF_controlDlg::SendSocketMessage(char(&data)[LENGTH])
{


}

void CLRF_controlDlg::OnBnClickedconnect()
{
	Connect_I90.Open(5, 115200);
}

void CLRF_controlDlg::OnBnClickedstop()
{
	carFLAG = false;

	for (int i = 0; i++; i < 10)
	{
		I90_PWM_send(0, 0);
		Sleep(20);
	}

}

void CLRF_controlDlg::OnBnClickedSTART()
{
	carFLAG = true;

	Continue_TARGET_control = TRUE;
	Thread_Info_TARGET_control.hWnd = m_hWnd;
	Thread_Info_TARGET_control.Continue = &Continue_TARGET_control;
	m_pThread_TARGET_control = AfxBeginThread(ThreadFun_TARGET_control, (LPVOID)&Thread_Info_TARGET_control);

	Continue_car_draw = TRUE;
	Thread_Info_car_draw.hWnd = m_hWnd;
	Thread_Info_car_draw.Continue = &Continue_car_draw;
	m_pThread_car_draw = AfxBeginThread(ThreadFun_car_draw, (LPVOID)&Thread_Info_car_draw);
}

void CLRF_controlDlg::OnBnClickedButtonspeedup()
{
	upupup = upupup - 0.1;
}

UINT CLRF_controlDlg::ThreadFun_TARGET_control(LPVOID lParam)
{
	THREAD_INFO_TARGET_control* Thread_Info = (THREAD_INFO_TARGET_control *)lParam;
	CLRF_controlDlg * hWnd = (CLRF_controlDlg *)CWnd::FromHandle((HWND)Thread_Info->hWnd);


	CString str_Value, str_Value2, str_Value3, str_Value4, str_Value5, str_Value6, str_Value7, str_Value8, str_Value9, str_Value10;
	CStatic * Static_num = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_roh);
	CStatic * Static_num2 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_theta);
	CStatic * Static_num3 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_angle);
	CStatic * Static_num4 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_alpha);
	CStatic * Static_num5 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_beta);
	CStatic * Static_num6 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_state);
	CStatic * Static_num7 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_VR);
	CStatic * Static_num8 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_VL);
	CStatic * Static_num9 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_speed);
	CStatic * Static_num10 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_time);

	LARGE_INTEGER tStart, tEnd, ts;

	target_pos[0] = 0;
	target_pos[1] = 1;
	target_pos[2] = PI/2;  //旋轉角

	int state = 1;
	int rho_gain;
	int times = 0;
	int past_time = 0;
	float M1, M2, N1, N2;
	float alpha_gain;
	float beta_gain;
	double rho = 0;
	double theta = 0;
	double alpha = 0;
	double beta = 0;
	double phi = 0;
	double car_past_x = 0, car_past_y = 0, car_speed2 = 0;
	sampleTime = 10; //毫秒

	remove("control_output.txt");
	remove("control_output_m.txt");
	remove("control_pos_m.txt");
	fstream app_control("control_output.txt", ios::app);
	fstream app_control_m("control_output_m.txt", ios::app); //為了給matlab畫圖用的
	fstream app_pos_m("control_pos_m.txt", ios::app);

	while (carFLAG)
	{
		QueryPerformanceFrequency(&ts);
		QueryPerformanceCounter(&tStart);

		car_x = CPSocket::m_pose_data_2.pose_position[0] - 0 /*+ (double)rand() / (RAND_MAX + 1.0) * 4*/;
		car_y = CPSocket::m_pose_data_2.pose_position[1] - 0 /*+ (double)rand() / (RAND_MAX + 1.0) * 4*/;
		car_zdir = CPSocket::m_pose_data_2.pose_orientation[2] + PI / 2 /*+ (double)rand() / (RAND_MAX + 1.0) * 6*/;


		phi = car_zdir;
		if (phi > PI)		phi = -2 * PI + phi;
		if (phi < -PI)		phi = 2 * PI + phi;


		rho = sqrt((car_x - target_pos[0])*(car_x - target_pos[0]) + (car_y - target_pos[1])*(car_y - target_pos[1]));
		theta = atan2(car_y - target_pos[1], car_x - target_pos[0]);


		alpha = -phi + theta + PI;
		if (alpha > PI)		alpha = -2 * PI + alpha;
		if (alpha < -PI)		alpha = 2 * PI + alpha;

		beta = -(theta + PI) + target_pos[2];
		if (beta > PI)		beta = -2 * PI + beta;
		if (beta < -PI)		beta = 2 * PI + beta;

		//----------------------第二模式新增之判斷------------------------------------------------------------
		float P2_1 = 1.1737;
		float P2_2 = 1.4317;
		float P2_3 = 0.2422;
		float P2_4 = 0.4223;

		float Vt2 = rho * rho * P2_1 +
			alpha*alpha*P2_2 +
			alpha*phi*P2_4 +
			alpha*phi*P2_4 +
			phi*phi*P2_3;

		//-----------------------------------------------------------------------------------------------------

#if 1

		//原始線性控制
// 		vr = 3 * rho + 0.15 * (8 * alpha - 4 * (beta));
// 		vl = 3 * rho - 0.15 * (8 * alpha - 4 * (beta));
// 		vr = vr / 10;
// 		vl = vl / 10;
#else
		//判斷要切換哪種模式TS-FUZZY
		if (times == 0 || alpha > (PI / 10) || alpha < (-PI / 10))
		{
			if (state == 3)
				state = 3;

			else if (state == 2)
				state = 2;

			else
				state = 1;
		}
		else if ((rho > 1 || Vt2 > 4))  //gamma = 2
		{
			if (state == 3)
				state = 3;
			else
				state = 2;
		}
		else
			state = 3;

		if (alpha < PI / 10 && alpha>-PI / 10)
			times = 1; //一開始必定Mode1，其餘只看角度來決定是否進入Mode1

					   //切換式TS-Fuzzy控制
		alpha_gain = 1.9161;

		switch (state)
		{
		case 1:  //旋轉Mode
				 //		alpha_gain = 10 * (alpha / pi);

			vr = 0 * rho + 0.15 * (alpha_gain*alpha - 0 * (beta));
			vl = 0 * rho - 0.15 * (alpha_gain*alpha - 0 * (beta));
			//			vr = vr *1.4;
			//			vl = vl *1.4;

			break;

		case 2:  //直線Mode
// 			rho_gain = (4 * rho / 200) + (1 - (rho / 200));
// 			if (rho > 150) rho_gain = 4;
			rho_gain = 1.0331;
			vr = rho_gain *rho + 0.15 * (alpha_gain*alpha - 0 * (beta));
			vl = rho_gain *rho - 0.15 * (alpha_gain*alpha - 0 * (beta));
			vr = vr*0.1;
			vl = vl*0.1;
			break;

		case 3:  //PDC Mode
//			rho_gain = (3 * rho / 125) + (1 - (rho / 125));
			rho_gain = 1.0331;
			M1 = (cos(alpha) - 0.031415926) / (1 - 0.031415926);
			M2 = 1 - M1;
			//		M2 = (1 - cos(alpha)) / (1 - 0.031415926);

			if (alpha == 0)
				N1 = 1;
			else
				N1 = (0.49*PI * sin(alpha) - sin(0.49*PI)*alpha) / (alpha*(0.49*PI - sin(0.49*PI)));

			N2 = 1 - N1;

			alpha_gain = M1*N1*5.8766 +
				M1*N2*5.6385 +
				M2*N1*5.8766 +
				M2*N2*5.6385;

			beta_gain = M1*N1*1.1052 +
				M1*N2*1.0776 +
				M2*N1*1.1052 +
				M2*N2*1.0776;

			// 			alpha_gain = M1*N1*1.2833 +
			// 				M1*N2*1.1022 +
			// 				M2*N1* 1.2833 +
			// 				M2*N2*1.1022;
			// 
			// 			beta_gain = -M1*N1*0.0487 +
			// 				-M1*N2*0.0517 +
			// 				-M2*N1*0.0487 +
			// 				-M2*N2*0.0517;

			vr = rho_gain * rho + 0.15 * (alpha_gain * alpha - beta_gain * (beta));
			vl = rho_gain * rho - 0.15 * (alpha_gain * alpha - beta_gain * (beta));
// 			vr = vr*0.3;
// 			vl = vl*0.3;
			break;

		default:
			break;
		}
#endif

		// 		if (vr > 10)
		// 			vr = 10;
		// 
		// 		if (vl > 10)
		// 			vl = 10;
		// 
		// 
		// 		if (vr < -10)
		// 			vr = -10;
		// 
		// 		if (vl < -10)
		// 			vl = -10;

		vr_draw = vr;
		vl_draw = vl;



// 		vr = vr * 4500;
// 		vl = vl * 4500;
// 
// 		if (vr > 0)
// 			vr = vr + 8100;
// 		else
// 			vr = vr - 8100;
// 
// 		if (vl > 0)
// 			vl = vl + 8100;
// 		else
// 			vl = vl - 8100;

		// 		if (state == 3)
		// 		{
		// 			if (vr > 0)
		// 				vr = vr -150;
		// 			else
		// 				vr = vr + 150;
		// 
		// 			if (vl > 0)
		// 				vl = vl - 150;
		// 			else
		// 				vl = vl + 150;
		// 		}


//		I90_Speed2PWM(vl_draw, vr_draw, vl, vr);
//		I90_PWM_send(vl, vr);

		str_Value.Format(_T("%f"), rho);
		Static_num->SetWindowText(str_Value);

		str_Value2.Format(_T("%f"), theta * 180 / PI);
		Static_num2->SetWindowText(str_Value2);

		str_Value3.Format(_T("%f"), phi * 180 / PI);
		Static_num3->SetWindowText(str_Value3);

		str_Value4.Format(_T("%f"), alpha * 180 / PI);
		Static_num4->SetWindowText(str_Value4);

		str_Value5.Format(_T("%f"), beta * 180 / PI);
		Static_num5->SetWindowText(str_Value5);

		str_Value6.Format(_T("%d"), state);
		Static_num6->SetWindowText(str_Value6);

		str_Value7.Format(_T("%f"), vr);
		Static_num7->SetWindowText(str_Value7);

		str_Value8.Format(_T("%f"), vl);
		Static_num8->SetWindowText(str_Value8);

		cout.flags(ios::left);

		// 		app_control
		// 			<< " |rho= " << setw(7) << setprecision(4) << rho << setw(6)
		// 			<< " |theta= " << setw(7) << setprecision(4) << theta << setw(6)
		// 			<< " |phi= " << setw(7) << setprecision(4) << phi << setw(6)
		// 			<< " |alpha= " << setw(10) << setprecision(4) << alpha << setw(6)
		// 			<< " |beta= " << setw(10) << setprecision(4) << beta << setw(4)
		// 			<< " |vl= " << setw(5) << setprecision(4) << vl << setw(4)
		// 			<< " |vr= " << setw(5) << setprecision(4) << vr
		// 			<< endl;


		Sleep(sampleTime);

		QueryPerformanceCounter(&tEnd);
		int m_time = ((tEnd.QuadPart - tStart.QuadPart) * 1000 / (double)(ts.QuadPart));
		past_time = past_time + m_time;

		double car_speed = sqrtf((car_x - car_past_x)*(car_x - car_past_x) + (car_y - car_past_y)*(car_y - car_past_y)) * 1000 / m_time;
		if (car_speed != 0)
		{
			car_speed2 = car_speed;
			str_Value9.Format(_T("%f"), car_speed);
			Static_num9->SetWindowText(str_Value9);

			car_past_x = car_x;
			car_past_y = car_y;
		}


		str_Value10.Format(_T("%d"), m_time);
		Static_num10->SetWindowText(str_Value10);

		app_control_m
			<< setw(15) << setprecision(4) << rho
			<< setw(15) << setprecision(4) << theta
			<< setw(15) << setprecision(4) << phi
			<< setw(15) << setprecision(4) << alpha
			<< setw(15) << setprecision(4) << beta
			<< setw(15) << setprecision(4) << vl_draw
			<< setw(15) << setprecision(4) << vr_draw
			<< setw(15) << setprecision(4) << state
			<< setw(15) << setprecision(4) << past_time
			<< setw(15) << setprecision(4) << car_speed2
			<< setw(15) << setprecision(4) << 9300 + upupup
			<< endl;

		app_pos_m
			<< car_zdir << "  "
			<< car_x << "  "
			<< car_y << "  "
			<< past_time << "  "
			<< endl;
	}




	*Thread_Info->Continue = false;
	::PostMessage(hWnd->m_hWnd, WNU_THREAD_EXIT, 0, 0);
	return(0);
}

UINT CLRF_controlDlg::ThreadFun_car_draw(LPVOID lParam)
{
	THREAD_INFO_car_draw* Thread_Info = (THREAD_INFO_car_draw *)lParam;
	CLRF_controlDlg * hWnd = (CLRF_controlDlg *)CWnd::FromHandle((HWND)Thread_Info->hWnd);

	CWnd* pWnd_IDC_Map = (CWnd *)hWnd->GetDlgItem(IDC_Path);
	CDC* dc = pWnd_IDC_Map->GetWindowDC();

	CPoint orgin;
	orgin.x = 350;
	orgin.y = 350;

	IplImage * draw_data = NULL;
	draw_data = cvCreateImage(cvSize(700, 700), IPL_DEPTH_8U, 3);

	int state = 1;
	int rho_gain;
	int times = 0;
	float M1, M2, N1, N2;
	float alpha_gain;
	float beta_gain;
	double rho = 0;
	double theta = 0;
	double alpha = 0;
	double beta = 0;
	double phi = 0;

	int carsize = 20;
	int speedLineSize = 50;
	int mapSize = 100;   //200
	CvPoint draw_oringin[2];
	CvPoint draw_car[6];
	CvPoint LR_speed[4];
	CvFont Font1 = cvFont(1, 2);
	vector <double> x_save, y_save;
	char xx[100];

	while (carFLAG)
	{
		// 		draw_oringin[0] = cvPoint(orgin.x + target_pos[0], orgin.y + target_pos[1]);
		// 		draw_oringin[1] = cvPoint(orgin.x + target_pos[0] + 30 * cos(target_pos[2]), 700 - (orgin.y + target_pos[1] + 30 * sin(target_pos[2])));

		draw_oringin[0] = cvPoint(orgin.x, orgin.y);
		draw_oringin[1] = cvPoint(orgin.x, 700 - orgin.y);

		draw_car[0] = cvPoint(car_x * mapSize + carsize * cos(car_zdir + 0.7854) + orgin.x, 700 - (car_y * mapSize + carsize * sin(car_zdir + 0.7854) + orgin.y));
		draw_car[1] = cvPoint(car_x * mapSize + carsize * cos(0.7854 - car_zdir) + orgin.x, 700 - (car_y * mapSize - carsize * sin(0.7854 - car_zdir) + orgin.y));
		draw_car[2] = cvPoint(car_x * mapSize - carsize * cos(car_zdir + 0.7854) + orgin.x, 700 - (car_y * mapSize - carsize * sin(car_zdir + 0.7854) + orgin.y));
		draw_car[3] = cvPoint(car_x * mapSize - carsize * cos(0.7854 - car_zdir) + orgin.x, 700 - (car_y * mapSize + carsize * sin(0.7854 - car_zdir) + orgin.y));
		draw_car[4] = cvPoint(car_x * mapSize + orgin.x, 700 - (car_y * mapSize + orgin.y));
		draw_car[5] = cvPoint(car_x * mapSize + 40 * cos(car_zdir) + orgin.x, 700 - (car_y * mapSize + 40 * sin(car_zdir) + orgin.y));



		cvLine(draw_data, cvPoint(0, 350), cvPoint(700, 350), CV_RGB(0, 0, 255), 2);
		cvLine(draw_data, cvPoint(350, 0), cvPoint(350, 700), CV_RGB(0, 0, 255), 2);
		cvLine(draw_data, cvPoint(orgin.x, orgin.y), cvPoint(orgin.x, orgin.y), CV_RGB(255, 0, 255), 5);

		cvLine(draw_data, draw_car[0], draw_car[1], CV_RGB(0, 255, 0), 4);
		cvLine(draw_data, draw_car[1], draw_car[2], CV_RGB(0, 255, 0), 4);
		cvLine(draw_data, draw_car[2], draw_car[3], CV_RGB(0, 255, 0), 4);
		cvLine(draw_data, draw_car[3], draw_car[0], CV_RGB(0, 255, 0), 4);
		cvLine(draw_data, draw_car[4], draw_car[5], CV_RGB(255, 255, 0), 2);
		cvLine(draw_data, draw_oringin[0], draw_oringin[1], CV_RGB(255, 100, 255), 2);



		if (car_x > 0 && car_y > 0)
		{
			sprintf_s(xx, "(%d, %d, %d)", ((int)draw_car[4].x - 350) * 100 / mapSize, (350 - (int)draw_car[4].y) * 100 / mapSize, (int)(car_zdir * 180 / PI));
			cvPutText(draw_data, xx, cvPoint(draw_car[4].x + 20, draw_car[4].y - 40), &Font1, CV_RGB(255, 255, 255));
		}
		if (car_x < 0 && car_y > 0)
		{
			sprintf_s(xx, "(%d, %d, %d)", ((int)draw_car[4].x - 350) * 100 / mapSize, (350 - (int)draw_car[4].y) * 100 / mapSize, (int)(car_zdir * 180 / PI));
			cvPutText(draw_data, xx, cvPoint(draw_car[4].x - 120, draw_car[4].y - 40), &Font1, CV_RGB(255, 255, 255));
		}
		if (car_x < 0 && car_y < 0)
		{
			sprintf_s(xx, "(%d, %d, %d)", ((int)draw_car[4].x - 350) * 100 / mapSize, (350 - (int)draw_car[4].y) * 100 / mapSize, (int)(car_zdir * 180 / PI));
			cvPutText(draw_data, xx, cvPoint(draw_car[4].x - 120, draw_car[4].y + 40), &Font1, CV_RGB(255, 255, 255));
		}
		if (car_x > 0 && car_y < 0)
		{
			sprintf_s(xx, "(%d, %d, %d)", ((int)draw_car[4].x - 350) * 100 / mapSize, (350 - (int)draw_car[4].y) * 100 / mapSize, (int)(car_zdir * 180 / PI));
			cvPutText(draw_data, xx, cvPoint(draw_car[4].x + 20, draw_car[4].y + 40), &Font1, CV_RGB(255, 255, 255));
		}


		LR_speed[0] = cvPoint(draw_car[4].x + 25 * cos(PI / 2 - car_zdir), draw_car[4].y + 25 * sin(PI / 2 - car_zdir));
		LR_speed[1] = cvPoint(LR_speed[0].x + vr_draw*speedLineSize*cos(car_zdir), LR_speed[0].y - vr_draw*speedLineSize*sin(car_zdir));
		LR_speed[2] = cvPoint(draw_car[4].x - 25 * cos(PI / 2 - car_zdir), draw_car[4].y - 25 * sin(PI / 2 - car_zdir));
		LR_speed[3] = cvPoint(LR_speed[2].x + vl_draw*speedLineSize*cos(car_zdir), LR_speed[2].y - vl_draw*speedLineSize*sin(car_zdir));

		if (vr > 0)
			cvLine(draw_data, LR_speed[0], LR_speed[1], CV_RGB(255, 100, 100), 2);
		else
			cvLine(draw_data, LR_speed[0], LR_speed[1], CV_RGB(100, 255, 255), 2);

		if (vl > 0)
			cvLine(draw_data, LR_speed[2], LR_speed[3], CV_RGB(255, 100, 100), 2);
		else
			cvLine(draw_data, LR_speed[2], LR_speed[3], CV_RGB(100, 255, 255), 2);

		phi = car_zdir;
		if (phi > PI)		phi = -2 * PI + phi;
		if (phi < -PI)		phi = 2 * PI + phi;

		rho = sqrt((car_x - target_pos[0])*(car_x - target_pos[0]) + (car_y - target_pos[1])*(car_y - target_pos[1]));
		theta = atan2(car_y - target_pos[1], car_x - target_pos[0]);

		alpha = -phi + theta + PI;
		if (alpha > PI)		alpha = -2 * PI + alpha;
		if (alpha < -PI)		alpha = 2 * PI + alpha;

		beta = -(theta + PI) + target_pos[2];
		if (beta > PI)		beta = -2 * PI + beta;
		if (beta < -PI)		beta = 2 * PI + beta;

		int jump_draw = 0;
		double vr_here, vl_here;
		// 		vr_here = vr;
		// 		vl_here = vl;
		state = 1;
		times = 0;

		while (1)
		{
			//原始線性控制
			double rho_dot, alpha_dot, beta_dot;
			double x_here, y_here, zdir_here, theta_here;
			double u1, u2;

			//----------------------第二模式新增之判斷------------------------------------------------------------
			float P2_1 = 1.1737;
			float P2_2 = 1.4317;
			float P2_3 = 0.2422;
			float P2_4 = 0.4223;

			float Vt2 = rho * rho * P2_1 +
				alpha*alpha*P2_2 +
				alpha*phi*P2_4 +
				alpha*phi*P2_4 +
				phi*phi*P2_3;

			//-----------------------------------------------------------------------------------------------------

#if 1

			//原始線性控制
			vr_here = 3 * rho + 0.15 * (8 * alpha - 4 * (beta));
			vl_here = 3 * rho - 0.15 * (8 * alpha - 4 * (beta));

			//			vr_here = vr_here / 10;
			//			vl_here = vl_here / 10;
#else
			//判斷要切換哪種模式TS-FUZZY
			if (times == 0 || alpha > (PI / 10) || alpha < (-PI / 10))
			{
				if (state == 3)
					state = 3;

				else if (state == 2)
					state = 2;

				else
					state = 1;
			}
			else if ((rho > 1 || Vt2 > 4))  //gamma = 2
			{
				if (state == 3)
					state = 3;
				else
					state = 2;
			}
			else
				state = 3;

			if (alpha < PI / 10 && alpha>-PI / 10)
				times = 1; //一開始必定Mode1，其餘只看角度來決定是否進入Mode1

						   //切換式TS-Fuzzy控制
			alpha_gain = 1.9161;

			switch (state)
			{
			case 1:  //旋轉Mode
					 //		alpha_gain = 10 * (alpha / pi);

				vr_here = 0 * rho + 0.15 * (alpha_gain*alpha - 0 * (beta));
				vl_here = 0 * rho - 0.15 * (alpha_gain*alpha - 0 * (beta));
				//				vr_here = vr_here*1.4;
				//				vl_here = vl_here*1.4;
				break;

			case 2:  //直線Mode
					 // 			rho_gain = (4 * rho / 200) + (1 - (rho / 200));
					 // 			if (rho > 150) rho_gain = 4;
				rho_gain = 1.0331;
				vr_here = rho_gain *rho + 0.15 * (alpha_gain*alpha - 0 * (beta));
				vl_here = rho_gain *rho - 0.15 * (alpha_gain*alpha - 0 * (beta));
				//				vr_here = vr_here*0.3;
				//				vl_here = vl_here*0.3;
				break;

			case 3:  //PDC Mode
					 //			rho_gain = (3 * rho / 125) + (1 - (rho / 125));
				rho_gain = 1.0331;
				M1 = (cos(alpha) - 0.031415926) / (1 - 0.031415926);
				M2 = 1 - M1;
				//		M2 = (1 - cos(alpha)) / (1 - 0.031415926);

				if (alpha == 0)
					N1 = 1;
				else
					N1 = (0.49*PI * sin(alpha) - sin(0.49*PI)*alpha) / (alpha*(0.49*PI - sin(0.49*PI)));

				N2 = 1 - N1;

				alpha_gain = M1*N1*5.8766 +
					M1*N2*5.6385 +
					M2*N1*5.8766 +
					M2*N2*5.6385;

				beta_gain = M1*N1*1.1052 +
					M1*N2*1.0776 +
					M2*N1*1.1052 +
					M2*N2*1.0776;

				// 				alpha_gain = M1*N1*1.2833 +
				// 					M1*N2*1.1022 +
				// 					M2*N1* 1.2833 +
				// 					M2*N2*1.1022;
				// 
				// 				beta_gain = -M1*N1*0.0487 +
				// 					-M1*N2*0.0517 +
				// 					-M2*N1*0.0487 +
				// 					-M2*N2*0.0517;

				// 				beta_gain = -M1*N1*1.2833 +
				// 					-M1*N2*1.1022 +
				// 					-M2*N1* 1.2833 +
				// 					-M2*N2*1.1022;
				// 
				// 
				// 				alpha_gain = M1*N1*0.0487 +
				// 					M1*N2*0.0517 +
				// 					M2*N1*0.0487 +
				// 					M2*N2*0.0517;

				vr_here = rho_gain * rho + 0.15 * (alpha_gain * alpha - beta_gain * (beta));
				vl_here = rho_gain * rho - 0.15 * (alpha_gain * alpha - beta_gain * (beta));
				vr_here = vr_here*0.3;
				vl_here = vl_here*0.3;
				break;

			default:
				break;
			}
#endif

			u1 = ((vr_here + vl_here) / 2);
			u2 = ((vr_here - vl_here) / 0.3);

			rho_dot = u1*(-cos(alpha));
			alpha_dot = (sin(alpha) / rho)*u1 - u2;
			beta_dot = -(sin(alpha) / rho)*u1;

			rho = rho + rho_dot * (float)sampleTime / 1000;
			alpha = alpha + alpha_dot * (float)sampleTime / 1000;
			if (alpha > PI)		alpha = -2 * PI + alpha;
			if (alpha < -PI)		alpha = 2 * PI + alpha;

			beta = beta + beta_dot * (float)sampleTime / 1000;
			if (beta > PI)		beta = -2 * PI + beta;
			if (beta < -PI)		beta = 2 * PI + beta;

			theta_here = -PI - beta + target_pos[2];  //加入 target_pos[2]
			if (theta_here > PI)		theta_here = -2 * PI + theta_here;
			if (theta_here < -PI)		theta_here = 2 * PI + theta_here;


			x_here = cos(theta_here) * rho + target_pos[0];
			y_here = sin(theta_here) * rho + target_pos[1];
			zdir_here = -beta - alpha + target_pos[2];  //加入 target_pos[2]
			jump_draw++;
			x_save.push_back(x_here* mapSize + orgin.x);
			y_save.push_back(700 - (y_here * mapSize + orgin.y));


			if (jump_draw % 10 == 0)
			{
				draw_car[0] = cvPoint(x_here * mapSize + carsize * cos(zdir_here + 0.7854) + orgin.x, 700 - (y_here * mapSize + carsize * sin(zdir_here + 0.7854) + orgin.y));
				draw_car[1] = cvPoint(x_here * mapSize + carsize * cos(0.7854 - zdir_here) + orgin.x, 700 - (y_here * mapSize - carsize * sin(0.7854 - zdir_here) + orgin.y));
				draw_car[2] = cvPoint(x_here * mapSize - carsize * cos(zdir_here + 0.7854) + orgin.x, 700 - (y_here * mapSize - carsize * sin(zdir_here + 0.7854) + orgin.y));
				draw_car[3] = cvPoint(x_here * mapSize - carsize * cos(0.7854 - zdir_here) + orgin.x, 700 - (y_here * mapSize + carsize * sin(0.7854 - zdir_here) + orgin.y));
				draw_car[4] = cvPoint(x_here * mapSize + orgin.x, 700 - (y_here * mapSize + orgin.y));
				draw_car[5] = cvPoint(x_here * mapSize + 40 * cos(zdir_here) + orgin.x, 700 - (y_here * mapSize + 40 * sin(zdir_here) + orgin.y));

				cvLine(draw_data, cvPoint(0, 350), cvPoint(700, 350), CV_RGB(0, 0, 255), 2);
				cvLine(draw_data, cvPoint(350, 0), cvPoint(350, 700), CV_RGB(0, 0, 255), 2);
				cvLine(draw_data, cvPoint(orgin.x, orgin.y), cvPoint(orgin.x, orgin.y), CV_RGB(255, 0, 255), 5);

				cvLine(draw_data, draw_car[0], draw_car[1], CV_RGB(0, 255, 0), 2);
				cvLine(draw_data, draw_car[1], draw_car[2], CV_RGB(0, 255, 0), 2);
				cvLine(draw_data, draw_car[2], draw_car[3], CV_RGB(0, 255, 0), 2);
				cvLine(draw_data, draw_car[3], draw_car[0], CV_RGB(0, 255, 0), 2);
				cvLine(draw_data, draw_car[4], draw_car[5], CV_RGB(255, 0, 0), 2);
				cvLine(draw_data, draw_oringin[0], draw_oringin[1], CV_RGB(255, 100, 255), 2);
			}



			if ((abs(x_here - target_pos[0]) < 0.001  && abs(y_here - target_pos[1]) < 0.001) || jump_draw > 600 /*|| (abs(zdir_here - target_pos[2]) < 0.2)*/)
				break;

		}

		/*
				for (;;)
				{
					double rho_dot, alpha_dot, beat_dot;
					double x_here, y_here, zdir_here, theta_here;



					rho_dot = 0.25*rho*(-cos(alpha));
					alpha_dot = (sin(alpha) / rho)*0.25*rho - alpha*0.75 + beta * -0.075;
					beat_dot = (-sin(alpha) / rho)*0.25*rho;


					rho = rho + rho_dot * (float)sampleTime / 1000;
					alpha = alpha + alpha_dot * (float)sampleTime / 1000;
					if (alpha > PI)		alpha = -2 * PI + alpha;
					if (alpha < -PI)		alpha = 2 * PI + alpha;

					beta = beta + beat_dot * (float)sampleTime / 1000;
					if (beta > PI)		beta = -2 * PI + beta;
					if (beta < -PI)		beta = 2 * PI + beta;

					theta_here = -PI - beta;
					if (theta_here > PI)		theta_here = -2 * PI + theta_here;
					if (theta_here < -PI)		theta_here = 2 * PI + theta_here;


					x_here = cos(theta_here) * rho;
					y_here = sin(theta_here) * rho;
					zdir_here = -beta - alpha;
					jump_draw++;

					if (jump_draw % 15 == 0)
					{
						draw_car[0] = cvPoint(x_here * mapSize + carsize * cos(zdir_here + 0.7854) + orgin.x, 700 - (y_here * mapSize + carsize * sin(zdir_here + 0.7854) + orgin.y));
						draw_car[1] = cvPoint(x_here * mapSize + carsize * cos(0.7854 - zdir_here) + orgin.x, 700 - (y_here * mapSize - carsize * sin(0.7854 - zdir_here) + orgin.y));
						draw_car[2] = cvPoint(x_here * mapSize - carsize * cos(zdir_here + 0.7854) + orgin.x, 700 - (y_here * mapSize - carsize * sin(zdir_here + 0.7854) + orgin.y));
						draw_car[3] = cvPoint(x_here * mapSize - carsize * cos(0.7854 - zdir_here) + orgin.x, 700 - (y_here * mapSize + carsize * sin(0.7854 - zdir_here) + orgin.y));
						draw_car[4] = cvPoint(x_here * mapSize + orgin.x, 700 - (y_here * mapSize + orgin.y));
						draw_car[5] = cvPoint(x_here * mapSize + 40 * cos(zdir_here) + orgin.x, 700 - (y_here * mapSize + 40 * sin(zdir_here) + orgin.y));

						cvLine(draw_data, cvPoint(0, 350), cvPoint(700, 350), CV_RGB(0, 0, 255), 2);
						cvLine(draw_data, cvPoint(350, 0), cvPoint(350, 700), CV_RGB(0, 0, 255), 2);
						cvLine(draw_data, cvPoint(orgin.x, orgin.y), cvPoint(orgin.x, orgin.y), CV_RGB(255, 0, 255), 5);

						cvLine(draw_data, draw_car[0], draw_car[1], CV_RGB(0, 255, 0), 4);
						cvLine(draw_data, draw_car[1], draw_car[2], CV_RGB(0, 255, 0), 4);
						cvLine(draw_data, draw_car[2], draw_car[3], CV_RGB(0, 255, 0), 4);
						cvLine(draw_data, draw_car[3], draw_car[0], CV_RGB(0, 255, 0), 4);
						cvLine(draw_data, draw_car[4], draw_car[5], CV_RGB(255, 255, 0), 2);
						cvLine(draw_data, draw_oringin[0], draw_oringin[1], CV_RGB(255, 100, 255), 2);
					}

					if (abs(x_here - target_pos[0]) < 0.02 / *&& abs(zdir_here - target_pos[2]) < 1* /)
						break;

				}*/

		for (int i = 0; i < x_save.size() - 1; i++)
		{
			cvLine(draw_data, cvPoint((int)x_save[i], (int)y_save[i]), cvPoint((int)x_save[i + 1], (int)y_save[i + 1]), CV_RGB(150, 150, 255), 2);
		}


		x_save.clear();
		y_save.clear();



		CvvImage show1;
		show1.CopyOf(draw_data);
		show1.Show(*dc, 0, 0, draw_data->width, draw_data->height);
		//		cvSaveImage("CAR4.png", draw_data);
		memset((unsigned char*)draw_data->imageData, 0, draw_data->imageSize);

		Sleep(20);
	}




	*Thread_Info->Continue = false;
	::PostMessage(hWnd->m_hWnd, WNU_THREAD_EXIT, 0, 0);
	return(0);
}


void CLRF_controlDlg::I90_PWM_send(int L_PWM, int R_PWM)
{
	int L, R, mid = 16384; //0~32767

// 	if (R_PWM > 0)
// 		R_PWM = R_PWM + c_Synchronous;
// 
// 	if (R_PWM < 0)
// 	{
// 		L_PWM = L_PWM - c_Synchronous;
// 		R_PWM = R_PWM;
// 	}

	//L.R.轉換
	L = mid + L_PWM;
	R = mid - R_PWM;
	I90_PWM_control[7] = R;
	I90_PWM_control[8] = R >> 8;
	I90_PWM_control[10] = L;
	I90_PWM_control[11] = L >> 8;
	I90_PWM_control[(I90_PWM_control[5] + 6)] = checksun(2, (I90_PWM_control[5] + 5));


	//I90rs232.Open(7,115200);
	Connect_I90.SendData(I90_PWM_control, sizeof(I90_PWM_control));
}

void CLRF_controlDlg::I90_Speed2PWM(double i_L_Speed, double i_R_Speed, double &o_L_PWM, double &o_R_PWM)
{
	//二次曲線逼近(左輪)，y = -1E-07x2 - 7E-06x + 0.8064
	//三次曲線逼近(左輪)，y = -3E-11x3 + 4E-08x2 - 0.0002x + 0.8417
	//四次曲線逼近(左輪)，y = 6E-14x4 - 4E-10x3 + 7E-07x2 - 0.0006x + 0.896
	//五次曲線逼近(左輪)，y = 9E-17x5 - 5E-13x4 + 1E-09x3 - 9E-07x2 + 7E-05x + 0.8381

	//二次曲線逼近(右輪)，y = -1E-07x2 - 1E-04x + 0.7581
	//三次曲線逼近(右輪)，y = 6E-11x3 - 3E-07x2 + 8E-05x + 0.7249
	//四次曲線逼近(右輪)，y = 1E-13x4 - 6E-10x3 + 7E-07x2 - 0.0004x + 0.781
	//五次曲線逼近(右輪)，y = 5E-17x5 - 1E-13x4 - 7E-11x3 + 2E-07x2 - 0.0003x + 0.7697

	double temp_L, temp_L2 = 1, temp_R, temp_R2 = 1, i;
	double index_L, index_R;


	for (i = 0; i < 2774; i++)
	{
		if (i_L_Speed > 0)
		{
//			temp_L = (-2E-07*pow(i, 2) - 1E-05*i + 1.4127);
// 			temp_L = (-3E-11*pow(i, 3) + 4E-08*pow(i, 2) - 0.0002*i + 0.8417)*2;
			temp_L =( 6E-12*pow(i, 4) - 1E-8*pow(i, 3) + 9E-06*pow(i, 2) - 0.0028*i + 0.7674);
		}
		else
		{
//			temp_L = -(-2E-07*pow(i, 2) - 1E-05*i + 1.4127);
// 			temp_L = -(-3E-11*pow(i, 3) + 4E-08*pow(i, 2) - 0.0002*i + 0.8417)*2;
 			temp_L =-(6E-12*pow(i, 4) - 1E-8*pow(i, 3) + 9E-06*pow(i, 2) - 0.0028*i + 0.7674);
		}

		if (i_R_Speed > 0)
		{
//			temp_R = (-2E-07*pow(i, 2) - 2E-04*i + 1.5161);
// 			temp_R = (6E-11*pow(i, 3) - 3E-07*pow(i, 2) + 8E-05*i + 0.7249)*2;
 			temp_R = (2E-12*pow(i, 4) + 2E-9*pow(i, 3) + 1E-06*pow(i, 2) - 0.00016*i + 0.7757);
		}
		else
		{
//			temp_R = (-(-2E-07*pow(i, 2) - 2E-04*i + 1.5161));
// 			temp_R = -(6E-11*pow(i, 3) - 3E-07*pow(i, 2) + 8E-05*i + 0.7249)*2;
 			temp_R = -(2E-12*pow(i, 4) + 2E-9*pow(i, 3) + 1E-06*pow(i, 2) - 0.00016*i + 0.7757);
		}

	

		if (abs(temp_L - i_L_Speed) < temp_L2)
		{
			temp_L2 = abs(temp_L - i_L_Speed);
			index_L = i;
		}

		if (abs(temp_R - i_R_Speed) < temp_R2)
		{
			temp_R2 = abs(temp_R - i_R_Speed);
			index_R = i;
		}
	}

	//二次曲線逼近(左輪PWM)，y = -0.0002x + 1.2915
	//二次曲線逼近(右輪PWM)，y = -0.0002x + 1.292
	if (i_L_Speed > 0)
		o_L_PWM = (-0.0002*index_L + 0.926) * 10000;
	else
		o_L_PWM = -((-0.0002*index_L + 0.926) * 10000);

	if (i_R_Speed > 0)
		o_R_PWM = (-0.0002*index_R + 0.9348) * 10000;
	else
		o_R_PWM = -((-0.0002*index_R + 0.9348) * 10000);

}


unsigned char CLRF_controlDlg::checksun(int nStar, int nEnd)
{
	unsigned char shift_reg, sr_lsb, data_bit, v, fb_bit;
	int i, j;

	shift_reg = 0;
	for (i = nStar; i <= (nEnd); i++)
	{
		v = (unsigned char)(I90_PWM_control[i] & 0x0000ffff);
		for (j = 0; j < 8; j++)
		{
			data_bit = v & 0x01;  // isolate least sign bit
			sr_lsb = shift_reg & 0x01;
			fb_bit = (data_bit ^ sr_lsb) & 0x01;  //calculate the feed back bit
			shift_reg = shift_reg >> 1;
			if (fb_bit == 1)
			{
				shift_reg = shift_reg ^ 0x8c;
			}
			v = v >> 1;
		}
	}
	return shift_reg;
}



void CPSocket::OnReceive(int nErrorCode) {
	static int counter(0);
	counter++;

	if (0 == nErrorCode) {

		static int i = 0;
		i++;

		int nRead(0);

		volatile int cbLeft(sizeof(PoseData)); // 4 Byte
		volatile int cbDataReceived(0);
		int cTimesRead(0);
		do {


			// Determine Socket State
			nRead = Receive(&m_pose_data + cbDataReceived, cbLeft);

			if (nRead == SOCKET_ERROR) {
				if (GetLastError() != WSAEWOULDBLOCK) {
					//AfxMessageBox(_T("Error occurred"));
					Close();
					// Trying to reconnect
					((CLRF_controlDlg*)m_parent)->DoPoseSocketConnect();
					return;
				}
				break;
			}


			cbLeft -= nRead;
			cbDataReceived += nRead;
			cTimesRead++;
		} while (cbLeft > 0 && cTimesRead < 50);
	}

	m_pose_data_2 = m_pose_data;

	CSocket::OnReceive(nErrorCode);
}

void CPSocket::registerParent(CWnd* _parent) {
	m_parent = _parent;
}


void CLRF_controlDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值

	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CLRF_controlDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此加入特定的程式碼和 (或) 呼叫基底類別

	double Lspeed1 = 0.8 + upupup, Rspeed1 = 0.8 + upupup;
	double Lspeed2 = -0.8 - upupup, Rspeed2 = -0.8 - upupup;
	double Lspeed3 = -0.8 - upupup, Rspeed3 = 0.8 + upupup;
	double Lspeed4 = 0.8 + upupup, Rspeed4 = -0.8 - upupup;
	double Lspeed5 = 0, Rspeed5 = 0.8 + upupup;
	double Lspeed6 = 0.8 + upupup, Rspeed6 = 0;

	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case 0x57:
			I90_Speed2PWM(Lspeed1, Rspeed1, Lspeed1, Rspeed1);
			I90_PWM_send(Lspeed1, Rspeed1); 
			vl = Lspeed1;
			vr = Rspeed1;
			break;
		case 0x53:
			I90_Speed2PWM(Lspeed2, Rspeed2, Lspeed2, Rspeed2);
			I90_PWM_send(Lspeed2, Rspeed2); 
			vl = Lspeed2;
			vr = Rspeed2; 
			break;
		case 0x41:
			I90_Speed2PWM(Lspeed3, Rspeed3, Lspeed3, Rspeed3);
			I90_PWM_send(Lspeed3, Rspeed3); break;
		case 0x44:
			I90_Speed2PWM(Lspeed4, Rspeed4, Lspeed4, Rspeed4);
			I90_PWM_send(Lspeed4, Rspeed4); break;
		case 0x51:
			I90_Speed2PWM(Lspeed5, Rspeed5, Lspeed5, Rspeed5);
			I90_PWM_send(Lspeed5, Rspeed5); 
			upupup = upupup + 0.1;
			break;
		case 0x45:
			I90_Speed2PWM(Lspeed6, Rspeed1, Lspeed6, Rspeed6);
			I90_PWM_send(Lspeed6, Rspeed6); 
			upupup = upupup + 0.1;
			break;
		case VK_F2:
			I90_PWM_send(0, 0); break;
		default:
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

