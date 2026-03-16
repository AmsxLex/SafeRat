#include "pch.h"
#include "ColumnTreeCtrl.h"
#include "Mytypedef.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// IE 5.0 or higher required
#ifndef TVS_NOHSCROLL
#error CColumnTreeCtrl requires IE 5.0 or higher; _WIN32_IE should be greater than 0x500.
#endif

IMPLEMENT_DYNAMIC(CCustomTreeChildCtrl, CTreeCtrl)
CCustomTreeChildCtrl::CCustomTreeChildCtrl()
{

#ifdef _OWNER_DRAWN_TREE // only if owner-drawn 
	// init bitmap image structure
	m_bkImage.hbm=NULL;
	m_bkImage.xOffsetPercent = 0;
	m_bkImage.yOffsetPercent = 0;

	// create imagelist for tree buttons
	m_imgBtns.Create (9, 9, ILC_COLOR32|ILC_MASK,2,1);
	CBitmap* pBmpBtns = CBitmap::FromHandle(LoadBitmap(AfxGetInstanceHandle(),	MAKEINTRESOURCE(IDB_TREEBTNS)));
	ASSERT(pBmpBtns);
	m_imgBtns.Add(pBmpBtns,RGB(255,0,255));
#endif //_OWNER_DRAWN_TREE
 
	m_hItemDrag = NULL;
	m_hFirstSelectedItem=(NULL);
}

CCustomTreeChildCtrl::~CCustomTreeChildCtrl()
{
}


BEGIN_MESSAGE_MAP(CCustomTreeChildCtrl, CTreeCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CCustomTreeChildCtrl::OnTvnBegindrag)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// Operations
//---------------------------------------------------------------------------

// gets control's background image
BOOL CCustomTreeChildCtrl::GetBkImage(LVBKIMAGE* plvbkImage) const
{
	memcpy(plvbkImage, &m_bkImage, sizeof(LVBKIMAGE));
	return TRUE;
}

// sets background image for control
BOOL CCustomTreeChildCtrl::SetBkImage(LVBKIMAGE* plvbkImage)
{
	memcpy(&m_bkImage, plvbkImage, sizeof(LVBKIMAGE));
	Invalidate();
	return TRUE;
}


//---------------------------------------------------------------------------
// Message Handlers
//---------------------------------------------------------------------------
void CCustomTreeChildCtrl::OnTimer(UINT_PTR nIDEvent)
{
	// Do nothing.
	// CTreeCtrl sends this message to scroll the bitmap in client area
	// which also causes background bitmap scrolling,
	// so we don't pass this message to the base class.
}

void CCustomTreeChildCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// CTreeCtrl may scroll the bitmap up or down in several cases,
	// so we need to invalidate entire client area
	Invalidate();

	//... and pass to the base class
	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CCustomTreeChildCtrl::ClearSelection(BOOL bMultiOnly/*=FALSE*/)  
{  
	//  if ( !bMultiOnly )  
	//      SelectItem( NULL );  

	for (HTREEITEM hItem = GetRootItem(); hItem != NULL; hItem = GetNextVisibleItem(hItem))
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			SetItemState(hItem, 0, TVIS_SELECTED);
}  

BOOL CCustomTreeChildCtrl::SelectItems( HTREEITEM hFromItem, HTREEITEM hToItem )  
{  
	// Determine direction of selection   
	// (see what item comes first in the tree)  
	HTREEITEM hItem = GetRootItem();  

	while ( hItem && hItem!=hFromItem && hItem!=hToItem )  
		hItem = GetNextVisibleItem( hItem );  

	if ( !hItem )  
		return FALSE;   // Items not visible in tree  

	BOOL bReverse = hItem==hToItem;  

	// "Really" select the 'to' item (which will deselect   
	// the previously selected item)  

	SelectItem( hToItem );  

	// Go through all visible items again and select/unselect  

	hItem = GetRootItem();  
	BOOL bSelect = FALSE;  

	while ( hItem )  
	{  
		if ( hItem == ( bReverse ? hToItem : hFromItem ) )  
			bSelect = TRUE;  

		if ( bSelect )  
		{  
			if ( !( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED ) )  
				SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );  
		}  
		else  
		{  
			if ( GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED )  
				SetItemState( hItem, 0, TVIS_SELECTED );  
		}  

		if ( hItem == ( bReverse ? hFromItem : hToItem ) )  
			bSelect = FALSE;  

		hItem = GetNextVisibleItem( hItem );  
	}  

	return TRUE;  
}  



HTREEITEM CCustomTreeChildCtrl::GetFirstSelectedItem()
{
	for (HTREEITEM hItem = GetRootItem(); hItem!=NULL; hItem = GetNextVisibleItem(hItem))
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			return hItem;

	return NULL;
}

HTREEITEM CCustomTreeChildCtrl::GetNextSelectedItem(HTREEITEM hItem)
{
	for (hItem = GetNextVisibleItem(hItem); hItem!=NULL; hItem = GetNextVisibleItem(hItem))
		if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
			return hItem;

	return NULL;
}

void CCustomTreeChildCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	/*
	if (CheckHit(point))
	{
	//TVS_EDITLABELS 表示可以控件鼠标单击修改树项的名称 
	HTREEITEM hItem = HitTest(point);
	if(hItem)
	{
	ClearSelection();
	// select the clicked item
	SelectItem(hItem);
	SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );  
	m_hFirstSelectedItem = hItem;  
	}
	}*/
	int nSelete = 0;
	HTREEITEM hItem = NULL;
	// 如果是多选的 
	for (HTREEITEM hItem2 = GetRootItem(); hItem2 != NULL; hItem2 = GetNextSiblingItem(hItem2))
	{
		// CString strTemp = GetItemText(hItem2);
		HTREEITEM childItem = GetChildItem(hItem2);
		while (childItem != NULL)
		{
			if (GetItemState(childItem, TVIS_SELECTED) & TVIS_SELECTED)
			{
				nSelete++;
				hItem = childItem;
			}
			childItem = GetNextSiblingItem(childItem);
		}
	}

	if (nSelete > 1)
	{
 		SetFocus(); 
	}
	else
	{

		if (CheckHit(point))
		{
			//TVS_EDITLABELS 表示可以控件鼠标单击修改树项的名称； 
			HTREEITEM hItem2 = HitTest(point);

			if (hItem2 != hItem)
			{
				ClearSelection();
				// select the clicked item
				SelectItem(hItem2);
				SetItemState( hItem2, TVIS_SELECTED, TVIS_SELECTED );  
				m_hFirstSelectedItem = hItem2;  

			}
		}

	    
	}
 

	CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CCustomTreeChildCtrl::SetChildCheck(HTREEITEM hTree,BOOL bCheck)
{
	hTree = GetChildItem(hTree);  //获取子项句柄
	while (hTree)
	{

		SetCheck(hTree, bCheck);
		SetChildCheck(hTree,bCheck);     //递归调用
		hTree = GetNextSiblingItem(hTree);    //获取兄弟的句柄
	}
} 


