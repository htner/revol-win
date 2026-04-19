//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

inline CXmlNode::CXmlNode()
{
	m_bAutoRelease = TRUE;
}

inline CXmlNode::CXmlNode(MSXML2::IXMLDOMNodePtr pNode,BOOL bAutoRelease)
{
	m_xmlnode.Attach(pNode);
	m_bAutoRelease = bAutoRelease;
}
inline void CXmlNode::operator=(MSXML2::IXMLDOMNodePtr pNode)
{
	if (IsValid())
		m_xmlnode.Release();
	m_xmlnode.Attach(pNode);
}

inline CXmlNode::~CXmlNode()
{
	if (!m_bAutoRelease)
		m_xmlnode.Detach();
}


inline xstring CXmlNode::GetValue(LPCTSTR valueName)
{
	if (!IsValid())
		return _T("");

	MSXML2::IXMLDOMNodePtr attribute = m_xmlnode->Getattributes()->getNamedItem(valueName);
	if (attribute)
	{
		return (LPCTSTR)attribute->Gettext();
	}
	return _T("");
}

inline BOOL CXmlNode::IsValid()
{
	if (m_xmlnode == NULL)
		return FALSE;
	if (m_xmlnode.GetInterfacePtr() == NULL)
		return FALSE;
	return TRUE;
}

inline MSXML2::IXMLDOMNode* CXmlNode::GetPrevSibling()
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->GetpreviousSibling().Detach();
}

inline MSXML2::IXMLDOMNode* CXmlNode::GetNextSibling()
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->GetnextSibling().Detach();
}

inline MSXML2::IXMLDOMNode* CXmlNode::GetNode(LPCTSTR nodeName)
{
	if (!IsValid())
		return NULL;
	try{
		return m_xmlnode->selectSingleNode(nodeName).Detach();
	}
	catch (_com_error e)
	{
		xstring err = e.ErrorMessage();
	}
	return NULL;
}

inline MSXML2::IXMLDOMNode* CXmlNode::GetNode(int nodeIndex)
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->GetchildNodes()->Getitem(nodeIndex).Detach();
}

inline MSXML2::IXMLDOMNode* CXmlNode::FindNode(LPCTSTR searchString)
{
	if (!IsValid())
		return NULL;
	try{
		return m_xmlnode->selectSingleNode(searchString).Detach();
	}
	catch (_com_error e)
	{
		xstring err = e.ErrorMessage();
	}
	return NULL;
}

inline MSXML2::IXMLDOMNode* CXmlNode::Detach()
{
	if (IsValid())
	{
		return m_xmlnode.Detach();
	}
	else
		return NULL;
}

inline long CXmlNode::NumNodes()
{
	if (IsValid())
	{
		return m_xmlnode->GetchildNodes()->Getlength();
	}
	else
		return 0;
}

inline void CXmlNode::SetValue(LPCTSTR valueName,LPCTSTR value)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNamedNodeMapPtr attributes = m_xmlnode->Getattributes();
		if (attributes)
		{
			MSXML2::IXMLDOMAttributePtr attribute = xmlDocument->createAttribute(valueName);
			if (attribute)
			{
				attribute->Puttext(value);
				attributes->setNamedItem(attribute);
			}
		}
	}
}



inline MSXML2::IXMLDOMNode* CXmlNode::InsertNode(int index,LPCTSTR nodeName)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName,_T(""));
		MSXML2::IXMLDOMNode* refNode = GetNode(index);
		if (refNode)
			newNode = m_xmlnode->insertBefore(newNode.Detach(),refNode);
		else
			newNode = m_xmlnode->appendChild(newNode.Detach());
		return newNode.Detach();
	}
	return NULL;
}

inline MSXML2::IXMLDOMNode* CXmlNode::InsertNode(int index,MSXML2::IXMLDOMNodePtr pNode)
{
	MSXML2::IXMLDOMNodePtr newNode = pNode->cloneNode(VARIANT_TRUE);
	if (newNode)
	{
		MSXML2::IXMLDOMNode* refNode = GetNode(index);
		if (refNode)
			newNode = m_xmlnode->insertBefore(newNode.Detach(),refNode);
		else
			newNode = m_xmlnode->appendChild(newNode.Detach());
		return newNode.Detach();
	}
	else
		return NULL;
}

