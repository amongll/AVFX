/*
 * VEditCommon.cpp
 *
 *  Created on: 2016��2��26��
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

bool is_macro_only(const char* str)
{
	hash_multimap<string,int> idxes;
	vector<string> segments;
	parse_macros(str,idxes, segments);
	if ( segments.size() == 1) {
		if (idxes.size() == 1) {
			return true;
		}
	}
	return false;
}

bool is_selector_only(const char* str)
{
	string enm,snm;
	parse_selector(str, enm, snm);
	if (enm.length() && snm.length() )
		return true;
	return false;
}


bool is_param_only(const char* str)
{
	hash_multimap<string,int> idxes;
	vector<string> segments;
	parse_params(str,idxes, segments);
	if ( segments.size() == 1) {
		if (idxes.size() == 1) {
			return true;
		}
	}
	return false;
}

void parse_params(const char* str, vector<string>& params)
{
	hash_multimap<string,int> dummy;
	parse_params(str, dummy, params);
}

void parse_params(const char* str, hash_multimap<string, int>& idxes,
		vector<string>& segments)
{
	if ( !str || !strlen(str) ) {
		idxes.clear();
		segments.clear();
		return;
	}

	string v (str);
	//string cur_token;

	size_t	pend_pos = 0, chk_pos =  0;
	size_t	ps1,ps2;

	do {
		ps1 = v.find("$(", chk_pos);
		if( ps1 != string::npos ) {
			ps2  = v.find(")", ps1 + 2);
			if (ps2 != string::npos) {
				if ( ps1 + 2 == ps2 ) {
					chk_pos = ps2 + 1;
				}
				else {
					string nm = v.substr(ps1 + 2, ps2 - ps1 - 2 );
					if ( is_valid_identifier(nm.c_str()) ) {
						if ( pend_pos < ps1 ) {
							segments.push_back(v.substr(pend_pos, ps1 - pend_pos));
						}
						chk_pos = pend_pos =  ps2 + 1;
						segments.push_back(nm);
						idxes.insert(make_pair(nm, segments.size() -1 ));
					}
					else {
						chk_pos = ps2 + 1;
					}
				}
			}
			else {
				//chk_pos = cur_token.size();
				chk_pos = v.size();
			}
		}
		else {
			//chk_pos = cur_token.size();
			chk_pos = v.size();
		}

		if (chk_pos == v.size()) {
			if ( pend_pos < chk_pos ) {
				segments.push_back(v.substr(pend_pos, chk_pos - 1));
			}
			break;
		}
	} while(1);
}

void parse_macros(const char* str, hash_multimap<string, int>& idxes,
		vector<string>& segments)
{

	if ( !str || !strlen(str) ) {
		idxes.clear();
		segments.clear();
		return;
	}

	string v (str);
	//string cur_token;

	size_t	pend_pos = 0, chk_pos =  0;
	size_t	ps1,ps2;

	do {
		ps1 = v.find("#(", chk_pos);
		if( ps1 != string::npos ) {
			ps2  = v.find(")", ps1 + 2);
			if (ps2 != string::npos) {
				if ( ps1 + 2 == ps2 ) {
					chk_pos = ps2 + 1;
				}
				else {
					string nm = v.substr(ps1 + 2, ps2 - ps1 - 2 );
					if ( is_valid_identifier(nm.c_str()) ) {
						if (pend_pos < ps1) {
							segments.push_back(v.substr(pend_pos, ps1 - pend_pos));
						}
						chk_pos = pend_pos =  ps2 + 1;
						segments.push_back(nm);
						idxes.insert(make_pair(nm, segments.size() -1 ));
						//idxes[nm] = segments.size() - 1;
					}
					else {
						chk_pos = ps2 + 1;
					}
				}
			}
			else {
				//chk_pos = cur_token.size();
				chk_pos = v.size();
			}
		}
		else {
			//chk_pos = cur_token.size();
			chk_pos = v.size();
		}

		if (chk_pos == v.size()) {
			if ( pend_pos < chk_pos ) {
				segments.push_back(v.substr(pend_pos, chk_pos - 1));
			}
			break;
		}
	} while(1);
}

void parse_selector(const char* str, string& enm, string& snm)
{
	enm.clear();
	snm.clear();
	if (!str || !strlen(str) || strlen(str) <= 3) {

		return;
	}

	if ( str[0] == '#' && str[1] == '('  && str[strlen(str)-1] == ')') {
		const char* p = str, *e = str + strlen(str) - 1;
		string tk;
		while ( p != e ) {
			if ( enm.length() == 0 && *p == ':' ) {
				if (tk.length()) {
					if ( !is_valid_identifier(tk.c_str()) ) {
						return;
					}
					else {
						enm = tk;
						tk.clear();
					}
				}
				else {
					return;
				}
			}
			else {
				tk.push_back(*p);
			}
		}

		if ( tk.length() ) {
			snm = tk;
			if ( !is_valid_identifier(snm.c_str())) {
				enm.clear();
				snm.clear();
				return;
			}
			else {
				return;
			}
		}
		else {
			enm.clear();
			return;
		}
	}
	else {
		enm.clear();
		snm.clear();
		return;
	}
}

bool is_call(const char* str, string& procnm)
{
	if ( !str || strlen(str) < strlen("$call()") )
		return false;

	if ( 0 == strncmp(str, "$call(", strlen("$call(")) ) {
		if ( str[strlen(str) - 1] != ')') {
			return false;
		}
		else {
			string tmp(str);
			tmp = tmp.substr(strlen("$call("));
			tmp = tmp.substr(0, tmp.size() - 1);

			if ( is_valid_identifier(tmp.c_str())) {
				procnm = tmp;
			}
			else {
				return false;
			}
		}
	}
	else {
		return false;
	}

	return false;
}



NMSP_END(vedit)

