#include "qq_spider.h"

#define null NULL

static int test(CURL** cd, qq_spider* qq, int time);
/*
    ^	匹配开头，在多行检测中，会匹配一行的开头
    $	匹配结尾，在多行检测中，会匹配一行的结尾
    n+	匹配任何包含至少一个 n 的字符串
    n*	匹配任何包含零个或多个 n 的字符串
    n?	匹配任何包含零个或一个 n 的字符串
    n{x}	匹配包含 x 个 n 的序列的字符串
    n{x,y}	匹配包含最少 x 个、最多 y 个 n 的序列的字符串
    n{x,}	匹配包含至少 x 个 n 的序列的字符串
    ()  子表达式
    .	查找单个字符，除了换行和行结束符
    \w	查找单词字符
    \W	查找非单词字符
    \d	查找数字
    \D	查找非数字字符
    \s	查找空白字符
    \S	查找非空白字符
    \b	匹配单词边界
    \B	匹配非单词边界
    \0	查找 NUL字符
    \n	查找换行符
    \f	查找换页符
    \r	查找回车符
    \t	查找制表符
    \v	查找垂直制表符
    \xxx	查找以八进制数 xxxx 规定的字符
    \xdd	查找以十六进制数 dd 规定的字符
    \uxxxx	查找以十六进制 xxxx规定的 Unicode 字符
    两个特殊的符号'^'和'$'。他们的作用是分别指出一个字符串的开始和结束。
    * == {0,}
    + == {1,}
    ? == {0,1}
    (H|h)ello == Hello|hello
    [a-e]8192 == [a|b|c|d|e]8192
    [a-z]%123 == 123&[no a-z]
*/
int main(int argc,char ** argv){
	printf("Hello qq.\n");
	CURL * curl = NULL;
	qq_spider * bot = NULL;
	char command[48] = { 0 };
	char * message = NULL;
	char * res = NULL;

	curl = init_web_spider();
	bot = init_qq_spider(&curl, "localhost", 4040,"zzer-qqbot", 127726076);
	message = convert_code("utf-8", "utf-8", "Hello World!!\n你好啊!!我是你的专属NPC");
	get_friend_list(&curl,bot);
	get_group_list(&curl,bot);
	get_friend_profile(&curl, bot, 1589915708);
    get_member_list(&curl, bot, 548590128);
    res = send_message(&curl, bot, 1589915708, message);
	match_command("\\b\\w+\\b","I am hk!!");
	match_command("\\{\\s*\"[T|t]ype\"\\s*:\\s*\"[P|p]lain\"\\s*,\\s*\"text\"\\s*:\\s*\"show\"\\s*\\}","I am hk!!");
    do{
        printf("continue?...\t\t yes/no\n");
        scanf("%s",command);
    }while(strcmp(command,"yes") != 0 && strcmp(command,"no") != 0);
    
    if(strcmp(command,"yes") == 0)  test(&curl, bot, 10010000);
    free(message);

	return 0;
}



int test(CURL** cd,qq_spider * qq,int time) {
	int i = 0;
    while (true) {
        int ret = unread_message_number(cd, qq);
        printf("ret=%d\n\n\n",ret);
        char* old = get_old_unread_information(cd, qq, 10);
        printf("old=%s\n\n\n",old);
        char* new = get_new_unread_information(cd, qq, 10);
        printf("new=%s\n\n\n",new);
        //parsing_events_information(cd, qq, old);
        printf("qq->mcount==%d\n\n\n", qq->mcount);
        parsing_events_information(old, &qq);
        parsing_events_information(new, &qq);
        printf("qq->mcount==%d\n\n\n", qq->mcount);
        char * mg = NULL;
        if(qq->lately_message != NULL){ 
            printf("Not full!!!\n");
            for(i = 0; i < qq->mcount; i++){
                mg = message_convert_json(qq->lately_message[i]);
                printf("mg=%s\n\n\n", mg);
                match_command("\\{\\s*\"[T|t]ype\"\\s*:\\s*\"[P|p]lain\"\\s*,\\s*\"text\"\\s*:\\s*\"show\"\\s*\\}",mg);
                free(mg);
                mg = NULL;
            }
        }
        if(qq->lately_message != NULL) {
		for(i = 0; i < qq->mcount; i++)
	       		destroy_message(qq->lately_message[i]);
		free(qq->lately_message);
		qq->lately_message = NULL;
		qq->mcount = 0;
	}
	printf("qq->mcount==%d\n\n\n", qq->mcount);
#ifdef _WIN32
        Sleep(time);
#endif
#ifdef __linux__
        usleep(time);
#endif
    }
}
