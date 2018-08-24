#include "QueryRecordInfoMsg.hpp"
#include "SipInterface.h"

bool CQueryRecordInfoMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Query>\r\n";
	content<<"<CmdType>RecordInfo</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";

	if( !m_startTime.empty() )
	{
		content<<"<StartTime>"<< m_startTime <<"</StartTime>\r\n";
	}

	if( !m_endTime.empty() )
	{
		content<<"<EndTime>"<< m_endTime <<"</EndTime>\r\n";
	}

	if( !m_filePath.empty() )
	{
		content<<"<FilePath>"<< m_filePath <<"</FilePath>\r\n";
	}

	if( !m_address.empty() )
	{
		content<<"<Address>"<< m_address <<"</Address>\r\n";
	}

	if( !m_secrecy.empty() )
	{
		content<<"<Secrecy>"<< m_secrecy <<"</Secrecy>\r\n";
	}

	if( !m_type.empty() )
	{
		content<<"<Type>"<< m_type <<"</Type>\r\n";
	}

	if( !m_recorder.empty() )
	{
		content<<"<RecorderID>"<< m_recorder <<"</RecorderID>\r\n";
	}

	if( !m_indistinct.empty() )
	{
		content<<"<IndistinctQuery>"<< m_indistinct <<"</IndistinctQuery>\r\n";
	}

	content<<"</Query>\r\n";
	message = content.str();
	return true;
}

