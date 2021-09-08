#pragma once


#ifndef _QQ_SPIDER_H_
#define _QQ_SPIDER_H_

#define _STATIC_

#ifdef _STATIC_
#define _QQ_SPIDER_ __attribute__((cdecl))
#else
#define _QQ_SPIDER_ __declspec(dllexport)
#endif

#include <web-spider/web_spider.h>
#include "qq_information.h"
#include "qq_message.h"
#include "qq_event.h"

typedef struct QQ_SPIDER qq_spider;
struct QQ_SPIDER
{
	char ip[128];
	int port;
	char surl[128];
	char authKey[128];
	char sessionKey[128];
	unsigned int qnumber;
	qq_friend* friend;
	int count;
	qq_group* group;
	int gcount;
	qq_message** lately_message;
	int mcount;
	qq_event* lately_event;
	int ecount;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/* ��ʼ��qq֩�� */
	qq_spider* _QQ_SPIDER_ init_qq_spider(CURL** curl,const char* url, int port, const char* aKey,size_t qnumber);
	/* ������Ϣ������ */
	char* _QQ_SPIDER_ send_message(CURL** curl, qq_spider* bot, size_t qq, char* message);
	/* ���ͻ�����Ϣ�� */
	char* _QQ_SPIDER_ send_basic_message(CURL** curl, qq_spider* bot, size_t qq, char* message);
	/* ������Ϣ��Ⱥ */
	char* _QQ_SPIDER_ send_group_message(CURL** curl, qq_spider* bot, size_t qq, char* message);
	/* ���ͻ�����Ϣ�� */
	char* _QQ_SPIDER_ send_basic_group_message(CURL** curl, qq_spider* bot, size_t qq, char* message);
	/* ��ȡδ����Ϣ���� */
	int _QQ_SPIDER_ unread_message_number(CURL** curl, qq_spider* bot);
	/* ��ȡδ����Ϣ */
	char* _QQ_SPIDER_ get_old_unread_information(CURL** curl, qq_spider* bot,int length);
	/* ��ȡ����δ����Ϣ */
	char* _QQ_SPIDER_ get_new_unread_information(CURL** curl, qq_spider* bot, int length);
	/* ��ȡ�����б� */
	int _QQ_SPIDER_ get_friend_list(CURL** curl, qq_spider* bot);
	/* ��ȡȺ�б� */
	int _QQ_SPIDER_ get_group_list(CURL** curl, qq_spider* bot);
	/* ��ȡȺ��Ա�б� */
	int _QQ_SPIDER_ get_member_list(CURL** curl, qq_spider* bot, size_t target);
	/* ��ȡ�������� */
	int _QQ_SPIDER_ get_friend_profile(CURL** curl, qq_spider* bot, size_t number);
	/* ��ȡȺ��Ա���� */
	int _QQ_SPIDER_ get_member_profile(CURL** curl, qq_spider* bot, size_t group_id, size_t number);
	/* ����ʱ������Ϣ���浽bot */
	int _QQ_SPIDER_ parsing_events_information(const char* data,qq_spider **pbot);
	/* ��Ϣ�� */
	char* _QQ_SPIDER_ message_convert_json(qq_message * msg);
	/* ������Ϣ */
	int _QQ_SPIDER_ destroy_message(qq_message* msg);
	/* ת�� */
	char* _QQ_SPIDER_ convert_code(const char* tocode, const char* fromcode, const char* inbuf);
	/* ƥ������ */
	qq_message* _QQ_SPIDER_ match_command(const char * command,const char* msg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_WEB_SPIDER_H_