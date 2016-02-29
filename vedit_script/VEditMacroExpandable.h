/*
 * VEditMacroExpandable.h
 *
 *  Created on: 2016Äê2ÔÂ24ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITMACROEXPANDABLE_H_
#define VEDITMACROEXPANDABLE_H_

#include "VEditCommon.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)

class MacroExpandable
{
public:
	virtual void expand_macro(const char* nm, const json_t* value) throw (Exception) ;

	void regist_self(Script& script) throw (Exception);

protected:
	virtual ~MacroExpandable();
	MacroExpandable(json_t* detail);

	json_t* expand_result;
	bool only_macro;

	typedef hash_multimap<string,int>::iterator MapIter;
	hash_multimap<string,int> macro_idxes;
	vector<string> segments;

	bool finished() {
		return macro_idxes.size() == 0;
	}
private:
	json_t* original;
	void parse_tokens() throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITMACROEXPANDABLE_H_ */
