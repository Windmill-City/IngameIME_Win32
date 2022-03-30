#pragma once
#include <algorithm>
#include <list>
#include <map>

#include <windows.h>
#pragma comment(lib, "imm32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <msctf.h>

#include "InputProcessor.hpp"

#include "FormatUtil.hpp"
#include "Singleton.hpp"
#include "tf\ComBSTR.hpp"
#include "tf\ComObjectBase.hpp"
#include "tf\ComPtr.hpp"
#include "tf\TfFunction.hpp"

namespace IngameIME
{
class InternalLocale
    : public Locale
    , public Singleton<InternalLocale, LANGID>
{
  protected:
    const LANGID langId;

  protected:
    static std::wstring getLocaleString(const LANGID langid);
    static std::wstring getLocaleName(const LANGID langid);

  public:
    InternalLocale(const LANGID langId) noexcept;
};

struct ComparableProfile : TF_INPUTPROCESSORPROFILE
{
    ComparableProfile(const TF_INPUTPROCESSORPROFILE&);
    bool operator<(const ComparableProfile& s2) const;
};

class InputProcessorImpl
    : public InputProcessor
    , public Singleton<InputProcessorImpl, ComparableProfile>
{
  protected:
    /**
     * @brief Check if the HKL correspond to a Imm InputMethod
     *
     * @param hkl hkl to check
     * @return true - Imm InputMethod
     * @return false - Normal KeyboardLayout
     */
    static bool isImm(HKL hkl);

    static std::wstring getInputProcessorName(const TF_INPUTPROCESSORPROFILE& profile);
    static std::wstring getKeyboardLayoutName(LANGID langId);
    static std::wstring getTextServiceName(const TF_INPUTPROCESSORPROFILE& profile);
    static std::wstring getImmName(HKL hkl);

  protected:
    const TF_INPUTPROCESSORPROFILE profile;

  public:
    static std::shared_ptr<const InputProcessorImpl>        getActiveInputProcessor();
    static std::list<std::shared_ptr<const InputProcessor>> getInputProcessors();

  public:
    /**
     * @brief If this is a Japanese InputProcessor
     *
     */
    bool isJap;

  public:
    InputProcessorImpl(const TF_INPUTPROCESSORPROFILE& profile);

  public:
    virtual void setActivated() const override;
};
} // namespace IngameIME