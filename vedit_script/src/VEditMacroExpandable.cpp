/*
 * VEditMacroExpandable.cpp
 *
 *  Created on: 2016-2-29
 *      Author: li.lei@youku.com
 */

#include "VEditMacroExpandable.h"
#include "VEditScript.h"

NMSP_BEGIN(vedit)
void MacroExpandable::expand_macro(const char* nm, const json_t* value)
		throw (Exception)
{
	MapIter oit = macro_idxes.find(nm);
	assert( oit != macro_idxes.end() );

	if ( only_macro) {
		expand_result = json_deep_copy((json_t*)value);
		macro_idxes.erase(oit);
		return;
	}
	else {
		pair<MapIter,MapIter> ranges =  macro_idxes.equal_range(nm);
		for ( oit = ranges.first; oit != ranges.second; oit++ ) {
			assert( !json_is_object(value) && !json_is_array(value));
			json_t* t = (json_t*)value;
			char buf[40];
			if ( json_is_string(t) ) {
				segments[oit->second] = json_string_value(t);
			}
			else if ( json_is_integer(t) ) {
				sprintf(buf, "%lld", json_integer_value(t));
				segments[oit->second] = buf;
			}
			else if ( json_is_real(t) ) {
				sprintf(buf, "%f", json_real_value(t));
				segments[oit->second] = buf;
			}
			else {
				sprintf(buf, json_is_true(t) ? "true" : "false");
				segments[oit->second] = buf;
			}
		}

		macro_idxes.erase(ranges.first, ranges.second);

		if (macro_idxes.size() == 0) {
			string temp;
			vector<string>::iterator is;
			for ( is = segments.begin(); is != segments.end(); is++ ) {
				temp += *is;
			}

			expand_result = json_string(temp.c_str());
		}
	}
}

void MacroExpandable::regist_self(Script& script) throw (Exception)
{
	MapIter it;
	for ( it = macro_idxes.begin(); it != macro_idxes.end(); it++ ) {
		script.regist_macro_usage(it->first.c_str(), this);
	}
}

MacroExpandable::~MacroExpandable()
{
	if (original) json_decref(original);
	if (expand_result) json_decref(expand_result);
}

MacroExpandable::MacroExpandable(json_t* detail):
	expand_result(NULL),
	only_macro(false),
	original(json_incref(detail))
{
	parse_tokens();
}

#define MACRO_SHARP 0x1
#define MACRO_OPEN 0x2
#define MACRO_CLOSE 0x4

void MacroExpandable::parse_tokens() throw (Exception)
{
	if ( !json_is_string(original) ) {
		expand_result = json_incref(original);
		return;
	}

	string v ( json_string_value(original));
	if ( v.size() == 0 )  {
		expand_result = json_incref(original);
		return;
	}

	//string cur_token;

	size_t  pend_pos = 0, chk_pos =  0;
	size_t ps1,ps2;

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
						macro_idxes.insert(make_pair(nm, segments.size() - 1));
					}
					else {
						chk_pos = ps2 + 1;
					}
				}
			}
			else {
				chk_pos = v.size();
			}
		}
		else {
			chk_pos = v.size();
		}

		if (chk_pos == v.size()) {
			if ( pend_pos < chk_pos ) {
				segments.push_back(v.substr(pend_pos, chk_pos - 1));
			}
			break;
		}
	} while(1);

	if ( segments.size() == 1) {
		if (macro_idxes.size() == 1) {
			only_macro = true;
		}
		else if (macro_idxes.size() == 0 ) {
			expand_result = json_incref(original);
		}
	}
}

NMSP_END(vedit)