std::vector<HTREEITEM> vTemp;
void CCustomTreeChildCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{

	if (CheckHit(point))
	{
		//TVS_EDITLABELS 表示可以控件鼠标单击修改树项的名称； 
		HTREEITEM hItem = HitTest(point);
		if(hItem)
		{
#ifdef _OWNER_DRAWN_TREE
			// if the clicked item is partially visible we won't process
			// the message to avoid background bitmap scrolling
			// TODO: need to avoid scrolling and process the message
			CRect rcItem, rcClient;
			GetClientRect(&rcClient);
			GetItemRect(hItem,&rcItem,FALSE);
			if(rcItem.bottom>rcClient.bottom)
			{
				Invalidate();
				EnsureVisible(hItem);
				SelectItem(hItem);
				//CTreeCtrl::OnLButtonDown(nFlags, point);
				return;
			}
#endif //_OWNER_DRAWN_TREE



			
			// Is the clicked item already selected ?  
			BOOL bIsClickedItemSelected = GetItemState( hItem, TVIS_SELECTED ) & TVIS_SELECTED;  
			if ( nFlags & MK_SHIFT )  
			{  
				// Select from first selected item to the clicked item  
				if ( !m_hFirstSelectedItem )  
					m_hFirstSelectedItem = GetSelectedItem();  

				SelectItems( m_hFirstSelectedItem, hItem ); 

			}
			else if (  nFlags & MK_CONTROL )
			{

				// Find which item is currently selected  
				typedef std::vector<HTREEITEM > TreeItemVector;

				TreeItemVector hSelectedItems;
				HTREEITEM hSelectedItem = GetFirstSelectedItem();
				while (hSelectedItem)
				{
					if (hSelectedItem != hItem)
						hSelectedItems.push_back(hSelectedItem);
					hSelectedItem = GetNextSelectedItem(hSelectedItem);
				}

				// We want the newly selected item to toggle its selected state,  
				// so unselect now if it was already selected before  
				if ( bIsClickedItemSelected )  
					SetItemState( hItem, 0, TVIS_SELECTED );  
				else  
				{  
					SelectItem(hItem);  
					SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );  
				}  

				// If the previously selected item was selected, re-select it
				for(TreeItemVector::const_iterator itr =  hSelectedItems.begin(); itr != hSelectedItems.end(); ++itr){
					SetItemState(*itr , TVIS_SELECTED, TVIS_SELECTED);
				}

				// Store as first selected item (if not already stored)  
				if ( m_hFirstSelectedItem == NULL )  
					m_hFirstSelectedItem = hItem;  

				return;
			}else
			{
				//if(!bIsClickedItemSelected)
				//SelectItem(hItem);


				vTemp.clear();
				for (HTREEITEM hItem2 = GetRootItem(); hItem2 != NULL; hItem2 = GetNextSiblingItem(hItem2))
				{
					// CString strTemp = GetItemText(hItem2);
					HTREEITEM childItem = GetChildItem(hItem2);
					while (childItem != NULL)
					{
						if (GetItemState(childItem, TVIS_SELECTED) & TVIS_SELECTED)
						{
							vTemp.push_back(childItem);
						}
						childItem = GetNextSiblingItem(childItem);
					}
				}


				ClearSelection();
				// select the clicked item
				SelectItem(hItem);
				SetItemState( hItem, TVIS_SELECTED, TVIS_SELECTED );  
				m_hFirstSelectedItem = hItem;  
			}
			
		    CTreeCtrl::OnLButtonDown(nFlags, point);
		}
	}
	else  
	{
		// if clicked outside the item's label
		// than set focus to contol window
		SetFocus(); 
		ClearSelection();
	}
}

void CCustomTreeChildCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_hItemDrag != NULL)
		EndDrag(nFlags, point);
	else
		CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CCustomTreeChildCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// process this message only if double-clicked the real item's label
	// mask double click if outside the real item's label
	if (CheckHit(point))
	{
		HTREEITEM hItem = HitTest(point);
		if(hItem)
		{
#ifdef _OWNER_DRAWN_TREE
			// if the clicked item is partially visible we should invalidate
			// entire client area to avoid background bitmap scrolling
			CRect rcItem, rcClient;
			GetClientRect(&rcClient);
			GetItemRect(hItem,&rcItem,FALSE);
			if(rcItem.bottom>rcClient.bottom)
			{
				Invalidate();
				CTreeCtrl::OnLButtonDown(nFlags, point);
				return;
			}
#endif //_OWNER_DRAWN_TREE

			SelectItem(hItem);

		}

		// call standard message handler
		CTreeCtrl::OnLButtonDblClk(nFlags, point);
	}	
	else 
	{
		// if clicked outside the item's label
		// than set focus to contol window
		SetFocus();
	}
}

void CCustomTreeChildCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// 拖拽
	if (m_hItemDrag != NULL)
	{
		CPoint pt;
		pt = point;
		ClientToScreen(&pt);

	    // 拖动项到现在的位置
		CImageList::DragMove(pt); //鼠标移动时显示被拖动的项图像
		CImageList::DragShowNolock(FALSE); //隐藏拖动项，避免经过时留下痕迹


		TVHITTESTINFO tvhti;
		tvhti.pt = pt;
		ScreenToClient(&tvhti.pt);
		HTREEITEM hItemSel = HitTest(&tvhti);

		if (CWnd::WindowFromPoint(pt) != this) 
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO)); // 不可拖拽区域 显示禁止图标
		else
		{
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

			//
			HTREEITEM finalDropItem = NULL;
			HTREEITEM parentItem = GetParentItem(tvhti.hItem);
			if (parentItem != NULL)
			{
				finalDropItem = parentItem;
			}
			else
			{
				finalDropItem = tvhti.hItem;
			}
			SelectDropTarget(finalDropItem);
		}

		CImageList::DragShowNolock(TRUE); //显示拖动的图像
	}

	// mask mouse move if outside the real item's label
	if (CheckHit(point))
	{
		// call standard handler
		CTreeCtrl::OnMouseMove(nFlags, point);
	}
}


#ifdef _OWNER_DRAWN_TREE // this code is only for owner-drawn contol

//-------------------------------------------------------------------------------
// Helper drawing funtions.
// I tried standard GDI fucntion LineTo with PS_DOT pen style, 
// but that didn't have the effect I wanted, so I had to use these ones. 
//-------------------------------------------------------------------------------

// draws a dotted horizontal line
static void _DotHLine(HDC hdc, LONG x, LONG y, LONG w, COLORREF cr)
{
	for (; w>0; w-=2, x+=2)
		SetPixel(hdc, x, y, cr);
}

// draws a dotted vertical line
static void _DotVLine(HDC hdc, LONG x, LONG y, LONG w, COLORREF cr)
{
	for (; w>0; w-=2, y+=2)
		SetPixel(hdc, x, y, cr);
}
//----------------------------------------------------------------------
// Sends NM_CUSTOMDRAW notification to the parent (CColumnTreeCtrl)
// The idea is to use one custom drawing code for both custom-drawn and
// owner-drawn controls
//----------------------------------------------------------------------

LRESULT CCustomTreeChildCtrl::CustomDrawNotify(LPNMTVCUSTOMDRAW lpnm)
{
	lpnm->nmcd.hdr.hwndFrom = GetSafeHwnd();
	lpnm->nmcd.hdr.code = NM_CUSTOMDRAW;
	lpnm->nmcd.hdr.idFrom = GetWindowLong(m_hWnd, GWL_ID);
	return GetParent()->SendMessage(WM_NOTIFY,(WPARAM)0,(LPARAM)lpnm);
}

//---------------------------------------------------------------------------
// Performs painting in the client's area.
// The pDC parameter is the memory device context created in OnPaint handler.
//---------------------------------------------------------------------------

