/*
 * VEditScriptProps.h
 *
 *  Created on: 2016Äê2ÔÂ23ÈÕ
 *      Author: li.lei@youku.com
 */

#ifndef VEDITSCRIPTPROPS_H_
#define VEDITSCRIPTPROPS_H_

#include "VEditCommon.h"
#include "VEditValuable.h"
#include "VEditEnumExpandable.h"

NMSP_BEGIN(vedit)

class ScriptProps : public EnumExpandable
{
public:
	class Property : private Evaluable, private MacroExpandable
	{

	};
};

typedef shared_ptr<ScriptProps> ScriptPropsPtr;
typedef ScriptProps::Property ScriptProp;
typedef shared_ptr<ScriptProp> PropPtr;

NMSP_END(vedit)

#endif /* VEDITSCRIPTPROPS_H_ */
