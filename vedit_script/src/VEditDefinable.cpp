/*
 * VEditDefinable.cpp
 *
 *  Created on: 2016��2��25��
 *      Author: li.lei@youku.com
 */

#include "VEditDefinable.h"

json_t* vedit::Definable::serialize_define() const throw (Exception)
{
	throw_error(ErrorFeatureNotImpl);
}