LRESULT CCustomTreeChildCtrl::OwnerDraw(CDC* pDC)
{
	NMTVCUSTOMDRAW nm;	// this structure is used by NM_CUSTOMDRAW message
	DWORD dwFlags;		// custom-drawing flags

	DWORD dwRet;		// current custom-drawing operation's return value
	CRect rcClient;		// client's area rectangle

	// get client's rectangle
	GetClientRect(&rcClient);

	// initialize the structure
	memset(&nm,0,sizeof(NMTVCUSTOMDRAW));

	// set current drawing stage to pre-paint
	nm.nmcd.dwDrawStage = CDDS_PREPAINT; 
	nm.nmcd.hdc = pDC->m_hDC;
	nm.nmcd.rc = rcClient;

	// notify the parent (CColumnTreeCtrl) about pre-paint stage
	dwFlags = (DWORD)CustomDrawNotify(&nm); // CDDS_PREPAINT

	//
	// draw control's background
	//

	// set control's background color
	COLORREF crBkgnd = IsWindowEnabled()?pDC->GetBkColor():GetSysColor(COLOR_BTNFACE);
	// ... and fill the background rectangle
	pDC->FillSolidRect( rcClient, crBkgnd ); 

	/*if(m_bkImage.hbm && IsWindowEnabled())
	{
	// draw background bitmap

	int xOffset = m_nOffsetX;
	int yOffset = rcClient.left;
	int yHeight = rcClient.Height();

	SCROLLINFO scroll_info;
	// Determine window viewport to draw into taking into account
	// scrolling position
	if ( GetScrollInfo( SB_VERT, &scroll_info, SIF_POS | SIF_RANGE ) )
	{
	yOffset = -scroll_info.nPos;
	yHeight = max( scroll_info.nMax+1, rcClient.Height());
	}

	// create temporary memory DC for background bitmap
	CDC dcTemp;
	dcTemp.CreateCompatibleDC(pDC);
	BITMAP bm;
	::GetObject(m_bkImage.hbm,sizeof(BITMAP),&bm);
	CBitmap* pOldBitmap = 
	dcTemp.SelectObject( CBitmap::FromHandle(m_bkImage.hbm) ); 

	// copy the background bitmap from temporary DC to painting DC
	float x = (float)m_bkImage.xOffsetPercent/100*(float)rcClient.Width();
	float y = (float)m_bkImage.yOffsetPercent/100*(float)rcClient.Height();
	pDC->BitBlt(/ *xOffset* /+(int)x, 
	/ *yOffset+* /(int)y, 
	bm.bmWidth, bm.bmHeight, &dcTemp, 0, 0, SRCCOPY);

	// clean up
	dcTemp.SelectObject(pOldBitmap);

	}
	*/

	// notify the parent about post-erase drawing stage
	if(dwFlags&CDRF_NOTIFYPOSTERASE)
	{
		nm.nmcd.dwDrawStage = CDDS_POSTERASE;
		dwRet = (DWORD)CustomDrawNotify(&nm); // CDDS_POSTERASE
	}

	// select correct font
	CFont* pOldFont = pDC->SelectObject(GetFont());

	// get control's image lists
	CImageList* pstateList = GetImageList(TVSIL_STATE);
	CImageList* pimgList = GetImageList(TVSIL_NORMAL);

	// here we will store dimensions of the images
	CSize iconSize, stateImgSize;

	// retreive information about item images
	if(pimgList)
	{
		// get icons dimensions
		IMAGEINFO ii;
		if(pimgList->GetImageInfo(0, &ii))
			iconSize = CSize(ii.rcImage.right-ii.rcImage.left,
			ii.rcImage.bottom-ii.rcImage.top);
	}		

	// retrieve information about state images
	if(pstateList)
	{
		// get icons dimensions
		IMAGEINFO ii;
		if(pstateList->GetImageInfo(0, &ii))
			stateImgSize = CSize(ii.rcImage.right-ii.rcImage.left,
			ii.rcImage.bottom-ii.rcImage.top);
	}

	//
	// draw all visible items
	//

	HTREEITEM hItem = GetFirstVisibleItem();

	while(hItem)
	{
		// set transparent background mode
		int nOldBkMode = pDC->SetBkMode(TRANSPARENT);

		// get CTreeCtrl's style
		DWORD dwStyle = GetStyle();

		// get current item's state
		DWORD dwState = GetItemState(hItem,0xFFFF);

		BOOL bEnabled = IsWindowEnabled();
		BOOL bSelected = dwState&TVIS_SELECTED;
		BOOL bHasFocus = (GetFocus() && GetFocus()->m_hWnd==m_hWnd)?TRUE:FALSE;


		// Update NMCUSTOMDRAW structure. 
		// We won't draw entire items here (only item icons and lines), 
		// all other work will be done in parent's notifications handlers.
		// This allows to use one code for both custom-drawn and owner-drawn controls.

		nm.nmcd.dwItemSpec = (DWORD_PTR)hItem;

		// set colors for item's background and text
		if(bEnabled)
		{	
			//if(bHasFocus)
			{
				nm.clrTextBk = bSelected?GetSysColor(COLOR_HIGHLIGHT):crBkgnd;
				nm.clrText = ::GetSysColor(bSelected?COLOR_HIGHLIGHTTEXT:COLOR_MENUTEXT);
				nm.nmcd.uItemState = dwState | (bSelected?CDIS_FOCUS:0);
 
			}
			/*else
			{
				
				if(GetStyle()&TVS_SHOWSELALWAYS)
				{
					nm.clrTextBk = bSelected?GetSysColor(COLOR_INACTIVEBORDER):crBkgnd;
					nm.clrText = ::GetSysColor(COLOR_MENUTEXT);
				}
				else
				{
					nm.clrTextBk = crBkgnd;
					nm.clrText = ::GetSysColor(COLOR_MENUTEXT);
				}	
			}*/
		}
		else
		{
			nm.clrTextBk = bSelected?GetSysColor(COLOR_BTNSHADOW):crBkgnd;
			nm.clrText = ::GetSysColor(COLOR_GRAYTEXT);
		}



		// set item's rectangle
		GetItemRect(hItem,&nm.nmcd.rc,0);

		// set clipping rectangle
		CRgn rgn;
		rgn.CreateRectRgn(nm.nmcd.rc.left, nm.nmcd.rc.top, 
			nm.nmcd.rc.left+m_nFirstColumnWidth, nm.nmcd.rc.bottom);
		pDC->SelectClipRgn(&rgn);

		dwRet = CDRF_DODEFAULT;

		// notify the parent about item pre-paint drawing stage
		if(dwFlags&CDRF_NOTIFYITEMDRAW)
		{
			nm.nmcd.dwDrawStage = CDDS_ITEMPREPAINT;
			dwRet = (DWORD)CustomDrawNotify(&nm); // CDDS_ITEMPREPAINT

		}

		if(!(dwFlags&CDRF_SKIPDEFAULT))
		{
			//
			// draw item's icons and dotted lines
			//

			CRect rcItem;
			int nImage,nSelImage;

			GetItemRect(hItem,&rcItem,TRUE);
			GetItemImage(hItem,nImage,nSelImage);

			int x = rcItem.left-3;
			int yCenterItem = rcItem.top + (rcItem.bottom - rcItem.top)/2; 

			// draw item icon
			if(pimgList)
			{
				int nImageIndex = dwState&TVIS_SELECTED?nImage:nSelImage;
				x-=iconSize.cx+1;
				pimgList->Draw(pDC,nImageIndex,
					CPoint(x, yCenterItem-iconSize.cy/2),ILD_TRANSPARENT);
			}

			// draw item state icon
			if(GetStyle()&TVS_CHECKBOXES && pstateList!=NULL )
			{
				// get state image index
				DWORD dwStateImg = GetItemState(hItem,TVIS_STATEIMAGEMASK)>>12;

				x-=stateImgSize.cx;

				pstateList->Draw(pDC,dwStateImg,
					CPoint(x, yCenterItem-stateImgSize.cy/2),ILD_TRANSPARENT);
			}


			if(dwStyle&TVS_HASLINES)
			{
				//
				// draw dotted lines 
				//

				// create pen
				CPen pen;
				pen.CreatePen(PS_DOT,1,GetLineColor());
				CPen* pOldPen = pDC->SelectObject(&pen);

				HTREEITEM hItemParent = GetParentItem(hItem);

				if(hItemParent!=NULL ||dwStyle&TVS_LINESATROOT)
				{
					_DotHLine(pDC->m_hDC,x-iconSize.cx/2-2,yCenterItem,
						iconSize.cx/2+2,RGB(128,128,128));
				}

				if(hItemParent!=NULL ||	dwStyle&TVS_LINESATROOT && GetPrevSiblingItem(hItem)!=NULL)
				{
					_DotVLine(pDC->m_hDC,x-iconSize.cx/2-2,rcItem.top,
						yCenterItem-rcItem.top, RGB(128,128,128));
				}

				if(GetNextSiblingItem(hItem)!=NULL)
				{
					_DotVLine(pDC->m_hDC,x-iconSize.cx/2-2,yCenterItem,
						rcItem.bottom-yCenterItem,RGB(128,128,128));
				}

				int x1 = x-iconSize.cx/2-2;

				while(hItemParent!=NULL )
				{
					x1-=iconSize.cx+3;
					if(GetNextSiblingItem(hItemParent)!=NULL)
					{
						_DotVLine(pDC->m_hDC,x1,rcItem.top,rcItem.Height(),RGB(128,128,128));
					}
					hItemParent = GetParentItem(hItemParent);
				}

				// clean up
				pDC->SelectObject(pOldPen);

			}

			if(dwStyle&TVS_HASBUTTONS && ItemHasChildren(hItem))
			{
				// 绘制+-按钮
				int nImg = GetItemState(hItem,TVIF_STATE)&TVIS_EXPANDED?1:0;
				m_imgBtns.Draw(pDC, nImg, CPoint(x-iconSize.cx/2-6,yCenterItem-4), 
					ILD_TRANSPARENT);
			}

		}

		pDC->SelectClipRgn(NULL);

		// notify parent about item post-paint stage
		if(dwRet&CDRF_NOTIFYPOSTPAINT)
		{
			nm.nmcd.dwDrawStage = CDDS_ITEMPOSTPAINT;
			dwRet = (DWORD)CustomDrawNotify(&nm); // CDDS_ITEMPOSTPAINT
		}

		// clean up
		pDC->SetBkMode(nOldBkMode);

		// get the next visible item
		hItem = GetNextVisibleItem(hItem);
	};

	//clean up

	pDC->SelectObject(pOldFont);

	return 0;
}

int CCustomTreeChildCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// inavlidate entire client's area to avoid bitmap scrolling
	Invalidate();

	// ... and call standard message handler
	return CTreeCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CCustomTreeChildCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	// inavlidate entire client's area to avoid bitmap scrolling
	Invalidate();

	// ... and call standard message handler
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}


#endif //_OWNER_DRAWN_TREE


void CCustomTreeChildCtrl::OnPaint()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	CPaintDC dc(this);

	CDC dcMem;
	CBitmap bmpMem;

	// use temporary bitmap to avoid flickering
	dcMem.CreateCompatibleDC(&dc);
	if (bmpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height()))
	{
		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);

		// paint the window onto the memory bitmap

