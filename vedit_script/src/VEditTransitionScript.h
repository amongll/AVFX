/*
 * VEditTransitionScript.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITTRANSITIONSCRIPT_H_
#define VEDITTRANSITIONSCRIPT_H_

#include "VEditScript.h"
#include "VEditMltRun.h"

NMSP_BEGIN(vedit)

class TransitionScript : public Script
{
public:
	TransitionScript(json_t* text):
		Script(text)
	{}

	virtual ~TransitionScript();
	virtual void pre_judge() throw (Exception);
	virtual json_t* compile() throw (Exception);
	virtual void parse_specific() throw (Exception);
};

struct TransitionLoader : public MltLoader
{
	static void declare() ;

	mlt_service get_transition(JsonWrap js) throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITTRANSITIONSCRIPT_H_ */
