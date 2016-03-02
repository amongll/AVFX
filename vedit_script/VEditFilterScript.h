/*
 * VEditFilterScript.h
 *
 *  Created on: 2016Äê2ÔÂ22ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITFILTERSCRIPT_H_
#define VEDITFILTERSCRIPT_H_


#include "VEditScript.h"

NMSP_BEGIN(vedit)

class FilterScript : public Script
{
public:
	FilterScript(json_t* text):
		Script(text)
	{}

	virtual ~FilterScript();
	virtual json_t* compile() throw(Exception);
	virtual void parse_specific() throw (Exception);
};


NMSP_END(vedit)




#endif /* VEDITFILTERSCRIPT_H_ */
