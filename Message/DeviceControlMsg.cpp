#include "DeviceControlMsg.hpp"
#include "SipInterface.h"

//////////////////////////////////////////////////////////////////////////
bool CPTZControlMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content << "<?xml version=\"1.0\"?>\r\n";
	content << "<Control>\r\n";
	content << "<CmdType>DeviceControl</CmdType>\r\n";
	content << "<SN>" << m_sn << "</SN>\r\n";
	content << "<DeviceID>" << m_deviceid << "</DeviceID>\r\n";
	content << "<PTZCmd>" << m_ptzCmd << "</PTZCmd>\r\n";
	if( !m_priority.empty() )
	{
		content << "<Info>\r\n";
		content << "<ControlPriority>" << m_priority << "</ControlPriority>\r\n";
		content << "</Info>\r\n";
	}

	content << "</Control>\r\n";
	message = content.str();
	return true;
}

bool CPTZControlMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pCmd = NULL;
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

		if( CGBMessage::CompareNoCase( type, "PTZCmd" ) )
		{
			pCmd = pNode;
		}
	}

	// ��ѡ����������
	if( pCmd == NULL )
	{
        ERROR_LOG( "����PTZCmdû�б�����" );
        reason = "����PTZCmdû�б�����!";
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

		XMLNode *pChild = pNode->FirstChild();
		if( pChild == NULL )
		{
			DEBUG_LOG( "����ֵû������" );
			continue;
		}

		const char* type = pNode->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		if( CGBMessage::CompareNoCase( type, "PTZCmd" ) )
		{
			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}
			else
			{
				m_ptzCmd = value;
			}
		}
		else if( CGBMessage::CompareNoCase( type, "Info" ) )
		{
			for( XMLNode *pItem = pNode->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
			{
				const char *value = pItem->Value();
				if( value != NULL && CGBMessage::CompareNoCase( value, "ControlPriority" ) )
				{
					items.push_back( pItem );
				}
				else
				{
					DEBUG_LOG( "����δ����Ĳ���:" << value );
				}
			}
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
		}
	}

	// ControlPriority
	std::list< XMLNode* >::iterator it = items.begin();
	std::list< XMLNode* >::iterator end = items.end();
	for( /*it*/; it != end; ++it )
	{
		XMLNode *pChild = (*it)->FirstChild();
		if( pChild != NULL )
		{
			m_priority = pChild->Value();
		}
	}

	return true;
}

void CPTZControlMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnPTZControl( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CPTZLockMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content << "<?xml version=\"1.0\"?>\r\n";
	content << "<Control>\r\n";
	content << "<CmdType>DeviceControl</CmdType>\r\n";
	content << "<SN>" << m_sn << "</SN>\r\n";
	content << "<DeviceID>" << m_deviceid << "</DeviceID>\r\n";
	content << "<ControlLock>" << m_lock << "</ControlLock>\r\n";
	content << "<Period>" << m_period << "</Period>\r\n";
	content << "</Control>\r\n";
	message = content.str();
	return true;
}

bool CPTZLockMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pLock = NULL;
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

		if( CGBMessage::CompareNoCase( type, "ControlLock" ) )
		{
			pLock = pNode;
		}
	}

	// ��ѡ����������
	if( pLock == NULL )
	{
        ERROR_LOG( "����ControlLockû�б�����" );
        reason = "����ControlLockû�б�����!";
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

		XMLNode *pChild = pNode->FirstChild();
		if( pChild == NULL )
		{
			DEBUG_LOG( "����ֵû������" );
			continue;
		}

		const char* type = pNode->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		if( CGBMessage::CompareNoCase( type, "ControlLock" ) )
		{
			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}
			else
			{
				m_lock = value;
			}
		}
		else if( CGBMessage::CompareNoCase( type, "Period" ) )
		{
			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}
			else
			{
				m_period = value;
			}
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
		}
	}

	return true;
}

void CPTZLockMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnPTZLock( *this );
	}
}


