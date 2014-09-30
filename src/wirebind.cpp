#include <node.h>
#include <v8.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <node_buffer.h>
#include <lcd.h>


using namespace node;
using namespace v8;
using std::string;

struct _wiringPiSetupGpioStruct{
	uv_work_t request;
	int ret;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _wiringPiSetupGpioMain(uv_work_t* request) {
	_wiringPiSetupGpioStruct* promise = static_cast<_wiringPiSetupGpioStruct*>(request->data);

	int ret = wiringPiSetupGpio();
	promise->ret = ret;

	if(ret==-1) {
		promise->error = true;
		promise->error_message = strerror(errno);
	}
}

void _wiringPiSetupGpioAfter(uv_work_t* request) {
	HandleScope scope;
	_wiringPiSetupGpioStruct* promise = static_cast<_wiringPiSetupGpioStruct*>(request->data);

	Handle<Value> argv[2];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
		argv[1] = Undefined();
	} else {
		argv[0] = Null();
		argv[1] = Number::New(promise->ret);
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 2, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _wiringPiSetupGpio(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[0]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[0]);

		_wiringPiSetupGpioStruct* promise = new _wiringPiSetupGpioStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		
		uv_queue_work(uv_default_loop(), &promise->request, _wiringPiSetupGpioMain, (uv_after_work_cb)_wiringPiSetupGpioAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _pinModeStruct{
	uv_work_t request;
	int pin;
	int mode;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _pinModeMain(uv_work_t* request) {
	_pinModeStruct* promise = static_cast<_pinModeStruct*>(request->data);

	pinMode(promise->pin, promise->mode);
}

void _pinModeAfter(uv_work_t* request) {
	HandleScope scope;
	_pinModeStruct* promise = static_cast<_pinModeStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _pinMode(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[2]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[2]);

		_pinModeStruct* promise = new _pinModeStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->pin = args[0]->Int32Value();
		promise->mode = args[1]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _pinModeMain, (uv_after_work_cb)_pinModeAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _digitalWriteStruct{
	uv_work_t request;
	int pin;
	int value;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _digitalWriteMain(uv_work_t* request) {
	_digitalWriteStruct* promise = static_cast<_digitalWriteStruct*>(request->data);

	digitalWrite(promise->pin, promise->value);
}

void _digitalWriteAfter(uv_work_t* request) {
	HandleScope scope;
	_digitalWriteStruct* promise = static_cast<_digitalWriteStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _digitalWrite(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[2]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[2]);

		_digitalWriteStruct* promise = new _digitalWriteStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->pin = args[0]->Int32Value();
		promise->value = args[1]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _digitalWriteMain, (uv_after_work_cb)_digitalWriteAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _digitalReadStruct{
	uv_work_t request;
	int ret;
	int pin;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _digitalReadMain(uv_work_t* request) {
	_digitalReadStruct* promise = static_cast<_digitalReadStruct*>(request->data);

	int ret = digitalRead(promise->pin);
	promise->ret = ret;
}

void _digitalReadAfter(uv_work_t* request) {
	HandleScope scope;
	_digitalReadStruct* promise = static_cast<_digitalReadStruct*>(request->data);

	Handle<Value> argv[2];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
		argv[1] = Undefined();
	} else {
		argv[0] = Null();
		argv[1] = Number::New(promise->ret);
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 2, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _digitalRead(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[1]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[1]);

		_digitalReadStruct* promise = new _digitalReadStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->pin = args[0]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _digitalReadMain, (uv_after_work_cb)_digitalReadAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _pullUpDnControlStruct{
	uv_work_t request;
	int pin;
	int pud;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _pullUpDnControlMain(uv_work_t* request) {
	_pullUpDnControlStruct* promise = static_cast<_pullUpDnControlStruct*>(request->data);

	pullUpDnControl(promise->pin, promise->pud);
}

void _pullUpDnControlAfter(uv_work_t* request) {
	HandleScope scope;
	_pullUpDnControlStruct* promise = static_cast<_pullUpDnControlStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _pullUpDnControl(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[2]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[2]);

		_pullUpDnControlStruct* promise = new _pullUpDnControlStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->pin = args[0]->Int32Value();
		promise->pud = args[1]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _pullUpDnControlMain, (uv_after_work_cb)_pullUpDnControlAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _pwmWriteStruct{
	uv_work_t request;
	int pin;
	int value;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _pwmWriteMain(uv_work_t* request) {
	_pwmWriteStruct* promise = static_cast<_pwmWriteStruct*>(request->data);

	pwmWrite(promise->pin, promise->value);
}

void _pwmWriteAfter(uv_work_t* request) {
	HandleScope scope;
	_pwmWriteStruct* promise = static_cast<_pwmWriteStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _pwmWrite(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[2]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[2]);

		_pwmWriteStruct* promise = new _pwmWriteStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->pin = args[0]->Int32Value();
		promise->value = args[1]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _pwmWriteMain, (uv_after_work_cb)_pwmWriteAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _pwmSetModeStruct{
	uv_work_t request;
	int mode;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _pwmSetModeMain(uv_work_t* request) {
	_pwmSetModeStruct* promise = static_cast<_pwmSetModeStruct*>(request->data);

	pwmSetMode(promise->mode);
}

void _pwmSetModeAfter(uv_work_t* request) {
	HandleScope scope;
	_pwmSetModeStruct* promise = static_cast<_pwmSetModeStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _pwmSetMode(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[1]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[1]);

		_pwmSetModeStruct* promise = new _pwmSetModeStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->mode = args[0]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _pwmSetModeMain, (uv_after_work_cb)_pwmSetModeAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _pwmSetRangeStruct{
	uv_work_t request;
	int range;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _pwmSetRangeMain(uv_work_t* request) {
	_pwmSetRangeStruct* promise = static_cast<_pwmSetRangeStruct*>(request->data);

	pwmSetRange((unsigned int)promise->range);
}

void _pwmSetRangeAfter(uv_work_t* request) {
	HandleScope scope;
	_pwmSetRangeStruct* promise = static_cast<_pwmSetRangeStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _pwmSetRange(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[1]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[1]);

		_pwmSetRangeStruct* promise = new _pwmSetRangeStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->range = args[0]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _pwmSetRangeMain, (uv_after_work_cb)_pwmSetRangeAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _pwmSetClockStruct{
	uv_work_t request;
	int divisor;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _pwmSetClockMain(uv_work_t* request) {
	_pwmSetClockStruct* promise = static_cast<_pwmSetClockStruct*>(request->data);

	pwmSetClock(promise->divisor);
}

void _pwmSetClockAfter(uv_work_t* request) {
	HandleScope scope;
	_pwmSetClockStruct* promise = static_cast<_pwmSetClockStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _pwmSetClock(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[1]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[1]);

		_pwmSetClockStruct* promise = new _pwmSetClockStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->divisor = args[0]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _pwmSetClockMain, (uv_after_work_cb)_pwmSetClockAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _wiringPiSPISetupStruct{
	uv_work_t request;
	int ret;
	int channel;
	int speed;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _wiringPiSPISetupMain(uv_work_t* request) {
	_wiringPiSPISetupStruct* promise = static_cast<_wiringPiSPISetupStruct*>(request->data);

	int ret = wiringPiSPISetup(promise->channel, promise->speed);
	promise->ret = ret;

	if(ret==-1) {
		promise->error = true;
		promise->error_message = strerror(errno);
	}
}

void _wiringPiSPISetupAfter(uv_work_t* request) {
	HandleScope scope;
	_wiringPiSPISetupStruct* promise = static_cast<_wiringPiSPISetupStruct*>(request->data);

	Handle<Value> argv[2];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
		argv[1] = Undefined();
	} else {
		argv[0] = Null();
		argv[1] = Number::New(promise->ret);
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 2, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _wiringPiSPISetup(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[2]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[2]);

		_wiringPiSPISetupStruct* promise = new _wiringPiSPISetupStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->channel = args[0]->Int32Value();
		promise->speed = args[1]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _wiringPiSPISetupMain, (uv_after_work_cb)_wiringPiSPISetupAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _wiringPiSPIDataRWStruct{
	uv_work_t request;
	int channel;
	char* data;
	int len;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _wiringPiSPIDataRWMain(uv_work_t* request) {
	_wiringPiSPIDataRWStruct* promise = static_cast<_wiringPiSPIDataRWStruct*>(request->data);

	int ret = wiringPiSPIDataRW(promise->channel, (unsigned char*)promise->data, promise->len);

	if(ret==-1) {
		promise->error = true;
		promise->error_message = strerror(errno);
	}
}

void _wiringPiSPIDataRWAfter(uv_work_t* request) {
	HandleScope scope;
	_wiringPiSPIDataRWStruct* promise = static_cast<_wiringPiSPIDataRWStruct*>(request->data);

	Handle<Value> argv[2];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
		argv[1] = Undefined();
	} else {
		argv[0] = Null();
		argv[1] = Buffer::New(String::New(promise->data, promise->len));
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 2, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _wiringPiSPIDataRW(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 4  || !args[0]->IsInt32() || !args[1]->IsObject() || !args[2]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[3]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[3]);

		_wiringPiSPIDataRWStruct* promise = new _wiringPiSPIDataRWStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->channel = args[0]->Int32Value();
		promise->data = Buffer::Data(args[1]->ToObject());
		promise->len = args[2]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _wiringPiSPIDataRWMain, (uv_after_work_cb)_wiringPiSPIDataRWAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _lcdInitStruct{
	uv_work_t request;
	int ret;
	int rows;
	int cols;
	int bits;
	int rs;
	int strb;
	int d0;
	int d1;
	int d2;
	int d3;
	int d4;
	int d5;
	int d6;
	int d7;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _lcdInitMain(uv_work_t* request) {
	_lcdInitStruct* promise = static_cast<_lcdInitStruct*>(request->data);

	int ret = lcdInit(promise->rows,promise->cols,promise->bits,promise->rs,promise->strb,promise->d0,promise->d1,promise->d2,promise->d3,promise->d4,promise->d5,promise->d6,promise->d7);
	promise->ret = ret;

	if(ret==-1) {
		promise->error = true;
		promise->error_message = strerror(errno);
	}
}

void _lcdInitAfter(uv_work_t* request) {
	HandleScope scope;
	_lcdInitStruct* promise = static_cast<_lcdInitStruct*>(request->data);

	Handle<Value> argv[2];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
		argv[1] = Undefined();
	} else {
		argv[0] = Null();
		argv[1] = Number::New(promise->ret);
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 2, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _lcdInit(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 14  || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsInt32() || !args[3]->IsInt32() || !args[4]->IsInt32() || !args[5]->IsInt32() || !args[6]->IsInt32() || !args[7]->IsInt32() || !args[8]->IsInt32() || !args[9]->IsInt32() || !args[10]->IsInt32() || !args[11]->IsInt32() || !args[12]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[13]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[13]);

		_lcdInitStruct* promise = new _lcdInitStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->rows = args[0]->Int32Value();
		promise->cols = args[1]->Int32Value();
		promise->bits = args[2]->Int32Value();
		promise->rs = args[3]->Int32Value();
		promise->strb = args[4]->Int32Value();
		promise->d0 = args[5]->Int32Value();
		promise->d1 = args[6]->Int32Value();
		promise->d2 = args[7]->Int32Value();
		promise->d3 = args[8]->Int32Value();
		promise->d4 = args[9]->Int32Value();
		promise->d5 = args[10]->Int32Value();
		promise->d6 = args[11]->Int32Value();
		promise->d7 = args[12]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _lcdInitMain, (uv_after_work_cb)_lcdInitAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _lcdHomeStruct{
	uv_work_t request;
	int fd;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _lcdHomeMain(uv_work_t* request) {
	_lcdHomeStruct* promise = static_cast<_lcdHomeStruct*>(request->data);

	lcdHome(promise->fd);
}

void _lcdHomeAfter(uv_work_t* request) {
	HandleScope scope;
	_lcdHomeStruct* promise = static_cast<_lcdHomeStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _lcdHome(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[1]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[1]);

		_lcdHomeStruct* promise = new _lcdHomeStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->fd = args[0]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _lcdHomeMain, (uv_after_work_cb)_lcdHomeAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _lcdClearStruct{
	uv_work_t request;
	int fd;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _lcdClearMain(uv_work_t* request) {
	_lcdClearStruct* promise = static_cast<_lcdClearStruct*>(request->data);

	lcdClear(promise->fd);
}

void _lcdClearAfter(uv_work_t* request) {
	HandleScope scope;
	_lcdClearStruct* promise = static_cast<_lcdClearStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _lcdClear(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[1]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[1]);

		_lcdClearStruct* promise = new _lcdClearStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->fd = args[0]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _lcdClearMain, (uv_after_work_cb)_lcdClearAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _lcdPositionStruct{
	uv_work_t request;
	int fd;
	int x;
	int y;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _lcdPositionMain(uv_work_t* request) {
	_lcdPositionStruct* promise = static_cast<_lcdPositionStruct*>(request->data);

	lcdPosition(promise->fd,promise->x,promise->y);
}

void _lcdPositionAfter(uv_work_t* request) {
	HandleScope scope;
	_lcdPositionStruct* promise = static_cast<_lcdPositionStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _lcdPosition(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 4  || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[3]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[3]);

		_lcdPositionStruct* promise = new _lcdPositionStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->fd = args[0]->Int32Value();
		promise->x = args[1]->Int32Value();
		promise->y = args[2]->Int32Value();
		
		uv_queue_work(uv_default_loop(), &promise->request, _lcdPositionMain, (uv_after_work_cb)_lcdPositionAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

struct _lcdPutsStruct{
	uv_work_t request;
	int fd;
	char* string;
	bool error;
	char* error_message;
	Persistent<Function> callback;
};

void _lcdPutsMain(uv_work_t* request) {
	_lcdPutsStruct* promise = static_cast<_lcdPutsStruct*>(request->data);

	lcdPuts(promise->fd,promise->string);
}

void _lcdPutsAfter(uv_work_t* request) {
	HandleScope scope;
	_lcdPutsStruct* promise = static_cast<_lcdPutsStruct*>(request->data);

	Handle<Value> argv[1];
	if (promise->error) {
		argv[0] = String::New(promise->error_message);
	} else {
		argv[0] = Null();
	}

	TryCatch try_catch;
	promise->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	promise->callback.Dispose();
	delete promise;

	if (try_catch.HasCaught()) {
		FatalException(try_catch);
	}
}

static Handle<Value> _lcdPuts(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsObject() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	if ( args[2]->IsFunction() ) {
		Local<Function> callback = Local<Function>::Cast(args[2]);

		_lcdPutsStruct* promise = new _lcdPutsStruct();
		promise->error = false;
		promise->request.data = promise;
		promise->callback = Persistent<Function>::New(callback);
		promise->fd = args[0]->Int32Value();
		promise->string = Buffer::Data(args[1]->ToObject());

		size_t len = Buffer::Length(args[1]->ToObject());
        promise->string[len] = '\0';
		
		uv_queue_work(uv_default_loop(), &promise->request, _lcdPutsMain, (uv_after_work_cb)_lcdPutsAfter);
	} else {
		ThrowException(Exception::TypeError(String::New("Callback missing")));
		return scope.Close(Undefined());
	}

	return scope.Close(Undefined());
}

static Handle<Value> _wiringPiSetupGpioSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 0  ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int ret = wiringPiSetupGpio();
	

	if(ret==-1) {
		ThrowException(Exception::TypeError(String::New(strerror(errno))));
	}

	return scope.Close(Number::New(ret));
}

static Handle<Value> _pinModeSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int pin = args[0]->Int32Value();
	int mode = args[1]->Int32Value();
	pinMode(pin, mode);
	

	return scope.Close(Undefined());
}

static Handle<Value> _digitalWriteSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int pin = args[0]->Int32Value();
	int value = args[1]->Int32Value();
	digitalWrite(pin, value);
	

	return scope.Close(Undefined());
}

static Handle<Value> _digitalReadSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int pin = args[0]->Int32Value();
	int ret = digitalRead(pin);
	

	return scope.Close(Number::New(ret));
}

static Handle<Value> _pullUpDnControlSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int pin = args[0]->Int32Value();
	int pud = args[1]->Int32Value();
	pullUpDnControl(pin, pud);
	

	return scope.Close(Undefined());
}

static Handle<Value> _pwmWriteSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int pin = args[0]->Int32Value();
	int value = args[1]->Int32Value();
	pwmWrite(pin, value);
	

	return scope.Close(Undefined());
}

static Handle<Value> _pwmSetModeSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int mode = args[0]->Int32Value();
	pwmSetMode(mode);
	

	return scope.Close(Undefined());
}

static Handle<Value> _pwmSetRangeSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int range = args[0]->Int32Value();
	pwmSetRange((unsigned int)range);
	

	return scope.Close(Undefined());
}

static Handle<Value> _pwmSetClockSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int divisor = args[0]->Int32Value();
	pwmSetClock(divisor);
	

	return scope.Close(Undefined());
}

static Handle<Value> _wiringPiSPISetupSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() || !args[1]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int channel = args[0]->Int32Value();
	int speed = args[1]->Int32Value();
	int ret = wiringPiSPISetup(channel, speed);
	

	if(ret==-1) {
		ThrowException(Exception::TypeError(String::New(strerror(errno))));
	}

	return scope.Close(Number::New(ret));
}

static Handle<Value> _wiringPiSPIDataRWSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsObject() || !args[2]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int channel = args[0]->Int32Value();
	char* data = Buffer::Data(args[1]->ToObject());
	int len = args[2]->Int32Value();
	int ret = wiringPiSPIDataRW(channel, (unsigned char*)data, len);
	

	if(ret==-1) {
		ThrowException(Exception::TypeError(String::New(strerror(errno))));
	}

	return scope.Close(Buffer::New(String::New(data, len)));
}

static Handle<Value> _lcdInitSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 13  || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsInt32() || !args[3]->IsInt32() || !args[4]->IsInt32() || !args[5]->IsInt32() || !args[6]->IsInt32() || !args[7]->IsInt32() || !args[8]->IsInt32() || !args[9]->IsInt32() || !args[10]->IsInt32() || !args[11]->IsInt32() || !args[12]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int rows = args[0]->Int32Value();
	int cols = args[1]->Int32Value();
	int bits = args[2]->Int32Value();
	int rs = args[3]->Int32Value();
	int strb = args[4]->Int32Value();
	int d0 = args[5]->Int32Value();
	int d1 = args[6]->Int32Value();
	int d2 = args[7]->Int32Value();
	int d3 = args[8]->Int32Value();
	int d4 = args[9]->Int32Value();
	int d5 = args[10]->Int32Value();
	int d6 = args[11]->Int32Value();
	int d7 = args[12]->Int32Value();
	int ret = lcdInit(rows, cols, bits, rs, strb, d0, d1, d2, d3, d4, d5, d6, d7);
	

	if(ret==-1) {
		ThrowException(Exception::TypeError(String::New(strerror(errno))));
	}

	return scope.Close(Number::New(ret));
}

static Handle<Value> _lcdHomeSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int fd = args[0]->Int32Value();
	lcdHome(fd);
	

	return scope.Close(Undefined());
}

static Handle<Value> _lcdClearSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 1  || !args[0]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int fd = args[0]->Int32Value();
	lcdClear(fd);
	

	return scope.Close(Undefined());
}

static Handle<Value> _lcdPositionSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 3  || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsInt32() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int fd = args[0]->Int32Value();
	int x = args[1]->Int32Value();
	int y = args[2]->Int32Value();
	lcdPosition(fd, x, y);
	

	return scope.Close(Undefined());
}

static Handle<Value> _lcdPutsSync(const Arguments& args) {
	HandleScope scope;

	if ( args.Length() != 2  || !args[0]->IsInt32() || !args[1]->IsObject() ) {
		ThrowException(Exception::TypeError(String::New("Bad parameter")));
		return scope.Close(Undefined());
	}

	int fd = args[0]->Int32Value();
	char* string = Buffer::Data(args[1]->ToObject());
	size_t len = Buffer::Length(args[1]->ToObject());
    string[len] = '\0';
	lcdPuts(fd, string);

	return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
	//___ASYNC PART___
	exports->Set(String::NewSymbol("_wiringPiSetupGpio"), FunctionTemplate::New(_wiringPiSetupGpio)->GetFunction());
	exports->Set(String::NewSymbol("_pinMode"), FunctionTemplate::New(_pinMode)->GetFunction());
	exports->Set(String::NewSymbol("_digitalWrite"), FunctionTemplate::New(_digitalWrite)->GetFunction());
	exports->Set(String::NewSymbol("_digitalRead"), FunctionTemplate::New(_digitalRead)->GetFunction());
	exports->Set(String::NewSymbol("_pullUpDnControl"), FunctionTemplate::New(_pullUpDnControl)->GetFunction());
	//---PWM---
	exports->Set(String::NewSymbol("_pwmWrite"), FunctionTemplate::New(_pwmWrite)->GetFunction());
	exports->Set(String::NewSymbol("_pwmSetMode"), FunctionTemplate::New(_pwmSetMode)->GetFunction());
	exports->Set(String::NewSymbol("_pwmSetRange"), FunctionTemplate::New(_pwmSetRange)->GetFunction());
	exports->Set(String::NewSymbol("_pwmSetClock"), FunctionTemplate::New(_pwmSetClock)->GetFunction());
	//---SPI---
	exports->Set(String::NewSymbol("_wiringPiSPISetup"), FunctionTemplate::New(_wiringPiSPISetup)->GetFunction());
	exports->Set(String::NewSymbol("_wiringPiSPIDataRW"), FunctionTemplate::New(_wiringPiSPIDataRW)->GetFunction());
	//---LCD---
	exports->Set(String::NewSymbol("_lcdInit"), FunctionTemplate::New(_lcdInit)->GetFunction());
	exports->Set(String::NewSymbol("_lcdHome"), FunctionTemplate::New(_lcdHome)->GetFunction());
	exports->Set(String::NewSymbol("_lcdClear"), FunctionTemplate::New(_lcdClear)->GetFunction());
	exports->Set(String::NewSymbol("_lcdPosition"), FunctionTemplate::New(_lcdPosition)->GetFunction());
	exports->Set(String::NewSymbol("_lcdPuts"), FunctionTemplate::New(_lcdPuts)->GetFunction());
	
	//___SYNC PART___
	exports->Set(String::NewSymbol("_wiringPiSetupGpioSync"), FunctionTemplate::New(_wiringPiSetupGpioSync)->GetFunction());
	exports->Set(String::NewSymbol("_pinModeSync"), FunctionTemplate::New(_pinModeSync)->GetFunction());
	exports->Set(String::NewSymbol("_digitalWriteSync"), FunctionTemplate::New(_digitalWriteSync)->GetFunction());
	exports->Set(String::NewSymbol("_digitalReadSync"), FunctionTemplate::New(_digitalReadSync)->GetFunction());
	exports->Set(String::NewSymbol("_pullUpDnControlSync"), FunctionTemplate::New(_pullUpDnControlSync)->GetFunction());
	//---PWM---
	exports->Set(String::NewSymbol("_pwmWriteSync"), FunctionTemplate::New(_pwmWriteSync)->GetFunction());
	exports->Set(String::NewSymbol("_pwmSetModeSync"), FunctionTemplate::New(_pwmSetModeSync)->GetFunction());
	exports->Set(String::NewSymbol("_pwmSetRangeSync"), FunctionTemplate::New(_pwmSetRangeSync)->GetFunction());
	exports->Set(String::NewSymbol("_pwmSetClockSync"), FunctionTemplate::New(_pwmSetClockSync)->GetFunction());
	//---SPI---
	exports->Set(String::NewSymbol("_wiringPiSPISetupSync"), FunctionTemplate::New(_wiringPiSPISetupSync)->GetFunction());
	exports->Set(String::NewSymbol("_wiringPiSPIDataRWSync"), FunctionTemplate::New(_wiringPiSPIDataRWSync)->GetFunction());
	//---LCD---
	exports->Set(String::NewSymbol("_lcdInitSync"), FunctionTemplate::New(_lcdInitSync)->GetFunction());
	exports->Set(String::NewSymbol("_lcdHomeSync"), FunctionTemplate::New(_lcdHomeSync)->GetFunction());
	exports->Set(String::NewSymbol("_lcdClearSync"), FunctionTemplate::New(_lcdClearSync)->GetFunction());
	exports->Set(String::NewSymbol("_lcdPositionSync"), FunctionTemplate::New(_lcdPositionSync)->GetFunction());
	exports->Set(String::NewSymbol("_lcdPutsSync"), FunctionTemplate::New(_lcdPutsSync)->GetFunction());
 }

/*****EXPORTS******
//___ASYNC PART___
_wiringPiSetupGpio( , cb )
_pinMode( pin, mode, cb )
_digitalWrite( pin, value, cb )
_digitalRead( pin, cb )
_pullUpDnControl( pin, pud, cb )
//---PWM---
_pwmWrite( pin, value, cb )
_pwmSetMode( mode, cb )
_pwmSetRange( range, cb )
_pwmSetClock( divisor, cb )
//---SPI---
_wiringPiSPISetup( channel, speed, cb )
_wiringPiSPIDataRW( channel, data, len, cb )
//---LCD---
_lcdInit( rows, cols, bits, rs, strb, d0, d1, d2, d3, d4, d5, d6, d7, cb )
_lcdHome( fd, cb )
_lcdClear( fd, cb )
_lcdPosition( fd, x, y, cb )
_lcdPuts( fd, string, cb )

//___SYNC PART___
_wiringPiSetupGpioSync(  )
_pinModeSync( pin, mode )
_digitalWriteSync( pin, value )
_digitalReadSync( pin )
_pullUpDnControlSync( pin, pud )
//---PWM---
_pwmWriteSync( pin, value )
_pwmSetModeSync( mode )
_pwmSetRangeSync( range )
_pwmSetClockSync( divisor ) 
//---SPI---
_wiringPiSPISetupSync( channel, speed )
_wiringPiSPIDataRWSync( channel, data, len )
//---LCD---
_lcdInitSync( rows, cols, bits, rs, strb, d0, d1, d2, d3, d4, d5, d6, d7 )
_lcdHomeSync( fd )
_lcdClearSync( fd )
_lcdPositionSync( fd, x, y )
_lcdPutsSync( fd, string )
*****EXPORTS******/
NODE_MODULE(wirebind, init);