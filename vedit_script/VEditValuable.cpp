/*
 * VEditValuable.cpp
 *
 *  Created on: 2016Äê2ÔÂ29ÈÕ
 *      Author: li.lei@youku.com
 */


#include "VEditValuable.h"

NMSP_BEGIN(vedit)
void Evaluable::expand_scalar(const char* nm, const json_t* v) throw (Exception)
{
}

void Evaluable::expand_position(const char* nm, const int& frame_in,
		const int& frame_out, int frame_seq) throw (Exception)
{
}

void Evaluable::parse(Script& script, json_t* detail) throw (Exception)
{
	assert( !json_is_object(detail) && !json_is_array(detail) ) ;
	original = json_incref(detail);

	if ( !json_is_string(original) ) {
		evalued = json_incref(original);
		return;
	}


}

Evaluable::Evaluable():
		replace_type(EValueNotParsed),
		evalued(NULL),
		original(NULL)
{
}

Evaluable::~Evaluable()
{
	if ( evalued) json_decref(evalued);
	if (original) json_decref(original);
}


NMSP_END(vedit)

