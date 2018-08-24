#ifndef _SERVER_REGISTERHANDLER_H_
#define _SERVER_REGISTERHANDLER_H_

#include <list>
#include <string>
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/InMemoryRegistrationDatabase.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/stack/Headers.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/InMemoryRegistrationDatabase.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/stack/Headers.hxx"
#include "SipInterface.h"

class CSipInterface;
class ClientRegHandler : public resip::ClientRegistrationHandler
{
public:
	ClientRegHandler( CSipInterface *pSip ) : m_pSip( pSip )
	{
	}

public:
    virtual void onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
    {
		// ���ϼ�ע��ɹ�
		if( m_pSip != NULL )
		{
			m_pSip->OnRegisterResponse( h.get(), &response );
		}
    }

    virtual void onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
    {
		// �ϼ������Ͽ�ע��
		if( m_pSip != NULL )
		{
		}
    }

    virtual void onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
    {
		// ���ϼ�ע��ʧ��
		if( m_pSip != NULL )
		{
			m_pSip->OnRegisterResponse( h.get(), &response );
		}
    }

    virtual int onRequestRetry(resip::ClientRegistrationHandle, int retrySeconds, const resip::SipMessage& response)
    {
        return 30;
    }

private:
	CSipInterface* m_pSip;
};

class ServerRegHandler: public resip::ServerRegistrationHandler
{
public:
	ServerRegHandler( CSipInterface *pSip );
	virtual ~ServerRegHandler();

public:
	/** �û�ˢ��ע��
	 *
	 */
    virtual void onRefresh(resip::ServerRegistrationHandle, const resip::SipMessage& reg);

	/** �û�ע��
	 *
	 */
    virtual void onRemove(resip::ServerRegistrationHandle, const resip::SipMessage& reg);

	/** �û�ע��
	 *
	 */
    virtual void onRemoveAll(resip::ServerRegistrationHandle, const resip::SipMessage& reg);

	/** �û�ע��
	 *
	 */
    virtual void onAdd(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
    virtual void onQuery(resip::ServerRegistrationHandle, const resip::SipMessage& reg);

private:
	CSipInterface* m_pSip;
};

/** �û���Ȩ����
 *
 */
class CNPClientAuthManager : public resip::ClientAuthManager
{
public:
	CNPClientAuthManager( CSipInterface *pSip ) : m_pSip( pSip )
	{
	}

public:
	virtual bool handle(resip::UserProfile& userProfile, resip::SipMessage& origRequest, const resip::SipMessage& response);

private:
	CSipInterface* m_pSip;
};

#endif