inline xstring CXmlNode::GetXML()
{
	if (IsValid())
		return (LPCTSTR)m_xmlnode->Getxml();
	else
		return _T("");
}

inline MSXML2::IXMLDOMNode* CXmlNode::RemoveNode(MSXML2::IXMLDOMNodePtr pNode)
{
	if (!IsValid())
		return NULL;
	return m_xmlnode->removeChild(pNode).Detach();
}

extern "C" const GUID __declspec(selectany) CLSID_XXDOMDocument =
    {0xf6d90f11,0x9c73,0x11d3,{0xb3,0x2e,0x00,0xc0,0x4f,0x99,0x0b,0xb4}};



/* ********************************************************************************************************* */
inline CXmlDocument::CXmlDocument()
{
	m_xmldoc.CreateInstance(CLSID_XXDOMDocument);
}

inline CXmlDocument::CXmlDocument(MSXML2::IXMLDOMDocumentPtr pDoc)
{
	m_xmldoc.Attach(pDoc);
}

inline CXmlDocument::~CXmlDocument()
{
}

inline BOOL CXmlDocument::IsValid()
{
	if (m_xmldoc == NULL)
		return FALSE;
	if (m_xmldoc.GetInterfacePtr() == NULL)
		return FALSE;
	return TRUE;
}

inline MSXML2::IXMLDOMDocument* CXmlDocument::Detach()
{
	if (!IsValid())
		return NULL;
	return m_xmldoc.Detach();
}

inline MSXML2::IXMLDOMDocument* CXmlDocument::Clone()
{
	if (!IsValid())
		return NULL;
	MSXML2::IXMLDOMDocumentPtr xmldoc;
	xmldoc.CreateInstance(CLSID_XXDOMDocument);
	_variant_t v(xmldoc.GetInterfacePtr());
	m_xmldoc->save(v);
	return xmldoc.Detach();
}

inline BOOL CXmlDocument::Load(LPCTSTR path)
{
	if (!IsValid())
		return FALSE;

	_variant_t v(path);
	m_xmldoc->put_async(VARIANT_FALSE);
	VARIANT_BOOL success = m_xmldoc->load(v);
	if (success == VARIANT_TRUE)
		return TRUE;
	else
		return FALSE;
}

inline BOOL CXmlDocument::LoadXML(LPCTSTR xml)
{
	if (!IsValid())
		return FALSE;
	VARIANT_BOOL success = m_xmldoc->loadXML(xml);
	if (success == VARIANT_TRUE)
		return TRUE;
	else
		return FALSE;
}

inline BOOL CXmlDocument::Save(LPCTSTR path)
{
	try
	{
		if (!IsValid())
			return FALSE;
		xstring szPath(path);
		if (szPath == _T(""))
		{
			_bstr_t curPath = m_xmldoc->Geturl();
			szPath = (LPTSTR)curPath;
		}
		_variant_t v(szPath.c_str());
		if (FAILED(m_xmldoc->save(v)))
			return FALSE;
		else
			return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

inline MSXML2::IXMLDOMNode* CXmlDocument::AsNode()
{
	if (!IsValid())
		return NULL;
	return m_xmldoc->GetdocumentElement().Detach();
}

inline xstring CXmlDocument::GetXML()
{
	if (IsValid())
		return (LPCTSTR)m_xmldoc->Getxml();
	else
		return _T("");
}

inline xstring CXmlDocument::GetUrl()
{
	return (LPTSTR)m_xmldoc->Geturl();
}

inline MSXML2::IXMLDOMDocument* CXmlNode::ParentDocument()
{
	return m_xmlnode->GetownerDocument().Detach();
}

inline MSXML2::IXMLDOMNode* CXmlNode::Interface()
{
	if (IsValid())
		return m_xmlnode;
	return NULL;
}

inline MSXML2::IXMLDOMNode* CXmlNode::InsertBefore(MSXML2::IXMLDOMNode *refNode, LPCTSTR nodeName)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName,_T(""));
		newNode = m_xmlnode->insertBefore(newNode.Detach(),(IUnknown*)refNode);
		return newNode.Detach();
	}
	return NULL;
}

