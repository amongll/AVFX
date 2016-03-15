#include "VEditVM.h"
#include "VEditMltRun.h"
#include <signal.h>
#include <stack>

using namespace std;
using namespace vedit;
struct sigaction _act;
static void my_sig_handler(int );

static void install_sig() {
	_act.sa_handler=my_sig_handler;
	sigemptyset(&_act.sa_mask);
	sigaddset(&_act.sa_mask,SIGINT);
	sigaddset(&_act.sa_mask,SIGTERM);
	sigaddset(&_act.sa_mask,SIGQUIT);
	sigaddset(&_act.sa_mask,SIGPIPE);
	_act.sa_flags=SA_SIGINFO;

	sigaction(SIGINT,&_act,NULL);
	sigaction(SIGQUIT,&_act,NULL);
	sigaction(SIGTERM,&_act,NULL);
	_act.sa_handler=SIG_IGN;
	sigaction(SIGPIPE,&_act,NULL);
}

typedef void (*trigger)(void*);
static std::stack<std::pair<void*,trigger> > sstoppers;

static void register_stop(void* obj, trigger stopper) {
	sstoppers.push(std::make_pair(obj, stopper));
}

void stop_mlt_run(void* a)
{
	MltRuntime* run = static_cast<MltRuntime*>(a);
	run->stop();
}

static void my_sig_handler(int )
{
	std::pair<void*,trigger> obj ;
	while( !sstoppers.empty() ) {
		obj = sstoppers.top();
		obj.second(obj.first);
		sstoppers.pop();
	}
}

int main(int argc, char* argv[])
{
    assert(mlt_factory_init(NULL));
	Vm::load_script_dir(argv[1]);
	ScriptSerialized  a
	= Vm::call_script(argv[2], vedit::VIDEO_RESOURCE_SCRIPT, NULL);
	json_dumpf(a.second.h, stdout, JSON_PRESERVE_ORDER|JSON_INDENT(2));
	Vm::cleanup_stream_resources();

	MltRuntime run(a.second.h);

	run.run();
	register_stop((void*)&run, stop_mlt_run);

	while(run.running()) {
		struct timespec req = {1,0};
		nanosleep(&req, NULL);
	}


    mlt_factory_close();
}