//////////////////////////////////////////////////////////////////////////
bool CRecordControlMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Control>\r\n";
	content<<"<CmdType>DeviceControl</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";

	if( m_record )
	{
		content<<"<RecordCmd>Record</RecordCmd>\r\n";
	}
	else
	{
		content<<"<RecordCmd>StopRecord</RecordCmd>\r\n";
	}

	content<<"</Control>\r\n";
	message = content.str();
	return true;
}

bool CRecordControlMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pRecord = NULL;
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

		if( CGBMessage::CompareNoCase( type, "RecordCmd" ) )
		{
			pRecord = pNode;
		}
	}

	// ��ѡ����������
	if( pRecord == NULL )
	{
        ERROR_LOG( "����RecordCmdû�б�����" );
        reason = "����RecordCmdû�б�����!";
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

		if( CGBMessage::CompareNoCase( type, "RecordCmd" ) )
		{
			XMLNode *pChild = pNode->FirstChild();
			if( pChild == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}

			const char* value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}

			if( CGBMessage::CompareNoCase( value, "Record" ) )
			{
				m_record = true;
			}
			else if( CGBMessage::CompareNoCase( value, "StopRecord" ) )
			{
				m_record = false;
			}
			else
			{
				ERROR_LOG( "RecordCmd����ֵ\'" << value << "\'��Ч" );
				return false;
			}
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
		}
	}

	return true;
}

void CRecordControlMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRecordControl( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTeleBootMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Control>\r\n";
	content<<"<CmdType>DeviceControl</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";
	content<<"<TeleBoot>" << m_cmd << "</TeleBoot>\r\n";
	content<<"</Control>\r\n";
	message = content.str();
	return true;
}

bool CTeleBootMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pTeleBoot = NULL;
	size_t size = nodes.size();
	for( size_t i = 0; i < size; ++i )
	{
		XMLNode *pNode = nodes[i];
		if( pNode == NULL )
		{
			DEBUG_LOG( "��������" );
			continue;
		}

		const char *value = pNode->Value();
		if( value == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		if( CGBMessage::CompareNoCase( value, "TeleBoot" ) )
		{
			pTeleBoot = pNode;
		}
	}

	// ��ѡ����������
	if( pTeleBoot == NULL )
	{
        ERROR_LOG( "����TeleBootû�б�����" );
        reason = "����TeleBootû�б�����!";
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

		XMLNode *pChild = pNode->FirstChild();
		if( pChild == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		if( CGBMessage::CompareNoCase( pNode->Value(), "TeleBoot" ) )
		{
			const char* value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "TeleBoot����ֵû������" );
				continue;
			}

			if( CGBMessage::CompareNoCase( value, "Boot" ) )
			{
				m_cmd = value;
			}
			else
			{
				ERROR_LOG( "TeleBoot����ֵ\'" << value << "\'��Ч" );
				return false;
			}
		}
		else
		{
			continue;
		}
	}

	return true;
}

void CTeleBootMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnTeleBoot( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CResetAlarmMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Control>\r\n";
	content<<"<CmdType>DeviceControl</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";
	content<<"<AlarmCmd>" << m_cmd << "</AlarmCmd>\r\n";
	content<<"<Info>\r\n";
	content<<"<AlarmMethod>" << m_method << "</AlarmMethod>\r\n";
	content<<"</Info>\r\n";
	content<<"</Control>\r\n";
	message = content.str();
	return true;
}

