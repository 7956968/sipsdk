#ifndef _SIP_SDK_RESIP_IMPL_H__
#define _SIP_SDK_RESIP_IMPL_H__

#include <map>
#include <string>

#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/Subsystem.hxx"
#include "resip/stack/Helper.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/StackThread.hxx"
#include "resip/stack/PlainContents.hxx"
#include "resip/stack/EventStackThread.hxx"
#include "resip/dum/ClientRegistration.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/PagerMessageHandler.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/OutOfDialogHandler.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/ClientPagerMessage.hxx"
#include "resip/dum/ServerPagerMessage.hxx"
#include "resip/dum/KeepAliveManager.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/ClientSubscription.hxx"
#include "resip/dum/ServerSubscription.hxx"
#include "resip/dum/SubscriptionHandler.hxx"
#include "resip/dum/ClientOutOfDialogReq.hxx"
#include "resip/dum/ServerOutOfDialogReq.hxx"
#include "resip/dum/DumException.hxx"
#include "resip/dum/InMemorySyncRegDb.hxx"
#include "resip/dum/UserAuthInfo.hxx"
#include "rutil/Timer.hxx"
#include "OutboundProxyContainer.h"

#include "../Message/QueryCatalogMsg.hpp"
#include "../Message/QueryRecordInfoMsg.hpp"
#include "../Message/QueryAlarmMsg.hpp"
#include "../Message/AlarmNotifyMsg.hpp"
#include "../Message/DeviceControlMsg.hpp"
#include "../Message/KeepaliveMsg.hpp"
#include "../Message/InviteMsg.hpp"
#include "../Message/RegisterMsg.hpp"
#include "../Message/MessageMsg.hpp"
#include "../Message/CatalogNotifyMsg.hpp"
#include "../Message/InviteMsg.hpp"
#include "../Message/BroadcastMsg.hpp"
#include "../Message/DeviceControlMsg.hpp"
#include "../Message/QueryDeviceInfoMsg.hpp"
#include "../Message/QueryRecordInfoMsg.hpp"
#include "../Message/SubscriptionMsg.hpp"
#include "../Message/CallInfoMsg.hpp"
#include "../Message/CallMessageMsg.hpp"
#include "../Message/QueryDeviceStatusMsg.hpp"
#include "../Message/ConfigDownloadMsg.hpp"
#include "../Message/DeviceConfigMsg.hpp"
#include "../Message/NpGatewayMsg.hpp"
#include "../Message/NpClientMsg.hpp"
#include "../Message/GetTempAccountMsg.hpp"
//#include "TaskMan.h"
#include "Log.hpp"

#undef  RESIPROCATE_SUBSYSTEM
#define RESIPROCATE_SUBSYSTEM resip::Subsystem::APP

#if defined(CATCH_RESIP_EXCEPTION)
#undef __DUM_TRY
#undef __DUM_CATCH
#define __DUM_TRY try {
#define __DUM_CATCH \
} catch(resip::DumException* be) { \
resip::Data ex; \
ex = "RESIP �����쳣��name = "; \
ex += be->name(); \
ex += "��what = "; \
ex += be->what(); \
throw new std::exception(ex.c_str()); \
}
#else
#undef  __DUM_TRY
#undef  __DUM_CATCH
#define __DUM_TRY
#define __DUM_CATCH
#endif
#define SendMessage  SendMessage
//////////////////////////////////////////////////////////////////////////
class CSipInterface;
class ClientRegHandler;
class ServerRegHandler;
class ClientMessageHandler;
class ServerMessageHandler;
class InviteHandler;
class CNotifyHandler;
class CSubscriptionHandler;
class CNPClientAuthManager;