bool CQueryRecordInfoMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	size_t size = nodes.size();
	for( size_t i = 0; i < size; ++i )
	{
		XMLNode *pNode = nodes[i];
		if( pNode == NULL )
		{
			DEBUG_LOG( "��������" );
			continue;
		}

		const char *type = pNode->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		XMLNode *pChild = pNode->FirstChild();
		if( pChild == NULL )
		{
			DEBUG_LOG( "����ֵû������" );
			continue;
		}

		const char *value = pChild->Value();
		if( value == NULL )
		{
			DEBUG_LOG( "����ֵΪ��" );
			continue;
		}		

		if( CGBMessage::CompareNoCase( type, "StartTime" ) )
		{
			m_startTime = value;
		}
		else if( CGBMessage::CompareNoCase( type, "EndTime" ) )
		{
			m_endTime = value;
		}
		else if( CGBMessage::CompareNoCase( type, "FilePath" ) )
		{
			m_filePath = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Address" ) )
		{
			m_address = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Secrecy" ) )
		{
			m_secrecy = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Type" ) )
		{
			m_type = value;
		}
		else if( CGBMessage::CompareNoCase( type, "RecorderID" ) )
		{
			m_recorder = value;
		}
		else if( CGBMessage::CompareNoCase( type, "IndistinctQuery" ) )
		{
			m_indistinct = value;
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
			continue;
		}
	}

	return true;
}

void CQueryRecordInfoMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnQueryRecordInfo( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CQueryRecordInfoResponseMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Response>\r\n";
	content<<"<CmdType>RecordInfo</CmdType>\r\n";
	content<<"<SN>"<< m_sn << "</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";
	content<<"<Name>"<< m_name <<"</Name>\r\n";
	content<<"<SumNum>"<< m_sum <<"</SumNum>\r\n";

	size_t size = m_records.size();
	content<<"<RecordList Num=\"" << size << "\">\r\n";
	if( size > 0 )
	{
		for( std::list< SRecordInfo >::iterator it = m_records.begin(); it != m_records.end(); ++it )
		{
			const SRecordInfo &recordinfo = (*it);
			content<<"<Item>\r\n";
			content<<"<DeviceID>"<< recordinfo.id <<"</DeviceID>\r\n";
			content<<"<Name>"<< recordinfo.name <<"</Name>\r\n";

			if( !recordinfo.avPath.empty() )
			{
				content<<"<FilePath>"<< recordinfo.avPath <<"</FilePath>\r\n";
			}

			if( !recordinfo.address.empty() )
			{
				content<<"<Address>"<< recordinfo.avPath <<"</Address>\r\n";
			}

			if( !recordinfo.beginTime.empty() )
			{
				content<<"<StartTime>"<< recordinfo.beginTime <<"</StartTime>\r\n";
			}

			if( !recordinfo.endTime.empty() )
			{
				content<<"<EndTime>"<< recordinfo.endTime <<"</EndTime>\r\n";
			}

			content<<"<Secrecy>" << recordinfo.secrecy << "</Secrecy>\r\n";

			if( !recordinfo.type.empty() )
			{
				content<<"<Type>"<< recordinfo.type <<"</Type>\r\n";
			}

			if( !recordinfo.recorder.empty() )
			{
				content<<"<RecorderID>"<< recordinfo.recorder <<"</RecorderID>\r\n";
			}

			content<<"</Item>\r\n";
		}
	}

	content<<"</RecordList>\r\n";
	content<<"</Response>\r\n";

	message = content.str();
	return true;
}

bool CQueryRecordInfoResponseMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pSumNum = NULL;
	XMLNode *pName = NULL;
	size_t size = nodes.size();
	for( size_t i = 0; i < size; ++i )
	{
		XMLNode *pNode = nodes[i];
		if( pNode == NULL )
		{
			DEBUG_LOG( "��������" );
			continue;
		}

		const char* type = pNode->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		if( CGBMessage::CompareNoCase( type, "SumNum" ) )
		{
			pSumNum = pNode;
		}
		else if( CGBMessage::CompareNoCase( type, "Name" ) )
		{
			pName = pNode;
		}
	}

	// ��ѡ����������
	if( pSumNum == NULL || pName == NULL )
    {
        reason = "����SumNum,Nameû�б�����!";
		return false;
	}

	// ��������
	std::list< XMLNode* > items;
	for( size_t i = 0; i < size; ++i )
	{
		XMLNode *pNode = nodes[i];
		if( pNode == NULL )
		{
			DEBUG_LOG( "��������" );
			continue;
		}

		const char* type = pNode->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		if( CGBMessage::CompareNoCase( type, "SumNum" ) )
		{
			XMLNode *pChild = pNode->FirstChild();
			if( pChild == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}

			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}
			else
			{
				m_sum = atoi( value );
			}
		}
		else if( CGBMessage::CompareNoCase( type, "Name" ) )
		{
			XMLNode *pChild = pNode->FirstChild();
			if( pChild == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}

			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}
			else
			{
				m_name = value;
			}
		}
		else if( CGBMessage::CompareNoCase( type, "RecordList" ) )
		{
			for( XMLNode *pItem = pNode->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
			{
				const char *type = pItem->Value();
				if( type == NULL )
				{
					DEBUG_LOG( "��������Ϊ��" );
					continue;
				}

				if( CGBMessage::CompareNoCase( type, "Item" ) )
				{
					items.push_back( pItem );
				}
				else
				{
					DEBUG_LOG( "����δ����Ĳ���:" << type );
					continue;
				}
			}
		}
		else
		{
			continue;
		}
	}

	// Ŀ¼��
	std::list< XMLNode* >::iterator it = items.begin();
	std::list< XMLNode* >::iterator end = items.end();
	for( /*it*/; it != end; ++it )
	{
		SRecordInfo record;
		for( XMLNode *pItem = (*it)->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
		{
			const char *type = pItem->Value();
			if( type == NULL )
			{
				DEBUG_LOG( "��������Ϊ��" );
				continue;
			}

			XMLNode *pValue = pItem->FirstChild();
			if( pValue == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}

			const char *value = pValue->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}

			if( CGBMessage::CompareNoCase( type, "DeviceID" ) )
			{
				record.id = value;
			}
			else if( CGBMessage::CompareNoCase( type, "Name" ) )
			{
				record.name = value;
			}
			else if( CGBMessage::CompareNoCase( type, "FilePath" ) )
			{
				record.avPath = value;
			}
			else if( CGBMessage::CompareNoCase( type, "Address" ) )
			{
				record.address = value;
			}
			else if( CGBMessage::CompareNoCase( type, "StartTime" ) )
			{
				record.beginTime = value;
			}
			else if( CGBMessage::CompareNoCase( type, "EndTime" ) )
			{
				record.endTime = value;
			}
			else if( CGBMessage::CompareNoCase( type, "Secrecy" ) )
			{
				record.secrecy = value;
			}
			else if( CGBMessage::CompareNoCase( type, "Type" ) )
			{
				record.type = value;
			}
			else if( CGBMessage::CompareNoCase( type, "RecorderID" ) )
			{
				record.recorder = value;
			}
			else
			{
				DEBUG_LOG( "����δ����Ĳ���:" << type );
			}
		}

		m_records.push_back( record );
	}

	return true;
}

void CQueryRecordInfoResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnQueryRecordInfoResponse( *this );
	}
}

