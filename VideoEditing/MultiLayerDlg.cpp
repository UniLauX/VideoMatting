// MultiLayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoEditing.h"
#include "MultiLayerDlg.h"
#include "LayerEditDlg.h"


// CMultiLayerDlg dialog

//IMPLEMENT_DYNAMIC(CMultiLayerDlg, CDialog)
IMPLEMENT_DYNCREATE(CMultiLayerDlg, CDialog)

CMultiLayerDlg::CMultiLayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiLayerDlg::IDD, pParent)
{

}

CMultiLayerDlg::~CMultiLayerDlg()
{
}

void CMultiLayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_Layerlist);
	DDX_Control(pDX, IDC_SLIDER1, depthCtrl);
	DDX_Control(pDX, IDC_CHECK_ENABLE_DEPTH, check_SelectPoint);
}


BEGIN_MESSAGE_MAP(CMultiLayerDlg, CDialog)

	
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMultiLayerDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CMultiLayerDlg::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_DEPTH, &CMultiLayerDlg::OnBnClickedCheckEnableDepth)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CMultiLayerDlg::OnNMCustomdrawSlider1)
END_MESSAGE_MAP()


// CMultiLayerDlg message handlers

BOOL CMultiLayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	LONG lStyle;
	lStyle = GetWindowLong(m_Layerlist.m_hWnd, GWL_STYLE);
	lStyle &= ~LVS_TYPEMASK; 
	lStyle |= LVS_REPORT; 
	SetWindowLong(m_Layerlist.m_hWnd, GWL_STYLE, lStyle);
	DWORD dwStyle = m_Layerlist.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;	//dwStyle |= LVS_EX_CHECKBOXES;
	dwStyle|=LVS_EX_FULLROWSELECT;
	m_Layerlist.SetExtendedStyle(dwStyle); 

	m_Layerlist.InsertColumn(0,"Name",LVCFMT_LEFT,120);
	m_Layerlist.InsertColumn(1,"Type",LVCFMT_LEFT,120);

	depthCtrl.SetRange(0,255);
	depthCtrl.SetPos(0);

	check_status=check_SelectPoint.GetCheck();
	GetDlgItem(IDC_SLIDER1)->EnableWindow(check_status?TRUE:FALSE);


	return TRUE;  
}

void CMultiLayerDlg::addLayer(CString name,int depth)
{
	if(name.IsEmpty())
	{
		MessageBox("Layer name can't be empty");
		return;
	}

	int nCount=m_Layerlist.GetItemCount();
	for (int i=0;i<nCount;++i)
	{
		CString nameE=m_Layerlist.GetItemText(i,0);
		CString depth=m_Layerlist.GetItemText(i,1);
		if (nameE==name)
		{
			MessageBox("Layer name already exists");
			return;
		}
	}

	CString str;
	str.Format("%d",depth);
	switch(depth)
	{
	case 0:
		str = "static plane";
		break;
	case 1:
		str = "static rigid";
		break;
	case 2:
		str = "plane motion";
		break;
	case 3:
		str = "rigid motion";
		break;
	case 4:
		str = "non-rigid motion";
		break;
	}
	m_Layerlist.InsertItem(nCount,name);
	m_Layerlist.SetItemText(nCount,1,str);
}

void CMultiLayerDlg::setCheckboxStatus(bool ischecked)
{
	if (ischecked)
	{
			check_SelectPoint.SetCheck(BST_CHECKED);
	}
	else
		check_SelectPoint.SetCheck(BST_UNCHECKED);

	GetDlgItem(IDC_SLIDER1)->EnableWindow(ischecked);

}

CVideoEditingView* CMultiLayerDlg::getView()
{
	CFrameWnd* pFrame=(CFrameWnd*)(AfxGetApp()->m_pMainWnd);
	return (CVideoEditingView*)pFrame->GetActiveView();

}
void CMultiLayerDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
}

void CMultiLayerDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)//选中一layer触发
{
	int n=m_Layerlist.GetSelectedCount();
	if (n==1)
	{
		CString name;
		int m=m_Layerlist.GetSelectionMark();
		name=m_Layerlist.GetItemText(m,0);
		printf("%s\n",name);
		getView()->SetLayerIndex(name);
		getView()->UpdateImageBuffers(name);
		this->setCheckboxStatus(false);
		
	}

	else
		getView()->SetLayerIndex("");

	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CMultiLayerDlg::OnBnClickedButtonEdit()
{
	// TODO: Add your control notification handler code here


	POSITION  pos=m_Layerlist.GetFirstSelectedItemPosition();
	if(pos==NULL)
	{
		MessageBox("Select a layer!");
		return;
	}
	while(pos)
	{
		int nItem=m_Layerlist.GetNextSelectedItem(pos);
		CString name=m_Layerlist.GetItemText(nItem,0);
		int	depth=atoi(m_Layerlist.GetItemText(nItem,1));

		if (name!="background"&&name!="editLayer")
		{
			CLayerEditDlg editDlg;
			editDlg.SetLayerNameDepth(name,depth);
			int nResponse=editDlg.DoModal();

			if (nResponse==IDOK)
			{
				CString d;
				d.Format("%d",editDlg.GetLayerDepth());
				//m_Layerlist.SetItemText(nItem,0,editDlg.GetLayerName());
				m_Layerlist.SetItemText(nItem,1,d);
				getView()->UpdateLayerInfo(name,editDlg.GetLayerName(),editDlg.GetLayerDepth());	
			}
			else
			{
			}
			
		}
		
	}
	
}

void CMultiLayerDlg::OnBnClickedCheckEnableDepth()
{
	// TODO: Add your control notification handler code here
	check_status=check_SelectPoint.GetCheck();

	if(check_status)
	{
		if(::MessageBox(NULL, "Make sure all the layers are generated,operation can't  restore", "Thread", MB_OKCANCEL)!=IDOK)
		{
			check_SelectPoint.SetCheck(0);
			return;
		}

		getView()->BeginCalDepth(true,true);
		GetDlgItem(IDC_SLIDER1)->EnableWindow(check_status?TRUE:FALSE);
	}
	else
		GetDlgItem(IDC_SLIDER1)->EnableWindow(check_status?TRUE:FALSE);



	
}

void CMultiLayerDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	int nPos = depthCtrl.GetPos(); //获得滑块的当前位置

	CString str;
	str.Format("%d",nPos);
	SetDlgItemText(IDC_EDIT_DEPTH,str);

	getView()->SetZDepth(nPos);

	*pResult = 0;
}
