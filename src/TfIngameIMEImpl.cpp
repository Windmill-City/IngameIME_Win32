#include "tf/TfIngameIMEImpl.hpp"

namespace IngameIME::tf

{

GlobalImpl::GlobalImpl()
{
    COM_HR_BEGIN(S_OK);

    handler = new InputProcessorHandler();

    ComQIPtr<ITfSource> source(IID_ITfSource, handler->mode);
    CHECK_HR(source->AdviseSink(IID_ITfCompartmentEventSink,
                                static_cast<ITfCompartmentEventSink*>(handler.get()),
                                &cookieComp));

    source = handler->compMgr;
    CHECK_HR(source->AdviseSink(IID_ITfInputProcessorProfileActivationSink,
                                static_cast<ITfInputProcessorProfileActivationSink*>(handler.get()),
                                &cookieProc));

    COM_HR_END();
    COM_HR_THR();
}

GlobalImpl::~GlobalImpl()
{
    if (cookieProc != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, handler);
        source->UnadviseSink(cookieProc);
        cookieProc = TF_INVALID_COOKIE;
    }

    if (cookieComp != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, handler->mode);
        source->UnadviseSink(cookieComp);
        cookieComp = TF_INVALID_COOKIE;
    }
}

std::shared_ptr<const InputProcessor> GlobalImpl::getActiveInputProcessor() const
{
    return InputProcessorImpl::getActiveInputProcessor();
}

std::list<std::shared_ptr<const InputProcessor>> GlobalImpl::getInputProcessors() const
{
    return InputProcessorImpl::getInputProcessors();
}

std::shared_ptr<InputContext> GlobalImpl::getInputContext(void* hWnd, ...)
{
    return std::make_shared<InputContextImpl>(reinterpret_cast<HWND>(hWnd));
}

} // namespace IngameIME::tf
