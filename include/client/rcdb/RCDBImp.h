#pragma once
#include <xstring>
#include <client/xbase.h>

NAMESPACE_BEGIN(rcdb)

class SQLiteSet
{
public:
	enum EXCUTE_RESULT
	{
		ER_ERROR	= 0,
		ER_OK,
		ER_ROW,
		ER_DONE
	};

};

struct IRCDB;

struct IRCMsgQueue
{
	virtual BOOL Push(LPCTSTR ) = 0;
	virtual void SetDB(IRCDB* pDB) = 0;
};

/// »Øµ÷º¯Êý
typedef int (*DBFunctionCallback)(void*,int,char**, char**);

struct IRCDB : public IObject
{
	//open DB
	virtual BOOL OpenDatabase(LPCTSTR lpfile,LPCTSTR lpPsw = NULL) = 0;

	/// the DB is close
 	virtual BOOL CreateTable(LPCTSTR lpTable,LPCTSTR lpPsw) = 0;

	//the DB is Open
	virtual BOOL CreateTable(LPCTSTR lpTable) = 0;
 
 	// close DB
 	virtual BOOL CloseDatabase() =0;

	// set Password
	virtual BOOL SetDBPassword(LPCTSTR lpsw) = 0;

	// reset Password

	virtual BOOL ResetDBPasswrod(LPCTSTR lpswNew, LPCTSTR lpswOld) = 0;
 
 	// execute sql 
 	virtual BOOL ExeSql(LPCTSTR sql) =0;

	// execute with 
	virtual BOOL ExeSqlImmediately(LPCTSTR sql) =0;

	// 
	virtual void SetQueueHolder(IRCMsgQueue* pHolder) = 0;

	// do query , the call back can be rev info,internal sure safety
	virtual BOOL Query(LPCTSTR sql, DBFunctionCallback lpCallback ,void* result) =0;

	// do query,for the statement to control, Must take the initiative to call Finalize
	virtual BOOL Query(LPCTSTR sql) = 0;

	//for step
	virtual int Step() = 0;
	
	// Fetch column text
	virtual void FetchColText(int nCol, __out std::wstring& text) = 0;

	// for statement finalize
	virtual void Finalize() = 0;

	// Get column count
 	virtual int GetColumnCount() = 0; 

	// begin transaction; lock this
	virtual BOOL BeginTransaction() = 0;
	//commit transaction; unlock this
	virtual BOOL CommitTransaction() = 0;
	//rollback transaction;
	virtual BOOL RollBackTransaction() = 0;

	virtual BOOL IsDBOpen() = 0;

	virtual void IsContaintTable(LPCTSTR lptable, __out BOOL& bContaint) = 0;

	// Get row count
	virtual int GetTableRowCount(LPCTSTR lpCondition) = 0;

	virtual void Release() = 0;

	virtual LPCTSTR GetDBFile() = 0;


};



extern "C" __declspec(dllexport) bool CoCreateDB(IRCDB** db);

NAMESPACE_END(rcdb)