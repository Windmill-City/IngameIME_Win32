# libtf
A library for text service framework
# Build
```
cmake --build ./build --config Release --target ALL_BUILD
```
# Known issue
`ITfThreadMgr` will be Marshaled if you `CoCreateInstance` it on a MTA thread,
resulting all the thread relevant call to it delegate to an internal STA thread;

So if you try to call ITfThreadMgr->Activate on a MTA thread, it actually being called
in another thread, and your calling thread does not get Activated at all!

If you try to call `libtf_create_ctx` on a MTA thread, you will get an error, as
the `ITfUIElementMgr` can't be Marshaled.

Do forget `CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)` 
or `CoInitialize(NULL)` before creating context on a new thread!