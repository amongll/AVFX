/*
 * VEditScriptProps.h
 *
 *  Created on: 2016��2��23��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITSCRIPTPROPS_H_
#define VEDITSCRIPTPROPS_H_

#include "VEditCommon.h"
#include "VEditValuable.h"
#include "VEditEnumExpandable.h"
#include "VEditMacroExpandable.h"

NMSP_BEGIN(vedit)

class Script;
class ScriptProps : public EnumExpandable
{
public:
	class PropertyAgent
	{
	public:
		class Property;
		friend class Property;

		PropertyAgent(const set<string>& delgs);

		virtual void expand_scalar(Evaluable& e,const char* nm, const json_t* v) throw(Exception)=0;
		virtual void expand_position(Evaluable& e, const char* nm, const int& frame_in, const int& frame_out,
				int frame_seq) throw(Exception)=0;

	private:
		set<string> delegates;
	};

	class Property : public Evaluable, public MacroExpandable
	{
		friend class ScriptProps;
		friend class shared_ptr<Property>;
	public:
		virtual void expand_macro(const char* nm, const json_t* value) throw (Exception);

		virtual void expand_scalar(const char* nm, const json_t* v) throw(Exception);
		virtual void expand_position(const char* nm, const int& frame_in, const int& frame_out,
				int frame_seq) throw(Exception);

		void regist_delegate(shared_ptr<PropertyAgent> agent) {
			this->agent = agent;
		}

		json_t* compile() throw (Exception) {
			if ( !MacroExpandable::finished() ) {
				throw Exception(ErrorImplError, "property resolved incompletely");
			}
			if ( !Evaluable::finished() ) {
				throw Exception(ErrorImplError, "property resolved incompletely");
			}
			return json_incref(Evaluable::evalued);
		}
	private:
		Property(ScriptProps& p, const char* nm, json_t* detail) throw (Exception);
		virtual ~Property();

		shared_ptr<PropertyAgent> agent;
		ScriptProps& parent;
	};

	virtual void expand_enum(const char* ename, const char* sname,
			const char* param_tag,
			const json_t* value) throw (Exception);

	json_t* compile() throw (Exception);

	Property& get_property(const char* nm) throw (Exception);

	Property& add_property(const char* nm, json_t* value) throw (Exception);

	ScriptProps(Script& script, const vector<string>& spec_props) throw(Exception);
	ScriptProps(Script& script, json_t* detail, const char* enum_apply_tag="$apply_props")
		throw(Exception);
private:
	friend class shared_ptr<ScriptProps>;
	friend class ScriptProps::Property;
	friend class Script;


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
