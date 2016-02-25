/*
 * VEditValue.h
 *
 *  Created on: 2016��2��23��
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
		EValueProducerInResolve = 0x01, //mlt service��in/out���Ա�������ֵ
		EValueProducerOutResolve = 0x02,
		EValuePositionResolve = 0x04, //��mlt service��in/outȷ���ĺ� ��Ч�����ڵ�ʱ��ε�λ��ֵ������ְ
		EValueScalarResolve = 0x08,
		EValueMltPropResolve = 0x10
	};

	enum EValueCtxType
	{
		EValueNoReplace, //����ֵ������Ҫ�滻
		EValuePositionReplace, //λ��������滻
		EValueScalarReplace, // �ַ���ֻ���������� ��������mlt���Ա���
		EValueStringCtxReplace // ���� ���� mlt���Ա��� ��Ϊ �ַ�����һ����
	};

	const EValueCtxType ctx_type;
	const uint32_t evalue_mask;
	const json_t* const value;

protected:
	std::string temp; //�����м�ֵ
	uint32_t evalued_mask;
};

NMSP_END(vedit)


#endif /* VEDITVALUE_H_ */