inline MSXML2::IXMLDOMNode* CXmlNode::InsertAfter(MSXML2::IXMLDOMNode *refNode, LPCTSTR nodeName)
{
	MSXML2::IXMLDOMDocumentPtr xmlDocument = m_xmlnode->GetownerDocument();
	if (xmlDocument)
	{
		MSXML2::IXMLDOMNodePtr newNode = xmlDocument->createNode(_variant_t((short)MSXML2::NODE_ELEMENT),nodeName,_T(""));
		MSXML2::IXMLDOMNodePtr nextNode = refNode->GetnextSibling();
		if (nextNode.GetInterfacePtr() != NULL)
			newNode = m_xmlnode->insertBefore(newNode.Detach(),(IUnknown*)nextNode);
		else
			newNode = m_xmlnode->appendChild(newNode.Detach());
		return newNode.Detach();
	}
	return NULL;
}

inline void CXmlNode::RemoveNodes(LPCTSTR searchStr)
{
	if (!IsValid())
		return;
	MSXML2::IXMLDOMNodeListPtr nodeList = m_xmlnode->selectNodes(searchStr);
	for (int i = 0; i < nodeList->Getlength(); i++)
	{
		try
		{
			MSXML2::IXMLDOMNode* pNode = nodeList->Getitem(i).Detach();
			pNode->GetparentNode()->removeChild(pNode);
		}
		catch (_com_error er)
		{
			OutputDebugString(er.ErrorMessage());
		}
	}
}

inline MSXML2::IXMLDOMNode* CXmlNode::Parent()
{
	if (IsValid())
		return m_xmlnode->GetparentNode().Detach();
	return NULL;
}

inline CXmlNodeList::CXmlNodeList()
{
}
inline CXmlNodeList::CXmlNodeList(MSXML2::IXMLDOMNodeListPtr pList)
{
	m_xmlnodelist.Attach(pList);
}

inline void CXmlNodeList::operator=(MSXML2::IXMLDOMNodeListPtr pList)
{
	if (IsValid())
		m_xmlnodelist.Release();
	m_xmlnodelist.Attach(pList);
}

inline CXmlNodeList::~CXmlNodeList()
{

}

inline int CXmlNodeList::Count()
{
	if (IsValid())
		return m_xmlnodelist->Getlength();
	else
		return 0;
	
}

inline BOOL CXmlNodeList::IsValid()
{
	if (m_xmlnodelist == NULL)
		return FALSE;
	if (m_xmlnodelist.GetInterfacePtr() == NULL)
		return FALSE;
	return TRUE;
}

inline MSXML2::IXMLDOMNode* CXmlNodeList::Next()
{
	if (IsValid())
		return m_xmlnodelist->nextNode().Detach();
	else
		return NULL;
}

inline void CXmlNodeList::Start()
{
	if (IsValid())
		m_xmlnodelist->reset();
}

inline MSXML2::IXMLDOMNode* CXmlNodeList::Node(int index)
{
	if (IsValid())
		return m_xmlnodelist->Getitem(index).Detach();
	else
		return NULL;
}

inline MSXML2::IXMLDOMDocument* CXmlNodeList::AsDocument()
{
	if (IsValid())
	{
		CXmlDocument doc;
		doc.LoadXML(_T("<NodeList></NodeList>"));
		CXmlNode root(doc.AsNode());
		
		for (int i = 0; i < m_xmlnodelist->Getlength(); i++)
		{
			root.InsertNode(root.NumNodes(),m_xmlnodelist->Getitem(i)->cloneNode(VARIANT_TRUE));
		}
		return doc.Detach();
	}
	else 
		return NULL;
}