#ifdef _OWNER_DRAWN_TREE	// if owner-drawn
		OwnerDraw(&dcMem);	// use our code
#else						// else use standard code
		CWnd::DefWindowProc(WM_PAINT, (WPARAM)dcMem.m_hDC, 0);
#endif //_OWNER_DRAWN_TREE

		// copy it to the window's DC
		dc.BitBlt(0, 0, rcClient.right, rcClient.bottom, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject(pOldBmp);

		bmpMem.DeleteObject();
	}
	dcMem.DeleteDC();

}

BOOL CCustomTreeChildCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;	// do nothing
}

BOOL CCustomTreeChildCtrl::CheckHit(CPoint point)
{
	// return TRUE if should pass the message to CTreeCtrl
	UINT fFlags;
	HTREEITEM hItem = HitTest(point, &fFlags);

	if (hItem == NULL )
	{
		return FALSE;
	}

	CRect rcItem,rcClient;
	GetClientRect(rcClient);
	GetItemRect(hItem, &rcItem, TRUE);

	if (  fFlags & TVHT_ONITEMSTATEICON  )
	{
		BOOL bCheck;
		bCheck = GetCheck(hItem);      //获取当前复选状态
		SetChildCheck(hItem,bCheck);                  //设置子项复选状态
	}

	if (fFlags & TVHT_ONITEMICON ||
		fFlags & TVHT_ONITEMBUTTON ||
		fFlags & TVHT_ONITEMSTATEICON
		)
		return TRUE;

	if(GetStyle()&TVS_FULLROWSELECT)
	{
		rcItem.right = rcClient.right;
		if(rcItem.PtInRect(point)) 
			return TRUE;

		return FALSE;
	}

	// verify the hit result 在标签(字符串)与项目。
	if (fFlags & TVHT_ONITEMLABEL)
	{
		rcItem.right = m_nFirstColumnWidth;
		// check if within the first column
		if (!rcItem.PtInRect(point))
			return FALSE;

		CString strSub;
		AfxExtractSubString(strSub, GetItemText(hItem), 0, '\t');

		CDC* pDC = GetDC();
		pDC->SelectObject(GetFont());
		rcItem.right = rcItem.left + pDC->GetTextExtent(strSub).cx + 6;
		ReleaseDC(pDC);

		// check if inside the label's rectangle 检查是否在标签的矩形内
		if (!rcItem.PtInRect(point))
			return FALSE;

		return TRUE;
	}

	return FALSE;
}
 


#define COLUMN_MARGIN		1		// 1 pixel between coumn edge and text bounding rectangle

// default minimum width for the first column
#ifdef _OWNER_DRAWN_TREE
#define DEFMINFIRSTCOLWIDTH 0 // we use clipping rgn, so we can have zero-width column		
#else
#define DEFMINFIRSTCOLWIDTH 100	// here we need to avoid zero-width first column	
#endif

IMPLEMENT_DYNCREATE(CColumnTreeCtrl, CStatic)
BEGIN_MESSAGE_MAP(CColumnTreeCtrl, CStatic)
ON_WM_PAINT()
ON_WM_ERASEBKGND()
ON_WM_SIZE()
ON_WM_HSCROLL()
ON_WM_SETTINGCHANGE()
ON_WM_ENABLE()
END_MESSAGE_MAP()

CColumnTreeCtrl* g_pTreeList = NULL;
CColumnTreeCtrl::CColumnTreeCtrl()
{
	// initialize members
	m_uMinFirstColWidth = DEFMINFIRSTCOLWIDTH;
	m_bHeaderChangesBlocked = FALSE;
	m_xOffset = 0;

	// 排序
	m_nSortedCol = 1;
	m_bAscending = true;
	g_pTreeList = this;
}

CColumnTreeCtrl::~CColumnTreeCtrl()
{
}

BOOL CColumnTreeCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CStatic::Create(_T(""), dwStyle, rect, pParentWnd, nID);
	Initialize();
	return TRUE;
}

void CColumnTreeCtrl::PreSubclassWindow()
{
	Initialize();
}

void CColumnTreeCtrl::AssertValid( ) const
{
	// validate control state
#if _DEBUG
	CStatic::AssertValid();

#endif

	ASSERT( m_Tree.m_hWnd ); 
	ASSERT( m_Header.m_hWnd );
//	ASSERT( m_Header2.m_hWnd );
}

// 排序----------------------------------------------------------------------------------------------------------------
struct SSortType
{
	int nCol;
	BOOL bAscending;
	LPVOID pThis;
};

int CALLBACK CColumnTreeCtrl::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	SSortType *pSortType = (SSortType *)lParamSort;
	CColumnTreeCtrl* pThis = (CColumnTreeCtrl*)pSortType->pThis;

	HTREEITEM hItem1,hItem2 ;
	hItem1 = ((ClientContext*)lParam1)->m_hItem ;
	hItem2 = ((ClientContext*)lParam2)->m_hItem ;

		 
/*
	CString str;
	str.Format("%s %s  \r\n" , pThis->GetItemText(hItem1,pSortType->nCol) , pThis->GetItemText(hItem2,pSortType->nCol));
	OutputDebugString(str);*/

	CString str1 =  pThis->GetItemText(hItem1,pSortType->nCol);
	CString str2 = pThis->GetItemText(hItem2,pSortType->nCol);

	int m_nComp;
	// compare the two strings, but
	// notice:
	// in this case, "xxxx10" comes after "xxxx2"
	{
		CString tmpStr1, tmpStr2;
		int index = str1.FindOneOf("0123456789");
		if(index!=-1)
			tmpStr1 = str1.Right(str1.GetLength()-index);
		index = str2.FindOneOf("0123456789");
		if(index!=-1)
			tmpStr2 = str2.Right(str2.GetLength()-index);

		tmpStr1 = tmpStr1.SpanIncluding("0123456789");
		tmpStr2 = tmpStr2.SpanIncluding("0123456789");

		if((tmpStr1==L"") && (tmpStr2==L""))
			m_nComp = str1.CompareNoCase(str2);
		else
		{
			/*
			int num1 = _wtoi(tmpStr1);
			int num2 = _wtoi(tmpStr2);*/
			int num1 = atoi(tmpStr1);
			int num2 = atoi(tmpStr2);
			tmpStr1 = str1.SpanExcluding("0123456789");
			tmpStr2 = str2.SpanExcluding("0123456789");

			if(tmpStr1 == tmpStr2)
			{
				if(num1 > num2)
					m_nComp = 1;
				else
					if(num1 < num2)
						m_nComp = -1;
					else
						m_nComp = str1.CompareNoCase(str2);
			}
			else
				m_nComp = str1.CompareNoCase(str2);			
		}
	}

	if(!pSortType->bAscending)
	{
		if(m_nComp == 1)
			m_nComp = -1;
		else
			if(m_nComp == -1)
				m_nComp = 1;
	}

	return m_nComp;
}

void CColumnTreeCtrl::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc; // 
	m_nSortedCol = iCol; // 要排序的列

	
/*
	CString str;
	str.Format("m_bAscending = %s  m_nSortedCol = %d \r\n" ,bAsc ? "TRUE" : "FALSE" , m_nSortedCol);
	OutputDebugString(str);*/

	// sort column.
	m_Header.SetSortImage(m_nSortedCol, m_bAscending);

	HTREEITEM hItem=m_Tree.GetRootItem();
	int i=0;
	do
	{

		TV_SORTCB tSort;
		HTREEITEM low;
		low = hItem;
		tSort.hParent = low;	
		tSort.lpfnCompare = CompareFunc;

		SSortType *pSortType = new SSortType;
		if(pSortType == NULL)
			return;
		pSortType->nCol = m_nSortedCol;
		pSortType->bAscending = m_bAscending;
		pSortType->pThis = this;
		tSort.lParam = (LPARAM)pSortType;
		BOOL m_bReturn = m_Tree.SortChildrenCB(&tSort);
		delete pSortType;	


	}while(hItem = m_Tree.GetNextItem(hItem , TVGN_NEXT), ++i, hItem);  

}

