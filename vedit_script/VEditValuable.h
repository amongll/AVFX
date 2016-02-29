/*
 * VEditValue.h
 *
 *  Created on: 2016年2月23日
 *      Author: li.lei@youku.com
 */

#ifndef VEDITVALUE_H_
#define VEDITVALUE_H_

#include "VEditCommon.h"

NMSP_BEGIN(vedit)

class Evaluable
{
public:
	enum EValueReplaceType
	{
		EValueNotParsed = 0,
		EValueNoReplace = 1, //字面值，不需要替换
		EValuePositionReplace, //只包含单独的位置类参数
		EValueScalarReplace, // 字符串只包含单独的 参数
		EValueStringCtxReplace // 字符串字面值中包含若干参数
	};

	virtual void expand_scalar(const char* nm, const json_t* v) throw(Exception);
	virtual void expand_position(const char* nm, const int& frame_in, const int& frame_out,
			int frame_seq) throw(Exception);

	bool finished() const {
		return evalued != NULL;
	}

protected:
	void parse(Script& script, json_t* detail) throw (Exception);

	Evaluable();
	virtual ~Evaluable();
	std::string temp; //计算中间值

	EValueReplaceType replace_type;
	json_t* evalued;
	hash_multimap<string,int> param_idxes;
	vector<string> segments;

private:
	json_t* original;
};

NMSP_END(vedit)


#endif /* VEDITVALUE_H_ */
