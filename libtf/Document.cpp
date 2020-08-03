#include "pch.h"
#include "Document.h"

Document::Document(Common* common)
{
	this->common = common;

	this->common->pThreadMgr->CreateDocumentMgr(&docMgr);
}

Document::~Document()
{
	if (docMgr) {
		docMgr->Release();
		docMgr = NULL;
	}
}

VOID Document::setFocus()
{
	common->pThreadMgr->SetFocus(docMgr);
}