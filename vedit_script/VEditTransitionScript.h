/*
 * VEditTransitionScript.h
 *
 *  Created on: 2016Äê2ÔÂ22ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITTRANSITIONSCRIPT_H_
#define VEDITTRANSITIONSCRIPT_H_

#include "VEditScript.h"

NMSP_BEGIN(vedit)

class TransitionScript : public Script
{
public:
	TransitionScript(json_t* text):
		SingleResourceScript(text)
	{}

	virtual ~TransitionScript();
	virtual json_t* compile() throw (Exception);
	virtual void parse_specific() throw (Exception);
};

NMSP_END(vedit)



#endif /* VEDITTRANSITIONSCRIPT_H_ */
