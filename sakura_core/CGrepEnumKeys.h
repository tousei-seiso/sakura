﻿/*!	@file
	
	@brief GREP support library
	
	@author wakura, Moca
	@date 2008/04/28
*/
/*
	Copyright (C) 2008, wakura
	Copyright (C) 2011, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include <vector>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "util/string_ex.h"
#include "util/file.h"

typedef std::vector< LPCTSTR > VGrepEnumKeys;

class CGrepEnumKeys {
public:
	VGrepEnumKeys m_vecSearchFileKeys;
	VGrepEnumKeys m_vecSearchFolderKeys;
	VGrepEnumKeys m_vecExceptFileKeys;
	VGrepEnumKeys m_vecExceptFolderKeys;

//	VGrepEnumKeys m_vecSearchAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFileKeys;
	VGrepEnumKeys m_vecExceptAbsFolderKeys;

public:
	CGrepEnumKeys(){
	}

	~CGrepEnumKeys(){
		ClearItems();
	}

	int SetFileKeys( LPCTSTR lpKeys ){
		const TCHAR* WILDCARD_ANY = _T("*.*");	//サブフォルダ探索用
		ClearItems();
		
		std::vector< tstring > patterns = SplitPattern(lpKeys);
		for (size_t i = 0; i < patterns.size(); i++) {
			const tstring& element = patterns[i];
			const TCHAR* token = element.c_str();

			//フィルタを種類ごとに振り分ける
			enum KeyFilterType{
				FILTER_SEARCH,
				FILTER_EXCEPT_FILE,
				FILTER_EXCEPT_FOLDER,
			};
			KeyFilterType keyType = FILTER_SEARCH;
			if( token[0] == _T('!') ){
				token++;
				keyType = FILTER_EXCEPT_FILE;
			}else if( token[0] == _T('#') ){
				token++;
				keyType = FILTER_EXCEPT_FOLDER;
			}

			bool bRelPath = _IS_REL_PATH( token );
			int nValidStatus = ValidateKey( token );
			if( 0 != nValidStatus ){

				return nValidStatus;
			}
			if( keyType == FILTER_SEARCH ){
				if( bRelPath ){
					push_back_unique( m_vecSearchFileKeys, token );
				}else{
//					push_back_unique( m_vecSearchAbsFileKeys, token );
//					push_back_unique( m_vecSearchFileKeys, token );
					return 2; // 絶対パス指定は不可
				}
			}else if( keyType == FILTER_EXCEPT_FILE ){
				if( bRelPath ){
					push_back_unique( m_vecExceptFileKeys, token );
				}else{
					push_back_unique( m_vecExceptAbsFileKeys, token );
				}
			}else if( keyType == FILTER_EXCEPT_FOLDER ){
				if( bRelPath ){
					push_back_unique( m_vecExceptFolderKeys, token );
				}else{
					push_back_unique( m_vecExceptAbsFolderKeys, token );
				}
			}
		}
		if( m_vecSearchFileKeys.size() == 0 ){
			push_back_unique( m_vecSearchFileKeys, WILDCARD_ANY );
		}
		if( m_vecSearchFolderKeys.size() == 0 ){
			push_back_unique( m_vecSearchFolderKeys, WILDCARD_ANY );
		}
		return 0;
	}

	/*!
		@brief 除外ファイルパターンを追加する
		@param[in]	lpKeys	除外ファイルパターン
	*/
	int AddExceptFile(LPCTSTR lpKeys) {
		return ParseAndAddException(lpKeys, m_vecExceptFileKeys, m_vecExceptAbsFileKeys);
	}

	/*!
		@brief 除外フォルダパターンを追加する
		@param[in]	lpKeys	除外フォルダパターン
	*/
	int AddExceptFolder(LPCTSTR lpKeys) {
		return ParseAndAddException(lpKeys, m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);
	}


