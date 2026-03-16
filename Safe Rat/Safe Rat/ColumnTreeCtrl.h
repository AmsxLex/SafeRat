#pragma once
#include "resource.h"

#ifdef _OWNER_DRAWN_TREE
#ifndef IDB_TREEBTNS
	#error You should insert IDB_TREEBTNS bitmap to project resources. See control documentation for more info.
#endif //IDB_TREEBTNS
#endif //_OWNER_DRAWN_TREE

typedef struct _CTVHITTESTINFO { 
  POINT pt; 
  UINT flags; 
  HTREEITEM hItem; 
  int iSubItem;
} CTVHITTESTINFO;


class CCustomTreeChildCtrl : public CTreeCtrl
{
	friend class CColumnTreeCtrl;

	DECLARE_DYNAMIC(CCustomTreeChildCtrl)

public:

	/*
	 *  Construction/destruction
	 */
	
	CCustomTreeChildCtrl();
	virtual ~CCustomTreeChildCtrl();

	
	void SetChildCheck(HTREEITEM hTree,BOOL bCheck);
	BOOL SelectItems( HTREEITEM hFromItem, HTREEITEM hToItem );
	void ClearSelection(BOOL bMultiOnly=FALSE) ;
	HTREEITEM   m_hFirstSelectedItem;  
	HTREEITEM GetFirstSelectedItem();
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem);
	/*
	 * Operations
	 */
	
	BOOL GetBkImage(LVBKIMAGE* plvbkImage) const;
	BOOL SetBkImage(LVBKIMAGE* plvbkImage);

protected:
	DECLARE_MESSAGE_MAP()

	int m_nFirstColumnWidth; // the width of the first column 
	int m_nOffsetX;      	 // offset of this window inside the parent 
	LVBKIMAGE m_bkImage;	 // information about background image
	CImageList m_imgBtns;	 // tree buttons images (IDB_TREEBTNS)

	BOOL CheckHit(CPoint point);

	
	/*
	 * Message Handlers
	 */

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
 
	/*
	 * Custom drawing related methods
	 */

#ifdef _OWNER_DRAWN_TREE
	LRESULT CustomDrawNotify(LPNMTVCUSTOMDRAW lpnm);
	LRESULT OwnerDraw(CDC* pDC);
	int OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
#endif //_OWNER_DRAWN_TREE

public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	HTREEITEM MoveTreeItem(HTREEITEM hItem, HTREEITEM hItemTo,HTREEITEM hItemPos = TVI_LAST);
	void FinishDragging();
private:
	typedef LPARAM(*PFNMTICOPYDATA)(HTREEITEM, LPARAM);

	HTREEITEM m_hItemDrag;

	void EndDrag(unsigned int nFlags, CPoint point);

	virtual void PreSubclassWindow();
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


class CColumnTreeCtrl : public CStatic
{
public:
	DECLARE_DYNCREATE(CColumnTreeCtrl)
	
	/*
	 * Construction/destruction
	 */
	 
	CColumnTreeCtrl();
	virtual ~CColumnTreeCtrl();

	void UpdataColor();
	// explicit construction 
	BOOL Create(DWORD dwStyle , const RECT& rect, CWnd* pParentWnd, UINT nID);

	virtual void PreSubclassWindow();

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	int             m_nSortedCol;
	bool            m_bAscending;
	void SortColumn(int iCol, bool bAsc);

	// new =====================================================================================

	int GetItemCount();
	int SetItemData(HTREEITEM hItem, DWORD_PTR dwItemData);
	// CString GetItemText(int nItem,int nSubItem);
	// void SetItemText(int hItem, int nSubItem, LPCTSTR lpszText);
	// void DeleteItem( int hItem);
	int GetSelectedCount();
	int GetSelectedCount_CheckBox();

	// int GetSelectionMark();
/*
	HTREEITEM  GetFirstSelectedItemPosition();
    int GetNextSelectedItem(HTREEITEM pos);*/


	HTREEITEM InsertItem(/*int nItem, */LPCTSTR lpszItem, int nImage ,char* lpszGroupName ="Ä¬ÈÏ·Ö×é");

	// HTREEITEM  GethItemFromIndex(int nIndex);
	//UINT GetitemState(int hItem, UINT nStateMask);
	//DWORD GetItemData(int nIndex);
	//BOOL SetItemState(int hItem, UINT nState, UINT nStateMask);
	//UINT SetCheck(int hItem ,BOOL b);
	//BOOL GetCheck(int hItem );

	HTREEITEM FindGroup(char* lpszName);
	HTREEITEM AddGroup(char* lpszName);
	HTREEITEM AddFindGroup(char* lpszName);
	BOOL UpDateNumber();
	BOOL DelGroup(HTREEITEM hItem);
	int GetGroupGount(HTREEITEM hItem);




	/*
	 *  Operations
	 */

	virtual void AssertValid( ) const; 
	CCustomTreeChildCtrl& GetTreeCtrl() { return m_Tree; }
	CHeaderCtrl& GetHeaderCtrl() { return m_Header; }

	int InsertColumn(int nCol,LPCTSTR lpszColumnHeading, int nFormat=0, int nWidth=-1, int nSubItem=-1);
	BOOL DeleteColumn(int nCol);
	int GetColumnWidth(int nCol);



	void SetFirstColumnMinWidth(UINT uMinWidth);
		
	CString GetItemText(HTREEITEM hItem, int nSubItem);
	void SetItemText(HTREEITEM hItem, int nSubItem, LPCTSTR lpszText);

	HTREEITEM HitTest(CPoint pt, UINT* pFlags=NULL) const;
	HTREEITEM HitTest(CTVHITTESTINFO* pHitTestInfo) const;
	
protected:
	
	DECLARE_MESSAGE_MAP()

	enum ChildrenIDs { HeaderID = 1, TreeID = 2, HScrollID = 3, Header2ID = 4};
	
	CCustomTreeChildCtrl m_Tree;
	CScrollBar m_horScroll;
	CXTPHeaderCtrl m_Header;

	
	int m_cyHeader;
	int m_cxTotal;
	int m_xPos;
	int m_xOffset;
	int m_uMinFirstColWidth;
	BOOL m_bHeaderChangesBlocked;

	enum{MAX_COLUMN_COUNT=16}; // change this value if you need more than 16 columns

	int m_arrColWidths[MAX_COLUMN_COUNT];
	DWORD m_arrColFormats[MAX_COLUMN_COUNT];
	
	virtual void Initialize();
	void UpdateColumns();
	void RepositionControls();


	virtual void OnDraw(CDC* pDC) {}
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHeaderItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCancelMode();
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};