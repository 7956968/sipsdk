#ifndef GBMSG_CREATER_HPP_
#define GBMSG_CREATER_HPP_

//////////////////////////////////////////////////////////////////////////
/** ���ݵİ汾����
 * 
 *   @note ��Ϣ����ʹ�õİ汾ʹ�õ����鱣���Ӧ����Ϣ�汾�ģ����ö��ֵ�Ķ����Ǳ��뱣��˳��
 */
enum EGBMsgVerion
{
	VER_GB28181 = 0,
	VER_GB28181Ex,
	VER_MAX_MSG_VERSION,
};

//////////////////////////////////////////////////////////////////////////
/**
 *  ������°汾����Ϣ����Ҫ�ػ��汾��CGBMsgCreaterT�����Ҽ̳�CGBMsgCreate
 *  ���磺
 *  CGBMsgCreateT< VER_GB28181 > : public CGBMsgCreate;
 *  CGBMsgCreateT< VER_GB28181Ex > : public CGBMsgCreateT< VER_GB28181 >;
 */
template< EGBMsgVerion T > class CGBMsgCreaterT;

//////////////////////////////////////////////////////////////////////////
class CGBMsgCreater
{
public:
	friend class CGBMsgFactory;
	friend class GBMsgTestCase;

protected:
	CGBMsgCreater()
	{
	}

	virtual ~CGBMsgCreater()
	{
	}

private:
	CGBMsgCreater( const CGBMsgCreater &rhs );
	CGBMsgCreater& operator=( const CGBMsgCreater &rhs );

public:
	virtual CGBMessage* CreateInvite( const char *body, const bool bDecoder, std::string &resaon  ) = 0;
	virtual CGBMessage* CreateInviteResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateAck( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateBye( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateCallInfo( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateCallInfoResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateCallMessage( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateCallMessageResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateMessage( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateMessageResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateNotify( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateNotifyResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateInfo( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateInfoResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateRegisterAdd( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateRegisterResponse( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateRegisterRefresh( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateRegisterRemove( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateRegisterRemoveAll( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateClientUpdateSubscription( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateClientCancelSubscription( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateClientAddSubscription( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateServerAddSubscription( const char *body, std::string &resaon ) = 0;
	virtual CGBMessage* CreateServerNotifySubscription( const char *body, std::string &resaon ) = 0;
};

//////////////////////////////////////////////////////////////////////////
#endif // GBMSG_CREATER_HPP_
