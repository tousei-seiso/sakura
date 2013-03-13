/*!	@file
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O

	@author Norio Nakatani
	@date 1998/12/23 �V�K�쐬
	@date 1999/12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgTypeList;

#ifndef _CDLGTYPELIST_H_
#define _CDLGTYPELIST_H_

const int PROP_TEMPCHANGE_FLAG = 0x10000;

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O
*/
class CDlgTypeList : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND, int* );	/* ���[�_���_�C�A���O�̕\�� */

	int				m_nSettingType;
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	void SetData();	/* �_�C�A���O�f�[�^�̐ݒ� */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */


/*[EOF]*/