#include "InviteMsg.hpp"
#include "SipInterface.h"
#include <sstream>

#ifndef NPLogError
#define NPLogError
#endif

//////////////////////////////////////////////////////////////////////////
bool CInviteMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
	// create sdp
	std::string body;
	if( !Encode( body ) )
	{
		ERROR_LOG( "��Ϣ������󣬷���ʧ��!" );
		return false;
	}

	__DUM_TRY
	resip::NameAddr uasAor;
	uasAor.uri().user() = m_to.c_str();
	uasAor.uri().host() = m_to.substr( 0, 10 ).c_str();

	resip::HeaderFieldValue hfv( body.c_str(), body.length() );
	resip::Mime mime_type( "APPLICATION", "SDP" );
	resip::PlainContents sdp( hfv, mime_type );
	resip::SharedPtr<resip::SipMessage> reqInvite = mDum.makeInviteSession( uasAor, &sdp );

	// ����ģʽ����˳����ForceTarget,route,RequestLine
	// ��Ϣû������ForceTarget��routeͷ�����Կ��Ը������������ô���ģʽ
	// request
	resip::Uri from;
	from.user() = m_from.c_str();
	from.host() = m_from.substr( 0, 10 ).c_str();
	reqInvite->header( resip::h_From ).uri() = from;

	// ������
	resip::Uri rl;
	rl.user() = m_request.c_str();
	rl.host() = m_ip.c_str();
	rl.port() = m_port;
	rl.param( resip::p_transport ) = "UDP";
	reqInvite->header( resip::h_RequestLine ).uri() = rl;

	// subject
	if( !m_subject.empty() )
	{
		reqInvite->header( resip::h_Subject ) = resip::StringCategory( resip::Data( m_subject.c_str() ) );
	}

	// store callid
	if( reqInvite->exists( resip::h_CallID ) )
	{
		m_callid = reqInvite->header( resip::h_CallID ).value().c_str();
	}

	mDum.sendCommand( reqInvite );
	__DUM_CATCH
	return true;
}

bool CInviteMsg::Encode( std::string &message )
{
	DEBUG_LOG( "��Ϣ������󣬷���ʧ��!" );
	return false;
}

bool CInviteMsg::Decode( const char *body, std::string &reason )
{
	DEBUG_LOG( "InviteMessage����δʵ��!" );
	return false;
}

void CInviteMsg::Process( CSipInterface *pInterface )
{
}

//////////////////////////////////////////////////////////////////////////
bool CVodDownloadMsg::Encode( std::string &message )
{
	// subject
	if( m_subject.empty() )
	{
		m_subject.clear();
        m_subject += m_sender.id;
        m_subject += ":";
        m_subject += m_sender.seq;
        m_subject += ",";
        m_subject += m_recver.id;
        m_subject += ":";
        m_subject += m_recver.seq;
	}

	// sdp
	std::stringstream context;
	context<<"v=0\r\n";
	context<<"o="<< m_recvid <<" 0 0 IN IP4 "<< m_mediaip <<"\r\n";
	context<<"s=Download\r\n";
	context<<"u=" << m_uri.id << ":" << m_uri.param << "\r\n";
	context<<"c=IN IP4 "<< m_connect <<"\r\n";
	context<<"t="<< m_starttime << " " << m_endtime << "\r\n";

	context<<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<" "<<key;
		}
	}
	context<<"\r\n";

	context<<"a=recvonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}

    if( !m_downloadspeed.empty() )
    {
        context<<"a=downloadspeed:"<<m_downloadspeed<<"\r\n";
    }

	if( !m_ssrc.empty() )
	{
		context<<"y="<< m_ssrc << "\r\n";
	}

	if( !m_audioproterty.empty() )
	{
		context<<"f="<< m_audioproterty << "\r\n";
	}

	message = context.str();
	return true;
}

