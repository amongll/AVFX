/*
 * VEditDefinalbe.h
 *
 *  Created on: 2016��2��23��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITDEFINALBE_H_
#define VEDITDEFINALBE_H_

#include "VEditCommon.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)

class Definable
{
public:
	virtual json_t* serialize_define()const throw (Exception) ;
protected:
	Definable(){}
	virtual ~Definable(){}
private:
	Definable(const Definable&);
	Definable& operator=(const Definable&);
};

NMSP_END(vedit)

#endif /* VEDITDEFINALBE_H_ */
