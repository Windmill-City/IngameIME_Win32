#pragma once
#include "tf_common.h"
class Document
{
public:
	ITfDocumentMgr* docMgr;
	Common* common;

	TF_API Document(Common* app);
	TF_API ~Document();
	TF_API VOID setFocus();
private:
};