static void SplitWord( const std::string& line, std::vector<std::string>& vec, const std::string& spec )
{
    vec.clear();
    std::string::const_iterator b = line.begin();
    std::string::const_iterator e = line.end();
    while (b != e)
    {
        std::string::const_iterator wordEnd = find_first_of(b, e, spec.begin(), spec.end());
        vec.push_back(std::string(b, wordEnd));
        if (wordEnd != line.end())
        {
            b = wordEnd + 1;
        }
        else
        {
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
class CSipInterface
{
public:
	enum ECatalogType
	{
		CATALOG_CIVIL,			// ��������Ŀ¼��(ʡ��/�м�/����/������뵥λ)
		CATALOG_SYSTEM,			// ϵͳĿ¼��
		CATALOG_BUSINESS,		// ҵ�����Ŀ¼��
		CATALOG_ORGANIZATION,	// ������֯Ŀ¼�� 
		CATALOG_DEVICE			// �豸Ŀ¼��
	};

public:
    CSipInterface();
    virtual ~CSipInterface();

public:
	/** ��ȡID��ʶ��Ŀ¼����
	 *
	 *  @param id [in] �豸Ŀ¼ID
	 *  @return �����豸Ŀ¼������
	 */
	static ECatalogType GetIDType( const std::string &id );

public:
    std::string ConvertIntoMD5password(const char* userName, const char* realm, const char* password);

public:
	virtual void Start();
	virtual void Stop();
	virtual void SetProxy();
	virtual void SetDefaultFrom();
	virtual void SetUserAgent();

public:
	/** ���ݴ���Ĺ���ID���ҵ���Ӧ��ͨ��������ƽ̨
	 *
	 *  @param code [in] ͨ������ID
	 *  @param userCode [out] �����������ƽ̨ID
	 *  @return �ҵ�ͨ��ID��Ӧ��ƽ̨ID������true,���򷵻�false
	 */
	virtual bool GetMatchedUser( const std::string& code, std::string& userCode )
	{
		return false;
	}

	/** ���ݴ���Ĺ���ID���ҵ�Ӧ��ID��Ŀ���ַ
	 *
	 *  @param code [in] ����ID
	 *  @param targetAddr [out] Ŀ���ַ
	 *  @return �ҵ�ͨ��ID��Ӧ��Ŀ���ַ������true,���򷵻�false
	 */
	virtual bool GetTargetAddr( const std::string& code, resip::Uri& targetAddr )
	{
		return false;
	}

	/** ���ͻ�ȡ�û���ע����Ϣ��������Ϣ
	 *
	 */
	virtual void RequestUserAuthInfo(const char* user, const char* realm, const char* transactionId );

	/** �ж��û��Ƿ���ע���û�
	 *
	 *  @param code [in] �û�ID
	 *  @return �����û��Ƿ���ע���û���true:��, false:����
	 */
	virtual bool IsRegistedUser( const std::string &code )
	{
		return false;
	}

	/** �ж��û��Ƿ���ע���û�
	 *
	 *  @param code [in] �û�ID
	 *  @param reg  [in] �Ƿ���ע����Ϣ
	 *  @return �����û��Ƿ���ע���û���true:��, false:����
	 */
	virtual bool NeedRegisted( const std::string &code )
	{
		return true;
	}

	/** �ж��û��Ƿ���ҪУʱ
	 *
	 *  @param code  [in] �û�ID
	 *  @return �����û��Ƿ���ҪУʱ,true:��, false:����
	 */
	virtual bool NeedAdjustTime( const std::string &code )
	{
		return false;
	}

	/** ȡ�û���ע����Ϣ
	 *
	 *  @param user [in]  �û�ID
	 *  @parma a1   [out] ���ص��û�ע����Ϣ
	 */
	virtual bool GetUserAuthInfo( const char* user, const char* realm, std::string& a1 ) 
	{
		return false;
	}

	/** У���û��Ƿ�ע�����
	 *
	 *  @param user [in] �û���Ϣ
	 *  @return �û��ѹ��ڣ�true:���ڣ�false:û�й���
	 */
	virtual bool CheckUserRegExpires( const std::string &user )
	{
		return false;
	}

public:
	/** ����SipSdk�ı�����Ϣ
	 *
	 *  @param ip   [in] SipSdk�󶨵�IP��ַ
	 *  @param port [in] SipSdk�󶨵Ķ˿ں�
	 *  @param id   [in] SipSdkʹ�õĹ���ID
	 */
	virtual void SetSipLocalInfo( const std::string &ip, int port, const std::string &id );

	/** ����SipSdk�ı���IP��ַ
	 *
	 *  @param ip   [in] SipSdk�󶨵�IP��ַ
	 */
	virtual void SetSipLocalIp( const std::string &ip );
    
	/** ����SipSdk�ı��ض˿ں�
	 *
	 *  @param port [in] SipSdk�󶨵Ķ˿ں�
	 */
	virtual void SetSipLocalPort( int port );
    
	/** ����SipSdkʹ�õĹ���ID
	 *
	 *  @param id [in] SipSdkʹ�õĹ���ID
	 */
	virtual void SetRegisterUserName( const std::string &id );

	/** ����SipSdk����־·��
	 *
	 *  @param path [in] ��־·��
	 */
    virtual void SetLogPath(std::string &path);

	/** ����SipSdk����־����
	 *
	 *  @param level [in] ��־����
	 */
	virtual void SetLogLevel(int level);

	/** ����SipSdk����־����
	 *
	 *  @param level [in] ��־����
	 */
	virtual void SetSdkLogLevel(int level);

	/** ���ù����̸߳���
	 *
	 *  @note �ú���������Start����֮ǰ������Ч������ʹ��Ĭ�ϵ��̸߳���
	 */
	void SetThreads( int threads );

	/** ����ϵͳʱ��
	 *
	 *  @param time [tm] ʱ��
	 */
	bool SetLocalTime( int year, int month, int day, int hour, int minute, int second );

	/** ����ע�ᳬʱʱ��
	 *
	 */
	virtual void SetRegTime(int regtime);

	/** ����һ����Ϣ
	 *
	 *  @param pMsg [in] ��Ϣָ��
	 *  @param tcp  [in] ��ʶ�Ƿ�ʹ��TCP���͸���Ϣ
	 */
	virtual void SendMessage( CGBMessage *pMsg, bool tcp = false );

	/** ��ʼ��SipЭ��ջ
	 *
	 */
	virtual void RunSipStack();
	//virtual void RegisterToParent( const std::string parentid );

public:
	virtual void OnTimer() {}
	virtual void OnRegister( const CRegisterAddMsg& ) {}
	virtual void OnRegister( const CRegisterRequestMsg& ) {}
	virtual void OnRegisterRemove( const CRegisterRemoveMsg& ) {}
	virtual void OnRegisterRemoveAll( const CRegisterRemoveAllMsg& ) {}
	virtual void OnRegisterRefresh( const CRegisterRefreshMsg& ) {}
	virtual void OnRegisterResponse( const CRegisterResponseMsg& ) {}
	virtual void OnQueryCatalog( const CQueryCatalogMsg& ){}
	virtual void OnQueryCatalogResponse( const CQueryCatalogResponseMsg& ) {}
	virtual void OnQueryRecordInfo( const CQueryRecordInfoMsg& ){}
	virtual void OnQueryRecordInfoResponse( const CQueryRecordInfoResponseMsg& ) {}
	virtual void OnQueryDeviceInfo( const CQueryDeviceInfoMsg& ) {}
	virtual void OnQueryDeviceInfoResponse( const CQueryDeviceInfoResponseMsg& ) {}
	virtual void OnQueryAlarm( const CQueryAlarmMsg& ){}
	virtual void OnQueryAlarmResponse( const CQueryAlarmResponseMsg& ){}
	virtual void OnPTZControl( const CPTZControlMsg& ) {}
	virtual void OnPTZLock( const CPTZLockMsg& ) {}
	virtual void OnRecordControl( const CRecordControlMsg& ) {}
	virtual void OnTeleBoot( const CTeleBootMsg& ) {}
	virtual void OnResetAlarm( const CResetAlarmMsg& ) {}
	virtual void OnGuard( const CGuardMsg& ) {}
	virtual void OnDragZoomIn( const CDragZoomInMsg& ) {}
	virtual void OnDragZoomOut( const CDragZoomOutMsg& ) {}
	virtual void OnDeviceControlResponse( const CDeviceControlResponseMsg& ) {}
	virtual void OnQueryDeviceStatus( const CQueryDeviceStatusMsg& ) {}
    virtual void OnQueryDeviceStatusResponse( const CQueryDeviceStatusResponseMsg& ) {}
	virtual void OnMessageResponse( const CMessageResponseMsg& ){}
	virtual void OnKeepAlive( const CKeepAliveMsg& ){}
	virtual void OnOpenRealStream( const CRealStreamMsg& ) {}
	virtual void OnOpenRealStreamToDecorder( const CRealStreamToDecorderMsg& ) {}
	virtual void OnOpenBroadcastStream( const CBroadcastStreamMsg& ) {}
    virtual void OnOpenVodStream( const CVodStreamMsg& ) {}
    virtual void OnVodCtrl( const CVodCtrlMsg& ) {}
	virtual void OnDownloadVodStream( const CVodDownloadMsg& ) {}
	virtual void OnInviteResponse( const CRealStreamResponseMsg& ) {}
	virtual void OnInviteResponse( const CBroadcastStreamResponseMsg& ) {}
	virtual void OnInviteResponse( const CVodStreamResponseMsg& ) {}
    virtual void OnInviteResponse( const CVodDownloadResponseMsg& ) {}
    virtual void OnInviteResponse( const CUnknowStreamResponseMsg& ) {}
	virtual void OnCallInfoResponse( const CCallInfoResponseMsg& ) {}
	virtual void OnCallMessageResponse( const CCallMessageResponseMsg & ) {}
	virtual void OnAck( const CAckMsg& ) {}
	virtual void OnBye( const CByeMsg& ) {}
	virtual void OnAddSubscription( const CAddSubscribeMsg& ) {}
	virtual void OnCancelSubscription( const CCancelSubscribeMsg& ) {}
	virtual void OnSubscriptionResponse( const CSubscribeResponseMsg& ) {}
	virtual void OnAlarmNotify( const CAlarmNotifyMsg& ) {}
	virtual void OnAlarmNotifyResponse( const CAlarmNotifyResponseMsg& ) {}
	virtual void OnBroadcast( const CBroadcastMsg& ) {}
	virtual void OnBroadcastResponse( const CBroadcastResponseMsg& ) {}
	virtual void OnCatalogNotify( const CCatalogNotifyMsg& ) {}
	virtual void OnNotifyResponse( const CNotifyResponseMsg& ) {}
    virtual void OnStreamPlayOver( const CStreamPlayEndMsg& ) {}

    virtual void OnConfigDownload( const CConfigDownloadMsg& ) {}
    virtual void OnBasicParamResponse( const CBasicParamResponseMsg& ) {}
    virtual void OnVideoParamConfigResponse( const CVideoParamConfigResponseMsg& ) {}
    virtual void OnAudioParamConfigResponse( const CAudioParamConfigResponseMsg& ) {}
    virtual void OnSVACEncodeConfigResponse( const CSVACEncodeConfigResponseMsg& ) {}
    virtual void OnSVACDecodeConfigResponse( const CSVACDecodeConfigResponseMsg& ) {}

    virtual void OnBasicParam( const CBasicParamMsg& ) {}
    virtual void OnVideoParamConfig( const CVideoParamConfigMsg& ) {}
    virtual void OnAudioParamConfig( const CAudioParamConfigMsg& ) {}
    virtual void OnSVACEncodeConfig( const CSVACEncodeConfigMsg& ) {}
    virtual void OnSVACDecodeConfig( const CSVACDecodeConfigMsg& ) {}
    virtual void OnDeviceConfigResponse( const CDeviceConfigResponseMsg& ) {}

    virtual void OnGetTempAccount( const CGetTempAccountMsg& ) {}
    virtual void OnGetTempAccountResponse(const CGetTempAccountResponseMsg& ) {}

public:
	virtual void OnRequetAuth( const char *realm );
	virtual void OnInvite( resip::ServerInviteSession *h, const resip::SipMessage *msg );
	virtual void OnInviteResponse( resip::ClientInviteSession *h, const resip::SipMessage *msg );
	virtual void OnAck( resip::InviteSession *h, const resip::SipMessage *msg );
	virtual void OnBye( resip::InviteSession *h, const resip::SipMessage *msg );
	virtual void OnCallInfo( resip::InviteSession *h, const resip::SipMessage *msg );
	virtual void OnCallInfoResponse( resip::InviteSession *h, const resip::SipMessage *msg );
	virtual void OnCallMessage( resip::InviteSession *h, const resip::SipMessage *msg );
	virtual void OnCallMessageResponse( resip::InviteSession *h, const resip::SipMessage *msg );
	virtual void OnMessageResponse( resip::ClientPagerMessage *h, const resip::SipMessage *msg );
	virtual void OnMessage( resip::ServerPagerMessage *h, const resip::SipMessage *msg );
	virtual void OnNotifyResponse( resip::ClientOutOfDialogReq *h, const resip::SipMessage *msg );
	virtual void OnNotify( resip::ServerOutOfDialogReq *h, const resip::SipMessage *msg );
	virtual void OnRegisterResponse( resip::ClientRegistration *h, const resip::SipMessage *msg );
	virtual void OnRegister( resip::ServerRegistration *h, const resip::SipMessage *msg );
	virtual void OnRefresh( resip::ServerRegistration *h, const resip::SipMessage *msg );
	virtual void OnRegisterRemove( resip::ServerRegistration *h, const resip::SipMessage *msg );
	virtual void OnRegisterRemoveAll( resip::ServerRegistration *h, const resip::SipMessage *msg );
	virtual void OnAddSubscription( resip::ClientSubscription *h, const resip::SipMessage *msg );
	virtual void OnUpdateSubscription( resip::ClientSubscription *h, const resip::SipMessage *msg );
	virtual void OnCancelSubscription( resip::ClientSubscription *h, const resip::SipMessage *msg );
	virtual void OnCancelSubscription( resip::ServerSubscription *h );
	virtual void OnAddSubscription( resip::ServerSubscription *h, const resip::SipMessage *msg );
	virtual void OnSubscriptionResponse( resip::ServerSubscription *h, const resip::SipMessage *msg );

protected:
	std::string m_sUserName;
	std::string m_sPassword;
	std::string m_sRealm;
	std::string m_UserAgent;
	std::string m_sLocalIp;
	int         m_iListenPort;
	int         m_regtime;
	int         m_threads;
	int         m_loglevel;
	int         m_sdkloglevel;
	bool		m_bKeepAlive;

protected:
#if !defined(WIN32)
    resip::EventThreadInterruptor* mEventIntr;
#endif
    resip::SipStack    mStack;
    resip::SharedPtr<resip::MasterProfile>  mSProfile;
	CNPClientAuthManager* mAClientAuthManager;
    resip::DialogUsageManager mDum;
#if !defined(WIN32)
    resip::EventStackThread mStackThread;
#else
    resip::StackThread mStackThread;
#endif
    resip::DumThread mDumThread;

protected:
    ClientRegHandler*      mClientRegHandler;
    ServerRegHandler*      mServerRegHandler;
    ClientMessageHandler*  mClientMessageHandler;
    ServerMessageHandler*  mServerMessageHandler;
    InviteHandler*         mInviteHandler;
	CNotifyHandler*        mNotifyHandler;
    CSubscriptionHandler*  mSubscriptionHandler;
	OutboundProxyContainer mOutboundProxyContainer;

protected:
	//NPTimerScope m_KeepAlive;
};

#endif /* ! _SIP_SDK_RESIP_IMPL_H__ */
