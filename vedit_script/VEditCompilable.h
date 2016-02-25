/*
 * VEditCompilable.h
 *
 *  Created on: 2016��2��23��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITCOMPILABLE_H_
#define VEDITCOMPILABLE_H_

#include "VEditCommon.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)

class Compilable
{
public:
	virtual json_t* compile()const throw (Exception) = 0;
	virtual mlt_service compile_runtime() throw (Exception) = 0;
protected:
	Compilable() {} ;
	virtual ~Compilable() {};
};

NMSP_END(vedit)

#endif /* VEDITCOMPILABLE_H_ */
