

#ifndef JSGlue_JSObject_h
#define JSGlue_JSObject_h

#include "JSBase.h"
#include "JSUtils.h"

typedef void (*JSObjectMarkProcPtr)(void *data);
JSObjectRef JSObjectCreateInternal(void *data, JSObjectCallBacksPtr callBacks, JSObjectMarkProcPtr markProc, int dataType);

class JSUserObject : public JSBase {
    public:
        JSUserObject(JSObjectCallBacksPtr callBacks, JSObjectMarkProcPtr markProc, void *data, int dataType);
        virtual ~JSUserObject();

        CFArrayRef CopyPropertyNames(void);
        JSUserObject* CopyProperty(CFStringRef propertyName);
        void SetProperty(CFStringRef propertyName, JSUserObject* value);
        CallType getCallData(CallData&);
        JSUserObject* CallFunction(JSUserObject* thisObj, CFArrayRef args);
        CFTypeRef CopyCFValue() const;
        virtual UInt8 Equal(JSBase* other);
        void *GetData();
        void Mark();

                int DataType() const { return fDataType; }
    private:
        JSObjectCallBacks fCallBacks;
        JSObjectMarkProcPtr fMarkProc;
        void *fData;
                int fDataType;
};

#endif // JSGlue_JSObject_h
