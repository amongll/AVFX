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
	enum EValueMask
	{
		EValueLiteral = 0x00,
		EValueProducerInResolve = 0x01, //mlt service的in/out属性必须先求值
		EValueProducerOutResolve = 0x02,
		EValuePositionResolve = 0x04, //在mlt service的in/out确定的后， 特效作用于的时间段的位置值可以求职
		EValueScalarResolve = 0x08,
		EValueMltPropResolve = 0x10
	};

	enum EValueCtxType
	{
		EValueNoReplace, //字面值，不需要替换
		EValuePositionReplace, //位置类参数替换
		EValueScalarReplace, // 字符串只包含单独的 参数或者mlt属性变量
		EValueStringCtxReplace // 参数 或者 mlt属性变量 作为 字符串的一部分
	};

	const EValueCtxType ctx_type;
	const uint32_t evalue_mask;
	const json_t* const value;

protected:
	std::string temp; //计算中间值
	uint32_t evalued_mask;
};

NMSP_END(vedit)


#endif /* VEDITVALUE_H_ */