void CColumnTreeCtrl::Initialize()
{
	if (m_Tree.m_hWnd) 
		return; // do not initialize twice

	CRect rcClient;
	GetClientRect(&rcClient);

	// create tree and header controls as children
	m_Tree.Create(WS_CHILD | WS_VISIBLE  | TVS_NOHSCROLL | TVS_NOTOOLTIPS, CRect(), this, TreeID);
	m_Header.Create(WS_CHILD | HDS_BUTTONS | WS_VISIBLE | HDS_FULLDRAG  , rcClient, this, HeaderID);
	//m_Header2.Create(WS_CHILD , rcClient, this, Header2ID);

	m_Header.EnablePopupMenus(FALSE);
	m_Header.SetTheme(xtpControlThemeVisualStudio2008);


	// create horisontal scroll bar
	m_horScroll.Create(SBS_HORZ|WS_CHILD|SBS_BOTTOMALIGN,rcClient,this,HScrollID);

	// set correct font for the header
	CFont* pFont = m_Tree.GetFont();
	m_Header.SetFont(pFont);
	//m_Header2.SetFont(pFont);

	// check if the common controls library version 6.0 is available
	BOOL bIsComCtl6 = FALSE;

	HMODULE hComCtlDll = LoadLibrary(_T("comctl32.dll"));

	if (hComCtlDll)
	{
		typedef HRESULT (CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);

		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hComCtlDll, "DllGetVersion");

		if (pfnDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			HRESULT hRes = (*pfnDllGetVersion)(&dvi);

			if (SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6)
				bIsComCtl6 = TRUE;
		}

		FreeLibrary(hComCtlDll);
	}

	// get header layout
	WINDOWPOS wp;
	HDLAYOUT hdlayout;
	hdlayout.prc = &rcClient;
	hdlayout.pwpos = &wp;
	m_Header.Layout(&hdlayout);
	m_cyHeader = hdlayout.pwpos->cy;

	// offset from column start to text start
	m_xOffset = bIsComCtl6 ? 9 : 6;

	m_xPos = 0;

	UpdateColumns();
}


void CColumnTreeCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	m_Tree.SendMessage(WM_SETTINGCHANGE);
	m_horScroll.SendMessage(WM_SETTINGCHANGE);

	// set correct font for the header
	CRect rcClient;
	GetClientRect(&rcClient);
 

	m_Header.SendMessage(WM_SETTINGCHANGE);
 
	m_Header.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
 
	// get header layout
	WINDOWPOS wp;
	HDLAYOUT hdlayout;
	hdlayout.prc = &rcClient;
	hdlayout.pwpos = &wp;
	m_Header.Layout(&hdlayout);
	m_cyHeader = hdlayout.pwpos->cy;

	RepositionControls();
}

void CColumnTreeCtrl::OnPaint()
{
	// do not draw entire background to avoid flickering
	// just fill right-bottom rectangle when it is visible

	CPaintDC dc(this);

	CRect rcClient;
	GetClientRect(&rcClient);

	int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	CBrush brush;
	brush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));

	CRect rc;

	// determine if vertical scroll bar is visible
	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	m_Tree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
	BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax?TRUE:FALSE;

	if(bVScrollVisible)
	{
		// fill the right-bottom rectangle
		rc.SetRect(rcClient.right-cxVScroll, rcClient.bottom-cyHScroll,
			rcClient.right, rcClient.bottom);
		dc.FillRect(rc,&brush);
	}
}

BOOL CColumnTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
	// do nothing, all work is done in OnPaint()
	return FALSE;
}

void CColumnTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	RepositionControls();
}


void CColumnTreeCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);

	m_Tree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);

	BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax?TRUE:FALSE;

	// determine full header width
	int cxTotal = m_cxTotal+(bVScrollVisible?GetSystemMetrics(SM_CXVSCROLL):0);

	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();

	int xLast = m_xPos;

	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_xPos -= 15;
		break;
	case SB_LINERIGHT:
		m_xPos += 15;
		break;
	case SB_PAGELEFT:
		m_xPos -= cx;
		break;
	case SB_PAGERIGHT:
		m_xPos += cx;
		break;
	case SB_LEFT:
		m_xPos = 0;
		break;
	case SB_RIGHT:
		m_xPos = cxTotal - cx;
		break;
	case SB_THUMBTRACK:
		m_xPos = nPos;
		break;
	}

	if (m_xPos < 0)
		m_xPos = 0;
	else if (m_xPos > cxTotal - cx)
		m_xPos = cxTotal - cx;

	if (xLast == m_xPos)
		return;

	m_Tree.m_nOffsetX = m_xPos;

	SetScrollPos(SB_HORZ, m_xPos);
	CWnd::OnHScroll(nSBCode,nPos,pScrollBar);
	RepositionControls();

}

void CColumnTreeCtrl::OnHeaderItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	// do not allow user to set zero width to the first column.
	// the minimum width is defined by m_uMinFirstColWidth;

	if(m_bHeaderChangesBlocked)
	{
		// do not allow change header size when moving it
		// but do not prevent changes the next time the header will be changed
		m_bHeaderChangesBlocked = FALSE;
		*pResult = TRUE; // prevent changes
		return;
	}

	*pResult = FALSE;

	LPNMHEADER pnm = (LPNMHEADER)pNMHDR;
	if(pnm->iItem==0)
	{
		CRect rc;
		m_Header.GetItemRect(0,&rc);
		if(pnm->pitem->cxy<m_uMinFirstColWidth)
		{
			// do not allow sizing of the first column 
			pnm->pitem->cxy=m_uMinFirstColWidth+1;
			*pResult = TRUE; // prevent changes
		}
		return;
	}

}

void CColumnTreeCtrl::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateColumns();

	m_Tree.Invalidate();
}


//std::vector<string> vMMKShowColor;
void CColumnTreeCtrl::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	// We use custom drawing to correctly display subitems.
	// Standard drawing code is used only for displaying icons and dotted lines
	// The rest of work is done here.

	NMCUSTOMDRAW* pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
	NMTVCUSTOMDRAW* pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;

	switch (pNMCustomDraw->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT: //在这里重绘要特殊显示的颜色
		{
			HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;

			ULONG_PTR dwContext = m_Tree.GetItemData(hItem);
			if (dwContext)
			{
				 if(((ClientContext*)dwContext)->m_dwLineRGB )
				{
					LPNMTVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);
					pNMCD->clrText = ((ClientContext*)dwContext)->m_dwLineRGB;
				}
			}

			*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		}
		break;
	case CDDS_ITEMPOSTPAINT:
		{
			HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
			CRect rcItem = pNMCustomDraw->rc;

			if (rcItem.IsRectEmpty())
			{
				// nothing to paint
				*pResult = CDRF_DODEFAULT;
				break;
			}

			CDC dc;
			dc.Attach(pNMCustomDraw->hdc);


			CRect rcLabel;
			m_Tree.GetItemRect(hItem, &rcLabel, TRUE);

			COLORREF crTextBk = pNMTVCustomDraw->clrTextBk;
			COLORREF crWnd = GetSysColor((IsWindowEnabled()?COLOR_WINDOW:COLOR_BTNFACE));


#ifndef _OWNER_DRAWN_TREE
			// clear the original label rectangle
			dc.FillSolidRect(&rcLabel, crWnd);
#endif //_OWNER_DRAWN_TREE

			int nColsCnt = m_Header.GetItemCount();

			// draw horizontal lines...
			int xOffset = 0;
			for (int i=0; i<nColsCnt; i++)
			{
				xOffset += m_arrColWidths[i];
				rcItem.right = xOffset-1;
				dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_RIGHT);
			}
			// ...and the vertical ones
			dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_BOTTOM);

			CString strText = m_Tree.GetItemText(hItem);
			CString strSub;
			AfxExtractSubString(strSub, strText, 0, '\t');

			// calculate main label's size
			CRect rcText(0,0,0,0);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_CALCRECT);
			rcLabel.right = min(rcLabel.left + rcText.right + 4, m_arrColWidths[0] - 4);

			BOOL bFullRowSelect = m_Tree.GetStyle()&TVS_FULLROWSELECT;

			if (rcLabel.Width() < 0)
				crTextBk = crWnd;
			if (crTextBk != crWnd)	// draw label's background
			{
				CRect rcSelect =  rcLabel;
				if(bFullRowSelect) rcSelect.right = rcItem.right;

				dc.FillSolidRect(&rcSelect, crTextBk);

				// draw focus rectangle if necessary
				if (pNMCustomDraw->uItemState & CDIS_FOCUS)
					dc.DrawFocusRect(&rcSelect);

			}

			// draw main label

			CFont* pOldFont = NULL;
			if(m_Tree.GetStyle()&TVS_TRACKSELECT && pNMCustomDraw->uItemState && CDIS_HOT)
			{
				LOGFONT lf;
				pOldFont = m_Tree.GetFont();
				pOldFont->GetLogFont(&lf);
				lf.lfUnderline = 1;
				CFont newFont;
				newFont.CreateFontIndirect(&lf);
				dc.SelectObject(newFont);
			}

			rcText = rcLabel;
			rcText.DeflateRect(2, 1);
			dc.SetTextColor(pNMTVCustomDraw->clrText);
			dc.DrawText(strSub, &rcText, DT_VCENTER | DT_SINGLELINE | 
				DT_NOPREFIX | DT_END_ELLIPSIS);

			xOffset = m_arrColWidths[0];
			dc.SetBkMode(TRANSPARENT);

			if(IsWindowEnabled() && !bFullRowSelect)	
				dc.SetTextColor(::GetSysColor(COLOR_MENUTEXT));

			// set not underlined text for subitems
			if( pOldFont &&  !(m_Tree.GetStyle()& TVS_FULLROWSELECT)) 
				dc.SelectObject(pOldFont);


			// draw other columns text
			for (int i=1; i<nColsCnt; i++)
			{
				if (AfxExtractSubString(strSub, strText, i, '\t'))
				{
					rcText = rcLabel;
					rcText.left = xOffset+ COLUMN_MARGIN;
					rcText.right = xOffset + m_arrColWidths[i]-COLUMN_MARGIN;
					rcText.DeflateRect(m_xOffset, 1, 2, 1);
					if(rcText.left<0 || rcText.right<0 || rcText.left>=rcText.right)
					{
						xOffset += m_arrColWidths[i];
						continue;
					}
					DWORD dwFormat = m_arrColFormats[i]&HDF_RIGHT?
DT_RIGHT:(m_arrColFormats[i]&HDF_CENTER?DT_CENTER:DT_LEFT);

					dc.DrawText(strSub, &rcText, DT_SINGLELINE |DT_VCENTER 
						| DT_NOPREFIX | DT_END_ELLIPSIS | dwFormat);




				}
				xOffset += m_arrColWidths[i];
			}


			pNMTVCustomDraw->clrTextBk = (RGB(255,0,0));



			if(pOldFont) 
				dc.SelectObject(pOldFont);
			dc.Detach();
		}
		*pResult = CDRF_DODEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
	}
}


