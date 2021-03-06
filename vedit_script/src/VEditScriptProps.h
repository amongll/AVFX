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
		friend class Property;

		PropertyAgent(const set<string>& delgs);

		virtual void expand_scalar(Evaluable& e,const char* nm, const json_t* v) throw(Exception)=0;
		virtual void expand_position(Evaluable& e, const char* nm, const int& frame_in, const int& frame_out,
				int frame_seq) throw(Exception)=0;

		set<string> delegates;
	};

	class Property : public Evaluable, public MacroExpandable
	{
		friend class ScriptProps;
	public:
		virtual void expand_macro(const char* nm, const json_t* value) throw (Exception);

		virtual void expand_scalar(const char* nm, const json_t* v) throw(Exception);
		virtual void expand_position(const char* nm, const int& frame_in, const int& frame_out,
				int frame_seq) throw(Exception);

		void regist_delegate(std::tr1::shared_ptr<PropertyAgent> agent) {
			this->agent = agent;
		}

		json_t* compile() const throw (Exception) {
			if ( !MacroExpandable::finished() ) {
				throw_error_v(ErrorImplError, "property resolved incompletely");
			}
			if ( !Evaluable::finished() ) {
				throw_error_v(ErrorImplError, "property resolved incompletely");
			}
			return json_incref(Evaluable::evalued);
		}
		virtual ~Property();
	private:
		Property(ScriptProps& p, const char* nm, json_t* detail) throw (Exception);

		std::tr1::shared_ptr<PropertyAgent> agent;
		ScriptProps& parent;
	};

	virtual void expand_enum(const char* ename, const char* sname,
			const char* param_tag,
			const json_t* value) throw (Exception);

	json_t* compile() throw (Exception);

	Property& get_property(const char* nm) throw (Exception);

	Property& add_property(const char* nm, json_t* value) throw (Exception);

	ScriptProps(Script& script, const vector<string>& spec_props) throw(Exception);
	ScriptProps(Script& script, json_t* detail, const vector<string>& spec_props)throw(Exception);
	ScriptProps(Script& script, json_t* detail, const char* enum_apply_tag="$apply_props")
		throw(Exception);

	typedef hash_map<string, std::tr1::shared_ptr<Property> >::const_iterator MapCIter;
	typedef MapCIter PropIter;

	PropIter begin() const
	{
		return props.begin();
	}
	PropIter end() const
	{
		return props.end();
	}
private:
	friend class ScriptProps::Property;
	friend class Script;

	Script& get_script() {
		return script;
	}
	typedef hash_map<string, std::tr1::shared_ptr<Property> >::iterator MapIter;

	hash_map<string, std::tr1::shared_ptr<Property> >  props;
};

typedef std::tr1::shared_ptr<ScriptProps> ScriptPropsPtr;
typedef ScriptProps::Property ScriptProp;
typedef std::tr1::shared_ptr<ScriptProp> PropPtr;

NMSP_END(vedit)

#endif /* VEDITSCRIPTPROPS_H_ */
