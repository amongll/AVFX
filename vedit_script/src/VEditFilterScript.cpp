/*
 * VEditFilterScript.cpp
 *
 *  Created on: 2016年3月7日
 *      Author: li.lei@youku.com
 */

#include "VEditFilterScript.h"
#include "VEditVM.h"

NMSP_BEGIN(vedit)

FilterScript::~FilterScript()
{
}

json_t* FilterScript::compile() throw (Exception)
{
	return NULL;
}

void FilterScript::parse_specific() throw (Exception)
{
	vector<string> specs;
	specs.push_back("filter");
	parse_specific_props(specs);
}

void FilterScript::pre_judge() throw (Exception)
{

}

NMSP_END(vedit)
