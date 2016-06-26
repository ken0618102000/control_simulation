#include "stdafx.h"
#include "RSocket.h"
#include "LRF_controlDlg.h"
#include "resource.h"


CRSocket::CRSocket() {}


CRSocket::~CRSocket() {}

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

	//CListBox* aListBox = (CListBox*)m_parent->GetDlgItem(IDC_SENSOR_DATA);
	//aListBox->SetRedraw(false);
	//aListBox->ResetContent();
	char buffer[50];
	sprintf_s(buffer, 50, "%2.4lf : %2.4lf : %2.4lf | %d", 
		m_pose_data[0], m_pose_data[1], m_pose_data[2], counter++);
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