void CColumnTreeCtrl::UpdateColumns()
{
	m_cxTotal = 0;

	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
	int nCnt = m_Header.GetItemCount();

	ASSERT(nCnt<=MAX_COLUMN_COUNT);

	// get column widths from the header control
	for (int i=0; i<nCnt; i++)
	{
		if (m_Header.GetItem(i, &hditem))
		{
			m_cxTotal += m_arrColWidths[i] = hditem.cxy;
			if (i==0)
				m_Tree.m_nFirstColumnWidth = hditem.cxy;
		}
	}
	m_bHeaderChangesBlocked = TRUE;
	RepositionControls();
}


void CColumnTreeCtrl::RepositionControls()
{
	// reposition child controls
	if (m_Tree.m_hWnd)
	{

		CRect rcClient;
		GetClientRect(&rcClient);
		int cx = rcClient.Width();
		int cy = rcClient.Height();

		// get dimensions of scroll bars
		int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
		int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);

		// determine if vertical scroll bar is visible
		SCROLLINFO scrinfo;
		scrinfo.cbSize = sizeof(scrinfo);
		m_Tree.GetScrollInfo(SB_VERT,&scrinfo,SIF_ALL);
		BOOL bVScrollVisible = scrinfo.nMin!=scrinfo.nMax?TRUE:FALSE;

		// determine full header width
		int cxTotal = m_cxTotal+(bVScrollVisible?cxVScroll:0);

		if (m_xPos > cxTotal - cx) m_xPos = cxTotal - cx;
		if (m_xPos < 0)	m_xPos = 0;

		scrinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrinfo.nPage = cx;
		scrinfo.nMin = 0;
		scrinfo.nMax = cxTotal;
		scrinfo.nPos = m_xPos;
		m_horScroll.SetScrollInfo(&scrinfo);

		CRect rcTree;
		m_Tree.GetClientRect(&rcTree);

		// move to a negative offset if scrolled horizontally
		int x = 0;
		if (cx < cxTotal)
		{
			x = m_horScroll.GetScrollPos();
			cx += x;
		}

		// show horisontal scroll only if total header width is greater
		// than the client rect width and cleint rect height is big enough
		BOOL bHScrollVisible = rcClient.Width()<cxTotal 
			&& rcClient.Height()>=cyHScroll+m_cyHeader;

		m_horScroll.ShowWindow(bHScrollVisible?SW_SHOW:SW_HIDE);

		m_Header.MoveWindow(-x, 0, cx  - (bVScrollVisible?cxVScroll:0), m_cyHeader);

		//m_Header2.MoveWindow(rcClient.Width()-cxVScroll, 0, cxVScroll, m_cyHeader);

		m_Tree.MoveWindow(-x, m_cyHeader, cx, cy-m_cyHeader-(bHScrollVisible?cyHScroll:0));

		m_horScroll.MoveWindow(0, rcClient.Height()-cyHScroll,
			rcClient.Width() - (bVScrollVisible?cxVScroll:0), cyHScroll);


		// show the second header at the top-right corner 
		// only when vertical scroll bar is visible
		//m_Header2.ShowWindow(bVScrollVisible?SW_SHOW:SW_HIDE);

		RedrawWindow();
	}
}
int CColumnTreeCtrl::GetColumnWidth(int nCol)
{
	HDITEM hditem = {0};
	hditem.mask = HDI_WIDTH ;
 	CHeaderCtrl& header = GetHeaderCtrl();
 
	if (header.GetItem(nCol, &hditem))
	{
		return hditem.cxy  ;
	}
 

	return 0;
}

int CColumnTreeCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem)
{
	// update the header control in upper-right corner
	// to make it look the same way as main header

	CHeaderCtrl& header = GetHeaderCtrl();

 
	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = nFormat;
	hditem.cxy = nWidth;
	hditem.pszText = (LPTSTR)lpszColumnHeading;
	m_arrColFormats[nCol] = nFormat;
	int indx =  header.InsertItem(nCol, &hditem);

	if(m_Header.GetItemCount()>0) 
	{
		// if the main header contains items, 
		// insert an item to m_Header2
		hditem.pszText = _T("");
		hditem.cxy = GetSystemMetrics(SM_CXVSCROLL)+5;
		//m_Header2.InsertItem(0,&hditem);
	};
	UpdateColumns();

	return indx;
}

BOOL  CColumnTreeCtrl::DeleteColumn(int nCol)
{
	// update the header control in upper-right corner
	// to make it look the same way as main header

	BOOL bResult = m_Header.DeleteItem(nCol);
/*
	if(m_Header.GetItemCount()==0) 
	{
		//m_Header2.DeleteItem(0);
	}*/

	UpdateColumns();
	return bResult;
}

CString CColumnTreeCtrl::GetItemText(HTREEITEM hItem, int nColumn)
{
	// retreive and return the substring from tree item's text
	CString szText = m_Tree.GetItemText(hItem);
	CString szSubItem;
	AfxExtractSubString(szSubItem,szText,nColumn,'\t');
	return szSubItem;
}

void CColumnTreeCtrl::SetItemText(HTREEITEM hItem,int nColumn,LPCTSTR lpszItem)
{
	CString szText = m_Tree.GetItemText(hItem);
	CString szNewText, szSubItem;
	int i;
	for(i=0;i<m_Header.GetItemCount();i++)
	{
		AfxExtractSubString(szSubItem,szText,i,'\t');
		if(i!=nColumn) 
			szNewText+=szSubItem+_T("\t");
		else 
			szNewText+=CString(lpszItem)+_T("\t");
	}
	m_Tree.SetItemText(hItem,szNewText);
}

void CColumnTreeCtrl::SetFirstColumnMinWidth(UINT uMinWidth)
{
	// set minimum width value for the first column
	m_uMinFirstColWidth = uMinWidth;
}

// Call this function to determine the location of the specified point 
// relative to the client area of a tree view control.
HTREEITEM CColumnTreeCtrl::HitTest(CPoint pt, UINT* pFlags) const
{
	CTVHITTESTINFO htinfo = {pt, 0, NULL, 0};
	HTREEITEM hItem = HitTest(&htinfo);
	if(pFlags)
	{
		*pFlags = htinfo.flags;
	}
	return hItem;
}

