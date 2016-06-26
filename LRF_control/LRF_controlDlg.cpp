
// LRF_controlDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "LRF_control.h"
#include "LRF_controlDlg.h"
#include "afxdialogex.h"
#include "RSocket.h"
#include "CvvImage.cpp"  
#include "Serial.cpp"
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
double CRSocket::m_pose_data_static[SIZE_POSE_DATA] = { 0 };
CSerial Connect_I90;

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
int c_Synchronous = 220;
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

CRSocket::CRSocket() {}

CRSocket::~CRSocket() {}

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
	m_socket.registerParent(this);
	m_socket_ip_c.SetAddress(192, 168, 1, 210);
	m_socket_port = 25650;


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
	if (fg_connected) {
		DoSocketDisconnect();
	}
	else {
		DoSocketConnect();
	}
	m_socket_connect_c.SetCheck(fg_connected);

}

void CLRF_controlDlg::DoSocketConnect() {

	// Sync the Panel data to the model
	UpdateData(TRUE);

	// Read the TCP/IP address setting from User
	byte aIpAddressUnit[4];
	m_socket_ip_c.GetAddress(
		aIpAddressUnit[0], aIpAddressUnit[1], aIpAddressUnit[2], aIpAddressUnit[3]);
	CString aStrIpAddress;
	aStrIpAddress.Format(_T("%d.%d.%d.%d"),
		aIpAddressUnit[0], aIpAddressUnit[1], aIpAddressUnit[2], aIpAddressUnit[3]);

	// Create a TCP Socket for transfer Camera data
	// m_tcp_socket.Create(m_tcp_ip_port, 1, aStrIpAddress);
	if (!m_socket.Create()) {

		// If Socket Create Fail Report Message
		TCHAR szMsg[1024] = { 0 };
		wsprintf(szMsg, _T("create faild: %d"), m_socket.GetLastError());
		ReportSocketStatus(TCPEvent::CREATE_SOCKET_FAIL);
		AfxMessageBox(szMsg);
	}
	else {

		ReportSocketStatus(TCPEvent::CREATE_SOCKET_SUCCESSFUL);
		// Connect to the Server ( Raspberry Pi Server )
		fg_connected = m_socket.Connect(aStrIpAddress, m_socket_port);

		//fg_tcp_ip_read = true;
		//m_tcp_ip_IOHandle_thread = AfxBeginThread(TcpIODataHandler, LPVOID(this));

		//For Test
		m_socket.Send("Test from here", 14);
	}

	if (fg_connected)
		ReportSocketStatus(TCPEvent::CONNECT_SUCCESSFUL, aStrIpAddress);
	else
		ReportSocketStatus(TCPEvent::CONNECT_FAIL, aStrIpAddress);



}

void CLRF_controlDlg::DoSocketDisconnect() {
	// Setup Connect-staus flag
	fg_connected = false;
	//fg_tcp_ip_read = false;

	// Close the TCP/IP Socket
	m_socket.Close();

	// Report TCP/IP connect status
	CString tmp_log; tmp_log.Format(_T("I/O event: %s"), _T("Close Socket"));
	m_socket_log_c.AddString(tmp_log);
}

void CLRF_controlDlg::ReportSocketStatus(TCPEvent event_, CString &msg) {
	CString tmp_log;
	switch (event_) {
	case CREATE_SOCKET_SUCCESSFUL:
		tmp_log.Format(_T("I/O event: %s"),
			_T("Create Socket Successful"));
		break;
	case CREATE_SOCKET_FAIL:
		tmp_log.Format(_T("I/O event: %s"),
			_T("Create Socket Fail"));
		break;
	case CONNECT_SUCCESSFUL:
		tmp_log.Format(_T("I/O event: %s%s%s"),
			_T("Connect "), msg, _T(" Successful"));
		break;
	case CONNECT_FAIL:
		tmp_log.Format(_T("I/O event: %s%s%s"),
			_T("Connect "), msg, _T(" Fail"));
		break;
	case DISCONNECT:
		tmp_log.Format(_T("I/O event: %s"),
			_T("Disconnect"));
		break;
	case SEND_MESSAGE_SUCCESSFUL:
		tmp_log.Format(_T("I/O event: %s%s%s"),
			_T("Sent Message"), msg, _T("Successful"));
		break;
	case SENT_MESSAGE_FAIL:
		tmp_log.Format(_T("I/O event: %s%s%s"),
			_T("Sent Message"), msg, _T("Fail"));
		break;
	}
	m_socket_log_c.AddString(tmp_log);
}

