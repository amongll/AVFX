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
	enum EValueReplaceType
	{
		EValueNotParsed = 0,
		EValueNoReplace = 1, //����ֵ������Ҫ�滻
		EValuePositionReplace, //ֻ����������λ�������
		EValueScalarReplace, // �ַ���ֻ���������� ����
		EValueStringCtxReplace // �ַ�������ֵ�а������ɲ���
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
	std::string temp; //�����м�ֵ

	EValueReplaceType replace_type;
	json_t* evalued;
	hash_multimap<string,int> param_idxes;
	vector<string> segments;

private:
	json_t* original;
};

NMSP_END(vedit)


#endif /* VEDITVALUE_H_ */