bool CResetAlarmMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pAlarmCmd = NULL;
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

		if( CGBMessage::CompareNoCase( type, "AlarmCmd" ) )
		{
			pAlarmCmd = pNode;
		}
	}

	// ��ѡ����������
	if( pAlarmCmd == NULL )
	{
        ERROR_LOG( "����AlarmCmdû�б�����" );
        reason = "����AlarmCmdû�б�����!";
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

		if( CGBMessage::CompareNoCase( type, "AlarmCmd" ) )
		{
			XMLNode *pChild = pNode->FirstChild();
			if( pChild == NULL )
			{
				DEBUG_LOG( "��������" );
				continue;
			}

			const char* value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "��������Ϊ��" );
				continue;
			}

			if( CGBMessage::CompareNoCase( value, "ResetAlarm" ) )
			{
				m_cmd = value;
			}
			else
			{
				DEBUG_LOG( "����δ����Ĳ���:" << value );
				continue;
			}
		}
		else if( CGBMessage::CompareNoCase( type, "Info" ) )
		{
			for( XMLNode *pItem = pNode->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
			{
				const char* value = pItem->Value();
				if( value == NULL )
				{
					DEBUG_LOG( "����ֵû������" );
					continue;
				}

				if( CGBMessage::CompareNoCase( value, "AlarmMethod" ) )
				{
					items.push_back( pItem );
				}				
			}
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
			continue;
		}
	}

	// AlarmMethod
	std::list< XMLNode* >::iterator it = items.begin();
	std::list< XMLNode* >::iterator end = items.end();
	for( /*it*/; it != end; ++it )
	{
		XMLNode *pChild = (*it)->FirstChild();
		if( pChild == NULL )
		{
			DEBUG_LOG( "����ֵû������" );
			continue;
		}
		else
		{
			const char *value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}
			else
			{
				m_method = atoi( pChild->Value() );
			}
		}
	}

	return true;
}

void CResetAlarmMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnResetAlarm( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CGuardMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Control>\r\n";
	content<<"<CmdType>DeviceControl</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";

	if( m_bSetGuard )
	{
		content<<"<GuardCmd>SetGuard</GuardCmd>\r\n";
	}
	else
	{
		content<<"<GuardCmd>ResetGuard</GuardCmd>\r\n";
	}

	content<<"</Control>\r\n";
	message = content.str();
	return true;
}

bool CGuardMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pGuardCmd = NULL;
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

		if( CGBMessage::CompareNoCase( type, "GuardCmd" ) )
		{
			pGuardCmd = pNode;
		}
	}

	// ��ѡ����������
	if( pGuardCmd == NULL )
	{
        ERROR_LOG( "����GuardCmdû�б�����" );
        reason = "����GuardCmdû�б�����!";
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

		if( CGBMessage::CompareNoCase( type, "GuardCmd" ) )
		{
			XMLNode *pChild = pNode->FirstChild();
			if( pChild == NULL )
			{
				DEBUG_LOG( "����ֵû������" );
				continue;
			}

			const char* value = pChild->Value();
			if( value == NULL )
			{
				DEBUG_LOG( "����ֵΪ��" );
				continue;
			}

			if( CGBMessage::CompareNoCase( value, "SetGuard" ) )
			{
				m_bSetGuard = true;
			}
			else if( CGBMessage::CompareNoCase( value, "ResetGuard" ) )
			{
				m_bSetGuard = false;
			}
			else
			{
				ERROR_LOG( "����ֵ\'" << value << "\'��Ч" );
				return false;
			}
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
			continue;
		}
	}

	return true;
}

void CGuardMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnGuard( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CDragZoomInMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Control>\r\n";
	content<<"<CmdType>DeviceControl</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";
	content<<"<DragZoomIn>\r\n";
	content<<"<Length>" << m_length << "</Length>\r\n";
	content<<"<Width>" << m_width << "</Width>\r\n";
	content<<"<MidPointX>" << m_midPointX << "</MidPointX>\r\n";
	content<<"<MidPointY>" << m_midPointY << "</MidPointY>\r\n";
	content<<"<LengthX>" << m_lengthX << "</LengthX>\r\n";
	content<<"<LengthY>" << m_lengthY << "</LengthY>\r\n";
	content<<"</DragZoomIn>\r\n";
	content<<"</Control>\r\n";
	message = content.str();
	return true;
}

