/*
 * VEditValue.h
 *
 *  Created on: 2016��2��23��
 *      Author: li.lei@youku.com
 */

#ifndef VEDITVALUE_H_
#define VEDITVALUE_H_

#include "VEditCommon.h"
#include "VEditException.h"

NMSP_BEGIN(vedit)

class Script;
class Evaluable
{
public:
	enum EValueReplaceType
	{
		EValueNotParsed = 0,
		EValueNoReplace = 1, //字面值
		EValuePositionReplace, //仅包含一个位置类型参数
		EValueScalarReplace, //仅包含一个非位置类型参数
		EValueStringCtxReplace, //包含多个非位置类型参数
		EValueStringCtxReplace2 //包含多个参数，其中包含位置类型参数
	};

	virtual void expand_scalar(const char* nm, const json_t* v) throw(Exception);
	virtual void expand_position(const char* nm, const int& frame_in, const int& frame_out,
			int frame_seq) throw(Exception);

	//void apply_params(Script& script, json_t* args) throw (Exception);

	void replace_asis(const char* nm, const string& v) throw (Exception);
	void replace_asis(const char* nm, int v) throw (Exception);
	void replace_asis(const char* nm, double v) throw (Exception);
	void replace_asis(const char* nm, bool v) throw (Exception);

	bool finished() const {
		return evalued != NULL;
	}

protected:
	void parse(Script& script, json_t* detail) throw (Exception);

	Evaluable();
	virtual ~Evaluable();
	std::string temp; //�����м�ֵ

	EValueReplaceType replace_type;
	json_t* evalued;

	typedef  hash_multimap<string,int>::iterator MapIter;
	hash_multimap<string,int> param_idxes;
	vector<string> segments;

private:
	json_t* original;
};

NMSP_END(vedit)


#endif /* VEDITVALUE_H_ */
