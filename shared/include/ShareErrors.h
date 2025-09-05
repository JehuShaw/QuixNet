/*
 * File:   Errors.h
 * Author: Jehu Shaw
 *
 */

#ifndef SERVER_ERRORS_H
#define SERVER_ERRORS_H

#include "PrintStackTrace.h"
// TODO: handle errors better

// An assert isn't necessarily fatal, but we want to stop anyways
#define WPAssert( EXPR ) if (!(EXPR)) { arcAssertFailed(__FILE__, __LINE__, #EXPR); assert(EXPR); ((void(*)())0)(); }

#define WPError( assertion, errmsg ) if( ! (assertion) ) { PrintError("%s:%i ERROR:\n  %s\n", __FILE__, __LINE__, (char *)errmsg); assert(#assertion &&0); }
#define WPWarning( assertion, errmsg ) if( ! (assertion) ) { PrintError("%s:%i WARNING:\n  %s\n", __FILE__, __LINE__, (char *)errmsg); }

// This should always halt everything.  If you ever find yourself wanting to remove the assert( false ), switch to WPWarning or WPError
#define WPFatal( assertion, errmsg ) if( ! (assertion) ) { PrintError("%s:%i FATAL ERROR:\n  %s\n", __FILE__, __LINE__, (char *)errmsg); assert(#assertion &&0); abort(); }

#define ASSERT WPAssert

#define NEGATIVE_SIGNS -1

enum eServerError {
	CACHE_ERROR_POINTER_NULL = -2147483647,
	CACHE_ERROR_RECORD_EXISTS,
	CACHE_ERROR_DATABASE_INVALID,
	CACHE_ERROR_EMPTY_DATA,
	CACHE_ERROR_EMPTY_TABLE,
	CACHE_ERROR_EMPTY_KEY,
	CACHE_ERROR_EMPTY_VALUES,
	CACHE_ERROR_NOTFOUND,
	CACHE_ERROR_CMD_UNKNOWN,
	CACHE_ERROR_EMPTY_ROUTE,
	CACHE_ERROR_PARSE_REQUEST,
	CACHE_ERROR_SERIALIZE_REQUEST,
	CACHE_ERROR_ROUTE_TYPE,
	SERVER_ERROR_ALREADY_EXIST,
	SERVER_ERROR_SERVER_STOP,
	SERVER_ERROR_NOTFOUND_CHANNEL,
	SERVER_ERROR_SERIALIZE,
	SERVER_ERROR_NOTFOUND_USER,
	SERVER_ERROR_NOTFOUND_CHARACTER,
	// ��Ϣ���޷�����
	PARSE_PACKAGE_FAIL,
	// ·��ID ����
	ROUTE_ID_NULL,
	// �Ҳ������ʵ��
	CANNT_FIND_PLAYER,
	// ����ɫ����
	MAX_CHARARER_LIMIT,
	// MD5 ��֤ʧ��
	AGENT_MD5_CHECK_FAIL,
	//////////////////////////////////
	// �����λ����дҵ��������
	// �����ñ�û���ҵ�����
	CONFIG_NOT_FOUND,
	// �����Ѿ��ӹ���
	QUEST_ALREADY_TAKEN,
	// �������ʧ��
	QUEST_ADD_FAIL,
	// ��������ʧ��
	QUEST_UPDATE_FAIL,
	// ��ȡ����ʧ��
	QUEST_LOAD_FAIL,
	// ��ȡ����ʧ��
	QUEST_GETS_FAIL,
	// ����δ��ȡ
	QUEST_NOT_TAKEN,
	// ��������û�ҵ�
	QUEST_NOT_FOUND,
	// �����Ѿ������
	QUEST_ALREADY_COMPLETED,
	// ���������ڽ��е�״̬
	QUEST_NO_TAKEN_STATUE,
	// �����������δ�ύ״̬
	QUEST_NO_FINISHED_STATUE,
	// �����Է���������
	QUEST_DONNT_ABORT,
	// ��������
	BACKPACK_IS_FULL,
	// ����λ�ò���ȷ
	BACKPACK_POS_INCORRECT,
	// ����ͬ���͵���Ʒ
	BACKPACK_NOT_SAME_TYPE,
	// ��ɫ���Ѿ�����
	PLAYER_NAME_ALREADY_EXIST,
	// û���㹻����Ʒ
	NOT_ENOUGH_ITEM,
	// ���������¼�ʧ��
	QUEST_EVENT_UPDATE_FAIL,
	// ��ɫ����������
	CHARACTER_NAME_LIMIT,
	// û���ҵ���Ʒʵ��
	CANNT_FIND_ITEM,
	// ��Ʒ���ǻ�õ�ʱ�����ʱ����
	NO_ITEM_CREATE_TIME_TYPE,
	// ��Ʒ��û����
	ITEM_NOT_EXPIRED,
	// ��Ʒ�����Գ���
	ITEM_CANNT_SELL,
	// ��Ʒû�е�ʹ��������
	ITEM_NOT_TIME_TO_USE,
	// �ͻ��˴�������˵Ĳ�������ȷ
	CLIENT_PARAM_FAIL,
	// �Ի�δ���
	QUEST_DIALOG_NOT_COMPLETE,
	// ��������ID ��Ч
	QUEST_CFG_ID_INVALID,
	// ����Ŀ�����ô���
	QUEST_CFG_TARGET_INVALID,
	// ����Ŀ�����õĶԻ�ID �Ϳͻ����ύ�Ĳ�ƥ��
	QUEST_DIALOG_CFG_ID_INVALID,

