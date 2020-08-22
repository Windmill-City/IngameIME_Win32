#pragma once

template<typename CPP_CALL, typename CS_CALL>
ref class Delegate
{
public:
	typedef CPP_CALL* PCPP_CALL;
	delegate CPP_CALL CLI_DLG;
	typedef CLI_DLG^ PCLI_CALL;

	delegate CS_CALL CS_DLG;
	typedef CS_DLG^ EVENT;

	static PCPP_CALL GetPointerForNative(PCLI_CALL call) {
		return reinterpret_cast<PCPP_CALL>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(call).ToPointer());
	}
};
