#include "TextBox.h"

TextBox::TextBox(HWND hWnd)
{
	m_hWnd = hWnd;
}

VOID TextBox::Draw(HWND hwnd, HDC hdc, PAINTSTRUCT* ps)
{
	using namespace Gdiplus;
	Gdiplus::Font fontText(L"Microsoft Yahei", 12);
	Graphics graphics(hdc);
	m_rectComp.bottom = m_rectComp.top + fontText.GetHeight(&graphics);
	Gdiplus::SolidBrush BrushFront(Color(255, 34, 142, 230));
	Gdiplus::StringFormat format = Gdiplus::StringFormat::GenericTypographic();
	PointF origin(8.0f, 8.0f);
	graphics.DrawString(m_Text.c_str(), m_Text.size(), &fontText, origin, &format, &BrushFront);
	RectF rectf;
	graphics.MeasureString(m_Text.c_str(), (INT)m_Text.size(), &fontText, origin, &rectf);
	m_rectComp.left = rectf.X + rectf.Width;
	origin.X = m_rectComp.left;
	graphics.DrawString(m_CompText.c_str(), m_CompText.size(), &fontText, origin, &format, &BrushFront);
	graphics.MeasureString(m_CompText.c_str(), (INT)m_Text.size(), &fontText, origin, &rectf);
	m_rectComp.right = m_rectComp.left + rectf.Width;
}

VOID TextBox::onKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_BACK && m_Text.size() > 0)
		m_Text.erase(m_Text.size() - 1);
	InvalidateRect(m_hWnd, NULL, NULL);
}

VOID TextBox::onKeyUp(WPARAM wParam, LPARAM lParam)
{
	InvalidateRect(m_hWnd, NULL, NULL);
}

VOID TextBox::GetCompExt(RECT* rect)
{
	InvalidateRect(m_hWnd, NULL, NULL);
	SetRect(rect, m_rectComp.left, m_rectComp.top, m_rectComp.right, m_rectComp.bottom);
}

VOID TextBox::QueryInsert(ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd)
{
	if (m_Text.size() + m_CompText.size() < m_maxLength)
		return;
	pacpResultStart = NULL;
}