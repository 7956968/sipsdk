#include "BroadcastMsg.hpp"
#include "SipInterface.h"

bool CBroadcastMsg::Encode( std::string &message )
{
	// check argument
	if( m_source.empty() || m_target.empty() )
	{
		ERROR_LOG( "SourceID��TargetID���뱻����" );
		return false;
	}

	// encode
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Notify>\r\n";
	content<<"<CmdType>Broadcast</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<SourceID>"<< m_source <<"</SourceID>\r\n";
	content<<"<TargetID>"<< m_target <<"</TargetID>\r\n";
	content<<"</Notify>\r\n";
	message = content.str();
	return true;
}

bool CBroadcastMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pSourceID = NULL;
	XMLNode *pTargetID = NULL;
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

		if( CGBMessage::CompareNoCase( type, "SourceID" ) )
		{
			pSourceID = pNode;
		}
		else if( CGBMessage::CompareNoCase( type, "TargetID" ) )
		{
			pTargetID = pNode;
		}
	}

	// ��ѡ����������
	if( pSourceID == NULL || pTargetID == NULL )
	{
        ERROR_LOG( "����SourceID��TargetID���뱻����" );
        reason = "����SourceID��TargetID���뱻����!";
		return false;
	}

	// ��������
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

		if( CGBMessage::CompareNoCase( type, "SourceID" ) )
		{
			m_source = value;
		}
		else if( CGBMessage::CompareNoCase( type, "TargetID" ) )
		{
			m_target = value;
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
		}
	}

	return true;
}

void CBroadcastMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnBroadcast( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBroadcastResponseMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Response>\r\n";
	content<<"<CmdType>Broadcast</CmdType>\r\n";
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

	content<<"</Response>\r\n";
	message = content.str();
	return true;
}

bool CBroadcastResponseMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pResult = NULL;
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

		if( CGBMessage::CompareNoCase( type, "Result" ) )
		{
			pResult = pNode;
		}
	}

	// ��ѡ����������
	if( pResult == NULL )
	{
        ERROR_LOG( "����Result���뱻����" );
        reason = "����Result���뱻����!";
		return false;
	}

	// ��������
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

		if( CGBMessage::CompareNoCase( type, "Result" ) )
		{
			XMLNode *pChild = pNode->FirstChild();
			if( pChild == NULL )
			{
				DEBUG_LOG( "��������" );
				continue;
			}

			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}

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
	}

	return true;
}

void CBroadcastResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnBroadcastResponse( *this );
	}
}

