/*
 * VEditCommon.h
 *
 *  Created on: 2016Äê2ÔÂ22ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITCOMMON_H_
#define VEDITCOMMON_H_
#include "VEditCommonDefine.h"
#include <string>
#include <tr1/memory>
#include <vector>
#include <deque>
#include <hash_map>
#include <hash_set>
#include <jansson.h>
#include <cassert>
#include <cstring>
#include <framework/mlt.h>

using namespace __gnu_cxx;
using namespace std::tr1;
using namespace std;

#define DECLARE_CONST_MEM_MODIFIER(mod,mem,type) \
	type mod = const_cast<type>(&(mem))

#endif /* VEDITCOMMON_H_ */
