// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// demo1-officeView.cpp: Cdemo1officeView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "demo1-office.h"
#endif

#include "demo1-officeDoc.h"
#include "demo1-officeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cdemo1officeView

IMPLEMENT_DYNCREATE(Cdemo1officeView, CView)

BEGIN_MESSAGE_MAP(Cdemo1officeView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &Cdemo1officeView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// Cdemo1officeView 构造/析构

Cdemo1officeView::Cdemo1officeView() noexcept
{
	// TODO: 在此处添加构造代码

}

Cdemo1officeView::~Cdemo1officeView()
{
}

BOOL Cdemo1officeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// Cdemo1officeView 绘图

void Cdemo1officeView::OnDraw(CDC* /*pDC*/)
{
	Cdemo1officeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// Cdemo1officeView 打印


void Cdemo1officeView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL Cdemo1officeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void Cdemo1officeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void Cdemo1officeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void Cdemo1officeView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void Cdemo1officeView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// Cdemo1officeView 诊断

#ifdef _DEBUG
void Cdemo1officeView::AssertValid() const
{
	CView::AssertValid();
}

void Cdemo1officeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

Cdemo1officeDoc* Cdemo1officeView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Cdemo1officeDoc)));
	return (Cdemo1officeDoc*)m_pDocument;
}
#endif //_DEBUG


// Cdemo1officeView 消息处理程序
