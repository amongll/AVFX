#include "VEditVM.h"

using namespace std;
using namespace vedit;

int main(int argc, char* argv[])
{
    assert(mlt_factory_init(NULL));
	Vm::load_script_dir(argv[1]);
	json_t* compile_value = Vm::call_script(argv[2], vedit::VIDEO_RESOURCE_SCRIPT, NULL);
	json_dumpf(compile_value, stdout, JSON_PRESERVE_ORDER|JSON_INDENT(2));
	json_decref(compile_value);
	Vm::cleanup_stream_resources();
    mlt_factory_close();
}
