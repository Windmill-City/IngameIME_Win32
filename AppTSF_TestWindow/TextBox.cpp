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
	PointF origin(8.0f, 8.0f);
	m_rectComp.top = origin.Y;
	//should use Font height, because some IME draw CompStr themselves, when CompStr is Empty
	//so the candidate window wont cover the text
	m_rectComp.bottom = m_rectComp.top + fontText.GetHeight(&graphics);

	Gdiplus::SolidBrush BrushFront(Color(255, 34, 142, 230));
	Gdiplus::StringFormat format = Gdiplus::StringFormat::GenericTypographic();
	//Draw Text
	graphics.DrawString(m_Text.c_str(), m_Text.size(), &fontText, origin, &format, &BrushFront);
	//Measure Text
	RectF rectfText;
	graphics.MeasureString(m_Text.c_str(), (INT)m_Text.size(), &fontText, origin, &rectfText);
	//Map rect Comp
	m_rectComp.left = rectfText.X + rectfText.Width;
	//Draw Comp
	PointF originComp(m_rectComp.left, origin.Y);
	graphics.DrawString(m_CompText.c_str(), m_CompText.size(), &fontText, originComp, &format, &BrushFront);
	//Measure Comp
	RectF rectfComp;
	graphics.MeasureString(m_CompText.c_str(), (INT)m_CompText.size(), &fontText, originComp, &rectfComp);
	//Map Comp
	m_rectComp.right = m_rectComp.left + rectfComp.Width;
	RectF rectfCaret;
	std::wstring toMeasure = m_CompText.substr(m_CaretPos);
	graphics.MeasureString(toMeasure.c_str(), (INT)toMeasure.size(), &fontText, originComp, &rectfCaret);
	//Draw Caret
	graphics.FillRectangle(&BrushFront, (int)(m_rectComp.right - rectfCaret.Width), m_rectComp.top, 2, (int)fontText.GetHeight(&graphics));
	//Draw Candidates
	int yOffset = fontText.GetHeight(&graphics);
	for (size_t i = 0; i < Count; i++)
	{
		std::wstring str = Candidates[i];
		origin.Y += yOffset;
		graphics.DrawString(str.c_str(), str.size(), &fontText, origin, &format, &BrushFront);
	}
}

VOID TextBox::GetCompExt(RECT* rect)
{
	InvalidateRect(m_hWnd, NULL, NULL);
	SetRect(rect, m_rectComp.left, m_rectComp.top, m_rectComp.right, m_rectComp.bottom);
}

VOID TextBox::onChar(WPARAM wParam, LPARAM lParam)
{
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