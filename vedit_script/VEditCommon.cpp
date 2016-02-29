/*
 * VEditCommon.cpp
 *
 *  Created on: 2016Äê2ÔÂ26ÈÕ
 *      Author: li.lei@youku.com
 */

#include "VEditCommon.h"

NMSP_BEGIN(vedit)

bool is_valid_identifier(const char* check)
{
	if ( !check || !check[0] )
		return false;

	char ch = *check;
	if ( ch >= '0' && ch <= '9' )
		return false;

	while(*check) {
		ch = *(check++);
		if ( (ch >= '0' && ch <= '9')
				|| ch=='_'
				|| (ch >= 'a' && ch <= 'z')
				|| (ch >= 'A' && ch <= 'Z') ) {
			continue;
		}
		else
			return false;
	}

	return true;
}

bool is_macro_only(const char* str) {
}

bool is_selector_only(const char* str) {
}

void parse_selector(const char* str, string& ename, string& sname) {
}

void parse_params(const char* str, vector<string>& params) {
}

void parse_params(const char* str, hash_multimap<string, int>& idxes,
		vector<string>& segments)
{
}

bool is_param_only(const char* str)
{
}

NMSP_END(vedit)
