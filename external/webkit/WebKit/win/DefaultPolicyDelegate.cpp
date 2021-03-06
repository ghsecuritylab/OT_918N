

#include "config.h"
#include "WebKitDLL.h"
#include "DefaultPolicyDelegate.h"

#include "WebKit.h"

#pragma warning(push, 0)
#include <WebCore/PlatformString.h>
#pragma warning(pop)

using namespace WebCore;

// FIXME: move this enum to a separate header file when other code begins to use it.
typedef enum WebExtraNavigationType {
    WebNavigationTypePlugInRequest = WebNavigationTypeOther + 1
} WebExtraNavigationType;

// DefaultPolicyDelegate ----------------------------------------------------------------

DefaultPolicyDelegate::DefaultPolicyDelegate()
    : m_refCount(0)
{
    gClassCount++;
    gClassNameCount.add("DefaultPolicyDelegate");
}

DefaultPolicyDelegate::~DefaultPolicyDelegate()
{
    gClassCount--;
    gClassNameCount.remove("DefaultPolicyDelegate");
}

DefaultPolicyDelegate* DefaultPolicyDelegate::sharedInstance()
{
    static COMPtr<DefaultPolicyDelegate> shared;
    if (!shared)
        shared.adoptRef(DefaultPolicyDelegate::createInstance());
    return shared.get();
}

DefaultPolicyDelegate* DefaultPolicyDelegate::createInstance()
{
    DefaultPolicyDelegate* instance = new DefaultPolicyDelegate();
    instance->AddRef();
    return instance;
}

// IUnknown -------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE DefaultPolicyDelegate::QueryInterface(REFIID riid, void** ppvObject)
{
    *ppvObject = 0;
    if (IsEqualGUID(riid, IID_IUnknown))
        *ppvObject = static_cast<IUnknown*>(this);
    else if (IsEqualGUID(riid, IID_IWebPolicyDelegate))
        *ppvObject = static_cast<IWebPolicyDelegate*>(this);
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE DefaultPolicyDelegate::AddRef()
{
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE DefaultPolicyDelegate::Release()
{
    ULONG newRef = --m_refCount;
    if (!newRef)
        delete(this);

    return newRef;
}

HRESULT STDMETHODCALLTYPE DefaultPolicyDelegate::decidePolicyForNavigationAction(
    /*[in]*/ IWebView* webView, 
    /*[in]*/ IPropertyBag* actionInformation, 
    /*[in]*/ IWebURLRequest* request, 
    /*[in]*/ IWebFrame* /*frame*/, 
    /*[in]*/ IWebPolicyDecisionListener* listener)
{
    int navType = 0;
    VARIANT var;
    if (SUCCEEDED(actionInformation->Read(WebActionNavigationTypeKey, &var, 0))) {
        V_VT(&var) = VT_I4;
        navType = V_I4(&var);
    }
    COMPtr<IWebViewPrivate> wvPrivate(Query, webView);
    if (wvPrivate) {
        BOOL canHandleRequest = FALSE;
        if (SUCCEEDED(wvPrivate->canHandleRequest(request, &canHandleRequest)) && canHandleRequest)
            listener->use();
        else if (navType == WebNavigationTypePlugInRequest)
            listener->use();
        else {
            BSTR url;
            // A file URL shouldn't fall through to here, but if it did,
            // it would be a security risk to open it.
            if (SUCCEEDED(request->URL(&url)) && !String(url, SysStringLen(url)).startsWith("file:")) {
                // FIXME: Open the URL not by means of a webframe, but by handing it over to the system and letting it determine how to open that particular URL scheme.  See documentation for [NSWorkspace openURL]
                ;
            }
            listener->ignore();
            SysFreeString(url);
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DefaultPolicyDelegate::decidePolicyForNewWindowAction(
    /*[in]*/ IWebView* /*webView*/, 
    /*[in]*/ IPropertyBag* /*actionInformation*/, 
    /*[in]*/ IWebURLRequest* /*request*/, 
    /*[in]*/ BSTR /*frameName*/, 
    /*[in]*/ IWebPolicyDecisionListener* listener)
{
    listener->use();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DefaultPolicyDelegate::decidePolicyForMIMEType(
    /*[in]*/ IWebView* webView, 
    /*[in]*/ BSTR type, 
    /*[in]*/ IWebURLRequest* request, 
    /*[in]*/ IWebFrame* /*frame*/, 
    /*[in]*/ IWebPolicyDecisionListener* listener)
{
    BOOL canShowMIMEType;
    if (FAILED(webView->canShowMIMEType(type, &canShowMIMEType)))
        canShowMIMEType = FALSE;

    BSTR url;
    request->URL(&url);

    if (String(url, SysStringLen(url)).startsWith("file:")) {
        BOOL isDirectory = FALSE;
        WIN32_FILE_ATTRIBUTE_DATA attrs;
        if (GetFileAttributesEx(url, GetFileExInfoStandard, &attrs))
            isDirectory = !!(attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

        if (isDirectory)
            listener->ignore();
        else if(canShowMIMEType)
            listener->use();
        else
            listener->ignore();
    } else if (canShowMIMEType)
        listener->use();
    else
        listener->ignore();
    SysFreeString(url);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DefaultPolicyDelegate::unableToImplementPolicyWithError(
    /*[in]*/ IWebView* /*webView*/, 
    /*[in]*/ IWebError* error, 
    /*[in]*/ IWebFrame* frame)
{
    BSTR errorStr;
    error->localizedDescription(&errorStr);

    BSTR frameName;
    frame->name(&frameName);

    LOG_ERROR("called unableToImplementPolicyWithError:%S inFrame:%S", errorStr ? errorStr : TEXT(""), frameName ? frameName : TEXT(""));
    SysFreeString(errorStr);
    SysFreeString(frameName);

    return S_OK;
}
