/*
 * VEditScriptParams.h
 *
 *  Created on: 2016��2��22��
 *      Author: li.lei@youku.com
 *      Desc: �ű����������ͨ����������ָ�����ű��滻Ϊû�����ֵ�İ汾��
 */

#ifndef VEDITSCRIPTPARAMS_H_
#define VEDITSCRIPTPARAMS_H_

#include "VEditCommon.h"
#include "VEditCompilable.h"
#include "VEditDefinable.h"
#include "VEditScriptEnums.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)
class Script;
class ScriptParams : public Definable
{
public:
	enum ParamStyle
	{
		UnknownParamStyle,
		ScalarParam, //json 字面值标量替换 的参数
		PosParam, // 位置类型参数
		EnumParam // 属性集合 选择 参数
	};
	enum ParamPosType
	{
		UnknownPosType,
		FramePos, //帧位置
		TimePos, //时间位置
		//PerctPos //百分比位置
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
		 *	\�����л����캯��
		 */
		Param(const char* nm, json_t* detail, const Script& script) throw(Exception);

		/**
		 *todo: script define feature
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


	public:
		~Param();
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