template<size_t LENGTH> void CLRF_controlDlg::SendSocketMessage(char(&data)[LENGTH]) {

	// Send Message
	if (fg_tcp_ip_connected) {
		m_tcp_socket.Send(data, LENGTH);
		ReportTCPStatus(TCPEvent::SEND_MESSAGE_SUCCESSFUL, CString(data));
	}
}

void CLRF_controlDlg::OnBnClickedconnect()
{
	Connect_I90.Open(2, 115200);
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

UINT CLRF_controlDlg::ThreadFun_TARGET_control(LPVOID lParam)
{
	THREAD_INFO_TARGET_control* Thread_Info = (THREAD_INFO_TARGET_control *)lParam;
	CLRF_controlDlg * hWnd = (CLRF_controlDlg *)CWnd::FromHandle((HWND)Thread_Info->hWnd);


	CString str_Value, str_Value2, str_Value3, str_Value4, str_Value5, str_Value6, str_Value7, str_Value8;
	CStatic * Static_num = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_roh);
	CStatic * Static_num2 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_theta);
	CStatic * Static_num3 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_angle);
	CStatic * Static_num4 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_alpha);
	CStatic * Static_num5 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_beta);
	CStatic * Static_num6 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_state);
	CStatic * Static_num7 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_VR);
	CStatic * Static_num8 = (CStatic *)hWnd->GetDlgItem(IDC_STATIC_VL);

	target_pos[0] = 1;
	target_pos[1] = 1;
	target_pos[2] = PI / 2;  //旋轉角

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
	sampleTime = 50; //毫秒

	remove("control_output.txt");
	remove("control_output_m.txt");
	remove("control_pos_m.txt");
	fstream app_control("control_output.txt", ios::app);
	fstream app_control_m("control_output_m.txt", ios::app); //為了給matlab畫圖用的
	fstream app_pos_m("control_pos_m.txt", ios::app);

	while (carFLAG)
	{
		car_x = CRSocket::m_pose_data_static[0] - 1 /*+ (double)rand() / (RAND_MAX + 1.0) * 4*/;
		car_y = CRSocket::m_pose_data_static[1] - 1 /*+ (double)rand() / (RAND_MAX + 1.0) * 4*/;
		car_zdir = CRSocket::m_pose_data_static[2] + 0 /*+ (double)rand() / (RAND_MAX + 1.0) * 6*/;


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
		vr = 3 * rho + 0.15 * (8*alpha - 4*(beta));
		vl = 3 * rho - 0.15 * (8*alpha - 4*(beta));
		vr = vr / 10;
		vl = vl / 10;
#else
		//判斷要切換哪種模式TS-FUZZY
		if (times == 0 || alpha > (PI / 10) || alpha < (-PI / 10))
		{
			if (state == 3)
				state = 3;
			
// 			else if (state == 2)
// 			state = 2;
			
			else
				state = 1;
		}
		else if ((rho > 0.6 || Vt2 > 4))  //gamma = 2
		{
			// 			if (state == 3)
			// 				state = 3;
			// 			else
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
			vr = vr *1.4;
			vl = vl *1.4;

			break;

		case 2:  //直線Mode
// 			rho_gain = (4 * rho / 200) + (1 - (rho / 200));
// 			if (rho > 150) rho_gain = 4;
			rho_gain = 1.0331;
			vr = rho_gain *rho + 0.15 * (alpha_gain*alpha - 0 * (beta));
			vl = rho_gain *rho - 0.15 * (alpha_gain*alpha - 0 * (beta));
			vr = vr*0.4;
			vl = vl*0.4;
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

			// 		alpha_gain = M1*N1*5.8766 +
			// 			                  M1*N2*5.6385 +
			// 			                  M2*N1*5.8766 +
			// 			                  M2*N2*5.6385;
			// 
			// 		beta_gain = M1*N1*1.1052 +
			// 			                M1*N2*1.0776 +
			// 			                M2*N1*1.1052 +
			// 			                M2*N2*1.0776;

			alpha_gain = M1*N1*1.2833 +
				M1*N2*1.1022 +
				M2*N1* 1.2833 +
				M2*N2*1.1022;

			beta_gain = -M1*N1*0.0487 +
				-M1*N2*0.0517 +
				-M2*N1*0.0487 +
				-M2*N2*0.0517;

			vr = rho_gain * rho + 0.15 * (alpha_gain * alpha - beta_gain * (beta));
			vl = rho_gain * rho - 0.15 * (alpha_gain * alpha - beta_gain * (beta));
			vr = vr*0.4;
			vl = vl*0.4;
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

		app_control_m
			<< setw(15) << setprecision(4) << rho
			<< setw(15) << setprecision(4) << theta
			<< setw(15) << setprecision(4) << phi
			<< setw(15) << setprecision(4) << alpha
			<< setw(15) << setprecision(4) << beta
			<< setw(15) << setprecision(4) << vl
			<< setw(15) << setprecision(4) << vr
			<< setw(15) << setprecision(4) << state
			<< endl;

		app_pos_m
			<< car_zdir << "  "
			<< car_x << "  "
			<< car_y << "  "
			<< endl;


		vr = vr * 3400;
		vl = vl * 3400;

		if (vr > 0)
			vr = vr + 8100;
		else
			vr = vr - 8100;

		if (vl > 0)
			vl = vl + 8100;
		else
			vl = vl - 8100;

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

		I90_PWM_send(vl, vr);

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
	int mapSize = 100;
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
			vr_here = 3 * rho + 0.15 * (8*alpha - 4*(beta));
			vl_here = 3 * rho - 0.15 * (8*alpha - 4*(beta));

			vr_here = vr_here / 10;
			vl_here = vl_here / 10;
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
			else if ((rho > 0.6 || Vt2 > 4))  //gamma = 2
			{
				// 			if (state == 3)
				// 				state = 3;
				// 			else
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
				vr_here = vr_here*1.4;
				vl_here = vl_here*1.4;
				break;

			case 2:  //直線Mode
					 // 			rho_gain = (4 * rho / 200) + (1 - (rho / 200));
					 // 			if (rho > 150) rho_gain = 4;
				rho_gain = 1.0331;
				vr_here = rho_gain *rho + 0.15 * (alpha_gain*alpha - 0 * (beta));
				vl_here = rho_gain *rho - 0.15 * (alpha_gain*alpha - 0 * (beta));
				vr_here = vr_here*0.3;
				vl_here = vl_here*0.3;
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

				// 		alpha_gain = M1*N1*5.8766 +
				// 			                  M1*N2*5.6385 +
				// 			                  M2*N1*5.8766 +
				// 			                  M2*N2*5.6385;
				// 
				// 		beta_gain = M1*N1*1.1052 +
				// 			                M1*N2*1.0776 +
				// 			                M2*N1*1.1052 +
				// 			                M2*N2*1.0776;

				alpha_gain = M1*N1*1.2833 +
					M1*N2*1.1022 +
					M2*N1* 1.2833 +
					M2*N2*1.1022;

				beta_gain = -M1*N1*0.0487 +
					-M1*N2*0.0517 +
					-M2*N1*0.0487 +
					-M2*N2*0.0517;

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


			if (jump_draw % 20 == 0 )
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



			if ((abs(x_here - target_pos[0]) < 0.02  && abs(y_here - target_pos[1]) < 0.02) || jump_draw > 600 /*|| zdir_here < 0.2*/)
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
			cvLine(draw_data, cvPoint((int)x_save[i], (int)y_save[i]), cvPoint((int)x_save[i + 1], (int)y_save[i + 1]), CV_RGB(150, 150, 255), 1);
		}


		x_save.clear();
		y_save.clear();



		CvvImage show1;
		show1.CopyOf(draw_data);
		show1.Show(*dc, 0, 0, draw_data->width, draw_data->height);

		memset((unsigned char*)draw_data->imageData, 0, draw_data->imageSize);

		Sleep(1);
	}




	*Thread_Info->Continue = false;
	::PostMessage(hWnd->m_hWnd, WNU_THREAD_EXIT, 0, 0);
	return(0);
}