private:
	void ClearItems( void ){
		ClearEnumKeys(m_vecExceptFileKeys);
		ClearEnumKeys(m_vecSearchFileKeys);
		ClearEnumKeys(m_vecExceptFolderKeys);
		ClearEnumKeys(m_vecSearchFolderKeys);
		return;
	}
	void ClearEnumKeys( VGrepEnumKeys& keys ){
		for( int i = 0; i < (int)keys.size(); i++ ){
			delete [] keys[ i ];
		}
		keys.clear();
	}

	void push_back_unique( VGrepEnumKeys& keys, LPCTSTR addKey ){
		if( ! IsExist( keys, addKey) ){
			TCHAR* newKey = new TCHAR[ _tcslen( addKey ) + 1 ];
			_tcscpy( newKey, addKey );
			keys.push_back( newKey );
		}
	}

	BOOL IsExist( VGrepEnumKeys& keys, LPCTSTR addKey ){
		for( int i = 0; i < (int)keys.size(); i++ ){
			if( _tcscmp( keys[ i ], addKey ) == 0 ){
				return TRUE;
			}
		}
		return FALSE;
	}

	/*
		@retval 0 正常終了
		@retval 1 *\file.exe などのフォルダ部分でのワイルドカードはエラー
	*/
	int ValidateKey( LPCTSTR key ){
		// 
		bool wildcard = false;
		for( int i = 0; key[i]; i++ ){
			if( !wildcard && (key[i] == _T('*') || key[i] == _T('?')) ){
				wildcard = true;
			}else if( wildcard && (key[i] == _T('\\') || key[i] == _T('/')) ){
				return 1;
			}
		}
		return 0;
	}

	typedef std::basic_string<TCHAR> tstring;

	/*!
		@brief ファイルパターンを解析して、要素ごとに分離して返す
		@param[in]		lpKeys					ファイルパターン
	*/
	std::vector< tstring > SplitPattern(LPCTSTR lpKeys)
	{
		std::vector< tstring > patterns;

		const TCHAR* WILDCARD_DELIMITER = _T(" ;,");	//リストの区切り
		int nWildCardLen = _tcslen(lpKeys);
		TCHAR* pWildCard = new TCHAR[nWildCardLen + 1];
		if (!pWildCard) {
			return patterns;
		}
		_tcscpy(pWildCard, lpKeys);

		int nPos = 0;
		TCHAR*	token;
		while (NULL != (token = my_strtok<TCHAR>(pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER))) {	//トークン毎に繰り返す。
			// "を取り除いて左に詰める
			TCHAR* p;
			TCHAR* q;
			p = q = token;
			while (*p) {
				if (*p != _T('"')) {
					if (p != q) {
						*q = *p;
					}
					q++;
				}
				p++;
			}
			*q = _T('\0');

			tstring element(token);
			patterns.push_back(element);
		}
		delete[] pWildCard;
		return patterns;
	}

	/*!
		@brief 除外ファイルパターンを追加する
		@param[in]		lpKeys					除外ファイルパターン
		@param[in,out]	exceptionKeys			除外ファイルパターンの解析結果を追加する
		@param[in,out]	exceptionAbsoluteKeys	除外ファイルパターンの絶対パスの解析結果を追加する
	*/
	int ParseAndAddException(LPCTSTR lpKeys, VGrepEnumKeys& exceptionKeys, VGrepEnumKeys & exceptionAbsoluteKeys) {
		std::vector< tstring > patterns = SplitPattern(lpKeys);

		for (size_t i = 0; i < patterns.size(); i++) {
			const tstring& element = patterns[i];
			const TCHAR* token = element.c_str();

			bool bRelPath = _IS_REL_PATH(token);
			int nValidStatus = ValidateKey(token);
			if (0 != nValidStatus) {
				return nValidStatus;
			}
			if (bRelPath) {
				push_back_unique(exceptionKeys, token);
			}
			else {
				push_back_unique(exceptionAbsoluteKeys, token);
			}
		}
		return 0;
	}
};