bool CVodDownloadMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "¼������������Ϣ�岻��SDP!" );
#endif
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
			{
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
			{
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
		{
			if( *pContent++ != 's' || *pContent++ != '=' )
			{
				return false;
			}

			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
		}
		else if( ch == 'u' )
		{
			if( *pContent++ != 'u' || *pContent++ != '=' )
			{
				return false;
			}

			// uri id
			anchor = pContent;
			while( *pContent != ':' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_uri.id.assign( anchor, pContent );
				pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// uri param
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_uri.param.assign( anchor, pContent );
			}
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
			{
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
			{
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
			{
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
					{
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
			{
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
				{
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else if( CGBMessage::CompareNoCase( type, "downloadspeed" ) )
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					m_downloadspeed.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
			{
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
			{
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CVodDownloadMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnDownloadVodStream( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CVodStreamMsg::Encode( std::string &message )
{
	// subject
	if( m_subject.empty() )
	{
        m_subject.clear();
        m_subject += m_sender.id;
        m_subject += ":";
        m_subject += m_sender.seq;
        m_subject += ",";
        m_subject += m_recver.id;
        m_subject += ":";
        m_subject += m_recver.seq;
	}

	// sdp
	std::stringstream context;
	context<<"v=0\r\n";
	context<<"o="<< m_recvid <<" 0 0 IN IP4 "<< m_mediaip <<"\r\n";
	context<<"s=Playback\r\n";
	context<<"u=" << m_uri.id << ":" << m_uri.param << "\r\n";
	context<<"c=IN IP4 "<< m_connect <<"\r\n";
	context<<"t="<< m_starttime << " " << m_endtime << "\r\n";

	context<<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<" "<<key;
		}
	}
	context<<"\r\n";

	context<<"a=recvonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}


	if( !m_ssrc.empty() )
	{
		context<<"y="<< m_ssrc << "\r\n";
	}

	if( !m_audioproterty.empty() )
	{
		context<<"f=" << m_audioproterty << "\r\n";
	}

	message = context.str();
	return true;
}

bool CVodStreamMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "¼��ط�������Ϣ�岻��SDP!" );
#endif
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
		{
			if( *pContent++ != 's' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
		}
		else if( ch == 'u' )
		{
			if( *pContent++ != 'u' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			// uri id
			anchor = pContent;
			while( *pContent != ':' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_uri.id.assign( anchor, pContent );
				pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// uri param
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_uri.param.assign( anchor, pContent );
			}
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
					{
						DEBUG_LOG( ch << "�ֶ�����" );
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
				DEBUG_LOG( ch << "�ֶ�����" );
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
				DEBUG_LOG( ch << "�ֶ�����" );
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
				{
					DEBUG_LOG( ch << "�ֶ�����" );
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
			{
				DEBUG_LOG( ch << "�ֶ�����" );
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
				DEBUG_LOG( ch << "�ֶ�����" );
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CVodStreamMsg::Process( CSipInterface *pInterface )
{
    if( pInterface != NULL )
    {
        pInterface->OnOpenVodStream( *this );
    }
}

//////////////////////////////////////////////////////////////////////////
bool CRealStreamMsg::Encode( std::string &message )
{
	// subject
	if( m_subject.empty() )
	{
        m_subject.clear();
        m_subject += m_sender.id;
        m_subject += ":";
        m_subject += m_sender.seq;
        m_subject += ",";
        m_subject += m_recver.id;
        m_subject += ":";
        m_subject += m_recver.seq;
	}

	// sdp
	std::stringstream context;
	context<<"v=0\r\n";
	context<<"o="<< m_recvid <<" 0 0 IN IP4 "<< m_mediaip <<"\r\n";
	context<<"s=Play\r\n";
	context<<"c=IN IP4 "<< m_connect <<"\r\n";
	context<<"t=0 0\r\n";

	context<<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<" "<<key;
		}
	}
	context<<"\r\n";

	context<<"a=recvonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}

	if( !m_ssrc.empty() )
	{
		context<<"y="<< m_ssrc << "\r\n";
	}

	if( !m_audioproterty.empty() )
	{
		context<<"f=" << m_audioproterty << "\r\n";
	}

	message = context.str();
	return true;
}

bool CRealStreamMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "ʵʱ��������Ϣ�岻��SDP!" );
#endif
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
			{
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
			{
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
		{
			if( *pContent++ != 's' || *pContent++ != '=' )
			{
				return false;
			}

			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
			{
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
			{
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
			{
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
					{
						return false;
					}
				}
			}
        }
        else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
			{
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
				{
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
			{
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
			{
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
        {
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CRealStreamMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnOpenRealStream( *this );
	}
}
// #define NP_GB_SDK
//////////////////////////////////////////////////////////////////////////
bool CRealStreamToDecorderMsg::Encode( std::string &message )
{
	// subject
	if( m_subject.empty() )
	{
        m_subject.clear();
        m_subject += m_sender.id;
        m_subject += ":";
        m_subject += m_sender.seq;
        m_subject += ",";
        m_subject += m_recver.id;
        m_subject += ":";
        m_subject += m_recver.seq;
	}

	// sdp
    // ������������Invite��Ϣ��Я��sdp
#ifdef NP_GB_SDK
	std::stringstream context;
	context<<"v=0\r\n";
	context<<"o="<< m_recvid <<" 0 0 IN IP4 "<< m_mediaip <<"\r\n";
	context<<"s=Play\r\n";
	context<<"c=IN IP4 "<< m_connect <<"\r\n";
	context<<"t=0 0\r\n";

	context<<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<" "<<key;
		}
	}
	context<<"\r\n";

	context<<"a=sendonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}

	if( !m_ssrc.empty() )
	{
		context<<"y="<< m_ssrc << "\r\n";
	}

	if( !m_audioproterty.empty() )
	{
		context<<"f=" << m_audioproterty << "\r\n";
	}

	message = context.str();
#endif

    return true;
}

bool CRealStreamToDecorderMsg::Decode( const char *body, std::string &reason )
{
    // message without sdp
    if( body == NULL )
    {
#ifdef _DEBUG
        WARN_LOG( "ʵʱ��������Ϣ�岻��SDP!" );
#endif
        return true;
    }

    const char* pContent = body;
    const char* anchor = 0;

    // ����SDP�ֶ�
    while( pContent != '\0' )
    {
        char ch = *pContent;
        if( ch == 'v' )
        {
            if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
        else if( ch == 'o' )
        {
            if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                return false;
            }

            // id
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_recvid.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // session
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_sessionid.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // version
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_version.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // in
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // ip version
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // media ip
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_mediaip.assign( anchor, pContent );
            }
        }
        else if( ch == 's' )
        {
            if( *pContent++ != 's' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
        else if( ch == 'c' )
        {
            if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                return false;
            }

            // in
            while( *pContent != ' ' ) pContent++;
            if( pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // ip version
            while( *pContent != ' ' ) pContent++;
            if( pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // connect ip
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_connect.assign( anchor, pContent );
            }
        }
        else if( ch == 't' )
        {
            if( *pContent++ != 't' || *pContent++ != '=' )
            {
                return false;
            }

            // start time
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_starttime.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // end time
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_endtime.assign( anchor, pContent );
            }
        }
        else if( ch == 'm' )
        {
            if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                return false;
            }

            // media type
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_mediatype.assign(anchor, pContent);
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // port
            anchor = pContent;
            while( *pContent != ' ') pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_mediaport.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // transport
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_transport.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // rtpmap
            while( *pContent != '\r' && *pContent != '\n' )
            {
                anchor = pContent;
                while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
                else
                {
                    std::string rtpmap( anchor, pContent );
                    m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
                    while( *pContent == ' ' ) pContent++;
                    if( *pContent == '\0' )
                    {
                        return false;
                    }
                }
            }
        }
        else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
        else if( ch == 'a' )
        {
            if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                return false;
            }

            std::string type;
            anchor = pContent;
            while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                type.assign( anchor, pContent );
                while( *pContent == ' ' || *pContent == ':' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
            {
                anchor = pContent;
                while( *pContent != ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }

                std::string rtpmap( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }

                std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
                if( it == m_rtpmap.end() )
                {
                    return false;
                }

                anchor = pContent;
                while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
                {
                    pContent++;
                }

                if( anchor != pContent )
                {
                    it->second.assign( anchor, pContent );
                }
            }
            else
            {
                anchor = pContent;
                while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
                {
                    pContent++;
                }

                if( *pContent != '\0' )
                {
                    // send or recv
                }
            }
        }
        else if( ch == 'y' )
        {
            if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                return false;
            }

            anchor = pContent;
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
            {
                pContent++;
            }

            if( anchor != pContent )
            {
                m_ssrc.assign( anchor, pContent );
            }
        }
        else if( ch == 'f' )
        {
            if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                return false;
            }

            anchor = pContent;
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
            {
                pContent++;
            }

            if( anchor != pContent )
            {
                m_audioproterty.assign( anchor, pContent );
            }
        }
        else
        {
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }

        // next line
        while( *pContent == '\r' || *pContent == '\n' ) pContent++;
        if( *pContent == '\0' )
        {
            break;
        }
    }

    return true;
}

void CRealStreamToDecorderMsg::Process( CSipInterface *pInterface )
{
    if( pInterface != NULL )
    {
        pInterface->OnOpenRealStreamToDecorder( *this );
    }
}

//////////////////////////////////////////////////////////////////////////
bool CBroadcastStreamMsg::Encode( std::string &message )
{
	// subject
	if( m_subject.empty() )
	{
		m_subject.clear();
		m_subject += m_sender.id;
		m_subject += ":";
		m_subject += m_sender.seq;
		m_subject += ",";
		m_subject += m_recver.id;
		m_subject += ":";
		m_subject += m_recver.seq;
	}

	// sdp
	std::stringstream context;
	context<<"v=0\r\n";
	context<<"o="<< m_recvid <<" 0 0 IN IP4 "<< m_mediaip <<"\r\n";
	context<<"s=Play\r\n";
	context<<"c=IN IP4 "<< m_connect <<"\r\n";
	context<<"t=0 0\r\n";

	context<<"m=audio "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<" "<<key;
		}
	}
	context<<"\r\n";

	context<<"a=recvonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			context<<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}

	if( !m_ssrc.empty() )
	{
		context<<"y="<< m_ssrc << "\r\n";
	}

	if( !m_audioproterty.empty() )
	{
		context<<"f=" << m_audioproterty << "\r\n";
	}

	message = context.str();
	return true;
}

bool CBroadcastStreamMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "�����㲥������Ϣ�岻��SDP!" );
#endif
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
			{
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
			{
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
		{
			if( *pContent++ != 's' || *pContent++ != '=' )
			{
				return false;
			}

			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
		}
		else if( ch == 'u' )
		{
			if( *pContent++ != 'u' || *pContent++ != '=' )
			{
				return false;
			}

			// uri id
			anchor = pContent;
			while( *pContent != ':' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_uri.id.assign( anchor, pContent );
				pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// uri param
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_uri.param.assign( anchor, pContent );
			}
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
			{
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
			{
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
			{
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
			{
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
					{
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
			{
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
			{
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
				{
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
				{
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
			{
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
			{
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CBroadcastStreamMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnOpenBroadcastStream( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CInviteResponseMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
	// ��Ӧ�ؼ���������Ϣ�������Ϊ��
	resip::ServerInviteSession *handle = dynamic_cast< resip::ServerInviteSession* >( m_handle );
	if( handle == NULL )
	{
		ERROR_LOG( "������ϢhandleΪNULL, ����ʧ��!" );
		return false;
	}

	// ������Ϣ
	__DUM_TRY
	if( m_statusCode == 200 )
	{
		// ������Ϣ������
		std::string body;
		if( !Encode( body ) )
		{
			ERROR_LOG( "��Ϣ������󣬷���ʧ��!" );
			return false;
		}
		resip::HeaderFieldValue hfv( body.c_str(), (unsigned int)body.size() );
		resip::Mime mime_type( "Application", "sdp" );
		resip::PlainContents sdp( hfv, mime_type );
		handle->provideAnswer( sdp );
		handle->acceptCommand();
	}
	else
	{
		handle->rejectCommand( m_statusCode );
	}

	return true;
	__DUM_CATCH
}

//////////////////////////////////////////////////////////////////////////
bool CRealStreamResponseMsg::Encode( std::string &message )
{
	std::ostringstream sdp;
	sdp << "v=0\r\n";
	sdp << "o="<< m_recvid << " 0 0 IN IP4 "<< m_mediaip << "\r\n";
	sdp << "s=Play\r\n";
	sdp << "c=IN IP4 " << m_connect << "\r\n";
	sdp << "t=0 0\r\n";
	sdp <<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<" "<<key;
		}
	}
	sdp <<"\r\n";
	sdp << "a=sendonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}
	sdp << "y=" << m_ssrc << "\r\n";

	message = sdp.str();
	return true;
}

bool CRealStreamResponseMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "ʵʱ����Ӧ��Ϣ�岻��SDP!" );
#endif
        //reason = "ʵʱ����Ӧ��Ϣ�岻��SDP!";
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                reason = "û�ҵ�v�ֶ�!";
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "�ֶ�v��β���ֽ�����!";
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                reason = "û�ҵ�o�ֶ�!";
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�idǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�id����ֽ�����!";
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�sessionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�session����ֽ�����!";
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�version����ֽ�����!";
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
		{
			if( *pContent++ != 's' || *pContent++ != '=' )
            {
                reason = "û�ҵ�s�ֶ�!";
				return false;
			}

            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( anchor != pContent )
            {
                m_sessionname.assign( anchor, pContent );
            }

            if( *pContent == '\0' )
            {
                reason = "�ֶ�s��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                reason = "û�ҵ�c�ֶ�!";
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "c�ֶ�connect ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
            {
                reason = "û�ҵ�t�ֶ�!";
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�start timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "t�ֶ�start time����ֽ�����!";
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�end timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                reason = "û�ҵ�m�ֶ�!";
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�media typeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�media type����ֽ�����!";
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�portǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�port����ֽ�����!";
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�transportǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�transport����ֽ�����!";
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�rtpmapǰ���ֽ�����!";
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
                    {
                        reason = "m�ֶ�rtpmap����ֽ�����!";
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                reason = "û�ҵ�b�ֶ�!";
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "�ֶ�b��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                reason = "û�ҵ�a�ֶ�!";
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "a�ֶ�rtpmapǰ���ֽ�����!";
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�rtpmap����ֽ�����!";
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload typeǰ���ֽ�����!";
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload type����ֽ�����!";
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
                {
                    reason = "m�ֶβ����ڸ�payload type!";
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                reason = "û�ҵ�y�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                reason = "û�ҵ�f�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "��ʶ����ֶβ��ҳ��ֽ�����!";
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CRealStreamResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnInviteResponse( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBroadcastStreamResponseMsg::Encode( std::string &message )
{
	std::ostringstream sdp;
	sdp << "v=0\r\n";
	sdp << "o="<< m_recvid << " 0 0 IN IP4 "<< m_mediaip << "\r\n";
	sdp << "s=Play\r\n";
	sdp << "c=IN IP4 " << m_connect << "\r\n";
	sdp << "t=0 0\r\n";
	sdp <<"m=audio "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<" "<<key;
		}
	}
	sdp <<"\r\n";
	sdp << "a=sendonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}
	sdp << "y=" << m_ssrc << "\r\n";

	message = sdp.str();
	return true;
}

bool CBroadcastStreamResponseMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "�����㲥��Ӧ��Ϣ�岻��SDP!" );
#endif
        reason = "�����㲥��Ӧ��Ϣ�岻��SDP!";
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                reason = "û�ҵ�v�ֶ�!";
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "�ֶ�v��β���ֽ�����!";
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                reason = "û�ҵ�o�ֶ�!";
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�idǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�id����ֽ�����!";
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�sessionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�session����ֽ�����!";
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�version����ֽ�����!";
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
        {
            if( *pContent++ != 's' || *pContent++ != '=' )
            {
                reason = "û�ҵ�s�ֶ�!";
                return false;
            }

            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( anchor != pContent )
            {
                m_sessionname.assign( anchor, pContent );
            }

            if( *pContent == '\0' )
            {
                reason = "�ֶ�s��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                reason = "û�ҵ�c�ֶ�!";
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "c�ֶ�connect ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
            {
                reason = "û�ҵ�t�ֶ�!";
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�start timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "t�ֶ�start time����ֽ�����!";
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�end timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                reason = "û�ҵ�m�ֶ�!";
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�media typeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�media type����ֽ�����!";
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�portǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�port����ֽ�����!";
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�transportǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�transport����ֽ�����!";
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�rtpmapǰ���ֽ�����!";
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
                    {
                        reason = "m�ֶ�rtpmap����ֽ�����!";
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                reason = "û�ҵ�b�ֶ�!";
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "�ֶ�b��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                reason = "û�ҵ�a�ֶ�!";
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "a�ֶ�rtpmapǰ���ֽ�����!";
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�rtpmap����ֽ�����!";
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload typeǰ���ֽ�����!";
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload type����ֽ�����!";
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
                {
                    reason = "m�ֶβ����ڸ�payload type!";
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                reason = "û�ҵ�y�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                reason = "û�ҵ�f�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "��ʶ����ֶβ��ҳ��ֽ�����!";
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CBroadcastStreamResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnInviteResponse( *this );
	}
}



//////////////////////////////////////////////////////////////////////////
bool CVodStreamResponseMsg::Encode( std::string &message )
{
	std::ostringstream sdp;
	sdp << "v=0\r\n";
	sdp << "o="<< m_recvid << " 0 0 IN IP4 "<< m_mediaip << "\r\n";
	sdp << "s=Playback\r\n";
	sdp << "c=IN IP4 " << m_connect << "\r\n";
	sdp << "t=0 0\r\n";
	sdp <<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<" "<<key;
		}
	}
	sdp <<"\r\n";
	sdp << "a=sendonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}
	sdp << "y=" << m_ssrc << "\r\n";
	message = sdp.str();
	return true;
}

bool CVodStreamResponseMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "¼��طŻ�Ӧ��Ϣ�岻��SDP!" );
#endif
        //reason = "¼��طŻ�Ӧ��Ϣ�岻��SDP!";
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                reason = "û�ҵ�v�ֶ�!";
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "�ֶ�v��β���ֽ�����!";
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                reason = "û�ҵ�o�ֶ�!";
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�idǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�id����ֽ�����!";
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�sessionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�session����ֽ�����!";
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�version����ֽ�����!";
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
        {
            if( *pContent++ != 's' || *pContent++ != '=' )
            {
                reason = "û�ҵ�s�ֶ�!";
                return false;
            }

            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( anchor != pContent )
            {
                m_sessionname.assign( anchor, pContent );
            }

            if( *pContent == '\0' )
            {
                reason = "�ֶ�s��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                reason = "û�ҵ�c�ֶ�!";
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "c�ֶ�connect ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
            {
                reason = "û�ҵ�t�ֶ�!";
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�start timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "t�ֶ�start time����ֽ�����!";
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�end timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                reason = "û�ҵ�m�ֶ�!";
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�media typeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�media type����ֽ�����!";
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�portǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�port����ֽ�����!";
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�transportǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�transport����ֽ�����!";
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�rtpmapǰ���ֽ�����!";
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
                    {
                        reason = "m�ֶ�rtpmap����ֽ�����!";
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                reason = "û�ҵ�b�ֶ�!";
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "�ֶ�b��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                reason = "û�ҵ�a�ֶ�!";
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "a�ֶ�rtpmapǰ���ֽ�����!";
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�rtpmap����ֽ�����!";
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload typeǰ���ֽ�����!";
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload type����ֽ�����!";
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
                {
                    reason = "m�ֶβ����ڸ�payload type!";
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
			}
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                reason = "û�ҵ�y�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                reason = "û�ҵ�f�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "��ʶ����ֶβ��ҳ��ֽ�����!";
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CVodStreamResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnInviteResponse( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CVodDownloadResponseMsg::Encode( std::string &message )
{
	std::ostringstream sdp;
	sdp << "v=0\r\n";
	sdp << "o="<< m_recvid << " 0 0 IN IP4 "<< m_mediaip << "\r\n";
	sdp << "s=Download\r\n";
	sdp << "c=IN IP4 " << m_connect << "\r\n";
	sdp << "t=0 0\r\n";
	sdp <<"m=video "<< m_mediaport <<" RTP/AVP";
	std::map< std::string, std::string >::const_iterator it;
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<" "<<key;
		}
	}
	sdp <<"\r\n";
	sdp << "a=sendonly\r\n";
	for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
	{
		std::string key = it->first;
		std::string text = it->second;
		if ( !text.empty() )
		{
			sdp <<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
		}
	}
	sdp << "a=filesize:" << m_filesize << "\r\n";

	sdp << "y=" << m_ssrc << "\r\n";

	message = sdp.str();
	return true;
}

bool CVodDownloadResponseMsg::Decode( const char *body, std::string &reason )
{
	// message without sdp
	if( body == NULL )
	{
#ifdef _DEBUG
		WARN_LOG( "¼�����ػ�Ӧ��Ϣ�岻��SDP!" );
#endif
        reason = "¼�����ػ�Ӧ��Ϣ�岻��SDP!";
		return true;
	}

	const char* pContent = body;
	const char* anchor = 0;

	// ����SDP�ֶ�
	while( pContent != '\0' )
	{
		char ch = *pContent;
		if( ch == 'v' )
		{
			if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                reason = "û�ҵ�v�ֶ�!";
				return false;
			}

			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "�ֶ�v��β���ֽ�����!";
				return false;
			}
		}
		else if( ch == 'o' )
		{
			if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                reason = "û�ҵ�o�ֶ�!";
				return false;
			}

			// id
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�idǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_recvid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�id����ֽ�����!";
					return false;
				}
			}

			// session
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�sessionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_sessionid.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�session����ֽ�����!";
					return false;
				}
			}

			// version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_version.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�version����ֽ�����!";
					return false;
				}
			}

			// in
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "o�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// media ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "o�ֶ�ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaip.assign( anchor, pContent );
			}
		}
		else if( ch == 's' )
        {
            if( *pContent++ != 's' || *pContent++ != '=' )
            {
                reason = "û�ҵ�s�ֶ�!";
                return false;
            }

            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( anchor != pContent )
            {
                m_sessionname.assign( anchor, pContent );
            }

            if( *pContent == '\0' )
            {
                reason = "�ֶ�s��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'c' )
		{
			if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                reason = "û�ҵ�c�ֶ�!";
				return false;
			}

			// in
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�inǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�in����ֽ�����!";
					return false;
				}
			}

			// ip version
			while( *pContent != ' ' ) pContent++;
			if( pContent == '\0' )
            {
                reason = "c�ֶ�ip versionǰ���ֽ�����!";
				return false;
			}
			else
			{
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "c�ֶ�ip version����ֽ�����!";
					return false;
				}
			}

			// connect ip
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "c�ֶ�connect ipǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_connect.assign( anchor, pContent );
			}
		}
		else if( ch == 't' )
		{
			if( *pContent++ != 't' || *pContent++ != '=' )
            {
                reason = "û�ҵ�t�ֶ�!";
				return false;
			}

			// start time
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�start timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_starttime.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "t�ֶ�start time����ֽ�����!";
					return false;
				}
			}

			// end time
			anchor = pContent;
			while( *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "t�ֶ�end timeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_endtime.assign( anchor, pContent );
			}
		}
		else if( ch == 'm' )
		{
			if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                reason = "û�ҵ�m�ֶ�!";
				return false;
			}

			// media type
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�media typeǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediatype.assign(anchor, pContent);
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�media type����ֽ�����!";
					return false;
				}
			}

			// port
			anchor = pContent;
			while( *pContent != ' ') pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�portǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_mediaport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�port����ֽ�����!";
					return false;
				}
			}

			// transport
			anchor = pContent;
			while( *pContent != ' ' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "m�ֶ�transportǰ���ֽ�����!";
				return false;
			}
			else
			{
				m_transport.assign( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�transport����ֽ�����!";
					return false;
				}
			}

			// rtpmap
			while( *pContent != '\r' && *pContent != '\n' )
			{
				anchor = pContent;
				while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "m�ֶ�rtpmapǰ���ֽ�����!";
					return false;
				}
				else
				{
					std::string rtpmap( anchor, pContent );
					m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
					while( *pContent == ' ' ) pContent++;
					if( *pContent == '\0' )
                    {
                        reason = "m�ֶ�rtpmap����ֽ�����!";
						return false;
					}
				}
			}
		}
		else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                reason = "û�ҵ�b�ֶ�!";
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "�ֶ�b��β���ֽ�����!";
                return false;
            }
		}
		else if( ch == 'a' )
		{
			if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                reason = "û�ҵ�a�ֶ�!";
				return false;
			}

			std::string type;
			anchor = pContent;
			while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
			if( *pContent == '\0' )
            {
                reason = "a�ֶ�rtpmapǰ���ֽ�����!";
				return false;
			}
			else
			{
				type.assign( anchor, pContent );
				while( *pContent == ' ' || *pContent == ':' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�rtpmap����ֽ�����!";
					return false;
				}
			}

			if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
			{
				anchor = pContent;
				while( *pContent != ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload typeǰ���ֽ�����!";
					return false;
				}

				std::string rtpmap( anchor, pContent );
				while( *pContent == ' ' ) pContent++;
				if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload type����ֽ�����!";
					return false;
				}

				std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
				if( it == m_rtpmap.end() )
                {
                    reason = "m�ֶβ����ڸ�payload type!";
					return false;
				}

				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( anchor != pContent )
				{
					it->second.assign( anchor, pContent );
				}
            }
            else if( CGBMessage::CompareNoCase( type, "filesize" ) )
            {
                anchor = pContent;
                while( *pContent != '\r' && *pContent != '\n' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
                else
                {
                    m_filesize.assign( anchor, pContent );
                }
            }
			else
			{
				anchor = pContent;
				while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
				{
					pContent++;
				}

				if( *pContent != '\0' )
				{
					// send or recv
				}
			}
		}
		else if( ch == 'y' )
		{
			if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                reason = "û�ҵ�y�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_ssrc.assign( anchor, pContent );
			}
		}
		else if( ch == 'f' )
		{
			if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                reason = "û�ҵ�f�ֶ�!";
				return false;
			}

			anchor = pContent;
			while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
			{
				pContent++;
			}

			if( anchor != pContent )
			{
				m_audioproterty.assign( anchor, pContent );
			}
		}
		else
		{
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "��ʶ����ֶβ��ҳ��ֽ�����!";
                return false;
            }
		}

		// next line
		while( *pContent == '\r' || *pContent == '\n' ) pContent++;
		if( *pContent == '\0' )
		{
			break;
		}
	}

	return true;
}

void CVodDownloadResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnInviteResponse( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CUnknowStreamResponseMsg::Encode( std::string &message )
{
    std::ostringstream sdp;
    sdp << "v=0\r\n";
    sdp << "o="<< m_recvid << " 0 0 IN IP4 "<< m_mediaip << "\r\n";
    sdp << "s="<< m_sessionname <<"\r\n";
    sdp << "c=IN IP4 " << m_connect << "\r\n";
    sdp << "t=0 0\r\n";
    sdp <<"m="<< m_mediatype << " " << m_mediaport <<" RTP/AVP";
    std::map< std::string, std::string >::const_iterator it;
    for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
    {
        std::string key = it->first;
        std::string text = it->second;
        if ( !text.empty() )
        {
            sdp <<" "<<key;
        }
    }
    sdp <<"\r\n";
    sdp << "a=sendonly\r\n";
    for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
    {
        std::string key = it->first;
        std::string text = it->second;
        if ( !text.empty() )
        {
            sdp <<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
        }
    }
    if( !m_filesize.empty() )
    {
        sdp << "a=filesize:" << m_filesize << "\r\n";
    }

    message = sdp.str();
    return true;
}

bool CUnknowStreamResponseMsg::Decode( const char *body, std::string &reason )
{
    // message without sdp
    if( body == NULL )
    {
#ifdef _DEBUG
        WARN_LOG( "¼�����ػ�Ӧ��Ϣ�岻��SDP!" );
#endif
        reason = "ʵʱ����Ӧ��Ϣ�岻��SDP!";
        return true;
    }

    const char* pContent = body;
    const char* anchor = 0;

    // ����SDP�ֶ�
    while( pContent != '\0' )
    {
        char ch = *pContent;
        if( ch == 'v' )
        {
            if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                reason = "û�ҵ�v�ֶ�!";
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "�ֶ�v��β���ֽ�����!";
                return false;
            }
        }
        else if( ch == 'o' )
        {
            if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                reason = "û�ҵ�o�ֶ�!";
                return false;
            }

            // id
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "o�ֶ�idǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_recvid.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "o�ֶ�id����ֽ�����!";
                    return false;
                }
            }

            // session
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "o�ֶ�sessionǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_sessionid.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "o�ֶ�session����ֽ�����!";
                    return false;
                }
            }

            // version
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "o�ֶ�versionǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_version.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "o�ֶ�version����ֽ�����!";
                    return false;
                }
            }

            // in
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "o�ֶ�inǰ���ֽ�����!";
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "o�ֶ�in����ֽ�����!";
                    return false;
                }
            }

            // ip version
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "o�ֶ�ip versionǰ���ֽ�����!";
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "o�ֶ�ip version����ֽ�����!";
                    return false;
                }
            }

            // media ip
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "o�ֶ�ipǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_mediaip.assign( anchor, pContent );
            }
        }
        else if( ch == 's' )
        {
            if( *pContent++ != 's' || *pContent++ != '=' )
            {
                reason = "û�ҵ�s�ֶ�!";
                return false;
            }

            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( anchor != pContent )
            {
                m_sessionname.assign( anchor, pContent );
            }

            if( *pContent == '\0' )
            {
                reason = "�ֶ�s��β���ֽ�����!";
                return false;
            }
        }
        else if( ch == 'c' )
        {
            if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                reason = "û�ҵ�c�ֶ�!";
                return false;
            }

            // in
            while( *pContent != ' ' ) pContent++;
            if( pContent == '\0' )
            {
                reason = "c�ֶ�inǰ���ֽ�����!";
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "c�ֶ�in����ֽ�����!";
                    return false;
                }
            }

            // ip version
            while( *pContent != ' ' ) pContent++;
            if( pContent == '\0' )
            {
                reason = "c�ֶ�ip versionǰ���ֽ�����!";
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "c�ֶ�ip version����ֽ�����!";
                    return false;
                }
            }

            // connect ip
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "c�ֶ�connect ipǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_connect.assign( anchor, pContent );
            }
        }
        else if( ch == 't' )
        {
            if( *pContent++ != 't' || *pContent++ != '=' )
            {
                reason = "û�ҵ�t�ֶ�!";
                return false;
            }

            // start time
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "t�ֶ�start timeǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_starttime.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "t�ֶ�start time����ֽ�����!";
                    return false;
                }
            }

            // end time
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "t�ֶ�end timeǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_endtime.assign( anchor, pContent );
            }
        }
        else if( ch == 'm' )
        {
            if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                reason = "û�ҵ�m�ֶ�!";
                return false;
            }

            // media type
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "m�ֶ�media typeǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_mediatype.assign(anchor, pContent);
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "m�ֶ�media type����ֽ�����!";
                    return false;
                }
            }

            // port
            anchor = pContent;
            while( *pContent != ' ') pContent++;
            if( *pContent == '\0' )
            {
                reason = "m�ֶ�portǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_mediaport.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "m�ֶ�port����ֽ�����!";
                    return false;
                }
            }

            // transport
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "m�ֶ�transportǰ���ֽ�����!";
                return false;
            }
            else
            {
                m_transport.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "m�ֶ�transport����ֽ�����!";
                    return false;
                }
            }

            // rtpmap
            while( *pContent != '\r' && *pContent != '\n' )
            {
                anchor = pContent;
                while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "m�ֶ�rtpmapǰ���ֽ�����!";
                    return false;
                }
                else
                {
                    std::string rtpmap( anchor, pContent );
                    m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
                    while( *pContent == ' ' ) pContent++;
                    if( *pContent == '\0' )
                    {
                        reason = "m�ֶ�rtpmap����ֽ�����!";
                        return false;
                    }
                }
            }
        }
        else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                reason = "û�ҵ�b�ֶ�!";
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "�ֶ�b��β���ֽ�����!";
                return false;
            }
        }
        else if( ch == 'a' )
        {
            if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                reason = "û�ҵ�a�ֶ�!";
                return false;
            }

            std::string type;
            anchor = pContent;
            while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "a�ֶ�rtpmapǰ���ֽ�����!";
                return false;
            }
            else
            {
                type.assign( anchor, pContent );
                while( *pContent == ' ' || *pContent == ':' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "a�ֶ�rtpmap����ֽ�����!";
                    return false;
                }
            }

            if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
            {
                anchor = pContent;
                while( *pContent != ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload typeǰ���ֽ�����!";
                    return false;
                }

                std::string rtpmap( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "a�ֶ�payload type����ֽ�����!";
                    return false;
                }

                std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
                if( it == m_rtpmap.end() )
                {
                    reason = "m�ֶβ����ڸ�payload type!";
                    return false;
                }

                anchor = pContent;
                while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
                {
                    pContent++;
                }

                if( anchor != pContent )
                {
                    it->second.assign( anchor, pContent );
                }
            }
            else if( CGBMessage::CompareNoCase( type, "filesize" ) )
            {
                anchor = pContent;
                while( *pContent != '\r' && *pContent != '\n' ) pContent++;
                if( *pContent == '\0' )
                {
                    reason = "a�ֶ�filesizeǰ���ֽ�����!";
                    return false;
                }
                else
                {
                    m_filesize.assign( anchor, pContent );
                }
            }
            else
            {
                anchor = pContent;
                while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
                {
                    pContent++;
                }

                if( *pContent != '\0' )
                {
                    // send or recv
                }
            }
        }
        else if( ch == 'y' )
        {
            if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                reason = "û�ҵ�y�ֶ�!";
                return false;
            }

            anchor = pContent;
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
            {
                pContent++;
            }

            if( anchor != pContent )
            {
                m_ssrc.assign( anchor, pContent );
            }
        }
        else if( ch == 'f' )
        {
            if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                reason = "û�ҵ�f�ֶ�!";
                return false;
            }

            anchor = pContent;
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
            {
                pContent++;
            }

            if( anchor != pContent )
            {
                m_audioproterty.assign( anchor, pContent );
            }
        }
        else
        {
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                reason = "��ʶ����ֶβ��ҳ��ֽ�����!";
                return false;
            }
        }

        // next line
        while( *pContent == '\r' || *pContent == '\n' ) pContent++;
        if( *pContent == '\0' )
        {
            break;
        }
    }

    return true;
}

