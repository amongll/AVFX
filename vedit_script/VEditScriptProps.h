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

class Script;
class ScriptProps : public EnumExpandable
{
public:
	class Property : private Evaluable, private MacroExpandable
	{
		friend class ScriptProps;
		friend class shared_ptr<Property>;
	public:
		virtual void expand_macro(const char* nm, const json_t* value) throw (Exception);

		virtual void expand_scalar(const char* nm, const json_t* v) throw(Exception);
		virtual void expand_position(const char* nm, const int& frame_in, const int& frame_out,
				int frame_seq) throw(Exception);
	private:
		Property(ScriptProps& p, const char* nm, json_t* detail) throw (Exception);
		~Property();

		ScriptProps& parent;
	};

	virtual void expand_enum(const char* ename, const char* sname,
			const char* param_tag,
			const json_t* value) throw (Exception);

private:
	friend class shared_ptr<ScriptProps>;
	friend class Property;
	ScriptProps(Script& script, json_t* detail, const char* enum_apply_tag="$apply_props")
		throw(Exception);

	Script& get_script() {
		return script;
	}

	typedef hash_map<string, shared_ptr<Property> >::iterator MapIter;
	typedef hash_map<string, shared_ptr<Property> >::const_iterator MapCIter;
	hash_map<string, shared_ptr<Property> >  props;
};

typedef shared_ptr<ScriptProps> ScriptPropsPtr;
typedef ScriptProps::Property ScriptProp;
typedef shared_ptr<ScriptProp> PropPtr;

NMSP_END(vedit)

#endif /* VEDITSCRIPTPROPS_H_ */
