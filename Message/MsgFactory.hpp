#ifndef GBMSG_FACTORY_HPP_
#define GBMSG_FACTORY_HPP_

#include "GBMessage.hpp"
#include "MsgCreater.hpp"
#include "resip/stack/SipMessage.hxx"

//////////////////////////////////////////////////////////////////////////
class CGBMsgFactory
{
private:
	enum EGBMsgType
	{
		INVITE,
		INVITE_REPONSE,
		ACK,
		BYE,
		CALLINFO,
		CALLINFO_REPONSE,
		CALLMESSAGE,
		CALLMESSAGE_REPONSE,
		MESSAGE,
		MESSAGE_REPONSE,
		NOTIFY,
		NOTIFY_REPONSE,
		INFO,
		INFO_RESPONSE,
		REGISTER_ADD,
		REGISTER_REPONSE,
		REGISTER_REFRESH,
		REGISTER_REMOVE,
		REGISTER_REMOVE_ALL,
		CLIENT_UPDATE_SUBSCRIPTION,
		CLIENT_CANCEL_SUBSCRIPTION,
		CLIENT_ADD_SUBSCRIPTION,
		SERVER_ADD_SUBSCRIPTION,
		SERVER_RESPONSE_SUBSCRIPTION,
	};

public:
	friend class GBMsgTestCase;

public:
	static CGBMessage* CreateInvite( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateInviteResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateAck( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateBye( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateCallInfo( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateCallInfoResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateCallMessage( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateCallMessageResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateMessage( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateMessageResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateNotify( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateNotifyResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateInfo( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateInfoResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateRegisterAdd( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateRegisterResponse( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateRegisterRefresh( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateRegisterRemove( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateRegisterRemoveAll( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateClientUpdateSubscription( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateClientCancelSubscription( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateClientAddSubscription( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateServerAddSubscription( const resip::SipMessage *message, std::string &resaon );
	static CGBMessage* CreateServerResponseSubscription( const resip::SipMessage *message, std::string &resaon );

public:
	/** ������Ϣ�汾
	 * 
	 *  @param version [in] �����õ���Ϣ�汾��������֮����Ϣ���������ð汾����Ϣ������NULL
	 */
	static void DisableVersion( int version );

	/** ������Ϣ�汾
	 * 
	 *  @param version [in] ������Ϣ�汾�������Ϣ�汾��ȷ��֮�����ʹ�øð汾����Ϣ����������Ϣ
	 */
	static void EnableVersion( int version );

private:
	/** ����һ����Ϣ���󣬴�������Ϣ������ʹ�����������٣���Ϣ����������
	 * 
	 *  @param msgType  [in] ��Ϣ����
	 *  @param message  [in] Sip��Ϣ
	 *  @return ��Ϣ�����ɹ����ش��������Ϣָ�룬���򷵻�NULL
	 *  @note ��Ϣ����ʧ�ܵ�ԭ������:
	 *        - ��Ϣ�İ汾�Ų���ȷ
	 *        - ��Ϣ��ʽ����ȷ
	 *        - �ڴ�ռ䲻��
	 */
	static CGBMessage* Create( EGBMsgType msgType, const resip::SipMessage *message, std::string &resaon );

private:
	CGBMsgFactory();
	~CGBMsgFactory();

private:
	CGBMsgFactory( const CGBMsgFactory &rhs );
	CGBMsgFactory& operator=( const CGBMsgFactory &rhs );

private:
	static CGBMsgFactory& GetInstance()
	{
		static CGBMsgFactory obj;
		return obj;
	}

private:
	CGBMsgCreater* m_creater[VER_MAX_MSG_VERSION];
};

#endif // GBMESSAGE_HPP_