#include "RegisterMsg.hpp"
#include "SipInterface.h"

//////////////////////////////////////////////////////////////////////////
bool CRegisterRequestMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
	__DUM_TRY
	resip::NameAddr userAor;
	userAor.uri().user() = m_from.c_str();
	userAor.uri().host() = m_from.substr( 0, 10 ).c_str();
	resip::SharedPtr< resip::SipMessage > request = mDum.makeRegistration( userAor, m_expires );

	// from
	resip::Uri from;
	from.user() = m_from.c_str();
	from.host() = m_from.substr( 0, 10 ).c_str();
	request->header( resip::h_From ).uri() = from;

	// ��Ϣ�ķ��͵�transportѡ���Ǹ���dns���ҵĽ��������ͣ���dns����˳����ForceTarget,route,RequestLine
	// ��ע����Ϣ������ForceTarget�����ע����Ϣ��Ҫ����transport������ѡ���ͷ�ʽ
	// ���ݹ����Ҫ��ע����Ϣ�ǲ���Ҫʹ��TCP����ģʽ�ģ����ֱ������ΪUDP
	// TransportSelector::dnsResolve(DnsResult* result,SipMessage* msg)
	// (msg->hasForceTarget())
	// msg->exists(h_Routes) && !msg->header(h_Routes).empty())
	// mDns.lookup(result, msg->header(h_RequestLine).uri());
	resip::Uri naTo;
	naTo.user() = m_request.c_str();
	naTo.host() = m_ip.c_str();
	naTo.port() = m_port;
	naTo.param( resip::p_transport ) = "UDP";
	request->setForceTarget( naTo );
	request->header( resip::h_RequestLine ).uri() = naTo;

	// ������Ϣ�ĻỰID
	if( request->exists( resip::h_CallID ) )
	{
		m_callid = request->header( resip::h_CallID ).value().c_str();
	}

	mDum.send( request );
	__DUM_CATCH
	return true;
}

bool CRegisterRequestMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterRequestMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterRequestMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRegister( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRegisterMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
	__DUM_TRY
	resip::NameAddr userAor;
	userAor.uri().user() = m_from.c_str();
	userAor.uri().host() = m_from.substr( 0, 10 ).c_str();
	resip::SharedPtr< resip::SipMessage > request = mDum.makeRegistration( userAor, m_expires );

	// from
	resip::Uri from;
	from.user() = m_from.c_str();
	from.host() = m_from.substr( 0, 10 ).c_str();
	request->header( resip::h_From ).uri() = from;

	// ��Ϣ�ķ��͵�transportѡ���Ǹ���dns���ҵĽ��������ͣ���dns����˳����ForceTarget,route,RequestLine
	// ��ע����Ϣ������ForceTarget�����ע����Ϣ��Ҫ����transport������ѡ���ͷ�ʽ
	// ���ݹ����Ҫ��ע����Ϣ�ǲ���Ҫʹ��TCP����ģʽ�ģ����ֱ������ΪUDP
	// TransportSelector::dnsResolve(DnsResult* result,SipMessage* msg)
	// (msg->hasForceTarget())
	// msg->exists(h_Routes) && !msg->header(h_Routes).empty())
	// mDns.lookup(result, msg->header(h_RequestLine).uri());
	resip::Uri naTo;
	naTo.user() = m_request.c_str();
	naTo.host() = m_ip.c_str();
	naTo.port() = m_port;
	naTo.param( resip::p_transport ) = "UDP";
	request->setForceTarget( naTo );
	request->header( resip::h_RequestLine ).uri() = naTo;

	// ������Ϣ�ĻỰID
	if( request->exists( resip::h_CallID ) )
	{
		m_callid = request->header( resip::h_CallID ).value().c_str();
	}

	mDum.send( request );
	__DUM_CATCH
	return true;
}

bool CRegisterMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterMsg::Process( CSipInterface *pInterface )
{
}

//////////////////////////////////////////////////////////////////////////
bool CRegisterAddMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterAddMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterAddMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRegister( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRegisterResponseMsg::Send( resip::DialogUsageManager &mDum, bool tcp )
{
	return false;
}

bool CRegisterResponseMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterResponseMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterResponseMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRegisterResponse( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRegisterRefreshMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterRefreshMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterRefreshMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRegisterRefresh( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRegisterRemoveMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterRemoveMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterRemoveMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRegisterRemove( *this );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CRegisterRemoveAllMsg::Encode( std::string &message )
{
	return true;
}

bool CRegisterRemoveAllMsg::Decode( const char *body, std::string &reason )
{
	return true;
}

void CRegisterRemoveAllMsg::Process( CSipInterface *pInterface )
{
	if( pInterface != NULL )
	{
		pInterface->OnRegisterRemoveAll( *this );
	}
}
