#include <iostream>

#include <napi.h>
#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */
#include "perlxsi.h"

// #include "perlapiobject.h"

#ifdef New
#undef New
#endif

class PerlApi : public Napi::ObjectWrap<PerlApi> {
    private:
        static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
        PerlInterpreter *my_perl;

    public:
        PerlApi(const Napi::CallbackInfo& info);
        ~PerlApi();
        Napi::Value perl2js(Napi::Env env, SV * sv);
        Napi::Value Evaluate(const Napi::CallbackInfo& info);
        Napi::Value perl2js_rv(Napi::Env env, SV * rv);
        Napi::Value PerlSub(const Napi::CallbackInfo& info);
        static Napi::Value PerlFunc(const Napi::CallbackInfo& info);
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
};
