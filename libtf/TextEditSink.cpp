#include "pch.h"
#include "TextEditSink.h"
#include "TextStore.h"

TextEditSink::TextEditSink(Context* ctx)
{
	m_Ctx = ctx;
	Microsoft::WRL::ComPtr<ITfSource> source;

	HRESULT hr = m_Ctx->m_pCtx.As(&source);
	THROWHR(hr, "Failed to As ITfSource");

	hr = source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &m_Cookie);
	THROWHR(hr, "Failed to AdviseSink");
}

TextEditSink::~TextEditSink()
{
	if (m_Cookie != TF_INVALID_COOKIE) {
		Microsoft::WRL::ComPtr<ITfSource> source;
		HRESULT hr = m_Ctx->m_pCtx.As(&source);
		THROWHR(hr, "Failed to As ITfSource");

		hr = source->UnadviseSink(m_Cookie);
		THROWHR(hr, "Failed to UnadviseSink");
	}
}

HRESULT __stdcall TextEditSink::OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord)
{
	TextStore* _TextStore = dynamic_cast<TextStore*> (m_Ctx->m_pTextStore.Get());
	if (m_Ctx->m_pCtx.Get() != pic || !_TextStore)
		return S_OK;
	_TextStore->m_Composing = FALSE;
	_TextStore->m_Commit = FALSE;
	_TextStore->m_CompStart = _TextStore->m_CompEnd = 0;

	const GUID* rgGuids[2] = { &GUID_PROP_COMPOSING,
								&GUID_PROP_ATTRIBUTE };

	Microsoft::WRL::ComPtr <ITfReadOnlyProperty> TrackProperty;
	Microsoft::WRL::ComPtr<ITfRange> Start2EndRange;
	Microsoft::WRL::ComPtr<ITfRange> EndRange;

	if (FAILED(pic->TrackProperties(rgGuids, 2, NULL, 0, &TrackProperty))) {
		return E_FAIL;
	}

	if (FAILED(pic->GetStart(ecReadOnly, &Start2EndRange))) {
		return E_FAIL;
	}
	if (FAILED(pic->GetEnd(ecReadOnly, &EndRange)))
		return E_FAIL;
	if (FAILED(Start2EndRange->ShiftEndToRange(
		ecReadOnly, EndRange.Get(), TF_ANCHOR_END))) {
		return E_FAIL;
	}

	Microsoft::WRL::ComPtr<IEnumTfRanges> Ranges;
	if (FAILED(TrackProperty->EnumRanges(ecReadOnly, &Ranges,
		Start2EndRange.Get()))) {
		return E_FAIL;
	}

	while (TRUE) {
		Microsoft::WRL::ComPtr<ITfRange> Range;
		ULONG cFetched;

		if (Ranges->Next(1, &Range, &cFetched) != S_OK)
			break;
		VARIANT var;
		VariantInit(&var);
		Microsoft::WRL::ComPtr<IEnumTfPropertyValue> EnumPropValue;

		if (FAILED(TrackProperty->GetValue(ecReadOnly, Range.Get(),
			&var))) {
			return E_FAIL;
		}

		if (FAILED(var.punkVal->QueryInterface(IID_PPV_ARGS(&EnumPropValue)))) {
			return E_FAIL;
		}
		VariantClear(&var);

		TF_PROPERTYVAL PropValue;
		BOOL IsComposing = FALSE;

		while (EnumPropValue->Next(1, &PropValue, &cFetched) == S_OK) {
			if (IsEqualGUID(PropValue.guidId, GUID_PROP_COMPOSING)) {
				IsComposing = (PropValue.varValue.lVal == TRUE);
				break;
			}
			VariantClear(&PropValue.varValue);
		}

		Microsoft::WRL::ComPtr<ITfRangeACP> RangeACP;
		Range.As(&RangeACP);
		LONG AcpStart, Len;
		RangeACP->GetExtent(&AcpStart, &Len);

		if (IsComposing) {
			if (!_TextStore->m_Composing) {
				_TextStore->m_Composing = TRUE;
				_TextStore->m_CompStart = _TextStore->m_CompEnd = AcpStart;
			}
			_TextStore->m_CompEnd += Len;
		}
		else {
			_TextStore->m_CommitStart = AcpStart;
			_TextStore->m_CommitEnd = AcpStart + Len;
		}
		_TextStore->m_Commit = _TextStore->m_CommitEnd - _TextStore->m_CommitStart > 0;
	}
	return S_OK;
}

TF_COM_QUERYINF(TextEditSink, TF_COM_ASUNK(ITfTextEditSink))