void CUnknowStreamResponseMsg::Process( CSipInterface *pInterface )
{
    if( pInterface != NULL )
    {
        pInterface->OnInviteResponse( *this );
    }
}

//////////////////////////////////////////////////////////////////////////
bool CAckMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
    if( m_handle == NULL )
    {
        ERROR_LOG( "m_handleΪ�գ�����ʧ��!" );
        return false;
    }
    else
    {
        // create sdp
        std::string body;
        if( !Encode( body ) )
        {
            ERROR_LOG( "��Ϣ������󣬷���ʧ��!" );
            return false;
        }

        resip::HeaderFieldValue hfv( body.c_str(), body.length() );
        resip::Mime mime_type("Application","sdp");
        resip::PlainContents sdp(hfv, mime_type);
		m_handle->provideAnswer(sdp);
        //m_handle->sendAckWithSDP( sdp );
        return true;
    }
}

bool CAckMsg::Encode( std::string &message )
{
    // sdp
    std::stringstream context;
    context<<"v=0\r\n";
    context<<"o="<< m_recvid <<" 0 0 IN IP4 "<< m_mediaip <<"\r\n";
    context<<"s=Play\r\n";
    context<<"c=IN IP4 "<< m_connect <<"\r\n";
    context<<"t=0 0\r\n";

    context<<"m=video "<< m_mediaport <<" RTP/AVP";
    std::map< std::string, std::string >::const_iterator it;
    for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
    {
        std::string key = it->first;
        std::string text = it->second;
        if ( !text.empty() )
        {
            context<<" "<<key;
        }
    }
    context<<"\r\n";

    context<<"a=sendonly\r\n";
    for (it = m_rtpmap.begin(); it != m_rtpmap.end(); it++)
    {
        std::string key = it->first;
        std::string text = it->second;
        if ( !text.empty() )
        {
            context<<"a=rtpmap:"<<key<<" "<<text<<"\r\n";
        }
    }

	if( !m_ssrc.empty() )
	{
	   context<<"y="<< m_ssrc << "\r\n";
	}

	if( !m_audioproterty.empty() )
	{
	   context<<"f=" << m_audioproterty << "\r\n";
	}

    message = context.str();
	return true;
}