inline MSXML2::IXMLDOMNodeList* CXmlNode::FindNodes(LPCTSTR searchStr)
{
	if(IsValid())
	{
		try{
			return m_xmlnode->selectNodes(searchStr).Detach();
		}
		catch (_com_error e)
		{
			xstring err = e.ErrorMessage();
			return NULL;
		}
	}
	else
		return NULL;
}

inline xstring CXmlNode::Name()
{
	if (IsValid())
		return (LPCTSTR)m_xmlnode->GetbaseName();
	return _T("");
}

inline MSXML2::IXMLDOMNode* CXmlNode::InsertAfter(MSXML2::IXMLDOMNode *refNode, MSXML2::IXMLDOMNode *pNode)
{
	MSXML2::IXMLDOMNodePtr nextNode = refNode->GetnextSibling();
	MSXML2::IXMLDOMNodePtr newNode;
	if (nextNode.GetInterfacePtr() != NULL)
		newNode = m_xmlnode->insertBefore(pNode,(IUnknown*)nextNode);
	else
		newNode = m_xmlnode->appendChild(pNode);
	return newNode.Detach();
}

inline void CXmlNode::SetText(LPCTSTR text)
{
	if (IsValid())
		m_xmlnode->Puttext(text);
}

inline xstring CXmlNode::GetText()
{
	if (IsValid())
		return (LPCTSTR)m_xmlnode->Gettext();
	else
		return _T("");
}

inline void CXmlNode::ReplaceNode(MSXML2::IXMLDOMNode *pOldNode, MSXML2::IXMLDOMNode *pNewNode)
{
	if (IsValid())
	{
		m_xmlnode->replaceChild(pNewNode,pOldNode);
	}
}

inline int CXmlNode::NumAttributes()
{
	if (IsValid())
	{
		MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
		if (attribs)
			return attribs->Getlength();
	}

	return 0;
}

inline xstring CXmlNode::GetAttribName(int index)
{
	if (IsValid())
	{
		MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
		if (attribs)
		{
			MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(index);
			if (attrib)
				return (LPCTSTR)attrib->Getname();
		}
	}

	return _T("");
}

inline xstring CXmlNode::GetAttribVal(int index)
{
	if (IsValid())
	{
		MSXML2::IXMLDOMNamedNodeMapPtr attribs = m_xmlnode->Getattributes();
		if (attribs)
		{
			MSXML2::IXMLDOMAttributePtr attrib = attribs->Getitem(index);
			if (attrib)
				return (LPCTSTR)attrib->Gettext();
		}
	}

	
	return _T("");
}

inline xstring CXmlNode::NodeType()
{
	if (IsValid())
		return (LPCTSTR)m_xmlnode->GetnodeTypeString();
	return _T("");
}





//**How to use it


/***********************************************

void CXMLViewDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	CXmlNode m_xmlDoc;
	CFileDialog fDlg(TRUE,_T("xml"),NULL,OFN_HIDEREADONLY,_T("Xml Files (*.xml)|*.xml||"));
	if (fDlg.DoModal() == IDOK)
	{
		m_xmlDoc.Load(fDlg.GetPathName());
		ParseNode(m_xmlDoc.AsNode(),TVI_ROOT);
	}
}

void CXMLViewDlg::ParseNode(IDispatch *pNode,HTREEITEM hParent)
{
	CXmlNode node(pNode);
	int img,imgSel;
	std::wstring str;
	if (node.NodeType() == _T("element"))
	{
		img = 0;
		imgSel = 1;
		str = node.Name();
	}
	else
	{
		img = imgSel = 2;
		str = node.GetText();
	}

	HTREEITEM hItem = m_tree.InsertItem(str.c_str(),img,imgSel,hParent);
	m_tree.SetItemData(hItem,(DWORD)pNode);

	for (int i = 0; i < node.NumAttributes(); i++)
	{
		xstring xx = node.GetAttribName(i) + _T(" = ") + node.GetAttribVal(i);
		HTREEITEM hAttribItem = m_tree.InsertItem(xx.c_str(),3,4,hItem);
	}

	for (int i = 0; i < node.NumNodes(); i++)
	{
		ParseNode(node.GetNode(i),hItem);
	}
}
*****************************************************************************************************/