#include "stdafx.h"
#include "msxml_wrap.h"

#ifndef _DEBUG
#define new DEBUG_NEW
#endif

// XmlNode
XmlNode::XmlNode(IXMLDOMNode *msxmlNode)
  : msxmlNode_(msxmlNode) {
}

// XmlElement
XmlElement::XmlElement(const XmlNode &node)
  : XmlNode(node) {
  IXMLElement *msxmlElm = NULL;
  if (!XmlNode::IsNull()) {
    msxmlNode_->QueryInterface(&msxmlElm_);
  }
}

XmlElement::XmlElement(IXMLDOMElement *msxmlElm)
  : XmlNode(msxmlElm)
  , msxmlElm_(msxmlElm) {
}

XmlElement &XmlElement::operator=(const XmlNode &node) {
  if (this == &node) {
    return *this;
  }
  Release();
  if (!XmlNode::IsNull()) {
    msxmlNode_->QueryInterface(&msxmlElm_);
  }
  return *this;
}

XmlElement XmlElement::FindFirstElement(LPCTSTR name, int length) {
  if (length == -1) {
    length = _tcslen(name);
  }
  for (XmlNode child = GetFirstChild(); !child.IsNull(); child = child.GetNextSibling()) {
    XmlElement elm = child;
    if (!elm.IsNull()) {
      CString tagName = elm.GetTagName();
      if (tagName.GetLength() == length && _tcsncmp(name, tagName, length) == 0) {
        return elm;
      }
    }
  }
  return XmlElement(NULL);
}

// XmlAttr
XmlAttr::XmlAttr()
  : isNull_(true) {
}

// XmlDocument
XmlDocument::XmlDocument() {
  HRESULT hr = msxmlDoc_.CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER);
  if (FAILED(hr)) {
    TRACE("ERROR: failed create CLSID_DOMDocument20(0x%08x)", hr);
  }
}

bool XmlDocument::Load(const CString &fileName) {
  ASSERT(msxmlDoc_ != NULL);
  
  VARIANT_BOOL successful = VARIANT_FALSE;

  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlDoc_->load(CComVariant(fileName), &successful), false);
  
  return true;
}

bool XmlDocument::Save(const CString &fileName) {
  ASSERT(msxmlDoc_ != NULL);
  
  IF_FAILED_SET_LAST_ERROR_RETURN(msxmlDoc_->save(CComVariant(fileName)), false);
  
  return true;
}