bool CDragZoomInMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pDragZoom = NULL;
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

		if( CGBMessage::CompareNoCase( type, "DragZoomIn" ) )
		{
			pDragZoom = pNode;
		}
	}

	// ��ѡ����������
	if( pDragZoom == NULL )
	{
        ERROR_LOG( "����DragZoomInû�б�����" );
        reason = "����DragZoomInû�б�����!";
		return false;
	}

	// ��������
	for( XMLNode *pItem = pDragZoom->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
	{
		const char* type = pItem->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������" );
			continue;
		}

		XMLNode *pChild = pItem->FirstChild();
		if( pChild == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		const char *value = pChild->Value();
		if( value == NULL )
		{
			DEBUG_LOG( "����ֵû������" );
			continue;
		}

		if( CGBMessage::CompareNoCase( type, "Length" ) )
		{
			m_length = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "Width" ) )
		{
			m_width = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "MidPointX" ) )
		{
			m_midPointX = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "MidPointY" ) )
		{
			m_midPointY = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "LengthX" ) )
		{
			m_lengthX = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "LengthY" ) )
		{
			m_lengthY = atoi( value );
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
		}
	}

	return true;
}

void CDragZoomInMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnDragZoomIn( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CDragZoomOutMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Control>\r\n";
	content<<"<CmdType>DeviceControl</CmdType>\r\n";
	content<<"<SN>"<< m_sn <<"</SN>\r\n";
	content<<"<DeviceID>"<< m_deviceid <<"</DeviceID>\r\n";
	content<<"<DragZoomOut>\r\n";
	content<<"<Length>" << m_length << "</Length>\r\n";
	content<<"<Width>" << m_width << "</Width>\r\n";
	content<<"<MidPointX>" << m_midPointX << "</MidPointX>\r\n";
	content<<"<MidPointY>" << m_midPointY << "</MidPointY>\r\n";
	content<<"<LengthX>" << m_lengthX << "</LengthX>\r\n";
	content<<"<LengthY>" << m_lengthY << "</LengthY>\r\n";
	content<<"</DragZoomOut>\r\n";
	content<<"</Control>\r\n";
	message = content.str();
	return true;
}

bool CDragZoomOutMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
{
	// �������У��
	XMLNode *pDragZoom = NULL;
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

		if( CGBMessage::CompareNoCase( type, "DragZoomOut" ) )
		{
			pDragZoom = pNode;
		}
	}

	// ��ѡ����������
	if( pDragZoom == NULL )
	{
        ERROR_LOG( "����DragZoomOutû�б�����" );
        reason = "����DragZoomOutû�б�����!";
		return false;
	}

	// ��������
	for( XMLNode *pItem = pDragZoom->FirstChild(); pItem != NULL; pItem = pItem->NextSibling() )
	{
		const char* type = pItem->Value();
		if( type == NULL )
		{
			DEBUG_LOG( "��������Ϊ��" );
			continue;
		}

		XMLNode *pChild = pItem->FirstChild();
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

		if( CGBMessage::CompareNoCase( type, "Length" ) )
		{
			m_length = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "Width" ) )
		{
			m_width = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "MidPointX" ) )
		{
			m_midPointX = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "MidPointY" ) )
		{
			m_midPointY = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "LengthX" ) )
		{
			m_lengthX = atoi( value );
		}
		else if( CGBMessage::CompareNoCase( type, "LengthY" ) )
		{
			m_lengthY = atoi( value );
		}
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
			continue;
		}
	}

	return true;
}

void CDragZoomOutMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnDragZoomOut( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CDeviceControlResponseMsg::Encode( std::string &message )
{
	std::ostringstream content;
	content<<"<?xml version=\"1.0\"?>\r\n";
	content<<"<Response>\r\n";
	content<<"<CmdType>" << m_cmdtype << "</CmdType>\r\n";
	content<<"<SN>"<<GetSN()<<"</SN>\r\n";
	content<<"<DeviceID>"<<GetDeviceID()<<"</DeviceID>\r\n";

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

bool CDeviceControlResponseMsg::Decode( const std::vector< XMLNode* > &nodes, std::string &reason )
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
        ERROR_LOG( "����Resultû�б�����" );
        reason = "����Resultû�б�����!";
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
			DEBUG_LOG( "����ֵΪ��" );
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
		else
		{
			DEBUG_LOG( "����δ����Ĳ���:" << type );
			continue;
		}
	}

	return true;
}

void CDeviceControlResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnDeviceControlResponse( *this );
	}
}