// Call this function to determine the location of the specified point 
// relative to the client area of a tree view control.
HTREEITEM CColumnTreeCtrl::HitTest(CTVHITTESTINFO* pHitTestInfo) const
{
	// We should use our own HitTest() method, because our custom tree
	// has different layout than the original CTreeCtrl.

	UINT uFlags = 0;
	HTREEITEM hItem = NULL;
	CRect rcItem, rcClient;

	CPoint point = pHitTestInfo->pt;
	point.x += m_xPos;
	point.y -= m_cyHeader;

	hItem = m_Tree.HitTest(point, &uFlags);

	// Fill the CTVHITTESTINFO structure
	pHitTestInfo->hItem = hItem;
	pHitTestInfo->flags = uFlags;
	pHitTestInfo->iSubItem = 0;

	if (! (uFlags&TVHT_ONITEMLABEL || uFlags&TVHT_ONITEMRIGHT) )
		return hItem;

	// Additional check for item's label.
	// Determine correct subitem's index.

	int i;
	int x = 0;
	for(i=0; i<m_Header.GetItemCount(); i++)
	{
		if(point.x>=x && point.x<=x+m_arrColWidths[i])
		{
			pHitTestInfo->iSubItem = i;
			pHitTestInfo->flags = TVHT_ONITEMLABEL;
			return hItem;
		}
		x += m_arrColWidths[i];
	}	

	pHitTestInfo->hItem = NULL;
	pHitTestInfo->flags = TVHT_NOWHERE;
	return NULL;
}


#define ID_FIRST1 10001
BOOL CColumnTreeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	// we need to forward all notifications to the parent window,
	// so use OnNotify() to handle all notifications in one step

	LPNMHDR pHdr = (LPNMHDR)lParam;

	// there are several notifications we need to precess
	HD_NOTIFY *pHDNotify = (HD_NOTIFY*)lParam;
	if (pHDNotify->hdr.code == HDN_ITEMCLICKA || pHDNotify->hdr.code == HDN_ITEMCLICKW)
	{
		SortColumn(pHDNotify->iItem, !m_bAscending);
		return TRUE;
	}

	if ((((LPNMHDR)lParam)->code == NM_RCLICK) && pHdr->idFrom == HeaderID)  
	{
		CMenu menu;
		menu.CreatePopupMenu();

		//TCHAR str[256];
		int nColNum = 0;
		CString strColumnName[20];//假如有255列

		TCHAR buf1[256]; 
		HDITEM hditem;
		hditem.mask = HDI_TEXT |HDI_WIDTH;
		hditem.pszText = buf1; 
		hditem.cchTextMax = 255; 

		int nCnt = m_Header.GetItemCount();
		for(int i = 0 ; i < nCnt; i++)
		{ 
			if (m_Header.GetItem(i, &hditem))
			{
				strColumnName[nColNum] = hditem.pszText;
				int n = hditem.cxy;
				menu.AppendMenu(MF_STRING,ID_FIRST1 + nColNum,strColumnName[nColNum]);
				menu.CheckMenuItem(ID_FIRST1 + nColNum, (n == 0) ? MF_UNCHECKED : MF_CHECKED);
				nColNum++;
			}
		}

		CPoint	p;
		GetCursorPos(&p);
		//CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN|TPM_RIGHTBUTTON,p.x,p.y,this);
		menu.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
		menu.DestroyMenu();
		return TRUE;
	}  


	if(pHdr->code==HDN_ITEMCHANGED && pHdr->idFrom == HeaderID)
	{
		OnHeaderItemChanged(pHdr,pResult);
		return TRUE; // do not forward
	}

	if(pHdr->code==NM_CUSTOMDRAW && pHdr->idFrom == TreeID)
	{
		OnTreeCustomDraw(pHdr,pResult);
		return TRUE; // do not forward
	}


#ifdef _OWNER_DRAWN_TREE

	if(pHdr->code==TVN_ITEMEXPANDING && pHdr->idFrom == TreeID)
	{
		// avoid bitmap scrolling 
		Invalidate(); // ... and forward
	}

#endif //_OWNER_DRAWN_TREE

	if(pHdr->code==TVN_ITEMEXPANDED && pHdr->idFrom == TreeID)
	{
		RepositionControls(); // ... and forward
	}

	if (pHdr->code == TVN_BEGINDRAG && pHdr->idFrom == TreeID)
	{
		return CStatic::OnNotify(wParam, lParam, pResult);
	}

	// forward notifications from children to the control owner
	pHdr->hwndFrom = GetSafeHwnd();
	pHdr->idFrom = GetWindowLong(GetSafeHwnd(),GWL_ID);
	return (BOOL)GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);

}




void CColumnTreeCtrl::OnCancelMode()
{
	m_Header.SendMessage(WM_CANCELMODE);
	//m_Header2.SendMessage(WM_CANCELMODE);
	m_Tree.SendMessage(WM_CANCELMODE);
	m_horScroll.SendMessage(WM_CANCELMODE);
}

void CColumnTreeCtrl::OnEnable(BOOL bEnable)
{
	m_Header.EnableWindow(bEnable);
	//m_Header2.EnableWindow(bEnable);
	m_Tree.EnableWindow(bEnable);
	m_horScroll.EnableWindow(bEnable);
}

#include "IniFile.h"
extern ListHead ListHead_User[];

BOOL CColumnTreeCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
 
    int menuID = LOWORD(wParam) - ID_FIRST1;

	CString str;
	str.Format("Width%d",menuID);

 	
	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
    m_Header.GetItem(menuID, &hditem);
	if(GetColumnWidth(menuID) == 0)
		hditem.cxy = ListHead_User[menuID].nWeight;
	else 
		hditem.cxy = 0;
	m_Header.SetItem(menuID,&hditem);
	UpdateColumns();
 

	CIniFile	m_IniFile;
	m_IniFile.SetInt("MainHeader_Column_Width", str, GetColumnWidth(menuID));
	 
	return CStatic::OnCommand(wParam, lParam);
}


void CCustomTreeChildCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	if (GetParentItem(pNMTreeView->itemNew.hItem) == NULL)
	{
		return;
	}

	*pResult = 0;

	CImageList* piml = NULL;    // image list 句柄
	POINT ptOffset;
	RECT rcItem;

	// 创建拖动项的位图
	if ((piml = CreateDragImage(pNMTreeView->itemNew.hItem)) == NULL)
	{
		return;
	}

	// 得到拖曳项的区域边界
	if (GetItemRect(pNMTreeView->itemNew.hItem, &rcItem, TRUE))
	{
		CPoint ptDragBegin;
		int nX, nY;
		// 获得项前面的图标的位置，项的区域位置不包括图标 
		ptDragBegin = pNMTreeView->ptDrag;
		ImageList_GetIconSize(piml->GetSafeHandle(), &nX, &nY);
		ptOffset.x = (ptDragBegin.x - rcItem.left) + (nX - (rcItem.right - rcItem.left));
		ptOffset.y = (ptDragBegin.y - rcItem.top) + (nY - (rcItem.bottom - rcItem.top));

		MapWindowPoints(NULL, &rcItem); // 转换该项区域位置到屏幕坐标
	}
	else
	{
		GetWindowRect(&rcItem);
		ptOffset.x = ptOffset.y = 8;
	}

	BOOL bDragBegun = piml->BeginDrag(0, ptOffset); // 开始拖动选中项的位图
	if (!bDragBegun)
	{
		delete piml;
		return;
	}

	CPoint ptDragEnter = pNMTreeView->ptDrag;
	ClientToScreen(&ptDragEnter);
	if (!piml->DragEnter(NULL, ptDragEnter)) // 在指定窗口(NULL表示桌面窗口)的指定位置画拖动项位图
	{
		delete piml;
		return;
	}

	delete piml;


	SetFocus();  // 获得输入焦点
	InvalidateRect(&rcItem, TRUE);  // 重画被拖曳的项
	UpdateWindow(); // 更新控件窗口
	SetCapture(); // 捕获鼠标，只有控件接收鼠标消息

	m_hItemDrag = pNMTreeView->itemNew.hItem; // 获得拖动项的句柄
	//TRACE( "%s \r\n ", GetItemText(m_hItemDrag));
 
	*pResult = 0;
}

void CColumnTreeCtrl::UpdataColor()
{
	RedrawWindow(); // 重画窗口
}

void CCustomTreeChildCtrl::EndDrag(unsigned int nFlags, CPoint point)
{
	if (m_hItemDrag == NULL)
		return;

	CPoint pt;

	pt = point;
	ClientToScreen(&pt);

	BOOL bCopy = (GetKeyState(VK_CONTROL) & 0x10000000); // 如按了CTRL键是拷贝拖动的项


	HTREEITEM hItemDrop = GetDropHilightItem(); // 获得放下拖动项的地方(作为拖动项的父项的句柄)
 
	for (std::vector<HTREEITEM>::const_iterator it = vTemp.begin(); it < vTemp.end(); ++it)
	{
		if (hItemDrop != NULL)
		{
			MoveTreeItem(*it, hItemDrop == NULL ? TVI_ROOT : hItemDrop);
		}
	}
 
	FinishDragging(); // 释放一些资源
	RedrawWindow(); // 重画窗口
	g_pTreeList->UpDateNumber();
}

