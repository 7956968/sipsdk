#include "QueryDeviceStatusMsg.hpp"
#include "SipInterface.h"
#include <fstream>

bool CQueryDeviceStatusMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Query>\r\n";
	content<<"<CmdType>DeviceStatus</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";
	content<<"</Query>\r\n";
	message = content.str();
	return true;
}

bool CQueryDeviceStatusMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	size_t size = nodes.size();
	for( size_t i = 0; i < size; ++i )
	{
	}

	return true;
}

void CQueryDeviceStatusMsg::Process( CSipInterface *pInterface )
{
    if( NULL != pInterface )
    {
		pInterface->OnQueryDeviceStatus( *this );
    }
}

//////////////////////////////////////////////////////////////////////////
bool CQueryDeviceStatusResponseMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Response>\r\n";
	content<<"<CmdType>DeviceStatus</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";

	if( IsOK() )
	{
		content<<"<Result>OK</Result>\r\n";
	}
	else
	{
		content<<"<Result>ERROR</Result>\r\n";
	}

    if( !m_online.empty() )
	{
	    content << "<Online>" << m_online << "</Online>\r\n";
	}

	if( !m_status.empty() )
	{
	    content << "<Status>" << m_status << "</Status>\r\n";
	}

	if( !m_reason.empty() )
	{
        content << "<Reason>" << m_reason << "</Reason>\r\n";
	}

	if( !m_encode.empty() )
	{
		content << "<Encode>" << m_encode << "</Encode>\r\n";
	}

	if( !m_record.empty() )
	{
		content << "<Record>" << m_record  << "</Record>\r\n";
	}

	if( !m_deivcetime.empty() )
	{
		content << "<DeviceTime>" << m_deivcetime << "</DeviceTime>\r\n";
	}

	if( !m_items.empty() )
	{
		content<<"<Alarmstatus Num=" << m_items.size() << ">\r\n";
		std::list< SAlarmStatus >::iterator it = m_items.begin();
		std::list< SAlarmStatus >::iterator end = m_items.end();
		for( /*it*/; it != end; ++it )
		{
			const SAlarmStatus &alarm = *it;
			content<<"<Item>\r\n";
			content<<"<DeviceID>"<< alarm.id << "</DeviceID>\r\n";
			content<<"<DutyStatus>"<< alarm.status << "</DutyStatus>\r\n";
			content<<"</Item>\r\n";
		}
		content<<"</Alarmstatus>\r\n";
	}

	content<<"</Response>\r\n";
	message = content.str();
    return true;
}

bool CQueryDeviceStatusResponseMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pResult = NULL;
	XMLNode *pOnline = NULL;
	XMLNode *pStatus = NULL;
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

		if( CGBMessage::CompareNoCase( type, "Result" ) )
		{
			pResult = pNode;
		}
		else if( CGBMessage::CompareNoCase( type, "Online" ) )
		{
			pOnline = pNode;
		}
		else if( CGBMessage::CompareNoCase( type, "Status" ) )
		{
			pStatus = pNode;
		}
	}

	// ��ѡ����������
	if( pResult == NULL || pOnline == NULL || pStatus == NULL )
	{
        ERROR_LOG( "����Result,Online,Statusû�б�����" );
        reason = "����Result,Online,Statusû�б�����!";
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

		const char *type = pNode->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		XMLNode *pChild = pNode->FirstChild();
		if( pChild == NULL )
		{
			continue;
		}

		const char *value = pChild->Value();
		if( value == NULL )
		{
			continue;
		}

		if( CGBMessage::CompareNoCase( type, "Result" ) )
		{
			if( CGBMessage::CompareNoCase( value, "OK" ) )
			{
				m_result = CGBMessage::RESULT_OK;
			}
			else if( CGBMessage::CompareNoCase( value, "Error" ) )
			{
				m_result = CGBMessage::RESULT_ERROR;
			}
			else
			{
                ERROR_LOG( "����ֵ\'" << value << "\'��Ч" );
                reason = "����Resultֵ��Ч!";
				return false;
			}
		}
		else if( CGBMessage::CompareNoCase( type, "Online" ) )
		{
			m_online = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Status" ) )
		{
			m_status = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Reason" ) )
		{
			m_reason = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Encode" ) )
		{
			m_encode = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Record" ) )
		{
			m_record = value;
		}
		else if( CGBMessage::CompareNoCase( type, "DeviceTime" ) )
		{
			m_deivcetime = value;
		}
		else if( CGBMessage::CompareNoCase( type, "Alarmstatus" ) )
		{
			for( XMLNode *pItem = pNode->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
			{
				const char *type = pItem->Value();
				if( type == NULL )
				{
					DEBUG_LOG( "����ֵû������" );
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
			DEBUG_LOG( "����δ����Ĳ���:" << type );
			continue;
		}
	}

	// ����״̬��
	std::list< XMLNode* >::iterator it = items.begin();
	std::list< XMLNode* >::iterator end = items.end();
	for( /*it*/; it != end; ++it )
	{
		SAlarmStatus alarm;
		for( XMLNode *pItem = (*it)->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
		{
			XMLNode *pValue = pItem->FirstChild();
			if( pValue == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}

			const char *type = pItem->Value();
			if( type == NULL )
			{
				DEBUG_LOG( "��������Ϊ��" );
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
				alarm.id = value;
			}
			else if( CGBMessage::CompareNoCase( type, "DutyStatus" ) )
			{
				alarm.status = value;
			}		
			else
			{
				DEBUG_LOG( "����δ����Ĳ���:" << type );
				continue;
			}
		}

		// add to devices
		m_items.push_back( alarm );
	}
	return true;
}

void CQueryDeviceStatusResponseMsg::Process( CSipInterface *pInterface )
{
    if( pInterface != NULL )
	{
		pInterface->OnQueryDeviceStatusResponse( *this );
	}
}