bool CAckMsg::Decode( const char *body, std::string &reason )
{
    // message without sdp
    if( body == NULL )
    {
        WARN_LOG( "ACK��Ϣ����SDP!" );
        return true;
    }

    const char* pContent = body;
    const char* anchor = 0;

    // ����SDP�ֶ�
    while( pContent != '\0' )
    {
        char ch = *pContent;
        if( ch == 'v' )
        {
            if( *pContent++ != 'v' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
        else if( ch == 'o' )
        {
            if( *pContent++ != 'o' || *pContent++ != '=' )
            {
                return false;
            }

            // id
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_recvid.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // session
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_sessionid.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // version
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_version.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // in
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // ip version
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // media ip
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_mediaip.assign( anchor, pContent );
            }
        }
        else if( ch == 's' )
        {
            if( *pContent++ != 's' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
        else if( ch == 'c' )
        {
            if( *pContent++ != 'c' || *pContent++ != '=' )
            {
                return false;
            }

            // in
            while( *pContent != ' ' ) pContent++;
            if( pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // ip version
            while( *pContent != ' ' ) pContent++;
            if( pContent == '\0' )
            {
                return false;
            }
            else
            {
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // connect ip
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_connect.assign( anchor, pContent );
            }
        }
        else if( ch == 't' )
        {
            if( *pContent++ != 't' || *pContent++ != '=' )
            {
                return false;
            }

            // start time
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_starttime.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // end time
            anchor = pContent;
            while( *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_endtime.assign( anchor, pContent );
            }
        }
        else if( ch == 'm' )
        {
            if( *pContent++ != 'm' || *pContent++ != '=' )
            {
                return false;
            }

            // media type
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_mediatype.assign(anchor, pContent);
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // port
            anchor = pContent;
            while( *pContent != ' ') pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_mediaport.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // transport
            anchor = pContent;
            while( *pContent != ' ' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                m_transport.assign( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            // rtpmap
            while( *pContent != '\r' && *pContent != '\n' )
            {
                anchor = pContent;
                while( *pContent != ' ' && *pContent != '\r' && *pContent != '\n' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
                else
                {
                    std::string rtpmap( anchor, pContent );
                    m_rtpmap.insert( std::make_pair( rtpmap, "" ) );
                    while( *pContent == ' ' ) pContent++;
                    if( *pContent == '\0' )
                    {
                        return false;
                    }
                }
            }
        }
        else if( ch == 'b' )
        {
            if( *pContent++ != 'b' || *pContent++ != '=' )
            {
                return false;
            }

            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }
        else if( ch == 'a' )
        {
            if( *pContent++ != 'a' || *pContent++ != '=' )
            {
                return false;
            }

            std::string type;
            anchor = pContent;
            while( *pContent != ':' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
            else
            {
                type.assign( anchor, pContent );
                while( *pContent == ' ' || *pContent == ':' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }
            }

            if( CGBMessage::CompareNoCase( type, "rtpmap" ) )
            {
                anchor = pContent;
                while( *pContent != ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }

                std::string rtpmap( anchor, pContent );
                while( *pContent == ' ' ) pContent++;
                if( *pContent == '\0' )
                {
                    return false;
                }

                std::map< std::string, std::string >::iterator it = m_rtpmap.find( rtpmap );
                if( it == m_rtpmap.end() )
                {
                    return false;
                }

                anchor = pContent;
                while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
                {
                    pContent++;
                }

                if( anchor != pContent )
                {
                    it->second.assign( anchor, pContent );
                }
            }
            else
            {
                anchor = pContent;
                while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
                {
                    pContent++;
                }

                if( *pContent != '\0' )
                {
                    // send or recv
                }
            }
        }
        else if( ch == 'y' )
        {
            if( *pContent++ != 'y' || *pContent++ != '=' )
            {
                return false;
            }

            anchor = pContent;
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
            {
                pContent++;
            }

            if( anchor != pContent )
            {
                m_ssrc.assign( anchor, pContent );
            }
        }
        else if( ch == 'f' )
        {
            if( *pContent++ != 'f' || *pContent++ != '=' )
            {
                return false;
            }

            anchor = pContent;
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' )
            {
                pContent++;
            }

            if( anchor != pContent )
            {
                m_audioproterty.assign( anchor, pContent );
            }
        }
        else
        {
            while( *pContent != '\0' && *pContent != '\r' && *pContent != '\n' ) pContent++;
            if( *pContent == '\0' )
            {
                return false;
            }
        }

        // next line
        while( *pContent == '\r' || *pContent == '\n' ) pContent++;
        if( *pContent == '\0' )
        {
            break;
        }
    }

	return true;
}

void CAckMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnAck( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CByeMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
	if( m_handle == NULL )
	{
		ERROR_LOG( "m_handleΪ�գ�����ʧ��!" );
		return false;
	}
	else
	{
        m_handle->end(resip::InviteSession::UserHangup);
		return true;
	}
}

bool CByeMsg::Encode( std::string &message )
{
	return true;
}

bool CByeMsg::Decode( const char *body, std::string &reason )
{
	if( body != NULL )
	{
		WARN_LOG(" Bye��Ϣ�岻Ӧ�ô���Ϣ�����ݣ� ");
		return false;
	}

	return true;
}

void CByeMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnBye( *this );
	}
}
