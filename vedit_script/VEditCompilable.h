/*
 * VEditCompilable.h
 *
 *  Created on: 2016Äê2ÔÂ23ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITCOMPILABLE_H_
#define VEDITCOMPILABLE_H_

#include "VEditCommon.h"
#include "VEditMltRun.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)

class Compilable
{
public:
	virtual json_t* compile() throw (Exception) = 0;
protected:
	Compilable() {} ;
	virtual ~Compilable() {};
};

NMSP_END(vedit)

#endif /* VEDITCOMPILABLE_H_ */
