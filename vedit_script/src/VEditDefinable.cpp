/*
 * VEditDefinable.cpp
 *
 *  Created on: 2016Äê2ÔÂ25ÈÕ
 *      Author: li.lei@youku.com
 */

#include "VEditDefinable.h"

json_t* vedit::Definable::serialize_define() const throw (Exception)
{
	throw Exception(ErrorFeatureNotImpl);
}
