/*
 * VEditScript.h
 *
 *  Created on: 2016年2月22日
 *      Author: li.lei@youku.com
 */

#ifndef VEDITSCRIPT_H_
#define VEDITSCRIPT_H_

#include "VEditCommon.h"
#include "VEditCompilable.h"
#include "VEditDefinable.h"
#include "VEditScriptParams.h"
#include "VEditScriptEnums.h"
#include "VEditScriptMacros.h"
#include "VEditScriptProps.h"
#include "VEditException.h"
#include <framework/mlt.h>

NMSP_BEGIN(vedit)

class Script : public Definable, public Compilable
{
public:
	const char* const proc_name;
	const char* const proc_type;
	const char* const desc;

	void applie_params(const json_t* param_values) throw(Exception);
	void applie_properties(const mlt_properties* properties) throw(Exception);

	const json_t* get_params_define()const;
	const json_t* get_macros_define()const;
	const json_t* get_enums_define()const;

	virtual void add_params(ScriptParamsPtr params) throw(Exception) ;

	virtual void add_macros(ScriptMacrosPtr macros) throw(Exception) ;

	virtual bool add_enums(ScriptEnumsPtr enums) throw (Exception) ;

	const json_t* get_macro(const char* macro_name) const {
		return macros->get_macro(macro_name);
	}
	const json_t* get_selector(const char* enum_name, const char* sel_name)const {
		return enums->get_selector(enum_name,sel_name);
	}
	bool has_enum(const char* ename) const {
		return enums->has_enum(ename);
	}

protected:
	friend class shared_ptr<Script>;
	Script() throw (Exception):
		proc_name(NULL),
		proc_type(NULL),
		desc(NULL),
		defines(NULL)
	{
		throw Exception(ErrorFeatureNotImpl);//todo: script define feature
	}

	virtual ~Script();

	json_t* defines;

	ScriptParamsPtr params;
	ScriptMacrosPtr macros;
	ScriptEnumsPtr enums;

	ScriptPropsPtr	proc_spec_props; //特定proctype所持有的特定属性
	ScriptPropsPtr	mlt_props; //最终应用于mlt service部件的属性

	void regist_macro_usage(const char* macro, MacroExpandable* obj) throw (Exception);
	void regist_enum_selector_usage(const char* enmae, const char* sname, EnumExpandable* obj) throw (Exception);
	void regist_enum_param_usage(const char* param, EnumExpandable* obj) throw (Exception);
	void regist_scalar_param_usage(const char* param, Evaluable* obj) throw (Exception);
	void regist_mlt_property_usage(const char* property, Evaluable* obj) throw (Exception);

private:

	hash_multimap<string, shared_ptr<EnumExpandable> > selector_enum_presents;
	hash_multimap<string, shared_ptr<EnumExpandable> > param_enum_presents;
	hash_multimap<string, shared_ptr<MacroExpandable> > macro_presents;
	hash_multimap<string, shared_ptr<Evaluable> > param_evalue_presents;
	hash_multimap<string, shared_ptr<Evaluable> > mlt_evalue_presents;

	typedef hash_multimap<string, shared_ptr<EnumExpandable> >::iterator EnumExpandableIter;
	typedef hash_multimap<string, shared_ptr<MacroExpandable> >::iterator MacroExpandableIter;
	typedef hash_multimap<string, shared_ptr<Evaluable> >::iterator EvaluableIter;

	hash_map<Evaluable*, shared_ptr<Evaluable> > all_pendings;
	typedef hash_map<Evaluable*, shared_ptr<Evaluable> >::iterator EvaluableCheckIter;
};

typedef shared_ptr<Script> ScriptPtr;

NMSP_END(vedit)


#endif /* VEDITSCRIPT_H_ */