extern CHpTcpServer* m_iocpServer;
HTREEITEM CCustomTreeChildCtrl::MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo , HTREEITEM hItemPos)
{
	if (hItem == NULL || hItemTo == NULL)
	{
		return NULL;
	}
	if (hItem == hItemTo || hItemTo == GetParentItem(hItem))
	{
		return hItem;
	}
	// 检查是否移动到自己下面，是则返回不操作
	HTREEITEM hItemParent = hItemTo;
	while (hItemParent != TVI_ROOT && (hItemParent = GetParentItem(hItemParent)) != NULL)
	{
		if (hItemParent == hItem)
			return NULL;
	}

	// 通过递归拷贝项到新的位置
	CString sText = GetItemText(hItem);
	ULONG_PTR dwContext = GetItemData(hItem);
	TVINSERTSTRUCT tvis;
	tvis.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvis.item.hItem = hItem;
	// 不拷贝选择，展开的状态
	tvis.item.stateMask = (UINT)-1 & ~(TVIS_DROPHILITED | TVIS_EXPANDED | TVIS_EXPANDEDONCE | TVIS_EXPANDPARTIAL | TVIS_SELECTED);
	GetItem(&tvis.item);
	tvis.hParent = hItemTo;
	tvis.hInsertAfter = hItemPos;
 
	HTREEITEM hItemNew = InsertItem(&tvis);
	SetItemText(hItemNew, sText);
	SetItemData(hItemNew, dwContext);

	// 移动子项到新项下面
	HTREEITEM hItemChild = GetChildItem(hItem);
	while (hItemChild != NULL)
	{
		HTREEITEM hItemNextChild = GetNextSiblingItem(hItemChild);
		MoveTreeItem(hItemChild, hItemNew);
		hItemChild = hItemNextChild;
	}
 
	ClientContext* pContext = (ClientContext*)dwContext;
	if (pContext)
	{
		pContext->m_hItem = hItemNew;


		CString m_NewGroup,strTemp;
		strTemp = GetItemText(hItemTo);
		int n = strTemp.Find(")");
		if (n >0)
		{
			m_NewGroup = strTemp.Mid(n+1);
		}
		else
		{
			m_NewGroup = strTemp;
		}
 
		int	nPacketLength = m_NewGroup.GetLength() + 2;
		BYTE	lpPacket[1024]={0};
		lpPacket[0] = COMMAND_CHANGE_GROUP;
		memcpy(lpPacket + 1,  m_NewGroup.GetBuffer(0), nPacketLength - 1);
		m_iocpServer->Send(pContext, lpPacket, nPacketLength);

	}

	// 如果是移动则需要删除老的项
	SetItemData(hItem, 0);
	DeleteItem(hItem);


	return hItemNew;
}

void CCustomTreeChildCtrl::FinishDragging()
{
	if (m_hItemDrag != NULL)
	{
		CImageList::DragLeave(NULL); // 解锁隐藏的拖动位图
		CImageList::EndDrag(); // 结束拖动

		ReleaseCapture(); // 释放鼠标，让客户区外的控件能捕获鼠标消息
		ShowCursor(TRUE); // 显示隐藏掉的光标
		m_hItemDrag = NULL;
		SelectDropTarget(NULL);
	}
}


void CCustomTreeChildCtrl::PreSubclassWindow()
{
	// TODO:  在此添加专用代码和/或调用基类
	ModifyStyle(TVS_DISABLEDRAGDROP, 0); // 确保树形控件能够拖拽	

	CTreeCtrl::PreSubclassWindow();
}


int CColumnTreeCtrl::GetItemCount()
{
	int i = 0;
	for (HTREEITEM hItem = m_Tree.GetRootItem(); hItem != NULL; hItem = m_Tree.GetNextSiblingItem(hItem))
	{
		HTREEITEM childItem = m_Tree.GetChildItem(hItem);
		while (childItem != NULL)
		{
			i++;
			childItem = m_Tree.GetNextSiblingItem(childItem);
		}
	}
	return i;
}




HTREEITEM CColumnTreeCtrl::FindGroup(char* lpszName)
{
	CString strTemp = "";
	CString strGroup = "";
	for (HTREEITEM hItem = m_Tree.GetRootItem(); hItem != NULL; hItem = m_Tree.GetNextSiblingItem(hItem))
	{
		 strTemp=m_Tree.GetItemText(hItem);

		 int n = strTemp.Find(')');
		 if( n > 0 )
		 {
			 strGroup = strTemp.Mid(n+1);
		 }
		 else
		 {
			 strGroup = strTemp;
		 }

		if ( strGroup == lpszName)
		{
			return hItem;
		}
	}

	return NULL;
}

int CColumnTreeCtrl::GetGroupGount(HTREEITEM hItem)
{
	int nCount = 0;

	HTREEITEM childItem = m_Tree.GetChildItem(hItem);
	while (childItem != NULL)
	{
		nCount++;
		childItem = m_Tree.GetNextSiblingItem(childItem);
	}
	return nCount;
}

BOOL CColumnTreeCtrl::DelGroup(HTREEITEM hItem)
{
	if ( GetGroupGount(hItem) > 0) return FALSE;

	m_Tree.DeleteItem(hItem);
	 
	return TRUE;
}

BOOL CColumnTreeCtrl::UpDateNumber()
{
	CString strTemp = "";
	CString strGroup = "";
	for (HTREEITEM hItem = m_Tree.GetRootItem(); hItem != NULL; hItem = m_Tree.GetNextSiblingItem(hItem))
	{
		strTemp=m_Tree.GetItemText(hItem);

		int n = strTemp.Find(')');
		if( n > 0 )
		{
			strGroup = strTemp.Mid(n+1);
		}
		else
		{
			strGroup = strTemp;
		}

		int nCount = GetGroupGount(hItem);
 
		
	    strTemp.Format("(%d)%s", nCount,strGroup);

		m_Tree.SetItemText(hItem,strTemp);
	}
	return TRUE;
}

HTREEITEM CColumnTreeCtrl::AddGroup(char* lpszName)
{
	HTREEITEM hRoot = m_Tree.InsertItem(lpszName,0,0);
	return hRoot;
}

HTREEITEM CColumnTreeCtrl::AddFindGroup(char* lpszName)
{
	HTREEITEM hRoot = FindGroup(lpszName);
	if (hRoot == NULL)
	{
		CString strGroup;
		strGroup.Format("(1)%s",lpszName);
		hRoot = AddGroup(strGroup.GetBuffer());
	}
	return hRoot;
}

HTREEITEM CColumnTreeCtrl::InsertItem(/*int nItem, */LPCTSTR lpszItem, int nImage,char* lpszGroupName)
{
	// 查找分组 获取分组 item  如果不存在则创建分组
	if (strlen(lpszGroupName) == 0)
	{
		lpszGroupName = "默认分组";
	}
	HTREEITEM hRoot  = AddFindGroup(lpszGroupName);
	// 插入分组数据
	HTREEITEM item = m_Tree.InsertItem(lpszItem, 1,1,hRoot);

	UpDateNumber();
	// 返回 索引 
	return item;
}



int CColumnTreeCtrl::SetItemData(HTREEITEM hItem, DWORD_PTR dwItemData)
{
	ClientContext	*pContext = (ClientContext	*)dwItemData;
	if(pContext){
		pContext->m_hItem = hItem;
	}
	return m_Tree.SetItemData(hItem,dwItemData );
}


int CColumnTreeCtrl::GetSelectedCount()
{
	int nCount = 0;
	for (HTREEITEM hItem = m_Tree.GetRootItem(); hItem != NULL; hItem = m_Tree.GetNextSiblingItem(hItem))
	{
		HTREEITEM childItem = m_Tree.GetChildItem(hItem );
		while (childItem != NULL)
		{
			if (m_Tree.GetItemState(childItem, TVIS_SELECTED) & TVIS_SELECTED)
			{
				nCount++;
			}
			childItem = m_Tree.GetNextSiblingItem(childItem);
		}
	}
	return nCount;
}

int CColumnTreeCtrl::GetSelectedCount_CheckBox()
{
	int nCount = 0;
	for (HTREEITEM hItem = m_Tree.GetRootItem(); hItem != NULL; hItem = m_Tree.GetNextSiblingItem(hItem))
	{
		HTREEITEM childItem = m_Tree.GetChildItem(hItem );
		while (childItem != NULL)
		{
			if (m_Tree.GetCheck(childItem))
			{
				nCount++;
			}
			childItem = m_Tree.GetNextSiblingItem(childItem);
		}
	}
	return nCount;
}


