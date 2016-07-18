#pragma once

#include <MsXml2.h>

#define IF_FAILED_SET_LAST_ERROR_RETURN(expr, res) \
  do { \
    HRESULT hr = expr; \
    if (FAILED(hr)) { \
      ::SetLastError(hr); \
      return res; \
    } \
  } while (0)

// class XmlNode
class XmlNode {
 public:
  XmlNode(IXMLDOMNode *msxmlNode);

  bool IsNull() const;
  
  XmlNode GetFirstChild() const;
  XmlNode GetNextSibling() const;
  XmlNode GetParentNode() const;
  bool AppendChild(XmlNode &child);

  IXMLDOMNode *GetMSXMLNode() const;

 protected:
  CComPtr<IXMLDOMNode> msxmlNode_;
};
inline bool XmlNode::IsNull() const {
  return msxmlNode_ == NULL;
}
inline XmlNode XmlNode::GetFirstChild() const {
  ASSERT(!IsNull());
  IXMLDOMNode *msxmlNode = NULL;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlNode_->get_firstChild(&msxmlNode), XmlNode(NULL));
  return XmlNode(msxmlNode);
}
inline XmlNode XmlNode::GetNextSibling() const {
  ASSERT(!IsNull());
  IXMLDOMNode *msxmlNode = NULL;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlNode_->get_nextSibling(&msxmlNode), XmlNode(NULL));
  return XmlNode(msxmlNode);
}
inline XmlNode XmlNode::GetParentNode() const {
  ASSERT(!IsNull());
  IXMLDOMNode *msxmlNode = NULL;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlNode_->get_parentNode(&msxmlNode), XmlNode(NULL));
  return XmlNode(msxmlNode);
}
inline bool XmlNode::AppendChild(XmlNode &child) {
  ASSERT(!IsNull());
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlNode_->appendChild(child.msxmlNode_, NULL), false);
  return true;
}
inline IXMLDOMNode *XmlNode::GetMSXMLNode() const {
  return msxmlNode_;
}

// class XmlAttr
class XmlAttr {
 public:
  XmlAttr();
  
  void SetValue(LPCTSTR value);
  bool IsNull() const;
  bool IsEmpty() const;
  const CString &GetValue() const;  

 private:
  bool    isNull_;
  CString value_;
};
inline void XmlAttr::SetValue(LPCTSTR value) {
  isNull_ = false;
  value_ = value;
}
inline bool XmlAttr::IsNull() const {
  return isNull_;
}
inline bool XmlAttr::IsEmpty() const {
  return value_.IsEmpty();
}
inline const CString &XmlAttr::GetValue() const {
  return value_;
}

// class XmlElement
class XmlElement : public XmlNode {
 public:
  XmlElement(IXMLDOMElement *msxmlElm);
  XmlElement(const XmlNode &node);  
  bool IsNull() const;  
  XmlAttr GetAttribute(LPCTSTR name);
  bool GetAttribute(LPCTSTR name, CString *value);
  CString GetTagName() const;
  CString GetText() const;
  bool SetAttribute(LPCTSTR name, LPCTSTR value);
  IXMLDOMElement *GetMSXMLElement() const;
  XmlElement &operator=(const XmlNode &node);
  void Release();
  XmlElement FindFirstElement(LPCTSTR name, int length);

 private:
  CComPtr<IXMLDOMElement> msxmlElm_;
};
inline bool XmlElement::IsNull() const {
  return msxmlElm_ == NULL;
}
inline XmlAttr XmlElement::GetAttribute(LPCTSTR name) {
  ASSERT(!IsNull());
  XmlAttr res;
  CComVariant value;  
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlElm_->getAttribute(CComBSTR(name), &value), res);  
  if (value.vt != VT_BSTR) {
    ::SetLastError(ERROR_DATATYPE_MISMATCH);
    return res;
  }
  res.SetValue(value.bstrVal);
  return res;
}
inline bool XmlElement::GetAttribute(LPCTSTR name, CString *value) {
  ASSERT(!IsNull());  
  CComVariant var;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlElm_->getAttribute(CComBSTR(name), &var), false);  
  if (var.vt != VT_BSTR) {
    ::SetLastError(ERROR_DATATYPE_MISMATCH);
    return false;
  }  
  ASSERT(value != NULL);
  *value = var.bstrVal;  
  return true;
}
inline CString XmlElement::GetTagName() const {
  ASSERT(!IsNull());
  CComBSTR name;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlElm_->get_tagName(&name), CString());
  return CString(name);
}
inline CString XmlElement::GetText() const {
  ASSERT(!IsNull());
  CComBSTR text;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlElm_->get_text(&text), CString());
  return CString(text);
}
inline bool XmlElement::SetAttribute(LPCTSTR name, LPCTSTR value) {
  ASSERT(!IsNull());
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlElm_->setAttribute(CComBSTR(name), CComVariant(value)), false);
  return true;
}
inline IXMLDOMElement *XmlElement::GetMSXMLElement() const {
  return msxmlElm_;
}
inline void XmlElement::Release() {
  msxmlElm_.Release();
}

// class XmlDocument
class XmlDocument {
 public:
  XmlDocument();
  bool IsNull() const;
  bool Load(const CString &fileName);
  bool Save(const CString &fileName);
  XmlElement GetDocElement();
  XmlElement CreateElement(const CString &tagName);
  bool AppendChild(const XmlElement &elm);

 private:
  CComPtr<IXMLDOMDocument2> msxmlDoc_;
};
inline bool XmlDocument::IsNull() const {
  return msxmlDoc_ == NULL;
}
inline XmlElement XmlDocument::GetDocElement() {
  ASSERT(!IsNull());
  IXMLDOMElement *msxmlElm = NULL;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlDoc_->get_documentElement(&msxmlElm), XmlElement(NULL));
  return XmlElement(msxmlElm);
}
inline XmlElement XmlDocument::CreateElement(const CString &tagName) {
  ASSERT(!IsNull());
  IXMLDOMElement *msxmlElm = NULL;
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlDoc_->createElement(CComBSTR(tagName), &msxmlElm), XmlElement(NULL));
  return XmlElement(msxmlElm);
}
inline bool XmlDocument::AppendChild(const XmlElement &elm) {
  ASSERT(!IsNull());
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlDoc_->appendChild(elm.GetMSXMLElement(), NULL), false);
  return true;
}