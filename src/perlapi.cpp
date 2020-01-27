#include "perlapi.h"

Napi::Value NodePerl::PerlSub(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    SV *sub = (SV *) info.Data();

    dSP;
    char *error = NULL;
    STRLEN error_len;
    SV    *sv;
    int count;
    int ret = 0;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    PUTBACK;

    count = call_sv(sub, G_NOARGS | G_SCALAR);

    SPAGAIN;
    if( count > 0) {
        sv = POPs;
        if(SvIOK(sv)) {
            ret = SvIV(sv);
        }
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return Napi::Number::New(env, ret);
}

NodePerl::NodePerl(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodePerl>(info)  {
    Napi::Env env = info.Env();

    int length = info.Length();

    /*
       if (length != 1 || !info[0].IsNumber()) {
       Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
       }
       Napi::Number value = info[0].As<Napi::Number>();
    // this->actualClass_ = new ActualClass(value.DoubleValue());
    */

    char **argv = {NULL};
    const char *embedding[] = { "", "-e", "0" };
    int argc = 0;

    PERL_SYS_INIT3(0,&argv,NULL);

    PerlInterpreter *my_perl;

    my_perl = perl_alloc();
    perl_construct( my_perl );
    perl_parse(my_perl, xs_init, 3, (char**)embedding, NULL);
    // perl_parse(this->my_perl, xs_init, 3, embedding, NULL);
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    perl_run(my_perl);

    this->my_perl = my_perl;
}

NodePerl::~NodePerl() {
    perl_destruct(this->my_perl);
    perl_free(this->my_perl);
    PERL_SYS_TERM();
}

Napi::Value NodePerl::perl2js(Napi::Env env, SV * sv) {

    // see xs-src/pack.c in msgpack-perl
    SvGETMAGIC(sv);

    if (SvPOKp(sv)) {
        STRLEN len;
        const char *s = SvPV(sv, len); // SvPV_nolen(val)
        return Napi::EscapableHandleScope(env).Escape(Napi::String::New(env, s, len));
    } else if (SvNOK(sv)) {
        return Napi::EscapableHandleScope(env).Escape(Napi::Number::New(env, SvNVX(sv)));
    } else if (SvIOK(sv)) {
        return Napi::EscapableHandleScope(env).Escape(Napi::Number::New(env, (double)SvIVX(sv)));
    } else if (SvROK(sv)) {
        return Napi::EscapableHandleScope(env).Escape(this->perl2js_rv(env, sv));
    } else if (!SvOK(sv)) {
        return Napi::EscapableHandleScope(env).Escape(env.Undefined());
    } else if (isGV(sv)) {
        std::cerr << "Cannot pass GV to v8 world" << std::endl;
        return Napi::EscapableHandleScope(env).Escape(env.Undefined());
    } else {
        sv_dump(sv);
        Napi::Error::New(env, "perl-api doesn't support this type").ThrowAsJavaScriptException();
        return Napi::EscapableHandleScope(env).Escape(env.Undefined());
    }
    // TODO: return callback function for perl code.
    // Perl callbacks should be managed by objects.
    // TODO: Handle async.
}

Napi::Value NodePerl::Evaluate(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    /*
       if (  info.Length() != 1 || !info[0].IsString()) {
       Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
       }
       */

    std::string code = info[0].ToString();
    // eval_pv((std::string) code, TRUE);
    SV *val = eval_pv(code.c_str(), TRUE);

    return this->perl2js(env, val);
}

Napi::Value NodePerl::perl2js_rv(Napi::Env env, SV * rv) {
    SV *sv = SvRV(rv);
    SvGETMAGIC(sv);
    svtype svt = (svtype)SvTYPE(sv);

    if (SvOBJECT(sv)) { // blessed object.
        Napi::Value arg0 = Napi::External<void>::New(env, &rv);
        Napi::Value arg1 = Napi::External<void>::New(env, &my_perl);
        Napi::Value args[] = {arg0, arg1};
        return env.Undefined(); // XXX for now
        //Napi::Object retval( Napi::New(env, NodePerlObject::constructor)->GetFunction()->NewInstance(2, args));
        //return Napi::EscapableHandleScope(env).Escape(retval);
    } else if (svt == SVt_PVHV) {
        HV* hval = (HV*)sv;
        HE* he;
        Napi::Object retval = Napi::Object::New(env);
        while ((he = hv_iternext(hval))) {
            retval.Set(
                    this->perl2js(env, hv_iterkeysv(he)),
                    this->perl2js(env, hv_iterval(hval, he))
                    );
        }
        return Napi::EscapableHandleScope(env).Escape(retval);
    } else if (svt == SVt_PVAV) {
        AV* ary = (AV*)sv;
        Napi::Array retval = Napi::Array::New(env);
        int len = av_len(ary) + 1;
        for (int i=0; i<len; ++i) {
            SV** svp = av_fetch(ary, i, 0);
            if (svp) {
                retval.Set(Napi::Number::New(env, i), this->perl2js(env, *svp));
            } else {
                retval.Set(Napi::Number::New(env, i), env.Undefined());
            }
        }
        return Napi::EscapableHandleScope(env).Escape(retval);
    } else if (svt == SVt_PVCV) {
        CV* sub = (CV*) sv;
        Napi::Function retval = Napi::Function::New(env, NodePerl::PerlFunc, NULL, sub );
        return Napi::EscapableHandleScope(env).Escape(retval);
    } else if (svt < SVt_PVAV) {
        sv_dump(sv);
        Napi::Error::New(env, "perl-api doesn't support scalarref").ThrowAsJavaScriptException();

        return Napi::EscapableHandleScope(env).Escape(env.Undefined());
    } else {
        return Napi::EscapableHandleScope(env).Escape(env.Undefined());
    }
}

Napi::Value NodePerl::PerlFunc(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    SV *sub = (SV *) info.Data();

    dSP;
    char *error = NULL;
    STRLEN error_len;
    SV    *sv;
    int count;
    int ret = 0;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    PUTBACK;

    count = call_sv(sub, G_NOARGS | G_SCALAR);

    SPAGAIN;
    if( count > 0) {
        sv = POPs;
        if(SvIOK(sv)) {
            ret = SvIV(sv);
        }
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return Napi::Number::New(env, ret);
}

Napi::Object NodePerl::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "NodePerl", {
            InstanceMethod("evaluate", &NodePerl::Evaluate),
            });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("NodePerl", func);
    return exports;
}


Napi::FunctionReference NodePerl::constructor;

