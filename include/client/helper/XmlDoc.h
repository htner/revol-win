// XmlNodeWrapper.h: interface for the CXmlNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLNODEWRAPPER_H__43622334_FDEB_4175_9E6D_19BBAA3992A5__INCLUDED_)
#define AFX_XMLNODEWRAPPER_H__43622334_FDEB_4175_9E6D_19BBAA3992A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#import "MSXML3.dll"  

#ifndef xstring
#include <string>
#define xstring  std::wstring
#endif


class CXmlNode  
{
public:
	xstring NodeType();
	xstring GetAttribVal(int index);
	xstring GetAttribName(int index);
	int NumAttributes();
	void ReplaceNode(MSXML2::IXMLDOMNode* pOldNode,MSXML2::IXMLDOMNode* pNewNode);
	xstring GetText();
	void SetText(LPCTSTR text);
	MSXML2::IXMLDOMNode* InsertAfter(MSXML2::IXMLDOMNode *refNode, MSXML2::IXMLDOMNode *pNode);
	xstring Name();
	MSXML2::IXMLDOMNodeList* FindNodes(LPCTSTR searchStr);
	MSXML2::IXMLDOMNode* Parent();
	void RemoveNodes(LPCTSTR searchStr);
	MSXML2::IXMLDOMNode* InsertAfter(MSXML2::IXMLDOMNode* refNode, LPCTSTR nodeName);
	MSXML2::IXMLDOMNode* InsertBefore(MSXML2::IXMLDOMNode* refNode, LPCTSTR nodeName);
	MSXML2::IXMLDOMNode* Interface();
	MSXML2::IXMLDOMDocument* ParentDocument();
	xstring GetXML();
	MSXML2::IXMLDOMNode* RemoveNode(MSXML2::IXMLDOMNodePtr pNode);
	MSXML2::IXMLDOMNode* InsertNode(int index,LPCTSTR nodeName);
	MSXML2::IXMLDOMNode* InsertNode(int index,MSXML2::IXMLDOMNodePtr pNode);
	long NumNodes();
	MSXML2::IXMLDOMNode* Detach();
	MSXML2::IXMLDOMNode* GetNode(LPCTSTR nodeName);
	MSXML2::IXMLDOMNode* GetNode(int nodeIndex);
	MSXML2::IXMLDOMNode* FindNode(LPCTSTR searchString);
	MSXML2::IXMLDOMNode* GetPrevSibling();
	MSXML2::IXMLDOMNode* GetNextSibling();
	void SetValue(LPCTSTR valueName,LPCTSTR value);
	void SetValue(LPCTSTR valueName,int value);
	void SetValue(LPCTSTR valueName,short value);
	void SetValue(LPCTSTR valueName,double value);
	void SetValue(LPCTSTR valueName,float value);
	void SetValue(LPCTSTR valueName,bool value);
	BOOL IsValid();
	xstring GetValue(LPCTSTR valueName);

	CXmlNode();
	CXmlNode(MSXML2::IXMLDOMNodePtr pNode,BOOL bAutoRelease = TRUE);
	void operator=(MSXML2::IXMLDOMNodePtr pNode);
	virtual ~CXmlNode();
private:
	BOOL m_bAutoRelease;
	MSXML2::IXMLDOMNodePtr m_xmlnode;
};

class CXmlDocument
{
public:
	xstring GetUrl();
	xstring GetXML();
	BOOL IsValid();
	BOOL Load(LPCTSTR path);
	BOOL LoadXML(LPCTSTR xml);
	BOOL Save(LPCTSTR path = _T(""));
	MSXML2::IXMLDOMDocument* Detach();
	MSXML2::IXMLDOMDocument* Clone();
	CXmlDocument();
	CXmlDocument(MSXML2::IXMLDOMDocumentPtr pDoc);
	MSXML2::IXMLDOMNode* AsNode();
	virtual ~CXmlDocument();
private:
	MSXML2::IXMLDOMDocumentPtr m_xmldoc;
};

class CXmlNodeList
{
public:
	MSXML2::IXMLDOMDocument* AsDocument();
	MSXML2::IXMLDOMNode* Node(int index);
	void Start();
	MSXML2::IXMLDOMNode* Next();
	BOOL IsValid();
	int Count();
	CXmlNodeList();
	CXmlNodeList(MSXML2::IXMLDOMNodeListPtr pList);
	void operator=(MSXML2::IXMLDOMNodeListPtr pList);
	virtual ~CXmlNodeList();

private:
	MSXML2::IXMLDOMNodeListPtr m_xmlnodelist;
};
#include "xmldoc.hpp"

#endif // !defined(AFX_XMLNODEWRAPPER_H__43622334_FDEB_4175_9E6D_19BBAA3992A5__INCLUDED_)