	PET_MODULE_INVALID,
	// �����Ѿ���ȡ
	PET_ALREADY_TAKEN,
	// �������ʧ��
	PET_ADD_FAIL,
	// �����ȡʧ��
	PET_GETS_FAIL,
	// �������ñ���Ч
	PET_PLAYER_DATA_INVALID,
	// ���������Ѵ���
	PET_TYPE_EXIST,
	// ���ﲻ����
	CANNT_FIND_PET,
	// �������ñ�ID����
	PET_TYPE_INVALID,
	// �������ñ����
	PET_TYPE_CONFIG,
	// �����������
	PET_TYPE_TOP_LEVEL,
	// ���������Ѵ�����
	PET_NUM_MAX,
	// ���ﾭ��ֵ��Ч
	PET_EXP_INVALID,
	// ��������ʧ�ܣ��޴���
	PET_UPGRADE_FAIL,
	// �������ʧ�ܣ��޴���,����δ���㣩
	PET_EVOLVE_FAIL,
	// ����������ò���
	PET_EVOLVE_CFGID,

	// ��ȡ������ʧ��
	TRIGGER_GETS_FAIL,
	// ���´�����ʧ��
	TRIGGER_UPDATE_FAIL,

	// �Ҳ�����ͼʵ��
	CANNT_FIND_MAP,
	// �Ҳ�������
	CANNT_FIND_INSTANCE,

	// ����װ�����ñ�û���ҵ�
	PET_EQUIP_CFG_CANNT_FIND,
	// ����װ������ʧ��
	PET_EQUIP_CREATE_FAIL,
	// �Ҳ�������װ��ʵ��
	PET_EQUIP_CANNT_FIND,
	// �Ҳ�����ͼ����
	CANNT_FIND_WORLD,
	// ������ʧ��
	ENTER_WORLD_FAIL,
	// �뿪��ʧ��
	LEAVE_WORLD_FAIL,
	// û�취ʶ���л���ͼ������
	UNKNOW_SWITCH_MAP_TYPE,
	// �Ѿ��ڸõ�ͼ
	ALREADY_ON_THE_MAP,
	// �޽�����ͼ
	PLAYER_UNLOCK_MAP,
	// ���͵㲻�����ŵ�ͼ��
	NO_THIS_MAP_TRANSPORT,
	// δ���������������ĵ�ͼ
	PLAYER_NOT_ALLOW_ENTER_MAP,
	// ������
	WORLD_IS_FULL,
	// ��װ�������쳣
	PLAYER_DRESS_ITEM_COUNT,
	// װ�粿��������
	PLAYER_DRESS_ITEM_NOT_EXIST,
	// װ�粿���ѹ���
	PLAYER_DRESS_ITEM_EXPIRED,
	// װ�粿��δ���� 
	PLAYER_DRESS_ITEM_NOT_EXPIRED,
	// װ�粿�����ñ�û�ҵ�
	PLAYER_DRESS_CFG_CANNT_FIND,
	// װ�粿�����ñ�������Ϊ��
	PLAYER_DRESS_CFG_CREATE,
	// װ�粿������ʧ��
	PLAYER_DRESS_ITEM_CREATE_FAIL,
	// ��Ҳ���ͬһ��
	PLAYER_NOT_IN_SAME_LINE,
	// װ����Ŀ���Ի�λ��ȫ
	PLAYER_DEF_DRESS_ITEM_COUNT,
	// װ�绺������쳣
	PLAYER_DRESS_CACHE_RECORD,
	// װ�����ñ��쳣
	PLAYER_DRESS_CFG_ERR,
	// װ�粿����λ�쳣
	PLAYER_DRESS_PART,
    // װ���Ա𲻷�
    PLAYER_DRESS_GENDER,
	// ���Ѳ�����������Լ�Ϊ����
	FRIEND_CANNT_APPLY_SELF,
    // ˽���Լ�
    PRIVATE_CHAT_SELF,
	// ����������Լ�Ϊ����
	FRIEND_CANNT_ADD_SELF,
	// ������ɾ���Լ�Ϊ����
	FRIEND_CANNT_REMOVE_SELF,
	// �ں�������
	IS_ON_BLACKLIST,
	// ˽��Ŀ��ID Ϊ��
	CHAT_PRIVATE_TARGETID_NULL,
	// �����Ժ��Լ�����
	CHAT_PRIVATE_CANNT_SELF,
	// ���Ǻ���
	NOT_BE_FRIEND,
	// ��������������
	MAX_FRIEND_SIZE_LIMIT,