void CRSocket::OnReceive(int nErrorCode) {

	static unsigned int counter(0);

	if (0 == nErrorCode) {
		static int i = 0;
		i++;

		int nRead(0);

		volatile int cbLeft(SIZE_POSE_DATA*sizeof(double)); // 4 Byte
		volatile int cbDataReceived(0);
		int cTimesRead(0);
		do {


			// Determine Socket State
			nRead = Receive((byte*)&m_pose_data + cbDataReceived, cbLeft);

			if (nRead == SOCKET_ERROR) {
				if (GetLastError() != WSAEWOULDBLOCK) {
					AfxMessageBox(_T("Error occurred"));
					Close();
				}
				break;
			}


			cbLeft -= nRead;
			cbDataReceived += nRead;
			cTimesRead++;
		} while (cbLeft > 0 && cTimesRead < 50);

	}

	m_pose_data_static[0] = m_pose_data[0];
	m_pose_data_static[1] = m_pose_data[1];
	m_pose_data_static[2] = m_pose_data[2];

	//CListBox* aListBox = (CListBox*)m_parent->GetDlgItem(IDC_SENSOR_DATA);
	//aListBox->SetRedraw(false);
	//aListBox->ResetContent();
// 	char buffer[50];
// 	sprintf_s(buffer, 50, "%2.4lf : %2.4lf : %2.4lf | %d",
// 		m_pose_data[0], m_pose_data[1], m_pose_data[2], counter++);
	//aListBox->AddString(CString(buffer));
	//aListBox->SetRedraw(true);

	// ***********************************
	// Show the Pose data to User
	// ***********************************
	// 	m_parent->GetDlgItem(IDC_POS_DATA)->SetWindowTextW(CString(buffer));
	// 
	// 	CDC *pDC = m_parent->GetDlgItem(IDC_POS_SHOW)->GetDC();
	// 	pDC->SetDCBrushColor(RGB(0, 0, 0));
	// 	pDC->Ellipse(0, 0, 200, 200);
	// 	pDC->SetDCPenColor(RGB(255, 255, 255));
	// 	pDC->MoveTo(100, 100);
	// 	pDC->LineTo(100 + 50*sin(m_pose_data[2]), 100 + 50*cos(m_pose_data[2]));
	// 	pDC->Ellipse(
	// 		100 + 50 * sin(m_pose_data[2]) -3,
	// 		100 + 50 * cos(m_pose_data[2]) -3,
	// 		100 + 50 * sin(m_pose_data[2]) +3,
	// 		100 + 50 * cos(m_pose_data[2]) +3);
	// 	pDC->MoveTo(100, 25);
	// 	CString aDegree;
	// 	aDegree.Format(L"Yaw(Deg): %lf3.2", m_pose_data[2] * 180 / 3.1415926);
	// 	pDC->TextOutW(10, 215, aDegree);


	CSocket::OnReceive(nErrorCode);
}

BOOL CRSocket::OnMessagePending() {


	return 0;
}

void CRSocket::registerParent(CWnd* _parent) {
	m_parent = _parent;
}

void CLRF_controlDlg::I90_PWM_send(int L_PWM, int R_PWM)
{
	int L, R, mid = 16384; //0~32767

	if (R_PWM > 0)
		R_PWM = R_PWM + c_Synchronous;

	if (R_PWM < 0)
	{
		L_PWM = L_PWM - c_Synchronous;
		R_PWM = R_PWM;
	}

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


