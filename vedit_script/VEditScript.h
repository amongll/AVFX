/*
 * VEditScript.h
 *
 *  Created on: 2016��2��22��
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
		defines(NULL),
		compiled(NULL),
		frame_in(0),
		frame_out(-1),
		args(NULL)
	{
		throw Exception(ErrorFeatureNotImpl);//todo: script define feature
	}

	Script(const json_t* detail) throw (Exception);
	virtual ~Script();

	virtual void parse_specific() throw (Exception) = 0;

	json_t* defines;
	json_t* compiled;

	ScriptParamsPtr params;
	ScriptMacrosPtr macros;
	ScriptEnumsPtr enums;

	ScriptPropsPtr  type_spec_props;
	ScriptPropsPtr	mlt_props; //����Ӧ����mlt service����������

	void regist_macro_usage(const char* macro, MacroExpandable* obj) throw (Exception);
	void regist_enum_selector_usage(const char* enmae, const char* sname, EnumExpandable* obj) throw (Exception);
	void regist_enum_param_usage(const char* param, EnumExpandable* obj) throw (Exception);
	void regist_scalar_param_usage(const char* param, Evaluable* obj) throw (Exception);

	json_t* get_arg_value(const char* nm) throw (Exception);
	void set_frame_range(int in, int out) throw (Exception);

	int frame_in;
	int frame_out;
private:

	json_t* args;

	void parse_impl() throw (Exception);
	void parse_mlt_props(json_t* detail) throw (Exception);

	void applies_selectors() throw (Exception); //�������֮������Ӧ�ýű��г��ֵ�#(enum:selector)����
	void applies_macros() throw (Exception);  //��Ӧ��#(macro)����

	void check_evaluables() ;


	hash_multimap<string, shared_ptr<EnumExpandable> > selector_enum_presents;
	hash_multimap<string, shared_ptr<MacroExpandable> > macro_presents;

	hash_multimap<string, shared_ptr<EnumExpandable> > param_enum_presents;
	hash_multimap<string, shared_ptr<Evaluable> > param_evalue_presents;

	typedef hash_multimap<string, shared_ptr<EnumExpandable> >::iterator EnumExpandableIter;
	typedef hash_multimap<string, shared_ptr<MacroExpandable> >::iterator MacroExpandableIter;
	typedef hash_multimap<string, shared_ptr<Evaluable> >::iterator EvaluableIter;

	hash_map<Evaluable*, shared_ptr<Evaluable> > all_pendings;
	typedef hash_map<Evaluable*, shared_ptr<Evaluable> >::iterator EvaluableCheckIter;
};

typedef shared_ptr<Script> ScriptPtr;

NMSP_END(vedit)


#endif /* VEDITSCRIPT_H_ */