    // ս�����ñ��쳣
    BATTLE_CONFIG_ERR,
    // ս�����������Ҳ���
    BATTLE_CONFIG_ROUND_CANNOT_FIND,
    // ս����֤����ʱ�䣨С�ڼ���ֵ���ж����ף�
    BATTLE_VERIFY_TIME_INTERVAL,
    // ս����֤�������ô���
    BATTLE_VERIFY_PET,
    // ս����֤�������ô���
    BATTLE_VERIFY_MONSTER,
    // ս����֤����У��ʧ��
    BATTLE_VERIFY_SKILL,
    // ս����֤�˺�У��ʧ��
    BATTLE_VERIFY_HURT,
    // ս����֤Ѫ��У��ʧ��
    BATTLE_VERIFY_HP,

	// �޷�ʶ�����������
	UNKNOW_RANK_TYPE,

	// �ʼ�������ID Ϊ��
	MAIL_RECEIVERID_NULL,
	// �����Է��ʼ����Լ�
	MAIL_CANNT_SEND_SELF,
	// �Ҳ�����Ӧ���ʼ�
	MAIL_CANNT_FOUND,
	// �ʼ������Ѿ���ȡ��
	MAIL_ATTACHMENT_ALREADY_TAKEN,

    // ��ɫID��ƥ��
    PLAYER_ID_NOT_MATCH,
    // ��ɫ��Ч
    PLAYER_IS_INVALID,
    // �Ҳ�����Ӧ��ʵ�������ã�
    ITEM_CANNT_FOUND,
    // �ƺ�δ����
    TITLE_NOT_EXPIRED,

	// ������Ƶ����������
	ADVERT_INCENT_MAX_COUNT_LIMIT,

    // �ϳ�ʧ�ܣ���Ƭ��ȫ
    COMPOSE_CARD_FAIL,
    // ��Ƭ�Ѵ���
    CARD_ALREADY_EXIST,
    // �Ҳ�����Ӧ���ȣ�����
    CANNT_FIND_SCHEDULE,
    // ���������
    SCHEDULE_ALREADY_COMPLETED,
    // ����δ���
    SCHEDULE_COMPLETE_COND,

    // �ǳƷǷ��ַ�
    CHARACTER_NAME_INVALID,

    // ��������ʧ��
    PROLOGUE_PROCESS_FAIL,
	// �������������
	NO_QUEST_LIST_TYPE,

	// Զ�̵��ó�ʱ
	SERVER_CALL_DEADLINE = -1,
	/////////////////////////////////
	// δ��������Ϣ�Ĵ���
	SERVER_ERROR_UNKNOW = 0,
	SERVER_FAILURE = 0,
	SERVER_SUCCESS = 1,
	SERVER_NO_RESPOND = 2,
	// ���λ�ò����ֵ
};

#endif /* SERVER_ERRORS_H */

