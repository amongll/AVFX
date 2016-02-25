/*
 * VEditScriptParams.h
 *
 *  Created on: 2016年2月22日
 *      Author: li.lei@youku.com
 *      Desc: 脚本的输入参数，通过输入参数的指定，脚本替换为没有悬空值的版本。
 */

#ifndef VEDITSCRIPTPARAMS_H_
#define VEDITSCRIPTPARAMS_H_

#include "VEditCommon.h"
#include "VEditCompilable.h"
#include "VEditDefinable.h"
#include "VEditScriptEnums.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)
class vedit::Script;
class ScriptParams : public Definable
{
public:
	enum ParamStyle
	{
		UnknownParamStyle,
		ScalarParam, //标量类型参数，字符串，整型，浮点，布尔
		PosParam, //位置类型参数，表示内容的位置，或者计算作用于的位置
		EnumParam //用于标记一类properties的组合
	};
	enum ParamPosType
	{
		UnknownPosType,
		FramePos, //帧编号位置类型
		TimePos, //时间表达的位置类型
		PerctPos //百分比表达的位置类型
	};
	class Param
	{
	public:
		const json_t* get_define()const {
			return define_detail;
		}
	private:
		friend class ScriptParams;
		friend class shared_ptr<Param>;
		/**
		 *	\反序列化构造函数
		 */
		Param(const char* nm, json_t* detail, const Script& script) throw(Exception);

		/**
		 * 脚本参数定义构造函数。todo: script define feature
		 */
		Param(const char* nm, ParamPosType pos_type, int dv,
			bool optional=true,const char* desc=NULL) throw(Exception) :
				name(NULL),
				define_detail(NULL),
				param_style(UnknownParamStyle),
				pos_type(UnknownPosType),
				enum_name(NULL),
				default_pos(0),
				default_scalar(NULL),
				default_selector(NULL),
				desc(NULL)
		{
			throw Exception(ErrorFeatureNotImpl);
		}

		Param(const char* nm, json_type scalar_type, const json_t* dv,
			bool optional=true, const char* desc = NULL) throw(Exception):
				name(NULL),
				define_detail(NULL),
				param_style(UnknownParamStyle),
				pos_type(UnknownPosType),
				enum_name(NULL),
				default_pos(0),
				default_scalar(NULL),
				default_selector(NULL),
				desc(NULL)
		{
			throw Exception(ErrorFeatureNotImpl);
		}

		Param(const char* nm, const char* enum_type, const char* dv,
			bool optional=true, const char* desc = NULL) throw(Exception):
				name(NULL),
				define_detail(NULL),
				param_style(UnknownParamStyle),
				pos_type(UnknownPosType),
				enum_name(NULL),
				default_pos(0),
				default_scalar(NULL),
				default_selector(NULL),
				desc(NULL)
		{
			throw Exception(ErrorFeatureNotImpl);
		}

		~Param();

	public:
		const char* const name;
		const ParamStyle param_style;
		const ParamPosType pos_type;
		const char* const enum_name;
		const int default_pos;
		const char* const default_selector;
		const json_t* const default_scalar;
		const char* const desc;
	private:
		json_t* define_detail;

		void parse(const Script& script) throw (Exception);
	};

	const Param* get_param(const char* name)const {
		if (!name || !strlen(name)) return NULL;
		MapCIter it = params.find(name);
		return it == params.end() ? NULL : (it->second.get());
	}

	void define_param(const char* name, shared_ptr<Param> param)throw(Exception) {
		throw Exception(ErrorFeatureNotImpl); //todo: script define feature
	}

protected:
	virtual ~ScriptParams();
private:
	friend class vedit::Script;
	friend class Param;

	ScriptParams(Script& script) throw(Exception);

	void parse_param_defines() throw(Exception);

	json_t* defines;
	hash_map<string, shared_ptr<Param> > params;

	typedef hash_map<string, shared_ptr<Param> >::iterator MapIter;
	typedef hash_map<string, shared_ptr<Param> >::const_iterator MapCIter;
	Script& parent;
};

typedef vedit::ScriptParams::Param ScriptParam;
typedef shared_ptr<ScriptParam> ParamPtr;
typedef shared_ptr<ScriptParams> ScriptParamsPtr;

NMSP_END(vedit)

#endif /* VEDITSCRIPTPARAMS_H_ */
