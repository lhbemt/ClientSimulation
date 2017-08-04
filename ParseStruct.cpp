#include "ParseStruct.h"
#include <sstream>

const int MaxBuffLen = 2044;
ParseStruct::~ParseStruct()
{
	if (!m_mapMessage.empty())
	{
		for (auto message : m_mapMessage)
		{
			delete[] message.second.pData;
		}
	}
	m_mapMessage.clear();
}

bool ParseStruct::InitXml()
{
	TiXmlElement* root = m_xmlDoc.RootElement();
	if (root == nullptr)
		return false;
	std::string strStructName = "";
	int nMainID;
	int nAssistantID;
	for (TiXmlElement* item = root->FirstChildElement("struct"); item; item = item->NextSiblingElement("struct"))
	{
		char* pBuff = new char[MaxBuffLen];
		memset(pBuff, 0, MaxBuffLen);
		int nTotalLen = 0;
		int nLen = 0;
        MessageID structField;
		TiXmlAttribute* attr = item->FirstAttribute(); // 第一个属性为结构体名
		strStructName = attr->Value();
		attr = attr->Next();
		nMainID = attr->IntValue(); // MainID
		attr = attr->Next();
		nAssistantID = attr->IntValue(); // AssistID
		structField.nMainID = nMainID;
		structField.nAssistID = nAssistantID;
		for (TiXmlElement* itemField = item->FirstChildElement("field"); itemField; itemField = itemField->NextSiblingElement("field"))
		{
			std::string fieldName = itemField->Attribute("name");
			std::string fieldType = itemField->Attribute("type");
			std::string filedValue = itemField->Attribute("value");
			// 根据 filedType 获取值
			ParseBody(fieldType, filedValue, pBuff + nTotalLen, nLen);
			nTotalLen += nLen;
		}
		struct MessageBody body;
		body.pData = pBuff;
		body.nLen = nTotalLen;
		m_mapMessage.insert(std::make_pair(structField, body));
	}
	return true;
}

void ParseStruct::ParseBody(std::string& filedType, std::string& filedValue, char* pData, int& nLen)
{
	nLen = 0;
	std::stringstream strStream;
	strStream << filedValue;
	if (filedType == "int")
	{
		int nValue;
		strStream >> nValue;
		memcpy(pData, &nValue, sizeof(nValue));
		nLen = sizeof(nValue);
	}
	else if (filedType == "char")
	{
		memcpy(pData, &filedValue[0], 1);
		nLen = 1;
	}
	else if (filedType.find_first_of('[') != std::string::npos)
	{

		int lLen = filedType.find_first_of('[');
		int rLen = filedType.find_first_of(']');
		std::string nLenStr = filedType.substr(lLen + 1, rLen);
		std::stringstream iString;
		iString << nLenStr;
		int nLenNum = 0;
		iString >> nLenNum;
		memcpy(pData, filedValue.c_str(), nLenNum);
		nLen = nLenNum;
	}
	else if (filedType == "bool")
	{
		bool b;
		strStream >> std::boolalpha >> b;
		memcpy(pData, &b, sizeof(b));
		nLen = sizeof(b);
	}
	else if (filedType == "int64")
	{
		__int64 i64Value;
		strStream >> i64Value;
		memcpy(pData, &i64Value, sizeof(__int64));
		nLen = sizeof(__int64);
	}
	else if (filedType == "uint")
	{
		unsigned int uInt8;
		strStream >> uInt8;
		memcpy(pData, &uInt8, sizeof(uInt8));
		nLen = sizeof(unsigned int);
	}
	else if (filedType == "uint64")
	{
		unsigned __int64 uInt64;
		strStream >> uInt64;
		memcpy(pData, &uInt64, sizeof(uInt64));
		nLen = sizeof(unsigned __int64);
	}
	else if (filedType == "float")
	{
		float fValue;
		strStream >> fValue;
		memcpy(pData, &fValue, sizeof(fValue));
		nLen = sizeof(fValue);
	}
	else if (filedType == "double")
	{
		double dbValue;
		strStream >> dbValue;
		memcpy(pData, &dbValue, sizeof(dbValue));
		nLen = sizeof(dbValue);
	}
}

bool ParseStruct::GetMessageBody(int nMaidID, int nAssistID, void* pData, int& nLen) // 获得数据
{
	struct MessageID message;
	message.nMainID = nMaidID;
	message.nAssistID = nAssistID;
	auto iter = m_mapMessage.find(message);
	if (iter != m_mapMessage.end())
	{
		memcpy(pData, iter->second.pData, iter->second.nLen);
		nLen = iter->second.nLen;
		return true;
	}
	else
		return false;
}
