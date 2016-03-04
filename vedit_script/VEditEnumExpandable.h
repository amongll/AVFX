/*
 * VEditEnumExpandable.h
 *
 *  Created on: 2016��2��24��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITENUMEXPANDABLE_H_
#define VEDITENUMEXPANDABLE_H_

#include "VEditCommon.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)
class Script;
class EnumExpandable
{
public:
	virtual void expand_enum(const char* ename, const char* sname,
		const char* param_tag,
		const json_t* value) throw (Exception);

	json_t* get_expanded_value() {
		if (finished())
			return expand_context;
		else {
			throw Exception(ErrorImplError, "enum expand incomplete");
		}
	}

	void register_self() throw (Exception);
	virtual ~EnumExpandable();
protected:
	EnumExpandable(Script& script, json_t* ctx, const char* oper_name)throw(Exception);
	EnumExpandable(Script& script, json_t* ctx, const vector<string>& props)throw(Exception);


	json_t* expand_context;
	Script& script;

	bool finished() const {
		return ( enum_ctx_tags.size() == 0);
	}

private:
	hash_map<string, int> value_max_tag;
	hash_map<string, int> enum_ctx_tags;
	//hash_set<string> enum_pendings;

	void parse(json_t* values) throw (Exception);
};

typedef shared_ptr<EnumExpandable> EnumExpandablePtr;

NMSP_END(vedit)



#endif /* VEDITENUMEXPANDABLE_H_ */
