/*
 * VEditMultiTrackScript.h
 *
 *  Created on: 2016Äê2ÔÂ22ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITMULTITRACKSCRIPT_H_
#define VEDITMULTITRACKSCRIPT_H_


#include "VEditScript.h"

NMSP_BEGIN(vedit)

class MultitrackScript : public Script
{
public:
	MultitrackScript(json_t* text):
		Script(text)
	{}

	virtual ~MultitrackScript();
	virtual json_t* compile() throw(Exception);
	virtual void parse_specific() throw (Exception);
};

NMSP_END(vedit)

#endif /* VEDITMULTITRACKSCRIPT_H_ */
